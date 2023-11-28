/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmanbitobj.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-29
 *   contents/description: HEADER - Huffman Bit Object
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:44 $
 * $Id: huffmanbitobj.h,v 1.1 2009/04/28 20:17:44 audiodsp Exp $
 */

#ifndef __HUFFMANBITOBJ_H__
#define __HUFFMANBITOBJ_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

class CBitStream;
class CHuffmanTable;

/*-------------------------------------------------------------------------*/

//
// Class holding one huffman value.
//
//  This object reads and decodes one huffman value from a CBitStream
//  object. One huffman value represents either two (big value part) or four
//  spectral lines (count-one part).
//

class CHuffmanBitObj
{
public:
  CHuffmanBitObj(const CHuffmanTable &HT);
  virtual ~CHuffmanBitObj();

  bool ReadFrom(CBitStream &BS);
  int  ToInt() const { return m_nValue; }

protected:

private:
  int                  m_nValue;
  const CHuffmanTable& m_HuffmanTable;
};

/*-------------------------------------------------------------------------*/
#endif
