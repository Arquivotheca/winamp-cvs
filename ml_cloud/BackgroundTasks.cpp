#include "api.h"
#include "main.h"
#include "../ml_pmp/pmp.h"
#include "../nu/threadpool/TimerHandle.h"
#include "BackgroundTasks.h"
#include <api/service/waServiceFactory.h>
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include "../replicant/cloud/ifc_clouddb.h"
#include "nx/nxsleep.h"
#include "resource.h"
#include "../Wasabi2/main.h"
#include <shlwapi.h>
#include <strsafe.h>

extern Cloud_Background cloud_background;

Cloud_Background::Cloud_Background()
{
	background_thread=0;
	db_connection=0;
	external_db_connection=0;
	ui_db_connection=0;
	killswitch=0;
	first_pull=0;
	load_attempts=0;
	mediahash_ids=0;
	login_attempts=0;
	num_mediahash_ids=0;
	mediahash_itr=0;
	last_menu_filepath[0]=0;
	last_menu_playlist=0;
	albumart_ids=0;
	num_albumart_ids=0;
	albumart_itr=0;
}

Cloud_Background::~Cloud_Background()
{
	this->Release();
}

void Cloud_Background::Kill()
{
	killswitch=1;
	AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_Run, this, 0, 0);
}

void Cloud_Background::Rescan(int missing_only)
{
	// trigger a local library rescan
	Config_SetLastScan((missing_only != -1 ? -1 - missing_only : 0));
	add_timer.Cancel();
	add_timer.Wait(1000);
}

void Cloud_Background::CredentialsChanged(int force)
{
	// only fire off if we're showing as logged out otherwise causes
	// some quirks when catching an already logged (dupe first pull)
	if (force || REPLICANT_API_CLOUD && REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
	{
		first_pull = 0;
		if (force != 2)
		{
			login_attempts = 0;
			login_timer.Cancel();
			login_timer.Poll(250);
			if (cloud_client)
				cloud_client->Flush();
		}

		// sends a message to ml_pmp to kill the cloud device nodes...
		HWND ml_pmp_window = FindWindow(L"ml_pmp_window", NULL);
		if (IsWindow(ml_pmp_window))
		{
			PostMessage(ml_pmp_window, WM_USER+5, 0, 0);
		}

		SetSignInNodeText(WASABI_API_LNGSTRINGW(IDS_SIGN_IN));
	}
}

void Cloud_Background::OnReset()
{
	// TODO: benski> stop cloudclient

	// if ok then make sure revision is reset
	// as we otherwise get "rev ahead" errors
	db_connection->BeginTransaction();
	db_connection->Reset_All();
	db_connection->Commit();

	Config_SetLastScan(0);

	// TODO: benski> start cloudclient

	if (mediahash_ids)
		free(mediahash_ids);
	mediahash_ids = 0;

	// wait a bit before we start to re-scan so everything settles
	add_timer.Cancel();
	add_timer.Wait(1000);
}

void Cloud_Background::OnFirstPull(int clean, bool forced)
{
	// if we've done a first run / complete re-pull then do a compact of the db
	if (clean == 1) db_connection->Compact();

	// setup recurring events to keep us reasonably in sync from continued usage
	first_pull=1;
	add_timer.Cancel();
	add_timer.Wait(1000);

	DebugConsole_SetStatus(WASABI_API_LNGSTRINGW((!forced ? IDS_FIRST_PULL_COMPLETED : IDS_FORCED_PULL_COMPLETED)));
	DebugConsole_ShowProgess(0);
}

int Cloud_Background::Background_Run(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;

	if (background->killswitch)
	{
		AGAVE_API_THREADPOOL->RemoveHandle(background->background_thread, background->add_timer);
		AGAVE_API_THREADPOOL->RemoveHandle(background->background_thread, background->login_timer);
		AGAVE_API_THREADPOOL->RemoveHandle(background->background_thread, background->media_timer);
		return 1;
	}
	else
	{
		if (!background->Internal_Initialize())
		{
			// as loading order can change / be delayed, attempt to try a few times before failing
			if (background->load_attempts < 5)
			{
				background->load_attempts++;
				Wasabi2::NXSleep(1000);
				AGAVE_API_THREADPOOL->RunFunction(background->background_thread, background->Background_Run, background, 0, 0);
			}
			return 1;
		}
	}

	return 0;
}

int Cloud_Background::Background_Add(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	wchar_t err_msg[1024];
	itemRecordW *record = (itemRecordW *)id;
	background->db_connection->BeginTransaction();
	ns_error_t ret = AddFileToLibrary(background->db_connection, background->attributes, record);
	background->db_connection->Commit();
	if (ret == NErr_Success)
	{
		StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_ADDING_X), record->filename);
	}
	else if (ret == NErr_Success)
	{
		StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_ALREADY_ADDED), record->filename);
	}
	else
	{
		StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_ADD_FAILED), ret, record->filename);
	}
	DebugConsole_SetStatus(err_msg);
	AGAVE_API_MLDB->FreeRecord(record);

	return 1;
}

int Cloud_Background::Background_Remove(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	nx_uri_t filename = (nx_uri_t)id;

	int internal_id = 0, is_ignored = 0;
	if (background->db_connection->Media_FindByFilename(filename, local_device_id, &internal_id, &is_ignored) == NErr_Success)
	{
		background->db_connection->IDMap_Remove(internal_id);
	}

#ifdef _DEBUG
	if (cloud_client)
		cloud_client->Flush();
#endif

	NXURIRelease(filename);
	return 1;
}

int Cloud_Background::Background_FilePlayed(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	nx_uri_t filename = (nx_uri_t)id;

	int internal_id = 0, is_ignored = 0;
	background->db_connection->BeginTransaction();
	if (background->db_connection->Media_FindByFilename(filename, local_device_id, &internal_id, &is_ignored) == NErr_Success)
	{
		/* TODO: use Media_Update() and define some sort of "playcount_increment" metadata key */
		int64_t playcount=0, last_updated=0, filetime=0, filesize=0, bitrate=0;
		double duration=0;
		background->db_connection->IDMap_GetPlayedProperties(internal_id, &playcount, 0); // so we can call SetProperties with all the values
		background->db_connection->IDMap_SetPlayedProperties(internal_id, playcount+1, _time64(0)); // to set the playcount and last played time
		background->db_connection->IDMap_AddDirty(internal_id, ifc_clouddb::DIRTY_LOCAL); // mark the playcount as dirty 
	}
	background->db_connection->Commit();

#ifdef _DEBUG
	if (cloud_client)
		cloud_client->Flush();
#endif

	NXURIRelease(filename);
	return 1;
}

int Cloud_Background::Background_Update(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	nx_uri_t filename = (nx_uri_t)id;

	wchar_t err_msg[1024];
	int internal_id = 0, is_ignored = 0;
	if (background->db_connection->Media_FindByFilename(filename, local_device_id, &internal_id, &is_ignored) == NErr_Success && is_ignored==0)
	{
		background->db_connection->BeginTransaction();
		UpdateFile(background->db_connection, background->attributes, internal_id, filename->string);
		background->db_connection->Commit();

		StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_METADATA_CHANGED), filename->string);
		DebugConsole_SetStatus(err_msg);
	}
	else
	{
		StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_IGNORE_METADATA_CHANGE), filename->string);
		DebugConsole_SetStatus(err_msg);
	}

#ifdef _DEBUG
	if (cloud_client)
		cloud_client->Flush();
#endif

	NXURIRelease(filename);
	return 1;
}

#define RegisterAttribute(x) db_connection->Attribute_Add(#x, &attributes. ## x)
bool Cloud_Background::Internal_Initialize()
{
	if (!REPLICANT_API_CLOUD)
	{
		// as loading order can change / be delayed, attempt to try a few times before failing
		if (this->load_attempts == 5)
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_CRITICAL_ERROR));
		}
		return false;
	}

	DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_INIT_DB));
	REPLICANT_API_CLOUD->CreateDatabaseConnection(&db_connection, local_device_token);
	REPLICANT_API_CLOUD->CreateDatabaseConnection(&external_db_connection, local_device_token);

	attributes.device_token = NXStringRetain(local_device_token);

	db_connection->BeginTransaction();
	db_connection->Devices_Find(local_device_token, &attributes.device_id, 0);
	RegisterAttribute(artist);
	RegisterAttribute(album);
	RegisterAttribute(trackno);
	RegisterAttribute(albumartist);
	RegisterAttribute(bpm);
	RegisterAttribute(category);
	RegisterAttribute(comment);
	RegisterAttribute(composer);
	RegisterAttribute(director);
	RegisterAttribute(disc);
	RegisterAttribute(discs);
	RegisterAttribute(genre);
	RegisterAttribute(producer);
	RegisterAttribute(publisher);
	RegisterAttribute(tracks);
	RegisterAttribute(year);
	RegisterAttribute(albumgain);
	RegisterAttribute(trackgain);
	RegisterAttribute(rating);
	RegisterAttribute(type);
	RegisterAttribute(lossless);

	db_connection->Commit();

	AGAVE_API_THREADPOOL->AddHandle(background_thread, add_timer, Background_AddFiles, this, 0, 0);
	AGAVE_API_THREADPOOL->AddHandle(background_thread, login_timer, Background_CheckLogin, this, 0, 0);
	AGAVE_API_THREADPOOL->AddHandle(background_thread, media_timer, Background_MediaHash, this, 0, 0);

	WASABI_API_SYSCB->syscb_registerCallback(this);
	login_timer.Poll(1000);
	return true;
}
#undef RegisterAttribute

int Cloud_Background::Background_AddFiles(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	if (!AddFilesToLibrary(background->db_connection, background->attributes, user_data))
	{
		AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_MediaHash, user_data, 0, 0);
	}
	else
	{
		AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_MediaHash, user_data, 0, 0);
		AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AddFiles, user_data, 0, 0);
	}
	background->add_timer.Cancel();
	return 1;
}

int Cloud_Background::Background_MediaHash(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	// TODO: let the thread process other things as we go.  
	// probably want to keep internal_ids and num_ids as a member variable.  and the counter
	if (background->mediahash_ids && background->mediahash_itr < background->num_mediahash_ids)
	{
		int internal_id = background->mediahash_ids[background->mediahash_itr++];
		DebugConsole_UpdateProgress(background->mediahash_itr, background->num_mediahash_ids);
		ReferenceCountedNXString media_hash;
		ReferenceCountedNXURI filename_uri;
		if (background->db_connection->IDMap_Get_Filepath(internal_id, &filename_uri) == NErr_Success)
		{
			wchar_t err_msg[1024];
			int compute_ret = NErr_Success;
			if (!PathFileExistsW(filename_uri->string))
			{
				compute_ret = NErr_FileNotFound;
			}
			else
			{
				StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALCULATING_MEDIA_HASH), filename_uri->string);
				DebugConsole_SetStatus(err_msg);
			}

			if (compute_ret == NErr_Success)
				compute_ret = ComputeMediaHash(filename_uri->string, &media_hash);
			if (compute_ret == NErr_Success)
			{
				MediaHashMetadata metadata(media_hash);
				background->db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);

#ifdef _DEBUG
				// do an announce every 32 tracks.  we can tune this
				if ((background->mediahash_itr & 31) == 0)
				{
					//DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_METADATA));
					if (cloud_client)
						cloud_client->Flush();
				}
#endif

				AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_MediaHash, user_data, 0, 0);
				return 1;		
			}
			else
			{
				if (compute_ret == NErr_FileNotFound)
					StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALUCATING_MEDIA_HASH_SKIPPED), filename_uri->string);
				else
					StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALUCATING_MEDIA_HASH_FAILED), compute_ret, filename_uri->string);
				DebugConsole_SetStatus(err_msg);

				// flag file to be ignored from further re-processing attempts
				if (background->db_connection->IDMap_SetIgnore(internal_id) != NErr_Success)
					DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_SET_IGNORE_FAILED));

				DebugConsole_UpdateIgnoredFiles();
			}
		}

		AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_MediaHash, user_data, 0, 0);
	}
	else
	{
#ifdef _DEBUG
		if (cloud_client)
			cloud_client->Flush();
#endif
		if (background->mediahash_ids)
		{
			if (background->num_mediahash_ids)
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_METADATA));
			if (background->mediahash_ids)
			{
				free(background->mediahash_ids);
			}
			background->mediahash_ids = 0;
			background->num_mediahash_ids = 0;
		}

		if (background->killswitch)
		{
			DebugConsole_ShowProgess(0);
			return 1;
		}

		if (background->db_connection->IDMap_Get_MediaHash_Null(&background->mediahash_ids, &background->num_mediahash_ids) == NErr_Success)
		{
			background->mediahash_itr = 0;
			// we only need to do the background add checking on loading so once done
			// we cancel though this will allow us to do one-off add checks later on
			background->add_timer.Cancel();

			if (background->num_mediahash_ids > 0)
			{
				wchar_t err_msg[1024];
				StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALCULATING_MEDIA_HASH_FOR_FILES), background->num_mediahash_ids);
				DebugConsole_SetStatus(err_msg);

				AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_MediaHash, user_data, 0, 0);
			}
			DebugConsole_ShowProgess(background->num_mediahash_ids);
			StatusWindow_Message((background->num_mediahash_ids > 0 ? IDS_STATUS_CALCULATING_MEDIA_HASH : 0));
			// TODO renable when sorted out (not ready for the prime-time currently)
			#ifdef DEBUG
			AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AlbumArt, user_data, 0, 0);
			#endif
		}
		else
		{
			DebugConsole_ShowProgess(0);
			// TODO renable when sorted out (not ready for the prime-time currently)
			#ifdef DEBUG
			AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AlbumArt, user_data, 0, 0);
			#endif
		}
	}

	return 1;
}

int Cloud_Background::Background_AlbumArt(HANDLE handle, void *user_data, intptr_t id)
{
#if 0
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	// TODO renable when sorted out (not ready for the prime-time currently)
	#ifndef DEBUG
	return 1;
	#endif

	// TODO: let the thread process other things as we go.  
	// probably want to keep internal_ids and num_ids as a member variable.  and the counter
	if (background->albumart_ids && background->albumart_itr < background->num_albumart_ids)
	{
		int internal_id = background->albumart_ids[background->albumart_itr++];
		//DebugConsole_UpdateProgress(background->albumart_itr, background->num_albumart_ids);
		ReferenceCountedNXString arthash;
		ReferenceCountedNXURI filename_uri;
		if (background->db_connection->IDMap_Get_Filepath(internal_id, &filename_uri) == NErr_Success)
		{
			/*wchar_t err_msg[1024];
			StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALCULATING_ALBUMART_HASH), filename_uri->string);
			DebugConsole_SetStatus(err_msg);*/

			// TODO TODO review this - allowed through on NErr_Empty to prevent it spinning
			//			 but not sure what the intention is for handling non-arthash items
			int compute_ret = ComputeArtHash(filename_uri, background->attributes.album, &arthash);
			if (compute_ret == NErr_Success || compute_ret == NErr_Empty)
			{
				ArtHashMetadata metadata(arthash);
				background->db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);

				// do an announce every 32 tracks.  we can tune this
				if ((background->albumart_itr & 31) == 0)
				{
					//DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_METADATA));
					if (cloud_client)
						cloud_client->Flush();
				}

				AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AlbumArt, user_data, 0, 0);
				return 1;		
			}
		}

		AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AlbumArt, user_data, 0, 0);
	}
	else
	{
		if (cloud_client)
			cloud_client->Flush();
		if (background->albumart_ids)
		{
			/*if (background->num_albumart_ids)
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_ALBUMART));*/

			free(background->albumart_ids);
			background->albumart_ids = 0;
			background->num_albumart_ids = 0;
		}

		if (background->killswitch)
		{
			DebugConsole_ShowProgess(0);
			return 1;
		}

		if (background->db_connection->Artwork_GetWork(background->attributes.device_id, background->attributes.album, 0, &background->albumart_ids, &background->num_albumart_ids) == NErr_Success)
		{
			background->albumart_itr = 0;
			// we only need to do the background add checking on loading so once done
			// we cancel though this will allow us to do one-off add checks later on
			background->add_timer.Cancel();

			if (background->num_albumart_ids > 0)
			{
				/*wchar_t err_msg[1024];
				StringCchPrintfW(err_msg, 1024, WASABI_API_LNGSTRINGW(IDS_CALCULATING_MEDIA_HASH_FOR_FILES), background->num_albumart_ids);
				DebugConsole_SetStatus(err_msg);*/

				AGAVE_API_THREADPOOL->RunFunction(background->background_thread, Background_AlbumArt, user_data, 0, 0);
			}
			//DebugConsole_ShowProgess(background->num_albumart_ids);
			StatusWindow_Message((background->num_albumart_ids > 0 ? IDS_STATUS_CALCULATING_ALBUMART_HASH : 0));
		}
		else
		{
			DebugConsole_ShowProgess(0);
		}
	}
#endif
	return 1;
}

bool SetCredentials();
int Cloud_Background::Background_CheckLogin(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud_Background *background = (Cloud_Background *)user_data;
	if (background->killswitch) return 1;

	if (!background->login_attempts)
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_WAITING_LOGIN));
	DebugConsole_ShowProgess(-1);
	StatusWindow_Message(IDS_STATUS_WAITING_LOGIN);
	if (SetCredentials() && cloud_client)
	{
		background->login_timer.Cancel();

		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(!background->first_pull ? IDS_FIRST_PULL : IDS_PULLING));
		if (!background->first_pull)
		{
			StatusWindow_Message(IDS_STATUS_FIRST_PULL);

			// if our node is the avtive one, re-create so we updated
			NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
			nvItem.hItem = NavigationItem_Find(0, L"cloud_sources", TRUE);
			if (MLNavCtrl_GetSelection(plugin.hwndLibraryParent) == nvItem.hItem)
			{
				PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
			}
		}
		DebugConsole_ShowProgess((!background->first_pull ? -1 : 0));

		// when the pull succeeds we'll have our OnFirstPull() called
		// at which point we'll start up the addfiles checking, etc
		cloud_client->Flush((!background->first_pull ? 1 : 0));
		background->login_attempts = 0;
		return 1;
	}
	else
	{
		// change to a 5s delay after the initial 1s delay
		// and then go to 60s delay if stays like for >30s
		background->login_timer.Cancel();
		background->login_timer.Poll((background->login_attempts > 5 ? 60000 : 5000));
		background->login_attempts++;
	}
	return 0;
}

int Cloud_Background::Initialize()
{
	if (!background_thread)
	{
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_STARTING_BG_THREAD));
		background_thread = AGAVE_API_THREADPOOL->ReserveThread(0);
		AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_Run, this, 0, 0);
	}
	return NErr_Success;
}

size_t Cloud_Background::Release()
{
	if (db_connection)
	{
		db_connection->Release();
		db_connection = 0;
	}

	if (external_db_connection)
	{
		external_db_connection->Release();
		external_db_connection = 0;
	}

	if (ui_db_connection)
	{
		ui_db_connection->Release();
		ui_db_connection = 0;
	}

	if (mediahash_ids)
	{
		free(mediahash_ids);
		mediahash_ids = 0;
	}

	if (albumart_ids)
	{
		free(albumart_ids);
		albumart_ids = 0;
	}

	return NErr_Success;
}

void Cloud_Background::OnFileAdded(const wchar_t *filename)
{
	itemRecordW *record = AGAVE_API_MLDB->GetFile(filename);
	if (record)
	{
		AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_Add, this, (intptr_t)record, 0);
		// wait a bit before we start to process the added files
		media_timer.Cancel();
		media_timer.Wait(1000);
	}
}

void Cloud_Background::OnFileUpdated(const wchar_t *filename, bool from_library)
{
	ReferenceCountedNXString nx_temp;
	nx_uri_t nx_filename;
	NXStringCreateWithUTF16(&nx_temp, filename);
	NXURICreateWithNXString(&nx_filename, nx_temp);
	AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_Update, this, (intptr_t)nx_filename, 0);
}

void Cloud_Background::OnFileRemove_Post(const wchar_t *filename)
{
	ReferenceCountedNXString nx_temp;
	nx_uri_t nx_filename;
	NXStringCreateWithUTF16(&nx_temp, filename);
	NXURICreateWithNXString(&nx_filename, nx_temp);
	AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_Remove, this, (intptr_t)nx_filename, 0);
}

void Cloud_Background::OnFilePlayed(const wchar_t *filename, time_t played, int count)
{
	ReferenceCountedNXString nx_temp;
	nx_uri_t nx_filename;
	NXStringCreateWithUTF16(&nx_temp, filename);
	NXURICreateWithNXString(&nx_filename, nx_temp);

	AGAVE_API_THREADPOOL->RunFunction(background_thread, Background_FilePlayed, this, (intptr_t)nx_filename, 0);
}

void Cloud_Background::OnCleared(const wchar_t **filenames, int count)
{
	if (count > 0 && filenames)
	{
		for(int i = 0; i < count; i++)
		{
			OnFileRemove_Post(filenames[i]);
		}
	}
	if (cloud_client)
		cloud_client->Flush();
}

void Cloud_Background::OnGetCloudStatus(const wchar_t *filepath, HMENU *menu)
{
	last_menu_playlist = 0;
	if (!first_pull)
	{
		InsertMenu(*menu, 0, MF_BYPOSITION | MF_GRAYED, CLOUD_SOURCE_MENUS + 1,
				   WASABI_API_LNGSTRINGW(IDS_UNABLE_DETERMINE_CLOUD_SOURCES));
		return;
	}

	if (!ui_db_connection)
	{
		REPLICANT_API_CLOUD->CreateDatabaseConnection(&ui_db_connection, local_device_token);
	}

	if (ui_db_connection)
	{
		if (*menu != (HMENU)0x666)
		{
			nx_string_t *devices = 0;
			size_t num_devices = 0;
			if (ui_db_connection->Devices_GetIDs(&devices, &num_devices) == NErr_Success)
			{
				for (size_t i = 0, j = 0; i < num_devices; i++)
				{
					DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct;
					int device_id = 0;
					if (ui_db_connection->Devices_Find(devices[i], &device_id, device_info) == NErr_Success && device_id > 0)
					{
						if (NXStringKeywordCompareWithCString(devices[i], OLE_WEB_CLIENT))
						{
							// TODO when we've a reliable way to know if a device supports uploads then change this hard-coding
							// TODO when further device <-> device support happens then this can be changed to allow the menu items, etc
							bool supports_uploads = (!NXStringKeywordCompareWithCString(devices[i], HSS_CLIENT) ||
													 !NXStringKeywordCompareWithCString(devices[i], DROPBOX_CLIENT));
							InsertMenu(*menu, (device_id == local_device_id ? 0 : ++j),
									   MF_BYPOSITION | (!supports_uploads && (device_id != local_device_id) ? MF_GRAYED : 0),
									   CLOUD_SOURCE_MENUS + device_id,
									   (device_id == local_device_id ? WASABI_API_LNGSTRINGW(IDS_LOCAL_LIBRARY) : device_info->name->string));
						}
					}
					NXStringRelease(devices[i]);
					delete device_info;
				}

				// see if we have a local track with a matching media hash and use that
				// otherwise we'll have to go the streaming attempt for playing the file
				lstrcpyn(last_menu_filepath, filepath, MAX_PATH);
				ReferenceCountedNXURI last_fileuri;
				NXURICreateWithUTF16(&last_fileuri, filepath);

				ReferenceCountedNXString media_hash;
				int internal_id = 0, is_ignored = 0;
				if (ui_db_connection->Media_FindByFilename(last_fileuri, local_device_id, &internal_id, &is_ignored) == NErr_Success)
				{
					ui_db_connection->IDMap_GetMediaHash(internal_id, &media_hash);
				}
				else
				{
					// getting here means we've likely not got a local file so will
					// have to fiddle things and use ComputeMediaHash() to attempt
					// to generate a mediahash we can then use in the later working
					if (ComputeMediaHash(last_fileuri->string, &media_hash) != NErr_Success)
					{
						// look for demostream as we can get the mediahash
						// and from there then get the other details needed
						// as we cannot guarantee getting it other clients
						ReferenceCountedNXString username, cloud_url, demo_stream;
						REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_True);
						REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0);
						NXStringCreateWithFormatting(&demo_stream, "%sdemostream/%s/",
													 AutoCharPrintfUTF8(cloud_url), AutoCharPrintfUTF8(username));
						if (wcsstr(last_fileuri->string, demo_stream->string))
						{
							wchar_t *p = _wcsdup(last_fileuri->string + NXStringGetLength(demo_stream)), *pt = wcstok(p, L"/");
							if (pt != NULL)
							{
								// should be able to get the cloud_id and mediahash from the demostream urls...
								int cloud_id = _wtoi(pt);
								if (cloud_id > 0)
								{
									pt = wcstok(NULL, L"/");
									if (pt != NULL)
									{
										NXStringCreateWithUTF16(&media_hash, pt);
									}
								}
							}
							if (p) free(p);
						}

						// if we got here, then we're going to have to do more work...
						if (!media_hash)
						{
							for (size_t i = 0, j = 0; i < num_devices; i++)
							{
								int device_id;
								if (ui_db_connection->Devices_Find(devices[i], &device_id, 0) == NErr_Success && device_id > 0)
								{
									internal_id = is_ignored = 0;
									if (ui_db_connection->Media_FindByFilename(last_fileuri, device_id, &internal_id, &is_ignored) == NErr_Success)
									{
										ui_db_connection->IDMap_GetMediaHash(internal_id, &media_hash);
										break;
									}
								}
							}
						}
					}
				}
				if (devices) free(devices);

				int *out_device_ids = 0;
				size_t num_device_ids = 0;
				ui_db_connection->IDMap_Get_Devices_From_MediaHash(media_hash, &out_device_ids, &num_device_ids, 0);
				if (num_device_ids > 0)
				{
					for (size_t i = 0; i < num_device_ids; i++)
					{
						CheckMenuItem(*menu, CLOUD_SOURCE_MENUS + (out_device_ids[i]), MF_CHECKED);
						// if we have availability, then we need to allow for removes even if adds are not supported
						EnableMenuItem(*menu, CLOUD_SOURCE_MENUS + (out_device_ids[i]), MF_ENABLED);
					}
				}

				if (out_device_ids) free(out_device_ids);
			}
		}
		// otherwise it's a playlist and so we need to check different aspects for what to display
		else
		{
			last_menu_playlist = 1;
			*menu = CreatePopupMenu();

			nx_string_t *devices = 0;
			size_t num_devices = 0;
			if (ui_db_connection->Devices_GetIDs(&devices, &num_devices) == NErr_Success)
			{
				// see if we have a local track with a matching media hash and use that
				// otherwise we'll have to go the streaming attempt for playing the file
				lstrcpyn(last_menu_filepath, filepath, MAX_PATH);
				ReferenceCountedNXString last_pl_uid;
				NXStringCreateWithUTF16(&last_pl_uid, filepath);

				// TODO - update status of the playlist here
				int64_t playlist_id = 0, dirty = 0;
				ReferenceCountedNXString uid;
				NXStringCreateWithUTF16(&uid, filepath);
				ui_db_connection->Playlists_Find(uid, &playlist_id, &dirty);
				// TODO check
				if (playlist_id > 0 && dirty == 4) playlist_id = 0;

				for (size_t i = 0, j = 0; i < num_devices; i++)
				{
					DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct;
					int device_id;
					if (ui_db_connection->Devices_Find(devices[i], &device_id, device_info) == NErr_Success)
					{
						// TODO if sending to other devices becomes supported then this can be changed
						if (!NXStringKeywordCompareWithCString(devices[i], HSS_CLIENT)/* ||
							(device_id == local_device_id)*/)
						{
							// TODO when we've a reliable way to know if a device supports uploads then change this hard-coding
							// TODO when further device <-> device support happens then this can be changed to allow the menu items, etc
							bool supports_uploads = (!NXStringKeywordCompareWithCString(devices[i], HSS_CLIENT) ||
													 !NXStringKeywordCompareWithCString(devices[i], DROPBOX_CLIENT));
							InsertMenu(*menu, (device_id == local_device_id ? 0 : ++j),
									   MF_BYPOSITION | (!supports_uploads && (device_id != local_device_id) ? MF_GRAYED : 0) | (playlist_id > 0 ? MF_CHECKED : 0),
									   CLOUD_SOURCE_MENUS + device_id,
									   (device_id == local_device_id ? WASABI_API_LNGSTRINGW(IDS_LOCAL_LIBRARY) : device_info->name->string));
						}
					}
					NXStringRelease(devices[i]);
					delete device_info;
				}
				if (devices) free(devices);
			}
		}
	}
}

void Cloud_Background::OnProcessCloudStatus(int menu_item, int *result)
{
	if (!first_pull)
	{
		*result = 0;
		return;
	}

	if (!last_menu_playlist)
	{
		// see if we have a local track with a matching media hash and use that
		// otherwise we'll have to go the streaming attempt for playing the file
		ReferenceCountedNXString media_hash;
		int internal_id = 0, is_ignored = 0;

		lstrcpyn(last_menu_filepath, last_menu_filepath, MAX_PATH);
		ReferenceCountedNXURI last_fileuri;
		NXURICreateWithUTF16(&last_fileuri, last_menu_filepath);

		if (ui_db_connection->Media_FindByFilename(last_fileuri, local_device_id, &internal_id, &is_ignored) == NErr_Success)
		{
			ui_db_connection->IDMap_GetMediaHash(internal_id, &media_hash);
		}

		int ret = 0, found = 0, device = (menu_item - CLOUD_SOURCE_MENUS);
		int *out_device_ids = 0, *out_media_ids = 0;
		size_t num_device_ids = 0;
		ui_db_connection->IDMap_Get_Devices_From_MediaHash(media_hash, &out_device_ids, &num_device_ids, &out_media_ids);
		if (num_device_ids > 0)
		{
			for (size_t i = 0; i < num_device_ids; i++)
			{
				if (device == out_device_ids[i])
				{
					nx_string_t name;
					if (ui_db_connection->Devices_GetName(out_device_ids[i], &name, 0) == NErr_Success)
					{
						found = out_device_ids[i];
						wchar_t buf[256];
						StringCchPrintfW(buf, ARRAYSIZE(buf), WASABI_API_LNGSTRINGW((device == local_device_id) ? IDS_REMOVE_FROM_LOCAL_CLOUD_DEVICE : IDS_REMOVE_FROM_CLOUD_DEVICE), name->string);
						if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_REMOVE_FROM_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
						{
							ui_db_connection->BeginTransaction();
							ui_db_connection->IDMap_Remove(out_media_ids[i]);
							ui_db_connection->Commit();
#ifdef _DEBUG
							if (cloud_client) cloud_client->Flush();
#endif
							*result = 4;
						}
						break;
					}
				}
			}
		}

		if (!found)
		{
			nx_string_t name, token;
			if (ui_db_connection->Devices_GetName(device, &name, &token) == NErr_Success)
			{
				wchar_t buf[1024];
				bool supports_uploads = (!NXStringKeywordCompareWithCString(token, HSS_CLIENT) ||
										 !NXStringKeywordCompareWithCString(token, DROPBOX_CLIENT));

				// local add
				if (device == local_device_id)
				{
					StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to add this song to the 'Local Library' device?");
					if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						this->OnFileAdded(last_menu_filepath);
						*result = 2;
					}
				}
				// uploads (hss / dropbox)
				else if (supports_uploads)
				{
					//StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to upload this song to '%s' so it will be available on your other Cloud sources?", name->string);
					//if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						HWND ml_pmp_window = FindWindow(L"ml_pmp_window", NULL);
						if (IsWindow(ml_pmp_window))
						{
							cloudDeviceTransfer *transfer = new (std::nothrow) cloudDeviceTransfer;
							ZeroMemory(transfer->filenames, MAX_PATH + 1);
							lstrcpyn(transfer->filenames, last_menu_filepath, MAX_PATH);
							transfer->device_token = NXStringRetain(token);
							PostMessage(ml_pmp_window, WM_PMP_IPC, (WPARAM)transfer, PMP_IPC_DEVICECLOUDTRANSFER);
							*result = 1;
						}
					}
				}
				// TODO device <-> device
				else
				{
					/*StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to add this song to the '%s' device so it can be accessed directly on it?", name->string);
					if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
					}*/
				}
			}
		}

		if (out_device_ids) free(out_device_ids);
	}
	else
	{
		int playlist_index = -*result;
		nx_string_t *devices = 0;
		size_t num_devices = 0;
		if (ui_db_connection->Devices_GetIDs(&devices, &num_devices) == NErr_Success)
		{
			// see if we have a local track with a matching media hash and use that
			// otherwise we'll have to go the streaming attempt for playing the file
			ReferenceCountedNXString last_pl_uid;
			NXStringCreateWithUTF16(&last_pl_uid, last_menu_filepath);

			// TODO - update status of the playlist here
			int64_t playlist_id = 0, dirty = 0;
			ui_db_connection->Playlists_Find(last_pl_uid, &playlist_id, &dirty);
			// TODO check
			if (playlist_id > 0 && dirty == 4) playlist_id = 0;

			for (size_t i = 0, j = 0; i < num_devices; i++)
			{
				DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct;
				int device_id;
				if (ui_db_connection->Devices_Find(devices[i], &device_id, device_info) == NErr_Success)
				{
					// TODO if sending to other devices becomes supported then this can be changed
					if (!NXStringKeywordCompareWithCString(devices[i], HSS_CLIENT)/* ||
						(device_id == local_device_id)*/)
					{
						ReferenceCountedNXString name;
						NXStringCreateWithUTF16(&name, L"test");
						if (!playlist_id)
						{
							*result = 1;

							size_t param1 = 0;
							size_t numItems = 0, length = 0, cloud = 0;
							ReferenceCountedNXString name;
							NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(playlist_index));
							AGAVE_API_PLAYLISTS->GetInfo(playlist_index, api_playlists_itemCount, &numItems, sizeof(numItems));
							AGAVE_API_PLAYLISTS->GetInfo(playlist_index, api_playlists_totalTime, &length, sizeof(length));
							AGAVE_API_PLAYLISTS->GetInfo(playlist_index, api_playlists_cloud, &cloud, sizeof(cloud));

							// add or update the state of an existing playlist as needed
							int mode = 0;
							ui_db_connection->Playlists_AddUpdate(last_pl_uid, name, length * 1.0, numItems, time(0), time(0), (!playlist_id ? 1 : 2), &mode);

							// TODO grab details of the playlist and fill here
							/*MessageBoxW(0, last_pl_uid->string, 0, 0);
							ui_db_connection->Playlists_AddUpdate(last_pl_uid, name, 0.0, 0, time(0), 1);*/
						}
						else
						{
							*result = 2;
							ui_db_connection->Playlists_Remove(last_pl_uid);
						}
					}
				}
				NXStringRelease(devices[i]);
				delete device_info;
			}
			if (devices) free(devices);

			if (cloud_client)
				cloud_client->Flush();
		}
	}
}