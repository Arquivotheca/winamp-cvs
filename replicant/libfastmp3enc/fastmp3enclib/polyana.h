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
*   $Id: polyana.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _POLYANA_H
#define _POLYANA_H

#include "mconfig.h"
#include "mp3alloc.h"
enum
{
  POLY_WINDOW_SIZE = 512,
  POLY_PHASE_BANDS =  32
};

struct POLY_PHASE {
  float *polyBuffer;
  float *polyBuffer2;
  float* anaWindow;
  int    curNdx;
};

typedef struct POLY_PHASE *POLY_PHASE_HANDLE ;

int PolyPhaseNew(POLY_PHASE_HANDLE *phPolyPhase);
int PolyPhaseInit(POLY_PHASE_HANDLE hPolyPhase);
void PolyPhaseDelete(POLY_PHASE_HANDLE hPolyPhase);

extern int (*PolyAnalyse) (POLY_PHASE_HANDLE hPolyPhase,
			   const float *restrict timeSigData,
			   float *restrict polyPhaseData);

#endif
