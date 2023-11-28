#pragma once

#include "api_playlists.h"
#include "nu/PtrList.h"
#include "nu/AutoLock.h"
#include "nx/nxstring.h"
#include "nx/nxonce.h"

class PlaylistInfo : public ifc_metadata
{
public:
	PlaylistInfo();
	~PlaylistInfo();

	nx_string_t uri;
	nx_string_t name;
	GUID id;
	double length; // in seconds
	uint64_t items;
	uint64_t bytes;
	uint64_t iTunesID; // this is used by ml_impex
	time_t time_added;
	time_t time_modified;

private:
	int WASABICALL Metadata_GetField(int field, int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, int index, double *value);
};

class Playlists : public api_playlists, public ifc_metadata
{
public:
	Playlists();

	static int NX_ONCE_API LoadPlaylists(nx_once_t once, void *object, void **unused);
	bool DelayLoad();
	void Lock();
	void Unlock();
	size_t GetIterator();
	int WASABICALL Playlists_Enumerate(ifc_playlists_enumerator *enumerator);
private:
	int WASABICALL Metadata_GetField(int field, int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, int index, double *value);

private:
	typedef nu::PtrList<PlaylistInfo> PlaylistsList;
	PlaylistsList playlists;

	nu::LockGuard playlistsGuard;
	nx_once_value_t once;
	bool loaded;
	size_t iterator;
};

