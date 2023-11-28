#include "api.h"
#include "CloudAPI.h"
#include "main.h"
#include "CloudThread.h"
#include "nswasabi/AutoCharNX.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudDB.h"
#include "TransactionQueue.h"
#include "nswasabi/AutoCharNX.h"

#ifdef _WIN32
#include "../Winamp/buildType.h"
#include <strsafe.h> // TODO
#include <shlwapi.h>
#endif

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__APPLE__)
#include <sys/utsname.h>
#endif

#ifndef __APPLE__
#include "version.h"
#else
#include "version-mac.h"
#endif
extern CloudAPI cloud_api;

#define QUEUE_MAX 100
CloudThread::CloudThread()
{
	killswitch = 0;
	db_connection = 0;
	device_id = 0;
	pull_required = false;
	first_pull = false;
	device_refresh = 0;
	playlist_refresh = false;
	queue_size = 100;
	dev_mode = 0;
}

CloudThread::~CloudThread()
{
	NXStringRelease(device_id);
	if (db_connection)
		db_connection->Release();
}

ns_error_t CloudThread::Initialize(nx_string_t device_id, int dev_mode)
{
	this->device_id = NXStringRetain(device_id);
	this->dev_mode = dev_mode;

	ns_error_t ret = cloud_socket.Initialize(device_id);
	if (ret != NErr_Success)
		return ret;

	return NXThreadCreate(&cloud_thread, RunFunc, this);
}

nx_thread_return_t CloudThread::Run()
{
	bool user_profile = false, pl_refresh = false;
	Internal_Initialize();

	// this could probably be optimized a bit, but I just want to get this working first
	while (!killswitch)
	{
		#ifndef BILLY_TEST
		if (!first_pull || !user_profile)
		{
			int ret = Internal_UserProfile();
			if (ret == NErr_ConnectionFailed)
			{
				goto connection_failed;
			}
			else if (ret == NErr_Unauthorized)
			{
				goto unauthorized;
			}
			else if (ret != NErr_Success)
			{
				// general failure
				goto connection_failed;
			}

			user_profile = true;
		}

		if (!first_pull || !device_refresh)
		{
			// retrieve a list of existing devices
			int ret = Internal_DevicesList();
			if (ret == NErr_ConnectionFailed)
			{
				goto connection_failed;
			}
			else if (ret == NErr_Unauthorized)
			{
				goto unauthorized;
			}
			else if (ret != NErr_Success)
			{
				// general failure
				goto connection_failed;
			}

			if (!first_pull)
			{
				// tell the server about ours
				Internal_DeviceUpdate();
			}
		}

		Internal_Pull();

		// if we've done a first pull and there were no playlist updates for sanity
		// we do a one time check which allows us to make sure that we are correct
		if (first_pull && !pl_refresh)
		{
			int ret = Internal_Playlists_Snapshot();
			if (ret == NErr_ConnectionFailed)
			{
				goto connection_failed;
			}
			else if (ret == NErr_Unauthorized)
			{
				goto unauthorized;
			}
			else if (ret != NErr_Success)
			{
				// general failure
				goto connection_failed;
			}

			pl_refresh = true;
		}

		// these functions will fill the transactinon queue
		Internal_Announce(); // check for items that need to be announced (mediahash IS NOT NULL and cloud_id IS NULL)
		Internal_Update(); // check for items that need to be updated (dirty=1)
		Internal_Remove(); // check for items that have been removed(dirty=3)
		Internal_Playlists(); // check for playlists to be announced or updated or removed

		// TODO: check for items that have been deleted (dirty=4)
		// TODO: check media-download-next (transfer queue API TBD)
		// TODO: check media-uplodad-next(transfer queue API TBD)
		ProcessQueue();
		#endif
		if (!first_pull || pull_required) // TODO: probably just return an appropriate value from ProcessQueue
		{
			thread_loop.Step(1000);
		}
		else
		{
			// trigger a devices update approx. 30min to keep the device list vaguely in sync
			// with changes made on all other devices - probably can be done nicer than this.
			device_refresh++;
			if (device_refresh > 30) device_refresh = 0;

connection_failed:
#ifdef DEBUG
			thread_loop.Step(5000);
#else
			thread_loop.Step(60000);
#endif
			continue;
unauthorized:
			{
				for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
				{
					itr->OnUnauthorized(this);
				}
				thread_loop.Step();
			}
		}
	}

	if (REPLICANT_API_SSDP)
		REPLICANT_API_SSDP->UnregisterCallback(this);

	return 0;
}

ns_error_t CloudThread::CloudClient_CreateDatabaseConnection(ifc_clouddb **out_connection)
{
	ReferenceCountedNXURI filepath;
	ns_error_t ret = CloudThread::CreatePathForDatabase(&filepath, device_id, dev_mode);
	if (ret != NErr_Success)
		return ret;
	Cloud_DBConnection *db;
	ret = Cloud_DBConnection::CreateConnection(&db, filepath, 0, device_id);
	if (ret != NErr_Success)
		return ret;

	db->Info_Populate(device_id);
	*out_connection = db;

	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_MetadataAnnounce1(ifc_clouddb *_db_connection, int internal_id)
{
	// TODO: not sure about this.  might want to do the transaction on the other thread
	Transaction_Announce *transaction = new (std::nothrow) Transaction_Announce;
	if (!transaction)
		return NErr_OutOfMemory;

	ns_error_t ret = transaction->Initialize(db_connection, attributes, internal_id);
	if (ret != NErr_Success)
	{
		delete transaction;
		return ret;
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
	{
		delete transaction;
		return ret;
	}

	apc->func = APC_Queue;
	apc->param1 = this;
	apc->param2 = transaction;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_RegisterCallback(cb_cloudevents *callback)
{
	if (callback)
	{
		threadloop_node_t *apc = thread_loop.GetAPC();
		if (!apc)
		{
			return NErr_OutOfMemory;
		}

		callback->Retain();
		apc->func = APC_RegisterCallback;
		apc->param1 = this;
		apc->param2 = callback;
		thread_loop.Schedule(apc);
	}

	return NErr_Success;
}

void CloudThread::Internal_Queue(Transaction *transaction)
{
	if (pull_required)
		Internal_Pull();

	transaction_queue.push_back(transaction);

	ProcessQueue();
}

void PopulateAttributes(ifc_clouddb *db_connection, Attributes &attributes, nx_string_t device_token)
{
	attributes.device_token = NXStringRetain(device_token);
	db_connection->Devices_Find(attributes.device_token, &attributes.device_id, 0);
	db_connection->Attribute_Add("artist", &attributes.artist);
	db_connection->Attribute_Add("album", &attributes.album);
	db_connection->Attribute_Add("trackno", &attributes.trackno);
	db_connection->Attribute_Add("albumartist", &attributes.albumartist);
	db_connection->Attribute_Add("bpm", &attributes.bpm);
	db_connection->Attribute_Add("category", &attributes.category);
	db_connection->Attribute_Add("comment", &attributes.comment);
	db_connection->Attribute_Add("composer", &attributes.composer);
	db_connection->Attribute_Add("director", &attributes.director);
	db_connection->Attribute_Add("disc", &attributes.disc);
	db_connection->Attribute_Add("discs", &attributes.discs);
	db_connection->Attribute_Add("genre", &attributes.genre);
	db_connection->Attribute_Add("producer", &attributes.producer);
	db_connection->Attribute_Add("publisher", &attributes.publisher);
	db_connection->Attribute_Add("tracks", &attributes.tracks);
	db_connection->Attribute_Add("year", &attributes.year);
	db_connection->Attribute_Add("albumgain", &attributes.albumgain);
	db_connection->Attribute_Add("trackgain", &attributes.trackgain);
	db_connection->Attribute_Add("rating", &attributes.rating);
	db_connection->Attribute_Add("type", &attributes.type);
	db_connection->Attribute_Add("lossless", &attributes.lossless);
}

void CloudThread::Internal_Initialize()
{
	if (REPLICANT_API_SSDP)
	{
		if (REPLICANT_API_SSDP)
			REPLICANT_API_SSDP->RegisterCallback(this);
	}

	ns_error_t ret = CloudClient_CreateDatabaseConnection((ifc_clouddb **)&db_connection);
	// TODO: what do we do if there's an error?
	db_connection->BeginTransaction();
	PopulateAttributes(db_connection, attributes, device_id);
	// for now we're going to assume that if a Cloud Client is made, that it's a local device
	db_connection->Devices_SetLocal(attributes.device_id);
	db_connection->Devices_ResetLAN();
	db_connection->Commit();

	if (db_connection->Info_GetLogging(&logMode) != NErr_Success)
	{
		db_connection->Info_SetLogging((logMode = 3));
	}
}

void CloudThread::Internal_Announce()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	int internal_id = 0;
	for (ns_error_t ret = db_connection->IDMap_Get_Unannounced(&internal_id, NErr_True); ret == NErr_Success; ret = db_connection->IDMap_Get_Unannounced(&internal_id, NErr_False))
	{
		Transaction_Announce *transaction = new (std::nothrow) Transaction_Announce;
		if (transaction)
		{
			ret = transaction->Initialize(db_connection, attributes, internal_id);
			if (ret != NErr_Success)
			{
				delete transaction;
				continue;
			}

//			db_connection->IDMap_SetAdding(internal_id);
			transaction_queue.push_back(transaction);
		}
	}
}

void CloudThread::Internal_Remove()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	int *internal_ids = 0;
	size_t num_ids = 0;

	if (db_connection->IDMap_Get_To_Remove(&internal_ids, &num_ids) == NErr_Success && num_ids > 0)
	{
		size_t itr = 0;
		while (itr < num_ids)
		{
			Transaction_Delete *transaction = new (std::nothrow) Transaction_Delete;
			if (transaction)
			{
				int internal_id = internal_ids[itr++];
				ns_error_t ret = transaction->Initialize(db_connection, internal_id, callbacks);
				if (ret != NErr_Success)
				{
					delete transaction;
					continue;
				}

				transaction_queue.push_back(transaction);
			}
		}
	}

	
	free(internal_ids);
}

void CloudThread::Internal_Update()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	int *internal_ids = 0;
	int *dirties = 0;
	size_t num_ids = 0;

	if (db_connection->IDMap_Get_Dirty(ifc_clouddb::DIRTY_LOCAL, &internal_ids, &dirties, &num_ids) == NErr_Success && num_ids > 0)
	{
		for (size_t itr=0;itr < num_ids;itr++)
		{
			if (dirties[itr] & ifc_clouddb::DIRTY_FULL)
			{
				Transaction_Update *transaction = new (std::nothrow) Transaction_Update;
				if (transaction)
				{
					int internal_id = internal_ids[itr];
					ns_error_t ret = transaction->Initialize(db_connection, attributes, internal_id, callbacks);
					if (ret != NErr_Success)
					{
						delete transaction;
						continue;
					}

					transaction_queue.push_back(transaction);
				}
			}
			else
			{
				// playcount-only transaction
				Transaction_Played *transaction = new (std::nothrow) Transaction_Played;
				if (transaction)
				{
					int internal_id = internal_ids[itr];
					ns_error_t ret = transaction->Initialize(db_connection, attributes, internal_id, callbacks);
					if (ret != NErr_Success)
					{
						delete transaction;
						continue;
					}

					transaction_queue.push_back(transaction);
				}
			}
		}
	}

	free(internal_ids);
	free(dirties);
}

void CloudThread::Internal_Playlists_Add(nx_string_t uuid, int64_t playlist_id)
{
	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "metadata-playlist-new");
	cloud_socket.JSON_Start_Action(hand, "playlist");
	yajl_gen_map_open(hand);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("id"), playlist_id);
	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("uuid"), uuid);
	// TODO
	cloud_socket.JSON_AddString(hand, JSON_FIELD("content-hash"), "472e061973cf97d91c6f2210f44055fb1b177039");

	nx_string_t name;
	int64_t id = 0, entries = 0, lastupdated = 0, created = 0;
	double duration = 0;
	db_connection->Playlists_Get(uuid, &id, &name, &duration, &entries, &lastupdated, &created);

	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("name"), name);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("entries"), entries);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("duration"), (int64_t)duration);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("duration-exact"), 0);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("created"), lastupdated);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("lastupd"), lastupdated);
//	cloud_socket.JSON_AddNXURI(hand, JSON_FIELD("imported-filepath"), upload->filename);

	yajl_gen_map_close(hand);
	cloud_socket.JSON_End_Action(hand);
	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "metadata-playlist-new");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "metadata-playlist-new");
	//int ret = cloud_socket.PostFile(AutoCharUTF8(cloud_url), upload->filename, buf, len, mime_type, json_hand, upload->callback, 0); 

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value, *revision;
		JSON_Tree_Finish(tree, &value);
		if (value->FindNextKey(0, "rev", &revision, 0) == NErr_Success)
		{
			db_connection->BeginTransaction();
			int64_t revision_number;
			if (revision->GetInteger(&revision_number) == NErr_Success)
			{
				db_connection->Info_SetRevision(revision_number);
			}
			db_connection->Playlists_Removed(uuid);
			db_connection->Commit();
		}
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, 0, 0);
		}
	}
	/*else
	{
		// if we have a not known then could be we're out of sync
		// so we drop the playlist and attempt to ask the user...
		const JSON::Value *value = 0, *code = 0, *error = 0;
		JSON_Tree_Finish(tree, &value);
		if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success)
		{
			if (error->FindNextKey(0, "code", &code, 0) == NErr_Success)
			{
				int64_t code_int = 0;
				code->GetInteger(&code_int);

				if (code_int == 80)
				{
					db_connection->Playlists_Removed(uuid);
					db_connection->Commit();

					for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
					{
						itr->OnPlaylistRemove(this, uuid);
					}
				}
			}
		}
	}*/

	JSON_Tree_Destroy(tree);
}

void CloudThread::Internal_Playlists_Remove(nx_string_t uuid, int64_t playlist_id)
{
	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "metadata-playlist-remove");
	cloud_socket.JSON_Start_Action(hand, "playlist");
	yajl_gen_map_open(hand);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("id"), playlist_id);
	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("uuid"), uuid);
	yajl_gen_map_close(hand);
	cloud_socket.JSON_End_Action(hand);
	cloud_socket.JSON_End(hand);

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "metadata-playlist-remove");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value, *revision;
		JSON_Tree_Finish(tree, &value);
		if (value->FindNextKey(0, "rev", &revision, 0) == NErr_Success)
		{
			db_connection->BeginTransaction();
			int64_t revision_number;
			if (revision->GetInteger(&revision_number) == NErr_Success)
			{
				db_connection->Info_SetRevision(revision_number);
			}
			db_connection->Playlists_Removed(uuid);
			db_connection->Commit();

			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnPlaylistRemove(this, db_connection, uuid);
			}
		}
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString action, message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);
		NXStringCreateWithUTF8(&action, "metadata-playlist-remove");

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, 0, 0);
		}
	}
	else
	{
		// if we have a not known then could be we're out of sync
		// so we drop the playlist and attempt to ask the user...
		const JSON::Value *value = 0, *code = 0, *error = 0;
		JSON_Tree_Finish(tree, &value);
		if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success)
		{
			if (error->FindNextKey(0, "code", &code, 0) == NErr_Success)
			{
				int64_t code_int = 0;
				code->GetInteger(&code_int);

				if (code_int == 80)
				{
					db_connection->Playlists_Removed(uuid);
					db_connection->Commit();

					for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
					{
						itr->OnPlaylistRemove(this, db_connection, uuid);
					}
				}
			}
		}
	}

	JSON_Tree_Destroy(tree);
}

void CloudThread::Internal_Playlists()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	nx_string_t *uuids = 0;
	int64_t *playlist_ids = 0;
	int *dirties = 0;
	size_t num_playlists = 0;

	if (db_connection->Playlists_Get_Dirty(&uuids, &playlist_ids, &dirties, &num_playlists) == NErr_Success && num_playlists > 0)
	{
		for (size_t i = 0; i < num_playlists; i++)
		{
			// TODO need to make sure we're not uploading unless we need to, etc
			if (dirties[i] & ifc_clouddb::PLAYLIST_LOCAL_ADD)
			{
				UploadPlaylist(uuids[i], playlist_ids[i], 0);
			}
			else if (dirties[i] & ifc_clouddb::PLAYLIST_LOCAL_UPDATE)
			{
				UploadPlaylist(uuids[i], playlist_ids[i], 1);
			}
			else if (dirties[i] & ifc_clouddb::PLAYLIST_REMOVE)
			{
				Internal_Playlists_Remove(uuids[i], playlist_ids[i]);
			}
		}
	}

	free(playlist_ids);
	free(dirties);
	free(uuids);
}

int CloudThread::Parse_Error(const JSON::Value *value, const JSON::Value *code, const char *subdir)
{
	const JSON::Value *error = 0, *str = 0, *field = 0;
	if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success)
	{
		int64_t code_int = 0;

		ReferenceCountedNXString code_string, str_string, field_string, action;
		if (error->FindNextKey(0, "code", &code, 0) == NErr_Success)
		{
			code->GetString(&code_string);
			code->GetInteger(&code_int);
		}
		if (error->FindNextKey(0, "str", &str, 0) == NErr_Success)
		{
			str->GetString(&str_string);
		}
		if (error->FindNextKey(0, "field", &field, 0) == NErr_Success)
		{
			field->GetString(&field_string);
		}
		else if (error->FindNextKey(0, "details", &field, 0) == NErr_Success)
		{
			field->GetString(&field_string);
		}
		else if (error->FindNextKey(0, "detail", &field, 0) == NErr_Success)
		{
			field->GetString(&field_string);
		}

		if (subdir)
		{
			NXStringCreateWithUTF8(&action, subdir);
			for (CallbackList::iterator itr = callbacks.begin(); itr != callbacks.end(); itr++)
			{
				itr->OnError(this, action, code_string, str_string, field_string);
			}
		}

		if (code_int == 50) // revision behind.  harmless
			return 2;
		else if (code_int == 51) // revision ahead. catastrophic
			return 3;
		else
			return 1; // some other general error
		
	}
	return 0;
}

void CloudThread::ProcessQueue()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	if (transaction_queue.empty())
		return;

	queue_size = QUEUE_MAX;

	do
	{
		int id_iterator = 0;
		bool do_logging = Config_GetLogging();
		yajl_gen hand = yajl_gen_alloc(0);
		if (do_logging)
			yajl_gen_config(hand, yajl_gen_beautify);
		cloud_socket.JSON_Start(db_connection, hand, "metadata-push");
		ReferenceCountedNXString revision_id;
		if (CloudSocket::GenerateRevisionID(&revision_id) == NErr_Success)
			cloud_socket.JSON_AddNXString(hand, JSON_FIELD("rid-new"), revision_id);
		
		cloud_socket.JSON_Start_Actions(hand);
		Transaction *transaction = transaction_queue.front();
		bool empty = true;
		size_t i = 0;
		for (; transaction && i < queue_size ; i++, transaction = (Transaction *)transaction->next)
		{
			if (transaction->GenerateJSON(db_connection, &cloud_socket, &id_iterator, hand) == NErr_Success)
				empty=false;

			if (queue_size == 1) break;
		}

		if (empty)
		{
			yajl_gen_free(hand);
			i = 0;
			transaction = transaction_queue.front();
			do
			{
				transaction_queue.erase(transaction);
				delete transaction;
				transaction = transaction_queue.front();
				i++;
			} while (transaction && i < queue_size);

			continue;
		}

		cloud_socket.JSON_End_Actions(hand);
		cloud_socket.JSON_End(hand);

		nsjson_tree_t tree;
		int ret = PostJSON(hand, &tree, "push");

		yajl_gen_free(hand);

		if (ret == NErr_Success)
		{
			db_connection->BeginTransaction();
			i = 0;
			transaction = transaction_queue.front();
			do
			{
				transaction->OnTransactionAccepted(db_connection);
				transaction_queue.erase(transaction);
				delete transaction;
				transaction = transaction_queue.front();
				i++;
			} while (transaction && i < queue_size);

			const JSON::Value *value, *revision;
			JSON_Tree_Finish(tree, &value);
			if (value->FindNextKey(0, "rev", &revision, 0) == NErr_Success)
			{
				int64_t revision_number;
				if (revision->GetInteger(&revision_number) == NErr_Success)
				{
					db_connection->Info_SetRevision(revision_number);
				}
			}
			if (value->FindNextKey(0, "rid", &revision, 0) == NErr_Success)
			{
				ReferenceCountedNXString revision_id;
				if (revision->GetString(&revision_id) == NErr_Success)
				{
					db_connection->Info_SetRevisionID(revision_id);
				}
				else
				{
					db_connection->Info_SetRevisionID(0);
				}
			}
			else
			{
				db_connection->Info_SetRevisionID(0);
			}
			db_connection->Commit();
			
		}
		else if (ret == NErr_Conflict)
		{
			// get the error code and if it relates to a revision issue, abort
			// and then try to nuke the db so we can do a re-pull to sort it out
			// including acting like we're doing a first pull for client updates
			const JSON::Value *value = 0, *code = 0;
			JSON_Tree_Finish(tree, &value);
			ret = Parse_Error(value, code, 0);
			if (ret == 3)
			{
				transaction_queue.deleteAll();
				first_pull = false;
				pull_required = true;
				db_connection->BeginTransaction();
				db_connection->Reset_All();
				db_connection->Commit();

				ReferenceCountedNXString action;
				NXStringCreateWithUTF8(&action, "forced-pull");
				for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
				{
					itr->OnAction(this, action, 0);
				}

			}
			else if (ret == 2)
			{
				// some other device snuck a revision.  we'll do a pull and try again
				pull_required = true;
				transaction_queue.deleteAll();
			}
			else
			{

				// TODO probably needs to be checked over but this will
				// do like with errors in trying to drop down the part
				// causing the conflict and remove it where possible
				// as just transaction_queue.deleteAll(); doesn't do it
				if (queue_size == 1)
				{
					// if an error happens then remove from the queue
					// and in the case of 'announces' we set ignore=1
					db_connection->BeginTransaction();
					transaction->OnTransactionError(db_connection);
					db_connection->Commit();
					transaction_queue.erase(transaction);
					delete transaction;
					transaction = transaction_queue.front();
				}
				else
				{
					// if we hit an error, where possible adjust the
					// size of the queue to what there is to work on
					// as it helps to speed up finding the error id.
					if (queue_size == QUEUE_MAX)
					{
						int size = transaction_queue.size();
						if (size < QUEUE_MAX)
						{
							queue_size = size;
						}
					}

					queue_size /= 4; // reduce queue size to try to get things moving
					if (queue_size == 0)
					{
						queue_size = 1;
					}
				}
			}
		}
		else if (ret == NErr_InternalServerError || ret == NErr_ServiceUnavailable)
		{
			// server is hating life right now.  we'll kill the queue and try later
			transaction_queue.deleteAll();
		}
		else
		{
				// benski> I took these lines out because _sometimes_ we don't get a JSON response.
			// and we aren't using the 'code' yet anyway
			//const JSON::Value *value = 0, *code = 0;
			//JSON_Tree_Finish(tree, &value);
			//if (Parse_Error(value, code, "push"))

			{
				// TODO need to determine which codes this relates to...
				// audiodsp:	yeah right now if it fails it just puts it back into the queue.
				//				if you take the code out.  be careful. because you do have to put
				//				it back in the queue if the error was "revision out of date"
				if (queue_size == 1)
				{
					// if an error happens then remove from the queue
					// and in the case of 'announces' we set ignore=1
					db_connection->BeginTransaction();
					transaction->OnTransactionError(db_connection);
					db_connection->Commit();
					transaction_queue.erase(transaction);
					delete transaction;
					transaction = transaction_queue.front();
				}
				else
				{
					// if we hit an error, where possible adjust the
					// size of the queue to what there is to work on
					// as it helps to speed up finding the error id.
					if (queue_size == QUEUE_MAX)
					{
						int size = transaction_queue.size();
						if (size < QUEUE_MAX)
						{
							queue_size = size;
						}
					}

					queue_size /= 4; // reduce queue size to try to get things moving
					if (queue_size == 0)
					{
						queue_size = 1;
					}
				}
			}
		}
		JSON_Tree_Destroy(tree);
	} while (!transaction_queue.empty());
}

int CloudThread::PostJSON(yajl_gen json, nsjson_tree_t *output, const char *subdir)
{
	bool do_logging = Config_GetLogging();
	ReferenceCountedNXString cloud_url;
	yajl_handle json_hand=0;
	nsjson_tree_t tree=0;
	int server_ret;
	const unsigned char *buf;
	size_t len;

	if (output)
		*output=0;
	yajl_gen_get_buf(json, &buf, &len);
	REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_False);
	if (output)
	{
		JSON_Tree_CreateParser(&tree);
		JSON_Tree_GetHandle(tree, &json_hand);
		*output=tree;
	}

	if (do_logging)
	{
		Logger logger(subdir, Logger::LOG_REQUEST_JSON);
		server_ret = cloud_socket.PostJSON(AutoCharUTF8(cloud_url), buf, len, json_hand, &logger);
		if (server_ret != NErr_Success && !Config_GetSlimLogging())
								logger.UpdateError(1);
		if (server_ret != NErr_ConnectionFailed)
		{
			const JSON::Value *value = 0;
			if (JSON_Tree_Finish(tree, &value) == NErr_Success)
			{
				const JSON::Value *error = 0;
				if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success ||
					!Config_GetSlimLogging())
				{
					logger.UpdateError(1);
				}
#ifdef DEBUG
				if (value) value->Dump(logger.ParseTest(), 0);
#endif
			}
		}
	}
	else
	{
		server_ret = cloud_socket.PostJSON(AutoCharUTF8(cloud_url), buf, len, json_hand, 0); 
	}

	// only process error responses if there was anything which came back
	if (server_ret != NErr_ConnectionFailed)
	{
		const JSON::Value *value = 0;
		JSON_Tree_Finish(tree, &value);
		// skip 'push' errors as it's handled later
		if (!strstr(subdir, "push") || server_ret == NErr_Conflict)
		{
			const JSON::Value *code = 0;
			Parse_Error(value, code, subdir);
		}
	}

	return server_ret;
}

ns_error_t CloudThread::CloudClient_Flush(bool restart)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
		return NErr_OutOfMemory;

	apc->func = APC_Flush;
	apc->param1 = this;
	thread_loop.Schedule(apc);
	// helps effectively act like a live re-start
	// e.g. desktop logged out and then logged in
	if (restart) first_pull = 0;
	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_Reset()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
		return NErr_OutOfMemory;

	apc->func = APC_Reset;
	apc->param1 = this;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_DeviceUpdate()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
		return NErr_OutOfMemory;

	apc->func = APC_DeviceUpdate;
	apc->param1 = this;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_DeviceRemove(RemoveStruct *remove)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
		return NErr_OutOfMemory;

	apc->func = APC_DeviceRemove;
	apc->param1 = this;
	apc->param2 = remove;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

ns_error_t CloudThread::CloudClient_DeviceRename(RenameStruct *rename)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
		return NErr_OutOfMemory;

	apc->func = APC_DeviceRename;
	apc->param1 = this;
	apc->param2 = rename;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

ns_error_t CloudThread::Internal_Pull()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return NErr_False;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "metadata-pull");
	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "pull");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret=PostJSON(hand, &tree, "pull");

	// TODO: we need to check for error 51 (revision ahead)
	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		// pass on to the client if we're doing a forced pull update
		// e.g. from a revision error needing a local db reset, etc
		bool forced = pull_required;
		pull_required = false;
		const JSON::Value *value = 0;
		JSON_Tree_Finish(tree, &value);
		if (Parse_Pull(value) == NErr_Success)
		{
			// if the pull succeded then send notification to clients
			// so they can then start any appropriate processing, etc
			// as improves reliability for new adds with a pull done
			if (!first_pull)
			{
				if (playlist_refresh)
				{
					Internal_Playlists_Snapshot();
				}

				first_pull = true;
				for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
				{
					itr->OnFirstPull(this, forced);
				}
			}
		}
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);

		if (!first_pull)
		{
			NXStringCreateWithUTF8(&message, "First Pull Failed - Connection To Cloud Server Failed");
		}
		else
		{
			NXStringCreateWithUTF8(&message, "Connection To Cloud Server Failed - Check Server Is Up And Plug-in Is Configured Correctly");
		}

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, message, 0);
		}
	}

	JSON_Tree_Destroy(tree);
	return ret;
}

void CloudThread::Internal_Reset()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "push");
	cloud_socket.JSON_Start_Actions(hand);
	cloud_socket.JSON_Start_Action(hand, "test-repo-reset");
	yajl_gen_map_open(hand);
	yajl_gen_map_close(hand);
	cloud_socket.JSON_End_Action(hand);
	cloud_socket.JSON_End_Actions(hand);
	cloud_socket.JSON_End(hand);

	nsjson_tree_t tree;
	int ret=PostJSON(hand, &tree, "reset");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		if (JSON_Tree_Finish(tree, &value) == NErr_Success)
		{
			int64_t revision = 0;
			if (value->data_type != JSON::DATA_MAP)
			{
				ret = NErr_Malformed;
			}
			else
			{
				const JSON::Value *found;
				if (value->FindNextKey(0, "rev", &found) != NErr_Success)
				{
					ret = NErr_Malformed;
				}
				else
				{
					ret = found->GetInteger(&revision);
				}
			}

			if (ret == NErr_Malformed) revision = -2;
			else if (ret != NErr_Success) revision = -1;

			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnRevision(this, revision, 1);
			}
		}
	}

	JSON_Tree_Destroy(tree);
}

static ns_error_t GetName(nx_string_t *device_name)
{
#ifdef _WIN32
	wchar_t computer[256];
	DWORD buffer_size_computer=256;
	if (GetComputerNameW(computer, &buffer_size_computer))
		return NXStringCreateWithUTF16(device_name, computer);
	else
		return NErr_Error;
#elif (defined(__linux__) && !defined(__ANDROID__)) || defined(__APPLE__)
	struct utsname ver;
	uname(&ver);
	return NXStringCreateWithUTF8(device_name, ver.nodename);
#elif defined(__ANDROID__)
	return NErr_NotImplemented;

#endif
}

#ifdef _WIN32
static void GetWindowsVersion(wchar_t version_string[256])
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize=sizeof(info);
	GetVersionEx(&info);
	StringCchPrintfW(version_string, 256, L"%u.%u.%u", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
}

static BOOL IsALaptop(void) {
	static int power_first = 1;
	typedef BOOL (WINAPI *getSystemPowerStatus)(LPSYSTEM_POWER_STATUS);
	static getSystemPowerStatus pGetSystemPowerStatus;

	if (power_first)
	{
		HMODULE hlib = LoadLibrary(L"KERNEL32.DLL");
		pGetSystemPowerStatus = (getSystemPowerStatus)GetProcAddress(hlib, "GetSystemPowerStatus");
		power_first = 0;
	}

	if (pGetSystemPowerStatus)
	{
		SYSTEM_POWER_STATUS SystemPowerStatus = {0};
		if (pGetSystemPowerStatus(&SystemPowerStatus))
		{
			return (SystemPowerStatus.BatteryFlag != 128 && SystemPowerStatus.BatteryFlag != 255);
		}
	}
	return 0;
}

#endif

void CloudThread::Internal_DeviceUpdate()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "user-devices-update");
	yajl_gen_string(hand, JSON_FIELD("devices"));
	yajl_gen_array_open(hand);
	{
		yajl_gen_map_open(hand);
		{
			cloud_socket.JSON_AddNXString(hand, JSON_FIELD("dev"), attributes.device_token);
			ReferenceCountedNXString device_name;
			if (db_connection->Info_GetDeviceName(&device_name) == NErr_Success)
			{
				cloud_socket.JSON_AddNXString(hand, JSON_FIELD("name"), device_name);
			}

#pragma region descr
			yajl_gen_string(hand, JSON_FIELD("descr"));
			yajl_gen_map_open(hand);
#pragma region descr/client
			yajl_gen_string(hand, JSON_FIELD("client"));
			yajl_gen_map_open(hand);
			{
				ReferenceCountedNXString winamp_version, product_short_name;
				if (WASABI2_API_APP->GetProductShortName(&product_short_name) == NErr_Success)
					cloud_socket.JSON_AddNXString(hand, JSON_FIELD("name"), product_short_name);
				if (WASABI2_API_APP->GetVersionString(&winamp_version) == NErr_Success)
					cloud_socket.JSON_AddNXString(hand, JSON_FIELD("v"), winamp_version);
#ifndef __APPLE__
				cloud_socket.JSON_AddInteger(hand, JSON_FIELD("p"), cloud_build_number);	
#else
				cloud_socket.JSON_AddInteger(hand, JSON_FIELD("p"), cloud_version_major);
#endif	
				cloud_socket.JSON_AddInteger(hand, JSON_FIELD("b"), WASABI2_API_APP->GetBuildNumber());
			}
			yajl_gen_map_close(hand);
#pragma endregion

#pragma region descr/os
			yajl_gen_string(hand, JSON_FIELD("os"));
			yajl_gen_map_open(hand);
#ifdef _WIN32
			cloud_socket.JSON_AddString(hand, JSON_FIELD("name"), L"Windows NT");
			wchar_t windows_version[256];
			GetWindowsVersion(windows_version);
			cloud_socket.JSON_AddString(hand, JSON_FIELD("v"), windows_version);
#elif (defined(__linux__) && !defined(__ANDROID__)) || defined(__APPLE__)
			struct utsname ver;
			uname(&ver);
			cloud_socket.JSON_AddString(hand, JSON_FIELD("name"), ver.sysname);
			cloud_socket.JSON_AddString(hand, JSON_FIELD("v"), ver.release);
			cloud_socket.JSON_AddString(hand, JSON_FIELD("m"), ver.machine);
#elif defined(__ANDROID__)
			cloud_socket.JSON_AddString(hand, JSON_FIELD("name"), "Android");
			if (WASABI2_API_ANDROID)
			{
				ReferenceCountedNXString android_release;
				if (WASABI2_API_ANDROID->GetRelease(&android_release) == NErr_Success)
					cloud_socket.JSON_AddNXString(hand, JSON_FIELD("v"), android_release);
				//TODO					cloud_socket.JSON_AddString(hand, JSON_FIELD("m"), ver.machine);
			}
#endif
			yajl_gen_map_close(hand);
#pragma endregion

#pragma region descr/hardware
			yajl_gen_string(hand, JSON_FIELD("hardware"));
			yajl_gen_map_open(hand);

#ifdef _WIN32
			yajl_gen_string(hand, JSON_FIELD("mobile"));
			yajl_gen_bool(hand, 0);
			yajl_gen_string(hand, JSON_FIELD("platform"));
			yajl_gen_string(hand, JSON_FIELD("windows"));

			// attempts to see if this is a laptop - checks for a battery
			// which isn't ideal but is crude enough for what is wanted
			if (IsALaptop())
			{
				yajl_gen_string(hand, JSON_FIELD("type"));
				yajl_gen_string(hand, JSON_FIELD("laptop"));
			}
			/*else
			{
			yajl_gen_string(hand, JSON_FIELD("type"));
			yajl_gen_null(hand);
			}*/
#elif (defined(__linux__) && !defined(__ANDROID__)) || defined(__APPLE__)
			yajl_gen_string(hand, JSON_FIELD("mobile"));
			yajl_gen_bool(hand, 0);
			yajl_gen_string(hand, JSON_FIELD("platform"));
			yajl_gen_string(hand, JSON_FIELD("apple"));
#elif defined(__ANDROID__)
			yajl_gen_string(hand, JSON_FIELD("mobile"));
			yajl_gen_bool(hand, 1);
			yajl_gen_string(hand, JSON_FIELD("platform"));
			yajl_gen_string(hand, JSON_FIELD("android"));
#endif
			yajl_gen_map_close(hand);
#pragma endregion

			yajl_gen_map_close(hand);
#pragma endregion

#pragma region capab
			yajl_gen_string(hand, JSON_FIELD("capab"));
			yajl_gen_map_open(hand);
#pragma region capab/xfer
			yajl_gen_string(hand, JSON_FIELD("xfer"));
			yajl_gen_map_open(hand);
#pragma region capab/xfer/rx
			yajl_gen_string(hand, JSON_FIELD("rx"));
			yajl_gen_map_open(hand);
			cloud_socket.JSON_AddInteger(hand, JSON_FIELD("media-download-queue"), 1);
			yajl_gen_map_close(hand);
#pragma endregion
			yajl_gen_map_close(hand);
#pragma endregion
			yajl_gen_map_close(hand);
#pragma endregion
		}
		yajl_gen_map_close(hand);
	}
	yajl_gen_array_close(hand);

	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "user-devices-update");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "user-devices-update");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);
		Parse_DevicesList(value);
	}

	JSON_Tree_Destroy(tree);
}

void CloudThread::Internal_DeviceRename(RenameStruct *rename)
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	ReferenceCountedNXString message, action;
	NXStringCreateWithUTF8(&action, "user-devices-update"); // TODO: make this global singleton that's created at initialization
	NXStringCreateWithFormatting(&message, "Changing Device Name [%s -> %s]", AutoCharPrintfUTF8(rename->old_name), AutoCharPrintfUTF8(rename->name));

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(0, action, message);
	}

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "user-devices-update");
	yajl_gen_string(hand, JSON_FIELD("devices"));
	yajl_gen_array_open(hand);
	{
		yajl_gen_map_open(hand);
		{
			cloud_socket.JSON_AddNXString(hand, JSON_FIELD("dev"), rename->device);
			cloud_socket.JSON_AddNXString(hand, JSON_FIELD("name"), rename->name);
		}
		yajl_gen_map_close(hand);
	}
	yajl_gen_array_close(hand);

	cloud_socket.JSON_End(hand);

	nsjson_tree_t tree;
	int ret=PostJSON(hand, &tree, "user-devices-update");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);
		Parse_DevicesList(value);
	}

	JSON_Tree_Destroy(tree);
	delete rename;
}

void CloudThread::Internal_DeviceRemove(RemoveStruct *remove)
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return;

	ReferenceCountedNXString message, action;
	NXStringCreateWithUTF8(&action, "user-device-remove"); // TODO: make this global singleton that's created at initialization
	NXStringCreateWithFormatting(&message, "Requesting Device Remove [%s]", AutoCharPrintfUTF8(remove->device_token));
	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, message);
	}

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "user-device-remove");
	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("dev-target"), remove->device_token);
	cloud_socket.JSON_End(hand);

	nsjson_tree_t tree;
	int ret=PostJSON(hand, &tree, "user-device-remove");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);
	}

	JSON_Tree_Destroy(tree);
	// only do a devices refresh if all ok
	if (ret == NErr_Success)
	{
		Internal_DevicesList();
	}
	delete remove;
}

void CloudThread::Internal_RegisterCallback(cb_cloudevents *callback)
{
	callbacks.push_back(callback);
	// call if the client was slow to be registered
	if (first_pull) callback->OnFirstPull(this, 0);

	if (callback->CloudEvents_GetRunDeviceEnum())
	{
		nx_string_t *devices = 0;
		size_t num_devices = 0;
		if (db_connection->Devices_GetIDs(&devices, &num_devices) == NErr_Success)
		{
			for (size_t i=0;i<num_devices;i++)
			{
				DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct;
				int device_id;
				if (db_connection->Devices_Find(devices[i], &device_id, device_info) == NErr_Success)
				{
					callback->OnDeviceAdded(this, devices[i], device_id, device_info);
				}
				NXStringRelease(devices[i]);
				delete device_info;
			}
		}
		if (devices) free(devices);
	}
}

ns_error_t CloudThread::CreatePathForDatabase(nx_uri_t *filepath, nx_string_t device_id, int dev_mode)
{
	ReferenceCountedNXURI nx_filename;	
	ReferenceCountedNXURI settings_path;

	WASABI2_API_APP->GetDataPath(&settings_path);

	const char *prefix=0;
	switch(dev_mode)
	{
	case 0: // production
		prefix="local";
		break;
	case 1: // dev
		prefix="cloud";
		break;
	case 2: // qa
		prefix="qa";
		break;
	case 3:
		prefix="stage";
		break;
	}
#ifdef _WIN32
	char database_filename[MAX_PATH];
	sprintf(database_filename, "Cloud\\%s-%.64s.db", prefix, AutoCharPrintfUTF8(device_id));
#else
	char database_filename[128];
	sprintf(database_filename, "%s-%.64s.db", prefix, AutoCharPrintfUTF8(device_id));
#endif
	NXURICreateWithUTF8(&nx_filename, database_filename);
	return NXURICreateWithPath(filepath, nx_filename, settings_path);
}

ns_error_t CloudThread::Internal_DevicesList()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return NErr_False;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "user-devices");

	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "user-devices");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "user-devices");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value = 0;
		JSON_Tree_Finish(tree, &value);
		Parse_DevicesList(value);
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, 0, 0);
		}
	}

	JSON_Tree_Destroy(tree);
	return ret;
}

ns_error_t CloudThread::Internal_UserProfile()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return NErr_False;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "user-profile");

	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "user-profile");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "user-profile");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);
		Parse_UserProfile(value);
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, 0, 0);
		}
	}

	JSON_Tree_Destroy(tree);
	return ret;
}

ns_error_t CloudThread::Internal_Playlists_Snapshot()
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
		return NErr_False;

	bool do_logging = Config_GetLogging();
	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, "metadata-snapshot-playlists");

	cloud_socket.JSON_End(hand);

	ReferenceCountedNXString action;
	NXStringCreateWithUTF8(&action, "metadata-snapshot-playlists");

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnAction(this, action, 0);
	}

	nsjson_tree_t tree;
	int ret = PostJSON(hand, &tree, "metadata-snapshot-playlists");

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);
		Parse_Playlist_Snapshot(value);
	}
	else if (ret == NErr_ConnectionFailed)
	{
		ReferenceCountedNXString message, code;
		NXStringCreateWithFormatting(&code, "%d", NErr_ConnectionFailed);

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code, 0, 0);
		}
	}

	JSON_Tree_Destroy(tree);
	return ret;
}

#pragma region SSDP Callbacks
/* --- SSDP Stuff --- */
void CloudThread::SSDPCallback_OnServiceConnected(nx_uri_t location, nx_string_t type, nx_string_t usn)
{
	if (!NXStringKeywordCompareWithCString(type, "urn:nullsoft-com:Sync:1"))
	{
		LANDeviceStruct *data = new (std::nothrow) LANDeviceStruct;
		if (data)
		{
			data->usn = NXStringRetain(usn);
			data->location = NXURIRetain(location);

			threadloop_node_t *apc = thread_loop.GetAPC();
			if (!apc)
			{
				delete data;
				return ;
			}

			apc->func = APC_OnServiceConnected;
			apc->param1 = this;
			apc->param2 = data;
			thread_loop.Schedule(apc);
		}
	}
}

void CloudThread::SSDPCallback_OnServiceDisconnected(nx_string_t usn)
{
	// with only a USN, we don't know if it's a service type we care about until we check our records
	// so we'll just bump it off the cloud client thread and have it check from there
	// service disconnects should be uncommon enough that this won't be a problem
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnServiceDisconnected;
		apc->param1 = this;
		apc->param2 = NXStringRetain(usn);
		thread_loop.Schedule(apc);
	}	
}

void CloudThread::Internal_OnServiceConnected(LANDeviceStruct *data)
{
	// TODO: benski> for now.  we really need to get the XML from the location
	AutoCharUTF8 x(data->usn);
	ReferenceCountedNXString woo;
	NXStringCreateWithUTF8(&woo, x + 25); // skip urn:nullsoft-com:Sync:1::   Hacky! I know!
	data->device_token = NXStringRetain(woo);
	int device_id;
	if (db_connection->Devices_Find(woo, &device_id, 0) == NErr_Success)
	{
		db_connection->Devices_SetLAN(device_id, 1);
	}
	lan_devices.push_back(data);
}

void CloudThread::Internal_OnServiceDisconnected(nx_string_t usn)
{
	LANDevices::iterator itr = lan_devices.begin();
	while (itr != lan_devices.end())
	{
		if (!NXStringCompare((*itr)->usn, usn, nx_compare_default))
		{
			int device_id;
			if (db_connection->Devices_Find(usn, &device_id, 0) == NErr_Success)
			{
				db_connection->Devices_SetLAN(device_id, 1);
			}

			LANDeviceStruct *data = *itr;
			lan_devices.erase(data);
			delete data;
			break;
		}
		itr++;
	}
}
#pragma endregion
