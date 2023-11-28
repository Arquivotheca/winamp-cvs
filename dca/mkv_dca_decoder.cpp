#include "mkv_dca_decoder.h"
#include <math.h>

int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_DTS"))
	{
		int32_t size=0;
		dca_state_t *ctx = dca_init(0);//MM_ACCEL_X86_MMX);
		if (ctx)
		{
				*decoder = new MKVDCADecoder(ctx, preferred_bits, max_channels, floating_point);
				return CREATEDECODER_SUCCESS;
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

MKVDCADecoder::MKVDCADecoder(dca_state_t *ctx, unsigned int bps, unsigned max_channels, bool floating_point) 
: decoder(ctx), bps(bps), max_channels(max_channels), floating_point(floating_point)
{
	this->bps = 16; // TODO
	preDelay=0;
	channels = 2; // TODO!!!
	syncd=false;
	// TODO: configure things like max_channels
}

int MKVDCADecoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = 256*12*(bps/8); 
	return MKV_SUCCESS;
}

int MKVDCADecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*isFloat = floating_point;
	*bitsPerSample = bps;
	*sampleRate = sample_rate;
	*channels = this->channels; // TODO!
	return MKV_SUCCESS;
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

static void Float32_To_Int16_Clip(void *destinationBuffer, void *sourceBuffer, size_t count, size_t n, double gain)
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
		dest +=n;
	}
}

int MKVDCADecoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
		*outputBufferBytes = 0;

	while (inputBufferBytes)
	{
	
int length = dca_syncinfo(decoder, (uint8_t *)inputBuffer, &flags, &sample_rate, &bit_rate, &frame_length);
if (length > inputBufferBytes)
return MKV_FAILURE;
	//int flags;
	    flags = DCA_STEREO;
//    float level = CONVERT_LEVEL;
	float level=1.0f;
	int error_code = dca_frame (decoder, (uint8_t *)inputBuffer, &flags, &level, 0.0f);
	if (!error_code)
	{
		dca_dynrng (decoder, NULL, NULL);
		for (int i = 0; i < dca_blocks_num (decoder); i++) 
		{
		    if (dca_block (decoder))
		        return MKV_FAILURE;
				float *samples = dca_samples (decoder);
				for (int c=0;c<channels;c++)
				{
					Float32_To_Int16_Clip((char *)outputBuffer + (bps/8)*c, samples + 256*c, 256, channels, 1.0);
					*outputBufferBytes = *outputBufferBytes + 256*(bps/8);
				}
				outputBuffer = (char *)outputBuffer + 256*(bps/8)*channels;
		}
	}
	inputBuffer = (char *)inputBuffer + length;
	inputBufferBytes -= length;
	}

	/*
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
*/
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

void MKVDCADecoder::Flush()
{
	//ac3decReset(decoder);
}

void MKVDCADecoder::Close()
{
	dca_free(decoder);
	delete this;
}

#define CBCLASS MKVDCADecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS
