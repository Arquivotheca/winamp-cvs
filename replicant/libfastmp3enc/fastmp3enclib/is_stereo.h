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
*   $Id: is_stereo.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __IS_STEREO_H__
#define __IS_STEREO_H__

#include "psy_const.h"

int CalcIntensityPositions(const int    mpegVersion,
                           const float *mdctSpectrumLeft,
                           const float *mdctSpectrumRight,
                           const float *sfbEnergyLeft,
                           const float *sfbEnergyRight,
                           const float *sfbThresholdLeft,
                           const float *sfbThresholdRight,
                           const int   *sfbOffset,
                           const int    sfbLimitLow,
                           const int    sfbActive,
                           const int    fIsShort,
                           const float  D2max,
                           int         *sfbIsPosition,
                           const int   *sfbPreviousIsPosition,
                           float       *sfbIsDirX,
                           float       *sfbIsDirY,
                           float       *sfbIsCrossProduct);

void
IsStereoProcessing(const int    mpegVersion,
                   float       *sfbEnergyLeft,     /* modified above isLimit */
                   float       *sfbEnergyRight,    /* modified above isLimit */
                   float       *mdctSpectrumLeft,  /* modified above isLimit */
                   float       *mdctSpectrumRight, /* modified above isLimit */
                   float       *sfbThresholdLeft,  /* modified above isLimit */
                   float       *sfbThresholdRight, /* modified above isLimit */
                   const float *sfbIsDirX,
                   const float *sfbIsDirY,
                   const float *sfbIsCrossProduct,
                   const int    isLimit[TRANS_FAC],
                   const int    sfbActive,
                   const int   *sfbOffset,
                   const int    nWindows);

void
AdaptIntensityPositions(const int mpegVersion,
                        const int fLastBlockShort,
                        const int fThisBlockShort,
                        const int lastSfbIsPosition[],
                        int       thisSfbIsPosition[]);

#endif /* __IS_STEREO_H__ */
