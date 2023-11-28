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
*   $Id: spreading.h,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _SPREADING_H

#define _SPREADING_H


/* 
InitSpreadingFunction() becomes obsolete
as soon as the spreading function is moved to ROM.
Dynamic initialization is only needed for development purposes
*/

float *InitSpreadingFunction();
void FreeSpreadingFunction(float *spreadingFunction); 

void InitBarcValues(const int    numLines, 
                    const long   samplingFrequency, 
                    const int    numPb, 
                    const int   *pbOffset, 
                    int         *barcValues,
                    int         *barcValScaling,
                    float       *pbSpreadNorm,
                    float       *pScratchSpreadNorm,
                    const float *spreadingFunction,
                    float       *maskLoFactor,
                    float       *maskHiFactor);

void Spreading(const float  *pbEnergy,
               const float  *pbWeightedChaosMeasure,
               const int     pbCnt,
               /*const int    *pbOffset,*/
               const float  *spreadingFunction,
               const int    *barcValues,
               float        *pbSpreadedEnergy,
               float        *pbSpreadedChaosMeasure);

void SpreadingMax(const int    pbCnt,
                  const float *maskLowFactor,
                  const float *maskHighFactor,
                  float       *pbSpreadedEnergy);



#endif /* #ifndef _SPREADING_H */
