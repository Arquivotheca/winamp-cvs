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
*   $Id: ms_stereo.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __MS_STEREO_H__
#define __MS_STEREO_H__

#include "psy_const.h"
#include "psy_configuration.h"

int
MsStereoProcessing(const int    nWindows,
                   float       *sfbEnergyLeft,     /* modified below isLimit */
                   float       *sfbEnergyRight,    /* modified below isLimit */
                   const float *sfbEnergyMid,
                   const float *sfbEnergySide,
                   float       *mdctSpectrumLeft,  /* modified below isLimit */
                   float       *mdctSpectrumRight, /* modified below isLimit */
                   float       *sfbThresholdLeft,  /* modified below isLimit */
                   float       *sfbThresholdRight, /* modified below isLimit */
                   int         *oldMsFlag, /* updated with current msFlag */
                   const int    granule,
                   const int    isLimit[TRANS_FAC],
                   const int   *sfbOffsets);

void
MSGainPreprocessing(float *mdctLeft,
                    float *mdctRight,
                    float dampingFactor,
                    int nLines);

void
AdaptMSGainPreprocessing(const int    isLimit,
                         const PSY_CONFIGURATION *psyConf,
                         const float *sfbEnergyMid,
                         const float *sfbEnergySide,
                         float *dampingFactor);
#endif
