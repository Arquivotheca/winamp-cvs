#include "MP4ALACDecoder.h"
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"

MP4ALACDecoder::MP4ALACDecoder()
{	
	decoder=0;
	mp4_file=0;
	mp4_track=mp4file_invalid_track_id;
	next_sample=1;
	max_sample_size = 0;
	bytes_per_frame=0;
	sample_buffer=0;

}

int MP4ALACDecoder::Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, alac_decoder_t decoder)
{
	this->mp4_file = mp4_file;
	mp4_file->Retain();
	this->mp4_track = mp4_track;
	this->decoder = decoder;

	/* get the maximum input size so we can make a buffer for it */
	if (mp4_file->Track_GetMaxSampleSize(mp4_track, &max_sample_size) != NErr_Success)
		return NErr_Error;

	/* allocate memory for it */
	sample_buffer = (uint8_t *)malloc(max_sample_size);
	if (!sample_buffer)
		return NErr_OutOfMemory;

		uint32_t sample_rate;
	uint8_t channels;
	uint8_t bits_per_sample;
	int ret = alac_get_information(decoder, &sample_rate, &channels, &bits_per_sample);
	if (ret != NErr_Success)
		return ret;

	bytes_per_frame = channels * 4;

	return NErr_Success;
}

MP4ALACDecoder::~MP4ALACDecoder()
{
	if (decoder)
		alac_destroy(decoder);
	free(sample_buffer);
	if (mp4_file)
		mp4_file->Release();
}


int MP4ALACDecoder::MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)
{
	uint32_t sample_rate;
	uint8_t channels;
	uint8_t bits_per_sample;
	int ret = alac_get_information(decoder, &sample_rate, &channels, &bits_per_sample);
	if (ret != NErr_Success)
		return ret;

	parameters->audio.sample_rate = (double)sample_rate;
	parameters->audio.format_type = nsaudio::format_type_pcm;
	parameters->audio.format_flags = nsaudio::FORMAT_FLAG_NONINTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->audio.bytes_per_sample = 4;
	parameters->audio.bits_per_sample = bits_per_sample;
	parameters->audio.number_of_channels = channels;
	
	return NErr_Success;
}

int MP4ALACDecoder::MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position)
{
	uint32_t sample_size=max_sample_size;

	ifc_mp4file::Timestamp timestamp;
	ifc_mp4file::Duration duration;
	ifc_mp4file::Duration offset;
	if (mp4_file->Sample_Read(mp4_track, next_sample++, &sample_buffer, &sample_size, &timestamp, &duration, &offset) == NErr_Success)
	{
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp, start_position);
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp+duration, end_position);

		size_t frames;
		int ret = alac_decode(decoder, sample_buffer, sample_size, (alac_buffer_t * const)output_buffer, &frames);
		if (ret != NErr_Success)
			return ret;

		*output_buffer_bytes = frames * bytes_per_frame;
		return NErr_Success;
	}
	else
		return NErr_EndOfFile;
}

int MP4ALACDecoder::MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number)
{
	next_sample = sample_number;
	return NErr_Success;
}

int MP4ALACDecoder::MP4AudioDecoder_SeekSeconds(double *seconds)
{
	ifc_mp4file::Duration duration;
	if (mp4_file->Track_ConvertToDuration(mp4_track, *seconds, &duration) != NErr_Success)
		return NErr_EndOfFile;

	ifc_mp4file::SampleID new_sample_id;
	if (mp4_file->Sample_GetFromDuration(mp4_track, duration, &new_sample_id) != NErr_Success)
		return NErr_Error;

	mp4_file->Track_ConvertFromTimestamp(mp4_track, duration, seconds);
	next_sample = new_sample_id;
	return NErr_Success;	
}

int MP4ALACDecoder::MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file)
{
	mp4_file->Release();
	mp4_file=new_file;
	mp4_file->Retain();
	return NErr_Success;
}
