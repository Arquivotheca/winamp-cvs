#include <stdint.h>

#define min(a,b) ((a<b)?(a):(b))

#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

#ifndef __ARM_ARCH_7A__
void Float32_To_Int16_Clip(void *destinationBuffer, const float *src, size_t count)
{
	int16_t *dest = (signed short*)destinationBuffer;

	while(count--)
	{
		long samp = (long)((*src)*32768.0f);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (int16_t) samp;

		src ++;
		dest ++;
	}
}
#endif
void Int16_To_Float32(float *destination_buffer, const int16_t *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = (float)source_buffer[x] * gain;
	}
}

void Int32_To_Float32(float *destination_buffer, const int32_t *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = (float)source_buffer[x] * gain;
	}
}

void Float32_To_Float32(float *destination_buffer, const float *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = source_buffer[x] * gain;
	}
}

void Float64_To_Float32(float *destination_buffer, const float *source_buffer, size_t count, float gain)
{
	size_t x;
	for (x = 0; x < count; x ++)
	{
		destination_buffer[x] = source_buffer[x] * gain;
	}
}

void Int32_To_Float32_Deinterleave(float *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
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

void Int16_To_Float32_Deinterleave(float *destination_buffer, const int16_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
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

void Float32_To_Float32_Deinterleave(float *destination_buffer, const float **source_buffer, size_t channels, size_t count_per_channel, float gain)
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

void Float64_To_Float32_Deinterleave(float *destination_buffer, const double **source_buffer, size_t channels, size_t count_per_channel, float gain)
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