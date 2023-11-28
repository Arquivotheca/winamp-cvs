/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1995-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3bitmux.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:       multiplexes header+si bitstream and dynpart bitstream          *
*                                                                                              *
************************************************************************************************/

#ifndef __BIT_MUX_H
#define __BIT_MUX_H

#include "mp3bitbuf.h"
#include "mp3alloc.h"

/*
 * how many outstanding frames can we have at the most? The extreme is
 * when we are writing a 8kBit/s, 24 kHz stereo bitstream with CRC (the
 * ratio of sideinfo to dynpart is the greatest). In this case, we have
 * 135 bits sideinfo, 32 bits for the header, 16 bits for the CRC, 192
 * bits per frame, i.e. 9 dynpart bits per frame. The backpointer can
 * point back 255*8 = 2040 bits; this corresponds to 2040/9 = 227 frames.
 *
 * But then again, who would want to do that?
 *
 * For all we care, 64 MUX_FRAMES should suffice. If they don't, we bail
 * out.
 */

#define MAX_MUX_FRAME 64

struct MUX_FRAME {
  int lengthHdSi;
  int lengthDy;
} ;

struct MUX_LIST {
  ALIGN_16_BYTE struct MUX_FRAME muxFrame[MAX_MUX_FRAME];
  HANDLE_BIT_BUF hBitstreamHdSi;
  HANDLE_BIT_BUF hBitstreamDy;
  int ReadPointer;
  int WritePointer;

  ALIGN_16_BYTE int frameStats[MAX_MUX_FRAME+1];
} ;


void mp3InitBitMux(struct MUX_LIST *muxList,
                   HANDLE_BIT_BUF hBitstreamHdSi,
                   HANDLE_BIT_BUF hBitstreamDy);

int mp3ComposeMuxFrames(struct MUX_LIST *muxList,
                        int crcActive,
                        int lengthHdSi,
                        int lengthDy,
                        unsigned char *output);

int mp3FlushMuxFrames(struct MUX_LIST *muxList,
                      int crcActive,
                      unsigned char *output) ;

#endif
