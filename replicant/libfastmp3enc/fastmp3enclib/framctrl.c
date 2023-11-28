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
*   $Id: framctrl.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: bitrate control                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mathmac.h"
#include "psy_const.h"
#include "framctrl.h"
#include "ratectrl.h"

enum
{
  ERROR_WRONGPARAMETER = 1
} ;


ALIGN_16_BYTE static const int mpegBitrates[3][15] =
{
  {0,      32000, 40000, 48000, 56000, 64000, 80000, 96000,
   112000,128000,160000,192000,224000,256000,320000},
  {    0, 8000,16000, 24000, 32000, 40000, 48000, 56000,
   64000,80000,96000,112000,128000,144000,160000},
  {    0, 8000,16000, 24000, 32000, 40000, 48000, 56000,
   64000,0,0,0,0,0,0}
};

int FCNew(struct FRAME_CONTROL **framectrl)
{
  struct FRAME_CONTROL *fc = (struct FRAME_CONTROL *) mp3Alloc(sizeof(struct FRAME_CONTROL));
  int error = RCNew(&(fc->rateControl));
  if (error)
  {
    FCDelete(fc);
    fc = 0;
  }
  *framectrl = fc;
  return (fc == 0);
}

int FCInit(struct FRAME_CONTROL *self,
           long bitrate, long sampleRate,
           int paddingMode)
{
  int i;
  int error;
  int nGranules;
  int frameLen;
  long bitsPerFrameNominator, bitsPerFrameDenominator;
  
#ifdef NO_PADDING
  paddingMode = FC_PADDING_NEVER;
#endif

  self->mpegVersion = (sampleRate < 16000 ? MPEG2_5 :
                     (sampleRate < 32000 ? MPEG2 : MPEG1));
  nGranules = (self->mpegVersion == MPEG1 ? 2 : 1);


  if (self->mpegVersion == MPEG2_5)
    self->maxBitrateIndex = 9;
  else
    self->maxBitrateIndex = 15;

  bitsPerFrameNominator   = bitrate * nGranules * FRAME_LEN_LONG;
  bitsPerFrameDenominator = sampleRate;

  for ( i = 0 ; i < self->maxBitrateIndex; i++)
  {
    self->frameLength[i] = nGranules * FRAME_LEN_LONG *
      mpegBitrates[self->mpegVersion][i] / sampleRate;
    self->frameLength[i] -= self->frameLength[i] % 8;
  }

  frameLen = bitsPerFrameNominator/bitsPerFrameDenominator;
  if( frameLen - frameLen%8 >
      self->frameLength[self->maxBitrateIndex-1] )
    return 1; /* error: bitrate is to high for this samplingrate */

  self->nFramesInMeta = 1; /* default will be patched later on */
  self->cbMetaframe   = 1;

  if (paddingMode != FC_PADDING_ISO)
  {
    int pb;
    int idxLow=0, idxHigh=0;
    long brLow, brHigh;

    /* decide upon padding byte on/off */
    switch (paddingMode)
    {
    case FC_PADDING_ALWAYS:
      pb = 1; break;
    case FC_PADDING_NEAREST:
      pb = (bitsPerFrameNominator % bitsPerFrameDenominator
            > bitsPerFrameDenominator / 2) ? 1 : 0; break;
    default:
      pb = 0; break;
    }

    /* patch the padding mode */
    paddingMode = pb ? FC_PADDING_ALWAYS : FC_PADDING_NEVER ;

    /* find the two bitrate indices between which we do switching */
    for ( i = 0; i < self->maxBitrateIndex; i++ )
    {
      if (mpegBitrates[self->mpegVersion][i] == bitrate)
      {
        /* perfect match, no bitrate switching */
        idxHigh = idxLow = i;
        break;
      }
      else if (mpegBitrates[self->mpegVersion][i] <= bitrate &&
               (i+1 == self->maxBitrateIndex || mpegBitrates[self->mpegVersion][i+1] > bitrate))
      {
        idxHigh = 1 + (idxLow = i);
        break;
      }
    }

    assert(i < self->maxBitrateIndex);

    brLow  = mpegBitrates[self->mpegVersion][idxLow ] ;
    brHigh = mpegBitrates[self->mpegVersion][idxHigh] ;

    if (brLow == brHigh)
    {
      bitsPerFrameNominator   = self->frameLength[idxLow] + pb*8;
      bitsPerFrameDenominator = 1;
      self->cbMetaframe   = self->frameLength[idxLow]/8 + pb;
      self->nFramesInMeta = 1;
    }
    else
    {
#ifdef _NO_FRAMESIZE_FIX
      /* 
	 This calculation seems to be needed for a correct meta frame size calculation,
	 but causes the Rate Control to be incorrect.
	 Leave this code inside as reference until the the meta frame size calculation is fixed!
       */
      long nom = brHigh - brLow;
      long den = brHigh - bitrate;

      bitsPerFrameNominator =
        ((self->frameLength[idxHigh]/8 + pb) * (bitrate - brLow)/1000 +
         (self->frameLength[idxLow] /8 + pb) * (brHigh - bitrate)/1000) * 8;

      bitsPerFrameDenominator = (brHigh-brLow)/1000;

      RCCancelFraction(&nom,&den);

      self->nFramesInMeta = nom;
      self->cbMetaframe   = (bitsPerFrameNominator / bitsPerFrameDenominator
                             * self->nFramesInMeta) / 8;
#else
      /* hopefully corect meta frame size calculation */
      /* calculate correct bitrates */
      int bitrateLow, bitrateHigh;
      int deltaLow, deltaHigh;

      /* find the two frame length indices to switch between */
      for ( i = 0; i < self->maxBitrateIndex; i++ ) {
	if ( (frameLen >= self->frameLength[i]+pb*8)   && 
	     (frameLen  < (self->frameLength[i+1]+pb*8)) ) {
	  break;
	}
      }
      idxLow  = i;
      idxHigh = i+1;
      /* check if idx are inside bitrate/frame length table */
      if (idxHigh>=self->maxBitrateIndex) {
	/* idx out of range -> take the largest frame size possible */
	idxLow = idxHigh = self->maxBitrateIndex-1;
	deltaLow  = 0;
	deltaHigh = 1;
	/* now patch the rate control initialization accordingly  */
	bitsPerFrameNominator   = self->frameLength[idxLow] + pb*8;
	bitsPerFrameDenominator = 1;
      }
      else if (idxLow==0) {
	/* idx out of range -> take the smallest frame size possible */
	idxLow = idxHigh = 1;
	deltaLow  = 0;
	deltaHigh = 1;
	/* now patch the rate control initialization accordingly  */
	bitsPerFrameNominator   = self->frameLength[idxLow] + pb*8;
	bitsPerFrameDenominator = 1;
      }
      else {
	bitrateLow  = (self->frameLength[idxLow]  + 8*pb)  * sampleRate;
	bitrateHigh = (self->frameLength[idxHigh] + 8*pb)  * sampleRate;

	deltaLow  = (bitrate*(nGranules * FRAME_LEN_LONG)) - bitrateLow;
	deltaHigh = bitrateHigh - (bitrate*(nGranules * FRAME_LEN_LONG));
      }

      RCCancelFraction(&deltaLow,&deltaHigh);

      self->nFramesInMeta = deltaLow + deltaHigh;
      self->cbMetaframe   = deltaHigh * (self->frameLength[idxLow]/8 + pb) + deltaLow * (self->frameLength[idxHigh]/8 + pb);
#endif
    }
  }

  error = RCInit(self->rateControl,
                 bitsPerFrameNominator,bitsPerFrameDenominator,
                 self->maxBitrateIndex, self->frameLength, paddingMode);

  return error;
}

void FCDelete(struct FRAME_CONTROL *self)
{
  if (self)
  {
    RCDelete(self->rateControl);
    mp3Free(self);
  }
}

int FCBitrateIdx(const struct FRAME_CONTROL *self)
{
  return RCBitrateIdx(self->rateControl);
}

/* was FCPaddingByte */
int FCNuansdroByte(const struct FRAME_CONTROL *self)
{
  return RCNuansdroByte(self->rateControl);
}

int FCSize(const struct FRAME_CONTROL *self, const int bitrateIdx)
{
  return self->frameLength[bitrateIdx];
}

/*
  advance to next frame.
 */

int FCAdvance(struct FRAME_CONTROL *self)
{
  return RCAdvance(self->rateControl);
}

/*
  find frame size / bitrate index for a frame that holds "bits" bits.
 
 */
int FCFindFit(const struct FRAME_CONTROL *self, const int bits, int* ancBitsToWrite, 
              int useDualPass)
{
  int i;
  int maxBitrateIndex = ((useDualPass)?(self->maxBitrateIndex-1):(self->maxBitrateIndex));
  int additionalBits = 8;
  int matchingIdx = -1;

#ifndef NO_PADDING
  if(RCNuansdroMode(self->rateControl) == FC_PADDING_NEVER)
    additionalBits = 0;
#else
  additionalBits = 0;
#endif

  for ( i = 1; i < maxBitrateIndex; i++ )
  {
    if (self->frameLength[i] + additionalBits >= bits) {
      matchingIdx = i;
      break;
    }
  }

/* This assert is obsolete */
  assert(matchingIdx<self->maxBitrateIndex); 

  return matchingIdx;
}

int FCMetaframeSize(const struct FRAME_CONTROL *self)
{
  return self->cbMetaframe ;
}

int FCFramesInMeta(const struct FRAME_CONTROL *self)
{
  return self->nFramesInMeta ;
}
