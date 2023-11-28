/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: bitstream.cpp
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: generic bitbuffer
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/18 23:00:53 $
 * $Id: bitstream.cpp,v 1.6 2011/01/18 23:00:53 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/
#ifdef DEBUG
#include <iostream>
#endif

#include "bitstream.h"
#include "giobase.h"
#include <string.h> // for memcpy
#include <assert.h>
/*-------------------------- defines --------------------------------------*/

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a):(b))
#endif

#ifndef NULL
  #define NULL 0
#endif

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C B i t S t r e a m
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*
#include <stdlib.h>
CBitStream::CBitStream(int cbSize)
{
  int i;

  // check cbSize (must be power of 2)
  for ( i=0; i<16; i++ )
    if ( (1 << i) >= cbSize )
      break;

  m_nBytes        = (1 << i);
	m_mask = m_nBytes-1;

#ifdef DEBUG
  if ( m_nBytes != cbSize )
    {
      // This case should never occur. Actually it
      // should be intercepted outside this constructor.
      std::cerr << "\n Wrong inbuffersize - use power of two! \n";
      std::cerr.flush();
      throw;
    }
#endif

  m_nBits         = m_nBytes * 8;
	m_bitMask = m_nBits-1;
  m_Buf           = (unsigned char *)malloc(m_nBytes);
  m_pGB           = NULL;
  m_fBufferIntern = true;

  Reset();
}

//-------------------------------------------------------------------------*

CBitStream::CBitStream(unsigned char *pBuf, int cbSize, bool fDataValid)
{
  int i;

  // check cbSize (must be power of 2)
  for ( i=0; i<16; i++ )
    if ( (1 << i) >= cbSize )
      break;

  m_nBytes        = (1 << i);
	m_mask = m_nBytes-1;

#ifdef DEBUG
  if ( m_nBytes != cbSize )
    {
      // This case should never occur. Actually it
      // should be intercepted outside this constructor.
      std::cerr << "\n Wrong inbuffersize - use power of two! \n";
      std::cerr.flush();
      throw;
    }
#endif

  m_nBits         = m_nBytes * 8;
	m_bitMask = m_nBits-1;
  m_Buf           = pBuf;
  m_pGB           = NULL;
  m_fBufferIntern = false;

  Reset();

  if ( fDataValid )
    {
    m_ValidBits   = m_nBits;
    }
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CBitStream::~CBitStream()
{
  if ( m_fBufferIntern && m_Buf )
    free(m_Buf);
}

//-------------------------------------------------------------------------*
//   Reset
//-------------------------------------------------------------------------*

void CBitStream::Reset()
{
  m_ValidBits  = 0;
  m_ReadOffset = 0;
  m_BitCnt     = 0;
  m_BitNdx     = 0;
  m_fEof       = false;
  m_ResetOccurred = true;
}

//-------------------------------------------------------------------------*
//   Connect
//-------------------------------------------------------------------------*

void CBitStream::Connect(CGioBase *pGB)
{
  m_pGB = pGB;
}

//-------------------------------------------------------------------------*
//   GetBits
//-------------------------------------------------------------------------*
unsigned int CBitStream::GetBits(unsigned int nBits)
{
	assert(nBits<=16);
	unsigned int ret;
	int pos;
	if (!nBits) return 0;
	pos = m_BitNdx>>3;
  m_ValidBits -= nBits;
	ret=(m_Buf[pos]<<16)|(m_Buf[(pos+1)&m_mask]<<8)|(m_Buf[(pos+2)&m_mask]);
  m_BitCnt    += nBits;
	ret <<= (m_BitNdx&7)+8;
	m_BitNdx+=nBits;
	m_BitNdx&= (m_bitMask);
	return ret>>(32-nBits);
}


unsigned int CBitStream::GetBits8(unsigned int nBits)
{
	assert(nBits<=8);
	unsigned int ret;
	int pos;
	if (!nBits) return 0;
	pos = m_BitNdx>>3;
  m_ValidBits -= nBits;
	ret=(m_Buf[pos]<<8)|(m_Buf[(pos+1)&m_mask]);
  m_BitCnt    += nBits;
	ret <<= (m_BitNdx&7)+16;
	m_BitNdx+=nBits;
	m_BitNdx&= (m_bitMask);
	return ret>>(32-nBits);
}


//-------------------------------------------------------------------------*
//   Get1Bit
//-------------------------------------------------------------------------*
unsigned int CBitStream::Get1Bit()
{
		unsigned char ret;
	ret=m_Buf[m_BitNdx>>3];
	ret <<= m_BitNdx&7;
	ret >>= 7;
	m_BitNdx++;
	m_BitNdx&=m_bitMask;
  m_BitCnt++;
  m_ValidBits--;
	return ret;
}


//-------------------------------------------------------------------------*
//   Get32Bits
//-------------------------------------------------------------------------*

unsigned long CBitStream::Get32Bits()
{
  unsigned long tmp;

  tmp  = (unsigned long)(GetBits(16)) << 16;
  tmp |= GetBits(16);

  return tmp;
}

//-------------------------------------------------------------------------*
//   GetFree
//-------------------------------------------------------------------------*

int CBitStream::GetFree() const
{
  return (m_nBits - m_ValidBits) / 8;
}

//-------------------------------------------------------------------------*
//   SetEof
//-------------------------------------------------------------------------*

void CBitStream::SetEof()
{
  m_fEof = true;
}

//-------------------------------------------------------------------------*
//   Fill
//-------------------------------------------------------------------------*

int CBitStream::Fill(const unsigned char *pBuf, int cbSize)
{
  const unsigned char *ptr    = pBuf;
  int                  bTotal = 0;
  int                  noOfBytes;
  int                  bToRead;

  bToRead   = GetFree();
  noOfBytes = min(bToRead, cbSize);

  while ( noOfBytes > 0 )
    {
    // Split Read to buffer size
    bToRead = min(m_nBytes - m_ReadOffset, noOfBytes);

    // copy 'bToRead' bytes from 'ptr' to buffer
    for ( int i=0; i<bToRead; i++ )
      m_Buf[m_ReadOffset + i] = ptr[i];

    // add noOfBits to number of valid bits in buffer
    m_ValidBits  += bToRead * 8;
    bTotal       += bToRead;
    ptr          += bToRead;

    m_ReadOffset  = (m_ReadOffset + bToRead) & m_mask;
    noOfBytes    -= bToRead;
    }

  return bTotal;
}

//-------------------------------------------------------------------------*

int CBitStream::Fill(CBitStream &Bs, int cbSize)
{
	int bTotal = 0;
	int noOfBytes;
	int bToRead;
	int i;

	// limit cbSize to number of valid bytes in 'Bs'
	bToRead   = Bs.GetValidBits() / 8;
	cbSize    = min(cbSize, bToRead);

	// limit to number of free bytes of this object
	bToRead   = GetFree();
	noOfBytes = min(bToRead, cbSize);

	while (noOfBytes > 0)
	{
		// Split Read to buffer size
		bToRead = min(m_nBytes - m_ReadOffset, noOfBytes);

		if (Bs.ByteAligned())
		{
			int source_byte_position = Bs.m_BitNdx>>3;
			bToRead = min(Bs.m_nBytes - source_byte_position, bToRead);
			memcpy(&m_Buf[m_ReadOffset], &Bs.m_Buf[source_byte_position], bToRead);
			//for ( i=0; i<bToRead; i++ )
//				m_Buf[m_ReadOffset + i] = Bs.m_Buf[source_byte_position + i];
			Bs.Seek(bToRead*8);
		}
		else
		{
			// copy 'bToRead' bytes from 'Bs' to buffer
			for ( i=0; i<bToRead; i++ )
				m_Buf[m_ReadOffset + i] = (unsigned char)Bs.GetBits8(8);
		}

		// add noOfBits to number of valid bits in buffer
		m_ValidBits  += bToRead * 8;
		bTotal       += bToRead;

		m_ReadOffset  = (m_ReadOffset + bToRead) & m_mask;
		noOfBytes    -= bToRead;
	}

	return bTotal;
}

//-------------------------------------------------------------------------*
//   Refill
//-------------------------------------------------------------------------*

int CBitStream::Refill()
{
  int noOfBytes = GetFree();
  int bTotal    = 0;
  int bToRead, bRead;

  // check if connected
  if ( !IsConnected() )
    {
    return 0;
    }

  while ( noOfBytes > 0 )
    {
    // split read to buffer size
    bToRead = min(m_nBytes - m_ReadOffset, noOfBytes);

    m_pGB->Read(m_Buf+m_ReadOffset, bToRead, &bRead);

    // missing: check for read errors!!

    // add noOfBits to number of valid bits in buffer
    m_ValidBits  += bRead * 8;
    bTotal       += bRead;

    m_ReadOffset  = (m_ReadOffset + bRead) & m_mask;
    noOfBytes    -= bToRead;

    if ( bRead < bToRead )
      break;
    }

  // check for EOF
  if ( m_pGB->IsEof() )
    SetEof();

  return bTotal;
}

//-------------------------------------------------------------------------*
//   IsEof
//-------------------------------------------------------------------------*

bool CBitStream::IsEof() const
{
  return m_fEof;
}

//-------------------------------------------------------------------------*
//   IsConnected
//-------------------------------------------------------------------------*

bool CBitStream::IsConnected() const
{
  return (m_pGB != NULL);
}

/*-------------------------------------------------------------------------*/
