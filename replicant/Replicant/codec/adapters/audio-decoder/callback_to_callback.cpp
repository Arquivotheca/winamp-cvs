#include "callback_to_callback.h"
#include "callback_to_callback_interleave.h"
#include "callback_to_callback_interleave_gain.h"
#include "callback_to_callback_interleave_decimate.h"
#include "callback_to_callback_convert.h"
#include "foundation/guid.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

/* do some basic sanity checks for things we don't support */
static int SupportedAudioParameters(const nsaudio::Parameters *parameters)
{
	/* empty flags is an error */
	if (parameters->format_flags == 0)
		return NErr_BadParameter;

	/* make sure there's no flags we don't understand */
	if (parameters->format_flags & ~nsaudio::FORMAT_FLAG_VALID_MASK)
		return NErr_IncompatibleVersion;

	/* we don't currently support big endian */
	if (parameters->format_flags & nsaudio::FORMAT_FLAG_BIG_ENDIAN)
		return NErr_NotImplemented;

	/* we don't currently support unsigned PCM */
	if (parameters->format_flags & nsaudio::FORMAT_FLAG_UNSIGNED)
		return NErr_NotImplemented;

	/* we only support PCM and float */
	if (parameters->format_type != nsaudio::format_type_pcm && parameters->format_type != nsaudio::format_type_float)
		return NErr_NotImplemented;

	/* for floating point, we can support 32bit (single) and 64bit (double), but bits_per_sample must match */
	if (parameters->format_type == nsaudio::format_type_float && parameters->bits_per_sample != parameters->bytes_per_sample*8)
		return NErr_NotImplemented;

	/* sanity check some values.  note that some of these might well be valid for other format_type values (e.g. pass-thru AC3) */
	if (parameters->number_of_channels == 0)
		return NErr_BadParameter;

	if (parameters->bits_per_sample == 0)
		return NErr_BadParameter;

	/* make sure the bytes_per_sample is big enough for the bits_per_sample */
	if (parameters->bits_per_sample > parameters->bytes_per_sample*8)
		return NErr_BadParameter;

	return NErr_Success;
}

static int SupportedAudioParameters_Destination(const nsaudio::Parameters *parameters)
{
	/* TODO: verify parameters, but we need to allow for some values to be 0 */

	/* we only support PCM and float (or unspecified) */
	if (parameters->format_type != INVALID_GUID && parameters->format_type != nsaudio::format_type_pcm && parameters->format_type != nsaudio::format_type_float)
		return NErr_NotImplemented;

	/* we don't currently support big endian */
	if (parameters->format_flags & nsaudio::FORMAT_FLAG_BIG_ENDIAN)
		return NErr_NotImplemented;

	/* we don't currently support unsigned PCM */
	if (parameters->format_flags & nsaudio::FORMAT_FLAG_UNSIGNED)
		return NErr_NotImplemented;

	return NErr_Success;
}


static bool NeedConversion(const nsaudio::Parameters *parameters, const nsaudio::Parameters *source_parameters)
{
	if (parameters->sample_rate && parameters->sample_rate != source_parameters->sample_rate)
		return true;
	if (parameters->format_type != INVALID_GUID && parameters->format_type != source_parameters->format_type)
		return true;
	if ((parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_INTERLEAVE) && (parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_INTERLEAVE) != (source_parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_INTERLEAVE))
		return true;
	if ((parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_ENDIAN) && (parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_ENDIAN) != (source_parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_ENDIAN))
		return true;
	if ((parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED) && (parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED) != (source_parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED))
		return true;
	if (parameters->bytes_per_sample && parameters->bytes_per_sample != source_parameters->bytes_per_sample)
		return true;
	if (parameters->bits_per_sample && parameters->bits_per_sample != source_parameters->bits_per_sample)
		return true;
	if (parameters->number_of_channels && parameters->number_of_channels != source_parameters->number_of_channels)
		return true;
	if (parameters->channel_layout && parameters->channel_layout != source_parameters->channel_layout)
		return true;

	return false;
}

int AudioDecoderAdapter_CallbackToCallback::CreateInterleaver(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters)
{
	double gain=0;
	InterleaverGain interleaver_gain=0;

	unsigned int shift_bits=0;
	InterleaverShift interleaver_shift=0;

	Interleaver interleaver=0;

	int ret;

	/* check if it's Int-to-Float */
	if (parameters->format_type == nsaudio::format_type_float
		&& source_parameters->format_type == nsaudio::format_type_pcm)
	{
		if (!parameters->bytes_per_sample)
		{
			parameters->bytes_per_sample=4;
			parameters->bits_per_sample=32;
		}

		/* currently only support conversion to 32bit float (not 64) */
		if (parameters->bytes_per_sample != 4)
			return NErr_NotImplemented;

		/* find an appropriate interleaver function */
		switch(source_parameters->bytes_per_sample)
		{
		case sizeof(int16_t):
			interleaver_gain = (InterleaverGain)Codec_Int16_To_Float32_Interleave;
			break;				
		case sizeof(int32_t):
			interleaver_gain = (InterleaverGain)Codec_Int32_To_Float32_Interleave;
			break;
		default:
			return NErr_NotImplemented;
		}

		/* compute the gain */
		gain = 1.0/(double)(1 << (source_parameters->bits_per_sample-1));
	}
	else if (parameters->format_type == nsaudio::format_type_pcm
		&& source_parameters->format_type == nsaudio::format_type_pcm)
	{
		if (!parameters->bytes_per_sample)
		{
			parameters->bytes_per_sample=2;
			parameters->bits_per_sample=16;
		}

		switch(parameters->bytes_per_sample)
		{
		case sizeof(int16_t):
			if (source_parameters->bits_per_sample > parameters->bits_per_sample)
			{
				interleaver_shift = (InterleaverShift)Codec_Int32_To_Int16_Interleave_Decimate;
				shift_bits = source_parameters->bits_per_sample-parameters->bits_per_sample;
			}
			else if (source_parameters->bits_per_sample < parameters->bits_per_sample)
			{
				interleaver_shift = (InterleaverShift)Codec_Int32_To_Int16_Interleave_Pad;
				shift_bits = parameters->bits_per_sample-source_parameters->bits_per_sample;
			}
			else
			{
				interleaver = (Interleaver)Codec_Int32_To_Int16_Interleave;
			}			

			break;
		default:
			return NErr_NotImplemented;
		}


	}
	else
		return NErr_NotImplemented;


	if (interleaver_gain)
	{
		AudioDecoderAdapter_CallbackToCallback_InterleaveGain *out = new (std::nothrow)ReferenceCounted<AudioDecoderAdapter_CallbackToCallback_InterleaveGain>;
		if (!out)
			return NErr_OutOfMemory;

		ret = out->Initialize(parameters, source_decoder, interleaver_gain, gain);
		if (ret != NErr_Success)
		{
			out->ifc_audio_decoder_callback::Release();
			return ret;
		}

		*out_decoder = out;
	}
	else if (interleaver_shift)
	{
		AudioDecoderAdapter_CallbackToCallback_InterleaveShift *out = new (std::nothrow)ReferenceCounted<AudioDecoderAdapter_CallbackToCallback_InterleaveShift>;
		if (!out)
			return NErr_OutOfMemory;

		ret = out->Initialize(parameters, source_decoder, interleaver_shift, shift_bits);
		if (ret != NErr_Success)
		{
			out->ifc_audio_decoder_callback::Release();
			return ret;
		}

		*out_decoder = out;
	}
	else if (interleaver)
	{
		AudioDecoderAdapter_CallbackToCallback_Interleave *out = new (std::nothrow)ReferenceCounted<AudioDecoderAdapter_CallbackToCallback_Interleave>;
		if (!out)
			return NErr_OutOfMemory;

		ret = out->Initialize(parameters, source_decoder, interleaver);
		if (ret != NErr_Success)
		{
			out->ifc_audio_decoder_callback::Release();
			return ret;
		}

		*out_decoder = out;
	}
	else
	{
		return NErr_NotImplemented;
	}

	return NErr_Success;			
}

int AudioDecoderAdapter_CallbackToCallback::CreateConverter(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters)
{
	double gain=0;
	ConverterGain converter=0;
	int ret;

	if (parameters->format_type == nsaudio::format_type_pcm
		&& source_parameters->format_type == nsaudio::format_type_float)
	{
		/* Float-to-Int */
		if (!parameters->bytes_per_sample)
		{
			parameters->bytes_per_sample=2;
			parameters->bits_per_sample=16;
		}

		/* currently only support conversion to 16bit PCM */

		/* find an appropriate converter function */
		switch(parameters->bytes_per_sample)
		{
		case sizeof(int16_t):
			converter = (ConverterGain)Codec_Float32_To_Int16_Clip;
			gain=1.0;
			break;				
		default:
			return NErr_NotImplemented;
		}

	}
	else if (parameters->format_type == nsaudio::format_type_float
		&& source_parameters->format_type == nsaudio::format_type_pcm)
	{
		/* Int-to-Float */
		if (!parameters->bytes_per_sample)
		{
			parameters->bytes_per_sample=4;
			parameters->bits_per_sample=32;
		}

		/* currently only support conversion to 32bit float */
		if (parameters->bytes_per_sample != 4)
			return NErr_NotImplemented;

		/* currently only support conversion from 16bit PCM */

		/* find an appropriate converter function */
		switch(source_parameters->bytes_per_sample)
		{
		case sizeof(int16_t):
			converter = (ConverterGain)Codec_Int16_To_Float32;
			gain=32768.0;
			break;				
		default:
			return NErr_NotImplemented;
		}
	}
	else
		return NErr_NotImplemented;

	/* compute the gain */
	AudioDecoderAdapter_CallbackToCallback_Convert *out = new (std::nothrow)ReferenceCounted<AudioDecoderAdapter_CallbackToCallback_Convert>;
	if (!out)
		return NErr_OutOfMemory;

	ret = out->Initialize(parameters, source_decoder, converter, gain);
	if (ret != NErr_Success)
	{
		out->ifc_audio_decoder_callback::Release();
		return ret;
	}

	*out_decoder = out;
	return NErr_Success;	
}

int AudioDecoderAdapter_CallbackToCallback::Create(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters)
{
	int ret;

	/* sanity check source parameters */
	ret = SupportedAudioParameters(source_parameters);
	if (ret != NErr_Success)
		return ret;

	if (!NeedConversion(parameters, source_parameters))
	{
		memcpy(parameters, source_parameters, sizeof(nsaudio::Parameters));
		*out_decoder = source_decoder;
		source_decoder->Retain();
		return NErr_Success;
	}

	ret = SupportedAudioParameters_Destination(parameters);
	if (ret != NErr_Success)
		return ret;

	if (parameters->sample_rate == 0) /* they don't care about the sample rate */
	{
		parameters->sample_rate = source_parameters->sample_rate;
	}
	else
	{
		/* we don't currently support sample rate conversion, so check */
		if (parameters->sample_rate != source_parameters->sample_rate)
		{
			return NErr_NotImplemented;
		}
	}

	if (parameters->number_of_channels == 0) /* they don't care about channel count */
	{
		parameters->number_of_channels = source_parameters->number_of_channels;
	}
	else
	{
		/* we don't currently support upmixing or downmixing, so check */
		if (parameters->number_of_channels != source_parameters->number_of_channels)
		{
			return NErr_NotImplemented;
		}
	}

	/* TODO: there's a lot more logic needed to be implemented.  I'm only going to do the known use cases 
	some of this might be able to be table-driven, too */

	/* check if we need to interleave */
	if ((parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_INTERLEAVE) == nsaudio::FORMAT_FLAG_INTERLEAVED
		&& (source_parameters->format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED))
	{
		return CreateInterleaver(out_decoder, parameters, flags, source_decoder, source_parameters);
	}
	else if ((parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_INTERLEAVE) == nsaudio::FORMAT_FLAG_INTERLEAVED
		&& (source_parameters->format_flags & nsaudio::FORMAT_FLAG_INTERLEAVED))
	{
		/* going from interleaved-to-interleaved, but need conversion */
		return CreateConverter(out_decoder, parameters, flags, source_decoder, source_parameters);


	}

	return NErr_NotImplemented;

}
