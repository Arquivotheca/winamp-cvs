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
*   $Id: band_nrg.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "band_nrg.h"

#ifndef P4_CODE
static void CalcBandEnergy_NoOpt(const float *mdctSpectrum,
                    const int   *bandOffset,
                    const int    numBands,
                    float       *bandEnergy) {
  int i, j;

  j = 0;
  for(i=0; i<numBands; i++) {
    bandEnergy[i] = 0.0f;
    while (j < bandOffset[i+1]) {
      bandEnergy[i] += mdctSpectrum[j] * mdctSpectrum[j];
      j++;
    }
  }
}

void (*CalcBandEnergy)(const float *mdctSpectrum,
		       const int   *bandOffset,
		       const int    numBands,
		       float       *bandEnergy) = CalcBandEnergy_NoOpt;
#endif	


void CalcBandEnergyFromLineEnergy(const float *lineEnergy,
                                  const int   *bandOffset,
                                  const int    numBands,
                                  float       *bandEnergy) {
  int i, j;
  j = 0;

  for(i=0; i<numBands; i++) {
    bandEnergy[i] = 0.0f;
    while (j < bandOffset[i+1]) {
      bandEnergy[i] += lineEnergy[j];
      j++;
    }
  }
}

#ifndef P4_CODE
static void CalcBandEnergyMS_NoOpt(const float *mdctSpectrumLeft,
				   const float *mdctSpectrumRight,
				   const int   *bandOffset,
				   const int    numBands,
				   float       *bandEnergyMid,
				   float       *bandEnergySide) {

  int i, j;

  j = 0;
  for(i=0; i<numBands; i++) {
    bandEnergyMid[i] = 0.0f;
    bandEnergySide[i] = 0.0f;
    while (j < bandOffset[i+1]) {
      bandEnergyMid[i]  += (mdctSpectrumLeft[j] + mdctSpectrumRight[j]) * (mdctSpectrumLeft[j] + mdctSpectrumRight[j]);
      bandEnergySide[i]  += (mdctSpectrumLeft[j] - mdctSpectrumRight[j]) * (mdctSpectrumLeft[j] - mdctSpectrumRight[j]);
      j++;
    }
    bandEnergyMid[i] *= 0.5;
    bandEnergySide[i] *= 0.5;
  }
}

void (*CalcBandEnergyMS) (const float *mdctSpectrumLeft,
			  const float *mdctSpectrumRight,
			  const int   *bandOffset,
			  const int    numBands,
			  float       *bandEnergyMid,
			  float       *bandEnergySide) = CalcBandEnergyMS_NoOpt;

#endif
