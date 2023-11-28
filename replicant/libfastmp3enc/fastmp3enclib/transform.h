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
*   $Id: transform.h,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                  *
*                                                                                              *
************************************************************************************************/

#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "psy_const.h"
#include "mp3alloc.h"

#define MDCT_BUF_SIZE (2*FRAME_LEN_LONG)
/*
  define this to use complex mdcts
*/
/*
#define USE_COMPLEX_MDCT
#define OLD_TRANSFORM
*/

typedef struct{
	float paMdctBuffer[MDCT_BUF_SIZE];
}TRANSFORM_BUFFER;


int InitTransform(TRANSFORM_BUFFER *transformBuffer);
int ShiftTransformBuffer(TRANSFORM_BUFFER *transformBuffer);
int Transform_LONG(TRANSFORM_BUFFER *transformBuffer,
                   float *cmplxOut,
                   int windowSequence);
int Transform_SHORT(TRANSFORM_BUFFER *transformBuffer,
                   float *cmplxOut,
                   int subWindow);
int MdctTransform_LONG(TRANSFORM_BUFFER *transformBuffer,
                   float *cmplxOut,
                   int windowSequence);
int MdctTransform_SHORT(TRANSFORM_BUFFER *transformBuffer,
                   float *cmplxOut,
                   int subWindow);
int AliasReduction(float *mdctRealData);




#endif
