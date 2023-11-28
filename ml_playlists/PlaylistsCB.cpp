#include "main.h"
#include "PlaylistsCB.h"
using namespace Nullsoft::Utility;

static UINT_PTR refreshTimer;
void CALLBACK RefreshPlaylistsCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (idEvent == 777)
	{
		refreshTimer = 0;
		RefreshPlaylistsList();
		KillTimer(plugin.hwndWinampParent, idEvent);
	}
}

int PlaylistsCB::notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
		case api_playlists::PLAYLIST_ADDED:
		{
			AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
			size_t newIndex = static_cast<size_t>(param1);
			PlaylistInfo playlist(newIndex);
			if (playlist.Valid())
			{
				// TODO: check where newIndex is, so we can insert into the middle of the list if necessary
				/* TODO: maybe stuff this in Set/GetInfo somewhere
				if (makeTree)
				*/
				MakeTree(playlist);
				if (refreshTimer) KillTimer(plugin.hwndWinampParent, refreshTimer);
				refreshTimer = SetTimer(plugin.hwndWinampParent, 777, 250, RefreshPlaylistsCallback);
				if (!param2) AGAVE_API_PLAYLISTS->Flush(); // REVIEW: save immediately? or only at the end?
			}
			return 1;
		}
		break;
		case api_playlists::PLAYLIST_REMOVED_PRE:
		{
			AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
			size_t index = static_cast<size_t>(param1);
			PlaylistInfo info(index);
			if (info.Valid())
			{
				mediaLibrary.RemoveTreeItem(info.treeId);
				tree_to_guid_map.erase(info.treeId);
			}
			return 1;
		}
		break;
		case api_playlists::PLAYLIST_REMOVED_POST:
		{
			if (refreshTimer) KillTimer(plugin.hwndWinampParent, refreshTimer);
			refreshTimer = SetTimer(plugin.hwndWinampParent, 777, 250, RefreshPlaylistsCallback);
			return 1;
		}
		break;
		case api_playlists::PLAYLIST_RENAMED:
		{
			AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
			// tell the media library to rename the treeview node
			size_t index = static_cast<size_t>(param1);
			PlaylistInfo playlist(index);
			if (playlist.Valid())
			{
				mediaLibrary.RenameTreeId(playlist.treeId, playlist.GetName());
			}
			return 1;
		}
		break;
		case api_playlists::PLAYLIST_SAVED:
			if (param2 != (intptr_t)&uniqueAddress)
			{
				AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt		
				size_t index = static_cast<size_t>(param1);
				playlist_ReloadGUID(AGAVE_API_PLAYLISTS->GetGUID(index));		
			}
			break;
		case api_playlists::PLAYLIST_FLUSH_REQUEST:
			if (param2 != (intptr_t)&uniqueAddress)
			{
				AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
				size_t index = static_cast<size_t>(param1);
				playlist_SaveGUID(AGAVE_API_PLAYLISTS->GetGUID(index));
			}
			break;
	}
	return 0;
}

#define CBCLASS PlaylistsCB
START_DISPATCH;
CB(SYSCALLBACK_GETEVENTTYPE, getEventType);
CB(SYSCALLBACK_NOTIFY, notify);
END_DISPATCH;
#undef CBCLASS