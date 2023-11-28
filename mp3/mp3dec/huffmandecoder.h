/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmandecoder.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-02-08
 *   contents/description: HEADER - huffman decoder
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:44 $
 * $Id: huffmandecoder.h,v 1.1 2009/04/28 20:17:44 audiodsp Exp $
 */

#ifndef __HUFFMANDECODER_H__
#define __HUFFMANDECODER_H__

/* ------------------------ includes --------------------------------------*/

#include "bitsequence.h"
#include "huffmanbitobj.h"
#include "huffmantable.h"

/*-------------------------- defines --------------------------------------*/

class CBitStream;

/*-------------------------------------------------------------------------*/

//
// Huffman decoder (helper) class.
//
//  This object reads and decodes MPEG Layer-3 huffman data.
//

class CHuffmanDecoder
{
public:
  CHuffmanDecoder();
  virtual ~CHuffmanDecoder();

  int ReadHuffmanCode(CBitStream &Bs,
                      int        *pIsp,
                      const int  *pTableSelect,
                      const int  *pRegionEnd,
                      int         Count1TableSelect,
                      int         Part2_3Length);

protected:

private:
  int  ReadBigValues(CBitStream  &Bs,
                     int         *pIsp,
                     const int   *pTableSelect,
                     const int   *pRegionEnd);

  int  ReadCount1Area(CBitStream &Bs,
                      int        *pIsp,
                      int         Count1TableSelect,
                      int         Count1Start,
                      int         Part2_3Length);

  bool ReadHuffmanDual   (CBitStream &Bs, int *pIsp);
  bool ReadHuffmanDualLin(CBitStream &Bs, int *pIsp);
  bool ReadHuffmanQuad   (CBitStream &Bs, int *pIsp);

  CHuffmanTable  m_HuffmanTable;
  CHuffmanBitObj m_HuffmanBitObj;
  CBitSequence   m_Sign;
  CBitSequence   m_LinBits;
};

/*-------------------------------------------------------------------------*/
#endif
