/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3decode.h
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: MPEG Layer-3 decoder
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/11/29 21:04:32 $
 * $Id: mp3decode.h,v 1.2 2009/11/29 21:04:32 audiodsp Exp $
 */

#ifndef __MP3DECODE_H__
#define __MP3DECODE_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"
#include "mpegbitstream.h"
#include "huffdec.h"
#include "mdct.h"
#include "polyphase.h"
#include "mp3ancofl.h"

#ifdef ERROR_CONCEALMENT
  #include "conceal.h"
#endif

/*-------------------------------------------------------------------------*/

//
// MPEG Layer-3 decoding class.
//
//  This is the main MPEG Layer-3 decoder object.
//

class CMp3Decode
{
public:

  CMp3Decode(CMpegBitStream &_Bs, int _Quality, int _Downmix, int _crc_check, DecoderHooks *_hooks=0);

  ~CMp3Decode();

  void Init(bool fFullReset = true);

  // PcmFormat: 0: integer, 1: 32 bit float (IEEE)
  SSC Decode(void *pPcm,
             int cbPcm,
             int *pcbUsed,
             unsigned char *ancData,
             int *numAncBytes = 0,
             int oflOn = 0,
             unsigned int *startDelay = 0,
             unsigned int *totalLength = 0);


#ifdef KSA_DRM
  int GetScfBuffer(const unsigned char** ppBuffer,
		   unsigned int* pBufSize) const;
#endif

  SSC GetLastAncData(unsigned char* ancData,
                     int *numAncBytes);

	  SSC GetOflVersion(int* oflVersion);

protected:

  SSC  DecodeOnNoMainData(void *pPcm);
  SSC  DecodeNormal      (void *pPcm, bool fCrcOk);

  void PolyphaseReorder();
  void ZeroISpectrum();
  void ZeroSpectrum();
  void ZeroPolySpectrum();
  void SetInfo();

  CMp3Huffman       m_Mp3Huffman;  // huffman decoder
  CMdct             m_Mdct;        // mdct
  CPolyphase        m_Polyphase;   // polyphase
  CMp3AncOfl	    m_AncOfl;	   // ancillary data and ofl

#ifdef ERROR_CONCEALMENT
  CErrorConcealment m_Conceal;     // error concealment
#endif

  MPEG_INFO         m_Info;        // info structure
  CMpegBitStream   &m_Bs;          // bitstream
  CBitStream        m_Db;          // dynamic buffer
  MP3SI             m_Si;          // side info
  MP3SCF            m_ScaleFac[2]; // scalefactors

  int               m_ISpectrum[2][SSLIMIT*SBLIMIT]; // spectrum (integer)
  SPECTRUM          m_Spectrum;                      // spectrum (float)
  POLYSPECTRUM      m_PolySpectrum;                  // spectrum (post-mdct)

  int               m_Quality;        // 0: full, 1: half, 2: quarter
	int								m_crc_check;			// 0: no CRC check, 1: fail on CRC errors
public:
  int               m_Downmix;        // 0: no downmix, 1: downmix
protected:

  enum { dynBufSize = 2048 } ;

  unsigned char     m_dynBufMemory [dynBufSize] ;

#ifdef KSA_DRM
  enum { scfBufSize = 4 * 288 } ;
  unsigned char     m_scfBuffer [scfBufSize];
  unsigned int m_scfBufferIdx;
#endif

private:
DecoderHooks *hooks;
};

/*-------------------------------------------------------------------------*/
#endif
