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
*   $Id: ms_stereo.c,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"

#include <math.h>

#include "mathmac.h"
#include "psy_const.h"
#include "ms_stereo.h"


static const float SCALE_MIN_THRESH = 1.5f; /* 1.5 for 3db boost */
static const float MATRIX_WEIGHT = (float)M_SQRT1_2; /* 1/sqrt(2) */

static const float MIN_MS_GAIN = 1.04f; /* old value: 1.5f */
static const float MS_HYST = 0.1f;

static const int PN_RENORMALIZATION_EXPONENT = 58;
static const double PN_RENORMALIZATION_LIMIT = 2.88230376152E17; /* 2^58 */

static int DecideMS(int oldMSFlag, double pnLR, double pnMS, 
                    int ldConstLR, int ldConstMS)
{
  float acc;
  int currentMSFlag;

  if (oldMSFlag)
    acc = MIN_MS_GAIN + MS_HYST;
  else
    acc = MIN_MS_GAIN - MS_HYST;
  
  if (acc*(log(pnLR)*M_LOG2E+ldConstLR) < (log(pnMS)*M_LOG2E+ldConstMS))
    currentMSFlag = 1;
  else
    currentMSFlag = 0;
  return (currentMSFlag);
}

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
                   int         *oldMsFlag,    /* updated with current msFlag */
                   const int    granule,
                   const int    isLimit[TRANS_FAC],
                   const int   *sfbOffsets)
{
  int sfb,j; /* loop counters */
  int win;

  int useMS;
  double pnlr = 1.0;
  double pnms = 1.0;
  int pnlrExponent = 0;
  int pnmsExponent = 0;

  float minThreshold[MAX_SFB] ;

  for (win = 0; win < nWindows; win++)
  {
    int offsetSfb  = win * MAX_SFB_SHORT;

    for(sfb = offsetSfb; sfb < offsetSfb + isLimit[win]; sfb++)
    {    
      minThreshold[sfb] = min(sfbThresholdLeft[sfb], sfbThresholdRight[sfb]) * SCALE_MIN_THRESH;

      pnlr *= max(sfbEnergyLeft [sfb]/sfbThresholdLeft [sfb], 1.0);
      pnlr *= max(sfbEnergyRight[sfb]/sfbThresholdRight[sfb], 1.0);

      if (pnlr > PN_RENORMALIZATION_LIMIT)
      {
        pnlr *= 1.0f/PN_RENORMALIZATION_LIMIT;
        pnlrExponent += PN_RENORMALIZATION_EXPONENT;

      }

      pnms *= max(sfbEnergyMid [sfb]/minThreshold[sfb], 1.0);
      pnms *= max(sfbEnergySide[sfb]/minThreshold[sfb], 1.0);

      if (pnms > PN_RENORMALIZATION_LIMIT)
      {
        pnms *= 1.0f/PN_RENORMALIZATION_LIMIT;
        pnmsExponent += PN_RENORMALIZATION_EXPONENT;
      }

      /*
        to avoid division use this, but overflow problems are possible
        pnms = minThres*minThres*sfbEnergyLeft[sfb]*sfbEnergyRight[sfb];
        pnlr = sfbThresholdLeft[sfb]*sfbThresholdRight[sfb]*sfbEnergyMid[sfb]*sfbEnergySide[sfb];
      */
    }
  }
  useMS = *oldMsFlag;
  *oldMsFlag = DecideMS(*oldMsFlag,
                        1.0/pnlr, 1.0/pnms,
                        -pnlrExponent, -pnmsExponent);

  if (granule == 0) 
  {
    useMS = *oldMsFlag;
  }

  if(useMS)
  {
    for (win = 0; win < nWindows; win++)
    {
      int offsetLine = win * FRAME_LEN_SHORT ;
      int offsetSfb  = win * MAX_SFB_SHORT;

      /* calculate MS matrix */
      for(j=offsetLine; j < offsetLine+sfbOffsets[isLimit[win]]; j++)
      {
        float mdctValueLeft = mdctSpectrumLeft[j];
        mdctSpectrumLeft[j] = (mdctSpectrumLeft[j] + mdctSpectrumRight[j]) * MATRIX_WEIGHT;
        mdctSpectrumRight[j] = (mdctValueLeft - mdctSpectrumRight[j]) * MATRIX_WEIGHT;
      }

      /* the new threshold is the minimum of left and right threshold */
      for(sfb=offsetSfb; sfb < offsetSfb + isLimit[win]; sfb++) 
      {
        sfbThresholdLeft[sfb] = sfbThresholdRight[sfb] = minThreshold[sfb];
        sfbEnergyLeft[sfb]    = sfbEnergyMid[sfb];
        sfbEnergyRight[sfb]   = sfbEnergySide[sfb];
      }
    }
  }
  return(useMS);
}

void
MSGainPreprocessing(float *mdctLeft,
                    float *mdctRight,
                    float dampingFactor,
                    int nLines)
{
  int i;
  float f1 = 0.5f*(1.0f + dampingFactor);
  float f2 = 0.5f*(1.0f - dampingFactor);

  for (i = 0; i < nLines; i++)
  {
    float t      = f1*mdctLeft[i] + f2*mdctRight[i];
    mdctRight[i] = f2*mdctLeft[i] + f1*mdctRight[i];
    mdctLeft[i]  = t;
  }
}

void
AdaptMSGainPreprocessing(const int    isLimit,
                         const PSY_CONFIGURATION *psyConf,
                         const float *sfbEnergyMid,
                         const float *sfbEnergySide,
                         float       *dampingFactor)
{
  int i;
  float nrgM = 0.0f, nrgS = 0.0f;

  for (i=0; i < isLimit; i++)
  {
    nrgM += sfbEnergyMid [i];
    nrgS += sfbEnergySide[i];
  }

  if (nrgS > 0.0f)
  {
    float deltaNrg = nrgM / nrgS ; /* > 1 : MS gain */

    deltaNrg /= psyConf->msRatioWanted;

    *dampingFactor *= (float)pow(deltaNrg, psyConf->msPreprocessingK);
    if (*dampingFactor > 1.0f) *dampingFactor = 1.0f;
  }
}
