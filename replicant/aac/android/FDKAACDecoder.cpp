#include "FDKAACDecoder.h"
#include "foundation/error.h"
#include <android/log.h>
AACDECODER_CLOSE __aacDecoder_Close=0;
AACDECODER_CONFIGRAW __aacDecoder_ConfigRaw=0;
AACDECODER_DECODEFRAME __aacDecoder_DecodeFrame=0;
AACDECODER_FILL __aacDecoder_Fill=0;
AACDECODER_GETSTREAMINFO __aacDecoder_GetStreamInfo=0;
AACDECODER_OPEN __aacDecoder_Open=0;
AACDECODER_SETPARAM __aacDecoder_SetParam=0;

FDKAACDecoder::FDKAACDecoder()
{
	decoder=0;
	stream_info=0;
	need_reset=false;
}

FDKAACDecoder::~FDKAACDecoder()
{
	if (decoder)
		__aacDecoder_Close(decoder);
}

int FDKAACDecoder::Initialize(int transport)
{
	FDK_TRANSPORT_TYPE fdk_transport_type;
	if (transport == TYPE_ADTS_AAC)
		fdk_transport_type=FDK_TT_MP4_ADTS;
	else if (transport == TYPE_RAW_AAC)
		fdk_transport_type=FDK_TT_MP4_RAW;
	else
		return NErr_Unknown;

	decoder = __aacDecoder_Open(fdk_transport_type, 1);
	if (!decoder)
		return NErr_FailedCreate;

	return NErr_Success;
}

int FDKAACDecoder::Decode(const void *input_buffer, size_t input_buffer_length, int16_t *output_buffer, size_t *samples_decoded)
{
	
	uint32_t buffer_valid=input_buffer_length;
	FDK_AAC_DECODER_ERROR err = __aacDecoder_Fill(decoder, (uint8_t **)&input_buffer, &input_buffer_length, &buffer_valid);
	if (err != FDK_AAC_DEC_OK)
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Fill() error=0x%X", err);
	if (buffer_valid != 0)
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Fill() buffer_valid=%u", buffer_valid);

	uint32_t flags = 0;
	if (need_reset)
	{
		flags |= FDK_AACDEC_INTR;
		need_reset=false;
	}

	err = __aacDecoder_DecodeFrame(decoder, output_buffer, 4096*sizeof(int16_t), flags);	
	if (FDK_IS_OUTPUT_VALID(err))
	{
		if (!stream_info)
			stream_info=__aacDecoder_GetStreamInfo(decoder);
		
		*samples_decoded=stream_info->frameSize * stream_info->numChannels;
	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Decode() error=0x%X", err);
		*samples_decoded=0;
		return NErr_Error;
	}
	
	return NErr_Success;
	
}

int FDKAACDecoder::Configure(const void *asc, size_t asc_length)
{
	FDK_AAC_DECODER_ERROR err = __aacDecoder_ConfigRaw(decoder, (uint8_t **)&asc, &asc_length);
	if (err != FDK_AAC_DEC_OK)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Configure() error=0x%X", err);
		return NErr_Error;
	}

	return NErr_Success;
}

void FDKAACDecoder::Reset()
{
	need_reset=true;
}

int FDKAACDecoder::FillParameters(nsaudio::Parameters *parameters)
{
	if (!stream_info)
			stream_info=__aacDecoder_GetStreamInfo(decoder);

	if (!stream_info)
			return NErr_NeedMoreData;

	parameters->sample_rate = stream_info->sampleRate;
	parameters->format_type = nsaudio::format_type_pcm;
	parameters->format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->bytes_per_sample = 2;
	parameters->bits_per_sample = 16;
	parameters->number_of_channels = stream_info->numChannels;
	return NErr_Success;
}
