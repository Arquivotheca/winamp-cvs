#include "nsmath/pcm.h"

#define min(a,b) ((a<b)?(a):(b))

#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }


void armv7a_vfp_nsmath_pcm_Convert_F32_S16(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	size_t i=0;
	for (i=0;i<sample_count;i++)
	{
		long samp = (long)((source[i])*gain);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		destination[i] = (int16_t) samp;
	}
}
