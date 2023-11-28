#include "api.h"
#include "ItemMetadata.h"
#include "metadata/MetadataKeys.h"

ItemMetadata::~ItemMetadata()
{
	NXStringRelease(filename);
	NXStringRelease(title);
	NXStringRelease(album);
	NXStringRelease(artist);
	NXStringRelease(comment);
	NXStringRelease(genre);
	NXStringRelease(albumartist);
	NXStringRelease(replaygain_album_gain);
	NXStringRelease(replaygain_track_gain);
	NXStringRelease(publisher);
	NXStringRelease(composer);
	NXStringRelease(category);
	NXStringRelease(producer);
	NXStringRelease(director);
	NXStringRelease(metahash);
}

static ns_error_t ItemRecord_GetField(int index, int record, nx_string_t *value)
{
	if (record > 0)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;
		return NXStringCreateWithInt64(value, record);
	}
	else
		return NErr_Empty;
}

static ns_error_t ItemRecord_GetField(int index, nx_string_t record, nx_string_t *value)
{
	if (NXStringGetLength(record) > 0)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(record);
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

static ns_error_t ItemRecord_GetInteger(int index, int64_t record, int64_t *value)
{
	if (index > 0)
		return NErr_EndOfEnumeration;
	*value = record;
	return NErr_Success;
}

static ns_error_t ItemRecord_GetReal(int index, int record, double *value)
{
	if (record > 0)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = record;
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

ns_error_t ItemMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
		case MetadataKeys::URI:
			return ItemRecord_GetField(index, filename, value);
		case MetadataKeys::TITLE:
			return ItemRecord_GetField(index, title, value);
		case MetadataKeys::ALBUM:
			return ItemRecord_GetField(index, album, value);
		case MetadataKeys::ARTIST:
			return ItemRecord_GetField(index, artist, value);
		case MetadataKeys::COMMENT:
			return ItemRecord_GetField(index, comment, value);
		case MetadataKeys::GENRE:
			return ItemRecord_GetField(index, genre, value);
		case MetadataKeys::ALBUM_ARTIST:
			return ItemRecord_GetField(index, albumartist, value);
		case MetadataKeys::ALBUM_GAIN:
			return ItemRecord_GetField(index, replaygain_album_gain, value);
		case MetadataKeys::TRACK_GAIN:
			return ItemRecord_GetField(index, replaygain_track_gain, value);
		case MetadataKeys::PUBLISHER:
			return ItemRecord_GetField(index, publisher, value);
		case MetadataKeys::COMPOSER:
			return ItemRecord_GetField(index, composer, value);
		case MetadataKeys::CATEGORY:
			return ItemRecord_GetField(index, category, value);
		case MetadataKeys::YEAR:
			return ItemRecord_GetField(index, year, value);
		case MetadataKeys::PRODUCER:
			return ItemRecord_GetField(index, producer, value);
		case MetadataKeys::DIRECTOR:
			return ItemRecord_GetField(index, director, value);
		case MetadataKeys::TRACK:
			return ItemRecord_GetField(index, track, value);
		case MetadataKeys::MIME_TYPE:
			return ItemRecord_GetField(index, mime, value);
		case MetadataKeys::METAHASH:
			return ItemRecord_GetField(index, metahash, value);
	}
	return NErr_Unknown;
}

ns_error_t ItemMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	switch(field)
	{
		case MetadataKeys::TRACK:
			return ItemRecord_GetInteger(index, track, value);
		case MetadataKeys::TRACKS:
			return ItemRecord_GetInteger(index, tracks, value);
		case MetadataKeys::LENGTH:
		{
			// TODO will need to look to get a real millisecond value
			//		instead of a scaled up seconds*1000 value for this
			int ret = ItemRecord_GetInteger(index, length, value);
			return ret;
		}
		case MetadataKeys::RATING:
			return ItemRecord_GetInteger(index, rating, value);
		case MetadataKeys::PLAY_COUNT:
			return ItemRecord_GetInteger(index, playcount, value);
		case MetadataKeys::LAST_PLAY:
			return ItemRecord_GetInteger(index, lastplay, value);
		case MetadataKeys::LAST_UPDATE:
			return ItemRecord_GetInteger(index, lastupd, value);
		case MetadataKeys::FILE_TIME:
			return ItemRecord_GetInteger(index, filetime, value);
		case MetadataKeys::FILE_SIZE:
			return ItemRecord_GetInteger(index, filesize, value);
		case MetadataKeys::BITRATE:
		{
			int ret = ItemRecord_GetInteger(index, bitrate, value);
			if (ret == NErr_Success)
				*value *= 1000;
			return ret;
		}
		case MetadataKeys::DISC:
			return ItemRecord_GetInteger(index, disc, value);
		case MetadataKeys::DISCS:
			return ItemRecord_GetInteger(index, discs, value);
		case MetadataKeys::BPM:
			return ItemRecord_GetInteger(index, bpm, value);
		case MetadataKeys::ADDED:
			return ItemRecord_GetInteger(index, dateadded, value);
		case MetadataKeys::CLOUD:
			return ItemRecord_GetInteger(index, cloud, value);
	}
	return NErr_Unknown;
}

ns_error_t ItemMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	switch(field)
	{
		case MetadataKeys::LENGTH:
		{
			// TODO will need to look to get a real millisecond value
			//		instead of a scaled up seconds*1000 value for this
			int ret = ItemRecord_GetReal(index, length, value);
			return ret;
		}
	}
	return NErr_NotImplemented;
}

ns_error_t ItemMetadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	return NErr_NotImplemented;
}

ns_error_t ItemMetadata::Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)
{
	return NErr_NotImplemented;
}

ns_error_t ItemMetadata::Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata)
{
	return NErr_NotImplemented;
}

ns_error_t ItemMetadata::Metadata_Serialize(nx_data_t *data)
{
	return NErr_NotImplemented;
}

static ns_error_t ItemRecord_SetField(int index, nx_string_t &record, nx_string_t value)
{
	if (NXStringGetLength(value) > 0)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;

		record = NXStringRetain(value);
		return NErr_Success;
	}
	else
	{
		return NErr_Empty;
	}
}

static ns_error_t ItemRecord_SetInteger(int index, int *record, int64_t value)
{
	if (index > 0)
		return NErr_EndOfEnumeration;
	*record = (int)value;
	return NErr_Success;
}

static ns_error_t ItemRecord_SetInteger(int index, int64_t *record, int64_t value)
{
	if (index > 0)
		return NErr_EndOfEnumeration;
	*record = value;
	return NErr_Success;
}

static ns_error_t ItemRecord_SetReal(int index, int *record, double value)
{
	if (index > 0)
		return NErr_EndOfEnumeration;
	*record = (int)value;
	return NErr_Success;
}

ns_error_t ItemMetadata::Metadata_SetInteger(int field, unsigned int index, int64_t value)
{
	switch(field)
	{
		case MetadataKeys::YEAR:
			return ItemRecord_SetInteger(index, &year, value);
		case MetadataKeys::TRACK:
			return ItemRecord_SetInteger(index, &track, value);
		case MetadataKeys::TRACKS:
			return ItemRecord_SetInteger(index, &tracks, value);
		case MetadataKeys::LENGTH:
			return ItemRecord_SetInteger(index, &length, value);
		case MetadataKeys::RATING:
			return ItemRecord_SetInteger(index, &rating, value);
		case MetadataKeys::PLAY_COUNT:
			return ItemRecord_SetInteger(index, &playcount, value);
		case MetadataKeys::LAST_PLAY:
			return ItemRecord_SetInteger(index, &lastplay, value);
		case MetadataKeys::LAST_UPDATE:
			return ItemRecord_SetInteger(index, &lastupd, value);
		case MetadataKeys::FILE_TIME:
			return ItemRecord_SetInteger(index, &filetime, value);
		case MetadataKeys::FILE_SIZE:
			return ItemRecord_SetInteger(index, &filesize, value);
		case MetadataKeys::BITRATE:
		{
			if (value) value /= 1000;
			return ItemRecord_SetInteger(index, &bitrate, value);
		}
		case MetadataKeys::DISC:
			return ItemRecord_SetInteger(index, &disc, value);
		case MetadataKeys::DISCS:
			return ItemRecord_SetInteger(index, &discs, value);
		case MetadataKeys::BPM:
			return ItemRecord_SetInteger(index, &bpm, value);
		case MetadataKeys::ADDED:
			return ItemRecord_SetInteger(index, &dateadded, value);
		case MetadataKeys::CLOUD:
			return ItemRecord_SetInteger(index, &cloud, value);
	}
	return NErr_Unknown;
}

ns_error_t ItemMetadata::Metadata_SetField(int field, unsigned int index, nx_string_t value)
{
	switch(field)
	{
		case MetadataKeys::URI:
			return ItemRecord_SetField(index, filename, value);
		case MetadataKeys::TITLE:
			return ItemRecord_SetField(index, title, value);
		case MetadataKeys::ALBUM:
			return ItemRecord_SetField(index, album, value);
		case MetadataKeys::ARTIST:
			return ItemRecord_SetField(index, artist, value);
		case MetadataKeys::COMMENT:
			return ItemRecord_SetField(index, comment, value);
		case MetadataKeys::GENRE:
			return ItemRecord_SetField(index, genre, value);
		case MetadataKeys::ALBUM_ARTIST:
			return ItemRecord_SetField(index, albumartist, value);
		case MetadataKeys::ALBUM_GAIN:
			return ItemRecord_SetField(index, replaygain_album_gain, value);
		case MetadataKeys::TRACK_GAIN:
			return ItemRecord_SetField(index, replaygain_track_gain, value);
		case MetadataKeys::PUBLISHER:
			return ItemRecord_SetField(index, publisher, value);
		case MetadataKeys::COMPOSER:
			return ItemRecord_SetField(index, composer, value);
		case MetadataKeys::CATEGORY:
			return ItemRecord_SetField(index, category, value);
		case MetadataKeys::PRODUCER:
			return ItemRecord_SetField(index, producer, value);
		case MetadataKeys::DIRECTOR:
			return ItemRecord_SetField(index, director, value);
		case MetadataKeys::MIME_TYPE:
			return ItemRecord_SetField(index, mime, value);
		case MetadataKeys::METAHASH:
			return ItemRecord_SetField(index, metahash, value);
	}
	return NErr_Unknown;
}

ns_error_t ItemMetadata::Metadata_SetReal(int field, unsigned int index, double value)
{
	switch(field)
	{
		case MetadataKeys::LENGTH:
		{
			// TODO will need to look to get a real millisecond value
			//		instead of a scaled up seconds*1000 value for this
			int ret = ItemRecord_SetReal(index, &length, value);
			return ret;
		}
	}
	return NErr_NotImplemented;
}