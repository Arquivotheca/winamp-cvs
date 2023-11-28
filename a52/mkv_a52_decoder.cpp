#include "mkv_a52_decoder.h"
#include "../a52dec/ac3_dec.h"
#include <math.h>

int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_AC3"))
	{
		int32_t size=0;
		AC3Status status = ac3decInit(0, &size);
		if (AC3_OK == status)
		{
			AC3Dec *context = (AC3Dec *)malloc(size);
			status = ac3decInit(context, &size);

			if (AC3_OK == status)
			{
				*decoder = new MKVA52Decoder(context, preferred_bits, max_channels, floating_point);
				return CREATEDECODER_SUCCESS;
			}
		}

		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

MKVA52Decoder::MKVA52Decoder(AC3Dec *ctx, unsigned int bps, unsigned max_channels, bool floating_point) 
: decoder(ctx), bps(bps), max_channels(max_channels), floating_point(floating_point)
{
	this->bps = 16; // TODO
	preDelay=0;
	// TODO: configure things like max_channels
}

int MKVA52Decoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = 6/*max_channels*/ * (256 * 6) * bps / 8; // each frame has 6 blocks of 256 samples each, per channel
	return MKV_SUCCESS;
}

int MKVA52Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*isFloat = floating_point;
	*bitsPerSample = bps;
	unsigned int bitrate;
	switch(ac3decGetInfo(sampleRate, channels, &bitrate, decoder))
	{
	case AC3_OK:
		return MKV_SUCCESS;
	case AC3_NOT_ENOUGH_DATA:
		return MKV_NEED_MORE_INPUT;
	default:
		return MKV_FAILURE;
	}
}

#define PA_CLIP_( val, min, max )\
	{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

#if defined(_M_IX86)
static __inline long float_to_long(double t)
{
	long r;
	__asm fld t
	__asm fistp r
	return r;
}
#else
#define float_to_long(x) ((long)( x ))
#endif

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

static void Float32_To_Int24_Clip(void *destinationBuffer, void *sourceBuffer, size_t count, double gain)
{
	float *src = (float*)sourceBuffer;
	unsigned char *dest = (unsigned char*)destinationBuffer;
	signed long temp;
	gain*=65536.*32768.;
	while ( count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src * gain;
		clip( scaled, -2147483648., 2147483647.);
		temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);

		src++;
		dest += 3;
	}
}

static void Float32_To_Int16_Clip(void *destinationBuffer, void *sourceBuffer, size_t count, double gain)
{
	float *src = (float*)sourceBuffer;
	signed short *dest = (signed short*)destinationBuffer;

	gain*=32768.0;
	while ( count-- )
	{
		long samp = float_to_long((*src) * gain/* - 0.5*/);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (signed short) samp;

		src ++;
		dest ++;
	}
}

int MKVA52Decoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	int32_t out_len = *outputBufferBytes;
	*outputBufferBytes = 0;

	while (inputBufferBytes)
	{
		int32_t decoded_bytes=0;
		AC3Status status = ac3decGetFrame((uint8_t *)inputBuffer, inputBufferBytes, &decoded_bytes, (int16_t *)outputBuffer, out_len, decoder);
		if (status != AC3_OK)
			return MKV_FAILURE;
		inputBufferBytes -= decoded_bytes;
		inputBuffer = (uint8_t *)inputBuffer + decoded_bytes;
		int32_t channels = 6;
		ac3decGetNumChannelOut(&channels, decoder);
		// TODO: get real size
		out_len -= channels * (256 * 6) * bps / 8;
		*outputBufferBytes += channels * (256 * 6) * bps / 8;
		outputBuffer = (uint8_t *)outputBuffer + channels * (256 * 6) * bps / 8;
	}

		return MKV_SUCCESS;
                   
#if 0
	float *data = temp;
	AACPLUSDEC_ERROR result = aacPlusFrameDecode(handle, reinterpret_cast<unsigned char *>(data), &audioBufInfo, static_cast<unsigned char *>(inputBuffer), &bitBufInfo, corruptFlag, 0,0);// reinterpret_cast<unsigned char *>(&ancData), &datastreamInfo);
	switch (result)
	{
	case AACPLUSDEC_OK:
		{
			size_t numSamples = audioBufInfo.nBytesWrittenOut / sizeof(float);
			*outputBufferBytes = numSamples * (bps / 8);

			if (!floating_point)
			{
				if (bps == 16)	
					Float32_To_Int16_Clip(outputBuffer, data, numSamples, 1.0f/*gain*/);
				else	
					Float32_To_Int24_Clip(outputBuffer, data, numSamples, 1.0f/*gain*/);
			}
			else
			{
				for (size_t i = 0;i != numSamples;i++)
					((float *)outputBuffer)[i] = data[i] /* * gain*/;
			}

		}
		return MKV_SUCCESS;
	default:
		return MKV_FAILURE;
	}	
#endif
}

void MKVA52Decoder::Flush()
{
	ac3decReset(decoder);
}

void MKVA52Decoder::Close()
{
	free(decoder);
	delete this;
}

#define CBCLASS MKVA52Decoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS