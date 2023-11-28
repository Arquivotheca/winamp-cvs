#include "main.h"
#include "PlaylistInfo.h"
#include "../nu/AutoLock.h"
#include <assert.h>
#include <strsafe.h>

int uniqueAddress;

using namespace Nullsoft::Utility;
static INT_PTR FindTreeID(GUID playlist_guid)
{
	INT_PTR treeId;
	if (tree_to_guid_map.reverseLookup(playlist_guid, &treeId))
		return treeId;
	else
		return 0;
}

PlaylistInfo::PlaylistInfo()
{
	Clear();
}

void PlaylistInfo::Clear()
{
	length = 0;
	numItems = 0;
	treeId = 0;
	index = 0;
	iterator = 0;
	cloud = 0;
	playlist_guid = INVALID_GUID;
}

bool PlaylistInfo::Associate(INT_PTR _treeId)
{
	Clear();

	treeId = _treeId;
	playlist_guid = tree_to_guid_map[treeId];
	if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
	{
		iterator = AGAVE_API_PLAYLISTS->GetIterator();
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &length, sizeof(length));
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
	}
	else
		playlist_guid = INVALID_GUID;

	return Valid();
}

// as a pre-condition, AGAVE_API_PLAYLISTS needs to be locked
PlaylistInfo::PlaylistInfo(size_t _index)
{
	Clear();

	index = _index;
	iterator = AGAVE_API_PLAYLISTS->GetIterator();
	playlist_guid = AGAVE_API_PLAYLISTS->GetGUID(index);
	AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
	AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &length, sizeof(length));
	AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
	treeId = FindTreeID(playlist_guid); // try to find treeId
}

// as a pre-condition, AGAVE_API_PLAYLISTS needs to be locked
PlaylistInfo::PlaylistInfo(GUID _playlist_guid)
{
	Clear();

	playlist_guid = _playlist_guid;
	if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
	{
		iterator = AGAVE_API_PLAYLISTS->GetIterator();
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &length, sizeof(length));
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
		treeId = FindTreeID(playlist_guid); // try to find treeId
	}
	else
		playlist_guid = INVALID_GUID;
}

PlaylistInfo::PlaylistInfo(const PlaylistInfo &copy)
{
	Clear();

	index = copy.index;
	iterator = copy.iterator;
	playlist_guid = copy.playlist_guid;
	length = copy.length;
	numItems = copy.numItems;
	treeId = copy.treeId;
	cloud = copy.cloud;
}

size_t PlaylistInfo::GetIndex()
{
	assert(Valid());
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	size_t curIterator = AGAVE_API_PLAYLISTS->GetIterator();
	if (curIterator != iterator)
	{
		iterator = curIterator;
		AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index);
	}
	return index;
}

void PlaylistInfo::IssueSaveCallback()
{
	assert(Valid());
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	WASABI_API_SYSCB->syscb_issueCallback(api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_SAVED, GetIndex(), (intptr_t)&uniqueAddress);
}

void PlaylistInfo::Refresh()
{
	assert(Valid());
	if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
	{
		iterator = AGAVE_API_PLAYLISTS->GetIterator();
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
		AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_totalTime, &length, sizeof(length));
	}
	else
		playlist_guid = INVALID_GUID;
}

const wchar_t *PlaylistInfo::GetFilename()
{
	assert(Valid());
	size_t index = GetIndex();
	return AGAVE_API_PLAYLISTS->GetFilename(index);
}

// TODO: we should investigate caching the title
const wchar_t *PlaylistInfo::GetName()
{
	assert(Valid());
	size_t index = GetIndex();
	return AGAVE_API_PLAYLISTS->GetName(index);
}

size_t PlaylistInfo::GetLength()
{
	assert(Valid());
	return length;
}

void PlaylistInfo::SetLength(size_t newLength)
{
	assert(Valid());
	length = newLength;
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	size_t index = GetIndex();
	AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_totalTime, &length, sizeof(length));
}

size_t PlaylistInfo::GetSize()
{
	assert(Valid());
	return numItems;
}

size_t PlaylistInfo::GetCloud()
{
	assert(Valid());
	return cloud;
}

void PlaylistInfo::SetCloud(size_t newCloud)
{
	assert(Valid());
	cloud = newCloud;
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	size_t index = GetIndex();
	AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
}

bool PlaylistInfo::Valid()
{
	return !!(playlist_guid != INVALID_GUID);
}

void PlaylistInfo::SetSize(size_t newSize)
{
	assert(Valid());
	numItems = newSize;
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	size_t index = GetIndex();
	AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_itemCount, &numItems, sizeof(numItems));
}