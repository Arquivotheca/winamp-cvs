#include "api.h"
#include "ItemRecordMetadata.h"
#include "metadata/MetadataKeys.h"

static nx_string_t ndestring_get_string(const wchar_t *str)
{
	if (!str)
		return 0;
	nx_string_t self = (nx_string_t)((uint8_t *)str - sizeof(size_t) - sizeof(size_t));
	self->len = wcslen(str);
	return self;
}

static ns_error_t ItemRecord_GetField(int index, const wchar_t *record, nx_string_t *value)
{
	if (record && record[0])
	{
		if (index > 0)
			return NErr_EndOfEnumeration;
		nx_string_t v = ndestring_get_string(record);
		*value = NXStringRetain(v);		
		return NErr_Success;		
	}
	else
	{
		return NErr_Empty;
	}
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

static ns_error_t ItemRecord_GetInteger(int index, int record, int64_t *value)
{
	if (record > 0)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;
		if (record)
		{
			*value = record;
			return NErr_Success;
		}
		else
			return NErr_Empty;
	}
	else
		return NErr_Empty;
}

static ns_error_t ItemRecord_GetInteger(int index, int64_t record, int64_t *value)
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

ns_error_t ItemRecordMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
		case MetadataKeys::URI:
			return ItemRecord_GetField(index, record->filename, value);
		case MetadataKeys::TITLE:
			return ItemRecord_GetField(index, record->title, value);
		case MetadataKeys::ALBUM:
			return ItemRecord_GetField(index, record->album, value);
		case MetadataKeys::ARTIST:
			return ItemRecord_GetField(index, record->artist, value);
		case MetadataKeys::COMMENT:
			return ItemRecord_GetField(index, record->comment, value);
		case MetadataKeys::GENRE:
			return ItemRecord_GetField(index, record->genre, value);
		case MetadataKeys::ALBUM_ARTIST:
			return ItemRecord_GetField(index, record->albumartist, value);
		case MetadataKeys::ALBUM_GAIN:
			return ItemRecord_GetField(index, record->replaygain_album_gain, value);
		case MetadataKeys::TRACK_GAIN:
			return ItemRecord_GetField(index, record->replaygain_track_gain, value);
		case MetadataKeys::PUBLISHER:
			return ItemRecord_GetField(index, record->publisher, value);
		case MetadataKeys::COMPOSER:
			return ItemRecord_GetField(index, record->composer, value);
		case MetadataKeys::CATEGORY:
			return ItemRecord_GetField(index, record->category, value);
		case MetadataKeys::YEAR:
		{
			ns_error_t ret = ItemRecord_GetField(index, record->year, value);
			// clamp year to be YYYY so anything less will be ignored
			if (ret == NErr_Success)
			{
				size_t byte_count = 0;
				if (NXStringGetBytesSize(&byte_count, *value, nx_charset_utf8, 0) != NErr_Error && byte_count != 4)
				{
					return NErr_Empty;
				}
			}
			return ret;
		}
		case MetadataKeys::PRODUCER:
			return ItemRecord_GetField(index, getRecordExtendedItem(record, L"producer"), value);
		case MetadataKeys::DIRECTOR:
			return ItemRecord_GetField(index, getRecordExtendedItem(record, L"director"), value);
		case MetadataKeys::TRACK:
			return ItemRecord_GetField(index, record->track, value);
		case MetadataKeys::MIME_TYPE:
		{
			if (index > 0)	
				return NErr_EndOfEnumeration;
			int ret = ItemRecord_GetField(index, getRecordExtendedItem(record, L"mimetype"), value);
			if (index == 0 && ret != NErr_Success)
			{
				wchar_t mime_metadata[256];
				if (AGAVE_API_METADATA->GetExtendedFileInfo(record->filename, L"mime", mime_metadata, 256) && mime_metadata[0])
				{
					return NXStringCreateWithUTF16(value, mime_metadata);
				}
			}
			return ret;
		}
		case MetadataKeys::METAHASH:
			return ItemRecord_GetField(index, getRecordExtendedItem(record, L"metahash"), value);
	}
	return NErr_Unknown;
}

ns_error_t ItemRecordMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	switch(field)
	{
		case MetadataKeys::TRACK:
			return ItemRecord_GetInteger(index, record->track, value);
		case MetadataKeys::TRACKS:
			return ItemRecord_GetInteger(index, record->tracks, value);
		case MetadataKeys::LENGTH:
		{
			// TODO will need to look to get a real millisecond value
			//		instead of a scaled up seconds*1000 value for this
			int ret = ItemRecord_GetInteger(index, record->length, value);
			return ret;
		}
		case MetadataKeys::RATING:
			return ItemRecord_GetInteger(index, record->rating, value);
		case MetadataKeys::PLAY_COUNT:
			return ItemRecord_GetInteger(index, record->playcount, value);
		case MetadataKeys::LAST_PLAY:
			return ItemRecord_GetInteger(index, record->lastplay, value);
		case MetadataKeys::LAST_UPDATE:
			return ItemRecord_GetInteger(index, record->lastupd, value);
		case MetadataKeys::FILE_TIME:
			return ItemRecord_GetInteger(index, record->filetime, value);
		case MetadataKeys::FILE_SIZE:
		{
			// not as clean as i'd like but it prefers 'realsize' over 'filesize'
			// on newer clients or when it's not provided since we cannot re-size
			// 'filesize' to be 64-bit without breaking compatibility in plug-ins
			int ret;
			wchar_t *realsize = getRecordExtendedItem(record, L"realsize");
			if (realsize && *realsize)
			{
				__int64 val = _wtoi64(realsize);
				ret = ItemRecord_GetInteger(index, val, value);
				if (ret == NErr_Success)
				{
					return ret;
				}
			}
			ret = ItemRecord_GetInteger(index, record->filesize, value);
			if (ret == NErr_Success)
				*value *= 1024;
			return ret;
		}
		case MetadataKeys::BITRATE:
		{
			int ret = ItemRecord_GetInteger(index, record->bitrate, value);
			if (ret == NErr_Success)
				*value *= 1000;
			return ret;
		}
		case MetadataKeys::DISC:
			return ItemRecord_GetInteger(index, record->disc, value);
		case MetadataKeys::DISCS:
			return ItemRecord_GetInteger(index, record->discs, value);
		case MetadataKeys::BPM:
			return ItemRecord_GetInteger(index, record->bpm, value);
		case MetadataKeys::ADDED:
		{
			wchar_t *added = getRecordExtendedItem(record, L"dateadded");
			if (added && *added)
			{
				__int64 val = _wtoi64(added);
				int ret = ItemRecord_GetInteger(index, val, value);
				if (ret == NErr_Success)
				{
					return ret;
				}
			}
			return NErr_Unknown;
		}
		case MetadataKeys::CLOUD:
		{
			wchar_t *cloud = getRecordExtendedItem(record, L"cloud");
			if (cloud && *cloud)
			{
				__int64 val = _wtoi64(cloud);
				int ret = ItemRecord_GetInteger(index, val, value);
				if (ret == NErr_Success)
				{
					return ret;
				}
			}
			return NErr_Unknown;
		}
		/* TODO case MetadataKeys::LOSSLESS:
			return ItemRecord_GetInteger(index, getRecordExtendedItem(record, L"lossless"), value); */
	}
	return NErr_Unknown;
}

ns_error_t ItemRecordMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	switch(field)
	{
		case MetadataKeys::LENGTH:
		{
			// TODO will need to look to get a real millisecond value
			//		instead of a scaled up seconds*1000 value for this
			int ret = ItemRecord_GetReal(index, record->length, value);
			return ret;
		}
	}
	return NErr_NotImplemented;
}

ns_error_t ItemRecordMetadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	return NErr_NotImplemented;
}

ns_error_t ItemRecordMetadata::Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)
{
	return NErr_NotImplemented;
}

ns_error_t ItemRecordMetadata::Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata)
{
	return NErr_NotImplemented;
}

ns_error_t ItemRecordMetadata::Metadata_Serialize(nx_data_t *data)
{
	return NErr_NotImplemented;
}