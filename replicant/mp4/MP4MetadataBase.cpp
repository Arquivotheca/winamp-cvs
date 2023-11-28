#include "api.h"
#include "main.h"
#include "MP4MetadataBase.h"
#include "metadata/MetadataKeys.h"
#include "nsmp4.h"
#include "nu/ByteReader.h"
#include "nswasabi/ReferenceCounted.h"
#include "MP4MetadataService.h"
static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

MP4MetadataBase::mime_types_t MP4MetadataBase::mime_types = {0,0,0,0};

MP4MetadataBase::MP4MetadataBase()
{
	mp4_file=0;
	mp4_metadata_filename=0;
}

MP4MetadataBase::~MP4MetadataBase()
{
	NXURIRelease(mp4_metadata_filename);
}

int MP4MetadataBase::InitMIME(nx_once_t, void *, void **)
{
	NXStringCreateWithUTF8(&mime_types.bmp,  "image/bmp");
	NXStringCreateWithUTF8(&mime_types.gif,  "image/gif");
	NXStringCreateWithUTF8(&mime_types.jpeg, "image/jpeg");
	NXStringCreateWithUTF8(&mime_types.png,  "image/png");

	return 1;
}


int MP4MetadataBase::Initialize(nx_uri_t filename, MP4FileHandle mp4_file)
{
	NXOnce(&(MP4MetadataService::mime_once), InitMIME, 0);
	NXURIRelease(mp4_metadata_filename);
	mp4_metadata_filename = NXURIRetain(filename);
	this->mp4_file = mp4_file;
	return MP4GetStat(mp4_file, &file_stats);
}

static int Metadata_iTunes_GetUnsignedFromKey(MP4FileHandle mp4_file, unsigned int index, const char *field, uint64_t *value)
{
	nsmp4_metadata_itunes_atom_t atom;

	int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, field, index, &atom);
	if (ret == NErr_EndOfEnumeration)
	{
		if (index > 0 && NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, field, index-1, &atom) == NErr_Success)
			return NErr_EndOfEnumeration;
		else
			return NErr_Empty;
	}
	else if (ret != NErr_Success)
		return ret;

	return NSMP4_Metadata_iTunes_GetUnsigned(mp4_file, atom, value);
}

static int Metadata_iTunes_GetStringFromKey(MP4FileHandle mp4_file, unsigned int index, const char *field, nx_string_t *value)
{
	nsmp4_metadata_itunes_atom_t atom;

	int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, field, index, &atom);
	if (ret == NErr_EndOfEnumeration)
	{
		if (index > 0 && NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, field, index-1, &atom) == NErr_Success)
			return NErr_EndOfEnumeration;
		else
			return NErr_Empty;
	}
	else if (ret != NErr_Success)
		return ret;

	return NSMP4_Metadata_iTunes_GetString(mp4_file, atom, value);
}

static int Metadata_iTunes_GetStringFromFreeform(MP4FileHandle mp4_file, unsigned int index, const char *name, const char *mean, nx_string_t *value)
{
	nsmp4_metadata_itunes_atom_t atom;

	int ret = NSMP4_Metadata_iTunes_FindFreeform(mp4_file, name, mean, &atom);
	if (ret != NErr_Success)
		return ret;

	if (index != 0)
		return NErr_EndOfEnumeration; 

	return NSMP4_Metadata_iTunes_GetString(mp4_file, atom, value);
}

int Metadata_iTunes_GetSet(MP4FileHandle mp4_file, unsigned int index, const char *name, uint16_t *item, uint16_t *items)
{
	nsmp4_metadata_itunes_atom_t atom;

	int ret = NSMP4_Metadata_iTunes_FindKey(mp4_file, name, &atom);
	if (ret != NErr_Success)
		return ret;

	if (index != 0)
		return NErr_EndOfEnumeration; 

	const uint8_t *value;
	size_t value_length;
	ret = NSMP4_Metadata_iTunes_GetBinary(mp4_file, atom, &value, &value_length);
	if (ret != NErr_Success)
		return ret;

	if (value_length < 6)
		return NErr_Malformed;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, value, value_length);

	bytereader_advance(&byte_reader, 2);
	*item=bytereader_read_u16_be(&byte_reader);
	*items=bytereader_read_u16_be(&byte_reader);
	return NErr_Success;
}

int MP4MetadataBase::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{	

	switch(field)
	{

	case MetadataKeys::ARTIST:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_artist, value);

	case MetadataKeys::ALBUM_ARTIST:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_albumartist, value);

	case MetadataKeys::ALBUM:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_album, value);

	case MetadataKeys::TITLE:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_name, value);

	case MetadataKeys::GENRE:
		{
			int ret = Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_genre, value);
			if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
				return ret;

			uint64_t numeric_genre;
			ret = Metadata_iTunes_GetUnsignedFromKey(mp4_file, index, nsmp4_metadata_itunes_genre_numeric, &numeric_genre);
			if (numeric_genre >= 0xFF)
				return NErr_Error;

			ret = REPLICANT_API_METADATA->GetGenre((uint8_t)numeric_genre, value);
			if (ret == NErr_Success)
			{
				NXStringRetain(*value);
				return NErr_Success;
			}
			else if (ret == NErr_Unknown)
			{
				return NErr_Empty;
			}
			else
				return ret;
		}

	case MetadataKeys::YEAR:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_year, value);

	case MetadataKeys::TRACK:
		{
			uint16_t track, tracks;
			int ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &track, &tracks);
			if (ret != NErr_Success)
				return ret;

			if (!track)
				return NErr_Empty;

			return NXStringCreateWithUInt64(value, track);
		}

			case MetadataKeys::TRACKS:
		{
			uint16_t track, tracks;
			int ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &track, &tracks);
			if (ret != NErr_Success)
				return ret;

			if (!tracks)
				return NErr_Empty;

			return NXStringCreateWithUInt64(value, tracks);
		}

	case MetadataKeys::DISC:
		{
			uint16_t disc, discs;
			int ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &disc, &discs);
			if (ret != NErr_Success)
				return ret;

			if (!disc)
				return NErr_Empty;

			return NXStringCreateWithUInt64(value, disc);
		}

	case MetadataKeys::DISCS:
		{
			uint16_t disc, discs;
			int ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &disc, &discs);
			if (ret != NErr_Success)
				return ret;

			if (!discs)
				return NErr_Empty;

			return NXStringCreateWithUInt64(value, discs);
		}

	case MetadataKeys::COMPOSER:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_writer, value);

	case MetadataKeys::PUBLISHER:
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "publisher", "com.nullsoft.winamp", value);

	case MetadataKeys::BPM:
		{
			uint64_t tempo;
			int ret = Metadata_iTunes_GetUnsignedFromKey(mp4_file, index, nsmp4_metadata_itunes_tempo, &tempo);
			if (ret != NErr_Success)
				return ret;

			if (tempo == 0)
				return NErr_Empty;

			return NXStringCreateWithUInt64(value, tempo);
		}

	case MetadataKeys::COMMENT:
		return Metadata_iTunes_GetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_comment, value);

	case MetadataKeys::MIME_TYPE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		// TODO: actually inspect and figure out if it's audio or video.  and maybe even what codecs are in it
		return NXStringCreateWithUTF8(value, "audio/mp4"); // TODO: singleton string

	case MetadataKeys::TRACK_GAIN:
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_track_gain", 0, value);

	case MetadataKeys::TRACK_PEAK:
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_track_peak", 0, value);

	case MetadataKeys::ALBUM_GAIN:
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_album_gain", 0, value);

	case MetadataKeys::ALBUM_PEAK:
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_album_peak", 0, value);		
	}

	if (field == MetadataKey_GracenoteFileID)
	{
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "gnid", 0, value);
	}
	else if (field == MetadataKey_GracenoteExtData)
	{
		return Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "gnxd", 0, value);
	}

	return NErr_Unknown;
}

static int IncSafe(const uint8_t *&value, size_t &value_length, size_t increment_length)
{
	/* eat leading spaces */
	while (*value == ' ' && value_length) 
	{
		value++;
		value_length--;
	}

	if (increment_length > value_length)
		return NErr_NeedMoreData;

	value += increment_length;
	value_length -= increment_length;
	/* eat trailing spaces */
	while (*value == ' ' && value_length) 
	{
		value++;
		value_length--;
	}

	return NErr_Success;
}

int MP4MetadataBase::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	int ret;
	uint16_t track, tracks;
	uint16_t disc, discs;

	switch(field)
	{
	case MetadataKeys::TRACK:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &track, &tracks);
		if (ret != NErr_Success)
			return ret;

		if (!track)
			return NErr_Empty;
		else
		{
			*value = (int64_t)track;
			return NErr_Success;
		}

	case MetadataKeys::TRACKS:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &track, &tracks);
		if (ret != NErr_Success)
			return ret;


		if (!tracks)
			return NErr_Empty;
		else
		{
			*value = (int64_t)tracks;
			return NErr_Success;
		}

	case MetadataKeys::DISC:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &disc, &discs);
		if (ret != NErr_Success)
			return ret;

		if (!disc)
			return NErr_Empty;
		else
		{
			*value = (int64_t)disc;
			return NErr_Success;
		}		

	case MetadataKeys::DISCS:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &disc, &discs);
		if (ret != NErr_Success)
			return ret;

		if (!discs)
			return NErr_Empty;
		else
		{
			*value = (int64_t)discs;
			return NErr_Success;
		}

	case MetadataKeys::BPM:
		{
			uint64_t tempo;
			ret = Metadata_iTunes_GetUnsignedFromKey(mp4_file, index, nsmp4_metadata_itunes_tempo, &tempo);
			if (ret != NErr_Success)
				return ret;

			if (tempo == 0)
				return NErr_Empty;

			*value = (int64_t)tempo;
			return NErr_Success;
		}
	case MetadataKeys::PREGAP:
		{
			nsmp4_metadata_itunes_atom_t gapless_atom;
			int ret = NSMP4_Metadata_iTunes_FindFreeform(mp4_file, "iTunSMPB", 0, &gapless_atom);
			if (ret != NErr_Success)
				return ret;

			const uint8_t *smpb_value;
			size_t value_length;
			/* to make our lives easier, we'll get this as binary */
			ret = NSMP4_Metadata_iTunes_GetBinary(mp4_file, gapless_atom, &smpb_value, &value_length);
			if (ret != NErr_Success)
				return ret;

			char temp[9];

			/* skip first set of meaningless values */
			if (IncSafe(smpb_value, value_length, 8) == NErr_Success && value_length >= 8)
			{
				/* read pre-gap */
				memcpy(temp, smpb_value, 8);
				temp[8]=0;
				*value = strtoul(temp, 0, 16);
				return NErr_Success;
			}
			else
				return NErr_Malformed;
		}
	case MetadataKeys::POSTGAP:
		{
			nsmp4_metadata_itunes_atom_t gapless_atom;
			int ret = NSMP4_Metadata_iTunes_FindFreeform(mp4_file, "iTunSMPB", 0, &gapless_atom);
			if (ret != NErr_Success)
				return ret;

			const uint8_t *smpb_value;
			size_t value_length;
			/* to make our lives easier, we'll get this as binary */
			ret = NSMP4_Metadata_iTunes_GetBinary(mp4_file, gapless_atom, &smpb_value, &value_length);
			if (ret != NErr_Success)
				return ret;

			char temp[9];

			/* skip first set of meaningless values */
			if (IncSafe(smpb_value, value_length, 8) == NErr_Success && value_length >= 8
				&& IncSafe(smpb_value, value_length, 8) == NErr_Success && value_length >= 8
				)
			{
				/* read post-gap */
				memcpy(temp, smpb_value, 8);
				temp[8]=0;
				*value = strtoul(temp, 0, 16);
				return NErr_Success;
			}
			else
				return NErr_Malformed;
		}
	}

	return NErr_Unknown;
}


int MP4MetadataBase::Metadata_GetReal(int field, unsigned int index, double *value)
{
	int ret;
	nx_string_t str;

	switch (field)
	{
	case MetadataKeys::BITRATE:
		if (index > 0)
		{
			return NErr_EndOfEnumeration;
		}
		else
		{
			/* benski> for now, this only does audio bitrate 
			in fact, it sums all audio tracks.  for things like HD-AAC, this makes sense.
			but for video files with multiple streams, it might not
			we could make this be more sophisticated, and check the presentation flag and dependency info */
			double bitrate=0;
			uint32_t numTracks = MP4GetNumberOfTracks(mp4_file, MP4_AUDIO_TRACK_TYPE, 0);
			for (int i = 0; i < numTracks; i++)
			{
				MP4TrackId trackId = MP4FindTrackId(mp4_file, i, MP4_AUDIO_TRACK_TYPE, 0);
				if (trackId != MP4_INVALID_TRACK_ID)
					bitrate += MP4GetTrackBitRate(mp4_file, trackId);				
			}
			*value = bitrate;
			return NErr_Success;
		}

	case MetadataKeys::LENGTH:
		{
			/* TODO: use sample rate and number of samples from iTunSMPB to get a more exact length */
			uint32_t timescale = MP4GetTimeScale(mp4_file);
			if (!timescale)
				return NErr_Error;
			*value = (double)MP4GetDuration(mp4_file) / (double)timescale;
			return NErr_Success;				
		}		

	case MetadataKeys::TRACK_GAIN:
		ret = Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_track_gain", 0, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::TRACK_PEAK:
		ret = Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_track_peak", 0, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_GAIN:
		ret = Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_album_gain", 0, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_PEAK:
		ret = Metadata_iTunes_GetStringFromFreeform(mp4_file, index, "replaygain_album_peak", 0, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	}

	return NErr_Unknown;
}

nx_string_t MP4MetadataBase::GetMIMEFromType(uint32_t type)
{
	switch(type)
	{
	case nsmp4_metadata_itunes_type_gif:
		return mime_types.gif;
	case nsmp4_metadata_itunes_type_jpeg:
		return mime_types.jpeg;
	case nsmp4_metadata_itunes_type_png:
		return mime_types.png;
	case nsmp4_metadata_itunes_type_bmp:
		return mime_types.bmp;
	default: // assume JPEG
		return mime_types.jpeg;
	}
}

int MP4MetadataBase::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;

	nsmp4_metadata_itunes_atom_t atom;
	int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, nsmp4_metadata_itunes_cover_art, index, &atom);

	/* NSMP4_Metadata_iTunes_EnumerateKey doesn't check that index-1 was valid before returning NErr_EndOfEnumeration, so we need to check ourselves */
	if (ret == NErr_EndOfEnumeration)
	{
		if (index > 0 && NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, nsmp4_metadata_itunes_cover_art, index-1, &atom) == NErr_Success)
			return NErr_EndOfEnumeration;
		else
			return NErr_Empty;
	}
	else if (ret == NErr_Success)
	{
		if (artwork)
		{
			nx_data_t data=0;
			if (flags != DATA_FLAG_NONE)
			{

				if (TestFlag(flags, DATA_FLAG_DATA))
				{
					const uint8_t *art_data=0;
					size_t art_length=0;
					ret = NSMP4_Metadata_iTunes_GetBinary(mp4_file, atom, &art_data, &art_length);
					if (ret != NErr_Success)
						return ret;

					ret = NXDataCreate(&data, art_data, art_length);
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
					uint32_t type;
					ret = NSMP4_Metadata_iTunes_GetInformation(mp4_file, atom, 0, &type);
					if (ret != NErr_Success)
						type=nsmp4_metadata_itunes_type_binary;

					nx_string_t mime_type = GetMIMEFromType(type);
					if (mime_type)
					{
						ret = NXDataSetMIME(data, mime_type);
						if (ret != NErr_Success)
						{
							NXDataRelease(data);
							return ret;
						}
					}
				}

				if (TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
				{				
					ret = NXDataSetSourceURI(data, mp4_metadata_filename);
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
			artwork->width=0;
			artwork->height=0;
			artwork->data=data;
		}
		return NErr_Success;
	}
	else
	{
		return ret;
	}
}
