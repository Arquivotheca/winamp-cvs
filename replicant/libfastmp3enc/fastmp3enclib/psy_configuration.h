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
*   $Id: psy_configuration.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PSY_CONFIGURATION_H
#define _PSY_CONFIGURATION_H

#include "psy_const.h"
#include "spreading.h"
#include "mp3alloc.h"

typedef struct{

  int pbCnt;       /* number of psych bands required to cover entire spectrum */
  int pbActive;    /* number of psych bands containing energy after lowpass operation */
  ALIGN_16_BYTE int pbOffset[MAX_PB+1];

  int sfbCnt;      /* number of sf bands required to cover entire spectrum */
  int sfbActive;   /* number of sf bands containing energy after lowpass */
  ALIGN_16_BYTE int sfbOffset[MAX_SFB+1];

  int nWindows ;   /* TRANS_FAC for short windows, 1 else */
  int mpegVersion ; /* 0=MPEG-1, 1=MPEG-2, 2=MPEG-2.5 */

  int   useMS;
  int   useMsPreprocessing ; /* try to always achieve the same M/S ratio */
  float msRatioWanted ;      /* M/S ratio targeted */
  float msPreprocessingK ;   /* speed of adaptation */

  int   useIntensity;
  float isD2max;   /* IS stereo max distortion measure */
  int   isLimitLow;/* do not attempt intensity below this sfb */

  int   cmLines;                  /* chaos measure */
  float toneDefault;

  float *pSpreadingFunction;        /* spreading function*/
  ALIGN_16_BYTE float pbSpreadNorm[MAX_PB];
  ALIGN_16_BYTE int   pbBarcVal[MAX_PB];
  int   barcValScaling;

  ALIGN_16_BYTE float pbMinRatio[MAX_PB];
  float noiseRatio;
  float toneRatio;
  ALIGN_16_BYTE float pbPCMquantThreshold[MAX_PB];

  float maxAllowedIncreaseFactor;   /* preecho control */
  float minRemainingThresholdFactor;

  int   lowpassLine;

  float clipEnergy;                 /* for level dependend tmn */

  float *fftWindow;

  /* for dummy psych */
  float ratio;

  int   fullPsych;

  ALIGN_16_BYTE float pbMaskLowFactor[MAX_PB];
  ALIGN_16_BYTE float pbMaskHighFactor[MAX_PB];

}PSY_CONFIGURATION;


void InitMinPCMResolution(int numPb, int sampleRate,
                  PSY_CONFIGURATION * psyConf);

int InitPsyConfiguration(long bitrate,
                         long samplerate,
                         float bandwidth,
                         int  blocktype,
                         PSY_CONFIGURATION *psyConf);
                          
int dp_InitPsyConfiguration(long bitrate,
                         long samplerate,
                         float bandwidth,
                         int  blocktype,
                         PSY_CONFIGURATION *psyConf);

#endif /* _PSY_CONFIGURATION_H */
