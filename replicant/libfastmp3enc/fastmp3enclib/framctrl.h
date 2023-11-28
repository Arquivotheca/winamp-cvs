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
*   $Id: framctrl.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: frame length / bitrate control                                       *
*                                                                                              *
************************************************************************************************/

#ifndef _FRAMECTRL_H
#define _FRAMECTRL_H

enum {
  FC_PADDING_ISO,
  FC_PADDING_NEVER,
  FC_PADDING_ALWAYS,
  FC_PADDING_NEAREST
} ;

struct FRAME_CONTROL
{
  struct RATE_CONTROL *rateControl;
  int frameLength[15];
  int nFramesInMeta;
  int cbMetaframe;
  int mpegVersion;
  int maxBitrateIndex;
};

int FCNew(struct FRAME_CONTROL **);
int FCInit(struct FRAME_CONTROL *, long bitrate, long sampleRate, int paddingMode);
void FCDelete(struct FRAME_CONTROL *);
int FCBitrateIdx(const struct FRAME_CONTROL *);
int FCSize(const struct FRAME_CONTROL *, const int bitrateIdx);
int FCNuansdroByte(const struct FRAME_CONTROL *); /*was FCPaddingByte*/
int FCAdvance(struct FRAME_CONTROL *);
int FCFindFit(const struct FRAME_CONTROL *, const int bits, int* ancBitsToWrite, int useDualPass);
int FCMetaframeSize(const struct FRAME_CONTROL *self);
int FCFramesInMeta(const struct FRAME_CONTROL *self);
#endif
