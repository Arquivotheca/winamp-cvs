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
*   $Id: line_pe.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"

#include <math.h>
#include "line_pe.h"
#include "mathmac.h"

static const float LOG2_1    = 1.442695041f;	 

#ifdef OLD_THR_REDUCTION
static const float minThreshold = 1.0E-36F;

int CalcPeSfbByLine(const float *mdctSpectrum,
                    const float *sfbEnergy,
                    const float *sfbThreshold, 
                    const int   *sfbOffset,
                    const int   sfbCnt)
{
   int i, j;
   float iSfbThreshold;
   float pe, peExp, sumPe;

   sumPe = 0.0f;

   /* Set highest SB PEs to zero (ideally run length coded) */
   for (i=sfbCnt-1; i>=0; i--)  {
      if (sfbThreshold[i] <= sfbEnergy[i])  break;
   } 

   for (; i>=0; i--) {
      iSfbThreshold = (sfbOffset[i+1]-sfbOffset[i]) /
                      (sfbThreshold[i] + minThreshold);
      /*
         Calculate pe by spec line. To avoid as many log()s as possible,
         accumulate the product of e^pe in peExp and only use log() when
         peExp goes over some bound.
       */
      pe = 0.0f;
      peExp = 1.0f;

      for(j=sfbOffset[i]; j<sfbOffset[i+1]; j++) {
         peExp *= (iSfbThreshold * mdctSpectrum[j] * mdctSpectrum[j] + 1.5f);
         if (peExp > 1.0E18f) {
            pe += (float)log(peExp); 
            peExp = 1.0f;
         }
      }
      pe += (float)log(peExp);
      pe *= LOG2_1;

      sumPe += pe;
   }

   return (int)(sumPe+0.5);
}

#else /* #ifdef OLD_THR_REDUCTION */

/* if the following is defined, log2() is only estimated */
#ifndef macintosh
#define USE_FAST_LOG2
#endif

static const float C1 = 3.0f;       /* log(8.0)/log(2) */
static const float C2 = 1.3219281f; /* log(2.5)/log(2) */
static const float C3 = 0.5593573f; /* 1-C2/C1 */

#ifndef P4_CODE
/* constants that do not change during successive pe calculations */
static void prepareSfbPe_NoOpt(PE_CHANNEL_DATA *peChanData,
			       const float *sfbEnergy,
			       const float *sfbThreshold,
			       const float *sfbFormFactor,
			       const int   *sfbOffset,
			       const int    sfbCnt,
			       const int    windowSequence)
{
   int sfb;
   int sfbWidth;
   float avgFormFactor;

   for (sfb=0; sfb<sfbCnt; sfb++) {
      if (sfbEnergy[sfb] > sfbThreshold[sfb]) {
         sfbWidth = sfbOffset[sfb+1] - sfbOffset[sfb];
         /* estimate number of active lines */
         avgFormFactor = (float)pow(sfbEnergy[sfb]/sfbWidth, 0.25f);
         peChanData->sfbNLines[sfb] = 
	    (int)(sfbFormFactor[sfb]/avgFormFactor+0.5f);
         /* ld(sfbEn) */
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

void (*prepareSfbPe) (PE_CHANNEL_DATA *peChanData,
		      const float *sfbEnergy,
		      const float *sfbThreshold,
		      const float *sfbFormFactor,
		      const int   *sfbOffset,
		      const int    sfbCnt,
		      const int    windowSequence) = prepareSfbPe_NoOpt;
#endif


/*
   formula for one sfb:
   pe = n * ld(en/thr),                if ld(en/thr) >= C1
   pe = n * (C2 + C3 * ld(en/thr)),    if ld(en/thr) <  C1
   n: estimated number of lines in sfb,
   ld(x) = log(x)/log(2)

   constPart is sfbPe without the threshold part n*ld(thr) or n*C3*ld(thr)
*/
void calcSfbPe(PE_CHANNEL_DATA *peChanData,
               const float *sfbEnergy,
               const float *sfbThreshold,
               const int    sfbCnt)
{
   int sfb;
   int nLines;
   float ldThr, ldRatio;

   peChanData->pe = (float)peChanData->offset;
   peChanData->constPart = (float)peChanData->offset;
   peChanData->nActiveLines = 0;
   for (sfb=0; sfb<sfbCnt; sfb++) {
      if (sfbEnergy[sfb] > sfbThreshold[sfb]) {
#ifndef USE_FAST_LOG2
         ldThr = (float)(log(sfbThreshold[sfb]) * LOG2_1);
#else
         ldThr = fastlog2(sfbThreshold[sfb]);
#endif
         ldRatio = peChanData->sfbLdEnergy[sfb] - ldThr;
         nLines = peChanData->sfbNLines[sfb];
         if (ldRatio >= C1) {
            peChanData->sfbPe[sfb] = nLines * ldRatio;
            peChanData->sfbConstPart[sfb] = nLines*peChanData->sfbLdEnergy[sfb];
         }
         else {
            peChanData->sfbPe[sfb] = nLines * (C2 + C3 * ldRatio);
            peChanData->sfbConstPart[sfb] = nLines * 
               (C2 + C3 * peChanData->sfbLdEnergy[sfb]);
            nLines = (int)(nLines * C3 + 0.5f);
         }
         peChanData->sfbNActiveLines[sfb] = nLines;
      }
      else {
         peChanData->sfbPe[sfb] = 0.0f;
         peChanData->sfbConstPart[sfb] = 0.0f;
         peChanData->sfbNActiveLines[sfb] = 0;
      }
      peChanData->pe += peChanData->sfbPe[sfb];
      peChanData->constPart += peChanData->sfbConstPart[sfb];
      peChanData->nActiveLines += peChanData->sfbNActiveLines[sfb];
   }
}

#endif /* #ifdef OLD_THR_REDUCTION */ 
