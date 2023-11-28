#include "JSONMetadata.h"
#include "main.h"

JSONMetadata::JSONMetadata(const JSON::Value *cmd, const JSON::Value *fields) : cmd(cmd), fields(fields) 
{
	art=0;
	if (fields)
	{
		if (fields->FindNextKey(0, "art", &art, 0) != NErr_Success)
			art=0; // just in case
	}
}

static int GetJSONValue(const JSON::Value *fields, const char *field, int index, const JSON::Value **json_value)
{
	size_t iterator;
	int ret = fields->FindNextKey(0, field, json_value, &iterator);
	if (ret == NErr_EndOfEnumeration)
	{
		// we return unknown-field here instead of empty, because a completely missing field means "no change" from the point of view of an update
		return NErr_Unknown;
	}
	else if (ret != NErr_Success)
	{
		return ret;
	}

	while (index--)
	{
		ret = fields->FindNextKey(iterator, field, json_value, &iterator);
		if (ret != NErr_Success)
		{
			return ret;
		}
	}

	if ((*json_value)->data_type == JSON::DATA_NULL)
		return NErr_Empty;

	return NErr_Success;
}

static int GetString(const JSON::Value *fields, const char *field, int index, nx_string_t *value)
{
	const JSON::Value *json_value;
	int ret = GetJSONValue(fields, field, index, &json_value);
	if (ret == NErr_Success)
		return json_value->GetString(value);
	else
		return ret;
}

static int GetJSONInteger(const JSON::Value *fields, const char *field, int index, int64_t *value)
{
	const JSON::Value *json_value;
	int ret = GetJSONValue(fields, field, index, &json_value);
	if (ret == NErr_Success)
		return json_value->GetInteger(value);
	else
		return ret;
}

ns_error_t JSONMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
	case MetadataKeys::ARTIST:
		return GetString(fields, "artist", index, value);
	case MetadataKeys::ALBUM_ARTIST:
		return GetString(fields, "albumartist", index, value);
	case MetadataKeys::ALBUM:
		return GetString(fields, "album", index, value);
	case MetadataKeys::TITLE:
		return GetString(fields, "title", index, value);
	case MetadataKeys::URI:
		return GetString(fields, "filepath", index, value);
	case MetadataKeys::GENRE:
		return GetString(fields, "genre", index, value);
	case MetadataKeys::YEAR:
		return GetString(fields, "year", index, value);
	case MetadataKeys::TRACK:
		return GetString(fields, "trackno", index, value);
	case MetadataKeys::DISC: 
		return GetString(fields, "disc", index, value);
	case MetadataKeys::COMPOSER:
		return GetString(fields, "composer", index, value);
	case MetadataKeys::PUBLISHER:
		return GetString(fields, "publisher", index, value);
	case MetadataKeys::BPM:
		return GetString(fields, "bpm", index, value);
	case MetadataKeys::COMMENT:
		return GetString(fields, "comment", index, value);
	case MetadataKeys::DISCS:
		return GetString(fields, "discs", index, value);
	case MetadataKeys::RATING:
		return GetString(fields, "rating", index, value);
	case MetadataKeys::MIME_TYPE:
		return GetString(fields, "mimetype", index, value);
	case MetadataKeys::TRACK_GAIN:
		return GetString(fields, "trackgain", index, value);
	case MetadataKeys::ALBUM_GAIN:
		return GetString(fields, "albumgain", index, value);
	case MetadataKeys::TRACKS: 
		return GetString(fields, "tracks", index, value);		
	case MetadataKeys::CATEGORY:
		return GetString(fields, "category", index, value);
	case MetadataKeys::DIRECTOR:
		return GetString(fields, "director", index, value);
	case MetadataKeys::PRODUCER:
		return GetString(fields, "producer", index, value);
	}

	if (field == MetadataKey_CloudIDHash)
		return GetString(fields, "idhash", index, value);
	else if (field == MetadataKey_CloudMetaHash)
		return GetString(fields, "metahash", index, value);
	else if (field == MetadataKey_CloudMediaHash)
		return GetString(fields, "mediahash", index, value);
	else if (field == MetadataKey_CloudAlbumHash)
		return GetString(fields, "albumhash", index, value);
	else if (field == MetadataKey_CloudDevice)
		return GetString(cmd, "dev", index, value);
	/* TODO:
	else if (field == MetadataKey_CloudArtHashAlbum)
	{
		if (art)
		{
		}
	}
	*/
		

	return NErr_Unknown;
}

ns_error_t JSONMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	switch(field)
	{
	case MetadataKeys::YEAR:
		return GetJSONInteger(fields, "year", index, value);
	case MetadataKeys::TRACK:
		return GetJSONInteger(fields, "trackno", index, value);
	case MetadataKeys::DISC: 
		return GetJSONInteger(fields, "disc", index, value);
	case MetadataKeys::BPM:
		return GetJSONInteger(fields, "bpm", index, value);
	case MetadataKeys::DISCS:
		return GetJSONInteger(fields, "discs", index, value);
	case MetadataKeys::FILE_SIZE:
		return GetJSONInteger(fields, "filesize", index, value);
	case MetadataKeys::FILE_TIME:
		return GetJSONInteger(fields, "filetime", index, value);
	case MetadataKeys::LENGTH:
		{
			int ret = GetJSONInteger(fields, "length", index, value);
			if (ret == NErr_Success)
				*value = (int64_t)(double(*value) / 1000.0);
			return ret;
		}
	case MetadataKeys::BITRATE:
		return GetJSONInteger(fields, "bitrate", index, value);
	case MetadataKeys::PLAY_COUNT:
		return GetJSONInteger(fields, "playcount", index, value);
	case MetadataKeys::RATING:
		return GetJSONInteger(fields, "rating", index, value);
	case MetadataKeys::TRACKS: 
		return GetJSONInteger(fields, "tracks", index, value);		
	case MetadataKeys::LAST_PLAY:
		return GetJSONInteger(fields, "lastplay", index, value);
	case MetadataKeys::LAST_UPDATE:
		return GetJSONInteger(fields, "lastupd", index, value);
	case MetadataKeys::ADDED:
		return GetJSONInteger(fields, "added", index, value);
	}

	if (field == MetadataKey_CloudID)
		return GetJSONInteger(fields, "id", index, value);

	return NErr_Unknown;
}

ns_error_t JSONMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	switch(field)
	{
	case MetadataKeys::LENGTH:
		{
			int64_t int_value;
			int ret = GetJSONInteger(fields, "length", index, &int_value);
			if (ret == NErr_Success)
				*value = (double(int_value) / 1000.0);
			return ret;
		}	
	}
	return NErr_Unknown;
}
