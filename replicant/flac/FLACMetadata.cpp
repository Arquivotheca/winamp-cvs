#include "main.h"
#include "FLACMetadata.h"
#include "metadata/MetadataKeys.h"
#include "nswasabi/ReferenceCounted.h"

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

FLACMetadata::FLACMetadata()
{
	memset(&file_stats, 0, sizeof(file_stats));
	metadata_block=0;
	stream_info=0;
	own_data=false;
}

FLACMetadata::~FLACMetadata()
{
	if (own_data)
	{
		if (stream_info)
			FLAC__metadata_object_delete(stream_info);
		if (metadata_block)
			FLAC__metadata_object_delete(metadata_block);

		for (size_t i=0;i<pictures.size();i++)		
			FLAC__metadata_object_delete(pictures[i]);
	}
}

int FLACMetadata::OwnStreamInfo(FLAC__StreamMetadata *stream_info)
{
	if (this->stream_info)
		FLAC__metadata_object_delete(this->stream_info);
	this->stream_info = stream_info;
	own_data=true;

	return NErr_Success;
}

int FLACMetadata::OwnMetadataBlock(FLAC__StreamMetadata *metadata_block)
{
	if (this->metadata_block)
		FLAC__metadata_object_delete(this->metadata_block);
	this->metadata_block = metadata_block;
	own_data=true;

	return NErr_Success;
}

int FLACMetadata::OwnPicture(FLAC__StreamMetadata *picture)
{
	if (!picture)
		return NErr_OutOfMemory;
	own_data=true;
	return pictures.push_back(picture);	
}

void FLACMetadata::SetFileStats(nx_file_stat_t stats)
{
	file_stats = *stats;
}

int FLACMetadata::GetPosition(const char *tag, unsigned int index, int *out_pos)
{
	if (!metadata_block)
		return NErr_Empty;

	int pos = -1;

	for (unsigned int i=0;i<=index;i++)
	{
		pos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata_block, pos+1, tag);
		if (pos < 0)
		{
			*out_pos = pos;
			if (i == 0) /* failed on first find attempt */
				return NErr_Empty;
			else /* tag exists, just not this index */
				return NErr_EndOfEnumeration;
		}
	}
	*out_pos = pos;
	return NErr_Success;
}

int FLACMetadata::GetMetadata(const char *tag, unsigned int index, nx_string_t *value)
{
	if (!metadata_block)
		return NErr_Empty;

	int pos;
	int ret = GetPosition(tag, index, &pos);
	if (ret != NErr_Success)
		return ret;

	const char *entry = (const char *)metadata_block->data.vorbis_comment.comments[pos].entry;
	if (entry)
	{
		const char *metadata = strchr(entry, '='); // find the first equal
		if (metadata)
		{
			return NXStringCreateWithUTF8(value, metadata+1);
		}
	}
	return NErr_Malformed;
}

// only one of value1 or value2 should be non-NULL
static int SplitSlash(nx_string_t track, nx_string_t *value1, nx_string_t *value2)
{
	char track_utf8[64];
	size_t bytes_copied;
	int ret;
	ret = NXStringGetBytes(&bytes_copied, track, track_utf8, 64, nx_charset_utf8, nx_string_get_bytes_size_null_terminate);
	if (ret == NErr_Success)
	{
		size_t len = strcspn(track_utf8, "/");

		if (value2)
		{
			const char *second = &track_utf8[len];
			if (*second)
				second++;

			if (!*second)
				return NErr_Empty;

			return NXStringCreateWithUTF8(value2, second);
		}
		else
		{
			if (len == 0)
				return NErr_Empty;
			return NXStringCreateWithBytes(value1, track_utf8, len, nx_charset_utf8);
		}

		return NErr_Success;						
	}

	return ret;
}

int FLACMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	int ret;
	switch (field)
	{
	case MetadataKeys::TITLE:
		return GetMetadata("TITLE", index, value);
	case MetadataKeys::ARTIST:
		return GetMetadata("ARTIST", index, value);
	case MetadataKeys::ALBUM:
		return GetMetadata("ALBUM", index, value);
	case MetadataKeys::ALBUM_ARTIST:
		ret = GetMetadata("ALBUM ARTIST", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = GetMetadata("ALBUMARTIST", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = GetMetadata("ENSEMBLE", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		return NErr_Empty;
	case MetadataKeys::COMMENT:
		return GetMetadata("COMMENT", index, value);
	case MetadataKeys::COMPOSER:
		return GetMetadata("COMPOSER", index, value);
	case MetadataKeys::TRACK:
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("TRACKNUMBER", index, &track); 
			if (ret != NErr_Success && ret != NErr_EndOfEnumeration)
				ret = GetMetadata("TRACK", index, &track);

			if (ret == NErr_Success)
				return SplitSlash(track, value, 0);

			return ret;
		}
		break;

	case MetadataKeys::TRACKS:
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("TRACKNUMBER", index, &track); 
			if (ret != NErr_Success && ret != NErr_EndOfEnumeration)
				ret = GetMetadata("TRACK", index, &track);

			if (ret == NErr_Success)
				return SplitSlash(track, 0, value);

			return ret;
		}
		break;

	case MetadataKeys::TRACK_GAIN:
		return GetMetadata("REPLAYGAIN_TRACK_GAIN", index, value);
	case MetadataKeys::TRACK_PEAK:
		return GetMetadata("REPLAYGAIN_TRACK_PEAK", index, value);
	case MetadataKeys::ALBUM_GAIN:
		return GetMetadata("REPLAYGAIN_ALBUM_GAIN", index, value);
	case MetadataKeys::ALBUM_PEAK:
		return GetMetadata("REPLAYGAIN_ALBUM_PEAK", index, value);
	case MetadataKeys::YEAR:
		ret = GetMetadata("DATE", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = GetMetadata("YEAR", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		return NErr_Empty;
	case MetadataKeys::GENRE: 
		return GetMetadata("GENRE", index, value);
	case MetadataKeys::DISC:
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("DISC", index, &track); 
			if (ret == NErr_Success)
				return SplitSlash(track, value, 0);

			return ret;
		}
		break;

	case MetadataKeys::DISCS:
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("DISC", index, &track); 
			if (ret == NErr_Success)
				return SplitSlash(track, 0, value);

			return ret;
		}
		break;
	case MetadataKeys::BPM:
		return GetMetadata("BPM", index, value);
	case MetadataKeys::PUBLISHER:
		ret = GetMetadata("PUBLISHER", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = GetMetadata("ORGANIZATION", index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;
		return NErr_Empty;
	case MetadataKeys::MIME_TYPE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		return NXStringCreateWithUTF8(value, "audio/flac"); // TODO: singleton string
	}

	if (field == MetadataKey_GracenoteFileID)
		return GetMetadata("GracenoteFileID", index, value);
	else if (field == MetadataKey_GracenoteExtData)
		return GetMetadata("GracenoteExtData", index, value);

	return NErr_Unknown;
}

static int SplitSlashInteger(nx_string_t track, unsigned int *value1, unsigned int *value2)
{
	char track_utf8[64];
	size_t bytes_copied;
	int ret;
	ret = NXStringGetBytes(&bytes_copied, track, track_utf8, 64, nx_charset_utf8, nx_string_get_bytes_size_null_terminate);
	if (ret == NErr_Success)
	{
		size_t len = strcspn(track_utf8, "/");

		if (track_utf8[len])
			*value2 = strtoul(&track_utf8[len+1], 0, 10);
		else
			*value2 = 0;

		track_utf8[len]=0;
		*value1 = strtoul(track_utf8, 0, 10);

		return NErr_Success;						
	}

	return ret;
}

int FLACMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	switch(field)
	{
	case MetadataKeys::TRACK:
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("TRACKNUMBER", index, &track); 
			if (ret != NErr_Success && ret != NErr_EndOfEnumeration)
				ret = GetMetadata("TRACK", index, &track);

			if (ret == NErr_Success)
			{
				unsigned int itrack, itracks;
				ret = SplitSlashInteger(track, &itrack, &itracks);
				if (ret == NErr_Success)
				{
					if (itrack == 0)
						return NErr_Empty;
					*value = itrack;
					return NErr_Success;
				}
			}
			return ret;
		}
		break;
		{
			ReferenceCountedNXString track;

			int ret = GetMetadata("TRACKNUMBER", index, &track); 
			if (ret != NErr_Success && ret != NErr_EndOfEnumeration)
				ret = GetMetadata("TRACK", index, &track);

			if (ret == NErr_Success)
			{
				unsigned int itrack, itracks;
				ret = SplitSlashInteger(track, &itrack, &itracks);
				if (ret == NErr_Success)
				{
					if (itracks == 0)
						return NErr_Empty;
					*value = itracks;
					return NErr_Success;
				}
			}
			return ret;
		}
		break;

	case MetadataKeys::DISC:
		{
			ReferenceCountedNXString disc;
			int ret = GetMetadata("DISC", index, &disc);
			if (ret == NErr_Success)
			{
				unsigned int idisc, idiscs;
				ret = SplitSlashInteger(disc, &idisc, &idiscs);
				if (ret == NErr_Success)
				{
					if (idisc == 0)
						return NErr_Empty;
					*value = idisc;
					return NErr_Success;
				}
			}
			return ret;
		}
		break;

	case MetadataKeys::DISCS:
		{
			ReferenceCountedNXString disc;
			int ret = GetMetadata("DISC", index, &disc);
			if (ret == NErr_Success)
			{
				unsigned int idisc, idiscs;
				ret = SplitSlashInteger(disc, &idisc, &idiscs);
				if (ret == NErr_Success)
				{
					if (idiscs == 0)
						return NErr_Empty;
					*value = idiscs;
					return NErr_Success;
				}
			}
			return ret;
		}
		break;

	case MetadataKeys::BPM:
		{
			ReferenceCountedNXString bpm;
			int ret = GetMetadata("BPM", index, &bpm);
			if (ret == NErr_Success)
			{
				int value32;
				ret = NXStringGetIntegerValue(bpm, &value32);
				if (ret == NErr_Success)
				{
					if (value32 == 0)
						return NErr_Empty;

					*value = value32;
					return NErr_Success;
				}
			}
			return ret;
		}

	case MetadataKeys::FILE_SIZE:
		if (index > 0)
			return NErr_EndOfEnumeration;

		*value = file_stats.file_size;
		return NErr_Success;
	case MetadataKeys::FILE_TIME:
		if (index > 0)
			return NErr_EndOfEnumeration;

		*value = file_stats.modified_time;
		return NErr_Success;

	}
	return NErr_Unknown;
}


int FLACMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	int ret;
	nx_string_t str;
	switch (field)
	{
	case MetadataKeys::TRACK_GAIN:
		ret = GetMetadata("REPLAYGAIN_TRACK_GAIN", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::TRACK_PEAK:
		ret = GetMetadata("REPLAYGAIN_TRACK_PEAK", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_GAIN:
		ret = GetMetadata("REPLAYGAIN_ALBUM_GAIN", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_PEAK:
		ret = GetMetadata("REPLAYGAIN_ALBUM_PEAK", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::LENGTH:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = ((double)(FLAC__int64)stream_info->data.stream_info.total_samples / (double)stream_info->data.stream_info.sample_rate);
		return NErr_Success;
	case MetadataKeys::BITRATE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		if (file_stats.file_size == 0)
			return NErr_Unknown;
		*value = 8.0 * (double)file_stats.file_size / ((double)stream_info->data.stream_info.total_samples / (double)stream_info->data.stream_info.sample_rate);
		return NErr_Success;
	}

	return NErr_Unknown;
}

int FLACMetadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;
	int ret;

	for (size_t i=0;i<pictures.size();i++)
	{
		FLAC__StreamMetadata_Picture &picture = pictures[i]->data.picture;
		if (picture.type == 3) /* TODO */
		{
			if (artwork)
			{
				nx_data_t data=0;
				if (flags != DATA_FLAG_NONE)
				{
					if (TestFlag(flags, DATA_FLAG_DATA))
					{
						ret = NXDataCreate(&data, picture.data, picture.data_length);
						if (ret != NErr_Success)
							return ret;
					}
					else
					{
						ret = NXDataCreateEmpty(&data);
						if (ret != NErr_Success)
							return ret;
					}

					if (TestFlag(flags, DATA_FLAG_MIME))
					{
						ReferenceCountedNXString mime_type;
						ret = NXStringCreateWithUTF8(&mime_type, picture.mime_type);
						if (ret == NErr_Success || ret == NErr_Empty)
						{
							if (ret  == NErr_Success)
							{
								ret = NXDataSetMIME(data, mime_type);
								if (ret != NErr_Success)
								{
									NXDataRelease(data);
									return ret;
								}
							}
						}
						else
						{
							NXDataRelease(data);
							return ret;
						}
					}

					if (TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
					{
						ReferenceCountedNXString filename;
						ret = this->GetField(MetadataKeys::URI, 0, &filename);
						if (ret == NErr_Success)
						{
							ReferenceCountedNXURI uri;
							NXURICreateWithNXString(&uri, filename);
							ret = NXDataSetSourceURI(data, uri);
							if (ret != NErr_Success)
							{
								NXDataRelease(data);
								return ret;
							}

							ret = NXDataSetSourceStat(data, &file_stats);
							if (ret != NErr_Success)
							{
								NXDataRelease(data);
								return ret;
							}
						}
					}

					if (TestFlag(flags, DATA_FLAG_DESCRIPTION))
					{
						ReferenceCountedNXString description;
						ret = NXStringCreateWithUTF8(&description, (const char *)picture.description);
						if (ret == NErr_Success || ret == NErr_Empty)
						{
							if (ret == NErr_Success)
							{
								ret = NXDataSetDescription(data, description);
								if (ret != NErr_Success)
								{
									NXDataRelease(data);
									return ret;
								}	
							}						
						}
						else
						{
							NXDataRelease(data);
							return ret;
						}
					}
				}
				artwork->data = data;
				artwork->width = picture.width;
				artwork->height = picture.height;
			}
			return NErr_Success;
		}
	}
	return NErr_Empty;
}