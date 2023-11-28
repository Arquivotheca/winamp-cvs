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
*   $Id: band_nrg.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef BAND_NRG_H
#define BAND_NRG_H

extern void (*CalcBandEnergy) (const float *mdctSpectrum,
			       const int   *bandOffset,
			       const int    numBands,
			       float       *bandEnergy);


void CalcBandEnergyFromLineEnergy(const float *lineEnergy,
                                  const int   *bandOffset,
                                  const int    numBands,
                                  float       *bandEnergy);


extern void (*CalcBandEnergyMS)(const float *mdctSpectrumLeft,
				const float *mdctSpectrumRight,
				const int   *bandOffset,
				const int    numBands,
				float       *bandEnergyMid,
				float       *bandEnergySide);

#endif
