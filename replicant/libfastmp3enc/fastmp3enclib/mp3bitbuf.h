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
*   $Id: mp3bitbuf.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:  modulo bit buffer functions                                         *
*                                                                                              *
************************************************************************************************/

#ifndef __BIT_BUF_H
#define __BIT_BUF_H

typedef unsigned long BIT_BUF_WORD;
typedef struct bitbuffer *HANDLE_BIT_BUF;

/* function prototypes */
HANDLE_BIT_BUF mp3CreateBitBuffer(int bitSize);

void           mp3DeleteBitBuffer(HANDLE_BIT_BUF hBitBuf);

int            mp3GetBitsAvail(HANDLE_BIT_BUF hBitBuf);

void           mp3WriteBits(HANDLE_BIT_BUF hBitBuf,BIT_BUF_WORD bValue,unsigned int nBits);

BIT_BUF_WORD   mp3ReadBits(HANDLE_BIT_BUF hBitBuf,unsigned int nBits);

void           mp3ReadBytes(HANDLE_BIT_BUF hBitBuf, unsigned char *dst, int nBytes);

int            mp3ByteAlign(HANDLE_BIT_BUF hBitBuf);

unsigned int   mp3GetBitcnt(struct bitbuffer *ptr);

#endif /* ifndef __BIT_BUF_H */
