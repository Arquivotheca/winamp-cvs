#pragma once
#include <api/syscb/callbacks/syscb.h>
#include "../playlist/api_playlists.h"
#include "../nu/AutoLock.h"
using namespace Nullsoft::Utility;

extern int uniqueAddress;
class PlaylistsCB : public SysCallback
{
private:
	FOURCC GetEventType()
	{
		return api_playlists::SYSCALLBACK;
	}

	int notify(int msg, intptr_t param1, intptr_t param2)
	{
		switch (msg)
		{
			case api_playlists::PLAYLIST_ADDED:
			{
				AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
				size_t index = static_cast<size_t>(param1);
				OnPlaylistAdded(index);
			}
			return 1;

			case api_playlists::PLAYLIST_REMOVED_PRE:
			{
				AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
				size_t index = static_cast<size_t>(param1);
				OnPlaylistRemoved(index);
			}
			return 1;

			case api_playlists::PLAYLIST_RENAMED:
			{
				if (param2 != (intptr_t)&uniqueAddress)
				{
					AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
					size_t index = static_cast<size_t>(param1);
					OnPlaylistRenamed(index);
				}
			}
			return 1;

			case api_playlists::PLAYLIST_SAVED:
			{
				if (param2 != (intptr_t)&uniqueAddress)
				{
					AutoLockT<api_playlists> lock(AGAVE_API_PLAYLISTS);  // should already be locked, but can't hurt
					size_t index = static_cast<size_t>(param1);
					OnPlaylistSaved(index);
				}
			}
			return 1;

			default:
			return 0;
		}
	}

	virtual void OnPlaylistAdded(int index) {}
	virtual void OnPlaylistRemoved(int index) {}
	virtual void OnPlaylistRenamed(int index) {}
	virtual void OnPlaylistSaved(int index) {}

#define CBCLASS PlaylistsCB
	START_DISPATCH_INLINE;
	CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
	CB(SYSCALLBACK_NOTIFY, notify);
	END_DISPATCH;
#undef CBCLASS
};