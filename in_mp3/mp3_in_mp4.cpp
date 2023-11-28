// used to decode an MPEG-1 audio object in an MPEG-4 ISO Media file
#include "main.h"
#include <windows.h>
#include "mp3_in_mp4.h"
#include "api.h"
#include "config.h"
#include "../nsutil/pcm.h"

#define FHG_DELAY 529
MPEG4_MP3::MPEG4_MP3(): gain(1.0f), floatingPoint(false)
{
	bits = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (bits >= 24)	bits = 24;
	else	bits = 16;

	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		maxChannels = 1;
	else
		maxChannels = 2;

	pregap = FHG_DELAY;
}

int MPEG4_MP3::OpenEx(size_t _bits, size_t _maxChannels, bool useFloat)
{
	floatingPoint = useFloat;
	if (floatingPoint)
		bits = 32;
	else
		bits = _bits;

	maxChannels = _maxChannels;
	return Open();
}

int MPEG4_MP3::Open()
{
	if (maxChannels == 1)
		mp3.SetDownmix();
	return MP4_SUCCESS;
}

const char *MPEG4_MP3::GetCodecInfoString()
{
	return 0;
}

int MPEG4_MP3::CanHandleCodec(const char *codecName)
{
	if (!lstrcmpA(codecName, "mp4a"))
		return 1;
	else
		return 0;
}

int MPEG4_MP3::CanHandleType(unsigned __int8 type)
{
	switch(type)
	{
	case MP4_MPEG4_LAYER3_AUDIO:
	case MP4_MPEG4_LAYER2_AUDIO:
	case MP4_MPEG4_LAYER1_AUDIO:
	case MP4_TYPE_MPEG1_AUDIO:
	case MP4_TYPE_MPEG2_AUDIO:
	//case MP4_TYPE_MPEG4_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int MPEG4_MP3::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	// feed compressed data into the decoder
	mp3.Fill((const unsigned char *)inputBuffer, inputBufferBytes);

	// get the decoded data out
	int pcm_buf_used;
	float decodeBuf[1152*2*2];
	SSC ret = mp3.DecodeFrame(decodeBuf, sizeof(decodeBuf), &pcm_buf_used);

	if (!pcm_buf_used)
	{
		*outputBufferBytes = 0;
		return MP4_NEED_MORE_INPUT;
	}

	// deal with pregap
	int numSamples = pcm_buf_used / sizeof(float);
	int offset = min(numSamples, pregap * mp3.m_Info.GetEffectiveChannels());
	numSamples -= offset;
	pregap -= offset / mp3.m_Info.GetEffectiveChannels();
	float *pcm_buf = decodeBuf + offset;

	// convert to destination sample format
	if (!floatingPoint)
	{
					nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, pcm_buf, bits, numSamples, gain);
	}
	else
	{
				float *data = (float *)outputBuffer;
			for (int i = 0;i < numSamples;i++)
				data[i] = pcm_buf[i] * gain;
	}
	
	*outputBufferBytes = numSamples * bits / 8;

	return MP4_SUCCESS;
}

int MPEG4_MP3::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*sampleRate = mp3.m_Info.GetEffectiveSFreq();
	*channels = mp3.m_Info.GetEffectiveChannels();
	*bitsPerSample = bits;
	*isFloat = floatingPoint;
	return MP4_SUCCESS;
}

int MPEG4_MP3::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	bool dummy;
	return GetOutputPropertiesEx(sampleRate, channels, bitsPerSample, &dummy);
}

void MPEG4_MP3::Close()
{}

void MPEG4_MP3::Flush()
{
	mp3.Reset();
	pregap = FHG_DELAY;
}

int MPEG4_MP3::SetGain(float _gain)
{
	gain = _gain;
	return MP4_SUCCESS;
}

int MPEG4_MP3::GetCurrentBitrate(unsigned int *bitrate)
{
	*bitrate = mp3.GetStreamInfo()->GetBitrate() / 1000;
	return MP4_SUCCESS;
}

int MPEG4_MP3::OutputFrameSize(size_t *frameSize)
{
	if (mp3.GetStreamInfo()->GetMpegVersion() == 0) // MPEG-1
		*frameSize = 1152;
	else
		*frameSize = 576;
	return MP4_SUCCESS;
}


int MPEG4_MP3::CanHandleMPEG4Type(unsigned __int8 type)
{
	switch (type)
	{
	case MP4_MPEG4_LAYER1_AUDIO:
	case MP4_MPEG4_LAYER2_AUDIO:
	case MP4_MPEG4_LAYER3_AUDIO:
		return 1;
	default:
		return 0;
	}
}

#define CBCLASS MPEG4_MP3
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN_EX, OpenEx)
CB(MPEG4_AUDIO_OPEN, Open)
CB(MPEG4_AUDIO_CODEC_INFO_STRING, GetCodecInfoString)
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_OUTPUTINFO_EX, GetOutputPropertiesEx)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_HANDLES_TYPE, CanHandleType)
CB(MPEG4_AUDIO_HANDLES_MPEG4_TYPE, CanHandleMPEG4Type)
CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;
