#include "api.h"
#include "BackgroundTasks.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "../gen_ml/itemlist.h"
#include "../replicant/nu/PtrList.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/ServiceWatcher.h"
#include "../Wasabi2/main.h"
#include "nx/nxuri.h"
#include "../nu/AutoURL.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include "service.h"
#include "resource.h"
#include "menu.h"
#include "JSAPI2_Creator.h"
#include "jnetlib/jnetlib.h"
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <bfc/multipatch.h>
#include "../playlist/api_playlist.h"
#include "../playlist/pl_entry.h"
#include "../playlist/plstring.h"
#include "../playlist/api_playlistloadercallback.h"

// to allow for easier db updates if needed, we check an ini flag for the time being
// and if less than this we assume an older client version and so nuke the cloud db
#define NUKE_VERSION 23

// to allow for easier media updates if needed, we check an ini flag for the time being
// and if less than this we assume an older client version and so nuke the ignored files
#define NUKE_IGNORED_VERSION 3

ifc_cloudclient *cloud_client=0;
int local_device_id = 0, first_pull = 0, clean_install = 0, network_fail = 0,
	first_login = 0, IPC_GET_CLOUD_HINST = -1, IPC_GET_CLOUD_ACTIVE = -1,
	IPC_LIBRARY_PLAYLISTS_REFRESH = -1, IPC_CLOUD_ENABLED = -1,
	IPC_CLOUD_SHOW_HIDE_LOCAL = -1, auth_error = 0, uniqueAddress;
Cloud_Background cloud_background;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
prefsDlgRecW preferences;
static wchar_t preferencesName[64];
static wchar_t szDescription[256];

enum {	patch_playlist,	patch_playlistloadercallback};

class Playlist : public MultiPatch<patch_playlist, ifc_playlist>,
				 public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS,ifc_plentryinfo *info);
	size_t GetNumItems();
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	int GetItemLengthMilliseconds(size_t item);
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);
	void AppendWithInfo(const wchar_t *filename, const wchar_t *title,
						int lengthInMS, const wchar_t *mediahash,
						const wchar_t *metahash, const wchar_t *cloud_id,
						const wchar_t *cloud_status, const wchar_t *cloud_devices);
	~Playlist();

protected:
	RECVS_MULTIPATCH;

private:
	typedef nu::PtrList<pl_entry> PlaylistEntries;
	PlaylistEntries entries;
};

Playlist::~Playlist()
{
	entries.deleteAll();
}

void Playlist::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	if (info)
	{
		// gets related 'cloud' information from the entry if we have
		// a ifc_plentryinfo param provided to us (mainly as a hint)
		const wchar_t *mediahash = info->GetExtendedInfo(L"mediahash");
		const wchar_t *metahash = info->GetExtendedInfo(L"metahash");
		const wchar_t *cloud_id = info->GetExtendedInfo(L"cloud_id");
		const wchar_t *cloud_status = info->GetExtendedInfo(L"cloud_status");
		const wchar_t *cloud_devices = info->GetExtendedInfo(L"cloud_devices");
		entries.push_back(new pl_entry(filename, title, lengthInMS, mediahash, metahash, cloud_id, cloud_status, cloud_devices));
	}
	else
		entries.push_back(new pl_entry(filename, title, lengthInMS));
}

void Playlist::AppendWithInfo(const wchar_t *filename, const wchar_t *title,
							  int lengthInMS, const wchar_t *mediahash,
							  const wchar_t *metahash, const wchar_t *cloud_id,
							  const wchar_t *cloud_status, const wchar_t *cloud_devices)
{
	entries.push_back(new pl_entry(filename, title, lengthInMS, mediahash,
								   metahash, cloud_id, cloud_status, cloud_devices));
}

size_t Playlist::GetNumItems()
{
	return entries.size();
}

size_t Playlist::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->GetFilename(filename, filenameCch);
}

size_t Playlist::GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->GetTitle(title, titleCch);
}

size_t Playlist::GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->GetExtendedInfo(metadata, info, infoCch);
}

int Playlist::GetItemLengthMilliseconds(size_t item)
{
	if (item >= entries.size())
		return -1;

	return entries[item]->GetLengthInMilliseconds();
}

/*#define CBCLASS Playlist
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
CB(IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
END_DISPATCH;
#undef CBCLASS*/

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Playlist

START_MULTIPATCH;
START_PATCH(patch_playlist)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_CLEAR, Clear)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPENDWITHINFO, AppendWithInfo)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPEND, Append)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEM, GetItem)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
//M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_REVERSE, Reverse)
//M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_SWAP, Swap)
//M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_RANDOMIZE, Randomize)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_REMOVE, Remove)
//M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_SORTBYTITLE, SortByTitle)
//M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_SORTBYFILENAME, SortByFilename)
NEXT_PATCH(patch_playlistloadercallback)
M_VCB(patch_playlistloadercallback, ifc_playlistloadercallback, IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile);
END_PATCH
END_MULTIPATCH;



class MLDB_Watcher : public ServiceWatcherSingle
{
	void OnRegister()
	{
		cloud_background.Initialize();
	}
	void OnDeregister() {
		cloud_background.Release();
	}
};

volatile int is_our_pl = 0;
class CloudPlaylistsCB : public PlaylistsCB
{
	void RebuildCloudPlaylist(int index)
	{
		// when the playlist is added, we need to process it so it'll have all of the cached details, etc
		Playlist local_playlist, cloud_playlist;
		if (AGAVE_API_PLAYLISTMANAGER->Load(AGAVE_API_PLAYLISTS->GetFilename(index), &local_playlist) != PLAYLISTMANAGER_FAILED)
		{
			for (size_t item = 0; item < local_playlist.GetNumItems(); item++)
			{
				wchar_t filename[1024] = {0}, title[400] = {0};
				local_playlist.GetItem(item, filename, 1024);
				local_playlist.GetItemTitle(item, title, 512);
				int duration = local_playlist.GetItemLengthMilliseconds(item);

				ReferenceCountedNXString mediahash, metahash, id_string, cloud_id, cloud_status;
				/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
				ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
				if (db_connection)
				{
					int internal_id = 0, is_ignored = 0;
					ReferenceCountedNXURI filepath;
					NXURICreateWithUTF16(&filepath, filename);
					if (db_connection->Media_FindByFilename(filepath, local_device_id, &internal_id, &is_ignored) == NErr_Success && is_ignored==0)
					{
						db_connection->IDMap_GetMediaHash(internal_id, &mediahash);
						db_connection->IDMap_GetMetaHash(internal_id, &metahash);

						int64_t found_cloud_id = 0;
						if (db_connection->IDMap_Get(internal_id, &found_cloud_id) == NErr_Success)
						{
							NXStringCreateWithInt64(&cloud_id, found_cloud_id);
						}
					}

					// determine the current cloud icon status for the playlist
					// will mean it's slower on building but helps in the views
					wchar_t devices[96] = {0};
					int *out_device_ids = 0;
					size_t num_device_ids = 0;
					nx_string_t *out_tokens = 0;
					db_connection->IDMap_Get_Devices_Token_From_MediaHash(mediahash, &out_device_ids, &out_tokens, &num_device_ids);
					if (num_device_ids > 0)
					{
						int local = 0, remote = 0;
						for (size_t i = 0; i < num_device_ids; i++)
						{
							wchar_t val[4] = {0};
							if (i) StringCchCatW(devices, 96, L"*");
							StringCchCatW(devices, 96, _itow(out_device_ids[i], val, 10));

							if (local_device_id == out_device_ids[i])
							{
								local = 1;
							}
							else
							{
								if (!NXStringKeywordCompareWithCString(out_tokens[i], HSS_CLIENT) ||
									!NXStringKeywordCompareWithCString(out_tokens[i], DROPBOX_CLIENT))
								{
									remote = 1;
								}
							}
						}

						if (local)
						{
							NXStringCreateWithUTF8(&cloud_status, (remote ? "0" : "4"));
						}
						else
						{
							if (remote)
							{
								NXStringCreateWithUTF8(&cloud_status, "1");
							}
							else
							{
								// for url items, only sane thing is to set as a 'full'
								// cloud as it's the best match (meaning 'always there'
								NXStringCreateWithUTF8(&cloud_status, (PathIsURL(filename) ? "1" : "3"));
							}
						}
					}
					if (out_device_ids) free(out_device_ids);
											
					cloud_playlist.AppendWithInfo(filename, title, duration,
								  (mediahash ? mediahash->string : L""),
								  (metahash ? metahash->string : L""),
								  (cloud_id ? cloud_id->string : L"0"),
								  (cloud_status ? cloud_status->string : L"4"),
								  devices);
				}
			}
			AGAVE_API_PLAYLISTMANAGER->Save(AGAVE_API_PLAYLISTS->GetFilename(index), &cloud_playlist);
		}
	}

	void OnPlaylistAdded(int index)
	{
		// if adding the playlist ourselves (e.g. from a snapshot) then skip processing
		if (is_our_pl == 1) return;

		// check if the playlist is one flagged for the cloud. if not then skip it asap
		int64_t cloud = 0;
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
		if (cloud > 0)
		{
			/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection)
			{
				int64_t entries = 0, duration = 0;
				ReferenceCountedNXString name, uuid;
				NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(index));

				GUID guid = AGAVE_API_PLAYLISTS->GetGUID(index);
				char temp[64] = {0};
				StringCbPrintfA(temp, sizeof(temp), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
								(int)guid.Data1, (int)guid.Data2, (int)guid.Data3, (int)guid.Data4[0],
								(int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
								(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
				NXStringCreateWithUTF8(&uuid, temp);

				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &entries, sizeof(entries));
				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &duration, sizeof(duration));

				// when the playlist is added, we need to process it so it'll have all of the cached details, etc
				RebuildCloudPlaylist(index);

				int mode = 0;
				db_connection->Playlists_AddUpdate(uuid, name, (duration*1000.0), entries, time(0), time(0), ifc_clouddb::PLAYLIST_LOCAL_ADD, &mode);
				// flush so we're not waiting
				cloud_client->Flush();
			}
		}
	}

	void OnPlaylistRemoved(int index)
	{
		// if removing the playlist from a remote request then skip processing
		if (is_our_pl == 1) return;

		// check if the playlist is one flagged for the cloud. if not then skip it asap
		int64_t cloud = 0;
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
		if (cloud > 0)
		{
			/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection)
			{
				int cloud = 0;
				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
				// make sure we only do it on what is marked as a cloud playlist
				if (cloud)
				{
					ReferenceCountedNXString uuid;
					GUID guid = AGAVE_API_PLAYLISTS->GetGUID(index);
					char temp[64] = {0};
					StringCbPrintfA(temp, sizeof(temp), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
									(int)guid.Data1, (int)guid.Data2, (int)guid.Data3, (int)guid.Data4[0],
									(int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
									(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
					NXStringCreateWithUTF8(&uuid, temp);
					db_connection->Playlists_Remove(uuid);
					// flush so we're not waiting
					cloud_client->Flush();
				}
			}
		}
	}

	void OnPlaylistRenamed(int index)
	{
		// if renaming the playlist ourselves then skip processing
		if (is_our_pl == 1) return;

		OnPlaylistSaved(index);
	}

	void OnPlaylistSaved(int index)
	{
		// check if the playlist is one flagged for the cloud. if not then skip it asap
		int64_t cloud = 0;
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
		if (cloud > 0)
		{
			/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection)
			{
				int64_t entries = 0, duration = 0;
				ReferenceCountedNXString name, uuid;
				NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(index));

				GUID guid = AGAVE_API_PLAYLISTS->GetGUID(index);
				char temp[64] = {0};
				StringCbPrintfA(temp, sizeof(temp), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
								(int)guid.Data1, (int)guid.Data2, (int)guid.Data3, (int)guid.Data4[0],
								(int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
								(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
				NXStringCreateWithUTF8(&uuid, temp);

				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &entries, sizeof(entries));
				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &duration, sizeof(duration));
				int mode = 0;
				db_connection->Playlists_AddUpdate(uuid, name, (duration*1000.0), entries, time(0), time(0), ifc_clouddb::PLAYLIST_LOCAL_UPDATE, &mode);
				// flush so we're not waiting
				cloud_client->Flush();
			}
		}
	}
};

class DeviceCloudCallback : public cb_cloudevents, public cb_cloud_upload
{
	void WASABICALL CloudEvents_OnRevision(ifc_cloudclient *client, int64_t revision, int from_reset)
	{
		if (!from_reset)
			DebugConsole_GetRevision(revision);
		else
		{
			if (revision >= 0)
			{
				cloud_background.OnReset();
				MessageBox(child_window, L"Reset Succeeded\r\nWill start re-adding files shortly.", L"Cloud Reset", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(child_window, L"Reset Failed\r\nSee logs for more details.", L"Cloud Reset", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	void WASABICALL CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced)
	{
		cloud_background.OnFirstPull(clean_install, forced);

		first_pull = 1;
		network_fail = 0;
		auth_error = 0;

		// enable some of the actions on the preferences dialog now that a pull has happened
		if (IsWindow(child_window))
		{
			EnableWindow(GetDlgItem(child_window, IDC_RESET), TRUE);
			EnableWindow(GetDlgItem(child_window, IDC_RESCAN), TRUE);
			/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection && db_connection->Devices_GetName(local_device_id, &current_device_name, 0) == NErr_Success && first_pull)
			{
				SetDlgItemText(child_window, IDC_CURRENT_NAME, current_device_name->string);
				EnableWindow(GetDlgItem(child_window, IDC_CURRENT_NAME), TRUE);

				if (IsWindow(debug_console_window))
				{
					HWND tab_window = GetDlgItem(debug_console_window, IDC_TAB1);
					TCITEM tc = {TCIF_TEXT|TCIF_PARAM,0,0,WASABI_API_LNGSTRINGW(IDS_VIEW),0,0,0};
					tab_window = GetDlgItem(debug_console_window, IDC_TAB1);
					tc.lParam = 4;
					TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
					InvalidateRect(GetDlgItem(debug_console_window, IDC_DEV_MODE), NULL, FALSE);
				}
			}
		}

		if (IsWindow(invalid_view))
			DebugConsole_UpdateIgnoredFiles();

		StatusWindow_Message(0);

		// post a message to indicate this has happened so other plug-ins (e.g. libary) can update as applicable
		PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_CLOUD_ENABLED);

		// after a first full pull has happened for the very first time
		// we check available playlists and ask to add them to the cloud
		int first_run = Config_GetFirstRun();
		if (AGAVE_API_PLAYLISTS && first_run < 4)
		{
			size_t count = AGAVE_API_PLAYLISTS->GetCount(), in_cloud = 0;
			if (count)
			{
				for (size_t index = 0; index < count; index++)
				{
					size_t cloud = 0;
					AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
					if (cloud) in_cloud++;
				}

				// playlists and nothing known to the cloud
				if (!in_cloud)
				{
					wchar_t msg[1024] = {0};
					StringCchPrintfW(msg, 1024, WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_ALL_PLAYLISTS), count);
					if (MessageBox(plugin.hwndLibraryParent, msg, WASABI_API_LNGSTRINGW(IDS_ADD_PL_TO_CLOUD),
								   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1) == IDYES)
					{
						/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection)
						{
							for (size_t index = 0; index < count; index++)
							{
								size_t cloud = 1;
								AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));

								int64_t entries = 0, duration = 0;
								ReferenceCountedNXString name, uuid;
								NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(index));

								GUID guid = AGAVE_API_PLAYLISTS->GetGUID(index);
								char temp[64] = {0};
								StringCbPrintfA(temp, sizeof(temp), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
												(int)guid.Data1, (int)guid.Data2, (int)guid.Data3, (int)guid.Data4[0],
												(int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
												(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
								NXStringCreateWithUTF8(&uuid, temp);

								AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &entries, sizeof(entries));
								AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &duration, sizeof(duration));
								int mode = 0;
								db_connection->Playlists_AddUpdate(uuid, name, (duration*1.0), entries, time(0), time(0), ifc_clouddb::PLAYLIST_LOCAL_ADD, &mode);
							}

							// refresh playlists in the tree so these forced changes will take effect
							PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
						}
					}
				}
				// playlist and some are known to the cloud
				else if (in_cloud < count)
				{
					wchar_t msg[1024] = {0};
					StringCchPrintfW(msg, 1024, WASABI_API_LNGSTRINGW(IDS_ANNOUNCE_PARTIAL_PLAYLISTS), count - in_cloud, count);
					if (MessageBox(plugin.hwndLibraryParent, msg, WASABI_API_LNGSTRINGW(IDS_ADD_PL_TO_CLOUD),
								   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1) == IDYES)
					{
						/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection)
						{
							for (size_t index = 0; index < count; index++)
							{
								// only process non-cloud flagged playlists
								size_t cloud = 0;
								AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
								if (!cloud)
								{
									cloud = 1;
									AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));

									int64_t entries = 0, duration = 0;
									ReferenceCountedNXString name, uuid;
									NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(index));

									GUID guid = AGAVE_API_PLAYLISTS->GetGUID(index);
									char temp[64] = {0};
									StringCbPrintfA(temp, sizeof(temp), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
													(int)guid.Data1, (int)guid.Data2, (int)guid.Data3, (int)guid.Data4[0],
													(int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
													(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
									NXStringCreateWithUTF8(&uuid, temp);

									AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &entries, sizeof(entries));
									AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &duration, sizeof(duration));
									int mode = 0;
									db_connection->Playlists_AddUpdate(uuid, name, (duration*1.0), entries, time(0), time(0), ifc_clouddb::PLAYLIST_LOCAL_ADD, &mode);
								}
							}

							// refresh playlists in the tree so these forced changes will take effect
							PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
						}
					}
				}
			}
			Config_SetFirstRun(4);
		}

		// TODO signal to views that the pull completed so we can refresh as needed i.e. enable cloud columns, etc ?
	}

	void WASABICALL CloudEvents_OnError(ifc_cloudclient *client, nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field)
	{
		wchar_t buf[2048] = {0};
		if (action && action->len)
		{
			if (field && field->len)
			{
				StringCchPrintf(buf, ARRAYSIZE(buf), L"Error - Action: %s, Code: %s, Message: %s, Field: %s", action->string, code->string, message->string, field->string);
			}
			else
			{
				if (code && code->len)
				{
					if (message && message->len)
					{
						StringCchPrintf(buf, ARRAYSIZE(buf), L"Error - Action: %s, Code: %s, Message: %s", action->string, code->string, message->string);
					}
					else
					{
						StringCchPrintf(buf, ARRAYSIZE(buf), L"Error - Action: %s, Code: %s", action->string, code->string);
					}
				}
				else
				{
					if (message && message->len)
					{
						StringCchPrintf(buf, ARRAYSIZE(buf), L"Error - Action: %s, Message: %s", action->string, message->string);
					}
					else
					{
						StringCchPrintf(buf, ARRAYSIZE(buf), L"Error - Action: %s", action->string);
					}
				}
			}

			if (!NXStringKeywordCompareWithCString(action, "user-device-remove"))
			{
				wchar_t buf2[2048];
				StringCchPrintf(buf2, ARRAYSIZE(buf2), L"The device could not be removed.\n\nError Code: %s\nMessage: %s\nField: %s",
								(code && code->len ? code->string : L"not specified"),
								(message->string ? message->string : L"not specified"),
								(field && field->len ? field->string : L"n\a"));
				MessageBox(plugin.hwndWinampParent, buf2, L"Device Removal", MB_ICONERROR);
			}

			if (!NXStringKeywordCompareWithCString(action, "pull") ||
				!NXStringKeywordCompareWithCString(action, "user-devices") ||
				!NXStringKeywordCompareWithCString(action, "user-profile"))
			{
				nx_string_t con_fail;
				NXStringCreateWithFormatting(&con_fail, "%d", NErr_ConnectionFailed);
				if (!NXStringKeywordCompare(code, con_fail))
				{
					if (!network_fail)
					{
						network_fail = 1;
						DebugConsole_SetStatus(L"Network Connection Failure - Changing To Read-only Mode");
					}
					else
					{
						DebugConsole_SetStatus(L"Network Connection Failure");
					}
					DebugConsole_ShowProgess(-1);
					StatusWindow_Message(IDS_STATUS_NETWORK_FAILURE);
					return;
				}
			}

			DebugConsole_SetStatus(buf);
		}
	}

	void WASABICALL CloudEvents_OnAction(ifc_cloudclient *client, nx_string_t action, nx_string_t message)
	{
		if (network_fail)
		{
			network_fail = 0;
			DebugConsole_SetStatus(L"Network Connection Restored - Leaving Read-only Mode");
			DebugConsole_ShowProgess(0);
		}

		if (!NXStringKeywordCompareWithCString(action, "pull"))
		{
			if (first_pull)
			{
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_PULLING));
			}
		}
		else if (!NXStringKeywordCompareWithCString(action, "forced-pull"))
		{
			// if this happens, indicate that we're doing something in the ui
			DebugConsole_ShowProgess(-1);
			StatusWindow_Message(IDS_STATUS_FIRST_PULL);
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_FORCED_PULLING));
		}
		else if (!NXStringKeywordCompareWithCString(action, "user-devices"))
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_REFRESH_DEVICES_LIST));
		}
		else if (!NXStringKeywordCompareWithCString(action, "user-devices-update"))
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_UPDATING_LOCAL_DEVICE));
		}
		else if (!NXStringKeywordCompareWithCString(action, "user-profile"))
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_GETTING_USER_PROFILE));
		}
		else if (!NXStringKeywordCompareWithCString(action, "metadata-snapshot-playlists"))
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_GETTING_PLAYLISTS));
		}
		else
		{
			DebugConsole_SetStatus((message ? message->string : action->string));
		}
	}

	void WASABICALL CloudEvents_OnUploadStart(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message)
	{
		DebugConsole_SetStatus(message->string);
		WASABI_API_SYSCB->syscb_issueCallback(api_cloud::SYSCALLBACK, api_cloud::CLOUD_UPLOAD_START,
											  (size_t)filepath->string, 0);
	}

	void WASABICALL CloudEvents_OnUploadDone(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message, int code)
	{
		DebugConsole_SetStatus(message->string);
		WASABI_API_SYSCB->syscb_issueCallback(api_cloud::SYSCALLBACK, api_cloud::CLOUD_UPLOAD_DONE,
											  (size_t)filepath->string, code);
	}

	void WASABICALL CloudEvents_OnUnauthorized(ifc_cloudclient *client)
	{
		if (!auth_error)
		{
			auth_error = 1;
			StatusWindow_Message(IDS_UNAUTHORIZED);
			DebugConsole_ShowProgess(-1);
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_UNAUTHORIZED_LOG));
			cloud_background.CredentialsChanged(2);
		}
	}

	void WASABICALL CloudEvents_OnUserProfile(ifc_cloudclient *client, UserProfileStruct *userProfile)
	{
		SetSignInNodeText(userProfile->friendly_name->string);
		Config_SetFriendlyUsername(userProfile->friendly_name->string);
	}

	void WASABICALL CloudEvents_OnPlaylistsDone(ifc_cloudclient *client, ifc_clouddb *db_connection)
	{
		// use this as a means to parse through the playlists and de-cloud any as applicable
		nx_string_t *uuids = 0;
		int64_t *playlist_ids = 0;
		size_t num_playlists = 0;
		db_connection->Playlist_GetIDs(&uuids, &playlist_ids, &num_playlists);
		if (num_playlists)
		{
			// handles where we have some cloud playlists
			size_t count = AGAVE_API_PLAYLISTS->GetCount();
			if (count)
			{
				AGAVE_API_PLAYLISTS->Lock();
				for (size_t index = 0; index < count; index++)
				{
					size_t cloud = 0;
					AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));

					bool found = false;
					GUID uuid = AGAVE_API_PLAYLISTS->GetGUID(index);
					for (size_t i = 0; i < num_playlists; i++)
					{
						GUID _uuid;
						UuidFromStringW((RPC_WSTR)uuids[i]->string, &_uuid);
						if (_uuid == uuid)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						cloud = 0;
						AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
					}
					else
					{
						if (!cloud)
						{
							cloud = 1;
							AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
						}
					}
				}
				AGAVE_API_PLAYLISTS->Unlock();
			}
		}
		else
		{
			// handles where we have no cloud playlists - can just batch remove cloud status
			size_t count = AGAVE_API_PLAYLISTS->GetCount();
			if (count > 0)
			{
				AGAVE_API_PLAYLISTS->Lock();
				for (size_t index = 0; index < count; index++)
				{
					size_t cloud = 0;
					AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
					if (cloud)
					{
						cloud = 0;
						AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
					}
				}
				AGAVE_API_PLAYLISTS->Unlock();
			}
		}
		if (uuids) free(uuids);
		if (playlist_ids) free(playlist_ids);

		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_FINISHED_PL_UPDATES));

		AGAVE_API_PLAYLISTS->Flush();
		// refresh playlists in the tree so these forced changes will take effect
		PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
	}

	void WASABICALL CloudEvents_OnPlaylistAddUpdate(ifc_cloudclient *client, ifc_clouddb *db_connection, int mode, PlaylistStruct *playlist)
	{
		// check if the playlist already exists as need to allow it through even
		// if known to the cloud if it's not here e.g. if playlists.xml was lost
		size_t index = -1;
		if (playlist->priorupdate)
		{
			GUID uuid;
			UuidFromStringW((RPC_WSTR)playlist->uuid->string, &uuid);
			if (AGAVE_API_PLAYLISTS->GetPosition(uuid, &index) == API_PLAYLISTS_FAILURE)
			{
				// force it through
				playlist->priorupdate = 0;
				index = -1;
			}
		}

		// also check the playlist file exists and force a download if lost
		// but not showing as needing an update according to the user's db
		ReferenceCountedNXURI filename;
		if (index != -1)
		{
			NXURICreateWithUTF16(&filename, AGAVE_API_PLAYLISTS->GetFilename(index));
		}
		else
		{
			wchar_t cloud_m3u8[MAX_PATH] = {0}, m3u8[MAX_PATH] = {0};
			StringCbPrintfW(m3u8, sizeof(m3u8), L"Cloud\\playlists\\cloud-%s.m3u8", playlist->uuid->string);
			mediaLibrary.BuildPath(m3u8, cloud_m3u8, MAX_PATH);
			NXURICreateWithUTF16(&filename, cloud_m3u8);
		}

		if (!PathFileExists(filename->string))
		{
			// force it through
			playlist->priorupdate = 0;
		}

		// do what we can to only pull down a playlist copy if it was new or older
		if (!playlist->priorupdate || playlist->lastupdated > playlist->priorupdate)
		{
			cloud_client->DownloadPlaylist(filename, playlist->uuid, 0, this);

			GUID uuid;
			UuidFromStringW((RPC_WSTR)playlist->uuid->string, &uuid);
			is_our_pl = 1;
			int index = AGAVE_API_PLAYLISTS->AddCloudPlaylist(filename->string, playlist->name->string, uuid);
			if (index >= 0)
			{
				size_t numItems = (size_t)playlist->entries, length = (playlist->duration > 0 ? (size_t)playlist->duration / 1000 : 0);
				AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
				AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_totalTime, &length, sizeof(length));
				if (mode & 2) AGAVE_API_PLAYLISTS->Flush();
			}
			is_our_pl = 0;

			wchar_t temp[512] = {0};
			StringCchPrintfW(temp, 512, WASABI_API_LNGSTRINGW(!mode ? IDS_ADD_PLAYLIST : IDS_UPDATE_PLAYLIST), playlist->name->string, playlist->entries);
			DebugConsole_SetStatus(temp);

			// refresh playlists in the tree so these forced changes will take effect
			if (mode & 2) PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
		}
	}

	void WASABICALL CloudEvents_OnPlaylistRemove(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid)
	{
		for (size_t index = 0; index < AGAVE_API_PLAYLISTS->GetCount(); index++)
		{
			GUID _uuid;
			UuidFromStringW((RPC_WSTR)uuid->string, &_uuid);
			if (AGAVE_API_PLAYLISTS->GetGUID(index) == _uuid)
			{
				wchar_t temp[512] = {0};
				StringCchPrintfW(temp, 512, WASABI_API_LNGSTRINGW(IDS_REMOVE_PLAYLIST), AGAVE_API_PLAYLISTS->GetName(index));
				DebugConsole_SetStatus(temp);

				size_t cloud = 0;
				AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
				// check the status and if it does not already have the cloud flag
				// it should be a file we locally cleared from being in the cloud
				// instead of a remote remove request and so don't prompt to remove
				if (cloud)
				{
					wchar_t buf[512] = {0};
					StringCchPrintfW(buf, 512, WASABI_API_LNGSTRINGW(IDS_CLOUD_PL_REMOVAL), AGAVE_API_PLAYLISTS->GetName(index));
					// check what to do from prompt, keep, remove (only need to handle prompt and remove in this part)
					int mode = Config_GetPlRemoveMode();
					if (mode == 2 || (!mode && MessageBox(plugin.hwndWinampParent, buf, WASABI_API_LNGSTRINGW(IDS_CLOUD_PL_REMOVAL_TITLE),
														  MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES))
					{
						wchar_t gs[MAX_PATH];
						lstrcpynW(gs, AGAVE_API_PLAYLISTS->GetFilename(index), MAX_PATH);
						is_our_pl = 1;
						AGAVE_API_PLAYLISTS->RemovePlaylist(index);
						is_our_pl = 0;
						DeleteFileW(gs);
					}
					else
					{
						// otherwise if we're keeping the playlist, make sure to de-cloud it's status
						cloud = 0;
						AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
						AGAVE_API_PLAYLISTS->Flush();
					}
				}

				// refresh playlists in the tree so these forced changes will take effect
				PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
			}
		}
	}

	Playlist upload_playlist;
	void WASABICALL CloudEvents_OnPlaylistUpload(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid, int entry, PlaylistEntry* item)
	{
		const wchar_t* filename = 0;
		for (size_t index = 0; index < AGAVE_API_PLAYLISTS->GetCount(); index++)
		{
			// TODO possibly do look up by guid for filename in the api instead of this to speed it up
			GUID _uuid;
			UuidFromStringW((RPC_WSTR)uuid->string, &_uuid);
			if (AGAVE_API_PLAYLISTS->GetGUID(index) == _uuid)
			{
				filename = AGAVE_API_PLAYLISTS->GetFilename(index);
			}
		}

		if (AGAVE_API_PLAYLISTMANAGER && filename)
		{
			// reset the playlist cache when asked for entry = 0 so we don't reload the playlist always
			if (!entry)
			{
				upload_playlist.Clear();
				AGAVE_API_PLAYLISTMANAGER->Load(filename, &upload_playlist);
			}

			if (upload_playlist.GetNumItems())
			{
				wchar_t title[400] = {0}, filename[MAX_PATH] = {0}, info[512] = {0};

				upload_playlist.GetItemTitle(entry, title, 400/*titleCch*/);
				NXStringCreateWithUTF16(&item->title, title);

				item->duration = upload_playlist.GetItemLengthMilliseconds(entry);

				upload_playlist.GetItem(entry, filename, MAX_PATH/*filenameCch*/);
				NXStringCreateWithUTF16(&item->location, filename);

				// if we get a response then we've got extra info, otherwise it's a non-cloud item
				int cloud_id = 0;
				if (upload_playlist.GetItemExtendedInfo(entry, L"cloud", info, 256/*infoCch*/) && (cloud_id = _wtoi(info)) > 0)
				{
					item->media_id = cloud_id;

					if (upload_playlist.GetItemExtendedInfo(entry, L"metahash", info, 256/*infoCch*/))
					{
						NXStringCreateWithUTF16(&item->metahash, info);
					}

					if (upload_playlist.GetItemExtendedInfo(entry, L"mediahash", info, 256/*infoCch*/))
					{
						NXStringCreateWithUTF16(&item->mediahash, info);
					}
				}
				else
				{
					int internal_id = 0, is_ignored = 0;
					ReferenceCountedNXURI filepath;
					NXURICreateWithNXString(&filepath, item->location);
					if (db_connection->Media_FindByFilename(filepath, local_device_id, &internal_id, &is_ignored) == NErr_Success && is_ignored==0 && internal_id > 0)
					{
						ReferenceCountedNXString media_hash;
						if (db_connection->IDMap_GetMediaHash(internal_id, &media_hash) == NErr_Success)
						{
							item->mediahash = media_hash;
						}

						ReferenceCountedNXString meta_hash;
						if (db_connection->IDMap_GetMetaHash(internal_id, &meta_hash) == NErr_Success)
						{
							item->metahash = meta_hash;
						}

						int64_t found_cloud_id = 0;
						if (db_connection->IDMap_Get(internal_id, &found_cloud_id) == NErr_Success)
						{
							item->media_id = found_cloud_id;
						}
					}
				}
			}
		}
	}

	void WASABICALL CloudUploadCallback_OnDownloadFinished(nx_uri_t filename, ifc_clouddb *db_connection, const JSON::Value *playlist)
	{
		if (AGAVE_API_PLAYLISTMANAGER && playlist)
		{
			Playlist local_playlist;

			ReferenceCountedNXString name_string, identifier_string;
			const JSON::Value *name=0, *identifier=0;
			if (playlist->FindNextKey(0, "name", &name) == NErr_Success)
				name->GetString(&name_string);
			if (playlist->FindNextKey(0, "identifier", &identifier) == NErr_Success)
				identifier->GetString(&identifier_string);

			const JSON::Value *tracks_array = 0;
			if (playlist->FindNextKey(0, "track", &tracks_array) == NErr_Success)
			{
				size_t i = 0;
				const JSON::Value *track=0, *track_value=0;
				while (tracks_array->EnumerateValues(i++, &track) == NErr_Success)
				{
					int64_t duration = 0;
					ReferenceCountedNXString track_title, track_location, track_identifier;
					if (track->FindNextKey(0, "title", &track_value) == NErr_Success)
						track_value->GetString(&track_title);

					if (track->FindNextKey(0, "duration", &track_value) == NErr_Success)
						track_value->GetInteger(&duration);

					ReferenceCountedNXString location_string;
					if (track->FindNextKey(0, "location", &track_value) == NErr_Success)
					{
						track_value->GetString(&track_location);

						size_t j = 0;
						const JSON::Value *location = 0;
						while (track_value->EnumerateValues(j++, &location) == NErr_Success)
						{
							if (location->GetString(&location_string) != NErr_Success)
								location_string = 0;
							break;
						}
					}

					ReferenceCountedNXString mediahash, metahash, id_string, cloud_id, cloud_status;
					if (track->FindNextKey(0, "identifier", &track_value) == NErr_Success)
					{
						track_value->GetString(&track_identifier);

						size_t j = 0;
						const JSON::Value *id = 0;
						while (track_value->EnumerateValues(j++, &id) == NErr_Success)
						{
							// TODO seem to be getting multiples of this... is that allowed??
							id->GetString(&id_string);
							wchar_t * str = wcsstr(id_string->string, L"urn:nullsoft-com:media-id:");
							if (str)
							{
								// TODO better handle multiple media ids...
								// 		as the webclient will send multiples
								NXStringCreateWithUTF16(&cloud_id, str+26);
								continue;
							}

							str = wcsstr(id_string->string, L"urn:nullsoft-com:metahash:");
							if (str)
							{
								NXStringCreateWithUTF16(&metahash, str+26);
								continue;
							}

							str = wcsstr(id_string->string, L"urn:nullsoft-com:mediahash:");
							if (str)
							{
								NXStringCreateWithUTF16(&mediahash, str+27);
								continue;
							}
						}
					}

					if (!mediahash)
					{
						// if there is no mediahash provided e.g. as the webclient does
						// then to make things easier later on we get it where possible
						if (cloud_id)
						{
							int internal_id = 0;
							db_connection->IDMap_Find(_wtoi(cloud_id->string), &internal_id);
							if (internal_id > 0)
							{
								if (db_connection->IDMap_GetMediaHash(internal_id, &mediahash) != NErr_Success)
								{
									mediahash = 0;
								}
							}
						}
						else
						{
							// look for demostream as we can get the mediahash
							// and from there then get the other details needed
							// as we cannot guarantee getting it other clients
							ReferenceCountedNXString username, cloud_url, demo_stream;
							REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_True);
							REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0);
							NXStringCreateWithFormatting(&demo_stream, "%sdemostream/%s/",
														 AutoCharPrintfUTF8(cloud_url), AutoCharPrintfUTF8(username));
							if (wcsstr(location_string->string, demo_stream->string))
							{
								wchar_t *p = _wcsdup(location_string->string + NXStringGetLength(demo_stream)), *pt = wcstok(p, L"/");
								if (pt != NULL)
								{
									// should be able to get the cloud_id and mediahash from the demostream urls...
									if (_wtoi(pt) > 0)
									{
										NXStringCreateWithUTF16(&cloud_id, pt);
										pt = wcstok(NULL, L"/");
										if (pt)
										{
											// TODO check this will be ok...
											NXStringCreateWithUTF16(&mediahash, pt);
										}
									}
								}
								if (p) free(p);
							}
						}
					}

					// if no location, then we attempt to get a local location to get the playlist working
					if (!location_string && cloud_id)
					{
						int internal_id = 0;
						db_connection->IDMap_Find(_wtoi(cloud_id->string), &internal_id);
						if (internal_id > 0)
						{
							ReferenceCountedNXURI value;
							if (db_connection->Media_FindFilepathByMediahash(local_device_id, mediahash, &value) == NErr_Success)
							{
								NXStringCreateWithUTF16(&location_string, value->string);
							}
							else
							{
								location_string = 0;
							}
						}

						// but if that fails (and we have a mediahash) then we resort to a demostream url
						// note: this is not ideal but no location from a client making a playlist sucks!
						if (!location_string && mediahash)
						{
							ReferenceCountedNXString username, cloud_url;
							REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_True);
							REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0);
							NXStringCreateWithFormatting(&location_string, "%sdemostream/%s/%s/%s",
														 AutoCharPrintfUTF8(cloud_url), AutoCharPrintfUTF8(username),
														 AutoCharPrintfUTF8(cloud_id), AutoCharPrintfUTF8(mediahash));
						}
					}

					if (location_string)
					{
						// determine the current cloud icon status for the playlist
						// will mean it's slower on building but helps in the views
						wchar_t devices[96] = {0};
						int *out_device_ids = 0;
						size_t num_device_ids = 0;
						nx_string_t *out_tokens = 0;
						db_connection->IDMap_Get_Devices_Token_From_MediaHash(mediahash, &out_device_ids, &out_tokens, &num_device_ids);
						if (num_device_ids > 0)
						{
							int local = 0, remote = 0;
							for (size_t i = 0; i < num_device_ids; i++)
							{
								wchar_t val[4] = {0};
								if (i) StringCchCatW(devices, 96, L"*");
								StringCchCatW(devices, 96, _itow(out_device_ids[i], val, 10));

								if (local_device_id == out_device_ids[i])
								{
									local = 1;
								}
								else
								{
									if (!NXStringKeywordCompareWithCString(out_tokens[i], HSS_CLIENT) ||
										!NXStringKeywordCompareWithCString(out_tokens[i], DROPBOX_CLIENT))
									{
										remote = 1;
									}
								}
							}

							if (local)
							{
								NXStringCreateWithUTF8(&cloud_status, (remote ? "0" : "4"));
							}
							else
							{
								if (remote)
								{
									NXStringCreateWithUTF8(&cloud_status, "1");
								}
								else
								{
									// for url items, only sane thing is to set as a 'full'
									// cloud as it's the best match (meaning 'always there'
									NXStringCreateWithUTF8(&cloud_status, (PathIsURL(location_string->string) ? "1" : "3"));
								}
							}
						}
						if (out_device_ids) free(out_device_ids);

						local_playlist.AppendWithInfo(location_string->string,
													  // without a title, use the location so we've something set
													  (track_title ? track_title->string : location_string->string),
													  (int)duration,
													  (mediahash ? mediahash->string : L""),
													  (metahash ? metahash->string : L""),
													  (cloud_id ? cloud_id->string : L"0"),
													  (cloud_status ? cloud_status->string : L"4"),
													  devices);
					}
				}
			}

			AGAVE_API_PLAYLISTMANAGER->Save(filename->string, &local_playlist);

			wchar_t * str = wcsstr(identifier_string->string, L"urn:nullsoft-com:playlist-uuid:");
			if (str)
			{
				GUID uuid;
				UuidFromStringW((RPC_WSTR)str+31, &uuid);

				size_t index = -1;
				if (AGAVE_API_PLAYLISTS->GetPosition(uuid, &index) == API_PLAYLISTS_SUCCESS)
				{
					WASABI_API_SYSCB->syscb_issueCallback(api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_SAVED, index, (intptr_t)&uniqueAddress);
					if (NXStringGetLength(name_string) > 0) AGAVE_API_PLAYLISTS->RenamePlaylist(index, name_string->string);
				}
			}
		}
	}
};

static MLDB_Watcher mldb_watcher;
static DeviceCloudCallback device_cloud_callback;
static CloudPlaylistsCB playlistsCB;
DEFINE_EXTERNAL_SERVICE(api_service,          WASABI_API_SVC);
DEFINE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG);
DEFINE_EXTERNAL_SERVICE(api_application,      WASABI_API_APP);
DEFINE_EXTERNAL_SERVICE(api_mldb,             AGAVE_API_MLDB);
DEFINE_EXTERNAL_SERVICE(api_syscb,            WASABI_API_SYSCB);
DEFINE_EXTERNAL_SERVICE(api_metadata,         AGAVE_API_METADATA);
DEFINE_EXTERNAL_SERVICE(api_config,           AGAVE_API_CONFIG);
DEFINE_EXTERNAL_SERVICE(api_memmgr,           WASABI_API_MEMMGR);
DEFINE_EXTERNAL_SERVICE(api_albumart,         AGAVE_API_ALBUMART);
DEFINE_EXTERNAL_SERVICE(JSAPI2::api_security, AGAVE_API_JSAPI2_SECURITY);
DEFINE_EXTERNAL_SERVICE(obj_ombrowser,        AGAVE_OBJ_BROWSER);
DEFINE_EXTERNAL_SERVICE(api_threadpool,       AGAVE_API_THREADPOOL);
DEFINE_EXTERNAL_SERVICE(api_cloud,			  REPLICANT_API_CLOUD);
DEFINE_EXTERNAL_SERVICE(api_playlists,		  AGAVE_API_PLAYLISTS);
DEFINE_EXTERNAL_SERVICE(api_playlistmanager,  AGAVE_API_PLAYLISTMANAGER);

Wasabi2::api_metadata *REPLICANT_API_METADATA=0;
Wasabi2::api_artwork *REPLICANT_API_ARTWORK=0;
Wasabi2::api_service *WASABI2_API_SVC=0;
Wasabi2::api_application *WASABI2_API_APP=0;
Wasabi2::api_mediaserver *REPLICANT_API_MEDIASERVER=0;
JSAPI2Factory jsapi2Factory;
int winampVersion=0;
uint32_t cloud_revision=0;
char winamp_id_str[40];
uint64_t revision=0;
wchar_t ini_file[MAX_PATH];
int cloud_treeItem=0, signin_treeItem=0;
nx_string_t local_device_token=0;
Nullsoft::Utility::LockGuard socket_guard;

static int Init();
static void Quit();
static INT_PTR CloudPluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
		api_t = NULL;
	}
}

winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_cloud.dll)",
	Init,
	Quit,
	CloudPluginMessageProc,
	0,
	0,
	0,
};

ns_error_t GetName(nx_string_t *device_name)
{
	wchar_t computer[256];
	DWORD buffer_size_computer=256;
	if (GetComputerNameW(computer, &buffer_size_computer))
		return NXStringCreateWithUTF16(device_name, computer);
	else
		return NErr_Error;
}


static WNDPROC cloud_oldWndProc;
static LRESULT WINAPI Cloud_WinampWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WA_IPC && lParam == IPC_GET_CLOUD_HINST && IPC_GET_CLOUD_HINST > 65536)
	{
		return (LRESULT)plugin.hDllInstance;
	}
	else if (uMsg == WM_WA_IPC && lParam == IPC_GET_CLOUD_ACTIVE && IPC_GET_CLOUD_ACTIVE > 65536)
	{
		return (REPLICANT_API_CLOUD && REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) == NErr_Success);
	}
	else if (uMsg == WM_WA_IPC && lParam == IPC_CLOUD_SHOW_HIDE_LOCAL && IPC_CLOUD_SHOW_HIDE_LOCAL > 65536)
	{
		Config_SetShowLocal(wParam);
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_REFRESH_DEVICES_LIST));
		cloud_client->DeviceUpdate();
		CheckDlgButton(child_window, IDC_SHOW_LOCAL_DEVICE, wParam);
	}

	return CallWindowProcW(cloud_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

static int Init()
{
	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	winampVersion = mediaLibrary.GetWinampVersion();
	if (winampVersion < 0x5070)
		return ML_INIT_FAILURE;

	mediaLibrary.BuildPath(L"Plugins\\gen_ml.ini", ini_file, MAX_PATH);

	// this is used to determine if we need to force the item places
	int first_run = Config_GetFirstRun();
	if (!first_run)
	{
		Config_SetFirstRun(1);
	}
	else if (first_run == 1)
	{
		Config_SetWebURL(0);
		Config_SetCloudAPIURL(0);
		Config_SetFirstRun(2);
	}

	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == 0	|| WASABI_API_SVC == (api_service*)1) 
		return ML_INIT_FAILURE;

	ServiceBuild(WASABI2_API_SVC, Wasabi2::api_service::GetServiceGUID());
	if (!WASABI2_API_SVC)
		return ML_INIT_FAILURE;

	if (WASABI2_API_SVC)
	{
		WASABI2_API_SVC->GetService(&WASABI2_API_APP);
		WASABI2_API_SVC->GetService(&REPLICANT_API_MEDIASERVER);
	}																								 

	if (REPLICANT_API_MEDIASERVER)
	{
		char hostname[256];
		jnl_dns_gethostname(hostname, 256);
		addrinfo *addr;

		if (jnl_dns_resolve_now(hostname, 0, &addr, SOCK_STREAM) == NErr_Success)
		{
			addrinfo *n = addr;
			while (n)
			{
				if (n->ai_family == AF_INET)
				{
					int ip = ((sockaddr_in *)(n->ai_addr))->sin_addr.S_un.S_addr;
					REPLICANT_API_MEDIASERVER->SetIPv4Address(ip);
					REPLICANT_API_MEDIASERVER->Start();
					break;
				}
				n = n->ai_next;
			}
			jnl_dns_freeaddrinfo(addr);
		}
	}

	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);
	ServiceBuild(AGAVE_OBJ_BROWSER, OBJ_OmBrowser);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(AGAVE_API_THREADPOOL, ThreadPoolGUID);
	ServiceBuild(AGAVE_API_PLAYLISTS, api_playlistsGUID);
	ServiceBuild(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	WASABI2_API_SVC->GetService(&REPLICANT_API_CLOUD);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_ARTWORK);
	if (REPLICANT_API_METADATA)
	{
		ReferenceCountedNXString metadata;
		if (NXStringCreateWithUTF8(&metadata, "cloud/mediahash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MediaHashMetadata::MetadataKey_CloudMediaHash);

		if (NXStringCreateWithUTF8(&metadata, "cloud/art/album/hash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &ArtHashMetadata::MetadataKey_CloudArtHashAlbum);
	}
	WASABI_API_SVC->service_register(&jsapi2Factory);

	// need to get WASABI_API_APP first
	plstring_init();

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance, MlCloudLangGUID);

	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_CLOUD), PLUGIN_VERSION);
	plugin.description = (char*)szDescription;

	ReferenceCountedNXString settings;
	NXStringCreateWithUTF16(&settings, mediaLibrary.GetIniDirectoryW());
	ReferenceCountedNXURI nx_filename;
	NXURICreateWithNXString(&nx_filename, settings);
	Logger::Init(nx_filename, "Cloud\\logs");

	GUID winamp_id;
	WASABI_API_APP->GetUserID(&winamp_id);
	StringCbPrintfA(winamp_id_str, sizeof(winamp_id_str), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					(int)winamp_id.Data1, (int)winamp_id.Data2, (int)winamp_id.Data3, (int)winamp_id.Data4[0],
					(int)winamp_id.Data4[1], (int)winamp_id.Data4[2], (int)winamp_id.Data4[3],
					(int)winamp_id.Data4[4], (int)winamp_id.Data4[5], (int)winamp_id.Data4[6], (int)winamp_id.Data4[7]);

	wchar_t cloud_db_file[MAX_PATH], cloud_db[MAX_PATH];
	switch(Config_GetDevMode())
	{
	case 0: StringCbPrintfW(cloud_db, sizeof(cloud_db), L"Cloud\\local-%hs.db", winamp_id_str); break;
	case 1: StringCbPrintfW(cloud_db, sizeof(cloud_db), L"Cloud\\cloud-%hs.db", winamp_id_str); break;
	case 2: StringCbPrintfW(cloud_db, sizeof(cloud_db), L"Cloud\\qa-%hs.db", winamp_id_str); break;
	case 3: StringCbPrintfW(cloud_db, sizeof(cloud_db), L"Cloud\\stage-%hs.db", winamp_id_str); break;
	}
	
	mediaLibrary.BuildPath(cloud_db, cloud_db_file, MAX_PATH);
	clean_install = !PathFileExistsW(cloud_db_file);

	switch(Config_GetDevMode())
	{
	case 1:
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_DEV_MODE));
		break;
	case 2:
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_QA_MODE));
		break;
	case 3:
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_STAGE_MODE));
		break;
	}

	wchar_t api_url[256] = {0}, web_url[256] = {0}, temp[320] = {0};
	Config_GetCloudAPIURL(api_url, 256);
	if (api_url[0] && wcsicmp(api_url, Config_GetDefaultAPIUrl()))
	{
		StringCchPrintfW(temp, 320, WASABI_API_LNGSTRINGW(IDS_CUSTOM_API_URL), api_url);
		DebugConsole_SetStatus(temp);
	}
	Config_GetWebURL(web_url, 256);
	if (web_url[0] && wcsicmp(web_url, Config_GetDefaultWebUrl()))
	{
		StringCchPrintfW(temp, 320, WASABI_API_LNGSTRINGW(IDS_CUSTOM_WEB_URL), web_url);
		DebugConsole_SetStatus(temp);
	}

	// to allow for easier db updates if needed, we check an ini flag for the time being
	if (Config_GetNukeVersion() < NUKE_VERSION)
	{
		if (!clean_install)
		{
			DeleteFile(cloud_db_file);
			clean_install = 1;
		}
		Config_SetNukeVersion(NUKE_VERSION);
	}

	int show_status = Config_GetShowStatus();
	first_login = Config_GetFirstLogin();
	if (show_status && first_login)
	{
		ToggleStatusWindow(show_status == 2);
	}

	if (REPLICANT_API_CLOUD)
	{
		REPLICANT_API_CLOUD->SetDevMode(Config_GetDevMode());
		NXStringCreateWithUTF8(&local_device_token, winamp_id_str);

		ifc_clouddb *db_connection;
		nx_string_t friendly_name, device_name, device_platform;
		ns_error_t get_name = GetName(&friendly_name);
		NXStringCreateWithUTF16(&device_platform, L"windows");
		if (REPLICANT_API_CLOUD->CreateDatabaseConnection(&db_connection, local_device_token) == NErr_Success)
		{
			db_connection->BeginTransaction();
			if (db_connection->Info_GetLogging(&logMode) != NErr_Success)
			{
				db_connection->Info_SetLogging((logMode = 3));
			}

			if ((logMode & 1) && (logMode & 8)) 
			{
				DebugConsole_ClearLog(0);
			}

			DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct;
			if (db_connection->Devices_Find(local_device_token, &local_device_id, device_info) != NErr_Success)
			{
				if (get_name == NErr_Success)
				{
					db_connection->Devices_Add(local_device_token, friendly_name, device_info, &local_device_id);
				}
				else
				{
					db_connection->Devices_Add(local_device_token, 0, device_info, &local_device_id);
				}
			}
			else
			{
				// this should make sure that we have a friendly name for the client
				// even if one hasn't been obtained properly or db was badly altered
				if (db_connection->Devices_GetName(local_device_id, &device_name, 0) != NErr_Success ||
					!device_name->len)
				{
					if (get_name == NErr_Success)
					{
						db_connection->Info_SetDeviceName(friendly_name);
						db_connection->Devices_Add(local_device_token, friendly_name, device_info, &local_device_id);
					}
				}
				else
				{
					db_connection->Info_SetDeviceName(device_name);
				}
			}
			db_connection->Commit();

			// to allow for easier media updates if needed, we check an ini flag for the time being
			if (Config_GetNukeIgnoredVersion() < NUKE_IGNORED_VERSION)
			{
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_RESET_IGNORED_FILES));
				db_connection->IDMap_ResetIgnored();
				Config_SetNukeIgnoredVersion(NUKE_IGNORED_VERSION);
			}

			db_connection->Release();
			delete device_info;
		}

		if (REPLICANT_API_CLOUD->CreateCloudClient(local_device_token, &cloud_client) == NErr_Success && cloud_client)
		{
			cloud_client->RegisterCallback(&device_cloud_callback);
		}
	}

	// no guarantee that AGAVE_API_MLDB will be available yet, so we'll start a watcher for it
	mldb_watcher.WatchWith(WASABI_API_SVC);
	mldb_watcher.WatchFor(&AGAVE_API_MLDB, mldbApiGuid);
	WASABI_API_SYSCB->syscb_registerCallback(&mldb_watcher);
	WASABI_API_SYSCB->syscb_registerCallback(&playlistsCB);

	preferences.hInst = WASABI_API_LNG_HINST;
	preferences.dlgID = IDD_DEBUG_CONSOLE;
	preferences.proc = (void *)PreferencesDialogBaseProc;
	preferences.name = WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_SOURCES,preferencesName,64);
	preferences.where = 0;
	mediaLibrary.AddPreferences(preferences);

	NAVINSERTSTRUCT nis = {0};
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText = preferencesName;
	nis.item.pszInvariant = L"cloud_sources";
	nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
	nis.item.iImage = nis.item.iSelectedImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_CLOUD);
	nis.item.style = NIS_HASCHILDREN | NIS_ALLOWCHILDMOVE;

	// only force to be after some items on a first run
	// with attempts to appear nearer the top of things
	if (!first_run)
	{
		NAVCTRLFINDPARAMS find = {0};
		find.pszName = L"Playlists";
		find.cchLength = -1;
		find.compFlags = NICF_INVARIANT;
		find.fFullNameSearch = FALSE;
		HNAVITEM item = MLNavCtrl_FindItemByName(plugin.hwndLibraryParent, &find);
		if (!item)
		{
			find.pszName = L"Local Media";
			item = MLNavCtrl_FindItemByName(plugin.hwndLibraryParent, &find);
		}
		if (item) nis.hInsertAfter = item;
	}

	NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
	nvItem.hItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);

	MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
	cloud_treeItem = nvItem.id;

	// TODO this really needs to be split out into a non-cloud plug-in (gen_ml?) for what it'll become
	int sneak = GetPrivateProfileInt(L"winamp", L"sneak", 0, (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIFILEW));
	if (!(sneak & 2))
	{
		wchar_t friendly[256] = {0};
		Config_GetFriendlyUsername(friendly, 256);
		nis.item.cbSize = sizeof(NAVITEM);
		nis.item.pszText = (friendly[0] ? friendly : WASABI_API_LNGSTRINGW(IDS_SIGN_IN));
		nis.item.pszInvariant = L"winamp_sign_in";
		nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
		nis.item.iImage = nis.item.iSelectedImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_SIGN_IN);
		nis.item.style = 0;
		// only force to the top on the first run
		if (first_run < 3)
		{
			nis.hInsertAfter = NCI_FIRST;
			Config_SetFirstRun(3);
		}
		else nis.hInsertAfter = 0;
		nvItem.hItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);

		MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
		signin_treeItem = nvItem.id;
	}

	// subclass the winamp window to get our leet menu item to work
	cloud_oldWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)Cloud_WinampWndProc);
	IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_GET_CLOUD_ACTIVE = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudActive", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_LIBRARY_PLAYLISTS_REFRESH = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_playlist_refresh", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_CLOUD_ENABLED = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudEnabled", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_CLOUD_SHOW_HIDE_LOCAL = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudLocal", IPC_REGISTER_WINAMP_IPCMESSAGE);

	return ML_INIT_SUCCESS;
}

static void Quit()
{
	StatusWindow_SavePos();

	WASABI_API_SYSCB->syscb_deregisterCallback(&cloud_background);
	cloud_background.Kill();
	WASABI_API_SYSCB->syscb_deregisterCallback(&mldb_watcher);
	mldb_watcher.StopWatching();

	if (AGAVE_OBJ_BROWSER)
	{	
		AGAVE_OBJ_BROWSER->Finish();
	}

	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_MLDB, mldbApiGuid);
	ServiceRelease(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);
	ServiceRelease(AGAVE_OBJ_BROWSER, OBJ_OmBrowser);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(AGAVE_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(AGAVE_API_PLAYLISTS, api_playlistsGUID);
	ServiceRelease(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	WASABI_API_SVC->service_deregister(&jsapi2Factory);
	if (REPLICANT_API_CLOUD) REPLICANT_API_CLOUD->Release();
	if (REPLICANT_API_METADATA) REPLICANT_API_METADATA->Release();
	if (REPLICANT_API_ARTWORK) REPLICANT_API_ARTWORK->Release();
	if (WASABI2_API_APP) WASABI2_API_APP->Release();
	if (WASABI2_API_SVC) WASABI2_API_SVC->Release();
}

static INT_PTR Cloud_OnContextMenu(INT_PTR param1, HWND hHost, POINTS pts)
{
	HNAVITEM hItem = (HNAVITEM)param1;
	HNAVITEM signinItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, signin_treeItem);
	HNAVITEM cloudItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, cloud_treeItem);

	if (hItem != signinItem && hItem != cloudItem)
		return FALSE;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	HMENU hMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENUS),
		  subMenu = (NULL != hMenu) ? GetSubMenu(hMenu, (hItem == signinItem)) : NULL;
	if (NULL != subMenu)
	{
		if (hItem == cloudItem)
		{
			CheckMenuItem(subMenu, ID_NAVIGATION_TOGGLESTATUSWINDOW, Config_GetShowStatus() && first_login ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(subMenu, ID_NAVIGATION_SHOW, Config_GetShowLocal() && first_login ? MF_CHECKED : MF_UNCHECKED);
		}
		else
		{
			EnableMenuItem(subMenu, ID_SIGN_ACCOUNT, MF_BYCOMMAND | (MLNavCtrl_GetSelection(plugin.hwndLibraryParent) == signinItem ? MF_DISABLED : MF_ENABLED));
		}

		INT r = Menu_TrackPopup(subMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY |
								TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
								pt.x, pt.y, hHost, NULL);
		if (hItem == signinItem)
		{
			switch(r)
			{
				case ID_SIGN_ACCOUNT:
				{
					HNAVITEM item = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, (signin_treeItem ? signin_treeItem : cloud_treeItem));
					MLNavItem_Select(plugin.hwndLibraryParent, item);
					break;
				}
				case ID_SIGN_TOS:
				{
					SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/legal/cloud");
					break;
				}
				case ID_SIGN_HELP:
					// TODO
					SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/help/Main_Page#Account");
					break;
			}
		}
		else
		{
			switch(r)
			{
				case ID_NAVIGATION_TOGGLESTATUSWINDOW:
					ToggleStatusWindow();
					break;
				case ID_NAVIGATION_PREFERENCES:
					SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE, &preferences);
					break;
				case ID_NAVIGATION_HELP:
					SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/help/Winamp_Cloud");
					break;
				case ID_SIGN_TOS:
					SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/legal/cloud");
					break;
				case ID_NAVIGATION_SHOW:
				{
					PostMessage(plugin.hwndWinampParent, WM_WA_IPC, !Config_GetShowLocal(), IPC_CLOUD_SHOW_HIDE_LOCAL);
				}
				break;
			}
		}
	}

	if (NULL != hMenu)
		DestroyMenu(hMenu);

	return TRUE;
}

static INT_PTR CloudMetadataHook(extendedFileInfoStructW *hookMetadata)
{
	if (NULL == hookMetadata ||
		NULL == hookMetadata->filename ||
		NULL == hookMetadata->metadata)
	{
		return 0;
	}

	// check the output time so we can allow for error messages to still appear, etc as needed
	if (PathIsURLW(hookMetadata->filename) && SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME))
	{
		ReferenceCountedNXString url, username;
		// gets us the http version instead of https
		REPLICANT_API_CLOUD->GetAPIURL(&url, /*http=*/NErr_True);
		REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0);

		if (url && username)
		{
			wchar_t fn[1024] = {0};
			StringCbPrintfW(fn, sizeof(fn), L"%sdemostream/%S/", url->string, AutoUrl(username->string));

			int offset = lstrlen(fn);
			if (offset && !wcsnicmp(hookMetadata->filename, fn, offset))
			{
				if (!_wcsicmp(hookMetadata->metadata, L"streammetadata"))
				{
					StringCchCopy(hookMetadata->ret, hookMetadata->retlen, L"1");
					return 1;
				}
				else if (!_wcsicmp(hookMetadata->metadata, L"streamtype"))
				{
					StringCchCopy(hookMetadata->ret, hookMetadata->retlen, L"0");
					return 1;
				}
				else
				{
					// TODO implement caching on this as am seeing this called repeatedly
					/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
					ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
					if (db_connection)
					{
						Attributes *attributes = cloud_background.Get_Attributes();
						int cloud_id = _wtoi(hookMetadata->filename+offset), internal_id = 0;
						if (cloud_id > 0)
						{
							db_connection->IDMap_Find(cloud_id, &internal_id);
							if (internal_id > 0)
							{
								wchar_t *p = _wcsdup(hookMetadata->filename+offset-1), *pt = wcstok(p, L"/");
								if (pt != NULL)
								{
									pt = wcstok(NULL, L"/");
									if (pt)
									{
										const char *attribute = 0;
										ReferenceCountedNXString value;
										if (!_wcsicmp(hookMetadata->metadata, L"artist")) attribute = "artist";
										else if (!_wcsicmp(hookMetadata->metadata, L"album")) attribute = "album";
										else if (!_wcsicmp(hookMetadata->metadata, L"title") ||
												 !_wcsicmp(hookMetadata->metadata, L"streamtitle"))
										{
											if (db_connection->IDMap_GetTitle(internal_id, &value) == NErr_Success)
											{
												StringCchCopy(hookMetadata->ret, hookMetadata->retlen, value->string);
												if (p) free(p);
												return 1;
											}
										}
										else if (!_wcsicmp(hookMetadata->metadata, L"length"))
										{
											int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
											double duration = 0;
											db_connection->IDMap_GetProperties(internal_id, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration);
											StringCchPrintf(hookMetadata->ret, hookMetadata->retlen, L"%d", (duration > 0 ? (size_t)(duration*1000) : 0));
											return 1;
										}
										else if (!_wcsicmp(hookMetadata->metadata, L"genre")) attribute = "genre";
										else if (!_wcsicmp(hookMetadata->metadata, L"year")) attribute = "year";
										else if (!_wcsicmp(hookMetadata->metadata, L"track")) attribute = "trackno"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"tracks")) attribute = "tracks"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"disc")) attribute = "disc"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"discs")) attribute = "discs"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"bitrate"))
										{
											int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
											double duration = 0;
											db_connection->IDMap_GetProperties(internal_id, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration);
											StringCchPrintf(hookMetadata->ret, hookMetadata->retlen, L"%d", bitrate);
											return 1;
										}
										else if (!_wcsicmp(hookMetadata->metadata, L"playcount"))
										{
											int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
											double duration = 0;
											db_connection->IDMap_GetProperties(internal_id, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration);
											//attribute = "bitrate"; // TODO
											StringCchPrintf(hookMetadata->ret, hookMetadata->retlen, L"%d", playcount);
											return 1;
										}
										else if (!_wcsicmp(hookMetadata->metadata, L"albumartist")) attribute = "albumartist";
										else if (!_wcsicmp(hookMetadata->metadata, L"composer")) attribute = "composer";
										else if (!_wcsicmp(hookMetadata->metadata, L"publisher")) attribute = "publisher";
										else if (!_wcsicmp(hookMetadata->metadata, L"comment")) attribute = "comment";
										else if (!_wcsicmp(hookMetadata->metadata, L"bpm")) attribute = "bpm"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"category")) attribute = "category";
										else if (!_wcsicmp(hookMetadata->metadata, L"director")) attribute = "director";
										else if (!_wcsicmp(hookMetadata->metadata, L"lossless")) attribute = "lossless"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"mime")) 
										{
											if (db_connection->IDMap_GetMIME(internal_id, &value) == NErr_Success)
											{
												StringCchCopy(hookMetadata->ret, hookMetadata->retlen, value->string);
												if (p) free(p);
												return 1;
											}
										}
										else if (!_wcsicmp(hookMetadata->metadata, L"producer")) attribute = "producer";
										else if (!_wcsicmp(hookMetadata->metadata, L"rating")) attribute = "rating"; // TODO
										else if (!_wcsicmp(hookMetadata->metadata, L"type")) attribute = "type"; // TODO

										if (attribute != 0)
										{
											if (db_connection->IDMap_GetString(internal_id, attribute, &value) == NErr_Success)
											{
												StringCchCopy(hookMetadata->ret, hookMetadata->retlen, value->string);
												if (p) free(p);
												return 1;
											}
										}
									}
								}
								if (p) free(p);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

HNAVITEM NavigationItem_Find(HNAVITEM root, const wchar_t *name, BOOL allow_root = 0)
{
	NAVCTRLFINDPARAMS find = {0};
	HNAVITEM item = {0};

	if (NULL == name)
		return NULL;

	if (NULL == plugin.hwndLibraryParent)
		return NULL;

	find.pszName = (wchar_t*)name;
	find.cchLength = -1;
	find.compFlags = NICF_INVARIANT;
	find.fFullNameSearch = FALSE;

	item = MLNavCtrl_FindItemByName(plugin.hwndLibraryParent, &find);
	if (NULL == item)
		return NULL;

	if (!allow_root)
	{
		// if allowed then we can look for root level items which
		// is really for getting 'cloud' devices to another group
		if (NULL != root && 
			root != MLNavItem_GetParent(plugin.hwndLibraryParent, item))
		{
			item = NULL;
		}
	}

	return item;
}

static INT_PTR CloudPluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch(message_type)
	{
		case ML_MSG_TREE_ONCREATEVIEW:
			{
				HRESULT ret = E_FAIL;
				OmService *om_service;

				NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
				nvItem.hItem = NavigationItem_Find(0, L"cloud_add_sources", TRUE);
				MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
				if (nvItem.id == param1)
				{
					ret = OmService::CreateInstance(SERVICE_SOURCES, WASABI_API_LNGSTRINGW(IDS_ADD_SOURCE), &om_service);
				}
				else
				{
					// TODO for the sign-in part, need to split out as needed
					if (cloud_treeItem == param1)
					{
						if (SetCredentials() && cloud_client)
						{
							// at this point, we'd then show an 'all sources' combined view
							break;
						}
						else
						{
							ret = OmService::CreateInstance(SERVICE_SOURCES, preferencesName, &om_service);
						}
					}
					else if(signin_treeItem == param1)
					{
						ret = OmService::CreateInstance(SERVICE_SIGN_IN, WASABI_API_LNGSTRINGW(IDS_WINAMP_ACCOUNT), &om_service);
					}
				}

				if (SUCCEEDED(ret) && AGAVE_OBJ_BROWSER)
				{
					AGAVE_OBJ_BROWSER->Initialize(NULL, plugin.hwndWinampParent);
					
					HWND hView = 0;
					HRESULT hr = AGAVE_OBJ_BROWSER->CreateView(om_service, (HWND)param2, 0, 0, &hView);
					om_service->Release();
					if (SUCCEEDED(hr)) 
					{
						return (INT_PTR)hView;
					}
				}
			}
			break;

		case ML_MSG_NAVIGATION_ONDESTROY:
			if (AGAVE_OBJ_BROWSER)
			{
				AGAVE_OBJ_BROWSER->Finish();
			}
			break;

		case ML_MSG_CONFIG:
			mediaLibrary.GoToPreferences(preferences._id);
			return TRUE;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			return Cloud_OnContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));

		case ML_IPC_HOOKEXTINFOW:
			return CloudMetadataHook((extendedFileInfoStructW *)param1);

		case 0x402/*ML_MSG_CLOUD_PREFS_ID*/:
			return (INT_PTR)&preferences;

		case 0x403/*ML_MSG_CLOUD_SYNC_FILEPATHS*/:
		{
			if (first_pull && !network_fail)
			{
				/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
				ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
				if (db_connection)
				{
					size_t num_filepaths = 0;;
					nx_string_t * out_filepaths = 0;
					// param1 == -1 means use the local cloud device
					// param1 == anything else then we use device id
					// param2 == the dest device id to compare with
					db_connection->IDMap_GetSyncFilePaths((param1 == -1 ? local_device_id : param1)/*source*/, param2/*dest*/, &out_filepaths, &num_filepaths);

					C_ItemList * filenameMaps2 = (C_ItemList *)param3;
					for (size_t i = 0; i < num_filepaths; i++)
					{
						filenameMaps2->Add((void*)_wcsdup(out_filepaths[i]->string));
					}
					if (out_filepaths) free(out_filepaths);
					return (!!num_filepaths);
				}
			}
			return 0;

		case 0x404/*ML_MSG_CLOUD_SYNC_FILEPATHS*/:
			if (param1 && param2)
			{
				if (first_pull && !network_fail || param3 == 0xDEADBEEF)
				{
					size_t num_files = 0;
					/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
					ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
					if (db_connection)
					{
						db_connection->IDMap_GetDeviceCloudFiles(local_device_id, (nx_string_t **)param1/*&out_filenames*/, (int **)param2/*&out_ids*/, &num_files);
					}
					return num_files;
				}
			}
			return 0;

		case 0x405/*ML_MSG_CLOUD_DEVICE_NAMES_FROM_FILEPATHS*/:
			{
				if (param1 && param2)
				{
					wchar_t *filename = (wchar_t*)param1;
					if (filename && *filename)
					{
						/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection)
						{
							ReferenceCountedNXURI fileuri;
							NXURICreateWithUTF16(&fileuri, (wchar_t*)param1/*filename*/);
							size_t num_names = 0;
							db_connection->IDMap_GetDeviceNameFromFilepath(fileuri, (nx_string_t **)param2/*&out_devicenames*/, &num_names);
							return num_names;
						}
					}
				}
			}
			return 0;
		}

		case 0x406:
		{
			if (AGAVE_API_PLAYLISTS)
			{
				/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
				ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
				if (db_connection)
				{
					int64_t playlist_id = 0, dirty = 0;
					GUID info = AGAVE_API_PLAYLISTS->GetGUID(param1);

					ReferenceCountedNXString uid, name;
					NXStringCreateWithFormatting(&uid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
												 (int)info.Data1, (int)info.Data2,
												 (int)info.Data3, (int)info.Data4[0],
												 (int)info.Data4[1], (int)info.Data4[2],
												 (int)info.Data4[3], (int)info.Data4[4],
												 (int)info.Data4[5], (int)info.Data4[6],
												 (int)info.Data4[7]);
					NXStringCreateWithUTF16(&name, AGAVE_API_PLAYLISTS->GetName(param1));

					db_connection->Playlists_Find(uid, &playlist_id, &dirty);
					// TODO check
					if (playlist_id > 0 && dirty == 4) playlist_id = 0;

					size_t numItems = 0, length = 0, cloud = 0;
					AGAVE_API_PLAYLISTS->GetInfo(param1, api_playlists_itemCount, &numItems, sizeof(numItems));
					AGAVE_API_PLAYLISTS->GetInfo(param1, api_playlists_totalTime, &length, sizeof(length));
					AGAVE_API_PLAYLISTS->GetInfo(param1, api_playlists_cloud, &cloud, sizeof(cloud));

					// add or update the state of an existing playlist as needed
					int mode = 0;
					db_connection->Playlists_AddUpdate(uid, name, length * 1.0, numItems, time(0), time(0), (!playlist_id ? 1 : 2), &mode);
				}
			}
			return 0;
		}

		case 0x407/*ML_MSG_CLOUD_DEVICE_NAMES_FROM_METAHASH*/:
			{
				if (param1 && param2)
				{
					wchar_t *filename = (wchar_t*)param1;
					if (filename && *filename)
					{
						/* benski> TODO TODO TODO you cannot just use cloud_background's DB connection on another thread!!!! */
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection)
						{
							ReferenceCountedNXString metahash;
							NXStringCreateWithUTF16(&metahash, (wchar_t*)param1/*metahash*/);
							size_t num_names = 0;
							db_connection->IDMap_GetDeviceNameFromMetahash(metahash, (nx_string_t **)param2/*&out_devicenames*/, &num_names);
							return num_names;
						}
					}
				}
			}
			return 0;
	}

	return FALSE;
}

void SetSignInNodeText(LPWSTR text)
{
	HNAVITEM hItem = NULL;
	if(signin_treeItem)
	{
		hItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, signin_treeItem);
		if (hItem != NULL)
		{
			NAVITEM item = {0};
			item.cbSize = sizeof(NAVITEM);
			item.mask = NIMF_TEXT;
			item.hItem = hItem;
			item.pszText = (LPWSTR)text;
			MLNavItem_SetInfo(plugin.hwndLibraryParent,  &item);
		}
	}
}

extern "C" 
{
	__declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}

#if 0
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// TODO need to get this to close correctly including in pmp_cloud as well, otherwise the db locks
		/*GUID winamp_id;
		WASABI_API_APP->GetUserID(&winamp_id);
		StringCbPrintfA(winamp_id_str, sizeof(winamp_id_str), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
						(int)winamp_id.Data1, (int)winamp_id.Data2, (int)winamp_id.Data3, (int)winamp_id.Data4[0],
						(int)winamp_id.Data4[1], (int)winamp_id.Data4[2], (int)winamp_id.Data4[3],
						(int)winamp_id.Data4[4], (int)winamp_id.Data4[5], (int)winamp_id.Data4[6], (int)winamp_id.Data4[7]);*/

		/*wchar_t cloud_db_file[MAX_PATH], cloud_db[MAX_PATH];
		StringCbPrintfW(cloud_db, sizeof(cloud_db), L"Cloud\\cloud-%hs.db", winamp_id_str);
		mediaLibrary.BuildPath(cloud_db, cloud_db_file, MAX_PATH);
		if (PathFileExistsW(cloud_db_file))
		{
			Quit();
			if (!DeleteFile(cloud_db_file))
			{
				MessageBox(0, cloud_db_file, 0, 0);
			}
		}*/

		wchar_t cloud_db_file[MAX_PATH], cloud_db[MAX_PATH], base_path[MAX_PATH];
		mediaLibrary.BuildPath(L"Cloud\\views", base_path, MAX_PATH);
		MessageBox(0, base_path, 0, 0);

		SHFILEOPSTRUCT shop = {0};
		shop.wFunc = FO_DELETE;
		shop.pFrom = base_path;
		shop.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS|FOF_NORECURSION|FOF_NOERRORUI|FOF_SILENT;
		SHFileOperation(&shop);

		return ML_PLUGIN_UNINSTALL_REBOOT;
	}
#endif
}