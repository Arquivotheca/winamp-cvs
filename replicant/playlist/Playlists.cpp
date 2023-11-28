#include "api.h"
#include "playlists.h"
#include "metadata/MetadataKeys.h"

#ifdef _WIN32
#pragma comment(lib, "Rpcrt4")
#endif

/*
benski> Notes to maintainers
be sure to call DelayLoad() before doing anything.
This is mainly done because the XML parsing service isn't guaranteed to be registered before this service.
It also improves load time.
*/

/* --------------------------------------------- */

PlaylistInfo::PlaylistInfo()
{
	uri=0;
	name=0;
#ifdef _WIN32
	UuidCreate(&id);
#else
	memset(&id, 0, sizeof(id));
	// TODO:  uuid_generate(&id);
#endif
	length=0;
	items=0;
	bytes=0;
	iTunesID=0;
	time_added=0;
	time_modified=0;
}

PlaylistInfo::~PlaylistInfo()
{
	NXStringRelease(uri);
	NXStringRelease(name);
}

static int ReturnString(nx_string_t in, nx_string_t *out)
{
	if (in)
	{
		*out = NXStringRetain(in);
		return NErr_Success;
	}
	else
		return NErr_Empty;
}
static int ReturnInt(uint64_t in, int64_t *out)
{
	if (in)
	{
		*out = in;
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

static int ReturnReal(double in, double  *out)
{
	if (in)
	{
		*out = in;
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

int PlaylistInfo::Metadata_GetField(int field, int index, nx_string_t *value)
{
	switch(field)
	{
	case MetadataKeys::URI:
		return ReturnString(uri, value);
	case MetadataKeys::TITLE:
		return ReturnString(name, value);			
	default:
		return NErr_Unknown;
	}
}

int PlaylistInfo::Metadata_GetInteger(int field, int index, int64_t *value)
{
	switch(field)
	{
	case MetadataKeys::TRACKS:
		return ReturnInt(items, value);
	case MetadataKeys::FILE_SIZE:
		return ReturnInt(bytes, value);
	case MetadataKeys::FILE_TIME:
		return ReturnInt(time_modified, value);
		// TODO: iTunesID
		// TODO: time_added
	default:
		return NErr_Unknown;
	}

}

int PlaylistInfo::Metadata_GetReal(int field, int index, double *value)
{
	switch(field)
	{		
	case MetadataKeys::LENGTH:
		return ReturnReal(length, value);
	default:
		return NErr_Unknown;
	}

}

/* --------------------------------------------- */
Playlists::Playlists()
{
	NXOnceInit(&once);
	loaded=false;
	iterator = 0;
}

int Playlists::LoadPlaylists(nx_once_t once, void *object, void **unused)
{
	Playlists *playlists = (Playlists *)object;
	/* TODO */

#if 0

	PlaylistsXML loader(this);

	const wchar_t *g_path = WASABI2_API_APP->path_getUserSettingsPath();
	wchar_t playlistsFilename[MAX_PATH];
	PathCombineW(playlistsFilename, g_path, L"plugins");
	PathAppendW(playlistsFilename, L"ml");
	PathAppendW(playlistsFilename,  L"playlists.xml");
	switch (loader.LoadFile(playlistsFilename))
	{
	case PLAYLISTSXML_SUCCESS:
		loaded = true;
		triedLoaded = true;
		if (AGAVE_API_STATS)
			AGAVE_API_STATS->SetStat(api_stats::PLAYLIST_COUNT, playlists.size());
		break;
	case PLAYLISTSXML_NO_PARSER:
		// if there's XML parser, we'll try again on the off-chance it eventually gets loaded (we might still be in the midst of loading the w5s/wac components)
		break;
	default:
		loaded = true;
		triedLoaded = true;
		break;
	}	
#endif
	return 1;
}


bool Playlists::DelayLoad()
{
	NXOnce(&once, LoadPlaylists, this);
	return loaded;
}

void Playlists::Lock()
{
	playlistsGuard.Lock();
}

void Playlists::Unlock()
{
	playlistsGuard.Unlock();
}

size_t Playlists::GetIterator()
{
	return iterator;
}

int Playlists::Playlists_Enumerate(ifc_playlists_enumerator *enumerator)
{
	if (DelayLoad())
	{
		Lock();
		enumerator->OnInfo(this);
		for (PlaylistsList::iterator itr=playlists.begin();itr!=playlists.end();itr++)
		{
			enumerator->OnPlaylist(*itr);
		}
		Unlock();
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Playlists::Metadata_GetField(int field, int index, nx_string_t *value)
{
	return NErr_NotImplemented;
}

int Playlists::Metadata_GetInteger(int field, int index, int64_t *value)
{
		switch(field)
	{
	case MetadataKeys::TRACKS:
		return ReturnInt(playlists.size(), value);
		// TODO: iTunesID
		// TODO: time_added
	default:
		return NErr_Unknown;
	}
}

int Playlists::Metadata_GetReal(int field, int index, double *value)
{
	return NErr_NotImplemented;
}

