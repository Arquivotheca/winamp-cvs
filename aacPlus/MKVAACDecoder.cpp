#include "MKVAACDecoder.h"
#include "aacplusdec.h"
#include <math.h>
#include "../enc_aacplus/AncillaryData.h"
#include "../nsutil/pcm.h"
MKVAACDecoder::MKVAACDecoder(HANDLE_AACPLUSDEC_DECODER handle, AACPLUSDEC_STREAMPROPERTIES *streamProperties, unsigned int bps, bool floating_point) 
: handle(handle), streamProperties(streamProperties), bps(bps), floating_point(floating_point)
{
	preDelay=0;
}

MKVAACDecoder *MKVAACDecoder::Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point)
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

		if (track_entry_data->codec_private && track_entry_data->codec_private_len)
		{
			AACPLUSDEC_BITSTREAMBUFFERINFO configBufferInfo = {track_entry_data->codec_private_len, 0, 0};
			AACPLUSDEC_ERROR result;
			result = aacPlusDecReadConfigStream(handle, reinterpret_cast<unsigned char *>(track_entry_data->codec_private), &configBufferInfo, AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG, 0, AACPLUSDEC_BITSTREAMFORMAT_RAW);
			if (result != AACPLUSDEC_OK)
			{
				aacPlusDecClose(&handle);
				return 0;
			}
		}

		AACPLUSDEC_STREAMPROPERTIES *streamProperties = aacPlusDecGetStreamPropertiesHandle(handle);
		return new MKVAACDecoder(handle, streamProperties, preferred_bits, floating_point);
	}

	return 0;
}

int MKVAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
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

			if (streamProperties->nDecodingState != AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED)
				return MKV_NEED_MORE_INPUT;
			else
				return MKV_SUCCESS;
		}
	}
	return MKV_FAILURE;
}

void MKVAACDecoder::Flush()
{
	aacPlusDecRestart(handle);
}

int MKVAACDecoder::OutputFrameSize(size_t *frame_size)
{
	if (!streamProperties)
		return MKV_FAILURE;
	AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
	if (!currentProgram)
		return MKV_FAILURE;

	*frame_size = currentProgram->nOutputSamplesPerFrame;
	return MKV_SUCCESS;
}

void MKVAACDecoder::Close()
{
	aacPlusDecClose(&handle);
	handle = 0;
	delete this;
}

int MKVAACDecoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	float temp[2048*6];

	int corruptFlag = 0;
	AACPLUSDEC_BITSTREAMBUFFERINFO bitBufInfo = {inputBufferBytes, 0, 0};
	AACPLUSDEC_AUDIOBUFFERINFO audioBufInfo = {sizeof(temp), 0, 0};
	//AncillaryData ancData;
	//AACPLUSDEC_DATASTREAMBUFFERINFO datastreamInfo = {sizeof(ancData), 0, 0};

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
				nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, data, bps, numSamples, 1.0f/*gain*/);
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
}

#define CBCLASS MKVAACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS