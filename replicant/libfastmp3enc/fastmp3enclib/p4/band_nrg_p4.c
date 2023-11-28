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
*   $Id: band_nrg_p4.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mp3ifc.h"
#include "mconfig.h"
#include "mp3alloc.h"
#include "band_nrg.h"

#ifdef P4_INTRINSIC
#include <xmmintrin.h>
#endif


#ifdef P4_INTRINSIC
static void
CalcBandEnergy_Opt(const float *mdctSpectrum,
		   const int   *bandOffset,
		   const int    numBands,
		   float       *bandEnergy)
{
  int i, j;

__m128 mdctS;
__m128 bandE;
__m128 bandE1;
__m128 temp1, temp2, temp3;

j = 0;
   for(i=0; i<numBands; i++) {
	bandEnergy[i] = 0.0f;
	bandE1 = _mm_setzero_ps();
    while (j < bandOffset[i+1]-3) {
	  mdctS = _mm_loadu_ps(&mdctSpectrum[j]);
	  bandE = _mm_mul_ps(mdctS, mdctS);
	  bandE1 = _mm_add_ps(bandE1, bandE);
	  j+=4;
    }
	temp1 = _mm_shuffle_ps (bandE1, bandE1, _MM_SHUFFLE(0,1,2,3));
	temp2 = _mm_shuffle_ps (bandE1, bandE1, _MM_SHUFFLE(2,2,1,1));
	temp3 = _mm_shuffle_ps (bandE1, bandE1, _MM_SHUFFLE(1,2,1,2));
	bandE1 = _mm_add_ss(bandE1, _mm_add_ss(temp1, _mm_add_ss(temp2, temp3)));
	_mm_store_ss((float *)&bandEnergy[i], bandE1);
	if((bandOffset[i+1]-j)%4) {
	  for(j; j<bandOffset[i+1]; j++) { 
	    bandEnergy[i]  += mdctSpectrum[j] * mdctSpectrum[j];
	  }
	}
  }
}
#endif	


#ifdef P4_INTRINSIC
static void
CalcBandEnergyMS_Opt(const float *mdctSpectrumLeft,
		     const float *mdctSpectrumRight,
		     const int   *bandOffset,
		     const int    numBands,
		     float       *bandEnergyMid,
		     float       *bandEnergySide) 
{

  int i, j;

__m128 mdctSL;
__m128 mdctSR;
__m128 bandEM;
__m128 bandES;
__m128 bandEM1;
__m128 bandES1;
__m128 temp1, temp2, temp3;

j = 0;

   for(i=0; i<numBands; i++) {
    bandEnergyMid[i] = 0.0f;
    bandEnergySide[i] = 0.0f;
	bandEM1 = _mm_setzero_ps();
	bandES1 = _mm_setzero_ps();
    while (j < bandOffset[i+1]-3) {
	  mdctSL = _mm_loadu_ps(&mdctSpectrumLeft[j]);
	  mdctSR = _mm_loadu_ps(&mdctSpectrumRight[j]);
	  bandEM = _mm_mul_ps(_mm_add_ps(mdctSL, mdctSR), _mm_add_ps(mdctSL, mdctSR));
	  bandES = _mm_mul_ps(_mm_sub_ps(mdctSL, mdctSR), _mm_sub_ps(mdctSL, mdctSR));
	  bandEM1 = _mm_add_ps(bandEM1, bandEM);
	  bandES1 = _mm_add_ps(bandES1, bandES);
	  j+=4;
    }
	temp1 = _mm_shuffle_ps (bandEM1, bandEM1, _MM_SHUFFLE(0,1,2,3));
	temp2 = _mm_shuffle_ps (bandEM1, bandEM1, _MM_SHUFFLE(2,2,1,1));
	temp3 = _mm_shuffle_ps (bandEM1, bandEM1, _MM_SHUFFLE(1,2,1,2));
	bandEM1 = _mm_add_ss(bandEM1, _mm_add_ss(temp1, _mm_add_ss(temp2, temp3)));
	temp1 = _mm_shuffle_ps (bandES1, bandES1, _MM_SHUFFLE(0,1,2,3));
	temp2 = _mm_shuffle_ps (bandES1, bandES1, _MM_SHUFFLE(2,2,1,1));
	temp3 = _mm_shuffle_ps (bandES1, bandES1, _MM_SHUFFLE(1,2,1,2));
	bandES1 = _mm_add_ss(bandES1, _mm_add_ss(temp1, _mm_add_ss(temp2, temp3)));
	_mm_store_ss((float *)&bandEnergyMid[i], bandEM1);
	_mm_store_ss((float *)&bandEnergySide[i], bandES1);
	if((bandOffset[i+1]-j)%4) {
	  for(j; j<bandOffset[i+1]; j++) { 
	    bandEnergyMid[i]  += (mdctSpectrumLeft[j] + mdctSpectrumRight[j]) * (mdctSpectrumLeft[j] + mdctSpectrumRight[j]);
        bandEnergySide[i]  += (mdctSpectrumLeft[j] - mdctSpectrumRight[j]) * (mdctSpectrumLeft[j] - mdctSpectrumRight[j]);
	  }
	}
    bandEnergyMid[i] *= 0.5;
    bandEnergySide[i] *= 0.5;
  }
}


#ifdef P4_CODE
void (*CalcBandEnergy) (const float *mdctSpectrum,
			const int   *bandOffset,
			const int    numBands,
			float       *bandEnergy) = CalcBandEnergy_Opt;

void (*CalcBandEnergyMS)(const float *mdctSpectrumLeft,
			 const float *mdctSpectrumRight,
			 const int   *bandOffset,
			 const int    numBands,
			 float       *bandEnergyMid,
			 float       *bandEnergySide) = CalcBandEnergyMS_Opt;
#endif

extern void initBandNrgSSE(void);
void initBandNrgSSE(void)
{
  CalcBandEnergyMS  = CalcBandEnergyMS_Opt;
  CalcBandEnergy    = CalcBandEnergy_Opt;
}

#endif
