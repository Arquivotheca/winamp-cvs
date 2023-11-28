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
*   $Id: line_pe_p4.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <math.h>
#include "mconfig.h"
#include "mp3alloc.h"
#include "line_pe.h"
#include "mathmac.h"

#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif

#if defined P4_INTRINSIC && !defined(__GNUC__) || (GCC_VERSION>30202)
#include <xmmintrin.h>
#include "emmintrin.h"

#define pM32(a) ((float*)(a))

#ifndef macintosh
#define USE_FAST_LOG2
#endif

static const float LOG2_1    = 1.442695041f;	 
static const float C1 = 3.0f;       /* log(8.0)/log(2) */
static const float C2 = 1.3219281f; /* log(2.5)/log(2) */
static const float C3 = 0.5593573f; /* 1-C2/C1 */


static void 
prepareSfbPe_Opt(PE_CHANNEL_DATA *peChanData,
		 const float *sfbEnergy,
		 const float *sfbThreshold,
		 const float *sfbFormFactor,
		 const int   *sfbOffset,
		 const int    sfbCnt,
		 const int    windowSequence)
{
   int sfb, i;
   ALIGN_16_BYTE float sfbWidth_new[MAX_SFB];
   float avgFormFactor_old;
   __m128 ratio;
   __m128 avgFormFactor_new;
   __m128 sfbFormFac;
   __m128 sfbWidth;
   __m128 sfbEnergy_new;
 
   ALIGN_16_BYTE union{
   __m128 F;
   __m128i I;
   }sfbNLin;

   for (sfb=0; sfb<sfbCnt; sfb++) {
	  sfbWidth_new[sfb] = (float)(sfbOffset[sfb+1] - sfbOffset[sfb]);
   }

   i=0;
   for (sfb=0; sfb<sfbCnt-4; sfb+=4) {	   
		 sfbWidth = _mm_load_ps(&sfbWidth_new[sfb]);
		 sfbEnergy_new = _mm_load_ps(&sfbEnergy[sfb]);
		 ratio = _mm_mul_ps(sfbEnergy_new, _mm_rcp_ps(sfbWidth));
	     avgFormFactor_new = _mm_rsqrt_ps(ratio);
	     avgFormFactor_new = _mm_rsqrt_ps(avgFormFactor_new);
	     avgFormFactor_new = _mm_rcp_ps(avgFormFactor_new);
	     sfbFormFac = _mm_loadu_ps(&sfbFormFactor[sfb]);
	     sfbNLin.F = _mm_mul_ps(sfbFormFac,avgFormFactor_new);
	     sfbNLin.I = _mm_cvtps_epi32(sfbNLin.F);
	     _mm_store_ps((float *)&peChanData->sfbNLines[i], sfbNLin.F);
	     i = i+4;	   
   }

   for (sfb; sfb<sfbCnt; sfb++){
	  if (sfbEnergy[sfb] > sfbThreshold[sfb]) {
		  avgFormFactor_old = (float)pow(sfbEnergy[sfb]/sfbWidth_new[sfb], 0.25f);
	      peChanData->sfbNLines[sfb] = (int)(sfbFormFactor[sfb]/avgFormFactor_old+0.5f);
		  }
      else {
		  peChanData->sfbNLines[sfb] = 0;
      }
   }

   for (sfb=0; sfb<sfbCnt; sfb++) {
      if (sfbEnergy[sfb] > sfbThreshold[sfb]) {
#ifndef USE_FAST_LOG2
         peChanData->sfbLdEnergy[sfb] = (float)log(sfbEnergy[sfb]) * LOG2_1; 
#else
         peChanData->sfbLdEnergy[sfb] = fastlog2(sfbEnergy[sfb]); 
#endif
      }
      else {
         peChanData->sfbNLines[sfb] = 0;
         peChanData->sfbLdEnergy[sfb] = 0.0f;
      }
   }
   if (windowSequence != SHORT_WINDOW) 
      peChanData->offset = 0;
   else 
      peChanData->offset = 125;
}


#ifdef P4_CODE
void (*prepareSfbPe) (PE_CHANNEL_DATA *peChanData,
		      const float *sfbEnergy,
		      const float *sfbThreshold,
		      const float *sfbFormFactor,
		      const int   *sfbOffset,
		      const int    sfbCnt,
		      const int    windowSequence) = prepareSfbPe_Opt;
#endif
#endif

void initLinePeSSE(void) 
{
#if !defined(__GNUC__) || (GCC_VERSION>30202)
    prepareSfbPe      = prepareSfbPe_Opt;
#endif
}
