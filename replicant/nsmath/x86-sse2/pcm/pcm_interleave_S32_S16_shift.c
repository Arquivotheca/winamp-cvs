#include "nsmath/pcm.h"
#include <emmintrin.h>

#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

void x86_SSE2_nsmath_pcm_Convert_S32_S16_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
{
		size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = source[c][b] >> decimation_bits;
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination = samp;
			destination++;
		}
	}	
}

