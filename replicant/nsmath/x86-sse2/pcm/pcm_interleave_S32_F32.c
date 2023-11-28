#include "nsmath/pcm.h"
#include <emmintrin.h>


void x86_SSE2_nsmath_pcm_Convert_S32_F32(float *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain)
{
	size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			*destination = (float)source[c][b] * gain;
			destination++;
		}
	}	
}

