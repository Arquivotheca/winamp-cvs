#include "faac.h"
#include "faaccfg.h"
#include "aacplusenc.h"
#include <bfc/platform/export.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>
#define FRAME_LEN 1024
static char *libCopyright =
"Coding Technology aacPlus Encoder - FAAC compatible library\n"
" (C) 1998-2006 Coding Technology\n"
" (C) 2007 Nullsoft, Inc.\n";

struct EncoderHandle 
{
	aacPlusEncHandle ctEncoder;
	faacEncConfiguration cfg;
	unsigned int numChannels;
	unsigned int sampleRate;
	int frameSize;

};
extern "C"
{

int FAACAPI faacEncGetVersion(char **faac_id_string, char **faac_copyright_string)
{
	if (faac_id_string)
		*faac_id_string = aacPlusEncGetLibraryVersion();

	if (faac_copyright_string)
		*faac_copyright_string = libCopyright;

	return FAAC_CFG_VERSION;
}

faacEncConfigurationPtr FAACAPI
faacEncGetCurrentConfiguration(faacEncHandle hEncoder)
{
	EncoderHandle *handle = (EncoderHandle *)hEncoder;

	aacPlusEncConfiguration *ctCfg = aacPlusEncGetCurrentConfiguration(handle->ctEncoder);
	aacPlusEncStreamType *streamType = aacPlusEncGetCurrentStreamType(handle->ctEncoder);
	aacPlusEncOutputFormat format;
	aacPlusEncGetOutputFormat(handle->ctEncoder, &format);

	handle->cfg.version=FAAC_CFG_VERSION;
	handle->cfg.name=aacPlusEncGetLibraryVersion();
	handle->cfg.copyright=libCopyright;
	handle->cfg.mpegVersion=(streamType->bsFormat==BSFORMAT_ADTS_MP4 || streamType->bsFormat==BSFORMAT_RAW)?MPEG4:MPEG2; // TODO: verify
	handle->cfg.aacObjectType=LOW;
	handle->cfg.allowMidside=1;
	handle->cfg.useLfe=1;
	handle->cfg.useTns=ctCfg->allowTns;
	handle->cfg.bitRate=format.bitRate / handle->numChannels;
	handle->cfg.bandWidth=ctCfg->effectiveBandwidth;
	handle->cfg.quantqual=100; // TODO
	handle->cfg.outputFormat=!!(streamType->bsFormat==BSFORMAT_ADTS || streamType->bsFormat==BSFORMAT_ADTS_MP4); // (ADTS)
	handle->cfg.psymodellist=0;
	handle->cfg.psymodelidx=0;
	handle->cfg.inputFormat=FAAC_INPUT_FLOAT;
	handle->cfg.shortctl=SHORTCTL_NORMAL;
	// TODO: handle->cfg.channel_map

	return &handle->cfg;

}

int FAACAPI faacEncSetConfiguration(faacEncHandle hEncoder,
																		faacEncConfigurationPtr config)
{
	EncoderHandle *handle = (EncoderHandle *)hEncoder;

	aacPlusEncInputFormat inputFormat=aacPlusEncInputShort;
	switch(config->inputFormat)
	{
	case FAAC_INPUT_FLOAT:
		inputFormat=aacPlusEncInputFloat;
		handle->frameSize=4;
		break;
	case FAAC_INPUT_24BIT:
		handle->frameSize=3;
		inputFormat=aacPlusEncInput24Packed;
		break;
	case FAAC_INPUT_16BIT:
		handle->frameSize=2;
		inputFormat=aacPlusEncInputShort;
		break;
	default: 
		return -1; 
	}

	aacPlusEncClose(handle->ctEncoder);
	handle->ctEncoder = aacPlusEncOpen(handle->sampleRate, handle->numChannels,
		inputFormat,
		1, // bAllowV2Features
		0 // bAllowOversampledSBR
		);

	aacPlusEncConfiguration *ctCfg = aacPlusEncGetCurrentConfiguration(handle->ctEncoder);
	aacPlusEncStreamType *streamType = aacPlusEncGetCurrentStreamType(handle->ctEncoder);
	aacPlusEncOutputFormat format;
	aacPlusEncGetOutputFormat(handle->ctEncoder, &format);
	format.channelMode=UNDEFINED_CHANNEL_MODE;
	format.sbrMode=UNDEFINED_SBR_MODE;
	format.signalType=DEFAULT_SIGNAL_TYPE;
	
	switch(config->mpegVersion)
	{
	case MPEG2:
		if (config->outputFormat == 1)
			streamType->bsFormat=BSFORMAT_ADTS;
		else
			; // TODO
		break;
	case MPEG4:
		if (config->outputFormat == 1)
			streamType->bsFormat=BSFORMAT_ADTS_MP4;
		else
			streamType->bsFormat=BSFORMAT_RAW; // TODO
		break;
	}

	ctCfg->allowTns=config->useTns;
	format.bitRate=config->bitRate * handle->numChannels;
	// TODO: maybe, maybe not ... ctCfg->effectiveBandwidth=config->bandWidth;

	ctCfg->qualityMode=aacPlusEncQualityHighest;
	aacPlusEncIfStatusCode ret ;
	ret = aacPlusEncSetConfiguration(handle->ctEncoder, ctCfg);
	ret = aacPlusEncSetStreamType(handle->ctEncoder, streamType);
	ret = aacPlusEncSetOutputFormat(handle->ctEncoder, &format);

	return 0;
}


faacEncHandle FAACAPI faacEncOpen(unsigned long sampleRate,
																	unsigned int numChannels,
																	unsigned long *inputSamples,
																	unsigned long *maxOutputBytes)
{
	*maxOutputBytes = (6144/8)*numChannels;

	
	EncoderHandle *handle= new EncoderHandle;
		handle->numChannels=numChannels;
		handle->sampleRate = sampleRate;
	handle->ctEncoder = aacPlusEncOpen(sampleRate, numChannels,
		aacPlusEncInputFloat,
		1, // bAllowV2Features
		0 // bAllowOversampledSBR
		);

	handle->frameSize=4;
	if (handle->ctEncoder)
	{
		aacPlusEncStreamType *streamType = aacPlusEncGetCurrentStreamType(handle->ctEncoder);
		streamType->bsFormat=BSFORMAT_ADTS;
		aacPlusEncSetStreamType(handle->ctEncoder, streamType);

		aacPlusEncOutputFormat format;
		aacPlusEncGetOutputFormat(handle->ctEncoder, &format);
		format.channelMode=UNDEFINED_CHANNEL_MODE;
		format.sbrMode=UNDEFINED_SBR_MODE;
		format.signalType=DEFAULT_SIGNAL_TYPE;

		int feedBytes;
		aacPlusEncGetOptimumBufferFeed(handle->ctEncoder, &feedBytes);
		*inputSamples = feedBytes / (sizeof(float)*numChannels); 
	}
		
	return (faacEncHandle)handle;

}


int FAACAPI faacEncGetDecoderSpecificInfo(faacEncHandle hEncoder, unsigned char **ppBuffer,
																					unsigned long *pSizeOfDecoderSpecificInfo)
{
	EncoderHandle *handle = (EncoderHandle *)hEncoder;
	unsigned char *audioSpecConfig = (unsigned char *) malloc(32);
	memset(audioSpecConfig,0 , 32);
	unsigned int size;
	aacPlusEncGetMPEG4Config(handle->ctEncoder, audioSpecConfig, &size, TYPE_AUDIO_SPECIFIC_CONFIG);
	*pSizeOfDecoderSpecificInfo = size;
	*ppBuffer = audioSpecConfig;
	return 0;
}


int FAACAPI faacEncEncode(faacEncHandle hEncoder, int32_t * inputBuffer, unsigned int samplesInput,
													unsigned char *outputBuffer,
													unsigned int bufferSize)
{
	int32_t dummy[128];
	if (!inputBuffer)
		inputBuffer=dummy;
	EncoderHandle *handle = (EncoderHandle *)hEncoder;
	unsigned int bytesOut = bufferSize, bytesConsumed=0;
		samplesInput*=handle->frameSize;
		aacPlusEncIfStatusCode ret =aacPlusEncEncode(handle->ctEncoder, inputBuffer, samplesInput, &bytesConsumed, outputBuffer, &bytesOut, 0, 0, 0);
		switch(ret)
		{
		case APEI_FINISHED:
			return bytesOut;
		case APEI_OK:
			assert(bytesConsumed==samplesInput);
			return bytesOut;
		default:
			return -1;
		}
	
}

int FAACAPI faacEncClose(faacEncHandle hEncoder)
{
	EncoderHandle *handle = (EncoderHandle *)hEncoder;
	aacPlusEncClose(handle->ctEncoder);
	delete handle;
	return 0;
}

}