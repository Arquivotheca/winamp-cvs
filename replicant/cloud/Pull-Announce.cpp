#include "Pull.h"
#include "main.h"
#include "CloudThread.h"
#include "api.h"
#include "JSON-Tree.h"
#include "CloudDB.h"
#include "Attributes.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include "JSONMetadata.h"
#include "nx/nxsleep.h"

extern CloudAPI cloudApi;

void CloudThread::Parse_Action_Add(const JSON::Value *cmd, const JSON::Value *fields)
{
	// build and ifc_metadata interface and add via Media_Add
	ReferenceCountedNXString filename, device_string;
	ReferenceCountedNXURI fileuri;
	int device_id;

	const JSON::Value *value;

	if (cmd->FindNextKey(0, "dev", &value, 0) == NErr_Success && value->GetString(&device_string) == NErr_Success)
	{
		if (db_connection->Devices_Add(device_string, 0, 0, &device_id) == NErr_Success)
		{
			// broadcast arrival of new device
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnDeviceAdded(this, device_string, device_id, 0);
			}
		}
	}

	if (fields->FindNextKey(0, "filepath", &value, 0) != NErr_Success || value->GetString(&filename) != NErr_Success)
		return ;

	NXURICreateWithNXString(&fileuri, filename);

	int64_t cloud_id;
	int internal_id;
	if (fields->FindNextKey(0, "id", &value, 0) == NErr_Success && value->GetInteger(&cloud_id) == NErr_Success)
	{
		/* we're not going to reference count this.  
		If we ever change Media_Update to keep the metadata object past the lifetime of the function, we'll need to "new" it instead */
		JSONMetadata update_metadata(cmd, fields);
		int dirty_flags = ifc_clouddb::DIRTY_NONE;
		ReferenceCountedNXString device_token;
		if (update_metadata.GetField(MetadataKey_CloudDevice, 0, &device_token) == NErr_Success && !NXStringCompare(device_token, attributes.device_token, nx_compare_default))
			dirty_flags = ifc_clouddb::DIRTY_REMOTE;
		if (db_connection->Media_Add(fileuri, &update_metadata, dirty_flags, &internal_id) == NErr_Success)
		{
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnID(this, fileuri, internal_id);  
			}
		}
	}
}

void CloudThread::Parse_Action_Delete(const JSON::Value *cmd, const JSON::Value *fields)
{
	const JSON::Value *value;
																										
	int64_t cloud_id;
	if (fields->FindNextKey(0, "id", &value, 0) == NErr_Success && value->GetInteger(&cloud_id) == NErr_Success)
	{
		int internal_id;
		if (db_connection->IDMap_Find(cloud_id, &internal_id) != NErr_Success) // find its associated internal_id
			return;

		// TODO: benski> delete local file. might want to set as ignore=## and let a background thread handle the delete
		db_connection->IDMap_Removed(internal_id); // TODO: benski> actually might want to set as ignore=2
	}
}

void CloudThread::Parse_Action_Remove(const JSON::Value *cmd, const JSON::Value *fields)
{
	const JSON::Value *value;

	// depending on the client, we can get differing 'remove' actions
	// via 'metahash' or using the 'cloudid' for a file so check and
	// then process accordingly (re-using the delete method as needed)

	// TODO: benski> after March release, ignore metahash-based removes.  we should be removing by ID
	ReferenceCountedNXString metahash;
	if (fields->FindNextKey(0, "metahash", &value, 0) == NErr_Success) 
	{
		value->GetString(&metahash);

		size_t num_ids = 0;
		int64_t *out_ids = 0;
		db_connection->IDMap_Get_IDs_From_MetaHash(metahash, &out_ids, 0, &num_ids);
		for (size_t i = 0; i < num_ids; i++)
		{
			db_connection->IDMap_Removed(out_ids[i]);  // TODO: benski> actually might want to set as ignore=2
		}
		if (out_ids)
			free(out_ids);
	}
	else
	{
		Parse_Action_Delete(cmd, fields);
	}
}

void CloudThread::Parse_Action_Played(const JSON::Value *cmd, const JSON::Value *fields)
{
	const JSON::Value *value = 0;

	int64_t cloud_id;
	fields->FindNextKey(0, "id", &value, 0);
	if (!value || value->GetInteger(&cloud_id) != NErr_Success)
		return;

	int internal_id;
	if (db_connection->IDMap_Find(cloud_id, &internal_id) != NErr_Success) // find its associated internal_id
		return;

	// get the cloud id and then use that to get the mediahash so we can then update
	// the lastplay and playcount for all instances of the media across the devices
	const JSON::Value *lastplay;
	if (fields->FindNextKey(0, "lastplay", &lastplay) == NErr_Success && lastplay->data_type == JSON::DATA_INTEGER)
	{
		// if we have a '+' then it is a cumulative play count change
		const JSON::Value *plus_value = 0;
		int64_t played = 1, played_inc = 1;
		fields->FindNextKey(0, "+", &plus_value, 0);
		if (!plus_value || plus_value->GetInteger(&played) != NErr_Success)
		{
			played_inc = 1;
		}

		if (db_connection->CloudDB_IDMap_GetPlayedProperties(internal_id, &played, 0) == NErr_Success && played > 0)
		{
			played += played_inc;
		}
		else
		{
			played = played_inc;
		}

		int64_t lastplayed = 0;
		lastplay->GetInteger(&lastplayed);
		db_connection->IDMap_SetPlayedProperties(internal_id, played, lastplayed);

		// TODO need to notify the client about this so any UI updates can be reflected and
		//		also need to determine how we apply this. i.e. all devices or for the origin
	}
}

void CloudThread::Parse_Action(const JSON::Value *cmd, const JSON::Value_Map *action)
{
	nx_string_t key_string;
	const JSON::Value_Key *key = 0;

	if (action->EnumerateValues(0, (const JSON::Value **)&key) == NErr_Success && key->GetKey_NoRetain(&key_string) == NErr_Success)
	{
		const JSON::Value *fields;
		if (key->GetValue(&fields) == NErr_Success)
		{
			if (!NXStringKeywordCompareWithCString(key_string, ">"))
			{
				Parse_Action_Played(cmd, fields);
			}
			else if (!NXStringKeywordCompareWithCString(key_string, "announce"))
			{
				Parse_Action_Add(cmd, fields);
			}
			else if (!NXStringKeywordCompareWithCString(key_string, "delete"))
			{
				Parse_Action_Delete(cmd, fields);
			}
			else if (!NXStringKeywordCompareWithCString(key_string, "remove"))
			{
				Parse_Action_Remove(cmd, fields);
			}
			else if (!NXStringKeywordCompareWithCString(key_string, "update"))
			{
				Parse_Action_Update(cmd, fields);
			}
		}
	}
}

int CloudThread::Parse_Pull(const JSON::Value *root)
{
	bool writer_blocks=true;
	playlist_refresh=false;

	if (db_connection->WriterBlocks() == NErr_False)
		writer_blocks=false;

	//bool saw_own_update=false;
	if (!root || root->data_type != JSON::DATA_MAP)
		return NErr_Empty;

	const JSON::Value *fullupdate;
	if (root->FindNextKey(0, "fullupdate", &fullupdate) != NErr_Success)
		return NErr_NotFound;

	bool in_transaction=false;
	size_t n=0;
	if (!writer_blocks)
	{
		db_connection->BeginTransaction();
		in_transaction=true;
	}
	const JSON::Value *push = 0;
	while (fullupdate->EnumerateValues(n++, &push) == NErr_Success)
	{
		/*ReferenceCountedNXString device_token;
		const JSON::Value *dev;
		if (push->FindNextKey(0, "dev", &dev) == NErr_Success && dev->GetString(&device_token) == NErr_Success && !NXStringKeywordCompare(device_token, attributes.device_token))
			saw_own_update=true;*/

		const JSON::Value *command=0;
		if (push->FindNextKey(0, "cmd", &command) == NErr_Success && command->data_type == JSON::DATA_MAP)
		{
			const JSON::Value *revision_value=0;
			if (command->FindNextKey(0, "rev", &revision_value) == NErr_Success && revision_value->data_type == JSON::DATA_INTEGER)
			{
				int64_t new_revision;
				if (revision_value->GetInteger(&new_revision) == NErr_Success)
				{
					ReferenceCountedNXString new_revision_id;
					if (command->FindNextKey(0, "rid-new", &revision_value) == NErr_Success && revision_value->data_type == JSON::DATA_STRING)
						revision_value->GetString(&new_revision_id);

					if (!in_transaction)
					{
						db_connection->BeginTransaction();
						in_transaction=true;
					}
					const JSON::Value *actions=0;
					if (command->FindNextKey(0, "acts", &actions) == NErr_Success && actions->data_type == JSON::DATA_ARRAY)
					{
						size_t action_n=0;
						const JSON::Value *action;
						while (actions->EnumerateValues(action_n++, &action) == NErr_Success)
						{
							if (action->data_type == JSON::DATA_MAP)
							{
								Parse_Action(command, (const JSON::Value_Map *)action);
							}
						}
					}
					else if (command->FindNextKey(0, "playlist", &actions) == NErr_Success && actions->data_type == JSON::DATA_MAP)
					{
						// due to the nature of playlist updates, if we've not done a complete first pull then
						// we're better to skip all of the actions and do a metadata-snapshot-playlists later.
						const JSON::Value *type_value=0;
						if (command->FindNextKey(0, "type", &type_value) == NErr_Success && type_value->data_type == JSON::DATA_STRING)
						{
							if (first_pull)
							{
								ReferenceCountedNXString type_string;
								if (type_value->GetString(&type_string) == NErr_Success)
								{
									if (!NXStringKeywordCompareWithCString(type_string, "metadata-playlist-new") ||
										!NXStringKeywordCompareWithCString(type_string, "metadata-playlist-set"))
									{
										Parse_Playlist_Add_Update(actions);
									}
									else if (!NXStringKeywordCompareWithCString(type_string, "metadata-playlist-remove"))
									{
										Parse_Playlist_Remove(actions);
									}
								}
							}
							else
							{
								playlist_refresh=true;
							}
						}
					}

					db_connection->Info_SetRevision(new_revision+1);
					db_connection->Info_SetRevisionID(new_revision_id);
					if (writer_blocks && ((n % 2) == 0))
					{
						db_connection->Commit();
						in_transaction=false;
						NXSleepYield();
					}
				}
			}
		}
	}


	if (in_transaction)
	{
		db_connection->Commit();
		//in_transaction=false;
	}

	return NErr_Success;
}
