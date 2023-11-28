#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif //_WIN32

/* TODO: move to nsmath */

#define min(a,b) ((a<b)?(a):(b))

#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }


void Codec_Float32_To_Int16_Clip(int16_t *destinationBuffer, const float *src, size_t count, float gain)
{
	int16_t *dest = (signed short*)destinationBuffer;
	gain *= 32768.0f;
	while(count--)
	{
		long samp = (long)((*src)*gain);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (int16_t) samp;

		src ++;
		dest ++;
	}
}

void Codec_Int16_To_Float32(float *destination_buffer, const int16_t *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = (float)source_buffer[x] * gain;
	}
}

void Codec_Int32_To_Float32(float *destination_buffer, const int32_t *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = (float)source_buffer[x] * gain;
	}
}

void Codec_Float32_To_Float32(float *destination_buffer, const float *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = source_buffer[x] * gain;
	}
}

void Codec_Float64_To_Float32(float *destination_buffer, const float *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = source_buffer[x] * gain;
	}
}

void Codec_Int32_To_Int16_Interleave(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = (long)source_buffer[c][b];
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination_buffer = (int16_t)samp;
			destination_buffer++;
		}
	}

}
void Codec_Int32_To_Int16_Interleave_Gain(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = (long)((float)source_buffer[c][b] * gain);
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination_buffer = (int16_t)samp;
			destination_buffer++;
		}
	}
}

void Codec_Int32_To_Int16_Interleave_Decimate(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = (long)(source_buffer[c][b] >> decimation_bits);
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination_buffer = (int16_t)samp;
			destination_buffer++;
		}
	}
}

void Codec_Int32_To_Int16_Interleave_Pad(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = (long)(source_buffer[c][b] << decimation_bits);
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination_buffer = (int16_t)samp;
			destination_buffer++;
		}
	}
}

void Codec_Int32_To_Float32_Interleave(float *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			*destination_buffer = (float)source_buffer[c][b] * gain;
			destination_buffer++;
		}
	}
}

void Codec_Int16_To_Float32_Interleave(float *destination_buffer, const int16_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			*destination_buffer = (float)source_buffer[c][b] * gain;
			destination_buffer++;
		}
	}
}

void Codec_Float32_To_Float32_Interleave(float *destination_buffer, const float **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			*destination_buffer = source_buffer[c][b] * gain;
			destination_buffer++;
		}
	}
}

void Codec_Float64_To_Float32_Interleave(float *destination_buffer, const double **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			*destination_buffer = source_buffer[c][b] * gain;
			destination_buffer++;
		}
	}
}