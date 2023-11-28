/* copyright 2006 Ben Allison */
#include "ALACDecoder.h"
#include "api.h"
#include "decomp.h"
#include <math.h>
#include <string.h>


// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

int ALACDecoder::OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat)
{
	if (useFloat)
		return MP4_FAILURE;
	// get requested output bits
	this->output_bits = output_bits;

	use_rg = false;
	rg = 1.0f;
	uint64_t val;
	if (MP4GetTrackIntegerProperty(mp4_file, mp4_track, "mdia.minf.stbl.stsd.*[0].channels", &val))
		channels = val;
	else
		channels=2;

	if (MP4GetTrackIntegerProperty(mp4_file, mp4_track, "mdia.minf.stbl.stsd.*[0].sampleSize", &val))
		bps=val;
	else
		bps=16;

	return MP4_SUCCESS;
}

int ALACDecoder::GetCurrentBitrate(unsigned int *bitrate)
{
	if (alac->last_bitrate)
	{
		*bitrate = alac->last_bitrate;
		return MP4_SUCCESS;
	}
	return MP4_GETCURRENTBITRATE_UNKNOWN; // TODO
}


int ALACDecoder::AudioSpecificConfiguration(void *buffer, size_t buffer_size)
{
	alac = create_alac(bps, channels); 
	alac_set_info(alac, reinterpret_cast<char *>(buffer));
	return MP4_SUCCESS;
}

void ALACDecoder::Flush()
{
	// TODO
}

void ALACDecoder::Close()
{
	if (alac)
		destroy_alac(alac);
	alac=0;
}

int ALACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	*sampleRate = alac->setinfo_sample_rate; // TODO: verify
	*channels = alac->numchannels; 
/*
	if (use_rg) 
		*bitsPerSample=this->bitsPerSample;
	else*/
		*bitsPerSample = alac->setinfo_sample_size;

	return MP4_SUCCESS;
}

int ALACDecoder::OutputFrameSize(size_t *frameSize)
{
	*frameSize=alac->setinfo_max_samples_per_frame; // TODO: verify
	return MP4_SUCCESS;
}


#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

int ALACDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	int buffer_size_int = (int)*outputBufferBytes;
	decode_frame(alac, reinterpret_cast<unsigned char *>(inputBuffer), outputBuffer, &buffer_size_int);
	*outputBufferBytes = buffer_size_int;

	if (use_rg && alac->setinfo_sample_size == 16)
	{
		size_t numSamples = buffer_size_int / (alac->setinfo_sample_size/8);
		//if (bitsPerSample == 16)
		{
					// TODO: this algorithm assumes ALAC bps is 16!!
			int16_t *audioBuffer = (int16_t *)outputBuffer;
			for (size_t i=0;i!=numSamples;i++)
			{
				float sample = (float)audioBuffer[i];
				int32_t temp = (int32_t) (sample*rg);
				PA_CLIP_( temp, -0x8000, 0x7FFF );
				audioBuffer[i] = (uint16_t) temp;
			}
		}/*
		else
		{
			// TODO: this algorithm assumes ALAC bps is 16!!
			const int16_t *audioBuffer = (int16_t *)outputBuffer;
			uint8_t *dest = (uint8_t *)outputBuffer;
			size_t i=numSamples;
			while (i--)
			{
				double sample = (double)audioBuffer[i]*rg;
				clip( sample, -2147483648., 2147483647.);
				int32_t temp = (int32_t) sample;

				dest[i*3] = (unsigned char)(temp >> 8);
				dest[i*3+1] = (unsigned char)(temp >> 16);
				dest[i*3+2] = (unsigned char)(temp >> 24);
			}
		}*/
	}
	return MP4_SUCCESS;
}

const char *ALACDecoder::GetCodecInfoString()
{
	return "mdia.minf.stbl.stsd.alac.alac.decoderConfig";
}


int ALACDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "alac");
}

int ALACDecoder::SetGain(float gain)
{
	if (gain != 1.0f)
	{
		use_rg = true;
		rg = gain;
	}
	return MP4_SUCCESS;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ALACDecoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPENMP4, OpenMP4)
CB(MPEG4_AUDIO_ASC, AudioSpecificConfiguration)
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_CODEC_INFO_STRING, GetCodecInfoString)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;