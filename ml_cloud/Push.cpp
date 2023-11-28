#include "main.h"
#include "api.h"
#include "../Winamp/wa_ipc.h"
#include "nswasabi/ReferenceCounted.h"
#include "BackgroundTasks.h"
#include "ItemRecordMetadata.h"
#include <shlwapi.h>
#include "resource.h"

static nx_string_t ndestring_get_string(wchar_t *str)
{
	if (!str)
		return 0;
	nx_string_t self = (nx_string_t)((uint8_t *)str - sizeof(size_t) - sizeof(size_t));
	return self;
}

ns_error_t AddFileToLibrary(ifc_clouddb *db_connection, Attributes &attributes, itemRecordW *record)
{
	nx_string_t nx_filename = ndestring_get_string(record->filename);
	ReferenceCountedNXURI nx_uri;
	
	int ret = NXURICreateWithNXString(&nx_uri, nx_filename);
	if (ret != NErr_Success)
		return ret;

	int internal_id = 0;
	ItemRecordMetadata metadata(record);
	return db_connection->Media_Add(nx_uri, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL, &internal_id);
}

ns_error_t UpdateFile(ifc_clouddb *db_connection, Attributes &attributes, int internal_id, const wchar_t *filename)
{
	itemRecordW *record = AGAVE_API_MLDB->GetFile(filename);
	if (!record)
		return NErr_Empty;

	ItemRecordMetadata metadata(record);
	int ret = db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);
	AGAVE_API_MLDB->FreeRecord(record);
	return ret;
}

bool AddFilesToLibrary(ifc_clouddb *db_connection, Attributes &attributes, void *user_data)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->IsKilled())
	{
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ABORT_ADD_NEW_FILES));
		return 0;
	}

	// first scan will look for recent additions up to the last time we had a scan
	// depending on the mode, we either just look for new files or all files
	// TODO if / when OGG and WAV are supported then alter these queries as needed
	wchar_t query[128] = {L"(!(filename ends \".ogg\"))"};
	time_t last_time = Config_GetLastScan(), raw_last_time = last_time;

	// check if we need to do a removed / missing files scan as well as a new scan
	bool full_check = (((int)last_time == -1) || ((int)last_time == -2));
	if (last_time < 0) last_time = 0;
	time_t time_diff = (!last_time ? 0 : abs((int)(time(0) - last_time)));

	// no need to re-run if there's no time difference
	if (last_time && !time_diff)
		return 1;

	if (time_diff > 0 && !full_check)
		swprintf(query, L"(dateadded > [%d s before now]) AND (!(filename ends \".ogg\"))", time_diff);

	if (!full_check) DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_CHECKING_FOR_NEW_FILES));
	StatusWindow_Message(IDS_STATUS_SCANNING);

	itemRecordListW *records = AGAVE_API_MLDB->Query(query);
	/*/itemRecordListW *records = AGAVE_API_MLDB->QueryLimit(query, 100);/**/
	int num_records = 0;
	if (records)
	{
		wchar_t err_msg[1024];
		swprintf(err_msg, WASABI_API_LNGSTRINGW(IDS_SCANNING_DETECTED_X_FILES), records->Size);
		DebugConsole_SetStatus(err_msg);

		db_connection->BeginTransaction();
		num_records = records->Size;

		for (int i = 0; i < records->Size; i++)
		{
			if (background->IsKilled())
			{
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ABORT_ADD_NEW_FILES));
				DebugConsole_ShowProgess(0);
				db_connection->Commit();
				return 0;
			}

			itemRecordW *record = &records->Items[i];

			if (AddFileToLibrary(db_connection, attributes, record) == NErr_NoAction)
			{
				num_records--;
			}
			else
			{
				wchar_t err_msg[1024];
				swprintf(err_msg, WASABI_API_LNGSTRINGW(IDS_ADDING_X), record->filename);
				DebugConsole_SetStatus(err_msg);
			}

			DebugConsole_UpdateProgress(i, records->Size);
		}

		if (num_records > 0)
		{
			wchar_t err_msg[1024];
			swprintf(err_msg, WASABI_API_LNGSTRINGW(IDS_FOUND_X_FILES_TO_PROCESS), num_records);
			DebugConsole_SetStatus(err_msg);
		}
		else
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_NO_NEW_FILES_FOUND));
	}
	db_connection->Commit();
	if (full_check)
	{
		db_connection->BeginTransaction();
		// look at what is already known and clean up as applicable
		// note: this should only happen when using the rescan mode
		int *ids = 0;
		size_t num_ids = 0, removed = 0;
		db_connection->Media_GetIDs(local_device_id, &ids, &num_ids);

		bool full_remove = ((int)last_time == -1);
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW((full_remove ? IDS_CHECKING_FOR_MISSING_REMOVED_FILES : IDS_CHECKING_FOR_MISSING_FILES_ONLY)));
		for (size_t i = 0; i < num_ids; i++)
		{
			ReferenceCountedNXURI filepath;
			if (db_connection->IDMap_Get_Filepath(ids[i], &filepath) == NErr_Success)
			{
				// first check is for anything which is no longer locally present
				if (!PathFileExistsW(filepath->string))
				{
					removed++;
					db_connection->IDMap_Remove(ids[i]);
				}
				// and then see if it still exists in the local library or not...
				else
				{
					if (full_remove)
					{
						bool found = false;
						if (records)
						{
							for (int it = 0; it < records->Size; it++)
							{
								itemRecordW *record = &records->Items[it];
								if (!wcsicmp(filepath->string, record->filename))
								{
									found = true;
									break;
								}
							}
						}

						if (!found)
						{
							removed++;
							db_connection->IDMap_Remove(ids[i]);
						}
					}
				}
			}
			DebugConsole_UpdateProgress(i, num_ids);
		}

		if (ids) free(ids);

		if (removed > 0)
		{
			wchar_t err_msg[1024];
			swprintf(err_msg, WASABI_API_LNGSTRINGW(IDS_FOUND_X_TO_REMOVE), removed);
			DebugConsole_SetStatus(err_msg);
		}
		db_connection->Commit();
	}

	if (records)
	{
		AGAVE_API_MLDB->FreeRecordList(records);
		//if (!num_records) DebugConsole_ShowProgess(0);

		// if we get here then we completed a scan of the files and
		// so we set it to prevent additional scans for those files
		Config_SetLastScan(time(0));
		return (num_records > 0);
	}
	else
	{
		// if we get here then we completed a scan of the files and
		// so we set it to prevent additional scans for those files
		Config_SetLastScan(time(0));
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_NO_NEW_FILES_FOUND));

		// if we're showing in scanning state message then abort this
		// TODO need to work out why this gets double-called once run
		if (last_string_id != IDS_STATUS_SCANNING && last_string_id != IDS_STATUS_CALCULATING_MEDIA_HASH)
			StatusWindow_Message(0);
	}

	return 0;
}