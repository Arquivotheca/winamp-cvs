#include "MP4MP3Decoder.h"
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"

int MP4MP3Decoder::CreateDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder)
{
	uint8_t *buffer = NULL;
	uint32_t buffer_size = 0;
	uint8_t *sample_buffer = 0;

	/* get the maximum input size so we can make a buffer for it */
	uint32_t max_sample_size;
	if (mp4_file->Track_GetMaxSampleSize(mp4_track, &max_sample_size) != NErr_Success)
	{
		mp4_file->Free(buffer);
		return NErr_Error;
	}

	/* allocate memory for it */
	sample_buffer = (uint8_t *)malloc(max_sample_size);
	if (!sample_buffer)
	{
		mp4_file->Free(buffer);
		return NErr_OutOfMemory;
	}


	/* create our C++ object */
	MP4MP3Decoder *mp4_decoder = new ReferenceCounted<MP4MP3Decoder>;
	if (!mp4_decoder)
	{
		mp4_file->Free(buffer);
		free(sample_buffer);
		return NErr_OutOfMemory;
	}

	/* initialize */
	int ret = mp4_decoder->Initialize(mp4_file, mp4_track, sample_buffer, max_sample_size);
	if (ret != NErr_Success)
	{
		mp4_decoder->Release();
		return ret;
	}

	*out_decoder = mp4_decoder;
	return NErr_Success;
}

MP4MP3Decoder::MP4MP3Decoder()
{	
	mp3=0;

	mp4_file=0;
	mp4_track=mp4file_invalid_track_id;
	next_sample=1;
	sample_rate=0;
	channels=0;
	pre_delay=0;
	sample_buffer = 0;
	max_sample_size = 0;
}

int MP4MP3Decoder::Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, uint8_t *sample_buffer, size_t max_sample_size)
{
	mp3 = new CMpgaDecoder;
	if (!mp3)
		return NErr_OutOfMemory;

	this->mp4_file = mp4_file;
	mp4_file->Retain();
	this->mp4_track = mp4_track;
	this->sample_buffer = sample_buffer;
	this->max_sample_size = max_sample_size;
	return NErr_Success;
}

MP4MP3Decoder::~MP4MP3Decoder()
{
	delete mp3;

	free(sample_buffer);
	if (mp4_file)
		mp4_file->Release();
}

int MP4MP3Decoder::MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)
{
	parameters->audio.sample_rate = mp3->m_Info.GetSFreq();
	parameters->audio.format_type = nsaudio::format_type_float;
	parameters->audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->audio.bytes_per_sample = 4;
	parameters->audio.bits_per_sample = 32;
	parameters->audio.number_of_channels = mp3->m_Info.GetChannels();

	return NErr_Success;
}

int MP4MP3Decoder::MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position)
{
	uint32_t sample_size=max_sample_size;

	for (;;)
	{
		ifc_mp4file::Timestamp timestamp;
		ifc_mp4file::Duration duration;
		ifc_mp4file::Duration offset;
		if (mp4_file->Sample_Read(mp4_track, next_sample++, &sample_buffer, &sample_size, &timestamp, &duration, &offset) == NErr_Success)
		{
			mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp, start_position);
			mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp+duration, end_position);

			// feed compressed data into the decoder
			mp3->Fill((const unsigned char *)sample_buffer, sample_size);

			size_t pcm_buf_used=0;
			SSC mpeg_ret = mp3->DecodeFrame(float_buffer, sizeof(float_buffer), &pcm_buf_used);
					if (SSC_SUCCESS(mpeg_ret))
					{
						*output_buffer = float_buffer;
				*output_buffer_bytes = pcm_buf_used;
				return NErr_Success;
					}
			else switch(mpeg_ret)
			{
			case SSC_W_MPGA_SYNCNEEDDATA:
			case SSC_W_MPGA_SYNCSEARCHED:
			case SSC_W_MPGA_SYNCLOST:
				break;
			case SSC_E_MPGA_WRONGLAYER:
				return NErr_Error;
			case SSC_W_MPGA_SYNCEOF:
				return NErr_EndOfFile;
			}
		}
		else
			return NErr_EndOfFile;
	}
}

int MP4MP3Decoder::MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number)
{
	next_sample = sample_number;
	mp3->Reset();
	return NErr_Success;
}

int MP4MP3Decoder::MP4AudioDecoder_SeekSeconds(double *seconds)
{
	ifc_mp4file::Duration duration;
	if (mp4_file->Track_ConvertToDuration(mp4_track, *seconds, &duration) != NErr_Success)
		return NErr_EndOfFile;

	ifc_mp4file::SampleID new_sample_id;
	if (mp4_file->Sample_GetFromDuration(mp4_track, duration, &new_sample_id) != NErr_Success)
		return NErr_Error;

	mp4_file->Track_ConvertFromTimestamp(mp4_track, duration, seconds);
	next_sample = new_sample_id;
	mp3->Reset();
	return NErr_Success;	
}

int MP4MP3Decoder::MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file)
{
	mp4_file->Release();
	mp4_file=new_file;
	mp4_file->Retain();
	return NErr_Success;
}