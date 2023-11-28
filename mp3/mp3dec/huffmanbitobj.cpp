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

/* ------------------------ includes --------------------------------------*/

#include "huffmanbitobj.h"
#include "bitstream.h"
#include "huffmantable.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C H u f f m a n B i t O b j
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CHuffmanBitObj::CHuffmanBitObj(const CHuffmanTable& HT) : m_HuffmanTable(HT)
{
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CHuffmanBitObj::~CHuffmanBitObj()
{
}

//-------------------------------------------------------------------------*
//   ReadFrom
//-------------------------------------------------------------------------*

bool CHuffmanBitObj::ReadFrom(CBitStream &BS)
{
  unsigned int bits;
  unsigned int tab_ndx       = 0;
  int          s_BitCnt      = BS.GetBitCnt();
  unsigned int nBitsPerLevel = m_HuffmanTable.GetBitsPerLevel();

  while ( 1 )
    {
    bits = BS.GetBits(nBitsPerLevel);

    if ( m_HuffmanTable.IsLengthZero(tab_ndx, bits) )
      {
      tab_ndx = m_HuffmanTable.GetCode(tab_ndx, bits);
      }
    else
      {
      /*
       * stuff back bits, that are not part of the actual huffman value
       * (<nBitsPerLevel>bit huffman decoder!)
       *
       * bits read              : GetBitCnt() - <saved bitcount>
       * bits to read           : CHuffmanTable::GetLength()
       *
       * bits to seek (forward!): <bits to read> - <bits read>
       */

      int nBitsRead   = BS.GetBitCnt() - s_BitCnt;
      int nBitsToRead = m_HuffmanTable.GetLength(tab_ndx, bits);

      BS.Seek(nBitsToRead - nBitsRead);

      m_nValue = m_HuffmanTable.GetCode(tab_ndx, bits);

      return true;
      }
    }
}

/*-------------------------------------------------------------------------*/
