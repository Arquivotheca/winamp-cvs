#include "foundation/types.h"

#define min(a,b) ((a<b)?(a):(b))

#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

void nsmath_pcm_Convert_F32_S16_C(int16_t *destination, const float *source, size_t sample_count)
{
	float gain=32768.0f;
	while(sample_count--)
	{
		int32_t samp = (int32_t)((*source++) * gain);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*destination++ = (int16_t) samp;
	}
}
