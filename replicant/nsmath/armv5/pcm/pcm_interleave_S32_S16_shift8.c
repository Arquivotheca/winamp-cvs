#include "nsmath/pcm.h"


#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

void armv5_vfp_nsmath_pcm_Convert_S32_S16_shift8(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
{
		size_t b, c;
	for (b = 0;b < count_per_channel;b++)
	{
		for (c = 0;c < channels;c++)
		{
			long samp = source[c][b] >> 8;
			PA_CLIP_( samp, -0x8000, 0x7FFF );
			*destination = samp;
			destination++;
		}
	}	
}

