/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 2002-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: sf_estim_p4.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "mconfig.h"
#include "sf_estim.h"
#include "sf_cmprs.h"
#include "interface.h"
#include "mathmac.h"
#include "mathlib.h"
#include "utillib.h"
#include "mp3alloc.h"

#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif


#if defined P4_INTRINSIC

#include "xmmintrin.h"
#if !defined(__GNUC__) || (GCC_VERSION>30202)
#include "emmintrin.h"
#else
typedef int __m128i __attribute__ ((__mode__(__V4SI__)));
#endif

typedef union 
{
  __m128i I;
  __m128  F;
  int    i[4];
} myAbs_mask;

static void 
CalcFormFactorChannel_Opt(
                          float * restrict sfbFormFactor,
                          float * restrict sfbMaxSpec,
                          const struct PSY_OUT_CHANNEL *psyOutChan )
{
  int i, j;

  /* calc sum of sqrt(spec) */

	float maxSpec, ep1, ep2 ;

	ALIGN_16_BYTE float sqrt_val[576];
	ALIGN_16_BYTE float abs_val[576];

	__m128 tmp1, tmp2, tmp3, tmp4, tmp5;
	__m128 min_val;
        myAbs_mask abs_mask;

        abs_mask.i[0] = 0x7fffffffUL;
        abs_mask.i[1] = 0x7fffffffUL;
        abs_mask.i[2] = 0x7fffffffUL;
        abs_mask.i[3] = 0x7fffffffUL;
	/* abs_mask.I = _mm_set1_epi32(0x7fffffff); */
	min_val = _mm_set1_ps(FLT_MIN);

	/* calculate at first all <abs> and <sqrt> values */
	for(i=0;i<576;i+=4) {
		tmp1 = _mm_load_ps(&psyOutChan->mdctSpectrum[i]);
		tmp2 = _mm_and_ps(tmp1, abs_mask.F);
		_mm_store_ps(&abs_val[i], tmp2);
		tmp3 = _mm_add_ps(tmp2,min_val);
		tmp4 = _mm_rsqrt_ps(tmp3);
		tmp5 = _mm_mul_ps(tmp3, tmp4);
		/* tmp5 = _mm_sqrt_ps(tmp2); */
		_mm_store_ps(&sqrt_val[i], tmp5);
	}

	for (i=0; i<psyOutChan->sfbActive; i++)
	{
		maxSpec = ep1 = ep2 = 0.0f;
		for(j = psyOutChan->sfbOffsets[i]; j < psyOutChan->sfbOffsets[i+1]; j+=2)
		{
			ep1 += sqrt_val[j];
			if (abs_val[j] > maxSpec)
				maxSpec = abs_val[j];

			ep2 += sqrt_val[j+1];
			if (abs_val[j+1] > maxSpec)
				maxSpec = abs_val[j+1];
		}
		sfbFormFactor[i] = 1e-10f + ep1 + ep2;
		sfbMaxSpec[i] = maxSpec;
	}
}

#ifdef P4_CODE
void (*CalcFormFactorChannel) (float * restrict sfbFormFactor,
                               float * restrict sfbMaxSpec,
                               const struct PSY_OUT_CHANNEL *psyOutChan ) = CalcFormFactorChannel_Opt;

#endif
#endif

void initSfEstimSSE2();
void initSfEstimSSE2()
{
#if !defined(__GNUC__) || (GCC_VERSION>30202)
  CalcFormFactorChannel = CalcFormFactorChannel_Opt;
#endif
}
