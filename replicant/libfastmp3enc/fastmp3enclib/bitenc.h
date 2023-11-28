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
*   $Id: bitenc.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: Bitstream encoder                                                    *
*                                                                                              *
************************************************************************************************/

/* Bitstream encoder  */

#ifndef _BITENC_H
#define _BITENC_H

#include "mp3alloc.h"

struct BITSTREAMENCODER_INIT
{
  int channelMode;
  int bitrate;
  int sampleRate;
  int protection;
  int privateBit;
  int copyRightBit;
  int originalCopyBit;
  int ancMode; /* if FhG mode allocate ancillary buffer */
};

ALIGN_16_BYTE struct BITSTREAM_ENCODER ;
ALIGN_16_BYTE struct QC_OUT ;

int  BSNew(struct BITSTREAM_ENCODER **phBS) ;
int  BSInit(struct BITSTREAM_ENCODER *hBS, const struct BITSTREAMENCODER_INIT *init);
void BSDelete(struct BITSTREAM_ENCODER *hBS) ;

int BSWrite(struct BITSTREAM_ENCODER *hBS,
            struct QC_OUT *qcOut [2],
            int bitrateIndex,
            int padding,
            int modeEx,
            unsigned char *bitStreamOutData,
            int *outBytes,
            int ancMode,           
            unsigned char * ancDataBytes,
            int * numAncDataBytes,
            int bitPerChannelUsed[2],
            int* oflOffset,
            unsigned int* writeOflOnce, 
            int ancBitsToWrite
            ) ;

void BSFlush (struct BITSTREAM_ENCODER *hBS,
              unsigned char *bitStreamOutData,
              int *outBytes) ;

int GetHdSiBits(const struct BITSTREAM_ENCODER *hBS) ;
int *mp3GetFrameStats(struct BITSTREAM_ENCODER *hBS) ;
int addOflDummy(struct BITSTREAM_ENCODER *hBS, unsigned int* writeOflOnce); 

#endif /* _BITENC_H */
