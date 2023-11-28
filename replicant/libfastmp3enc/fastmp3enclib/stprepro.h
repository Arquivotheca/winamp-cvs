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
*   $Id: stprepro.h,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __STPREPRO_H
#define __STPREPRO_H

struct STEREO_PREPRO;
typedef struct STEREO_PREPRO *HANDLE_STEREO_PREPRO;

#include "interface.h"

struct PSY_OUT;

struct STEREO_PREPRO {

  float   bitsPerFrameRaw;
  float   ConstAtt;
  float   stereoAttenuation;
  float   stereoAttenuationInc;
  float   stereoAttenuationDec;
  int     stereoAttenuationFlag;

/*   int     numStereoFacHist; */
/*   int     maxStereoFacHist; */
/*   float*  stereoFacHist; */
/*   float   avrgStereoFac; */
  float   avrgFreqEnergyM;
  float   avrgFreqEnergyS;

  float   stereoAttResol;
  float   stereoAttMax;
  
/*   float   minMSratio; */
  float*  stereoAttTable[2];

  float   avgStoM; /* dB */

  float   SMMin; /* dB */
  float   SMMid; /* dB */
  float   SMMax; /* dB */

  float   PeMin;
  float   PeCrit;
  float   PeImpactMax;

  float   smoothedPeSumSum;

  float   ImpactFactor;
};

int CreateStereoPreProcessing(HANDLE_STEREO_PREPRO* hStPrePro, 
                              int nChannels, int bitRate, int sampleRate,
                              int stPreProFlag);

void AdvanceStereoPreStep1(HANDLE_STEREO_PREPRO hStPrePro,  
                           const int nChannels,
                           float *timeData,
                           int granuleLen);

void AdvanceStereoPreStep2(struct PSY_OUT *hPout,
                           HANDLE_STEREO_PREPRO hStPrePro);

void DeleteStereoPreProcessing(HANDLE_STEREO_PREPRO hStPrePro);

#endif
