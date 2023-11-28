#include "flv_aac_decoder.h"
#include "aacplusdec.h"
#include <math.h>
#include "../nsutil/pcm.h"

int FLVDecoderCreator::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_AAC)
	{
		HANDLE_AACPLUSDEC_DECODER handle;
		if (aacPlusDecEasyOpen(&handle, AACPLUSDEC_OUTPUTFORMAT_FLOAT, 6) != AACPLUSDEC_OK)
			return CREATEDECODER_FAILURE;
		*decoder = new FLVAAC(handle);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesAudio(int format_type)
{
	if (format_type == FLV::AUDIO_FORMAT_AAC)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
CB(HANDLES_AUDIO, HandlesAudio)
END_DISPATCH;
#undef CBCLASS

/* --- */
FLVAAC::FLVAAC(HANDLE_AACPLUSDEC_DECODER handle) : handle(handle)
{
	bps = 16;
	preDelay=0;
	got_decoder_config = false;

			AACPLUSDEC_EXPERTSETTINGS *pConfig = aacPlusDecGetDecoderSettingsHandle(handle);
		pConfig->bEnableOutputLimiter = 0;
		pConfig->nDmxOutputChannels = 6;
		aacPlusDecSetDecoderSettings(handle);

		streamProperties = aacPlusDecGetStreamPropertiesHandle(handle);
}

int FLVAAC::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *_bits)
{
	if (streamProperties)
	{
		AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
		if (currentProgram)
		{
			*sample_rate = currentProgram->nOutputSamplingRate;
			*channels = currentProgram->nOutputChannels;
			*_bits = bps;

			if (streamProperties->nDecodingState != AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED)
				return FLV_AUDIO_NEEDS_MORE_INPUT;
			else
				return FLV_AUDIO_SUCCESS;
		}
	}
	return FLV_AUDIO_FAILURE;
}

int FLVAAC::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	const uint8_t *type = (const uint8_t *)input_buffer;
	if (type[0] == 0)
	{
		// decoder config
		AACPLUSDEC_BITSTREAMBUFFERINFO configBufferInfo = {input_buffer_bytes-1, 0, 0};
		AACPLUSDEC_ERROR result;
		result = aacPlusDecReadConfigStream(handle, const_cast<unsigned char *>(type+1), &configBufferInfo, AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG, 0, AACPLUSDEC_BITSTREAMFORMAT_RAW);
		got_decoder_config=true;
		*samples_size_bytes=0;
		return FLV_AUDIO_SUCCESS;
	}
	else if (type[0] == 1)
	{
		float temp[2048*6];
		int corruptFlag = 0;
		AACPLUSDEC_BITSTREAMBUFFERINFO bitBufInfo = {input_buffer_bytes-1, 0, 0};
		AACPLUSDEC_AUDIOBUFFERINFO audioBufInfo = {sizeof(temp), 0, 0};

		float *data = temp;
		AACPLUSDEC_ERROR result = aacPlusFrameDecode(handle, reinterpret_cast<unsigned char *>(data), &audioBufInfo, const_cast<unsigned char *>(type+1), &bitBufInfo, corruptFlag, 0,0);
		switch (result)
		{
		case AACPLUSDEC_OK:
			{
				size_t numSamples = audioBufInfo.nBytesWrittenOut / sizeof(float);
				*samples_size_bytes = numSamples * (bps / 8);

				nsutil_pcm_FloatToInt_Interleaved_Gain(samples, data, bps, numSamples, 1.0f/*gain*/);

						if (streamProperties->nBitrate)
						{
							*bitrate = (double)streamProperties->nBitrate/1000.0;
						}
						else
						{
							AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);
							*bitrate = (double)bitBufInfo.nBytesReadOut*(double)currentProgram ->nAacSamplingRate/128000.0;
						}
			}
			return FLV_AUDIO_SUCCESS;
		default:
			return FLV_AUDIO_FAILURE;
		}		
	}
	else
		return FLV_AUDIO_FAILURE;
}

void FLVAAC::Flush()
{
	aacPlusDecRestart(handle);
}

void FLVAAC::Close()
{
	aacPlusDecClose(&handle);
	handle = 0;
	delete this;
}

int FLVAAC::Ready()
{
	return !!got_decoder_config;
}

void FLVAAC::SetPreferences(unsigned int _max_channels, unsigned int preferred_bits)
{
	if (preferred_bits)
		bps = preferred_bits;

	if (_max_channels && _max_channels <= 6)
	{
		AACPLUSDEC_EXPERTSETTINGS *pConfig = aacPlusDecGetDecoderSettingsHandle(handle);
		pConfig->bEnableOutputLimiter = 0;
		pConfig->nDmxOutputChannels = _max_channels;
		aacPlusDecSetDecoderSettings(handle);
	}
}

#define CBCLASS FLVAAC
START_DISPATCH;
CB(FLV_AUDIO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_AUDIO_DECODE, DecodeSample)
VCB(FLV_AUDIO_FLUSH, Flush)
VCB(FLV_AUDIO_CLOSE, Close)
CB(FLV_AUDIO_READY, Ready)
VCB(FLV_AUDIO_SETPREFERENCES, SetPreferences)
END_DISPATCH;
#undef CBCLASS
