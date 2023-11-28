#include "GioReplicant.h"
#include <stdlib.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

GioReplicant::GioReplicant()
{
	mpeg_length=0;
	mpeg_position=0;
	mpeg_samples=0;
	mpeg_duration=0;
	mpeg_frames=0;
}

GioReplicant::~GioReplicant()
{
	if (id3v2.tag)
		NSID3v2_Tag_Destroy(id3v2.tag);

	delete lame.tag;
	delete vbri.tag;
	delete ofl.tag;
}

double GioReplicant::GetLengthSeconds(double average_bitrate, ns_error_t *exact) const
{
	if (mpeg_duration)
	{
		if (exact)
			*exact=NErr_True;
		return mpeg_duration;
	}
	else if (average_bitrate)
	{
		if (exact)
			*exact=NErr_False;
		return mpeg_length*8.0/average_bitrate;
	}
	else
	{
		if (exact)
			*exact=NErr_False;
		return 0;
	}
}

int GioReplicant::GetBitrate(double *bitrate, ns_error_t *exact)
{
	if (mpeg_duration)
	{
		if (exact)
			*exact = NErr_True;
		*bitrate = 8.0 * (double)mpeg_length / (double)mpeg_duration;
		return NErr_Success;
	}

	return NErr_Unknown;
}

int GioReplicant::GetSeekPosition(double seconds, double average_bitrate, uint64_t *position)
{
	uint64_t offs;
	if (vbri.tag)
	{
		offs = vbri.tag->seekPointByTime(seconds*1000.0);
	}
	else if (lame.tag && lame.tag->Flag(TOC_FLAG))
	{
		double percent = seconds / GetLengthSeconds(average_bitrate);
		offs = lame.tag->GetSeekPoint(percent);
	}
	else 
	{
		offs = (uint64_t)(seconds*average_bitrate/8.0);
	}

	*position = offs + mpeg_position;
	return NErr_Success;
}


/* ifc_metadata */
int GioReplicant::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
	case MetadataKeys::MIME_TYPE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		return NXStringCreateWithUTF8(value, "audio/mpeg"); // TODO: singleton
	}
	
	return id3v2_metadata.Metadata_GetField(field, index, value);
}

int GioReplicant::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	int ret;
	bool known=false;

	switch(field)
	{
	case MetadataKeys::PREGAP:
		/* try LAME info first */
		if (lame.tag)
		{
			size_t pregap, postgap;
			ret = lame.tag->GetGaps(&pregap, &postgap);
			if (ret == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;
				*value = (int64_t)pregap + 529; 
				return NErr_Success;
			}
		}

		/* try VBRI next */
		if (vbri.tag)
		{
			/* TODO
			if (vbri_header->encoderDelay)
			{
			*pregap = vbri_header->encoderDelay + 529;
			}
			*/
		}

		/* then try FhG's OFL data */
		if (ofl.tag)
		{
			size_t pregap, postgap;
			if (ofl.tag->GetGaps(&pregap, &postgap) == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;

				*value = pregap-529;
				return NErr_Success;
			}
		}
		return NErr_Empty;

	case MetadataKeys::POSTGAP:
		/* try LAME info first */
		if (lame.tag)
		{
			size_t pregap, postgap;
			ret = lame.tag->GetGaps(&pregap, &postgap);
			if (ret == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;
				if (postgap < 529)
					*value = 0; // hmmm
				else
					*value = (int64_t)postgap - 529;
				return NErr_Success;
			}
		}

		/* try VBRI next */
		if (vbri.tag)
		{
			/* TODO
			if (vbri_header->encoderDelay)
			{
			}
			*/
		}

		/* then try FhG's OFL data */
		if (ofl.tag)
		{
			size_t pregap, postgap;
			if (ofl.tag->GetGaps(&pregap, &postgap) == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;

				*value = postgap+529;
				return NErr_Success;
			}
		}
		return NErr_Empty;
	}

	return id3v2_metadata.Metadata_GetInteger(field, index, value);
}

int GioReplicant::Metadata_GetReal(int field, unsigned int index, double *value)
{
	/* TODO: use lame_info for replay gain values, if present.  probably last in priority */
	switch(field)
	{
	case MetadataKeys::LENGTH:
		if (index > 0)
			return NErr_EndOfEnumeration;
		/* TODO: we might need to scan some of the file if there's no LAME/VBRI/OFL header */
		*value = GetLengthSeconds(128000);
		return NErr_Success;
	case MetadataKeys::BITRATE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		return GetBitrate(value);
	}

	return id3v2_metadata.Metadata_GetReal(field, index, value);
}

int GioReplicant::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	return id3v2_metadata.Metadata_GetArtwork(field, index, artwork, flags);
	
}
