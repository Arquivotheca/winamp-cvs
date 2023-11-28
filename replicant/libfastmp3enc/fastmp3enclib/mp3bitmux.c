/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1997-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3bitmux.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:  multiplexes header+si bitstream and dynpart bitstream               *
*                                                                                              *
************************************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mconfig.h"
#include "mp3bitmux.h"
#include "mp3crc.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

/*****************************************************************************

    functionname:InitBitMux 
    description:   
    returns:
    input:         
    output:        
    globals:       

*****************************************************************************/
void mp3InitBitMux(struct MUX_LIST *muxList,
                   HANDLE_BIT_BUF hBitstreamHdSi,
                   HANDLE_BIT_BUF hBitstreamDy)
{
  muxList->hBitstreamHdSi = hBitstreamHdSi ;
  muxList->hBitstreamDy   = hBitstreamDy ;
  muxList->ReadPointer  = 0 ;
  muxList->WritePointer = 0 ;
  muxList->muxFrame[0].lengthHdSi = 0 ;
  muxList->muxFrame[0].lengthDy   = 0 ;

  muxList->frameStats [0] = 0 ;
}

/*****************************************************************************

    functionname:ComposeMuxFrames 
    description: put lengthHdSi bits into hdsi queue, lengthDy into dyn queue
                 if one or more complete frames can be output, do so. 
    returns:     number of bytes output
    input:       
    output:        

*****************************************************************************/
int mp3ComposeMuxFrames(struct MUX_LIST *muxList,
                        int crcActive,
                        int lengthHdSi,
                        int lengthDy,
                        unsigned char *output)
{
  int outBytes = 0;
  int headerBytes = crcActive ? 6 : 4;

  muxList->muxFrame[muxList->WritePointer].lengthHdSi   = lengthHdSi;
  muxList->muxFrame[muxList->WritePointer].lengthDy     = lengthDy;
  muxList->WritePointer = (muxList->WritePointer+1) % MAX_MUX_FRAME;

  while(muxList->ReadPointer != muxList->WritePointer)
  {
    struct MUX_FRAME *thisMuxFrame = muxList->muxFrame + muxList->ReadPointer ;
    int bitsAvailDy   = mp3GetBitsAvail(muxList->hBitstreamDy) ;

    if(bitsAvailDy >= thisMuxFrame->lengthDy)
    {
      unsigned long lastoutBytes = outBytes ;
      unsigned long crcReg;

      assert(thisMuxFrame->lengthHdSi % 8 == 0);
      assert(thisMuxFrame->lengthDy % 8 == 0);

      /* read the header */
      mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, 4);
      outBytes += 4;

      if (crcActive)
      {
        int i;
        crcReg = 0xffff;
        for (i=0; i<2; i++)
          mp3CrcAdvance(0x8005,0x8000,&crcReg, output[outBytes-2+i]);

        mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, 2);
        outBytes += 2; /* leave 2 bytes for CRC */
      }

      /* read the rest */
      mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, thisMuxFrame->lengthHdSi/8 - headerBytes);

      if (crcActive)
      {
        int i;
        for (i=0; i<thisMuxFrame->lengthHdSi/8 - headerBytes; i++)
        {
          mp3CrcAdvance(0x8005,0x8000,&crcReg, output[outBytes+i]);
        }

        /* encode crc word into bitstream */
        crcReg &= 0xffff;
        output[outBytes-2] = (unsigned char)(crcReg >> 8);
        output[outBytes-1] = (unsigned char)(crcReg & 0xff);
      }
      outBytes += thisMuxFrame->lengthHdSi/8 - headerBytes;

      /* read the dynpart */
      mp3ReadBytes(muxList->hBitstreamDy, output+outBytes, thisMuxFrame->lengthDy/8);
      outBytes += thisMuxFrame->lengthDy/8;

      muxList->ReadPointer = (muxList->ReadPointer+1) % MAX_MUX_FRAME;

      muxList->frameStats [++muxList->frameStats [0]] = outBytes - lastoutBytes ;
    }
    else
    {
      break;
    }
  }

  return(outBytes);
}

/*****************************************************************************

    functionname:FlushMuxFrames 
    description: flush the hdsi queue, filling 0 bits into dyn queue as
                 needed.
    returns:     number of bytes output
    input:       
    output:        

*****************************************************************************/
int mp3FlushMuxFrames(struct MUX_LIST *muxList,
                      int crcActive,
                      unsigned char *output)
{
  int outBytes = 0;
  int headerBytes = crcActive ? 6 : 4;

  while(muxList->ReadPointer != muxList->WritePointer)
  {
    struct MUX_FRAME *thisMuxFrame = muxList->muxFrame + muxList->ReadPointer ;
    int dynBytesAvail;
    int dynBytesNeeded;

    unsigned long lastoutBytes = outBytes ;
    unsigned long crcReg;

    assert(mp3GetBitsAvail(muxList->hBitstreamHdSi) >= thisMuxFrame->lengthHdSi);
    assert(mp3GetBitsAvail(muxList->hBitstreamHdSi) % 8 == 0);

    /* read the header */
    mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, 4);
    outBytes += 4;

    if (crcActive)
    {
      int i;
      crcReg = 0xffff;
      for (i=0; i<2; i++)
        mp3CrcAdvance(0x8005,0x8000,&crcReg, output[outBytes-2+i]);

      mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, 2);
      outBytes += 2; /* leave 2 bytes for CRC */
    }

    /* read the rest */
    mp3ReadBytes(muxList->hBitstreamHdSi, output+outBytes, thisMuxFrame->lengthHdSi/8 - headerBytes);

    if (crcActive)
    {
      int i;
      for (i=0; i<thisMuxFrame->lengthHdSi/8 - headerBytes; i++)
      {
        mp3CrcAdvance(0x8005,0x8000,&crcReg, output[outBytes+i]);
      }

      /* encode crc word into bitstream */
      crcReg &= 0xffff;
      output[outBytes-2] = (unsigned char)(crcReg >> 8);
      output[outBytes-1] = (unsigned char)(crcReg & 0xff);
    }
    outBytes += thisMuxFrame->lengthHdSi/8 - headerBytes;

    assert(mp3GetBitsAvail(muxList->hBitstreamDy) % 8 == 0);
    dynBytesAvail  = mp3GetBitsAvail(muxList->hBitstreamDy)/8;
    dynBytesNeeded = thisMuxFrame->lengthDy/8 - dynBytesAvail;

    assert(dynBytesNeeded > 0);

    mp3ReadBytes(muxList->hBitstreamDy, output+outBytes, dynBytesAvail);
    outBytes += dynBytesAvail ;
    memset(output + outBytes, 0, dynBytesNeeded);
    outBytes += dynBytesNeeded ;

    muxList->ReadPointer = (muxList->ReadPointer+1) % MAX_MUX_FRAME;

    muxList->frameStats [++muxList->frameStats [0]] = outBytes - lastoutBytes ;
  }

  return(outBytes);
}
