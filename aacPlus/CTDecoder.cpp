#include "aacplusdec.h"
#include "CTDecoder.h"
#include "../enc_aacplus/AncillaryData.h"
#include "api.h"
#include <math.h>
#include <bfc/platform/minmax.h>
#include "../nsutil/pcm.h"



#define CT_USE_LIMITER 0

#if CT_USE_LIMITER == 1
#define LIMITER_DELAY 64
#else
#define LIMITER_DELAY 0
#endif




// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
    { 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

CTDecoder::CTDecoder()
		: streamProperties(0), preDelay(LIMITER_DELAY), handle(0), isFloat(false), gain(1.0f)
{
	// get bps
	bitsPerSample = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (bitsPerSample >= 24)	bitsPerSample = 24;
	else	bitsPerSample = 16;

	// get max channels
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true))
		maxChannels = 6;
	else if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		maxChannels = 1;
	else
		maxChannels = 2;
}

int CTDecoder::Open()
{
	if (aacPlusDecEasyOpen(&handle, AACPLUSDEC_OUTPUTFORMAT_FLOAT, 6) == AACPLUSDEC_OK)
	{
		AACPLUSDEC_EXPERTSETTINGS *pConfig = aacPlusDecGetDecoderSettingsHandle(handle);
		pConfig->bEnableOutputLimiter = CT_USE_LIMITER;
		pConfig->nDmxOutputChannels = maxChannels;
		aacPlusDecSetDecoderSettings(handle);

		streamProperties = aacPlusDecGetStreamPropertiesHandle(handle);

		return MP4_SUCCESS;
	}

	return MP4_FAILURE;
}

int CTDecoder::OpenEx(size_t _bits, size_t _maxChannels, bool useFloat)
{
	isFloat = useFloat;
	if (isFloat)
	{
		bitsPerSample = 32;
	}
	else
	{
		if (_bits)
			bitsPerSample = _bits;
		if (bitsPerSample >= 24)	bitsPerSample = 24;
		else	bitsPerSample = 16;
	}
	if (_maxChannels)
		maxChannels = _maxChannels;
	return Open();
}

int CTDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample)
{
	bool dummy;
	return GetOutputPropertiesEx(sampleRate, channels, _bitsPerSample, &dummy);
}

int CTDecoder::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample, bool *useFloat)
{
	if (streamProperties)
	{

		AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
		if (currentProgram)
		{
			*sampleRate = currentProgram->nOutputSamplingRate;
			*channels = currentProgram->nOutputChannels;
			*_bitsPerSample = bitsPerSample;
			*useFloat = isFloat;

			if (streamProperties->nDecodingState != AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED)
				return MP4_GETOUTPUTPROPERTIES_NEED_MORE_INPUT;
			else
				return MP4_SUCCESS;
		}
	}
	return MP4_FAILURE;
}

void CTDecoder::Flush()
{
	aacPlusDecRestart(handle);
}

int CTDecoder::GetCurrentBitrate(unsigned int *bitrate)
{
	if (!streamProperties)
		return MP4_FAILURE;

	if (streamProperties->nBitrate)
	{
		*bitrate = streamProperties->nBitrate;
		return MP4_SUCCESS;
	}
	else
		return MP4_GETCURRENTBITRATE_UNKNOWN;
}

int CTDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	float temp[2048*6];

	int corruptFlag = 0;
	AACPLUSDEC_BITSTREAMBUFFERINFO bitBufInfo = {inputBufferBytes, 0, 0};
	AACPLUSDEC_AUDIOBUFFERINFO audioBufInfo = {sizeof(temp), 0, 0};
	AncillaryData ancData;
	AACPLUSDEC_DATASTREAMBUFFERINFO datastreamInfo = {sizeof(ancData), 0, 0};


	float *data = temp;
	AACPLUSDEC_ERROR result = aacPlusFrameDecode(handle, reinterpret_cast<unsigned char *>(data), &audioBufInfo, static_cast<unsigned char *>(inputBuffer), &bitBufInfo, corruptFlag, reinterpret_cast<unsigned char *>(&ancData), &datastreamInfo);
	switch (result)
	{
	case AACPLUSDEC_OK:
		{
			// maybe our codec delay is stored :)
			if (datastreamInfo.nBytesWrittenOut >= 6 && ancData.magicWord == PRE_PADDING_MAGIC_WORD)
			{
				unsigned int nch, srate, bps;
				GetOutputProperties(&srate, &nch, &bps); // TODO: error check
				preDelay = (ancData.padding + LIMITER_DELAY) * nch * (sizeof(float)); // magic 75 for limiter + bullshit TODO: verify
			}

			/* begin: handle pre-delay */
			size_t outCut =  MIN((size_t)audioBufInfo.nBytesWrittenOut, preDelay);

			data += outCut / sizeof(float);
			size_t written = audioBufInfo.nBytesWrittenOut - outCut;
			preDelay -= outCut;
			/* end: handle pre-delay */

			size_t numSamples = written / sizeof(float);
			*outputBufferBytes = numSamples * (bitsPerSample / 8);

			if (!isFloat)
			{
				nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, data, bitsPerSample, numSamples, gain);
			}
			else
			{
				for (size_t i = 0;i != numSamples;i++)
					((float *)outputBuffer)[i] = data[i] * gain;
			}

		}
		return MP4_SUCCESS;
	default:
		return MP4_FAILURE;

	}
}

int CTDecoder::OutputFrameSize(size_t *frameSize)
{
	if (!streamProperties)
		return MP4_FAILURE;
	AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
	if (!currentProgram)
		return MP4_FAILURE;

	*frameSize = currentProgram->nOutputSamplesPerFrame;
	return MP4_SUCCESS;
}


int CTDecoder::AudioSpecificConfiguration(void *buffer, size_t buffer_size) // reads ASC block from mp4 file
{
	AACPLUSDEC_BITSTREAMBUFFERINFO configBufferInfo = {buffer_size, 0, 0};
	AACPLUSDEC_ERROR result;
	result = aacPlusDecReadConfigStream(handle, reinterpret_cast<unsigned char *>(buffer), &configBufferInfo, AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG, 0, AACPLUSDEC_BITSTREAMFORMAT_RAW);
	// TODO: error check
	return MP4_SUCCESS;
}

void CTDecoder::Close()
{
	aacPlusDecClose(&handle);
	handle = 0;
}

int CTDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "mp4a");
}

int CTDecoder::CanHandleType(uint8_t type)
{
	switch (type)
	{
	case MP4_TYPE_MPEG4_AUDIO:
		return 1;
	case MP4_TYPE_MPEG2_AAC_LC_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int CTDecoder::CanHandleMPEG4Type(uint8_t type)
{
	switch (type)
	{
	case MP4_MPEG4_TYPE_AAC_LC_AUDIO:
	case MP4_MPEG4_TYPE_AAC_HE_AUDIO:
	case MP4_MPEG4_TYPE_PARAMETRIC_STEREO:
		return 1;
	default:
		return 0;
	}
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS CTDecoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN, Open)
CB(MPEG4_AUDIO_OPEN_EX, OpenEx)
CB(MPEG4_AUDIO_ASC, AudioSpecificConfiguration)
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
