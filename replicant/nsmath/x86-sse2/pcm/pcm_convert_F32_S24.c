#include "nsmath/pcm.h"
#include <math.h>

static void clip(double *out, double a, double b)
{
	double x = *out;
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
	*out = x;
}

void x86_SSE2_nsmath_pcm_Convert_F32_S24(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	unsigned char *dest = (unsigned char*)destination;
	signed long temp;
	gain *= 256.0f;
	while ( sample_count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *source * gain;
		clip( &scaled, -2147483648., 2147483647.);
		temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);

		source++;
		dest += 3;
	}
}
