#include "nsmath/pcm.h"
#include "foundation/error.h"
#include "pcm/pcm.h"
#include <math.h>
static bool Unconvertable(const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters)
{
	if (source_parameters->number_of_channels != destination_parameters->number_of_channels)
		return true;

	if (source_parameters->sample_rate != destination_parameters->sample_rate)
		return true;

	return false;
}

static bool IsSigned(const nsaudio::Parameters *parameters)
{
	return (parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED) == nsaudio::FORMAT_FLAG_SIGNED;
}

static int CreateInterleaver_IntToInt(nsmath_pcm_interleaver_t interleaver, const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters, double optional_gain)
{
	if (IsSigned(source_parameters) && source_parameters->bytes_per_sample == sizeof(int32_t))
	{
		/* converting from int32_t */

		if (IsSigned(destination_parameters) && destination_parameters->bytes_per_sample == sizeof(int16_t))
		{
			/* interleave int32_t to int16_t */
			if (optional_gain != 0)
			{
				switch(source_parameters->number_of_channels)
				{
				case 1:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_S16_gain_mono;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;						
				case 2:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_S16_gain_stereo;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;
				default:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_S16_gain;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;
				}
			}
			else if (destination_parameters->bits_per_sample > source_parameters->bits_per_sample)
			{
				/* need to pad bits */
				return NErr_NotImplemented;
			}
			else if (destination_parameters->bits_per_sample < source_parameters->bits_per_sample)
			{
				/* need to decimate bits */
				switch(source_parameters->number_of_channels)
				{
				case 1:
					interleaver->shift_bits = source_parameters->bits_per_sample - destination_parameters->bits_per_sample;
					if (interleaver->shift_bits == 8)
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_mono_decimate8;
						return NErr_Success;
					}
					else
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_mono_decimate;
						return NErr_Success;
					}
				case 2:
					interleaver->shift_bits = source_parameters->bits_per_sample - destination_parameters->bits_per_sample;
					if (interleaver->shift_bits == 8)
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_stereo_decimate8;
						return NErr_Success;
					}
					else
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_stereo_decimate;
						return NErr_Success;
					}
				default:
					interleaver->shift_bits = source_parameters->bits_per_sample - destination_parameters->bits_per_sample;
					if (interleaver->shift_bits == 8)
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_shift8;
						return NErr_Success;
					}
					else
					{
						interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S16_decimate;
						return NErr_Success;
					}
				}
			}
			else
			{
				/* straight conversion */
				switch(source_parameters->number_of_channels)
				{
				case 1:
					interleaver->interleave = (Interleaver)x86_SSE2_nsmath_pcm_Convert_S32_S16_mono;
					return NErr_Success;
				case 2:
					interleaver->interleave = (Interleaver)x86_SSE2_nsmath_pcm_Convert_S32_S16_stereo;
					return NErr_Success;
				default:						
					interleaver->interleave = (Interleaver)x86_SSE2_nsmath_pcm_Convert_S32_S16;
					return NErr_Success;
				}
			}
		}
		else if (IsSigned(destination_parameters) && destination_parameters->bytes_per_sample == sizeof(int32_t))
		{
			/* interleave int32_t to int32_t */
			if (optional_gain != 0)
			{
				switch(source_parameters->number_of_channels)
				{
				case 1:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_F32_mono;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;						
				case 2:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_F32_stereo;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;
				default:						
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_F32;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;
				}
			}
			else if (destination_parameters->bits_per_sample > source_parameters->bits_per_sample)
			{
				/* need to pad bits */
				return NErr_NotImplemented;
			}
			else if (destination_parameters->bits_per_sample < source_parameters->bits_per_sample)
			{
				/* need to decimate bits */
				switch(source_parameters->number_of_channels)
				{
				default:
					return NErr_NotImplemented;
				}
			}
			else
			{
				/* straight conversion */
				switch(source_parameters->number_of_channels)
				{
				default:
					return NErr_NotImplemented;
				}
			}
		}
		else if (IsSigned(destination_parameters) && destination_parameters->bytes_per_sample == 3)
		{
			/* interleave int32_t to 24bit packed */
			if (optional_gain != 0)
			{
				switch(source_parameters->number_of_channels)
				{
				default:
					interleaver->interleave_gain = (InterleaverGain)x86_SSE2_nsmath_pcm_Convert_S32_S24_gain;
					interleaver->gain = optional_gain * pow(2.0f, (float)(destination_parameters->bits_per_sample - source_parameters->bits_per_sample));
					return NErr_Success;
				}
			}
			else if (destination_parameters->bits_per_sample > source_parameters->bits_per_sample)
			{
				/* need to pad bits */
				interleaver->shift_bits =  destination_parameters->bits_per_sample - source_parameters->bits_per_sample;
				interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S24_pad;				
				return NErr_Success;
			}
			else if (destination_parameters->bits_per_sample < source_parameters->bits_per_sample)
			{
				/* need to decimate bits */
				switch(source_parameters->number_of_channels)
				{
				default:
					interleaver->shift_bits = source_parameters->bits_per_sample - destination_parameters->bits_per_sample;
					interleaver->interleave_shift = (InterleaverShift)x86_SSE2_nsmath_pcm_Convert_S32_S24_decimate;
					return NErr_Success;
				}
			}
			else
			{
				/* straight conversion */
				switch(source_parameters->number_of_channels)
				{
				default:						
					interleaver->interleave = (Interleaver)x86_SSE2_nsmath_pcm_Convert_S32_S24;
					return NErr_Success;
				}
			}
		}
	}

	return NErr_NotImplemented;
}

int nsmath_pcm_CreateInterleaver(nsmath_pcm_interleaver_t interleaver, const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters, double optional_gain)
{
	memset(interleaver, 0, sizeof(*interleaver));

	if (Unconvertable(source_parameters, destination_parameters))
		return NErr_NotImplemented;

	/* see what we're converting from */
	if (source_parameters->format_type == nsaudio::format_type_float)
	{
		/* currently no floating point interleavers are written */
		return NErr_NotImplemented;
	}
	else if (source_parameters->format_type == nsaudio::format_type_pcm)
	{
		if (destination_parameters->format_type == nsaudio::format_type_pcm)
			return CreateInterleaver_IntToInt(interleaver, source_parameters, destination_parameters, optional_gain);

	}

	return NErr_NotImplemented;
}

static bool NeedsConversion(const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters)
{
	if ((source_parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED) != (destination_parameters->format_flags & nsaudio::FORMAT_FLAG_VALID_SIGNED))
		return true;

	if (source_parameters->format_type != destination_parameters->format_type)
		return true;

	if (source_parameters->bits_per_sample != destination_parameters->bits_per_sample)
		return true;

	if (source_parameters->bytes_per_sample != destination_parameters->bytes_per_sample)
		return true;
	// TODO: check endian
	return false;
}

int nsmath_pcm_CreateConverter(nsmath_pcm_converter_t converter, const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters, double optional_gain)
{
	memset(converter, 0, sizeof(*converter));

	if (Unconvertable(source_parameters, destination_parameters))
		return NErr_NotImplemented;

	if (!NeedsConversion(source_parameters, destination_parameters))
		return NErr_DirectPointer;

	/* see what we're converting from */
	if (source_parameters->format_type == nsaudio::format_type_float)
	{
		if (IsSigned(source_parameters) && source_parameters->bytes_per_sample == sizeof(float))
		{
			if (IsSigned(destination_parameters) && destination_parameters->bytes_per_sample == sizeof(int16_t))
			{
				/* convert float to int16_t */
				converter->convert_gain = (ConverterGain)x86_SSE2_nsmath_pcm_Convert_F32_S16;
				if (optional_gain)
					converter->gain = optional_gain * 32768.0f;
				else
					converter->gain = 32768.0f;
				return NErr_Success;
			}
			else if (IsSigned(destination_parameters) && destination_parameters->bytes_per_sample == 3)
			{
				/* convert float to 24bit packed */
				converter->convert_gain = (ConverterGain)x86_SSE2_nsmath_pcm_Convert_F32_S24;
				if (optional_gain)
					converter->gain = optional_gain * 8388608.0f;
				else
					converter->gain = 8388608.0f;
				return NErr_Success;
			}
		}
	}

	return NErr_NotImplemented;
}