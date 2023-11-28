/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpegbitstream.cpp
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: ISO/MPEG bitstream
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mpegbitstream.cpp,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "mpegbitstream.h"

/*-------------------------- defines --------------------------------------*/

/*
 * mask syncword, idex, id, layer, sampling-frequency
 */
static const unsigned long gdwHeaderSyncMask = 0xfffe0c00L;


/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C M p e g B i t S t r e a m
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CMpegBitStream::CMpegBitStream(int cbSize) : CBitStream(cbSize)
{
  Reset();
}

//-------------------------------------------------------------------------*

CMpegBitStream::CMpegBitStream(unsigned char *pBuf, int cbSize, bool fDataValid) :
    CBitStream(pBuf, cbSize, fDataValid)
{
  // must do the same as Reset(), exept of call to CBitStream::Reset()!!
  m_SyncState      = SSC_W_MPGA_SYNCSEARCHED;
  m_FirstHdr       = 0;
  m_nFramesToCheck = 0;
  m_SyncPosition   = 0;
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CMpegBitStream::~CMpegBitStream()
{
}

//-------------------------------------------------------------------------*
//   Reset
//-------------------------------------------------------------------------*

void CMpegBitStream::Reset()
{
  CBitStream::Reset();

  m_SyncState      = SSC_W_MPGA_SYNCSEARCHED;
	// JUSTIN 4/28/1999 removed m_FirstHdr=0 for better seeking
  //m_FirstHdr       = 0;
  m_nFramesToCheck = 0;
  m_SyncPosition   = 0;
}

//-------------------------------------------------------------------------*
//   DoSync
//-------------------------------------------------------------------------*

SSC CMpegBitStream::DoSync()
{
  // don't do anything in case of EOF
  if ( m_SyncState == SSC_W_MPGA_SYNCEOF )
    return m_SyncState;

  // automatic refill if connected
  if ( IsConnected() &&
       ( (m_Hdr.GetFrameLen() && GetValidBits() < m_Hdr.GetFrameLen()) ||
         (m_SyncState == SSC_W_MPGA_SYNCNEEDDATA) ||
         (m_SyncState == SSC_W_MPGA_SYNCSEARCHED) ||
         (GetValidBits() == 0)
       )
     )
    {
    Refill();
    }

  // do the sync
  if ( GetValidBits() < 32 )
    {
    // if there are less than 32 bits -> no sync possible
    if ( (m_SyncState == SSC_OK) || (m_SyncState == SSC_W_MPGA_SYNCNEEDDATA) )
      m_SyncState = SSC_W_MPGA_SYNCNEEDDATA;
    else
      m_SyncState = SSC_W_MPGA_SYNCSEARCHED;
    }
  else
    {
    // at least 32bits in buffer, try to sync
    if ( (m_SyncState == SSC_OK) || (m_SyncState == SSC_W_MPGA_SYNCNEEDDATA) )
      {
      // sync continue
      m_SyncState = DoSyncContinue();
      }
    else
      {
      // initial sync
      m_SyncState = DoSyncInitial();
      }
    }

  // check for EOF
  if ( IsEof() &&
       (m_SyncState == SSC_W_MPGA_SYNCSEARCHED || m_SyncState == SSC_W_MPGA_SYNCNEEDDATA) )
    m_SyncState = SSC_W_MPGA_SYNCEOF;

  return m_SyncState;
}

//-------------------------------------------------------------------------*
//   DoSyncInitial
//-------------------------------------------------------------------------*
#define CHECK_SIZE(x) if (GetValidBits() < x.GetFrameLen())     {        Rewind(GetBitCnt());        return SSC_W_MPGA_SYNCSEARCHED;        }
SSC CMpegBitStream::DoSyncInitial()
{
  unsigned long ulHdr1;
  unsigned long ulHdr2;
  CMpegHeader hdr2;

  // keep track of actual position
  ResetBitCnt();

  // (try to) sync while there are more than 32 bits
  while ( GetValidBits() >= 32 )
    {
    // read header (32bits)
    ulHdr1 = Get32Bits();

    // check, if header is valid
    if ( m_Hdr.FromInt(ulHdr1) )
      {
				CHECK_SIZE(m_Hdr);
        Ff(m_Hdr.GetFrameLen()-32);
        ulHdr2 = Get32Bits();
        if (!((ulHdr1 ^ ulHdr2) & gdwHeaderSyncMask) && hdr2.FromInt(ulHdr2))
        {
					CHECK_SIZE(hdr2);
          Ff(hdr2.GetFrameLen()-32);
          ulHdr2 = Get32Bits();
          if (!((ulHdr1 ^ ulHdr2) & gdwHeaderSyncMask) && hdr2.FromInt(ulHdr2))
          {
						CHECK_SIZE(hdr2);
            Ff(hdr2.GetFrameLen()-32);
            ulHdr2 = Get32Bits();
            if (!((ulHdr1 ^ ulHdr2) & gdwHeaderSyncMask) && hdr2.FromInt(ulHdr2))
            {
              m_FirstHdr = ulHdr1 & gdwHeaderSyncMask;
              Rewind(GetBitCnt());
              return SSC_OK;
            }
          }
        }
      
      }

      // skip one byte and try again [ JF-11/17/00, they were seeking by 1 bit, silly them ]
    m_SyncPosition++;
    Rewind(GetBitCnt()-8); 
    ResetBitCnt();
    }

  // buffer emtpy
  return SSC_W_MPGA_SYNCSEARCHED;
}
#undef CHECK_SIZE
//-------------------------------------------------------------------------*
//   DoSyncContinue
//-------------------------------------------------------------------------*

SSC CMpegBitStream::DoSyncContinue()
{
  unsigned long ulHdr1;
  ResetBitCnt();
  ulHdr1 = Get32Bits();
  Rewind(GetBitCnt());
  if ( ((ulHdr1 & gdwHeaderSyncMask) != m_FirstHdr) || !m_Hdr.FromInt(ulHdr1) )
  {
    return SSC_W_MPGA_SYNCLOST;
  }
  if (GetValidBits() < m_Hdr.GetFrameLen()) // see if we have enough data for the frame :)
  {
    return SSC_W_MPGA_SYNCNEEDDATA;
  }
  return SSC_OK;
}

/*-------------------------------------------------------------------------*/
