#include "avi_aac_decoder.h"
#include "aacplusdec.h"
#include <math.h>
#include "../enc_aacplus/AncillaryData.h"
#include "../nsutil/pcm.h"

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::audio_format *waveformat = (nsavi::audio_format *)stream_format;

	if (waveformat->format == nsavi::audio_format_aac)
	{
		AVIAACDecoder *aac_decoder = AVIAACDecoder::Create(waveformat, preferred_bits, max_channels, floating_point);
		if (aac_decoder)
		{
			*decoder = aac_decoder;
			return CREATEDECODER_SUCCESS;
		}
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

AVIAACDecoder *AVIAACDecoder::Create(const nsavi::audio_format *waveformat, unsigned int preferred_bits, unsigned int max_channels, bool floating_point)
{
	HANDLE_AACPLUSDEC_DECODER handle;

	if (preferred_bits > 24) 
		preferred_bits=24;
	else
		preferred_bits=16;

	if (!max_channels)
		max_channels = 6;

	if (aacPlusDecEasyOpen(&handle, AACPLUSDEC_OUTPUTFORMAT_FLOAT, 6) == AACPLUSDEC_OK)
	{
		AACPLUSDEC_EXPERTSETTINGS *pConfig = aacPlusDecGetDecoderSettingsHandle(handle);
		pConfig->bEnableOutputLimiter = 0;
		pConfig->nDmxOutputChannels = max_channels;
		aacPlusDecSetDecoderSettings(handle);

		if (waveformat->extra_size_bytes)
		{
			AACPLUSDEC_BITSTREAMBUFFERINFO configBufferInfo = {waveformat->extra_size_bytes, 0, 0};
			AACPLUSDEC_ERROR result;
			result = aacPlusDecReadConfigStream(handle, (unsigned char *)(waveformat + 1), &configBufferInfo, AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG, 0, AACPLUSDEC_BITSTREAMFORMAT_RAW);
			if (result != AACPLUSDEC_OK)
			{
				aacPlusDecClose(&handle);
				return 0;
			}
		}

		AACPLUSDEC_STREAMPROPERTIES *streamProperties = aacPlusDecGetStreamPropertiesHandle(handle);
		return new AVIAACDecoder(handle, streamProperties, preferred_bits, floating_point);
	}

	return 0;
}

AVIAACDecoder::AVIAACDecoder(HANDLE_AACPLUSDEC_DECODER handle, AACPLUSDEC_STREAMPROPERTIES *streamProperties, unsigned int bps, bool floating_point) 
: handle(handle), streamProperties(streamProperties), bps(bps), floating_point(floating_point)
{
	preDelay=0;
}

int AVIAACDecoder::OutputFrameSize(size_t *frame_size)
{
	if (!streamProperties)
		return AVI_FAILURE;
	AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
	if (!currentProgram)
		return AVI_FAILURE;

	*frame_size = currentProgram->nOutputSamplesPerFrame;
	return AVI_SUCCESS;
}

int AVIAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (streamProperties)
	{
		AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
		if (currentProgram)
		{
			*sampleRate = currentProgram->nOutputSamplingRate;
			*channels = currentProgram->nOutputChannels;
			*bitsPerSample = bps;
			*isFloat = floating_point;
			/* TODO: uncomment when we fix up in_avi
			if (streamProperties->nDecodingState != AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED)
			return AVI_NEED_MORE_INPUT;
			else*/
			return AVI_SUCCESS;
		}
	}
	return AVI_FAILURE;
}

int AVIAACDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	float temp[2048*6];

	int corruptFlag = 0;
	AACPLUSDEC_BITSTREAMBUFFERINFO bitBufInfo = {*inputBufferBytes, 0, 0};
	AACPLUSDEC_AUDIOBUFFERINFO audioBufInfo = {sizeof(temp), 0, 0};
	//AncillaryData ancData;
	//AACPLUSDEC_DATASTREAMBUFFERINFO datastreamInfo = {sizeof(ancData), 0, 0};

	float *data = temp;
	AACPLUSDEC_ERROR result = aacPlusFrameDecode(handle, reinterpret_cast<unsigned char *>(data), &audioBufInfo, static_cast<unsigned char *>(*inputBuffer), &bitBufInfo, corruptFlag, 0,0);// reinterpret_cast<unsigned char *>(&ancData), &datastreamInfo);
	switch (result)
	{
	case AACPLUSDEC_OK:
		{
			*inputBufferBytes -= bitBufInfo.nBytesReadOut;
			*inputBuffer = (uint8_t *)(*inputBuffer) + bitBufInfo.nBytesReadOut;
			size_t numSamples = audioBufInfo.nBytesWrittenOut / sizeof(float);
			*outputBufferBytes = numSamples * (bps / 8);

			if (!floating_point)
			{
				nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, data, bps, numSamples, 1.0f/*gain*/);
			}
			else
			{
				for (size_t i = 0;i != numSamples;i++)
					((float *)outputBuffer)[i] = data[i] /* * gain*/;
			}

		}
		return AVI_SUCCESS;
	default:
		return AVI_FAILURE;
	}		

}

void AVIAACDecoder::Flush()
{
	aacPlusDecRestart(handle);
}

void AVIAACDecoder::Close()
{
	aacPlusDecClose(&handle);
	handle = 0;
	delete this;
}

#define CBCLASS AVIAACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS