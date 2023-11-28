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
*   $Id: pre_echo_control.c,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "pre_echo_control.h"
#include "mathmac.h"

int mp3InitPreEchoControl(/*float *pbThresholdnm1 */)
{
  return 0;
}

void mp3PreEchoControl(float *pbThresholdNm1,
                       int   numPb,
                       float maxAllowedIncreaseFactor,
                       float minRemainingThresholdFactor,
                       float *pbThreshold)
{
  int i;
  float tmpThreshold1, tmpThreshold2;
  
  for(i = 0; i < numPb; i++) {
    tmpThreshold1 = maxAllowedIncreaseFactor * pbThresholdNm1[i];
    tmpThreshold2 = minRemainingThresholdFactor * pbThreshold[i];
    
    /* copy thresholds to internal memory */
    pbThresholdNm1[i] = pbThreshold[i];

    if(pbThreshold[i] > tmpThreshold1) {
      pbThreshold[i] = tmpThreshold1;
    }
    
    if(tmpThreshold2 > pbThreshold[i]) {
      pbThreshold[i] = tmpThreshold2;
    }
  }
}
