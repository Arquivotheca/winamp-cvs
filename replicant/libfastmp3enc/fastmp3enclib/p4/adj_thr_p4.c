/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: adj_thr_p4.c,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <math.h>

#include "adj_thr.h"
#include "line_pe.h"
#include "utillib.h"
#include "mathmac.h"

#ifdef P4_INTRINSIC
#include <xmmintrin.h>
#endif

static __inline float pow_1_4(float x)
{
   return (float)sqrt(sqrt(x));
}

/* loudness calculation (threshold to the power of redExp) */
#ifdef P4_INTRINSIC
static void calcThreshExp_Opt(float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
			      struct PSY_OUT *psyOut,
			      /* const int nChannels,*/
			      const int startCh,
			      const int endCh)
{   

  int ch, sfb;
  __m128 sfbThreshold_new;
  __m128 thrExp_new;

  for (ch=startCh; ch<endCh; ch++) {
    struct PSY_OUT_CHANNEL *psyOutChan = &(psyOut->psyOutChannel[ch]);
    for (sfb=0; sfb<psyOutChan->sfbActive - 4; sfb+=4) {
      sfbThreshold_new = _mm_load_ps(&psyOutChan->sfbThreshold[sfb]);
      thrExp_new = _mm_rsqrt_ps(_mm_rsqrt_ps(sfbThreshold_new));
      _mm_storeu_ps((float *)&thrExp[ch][sfb], thrExp_new);
    }
    for (sfb; sfb<psyOutChan->sfbActive; sfb++){
      thrExp[ch][sfb] = pow_1_4(psyOutChan->sfbThreshold[sfb]);
    }
  }
}

#ifdef P4_CODE
void (*calcThreshExp)(float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
		      struct PSY_OUT *psyOut,
		      /* const int nChannels,*/
		      const int startCh,
		      const int endCh) = calcThreshExp_Opt;
#endif /* #ifdef P4_CODE */

extern void initAdjThrSSE(void);
void initAdjThrSSE(void)
{
  calcThreshExp = calcThreshExp_Opt;
}
#endif /* #ifdef P4_INTINSIC */
