#include "api.h"
#include "main.h"
#include "CloudThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include "nswasabi/AutoCharNX.h"
#include "CloudDB.h"

extern CloudAPI cloud_api;

int CloudThread::DownloadPlaylist(UploadStruct *upload)
{
	if (!upload) return NErr_Error;

	ReferenceCountedNXString username, cloud_url;
	REPLICANT_API_CLOUD->Cloud_GetAPIURL(&cloud_url, /*http=*/NErr_True);
	if (REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0) != NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unauthorized);
		return NErr_Success; // TODO?
	}

	char url[1024];
	sprintf(url, "%splaylist/%s/%s", AutoCharPrintfUTF8(cloud_url), AutoCharPrintfUTF8(username), AutoCharPrintfUTF8(upload->destination_device));

	yajl_handle json_hand=0;
	nsjson_tree_t tree;
	JSON_Tree_CreateParser(&tree);
	JSON_Tree_GetHandle(tree, &json_hand);

	int ret;
	bool do_logging = Config_GetLogging();
	if (do_logging)
	{
		Logger logger("download-playlists", Logger::LOG_ALL);
		ret = cloud_socket.DownloadFile(url, json_hand, 0, 0, upload->callback, &logger);
		if (ret != NErr_ConnectionFailed)
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
			}
		}
	}
	else
	{
		ret = cloud_socket.DownloadFile(url, json_hand, 0, 0, upload->callback, 0);
	}

	if (ret != NErr_ConnectionFailed)
	{
		const JSON::Value *value=0, *playlist=0;
		if (JSON_Tree_Finish(tree, &value) == NErr_Success)
		{
			if (value->FindNextKey(0, "playlist", &playlist) == NErr_Success && playlist->data_type == JSON::DATA_MAP)
			{
				// TODO since i don't know what other clients are going to need,
				//		for the moment am just providing the server response and
				//		leaving the client in the callback to process as needed.
				//		probably should do something better but it's a start...
				if (upload->callback)
					upload->callback->OnDownloadFinished(upload->filename, db_connection, playlist);
			}
		}
	}

	JSON_Tree_Destroy(tree);
	return NErr_Success;
}

void CloudThread::Internal_DownloadPlaylist(UploadStruct *upload)
{
	DownloadPlaylist(upload);
	delete upload;
}

ns_error_t CloudThread::CloudClient_DownloadPlaylist(nx_uri_t destination, nx_string_t uuid, int internal_id, cb_cloud_upload *callback)
{
	UploadStruct *upload = new (std::nothrow) UploadStruct;
	if (!upload)
		return NErr_OutOfMemory;

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
	{
		delete upload;
		return NErr_OutOfMemory;
	}

	upload->filename = NXURIRetain(destination);
	upload->destination_device = NXStringRetain(uuid);
	// TODO check how best to handle this all, etc
	upload->internal_id = internal_id;
	upload->callback = callback;
	if (callback)
		callback->Retain();

	apc->func = APC_DownloadPlaylist;
	apc->param1 = this;
	apc->param2 = upload;
	thread_loop.Schedule(apc);
	return NErr_Success;
}

void CloudThread::Parse_Playlist_Snapshot(const JSON::Value *root)
{
	const JSON::Value *playlists_array;
	root->FindNextKey(0, "results", &playlists_array);

	const JSON::Value *playlist;
	size_t i = 0;
	Vector<int, 32, 2> ids;

	while (playlists_array->EnumerateValues(i++, &playlist) == NErr_Success)
	{
		ReferenceCountedNXString uuid, name;
		int64_t id = 0, entries = 0, lastupd = 0, created = 0, duration = 0, priorupd = 0;

		const JSON::Value *value;
		if (playlist->FindNextKey(0, "id", &value) == NErr_Success)
			value->GetInteger(&id);
		if (playlist->FindNextKey(0, "uuid", &value) == NErr_Success)
			value->GetString(&uuid);
		if (playlist->FindNextKey(0, "name", &value) == NErr_Success)
			value->GetString(&name);
		if (playlist->FindNextKey(0, "duration", &value) == NErr_Success)
			value->GetInteger(&duration);
		if (playlist->FindNextKey(0, "entries", &value) == NErr_Success)
			value->GetInteger(&entries);
		if (playlist->FindNextKey(0, "lastupd", &value) == NErr_Success)
			value->GetInteger(&lastupd);
		if (playlist->FindNextKey(0, "created", &value) == NErr_Success)
			value->GetInteger(&created);

		int mode = 0;
		db_connection->Playlists_GetLastUpdate(uuid, &priorupd);
		db_connection->Playlists_AddUpdate(uuid, name, duration*1.0, entries, lastupd, created, ifc_clouddb::PLAYLIST_DONE, &mode);

		// makes sure we've got the playlist_id correct...
		int64_t playlist_id = 0;
		db_connection->Playlists_GetID(uuid, &playlist_id);
		if (id > 0 && id != playlist_id) db_connection->Playlists_SetID(uuid, id);
		PlaylistStruct *the_playlist = new (std::nothrow) PlaylistStruct;
		the_playlist->playlist_id = id;
		the_playlist->name = name;
		the_playlist->uuid = uuid;
		the_playlist->duration = duration*1.0;
		the_playlist->entries = entries;
		the_playlist->lastupdated = lastupd;
		the_playlist->priorupdate = priorupd;

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			// indicate a multi-action update
			itr->OnPlaylistAddUpdate(this, db_connection, mode, the_playlist);
		}
		delete the_playlist;
	}

	// once we're done, db should be updated so we send a done which
	// allows the client to clear up anything missed (the shouldn't
	// happen but is more for helping switching between dev & prod)
	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnPlaylistsDone(this, db_connection);
	}
}

void CloudThread::Parse_Playlist_Add_Update(const JSON::Value *playlist)
{
	ReferenceCountedNXString uuid, name;
	int64_t id = 0, entries = 0, lastupd = 0, created = 0, duration = 0, priorupd = 0;

	const JSON::Value *value;
	playlist->FindNextKey(0, "id", &value);
	value->GetInteger(&id);
	value = 0;
	if (playlist->FindNextKey(0, "uuid", &value) == NErr_Success)
		value->GetString(&uuid);
	if (playlist->FindNextKey(0, "name", &value) == NErr_Success)
		value->GetString(&name);
	if (playlist->FindNextKey(0, "duration", &value) == NErr_Success)
		value->GetInteger(&duration);
	if (playlist->FindNextKey(0, "entries", &value) == NErr_Success)
		value->GetInteger(&entries);
	if (playlist->FindNextKey(0, "lastupd", &value) == NErr_Success)
		value->GetInteger(&lastupd);
	if (playlist->FindNextKey(0, "created", &value) == NErr_Success)
		value->GetInteger(&created);

	int mode = 0;
	db_connection->Playlists_GetLastUpdate(uuid, &priorupd);
	int updated = db_connection->Playlists_AddUpdate(uuid, name, duration*1.0, entries, lastupd, created, 0, &mode);

	// makes sure we've got the playlist_id correct...
	int64_t playlist_id = 0;
	db_connection->Playlists_GetID(uuid, &playlist_id);
	if (id > 0 && id != playlist_id) db_connection->Playlists_SetID(uuid, id);
	PlaylistStruct *the_playlist = new (std::nothrow) PlaylistStruct;
	the_playlist->playlist_id = id;
	the_playlist->name = name;
	the_playlist->uuid = uuid;
	the_playlist->duration = duration*1.0;
	the_playlist->entries = entries;
	the_playlist->lastupdated = lastupd;
	the_playlist->priorupdate = priorupd;
	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		// indicate a single-action update
		itr->OnPlaylistAddUpdate(this, db_connection, (mode | 2), the_playlist);
	}
	delete the_playlist;
}

void CloudThread::Parse_Playlist_Remove(const JSON::Value *playlist)
{
	ReferenceCountedNXString uuid;

	const JSON::Value *value = 0;
	if (playlist->FindNextKey(0, "uuid", &value) == NErr_Success)
		value->GetString(&uuid);

	db_connection->Playlists_Removed(uuid);

	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnPlaylistRemove(this, db_connection, uuid);
	}
}