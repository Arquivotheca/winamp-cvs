#include "main.h"
#include "api.h"
#include "collect.h"
#include <shlwapi.h>
#include <strsafe.h>

	/* MUD data
   ** Artist
	 ** Album
	 ** Title
	 ** Track Number
	 ** Length of Play
	 ** mime type
	 ** URI (filename)
	 ** genre
	 */


CollectedData::CollectedData()
{
	Init();
}

CollectedData::CollectedData(const CollectedData &copy)
{
	Init();
	// make a copy of the incoming data, adding reference to all the strings
	id = copy.id;
	filename = copy.filename;
	ndestring_retain(filename);
	timestamp = copy.timestamp;
	artist = copy.artist;
	ndestring_retain(artist);
	album = copy.album;
	ndestring_retain(album);
	title = copy.title;
	ndestring_retain(title);
	track = copy.track;
	genre = copy.genre;
	ndestring_retain(genre);
	mimetype = copy.mimetype;
	ndestring_retain(mimetype);
	playLength = playLength;
}

CollectedData::~CollectedData()
{
	ndestring_release(filename);
	ndestring_release(artist);
	ndestring_release(album);
	ndestring_release(title);
	ndestring_release(genre);
	ndestring_release(mimetype);
}

void CollectedData::Init()
{
	id=0;
	filename=0;
	timestamp=0;
	artist=0;
	album=0;
	title=0;
	track=0;
	genre=0;
	mimetype=0;
	playLength=0;
	populated=false;
}

void CollectedData::Reset()
{
	ndestring_release(filename);
	ndestring_release(artist);
	ndestring_release(album);
	ndestring_release(title);
	ndestring_release(genre);
	ndestring_release(mimetype);
	id=0;
	filename=0;
	timestamp=0;
	artist=0;
	album=0;
	title=0;
	track=0;
	genre=0;
	mimetype=0;
	playLength=0;
	populated=false;
}

static wchar_t *GetMimeType(const wchar_t *extension)
{
	// TODO: add "mimetype" metadata support
	return 0;
}

static bool Populate_MLDB(CollectedData *data, const wchar_t *filename)
{
	if (!AGAVE_API_MLDB)
		return false;

	itemRecordW *item = AGAVE_API_MLDB->GetFile(filename);
	if (!item)
		return false;

	// benski> I know these things are NDE strings, so I'm gonna cheat a bit
	// this doesn't work when either ml_local or gen_mud is in debug mode
#ifndef _DEBUG
	ndestring_retain(data->filename = item->filename);
	ndestring_retain(data->artist = item->artist);
	ndestring_retain(data->album = item->album);
	ndestring_retain(data->title = item->title);
	ndestring_retain(data->genre = item->genre);
#else // in DEBUG mode just do ndestring_wcsdup
	data->filename = ndestring_wcsdup(item->filename);
	data->artist = ndestring_wcsdup(item->artist);
	data->album = ndestring_wcsdup(item->album);
	data->title = ndestring_wcsdup(item->title);
	data->genre = ndestring_wcsdup(item->genre);
#endif
	data->track = item->track;
	return true;
}

static wchar_t *GetMetadataString(const wchar_t *filename, const wchar_t *tag)
{
	wchar_t metadata[256];
	metadata[0]=0;
	if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, tag, metadata, 256) && metadata[0])
		return ndestring_wcsdup(metadata);
	else
		return 0;
}

static int GetMetadataInt(const wchar_t *filename, const wchar_t *tag)
{
	wchar_t metadata[256];
	metadata[0]=0;
	if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, tag, metadata, 256) && metadata[0])
		return _wtoi(metadata);
	else
		return 0;
}

static bool Populate_Metadata(CollectedData *data, const wchar_t *filename)
{
	if (!AGAVE_API_METADATA)
		return false;

	data->filename = ndestring_wcsdup(filename);
	ndestring_retain(data->artist = GetMetadataString(filename, L"artist"));
	ndestring_retain(data->album = GetMetadataString(filename, L"album"));
	ndestring_retain(data->title = GetMetadataString(filename, L"title"));
	ndestring_retain(data->genre = GetMetadataString(filename, L"genre"));
	data->track = GetMetadataInt(filename, L"track");
	return true;
}

bool CollectedData::Populate(const wchar_t *filename)
{
	// if already populated, return immediately
	if ( populated )
	{
		return (this->artist && this->title);
	}

	// if not already populated
	// set populate to true whether the actual populating process is successful or not 
	// to prevent duplicated unsuccessful metadata retrieval
	populated=true;

	wchar_t built_mimetype[64];
	const wchar_t *ext = PathFindExtension(filename);
	if (ext && *ext)
	{
		ext++;
		mimetype = GetMimeType(ext);
		if (!mimetype)
		{
			StringCbPrintf(built_mimetype, sizeof(built_mimetype), L"audio/%s", ext);
			mimetype = ndestring_wcsdup(built_mimetype);
		}
	}

	// first let's try the media library, it's faster
	if (Populate_MLDB(this, filename))
		return true;

	// if that didn't work, read metadata directly fromt he file
	if (Populate_Metadata(this, filename))
		return true;

	return false;
}
