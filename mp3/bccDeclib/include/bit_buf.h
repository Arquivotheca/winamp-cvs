/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1997 - 2008 Fraunhofer IIS
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#ifndef __BIT_BUF_H
#define __BIT_BUF_H


#include <stdio.h>
#include <assert.h>
#include "bastypes.h"

typedef DWORD BIT_BUF_WORD;

typedef struct{
  BIT_BUF_WORD *Data;
  INT          ReadPointer;
  INT          WritePointer;
  INT          Size;

  BIT_BUF_WORD cache;
  INT          mask;

  INT reading;

  /* crc stuff */
  BOOL         doCRCCheck;
  INT          bitsToCheck;
  WORD         crcState;
  WORD         crcMask;
  WORD         crcPoly;
}BIT_BUF;

typedef BIT_BUF *HANDLE_BIT_BUF;

/* function prototypes */
HANDLE_BIT_BUF  CreateBitBuffer(INT bitSize);

void            DeleteBitBuffer(HANDLE_BIT_BUF hBitBuf);

INT             SetReadPointer(HANDLE_BIT_BUF hBitBuf,INT delta);

INT             SetWritePointer(HANDLE_BIT_BUF hBitBuf,INT delta);

INT             GetBitsAvail(HANDLE_BIT_BUF hBitBuf);

BOOL            WriteBits(HANDLE_BIT_BUF hBitBuf,BIT_BUF_WORD bValue,INT nBits);

BIT_BUF_WORD    ReadBits(HANDLE_BIT_BUF hBitBuf,INT nBits);

BOOL            ngsCopyBits(HANDLE_BIT_BUF hBitBufWrite,HANDLE_BIT_BUF hBitBufRead,INT nBits);

void           SetCRCPolynom(HANDLE_BIT_BUF hBitBuf, WORD crcPoly, WORD crcMask);
void           SetCRC(HANDLE_BIT_BUF hBitBuf, WORD crcReg);
WORD           GetCRC(HANDLE_BIT_BUF hBitBuf);
void           crcCheckNextNBits(HANDLE_BIT_BUF hBitBuf, INT nBits);
void           crcFinalize(HANDLE_BIT_BUF hBitBuf);
void           DoCRCCheck(HANDLE_BIT_BUF hBitBuf, BOOL check);

#endif /* ifndef __BIT_BUF_H */

 
