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
*   $Id: time_buffer.c,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: circular input buffer                                                *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "psy_const.h"
#include "time_buffer.h"
#include "mathmac.h"
#include "mathlib.h"
#include "utillib.h"

int InputBufferNew ( INPUT_BUFFER *inputBuffer, int blockSwitchOffset)
{
  inputBuffer->timeSignal = 0;
  inputBuffer->SIM_SPACE = (blockSwitchOffset + BLOCK_SWITCHING_DATA_SIZE); /* 1408 */
  inputBuffer->INPUT_BUFFER_SIZE = (inputBuffer->SIM_SPACE + 2*FRAME_LEN_LONG) ; /* 2560 */

  inputBuffer->timeSignal = (float *)mp3Alloc(((inputBuffer->INPUT_BUFFER_SIZE+inputBuffer->SIM_SPACE)*2)*sizeof(float));
  if ( !inputBuffer->timeSignal )
    return -1;

  inputBuffer->writeOffset=blockSwitchOffset;
  return 0;
}

int InputBufferDelete ( INPUT_BUFFER *inputBuffer ){
  if ( inputBuffer )
    if (inputBuffer->timeSignal)
      mp3Free(inputBuffer->timeSignal);
  return 0;
}

int InitInputBuffer(INPUT_BUFFER *inputBuffer)
{
  setFLOAT(0.0f, inputBuffer->timeSignal, (inputBuffer->INPUT_BUFFER_SIZE+inputBuffer->SIM_SPACE)*2);

  inputBuffer->readOffset=0;

  return(0);

}


int FeedInputBuffer(INPUT_BUFFER *inputBuffer,float *timeSigData,int noOfSamples)
{
  int firstPart;

  sendDebout("inputBuffer", (inputBuffer->INPUT_BUFFER_SIZE+inputBuffer->SIM_SPACE)*2,1,"in",
                   MTV_FLOAT,inputBuffer->timeSignal);


  /*
    the input buffer is circular with size INPUT_BUFFER_SIZE.
    Buffer elements are *pairs* of floats. (i.e. stereo always)

    To make it possible to access the buffer at any point as one
    chunk without utilizing mod arithmetic, the first SIM_SPACE
    sample pairs are mirrored past the end.

    This is the situation: (A+B are the buffer itself, C is additional
    space to make reading contigous)

    Buffer: (sizeof(A) == sizeof(C) == SIM_SPACE,
             sizeof(A)+sizeof(B) == INPUT_BUFFER_SIZE)
    AAAAA BBBBBBBBBBBBB CCCCCC

    when writing, we have two special cases to care for:
    (X=samples written)

       XXXXXXXXXXX

    -> need to mirror into C
       XXXXXXXXXXX      AAAAXX

    and

    XX            XXXXX

    -> need to mirror into C

    XX            XXXXX XXAAAA
  */

  /* how many sample pairs to can we write before reaching wraparound? */
  firstPart = min(noOfSamples, inputBuffer->INPUT_BUFFER_SIZE-inputBuffer->writeOffset);

  copyFLOAT(timeSigData,
            & (inputBuffer->timeSignal [ 2*inputBuffer->writeOffset ]),
            2*firstPart);

  if (firstPart < noOfSamples)
  {
    /*
      wraparound occured.
      need to copy some more floats beginning from start-of-buffer
    */
    copyFLOAT(& (timeSigData [ 2*firstPart ]),
              inputBuffer->timeSignal,
              2*(noOfSamples-firstPart));
  }

  /* now mirror the first SIM_SPACE sample pairs to the end */

  /*
    did wraparound occur? if so, mirror lower part of A to C
  */
  if (firstPart < noOfSamples)
  {
    copyFLOAT(inputBuffer->timeSignal,
              & (inputBuffer->timeSignal [2*inputBuffer->INPUT_BUFFER_SIZE]),
              2*(noOfSamples-firstPart));
  }

  /* did we start writing in A? if so, mirror upper part of A to C */
  if (inputBuffer->writeOffset < inputBuffer->SIM_SPACE)
  {
    copyFLOAT(& (inputBuffer->timeSignal [2*inputBuffer->writeOffset]),
              & (inputBuffer->timeSignal [2*(inputBuffer->INPUT_BUFFER_SIZE + inputBuffer->writeOffset)]),
              2*(inputBuffer->SIM_SPACE-inputBuffer->writeOffset));
  }

  /* advance write pointer */
  inputBuffer->writeOffset += noOfSamples;
  inputBuffer->writeOffset %= inputBuffer->INPUT_BUFFER_SIZE;

  sendDebout("inputBuffer", (inputBuffer->INPUT_BUFFER_SIZE+inputBuffer->SIM_SPACE)*2,1,"out",
                   MTV_FLOAT,inputBuffer->timeSignal);

  return(0);
}

float *AccessInputBuffer(INPUT_BUFFER *inputBuffer,int offset,int ch)
{
 
  return(inputBuffer->timeSignal+((inputBuffer->readOffset+offset)%inputBuffer->INPUT_BUFFER_SIZE)*2+ch);
}


int InvalidateInputBuffer(INPUT_BUFFER *inputBuffer,int size)
{
  inputBuffer->readOffset=(inputBuffer->readOffset+size) % inputBuffer->INPUT_BUFFER_SIZE;
  return(0);
}
