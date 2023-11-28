#include "MP4AACDecoder.h"
#include "nswasabi/ReferenceCounted.h"
#include <android/log.h>
#include <stdlib.h>
#include <new>

int MP4AACDecoder::CreateDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder)
{
	uint8_t *buffer = NULL;
	uint32_t buffer_size = 0;
	uint8_t *sample_buffer = 0;
	int ret;

	/* read the decoder configuration data */
	if (mp4_file->Track_GetESConfiguration(mp4_track, &buffer, &buffer_size) != NErr_Success || !buffer)
		return NErr_WrongFormat;

	/* create our C++ object */
	MP4AACDecoder *mp4_decoder = new (std::nothrow) ReferenceCounted<MP4AACDecoder>;
	if (!mp4_decoder)
	{
		mp4_file->Free(buffer);
		return NErr_OutOfMemory;
	}

	ret = mp4_decoder->ConfigureDecoder(buffer, buffer_size);
	if (ret != NErr_Success)
	{
		mp4_file->Free(buffer);
		mp4_decoder->Release();
		return ret;
	}

	ret = mp4_decoder->Initialize(mp4_file, mp4_track);
	if (ret != NErr_Success)
	{
		mp4_file->Free(buffer);
		mp4_decoder->Release();
		return ret;
	}

	mp4_file->Free(buffer);
	*out_decoder = mp4_decoder;
	return NErr_Success;
}


MP4AACDecoder::MP4AACDecoder()
{
	decoder=0;
	mp4_file=0;
	mp4_track=mp4file_invalid_track_id;
	next_sample=1;
	sample_buffer = 0;
	max_sample_size = 0;
}

MP4AACDecoder::~MP4AACDecoder()
{
	free(sample_buffer);
	if (mp4_file)
		mp4_file->Release();
	delete decoder;
}

int MP4AACDecoder::ConfigureDecoder(const uint8_t *asc, size_t asc_length)
{
	int ret = AACDecoder_Create(&decoder, TYPE_RAW_AAC);
	if (ret != NErr_Success)
		return ret;

	ret = decoder->Configure(asc, asc_length);
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}

int MP4AACDecoder::Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track)
{
	this->mp4_file = mp4_file;
	mp4_file->Retain();
	this->mp4_track=mp4_track;

	if (mp4_file->Track_GetMaxSampleSize(mp4_track, &max_sample_size) != NErr_Success)
		return NErr_Error;

	sample_buffer = (uint8_t *)malloc(max_sample_size);
	if (!sample_buffer)
		return NErr_OutOfMemory;

	return NErr_Success;
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

int MP4AACDecoder::MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)
{
	decoder->FillParameters(&parameters->audio);

	/* try to read gapless data. TODO: move to MP4Metadata and read from there? */
	ifc_mp4file::metadata_itunes_atom_t gapless_atom;
	int ret = mp4_file->Metadata_iTunes_FindFreeform("iTunSMPB", 0, &gapless_atom);
	if (ret == NErr_Success)
	{
		const uint8_t *value;
		size_t value_length;
		/* to make our lives easier, we'll get this as binary */
		ret = mp4_file->Metadata_iTunes_GetBinary(gapless_atom, &value, &value_length);
		if (ret == NErr_Success)
		{
			char temp[9];

			/* skip first set of meaningless values */
			if (IncSafe(value, value_length, 8) == NErr_Success && value_length >= 8)
			{
				/* read pre-gap */
				memcpy(temp, value, 8);
				temp[8]=0;
				parameters->frames_trim_start = strtoul(temp, 0, 16);
				
				if (IncSafe(value, value_length, 8) == NErr_Success && value_length >= 8)
				{
					memcpy(temp, value, 8);
					temp[8]=0;
					parameters->frames_trim_end = strtoul(temp, 0, 16);
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] pre-gap = %u, post-gap = %u", parameters->frames_trim_start, parameters->frames_trim_end);
				}
				else
				{
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] pre-gap = %u", parameters->frames_trim_start);
				}
			}			
		}
	}

	return NErr_Success;
}

int MP4AACDecoder::MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position)
{
	uint32_t sample_size=max_sample_size;
	ifc_mp4file::Timestamp timestamp;
	ifc_mp4file::Duration duration;
	ifc_mp4file::Duration offset;
	if (mp4_file->Sample_Read(mp4_track, next_sample++, &sample_buffer, &sample_size, &timestamp, &duration, &offset) == NErr_Success)
	{
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp, start_position);
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp+duration, end_position);
		
		size_t samples_decoded;
		if (decoder->Decode(sample_buffer, sample_size, decoder_output, &samples_decoded) == NErr_Success)		
		{
			*output_buffer = decoder_output;
			*output_buffer_bytes = samples_decoded*sizeof(int16_t);
			return NErr_Success;
		}
		else
		{
			return NErr_Error;
		}

	}
	else
		return NErr_EndOfFile;
}

int MP4AACDecoder::MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number)
{
	next_sample = sample_number;
	decoder->Reset();
	return NErr_Success;
}

int MP4AACDecoder::MP4AudioDecoder_SeekSeconds(double *seconds)
{
	ifc_mp4file::Duration duration;
	if (mp4_file->Track_ConvertToDuration(mp4_track, *seconds, &duration) != NErr_Success)
		return NErr_EndOfFile;

	ifc_mp4file::SampleID new_sample_id;
	if (mp4_file->Sample_GetFromDuration(mp4_track, duration, &new_sample_id) != NErr_Success)
		return NErr_Error;

	mp4_file->Track_ConvertFromTimestamp(mp4_track, duration, seconds);
	next_sample = new_sample_id;
	decoder->Reset();
	return NErr_Success;	
}

int MP4AACDecoder::MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file)
{
	mp4_file->Release();
	mp4_file=new_file;
	mp4_file->Retain();
	return NErr_Success;
}