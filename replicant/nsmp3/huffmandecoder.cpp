/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmandecoder.cpp
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-02-08
 *   contents/description: huffman decoder
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/21 22:25:58 $
 * $Id: huffmandecoder.cpp,v 1.6 2011/01/21 22:25:58 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "huffmandecoder.h"
#include "bitstream.h"
#include <string.h>
/*-------------------------- defines --------------------------------------*/

#define MAXSAMPLES 576
#define REGIONS      3

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C H u f f m a n D e c o d e r
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CHuffmanDecoder::CHuffmanDecoder() : m_HuffmanBitObj(m_HuffmanTable)
{
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CHuffmanDecoder::~CHuffmanDecoder()
{
}

//-------------------------------------------------------------------------*
//   ReadHuffmanCode
//-------------------------------------------------------------------------*

int CHuffmanDecoder::ReadHuffmanCode
    (
    CBitStream  &Bs,
    int         *pIsp,
    const int   *pTableSelect,
    const int   *pRegionEnd,
    int          Count1TableSelect,
    int          Part2_3Length
    )
{
  int Count1Start;

  Count1Start = ReadBigValues(Bs,
                              pIsp,
                              pTableSelect,
                               pRegionEnd);

  return ReadCount1Area(Bs,
                        pIsp,
                        Count1TableSelect,
                        Count1Start,
                        Part2_3Length);
}

//-------------------------------------------------------------------------*
//   ReadBigValues
//-------------------------------------------------------------------------*

int CHuffmanDecoder::ReadBigValues
    (
    CBitStream  &Bs,
    int         *pIsp,
    const int   *pTableSelect,
    const int   *pRegionEnd
    )
{
	int i = 0;
	int j;
	int nLb;

	for ( j=0; j<REGIONS; j++ )
	{
		// set huffman table (also used by HuffmanBitObj)
		m_HuffmanTable.SetTableIndex(pTableSelect[j]);

		if ( m_HuffmanTable.IsTableValid() )
		{
			// get number of linbits
			nLb = m_HuffmanTable.GetLinBits();

			// read huffman data
			if ( nLb == 0 )
			{
				for ( ; i<pRegionEnd[j]; i+=2 )
					ReadHuffmanDual(Bs, &pIsp[i]);
			}
			else
			{
				// set number of bits for LinBits
				m_LinBits.SetNumberOfBits(nLb);

				for ( ; i<pRegionEnd[j]; i+=2 )
					ReadHuffmanDualLin(Bs, &pIsp[i]);
			}
		}
		else
		{
			// zero-table, zero samples
			for ( ; i<pRegionEnd[j]; i++ )
				pIsp[i] = 0;
		}
	}

	return pRegionEnd[REGIONS-1];
}

//-------------------------------------------------------------------------*
//   ReadCount1Area
//-------------------------------------------------------------------------*

int CHuffmanDecoder::ReadCount1Area
    (
    CBitStream &Bs,
    int        *pIsp,
    int         Count1TableSelect,
    int         Count1Start,
    int         Part2_3Length
    )
{
  int i = Count1Start;
  int nZero;

  // set huffman table (also used by HuffmanBitObj)
  m_HuffmanTable.SetTableIndex(Count1TableSelect);

  //
  // read quadruples until
  //   - all bits (Part2_3Length) are used or
  //   - maximum index is reached
  //
  while ( (Bs.GetBitCnt() < Part2_3Length ) && (i<=MAXSAMPLES-4) )
    {
    ReadHuffmanQuad(Bs, &pIsp[i]);
    i += 4;
    }

  int DecodingOffset = Part2_3Length - Bs.GetBitCnt();

  // check, if we have decoded a quadtruple that isn't one
  if ( DecodingOffset < 0 )
    i -= 4;

  // dismiss stuffing bits, if necessary
  if ( DecodingOffset != 0 )
    Bs.Seek(DecodingOffset);

  // step backward to see if more samples are zero
  for ( nZero = i-1; nZero>0; nZero-- )
    {
    if ( pIsp[nZero] != 0 )
      break;
    }

	// Avoid writing to negative buffer indices
  if ( nZero < -1 )
    nZero = -1;

  // zero all samples above nZero
	memset(&pIsp[nZero+1], 0, sizeof(pIsp[0])*(MAXSAMPLES-(nZero+1)));
  //for ( i=nZero+1; i<MAXSAMPLES; i++ )
//    pIsp[i] = 0;

  // above nZero all samples are zero
  // (nZero+1) is the index of the first zero-sample)
  return nZero+1;
}

//-------------------------------------------------------------------------*
//   ReadHuffmanDual
//-------------------------------------------------------------------------*

void CHuffmanDecoder::ReadHuffmanDual(CBitStream &Bs, int *pIsp)
{
  int t; 
  int x, y;
  
  
  // read huffman code
  t = m_HuffmanBitObj.ReadFrom(Bs);

  // split huffman code
  x = (t >> 4) & 0xf;
 
  // read sign bit, if necc.
  if (x)
  {
    if (Bs.Get1Bit())
      x = -x;
  }
  
	y = (t     ) & 0xf;
	// read sign bit, if necc.
  if (y )
  {
    if (Bs.Get1Bit())
			y = -y;
  }

  // store values
  pIsp[0] = x;
  pIsp[1] = y;
}

//-------------------------------------------------------------------------*
//   ReadHuffmanDualLin
//-------------------------------------------------------------------------*

void CHuffmanDecoder::ReadHuffmanDualLin
(
 CBitStream &Bs,
 int        *pIsp
 )
{
	int t;
	int x, y;

	// read huffman code
	t = m_HuffmanBitObj.ReadFrom(Bs);

	// split huffman code
	x = (t >> 4) & 0xf;


	// ESC code? read linbits
	if ( x == 15 )
	{
		m_LinBits.ReadFrom(Bs);
		x += m_LinBits.ToInt();
	}

	// read sign bit, if necc.
	if ( x > 0 )
	{
		if (Bs.Get1Bit())
			x = -x;
	}

	// store value
	pIsp[0] = x;

	y = (t     ) & 0xf;
	// ESC code? read linbits
	if ( y == 15 )
	{
		m_LinBits.ReadFrom(Bs);
		y += m_LinBits.ToInt();
	}

	// read sign bit, if necc.
	if ( y > 0 )
	{
		if (Bs.Get1Bit())
			y = -y;
	}

	// store value	
	pIsp[1] = y;
}

//-------------------------------------------------------------------------*
//   ReadHuffmanQuad
//-------------------------------------------------------------------------*

bool CHuffmanDecoder::ReadHuffmanQuad
    (
    CBitStream &Bs,
    int        *pIsp
    )
{
  int t;
  int v, w, x, y;

  // read huffman code
  t = m_HuffmanBitObj.ReadFrom(Bs);

  // split huffman code
  v = (t >> 3) & 0x1;
  // read sign bit, if necc.
  if ( v)
    {
    if (Bs.Get1Bit())
			v = -v;
    }

	w = (t >> 2) & 0x1;
  // read sign bit, if necc.
  if ( w )
    {
    if (Bs.Get1Bit())
			w = -w;
    }

	x = (t >> 1) & 0x1;
  // read sign bit, if necc.
  if ( x )
    {
    if (Bs.Get1Bit())
			x = -x;
    }

	y = (t     ) & 0x1;
  // read sign bit, if necc.
  if ( y )
    {
    if (Bs.Get1Bit())
			y = -y;
    }

  // store values
  pIsp[0] = v;
  pIsp[1] = w;
  pIsp[2] = x;
  pIsp[3] = y;

  return true;
}

/*-------------------------------------------------------------------------*/
