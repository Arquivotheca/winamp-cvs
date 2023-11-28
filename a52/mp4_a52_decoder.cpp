#include "mp4_a52_decoder.h"

MP4A52Decoder::MP4A52Decoder()
{
	decoder = 0;
	bps = 16;
}

int MP4A52Decoder::Open()
{
	int32_t size=0;
	AC3Status status = ac3decInit(0, &size);
	if (AC3_OK == status)
	{
		decoder = (AC3Dec *)malloc(size);
		status = ac3decInit(decoder, &size);

		if (AC3_OK == status)
		{
			return MP4_SUCCESS;
		}
	}

	return MP4_FAILURE;
}

void MP4A52Decoder::Close()
{
}

void MP4A52Decoder::Flush()
{
	ac3decReset(decoder);
}

int MP4A52Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	//	*isFloat = floating_point;
	*bitsPerSample = bps;
	unsigned int bitrate;
	switch(ac3decGetInfo(sampleRate, channels, &bitrate, decoder))
	{
	case AC3_OK:
		return MP4_SUCCESS;
	case AC3_NOT_ENOUGH_DATA:
		return MP4_GETOUTPUTPROPERTIES_NEED_MORE_INPUT;
	default:
		return MP4_FAILURE;
	}
}

int MP4A52Decoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	int32_t out_len = *outputBufferBytes;
	*outputBufferBytes = 0;

	while (inputBufferBytes)
	{
		int32_t decoded_bytes=0;
		AC3Status status = ac3decGetFrame((uint8_t *)inputBuffer, inputBufferBytes, &decoded_bytes, (int16_t *)outputBuffer, out_len, decoder);
		if (status != AC3_OK)
			return MP4_FAILURE;
		inputBufferBytes -= decoded_bytes;
		inputBuffer = (uint8_t *)inputBuffer + decoded_bytes;
		int32_t channels = 6;
		ac3decGetNumChannelOut(&channels, decoder);
		// TODO: get real size
		out_len -= channels * (256 * 6) * bps / 8;
		*outputBufferBytes += channels * (256 * 6) * bps / 8;
		outputBuffer = (uint8_t *)outputBuffer + channels * (256 * 6) * bps / 8;
	}

	return MP4_SUCCESS;
}

int MP4A52Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "ac-3") 
		|| !strcmp(codecName, "mp4a"); // just to see if type == MP4_AC3_AUDIO_TYPE
}

int MP4A52Decoder::CanHandleType(unsigned __int8 type)
{
	switch(type)
	{
	case MP4_TYPE_AC3_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int MP4A52Decoder::OutputFrameSize(size_t *frameSize)
{
	*frameSize = 6/*max_channels*/ * (256 * 6) * bps / 8; // each frame has 6 blocks of 256 samples each, per channel
	return MP4_SUCCESS;
}

#define CBCLASS MP4A52Decoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN, Open)
//CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_HANDLES_TYPE, CanHandleType)
//CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;
#undef CBCLASS