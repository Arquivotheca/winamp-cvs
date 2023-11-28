/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3decode.c
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: MPEG Layer-3 decoder
 *
 *
 \***************************************************************************/

/*
 * $Date: 2009/11/29 21:04:32 $
 * $Id: mp3decode.cpp,v 1.2 2009/11/29 21:04:32 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "mp3decode.h"

#include "mp3read.h"
#include "mp3quant.h"
#include "mp3tools.h"

/* ------------------------------------------------------------------------*/

#ifdef _MSC_VER
#ifdef ERROR_CONCEALMENT
  #pragma message(__FILE__": Error-Concealment enabled")
#else
  #pragma message(__FILE__": Error-Concealment disabled")
#endif
#endif

//-------------------------------------------------------------------------*
//
//                   C M p 3 D e c o d e
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CMp3Decode::CMp3Decode
    (
    CMpegBitStream &_Bs,
    int             _Quality,
    int             _Downmix,
		int							_crc_check,
		DecoderHooks *_hooks
    ) :

  m_Mdct(m_Info, _Quality),
  m_Polyphase(m_Info, _Quality, _Downmix),
  m_AncOfl(m_Db),

  m_Bs(_Bs),
  m_Db(m_dynBufMemory, dynBufSize),

  m_Quality(_Quality),
  m_Downmix(_Downmix),
	m_crc_check(_crc_check),
	hooks(_hooks)
{
  // full reset
  Init(true);
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CMp3Decode::~CMp3Decode()
{
}

//-------------------------------------------------------------------------*
//   Init
//-------------------------------------------------------------------------*

void CMp3Decode::Init(bool fFullReset)
{
  // flush huffman buffer
  m_Db.Reset();

  // reset ancillary data and ofl
  m_AncOfl.Reset();

  if ( fFullReset )
    {
    // reset mdct
    m_Mdct.Init();

    // reset polyphase
    m_Polyphase.Init();

#ifdef ERROR_CONCEALMENT
    // reset error concealment
    m_Conceal.Init();
#endif

    // reset all spectrum members
    ZeroISpectrum();
    ZeroSpectrum();
    ZeroPolySpectrum();
    }
#ifdef KSA_DRM
  m_scfBufferIdx = 0;
#endif
}

//-------------------------------------------------------------------------*
//   Decode
//-------------------------------------------------------------------------*

SSC CMp3Decode::Decode
    (
     void *pPcm,
     int   cbPcm,
     int  *pcbUsed,
     unsigned char *ancData,
     int *numAncBytes,
     int oflOn,
     unsigned int *startDelay,
     unsigned int *totalLength
    )
{
  int  nChannels = m_Downmix ? 1 : m_Bs.GetHdr()->GetChannels();
  SSC  dwResult  = SSC_OK;
  int  nOutBytes;
  bool fCrcOk;
  bool fMainData;

  //
  // return if wrong layer
  //
  if ( m_Bs.GetHdr()->GetLayer() != 3 )
    {
    // error wrong layer
    return SSC_E_MPGA_WRONGLAYER;
    }

  //
  // calculate number of ouput bytes
  //
    nOutBytes = (m_Bs.GetHdr()->GetSamplesPerFrame() * sizeof(float) * nChannels) >> m_Quality;

  //
  // check if PCM buffer is large enough
  //
  if ( cbPcm < nOutBytes )
    {
    // error buffer too small
    return SSC_E_MPGA_BUFFERTOOSMALL;
    }

  //
  // skip mpeg header
  //
  m_Bs.Ff(m_Bs.GetHdr()->GetHeaderLen());

  //
  // set info structure
  //
  SetInfo();

  //
  // read side info (will check for crc error)
  //
  fCrcOk = mp3SideInfoRead(m_Bs, m_Si, m_Info, m_crc_check);

  //
  // prepare for ancillary data
  //
  int oldCnt = m_Db.GetBitCnt();

  //
  // read main data (will check if enough available)
  //
  fMainData = mp3MainDataRead(m_Bs, m_Db, m_Si, m_Info);

  //
  // read ancillary data
  //
  if ( m_Db.ResetOccurred() ) {

    // no ancillary data processing while dynamic buffer
    // reset, because m_Db.GetBitCnt() differences are forged.

    if ( numAncBytes != 0 ) {
      *numAncBytes = 0;
    }

    if ( fMainData ) {

      // enough main data for decoding is available, thus
      // the occurred reset is entirely processed and the
      // next ancillary data package should be valid
      // once again.

      m_Db.SetResetState( false );
    }
  }
  else {
    int newCnt = m_Db.GetBitCnt();
    if ( numAncBytes != 0 ) {
      *numAncBytes = m_AncOfl.readAnc(ancData, m_Db, (newCnt-oldCnt));
    }
  }


  //
  // decode this frame
  //
  if ( fMainData )
    {
      //
      // prepare for ofl
      //
      int beforeScf = m_Db.GetValidBits();

      //
      // decode
      //
      dwResult = DecodeNormal(pPcm, fCrcOk);

      //
      // read ofl
      //
      m_AncOfl.fetchOfl( oflOn,
                         m_Db,
                         beforeScf,
                         startDelay,
                         totalLength );
    }
  else
    {
      dwResult = DecodeOnNoMainData(pPcm);
    }

  //
  // seek to end of frame
  //
  m_Bs.Seek(m_Bs.GetHdr()->GetFrameLen() - m_Bs.GetBitCnt());

  //
  // set number of bytes used in PCM buffer
  //
  if ( pcbUsed && SSC_SUCCESS(dwResult) )
    *pcbUsed = nOutBytes;

  //
  // do a "soft" decoder reset in case of CRC error
  //
  if ( !fCrcOk )
    {
    Init(false);

    // patch dwResult (only if decoding was successfull)
    if ( SSC_SUCCESS(dwResult) )
      dwResult = SSC_I_MPGA_CRCERROR;
    }

  return dwResult;
}


//-------------------------------------------------------------------------*
//   DecodeNormal
//-------------------------------------------------------------------------*

SSC CMp3Decode::DecodeNormal(void *pPcm, bool fCrcOk)
{
  //
  // decode all channels, granules
  //

  int nChannels = m_Downmix ? 1 : m_Bs.GetHdr()->GetChannels();
  int gr, ch;

#ifdef KSA_DRM
  unsigned char* pScfBuffer = &m_scfBuffer[0];
#endif

  for ( gr=0; gr<(m_Info.IsMpeg1 ? 2:1); gr++ )
    {
    for ( ch=0; ch<m_Info.stereo; ch++ )
      {
      // read scalefactors
      mp3ScaleFactorRead(m_Db,
        m_Si.ch[ch].gr[gr],
        m_ScaleFac[ch],
        m_Info,
        m_Si.ch[ch].scfsi,
        gr,
        ch);

#ifdef KSA_DRM
      {
	/* read all scf-bits at once */
	int cnt = m_Db.GetBitCnt();
	if (cnt)
	{
	  m_Db.Rewind(cnt);
	  const int noOfBytes = cnt/8;
	  int i = 0;
	  if (noOfBytes)
	  {
	    while (i++ < noOfBytes)
	    {
	      *pScfBuffer++ = m_Db.GetBits(8);
	      cnt -= 8;
	    }

	  }
	  if (cnt)
	  {
	    *pScfBuffer++ = m_Db.GetBits(cnt);
	  }
	}
      }
#endif

      // read huffman data
      m_Mp3Huffman.Read(m_Db, m_ISpectrum[ch], m_Si.ch[ch].gr[gr], m_Info);

      // dequantize spectrum
      mp3DequantizeSpectrum(m_ISpectrum[ch], &m_Spectrum[ch][0][0],
        m_Si.ch[ch].gr[gr],
        m_ScaleFac[ch],
        m_Info);
      }
    // set last scalefactor
    mp3ScaleFactorUpdate(
      m_Si.ch[0].gr[gr], m_Si.ch[1].gr[gr],
      m_Info,
      m_ScaleFac[1] );

    // stereo processing
    mp3StereoProcessing(&m_Spectrum[0][0][0], &m_Spectrum[1][0][0],
      m_Si.ch[0].gr[gr], m_Si.ch[1].gr[gr],
      m_ScaleFac[1],
      m_Info,
      m_Downmix);

		if (hooks)
		{
			hooks->layer3_eq(&m_Spectrum[0][0][0],nChannels,m_Bs.GetHdr()->GetSampleRate());
			hooks->layer3_vis(m_Spectrum,gr,nChannels);
		}
    for ( ch=0; ch<nChannels; ch++ )
      {
      // sample reordering
      mp3Reorder(&m_Spectrum[ch][0][0], m_Si.ch[ch].gr[gr], m_Info);

      // antialiasing
      mp3Antialias(&m_Spectrum[ch][0][0], m_Si.ch[ch].gr[gr], m_Info, m_Quality);

#ifdef ERROR_CONCEALMENT
      //
      // error concealment (apply in case of crc error)
      //
      m_Conceal.Apply(!fCrcOk, m_Info, m_Si, &m_Spectrum[ch][0][0], gr, ch);
#else
      // stop compiler from complaining about unused arg
      fCrcOk = fCrcOk;
#endif

      // mdct
      m_Mdct.Apply(ch, m_Si.ch[ch].gr[gr], m_Spectrum);
      }

    // reordering (neccessary for polyphase)
    PolyphaseReorder();

    // polyphase
      pPcm = m_Polyphase.Apply(m_PolySpectrum, (float*)pPcm);
    } // for ( gr=0...)

#ifdef KSA_DRM
  m_scfBufferIdx = (pScfBuffer - &m_scfBuffer[0]);
#endif

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   DecodeOnNoMainData
//-------------------------------------------------------------------------*

SSC CMp3Decode::DecodeOnNoMainData(void *pPcm)
{
  //
  // not enough data in dynamic buffer
  //

  int nChannels = m_Downmix ? 1 : m_Bs.GetHdr()->GetChannels();
  int gr, ch;

  for ( gr=0; gr<(m_Info.IsMpeg1 ? 2:1); gr++ )
    {
    // zero spectrum
    ZeroSpectrum();

    for ( ch=0; ch<nChannels; ch++ )
      {
      // set some fields in sideinfo
      m_Si.ch[ch].gr[gr].zeroSbStartNdx        = 0;
      m_Si.ch[ch].gr[gr].window_switching_flag = 0;
      m_Si.ch[ch].gr[gr].mixed_block_flag      = 0;
      m_Si.ch[ch].gr[gr].block_type            = 0;

#ifdef ERROR_CONCEALMENT
      //
      // error concealment, predict spectrum
      //
      m_Conceal.Apply(1, m_Info, m_Si, &m_Spectrum[ch][0][0], gr, ch);
#endif

      // mdct
      m_Mdct.Apply(ch, m_Si.ch[ch].gr[gr], m_Spectrum);
      }

    // reordering (neccessary for polyphase)
    PolyphaseReorder();

    // polyphase
      pPcm = m_Polyphase.Apply(m_PolySpectrum, (float*)pPcm);
    }

  return SSC_I_MPGA_NOMAINDATA;
}

//-------------------------------------------------------------------------*
//   PolyphaseReorder
//-------------------------------------------------------------------------*

void CMp3Decode::PolyphaseReorder()
{
  int nChannels = m_Downmix ? 1 : m_Bs.GetHdr()->GetChannels();
  int ch, sb, ss;

  for ( ch=0; ch<nChannels; ch++ )
    {
    for ( ss=0; ss<SSLIMIT; ss++ )
      for ( sb=0; sb<SBLIMIT; sb++ )
        m_PolySpectrum[ch][ss][sb] = m_Spectrum[ch][sb][ss];
    }
}

//-------------------------------------------------------------------------*
//   ZeroISpectrum
//-------------------------------------------------------------------------*

void CMp3Decode::ZeroISpectrum()
{
  int ch, i;

  // reset spectrum to zero
  for ( ch=0; ch<2; ch++ )
    for ( i=0; i<SSLIMIT*SBLIMIT; i++ )
      m_ISpectrum[ch][i] = 0;
}

//-------------------------------------------------------------------------*
//   ZeroSpectrum
//-------------------------------------------------------------------------*

void CMp3Decode::ZeroSpectrum()
{
  int ch, ss, sb;

  // reset spectrum to zero
  for ( ch=0; ch<2; ch++ )
    for ( sb=0; sb<SBLIMIT; sb++ )
      for ( ss=0; ss<SSLIMIT; ss++ )
        m_Spectrum[ch][sb][ss] = 0.0f;
}

//-------------------------------------------------------------------------*
//   ZeroPolySpectrum
//-------------------------------------------------------------------------*

void CMp3Decode::ZeroPolySpectrum()
{
  int ch, ss, sb;

  // reset spectrum to zero
  for ( ch=0; ch<2; ch++ )
    for ( ss=0; ss<SSLIMIT; ss++ )
      for ( sb=0; sb<SBLIMIT; sb++ )
        m_PolySpectrum[ch][ss][sb] = 0.0f;
}

//-------------------------------------------------------------------------*
//   SetInfo
//-------------------------------------------------------------------------*

void CMp3Decode::SetInfo()
{
  static const int fhgVTab[] = {1, 0, 2};

  const CMpegHeader *hdr = m_Bs.GetHdr();

  m_Info.stereo             = hdr->GetChannels();
  m_Info.sample_rate_ndx    = hdr->GetSampleRateNdx();
  m_Info.frame_bits         = hdr->GetFrameLen();
  m_Info.mode               = hdr->GetMode();
  m_Info.mode_ext           = hdr->GetModeExt();
  m_Info.header_size        = hdr->GetHeaderLen();
  m_Info.IsMpeg1            = hdr->GetMpegVersion()==0 ? true:false;
  m_Info.fhgVersion         = fhgVTab[hdr->GetMpegVersion()];
  m_Info.protection         = hdr->GetCrcCheck();
}

//-------------------------------------------------------------------------*
//   GetScfBuffer
//-------------------------------------------------------------------------*
#ifdef KSA_DRM
int CMp3Decode::GetScfBuffer(const unsigned char** ppBuffer, unsigned int* pBufSize) const
{
  *ppBuffer = &m_scfBuffer[0];
  *pBufSize = m_scfBufferIdx;

  return 0;
}
#endif


//-------------------------------------------------------------------------*
//   GetLastAncData
//-------------------------------------------------------------------------*

SSC CMp3Decode::GetLastAncData(unsigned char* ancData,
                               int* numAncBytes)
{
  int i=0;
  int align=0;
  unsigned int waste=0;
  int numAncBits=0;

  if ( numAncBytes != 0 )
    {


      if ( m_AncOfl.doReadBytes() ) {
        align = m_Db.GetValidBits()%8;

        if(align > 0) {
          // Quick Hack:
          // Arrange last ancillary bits byte aligned for the BCC decoder.
          // Actually ancillary data is to handle bitwise. This is a future
          // TODO point.
          waste = m_Db.GetBits(align);
        }

        *numAncBytes = m_Db.GetValidBits() / 8;
      }
      else {
        if ( m_Db.GetValidBits() % 8 )
          *numAncBytes = m_Db.GetValidBits() / 8 + 1;
        numAncBits = m_Db.GetValidBits();
      }

      for(i=0; i < *numAncBytes; i++) {
        ancData[i] = m_Db.GetBits(8);
      }

      if ( !m_AncOfl.doReadBytes() && numAncBits % 8) {
        int bits = numAncBits - (*numAncBytes-1)*8;
        ancData[*numAncBytes-1] = ancData[*numAncBytes-1]<<(8-bits);
      }
    }

  return SSC_OK;
}


//-------------------------------------------------------------------------*
//   GetOflVersion
//-------------------------------------------------------------------------*

SSC CMp3Decode::GetOflVersion(int* oflVersion)
{
  if (oflVersion != 0)
    { 
      *oflVersion = m_AncOfl.getVersion();
      return SSC_OK;
    }
  else
    {
      return SSC_E_WRONGPARAMETER; 
    }
}


/*-------------------------------------------------------------------------*/