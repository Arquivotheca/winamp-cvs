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
*   $Id: mp3bitbuf.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include "mp3bitbuf.h"

#include <assert.h>

typedef unsigned short bbWord;
enum {
  bbWordSize = 8*sizeof(bbWord)
};

struct bitbuffer
{
  int size;        /* size of bit buffer in bitbuffer words */
  bbWord *buffer;  /* the bit buffer itself                 */

  bbWord rCache;
  bbWord wCache;

  signed int wBitsLeft;   /* bits left in write cache */
  signed int rBitsLeft;   /* bits left in read cache  */

  int wIdx;               /* points to memory word under write cache */
  int rIdx;               /* points to memory word under read cache  */

  unsigned int bitsAvail; /* bits available to a read operation      */
  unsigned int bitcnt;    /* bits written since last align() call    */

  int rCacheDirty; /* if != 0, the read cache needs to be brought into sync  */
};

ALIGN_16_BYTE static const unsigned long powof2[] =
{
  1,2,4,8,16,32,64,128,
  256,512,1024,2048,4096,8192,16384,32768,
  256*256,256*512,256*1024,256*2048,256*4096,256*8192,256*16384,256*32768,
  65536*256,65536*512,65536*1024,65536*2048,65536*4096,65536UL*8192,65536UL*16384,65536UL*32768,
  0
};

static void bbInit(struct bitbuffer *pBuffer, int size)
{
  assert ( 2*bbWordSize <= 8*sizeof(unsigned long) );

  pBuffer->size        = (size + bbWordSize - 1) / bbWordSize;
  pBuffer->buffer      = (unsigned short *) mp3Calloc(pBuffer->size,
                                bbWordSize / 8);

  pBuffer->rCache      = 0;
  pBuffer->wCache      = 0;

  pBuffer->rBitsLeft   = 0;
  pBuffer->wBitsLeft   = bbWordSize;

  pBuffer->rIdx        = -1;
  pBuffer->wIdx        = 0;

  pBuffer->bitsAvail   = 0;
  pBuffer->bitcnt      = 0;

  pBuffer->rCacheDirty = 0;
}

static void bbDestroy(struct bitbuffer *pBuffer)
{
  if (pBuffer && pBuffer->buffer) mp3Free(pBuffer->buffer);
}

static void
bbWriteBitsShort(struct bitbuffer *pBuffer,
                 bbWord w,
                 unsigned int len)
{
  unsigned long wCache;

  assert(len <= bbWordSize);

  pBuffer->bitsAvail += len;
  pBuffer->bitcnt    += len;

  wCache   = pBuffer->wCache;
  wCache <<= len;
  wCache  |= w;

  pBuffer->wBitsLeft -= len;

  /* if read and write cache overlapped, mark the read cache as unclean */
  pBuffer->rCacheDirty |= (pBuffer->wIdx == pBuffer->rIdx);

  if (pBuffer->wBitsLeft >= 0)
  {
    pBuffer->wCache = (bbWord)wCache;
  }
  else
  {
    unsigned long t;

    /* flush write cache word */
    t =  wCache >> (-pBuffer->wBitsLeft);
    wCache ^= t << (-pBuffer->wBitsLeft);
    pBuffer->buffer[pBuffer->wIdx] = (bbWord)t;

    /* Advance write pointer */
    if (++(pBuffer->wIdx) == pBuffer->size) pBuffer->wIdx = 0;

    pBuffer->wBitsLeft += bbWordSize;
    pBuffer->wCache    = (bbWord)wCache;
  }
}

static void bbSyncWriteCache(struct bitbuffer *pBuffer)
{
  /*
   * write the partial write cache word to memory.
   */

  bbWord dstmask;

  dstmask = (bbWord)~((powof2[bbWordSize - pBuffer->wBitsLeft]-1) << pBuffer->wBitsLeft);

  pBuffer->buffer[pBuffer->wIdx] &= dstmask;
  pBuffer->buffer[pBuffer->wIdx] |= (pBuffer->wCache << pBuffer->wBitsLeft);;
}

static bbWord bbReadWord(struct bitbuffer *pBuffer)
{
  pBuffer->bitsAvail -= bbWordSize;

  if (pBuffer->rBitsLeft == 0)
  {
    if (++(pBuffer->rIdx) == pBuffer->size) pBuffer->rIdx = 0;
  }
  else
  {
    assert(pBuffer->rBitsLeft == bbWordSize);
    pBuffer->rBitsLeft = 0;
  }
  return pBuffer->buffer[pBuffer->rIdx];
}

static bbWord bbReadBitsShort(struct bitbuffer *pBuffer, unsigned int len)
{
  unsigned int rc;

  assert(len <= bbWordSize);
  assert(len <= pBuffer->bitsAvail);

  pBuffer->bitsAvail -= len;

  if (pBuffer->rCacheDirty)
  {
    /* sync write cache to memory */
    bbSyncWriteCache(pBuffer);

    pBuffer->rCache = pBuffer->buffer[pBuffer->rIdx] << (bbWordSize-pBuffer->rBitsLeft);
    pBuffer->rCacheDirty = 0;
  }

  rc = (pBuffer->rCache >>  (bbWordSize - len));

  if (len <= (unsigned)pBuffer->rBitsLeft)
  {
    pBuffer->rCache   <<= len;
    pBuffer->rBitsLeft -= len;
  }
  else
  {
    /* we got the first word; now read the second */

    len -= pBuffer->rBitsLeft;

    /* Advance read pointer */
    if (++(pBuffer->rIdx) == pBuffer->size) pBuffer->rIdx = 0;

    if (pBuffer->rIdx == pBuffer->wIdx)
    {
      /* sync write cache to memory */
      bbSyncWriteCache(pBuffer);
    }

    pBuffer->rCache = pBuffer->buffer[pBuffer->rIdx];
    pBuffer->rCacheDirty = 0;

    rc |= (pBuffer->rCache >>  (bbWordSize - len));
    pBuffer->rCache        <<= len;

    pBuffer->rBitsLeft = bbWordSize - len;
  }

  return rc;
}

void mp3ReadBytes(struct bitbuffer *pBuffer, unsigned char *dst, int n)
{
  int i;

  if (n == 0) return; /* early bail out */

  if (pBuffer->rBitsLeft % 8)
  {
    /*
      read pointer is not byte-aligned. Do a slow read.
    */
    for (i = 0; i < n; i++)
    {
      dst[i] = (unsigned char)bbReadBitsShort(pBuffer, 8);
    }
  }
  else
  {
    /* make sure we are dealing with 16-bit bitbuf words */
    assert(bbWordSize == 16);

    for (i = 0; i < n && (pBuffer->rBitsLeft % bbWordSize); i++)
    {
      dst[i] = (unsigned char)bbReadBitsShort(pBuffer, 8);
    }

    /* sync the write cache */
    bbSyncWriteCache(pBuffer);

    for (; i <= n-(signed)bbWordSize / 8; i+= bbWordSize/8)
    {
      bbWord t = bbReadWord(pBuffer);

      dst[i]   = t >> 8;
      dst[i+1] = t & 0xff;
    }

    for (; i < n; i++)
    {
      dst[i] = (unsigned char)bbReadBitsShort(pBuffer, 8);
    }
  }
}

static int bbByteAlign(struct bitbuffer *pBuffer)
{
  int bits = (8-(pBuffer->bitcnt % 8)) % 8;
  bbWriteBitsShort(pBuffer, 0, bits);

  pBuffer->bitcnt = 0;
  return bits;
}

HANDLE_BIT_BUF mp3CreateBitBuffer(int bitSize)
{
  HANDLE_BIT_BUF hBitbuf = (HANDLE_BIT_BUF) mp3Alloc(sizeof(struct bitbuffer));
  bbInit((struct bitbuffer *)hBitbuf, bitSize);
  return hBitbuf;
}

void mp3DeleteBitBuffer(HANDLE_BIT_BUF hBitBuf)
{
  if (hBitBuf)
  {
    bbDestroy(hBitBuf);
    mp3Free(hBitBuf);
  }
}

int mp3GetBitsAvail(HANDLE_BIT_BUF hBitBuf)
{
  return hBitBuf->bitsAvail;
}

void mp3WriteBits(HANDLE_BIT_BUF hBitBuf,BIT_BUF_WORD bValue, unsigned int nBits)
{
  assert(bValue < (1UL<<nBits));
  if (nBits <= bbWordSize)
  {
    bbWriteBitsShort(hBitBuf, (bbWord)bValue, nBits);
  }
  else
  {
    bbWriteBitsShort(hBitBuf, (bbWord)(bValue >> 16), nBits - bbWordSize);
    bbWriteBitsShort(hBitBuf, (bbWord)(bValue & 0xffff), bbWordSize);
  }
}

BIT_BUF_WORD mp3ReadBits(HANDLE_BIT_BUF hBitBuf,unsigned int nBits)
{
  BIT_BUF_WORD rc;
  if (nBits <= bbWordSize)
  {
    rc = bbReadBitsShort(hBitBuf, nBits);
  }
  else
  {
    assert(bbWordSize*2 <= sizeof(BIT_BUF_WORD)*8);

    rc   = bbReadBitsShort(hBitBuf, bbWordSize);
    rc <<= nBits - bbWordSize;
    rc  |= bbReadBitsShort(hBitBuf, nBits-bbWordSize);
  }

  return rc;
}

int mp3ByteAlign(HANDLE_BIT_BUF hBitbuf)
{
  return bbByteAlign(hBitbuf);
}

unsigned int mp3GetBitcnt(struct bitbuffer *ptr) {
  return ptr->bitcnt;
}
