#include "PVMP4.h"
#include "foundation/error.h"

#include <stdlib.h>
#include <android/log.h>
#include <stdio.h>
#include "foundation/align.h"


GETMEMFUNC __AACDecoderGetMemoryRequirements=0;
AACIPPFUNC __AACDecoderInit=0, __AACDecoderDecode=0, __AACDecoderConfig=0;
AACPFUNC __AACDecoderReset=0;


PVAACDecoder::PVAACDecoder()
{
	memset(&decoder, 0, sizeof(decoder));
	decoder_memory=0;
	decoder_memory_size=0;
	decoder_config=0;
	decoder_config_size=0;
}

int PVAACDecoder::Initialize(int desired_channels)
{
	unsigned int memory_requirements;
	
	int ret = AACDecoderLibraryInit();
	if (ret != NErr_Success)
		return ret;

	/* the android decoder requires us to give it some scratch memory */
	decoder_memory_size = __AACDecoderGetMemoryRequirements();
	decoder_memory = malloc(decoder_memory_size);
	if (!decoder_memory)
		return NErr_OutOfMemory;
	
	memset(decoder_memory, 0, decoder_memory_size);
	decoder.inputBufferMaxLength = PVMP4AUDIODECODER_INBUFSIZE;
	decoder.outputFormat         = OUTPUTFORMAT_16PCM_INTERLEAVED;
	decoder.desiredChannels      = desired_channels;
	decoder.aacPlusEnabled       = 1;
	
	ret = __AACDecoderInit(&decoder, decoder_memory);
	if (ret != 0)
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] init return = %d", ret);

	return NErr_Success;
}

int PVAACDecoder::Decode(const void *input_buffer, size_t input_buffer_length, int16_t *output_buffer, size_t *samples_decoded)
{
	decoder.pOutputBuffer        = &output_buffer[0];
	decoder.pOutputBuffer_plus   = &output_buffer[2048];
	
	decoder.pInputBuffer         = (uint8_t *)input_buffer;
	decoder.inputBufferCurrentLength = input_buffer_length;
	decoder.inputBufferUsedLength = 0;
	if (__AACDecoderDecode(&decoder, decoder_memory) == 0)
	{
		size_t decoded = 1024 * decoder.desiredChannels;
	
		if (decoder.aacPlusUpsamplingFactor == 2)
			decoded *= 2;
		*samples_decoded = decoded;
		return NErr_Success;
	}
	else 
		return NErr_Error;
}

PVAACDecoder::~PVAACDecoder()
{
	free(decoder_memory);
	free(decoder_config);
}

int PVAACDecoder::Configure(const void *asc, size_t asc_length)
{
	if (!decoder_config)
	{
		void *config = malloc(asc_length);
		if (!config)
			return NErr_OutOfMemory;
		decoder_config = config;
		decoder_config_size = asc_length;
		memcpy(decoder_config, asc, asc_length);
	}
	decoder.pInputBuffer         = (uint8_t *)asc;
	decoder.inputBufferCurrentLength = asc_length;
	decoder.inputBufferUsedLength = 0;

	int status = __AACDecoderConfig(&decoder, decoder_memory);
	if (status != 0)
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] config status = %d", status);
	return NErr_Success;
}

void PVAACDecoder::Reset()
{
	if (__AACDecoderReset)
	{
		__AACDecoderReset(decoder_memory);
	}
	else
	{
		unsigned int desired_channels = decoder.desiredChannels;
		memset(&decoder, 0, sizeof(tPVMP4AudioDecoderExternal));
		memset(decoder_memory, 0, decoder_memory_size);
		decoder.inputBufferMaxLength = PVMP4AUDIODECODER_INBUFSIZE;
		decoder.outputFormat         = OUTPUTFORMAT_16PCM_INTERLEAVED;
		decoder.desiredChannels      = desired_channels;
		decoder.aacPlusEnabled       = 1;
		__AACDecoderInit(&decoder, decoder_memory);
		if (decoder_config)
			Configure(decoder_config, decoder_config_size);
	}
}

int PVAACDecoder::FillParameters(nsaudio::Parameters *parameters)
{
	parameters->sample_rate = decoder.samplingRate;
	parameters->format_type = nsaudio::format_type_pcm;
	parameters->format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->bytes_per_sample = 2;
	parameters->bits_per_sample = 16;
	parameters->number_of_channels = decoder.desiredChannels;
	return NErr_Success;
}