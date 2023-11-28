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
*   $Id: time_buffer.h,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: circular input buffer                                                *
*                                                                                              *
************************************************************************************************/

#ifndef _TIME_BUFFER_H
#define _TIME_BUFFER_H

#include "psy_const.h"
#include "mp3alloc.h"

typedef struct{

ALIGN_16_BYTE  float * timeSignal;

  int   readOffset;
  int   writeOffset;


  /* private */
  int SIM_SPACE;
  int INPUT_BUFFER_SIZE;

}INPUT_BUFFER;

int   InputBufferNew ( INPUT_BUFFER *inputBuffer, int blockSwitchOffset);
int   InputBufferDelete ( INPUT_BUFFER *inputBuffer );
int   InitInputBuffer(INPUT_BUFFER *inputBuffer);
int   FeedInputBuffer(INPUT_BUFFER *inputBuffer,float *timeSigData,int noOfSamples);
float *AccessInputBuffer(INPUT_BUFFER *inputBuffer,int offset,int ch);
int   InvalidateInputBuffer(INPUT_BUFFER *inputBuffer,int size);

#endif /* _TIME_BUFFER_H */
