#include "avi_a52_decoder.h"
#include "../a52dec/ac3_dec.h"
#include <math.h>
#include <assert.h>
int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::audio_format *waveformat = (nsavi::audio_format *)stream_format;

	if (waveformat->format == nsavi::audio_format_a52)
	{
		int32_t size=0;
		AC3Status status = ac3decInit(0, &size);
		if (AC3_OK == status)
		{
			AC3Dec *context = (AC3Dec *)malloc(size);
			status = ac3decInit(context, &size);
			if (AC3_OK == status)
			{
				*decoder = new AVIA52Decoder(context, preferred_bits, max_channels, floating_point);
				return CREATEDECODER_SUCCESS;
			}
		}
		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

AVIA52Decoder::AVIA52Decoder(AC3Dec *ctx, unsigned int bps, unsigned max_channels, bool floating_point) 
: decoder(ctx), bps(bps), max_channels(max_channels), floating_point(floating_point)
{
	this->bps = 16; // TODO
	preDelay=0;
	spill_buffer.reserve(65536); // TODO: determine more accurate size
	// TODO: configure things like max_channels
}

int AVIA52Decoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = 6/*max_channels*/ * (256 * 6) * bps / 8; // each frame has 6 blocks of 256 samples each, per channel
	return AVI_SUCCESS;
}

int AVIA52Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*isFloat = floating_point;
	*bitsPerSample = bps;
	unsigned int bitrate;
	switch(ac3decGetInfo(sampleRate, channels, &bitrate, decoder))
	{
	case AC3_OK:
		return AVI_SUCCESS;
	case AC3_NOT_ENOUGH_DATA:
		return AVI_NEED_MORE_INPUT;
	default:
		return AVI_FAILURE;
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

/* inputBufferBytes > 0
*/
static size_t find_a52_start_code(const void *inputBuffer, size_t inputBufferBytes)
{
	const uint8_t *scan = (const uint8_t *)inputBuffer;
	for (size_t i=0;i!=inputBufferBytes-1;i++)
	{
		if (scan[i] == 0x0b && scan[i+1] == 0x77)
			return i;
	}
	return inputBufferBytes; // if not found, buffer the whole thing
}

int AVIA52Decoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	int32_t out_len = *outputBufferBytes;
	*outputBufferBytes = 0;

	while (!spill_buffer.empty() || *inputBufferBytes)
	{
		AC3Status status;
		int32_t decoded_bytes=0;

		if (!spill_buffer.empty() && *inputBufferBytes)
		{
			decoded_bytes = find_a52_start_code(*inputBuffer, *inputBufferBytes);
			if (decoded_bytes)
			{
			if (decoded_bytes >= spill_buffer.remaining()) // error syncing
			{
				*inputBufferBytes -= decoded_bytes;
				*inputBuffer = (uint8_t *)*inputBuffer + decoded_bytes;
				return AVI_RESYNC;
			}
			spill_buffer.write(*inputBuffer, decoded_bytes);
			*inputBufferBytes -= decoded_bytes;
			*inputBuffer = (uint8_t *)*inputBuffer + decoded_bytes;		

			void *spill;
			size_t spill_size;
			spill_buffer.get(&spill, &spill_size);
			status = ac3decGetFrame((uint8_t *)spill, spill_size, &decoded_bytes, (int16_t *)outputBuffer, out_len, decoder);
			spill = (uint8_t *)spill + decoded_bytes;
			spill_size -= decoded_bytes;
			if (spill_size)
				spill_buffer.write(spill, spill_size);
			decoded_bytes=0;
			}
			else
			{
				void *spill;
				size_t spill_size;
				spill_buffer.get(&spill, &spill_size);
				status = ac3decGetFrame((uint8_t *)spill, spill_size, &decoded_bytes, (int16_t *)outputBuffer, out_len, decoder);
				if (status == AC3_NOT_FIND_SYNCWORD)
				{
				spill_buffer.clear();
				continue;
				}
			}
		}
		else
		{
			assert(spill_buffer.empty());
			spill_buffer.write(*inputBuffer, *inputBufferBytes);
						void *spill;
			size_t spill_size;
			spill_buffer.get(&spill, &spill_size);

			status = ac3decGetFrame((uint8_t *)spill, spill_size, &decoded_bytes, (int16_t *)outputBuffer, out_len, decoder);
			*inputBufferBytes -= decoded_bytes;
			*inputBuffer = (uint8_t *)*inputBuffer + decoded_bytes;
		}
		if (status == AC3_NOT_ENOUGH_DATA)
		{
			decoded_bytes = spill_buffer.write(*inputBuffer, *inputBufferBytes);
			*inputBufferBytes -= decoded_bytes;
			*inputBuffer = (uint8_t *)*inputBuffer + decoded_bytes;
			return AVI_NEED_MORE_INPUT;
		}
		else if (status == AC3_NOT_ENOUGH_BUFFER)
		{
			return AVI_SUCCESS; // we'll get called again with the remaining buffer
		}
		else if (status != AC3_OK)
		{
			return AVI_FAILURE;
		}

		int32_t channels = 6;
		ac3decGetNumChannelOut(&channels, decoder);
		// TODO: get real size
		out_len -= channels * (256 * 6) * bps / 8;
		*outputBufferBytes += channels * (256 * 6) * bps / 8;
		outputBuffer = (uint8_t *)outputBuffer + channels * (256 * 6) * bps / 8;
	}

	return AVI_SUCCESS;
}

void AVIA52Decoder::Flush()
{
		ac3decReset(decoder);
}

void AVIA52Decoder::Close()
{
	free(decoder);
	delete this;
}

#define CBCLASS AVIA52Decoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS