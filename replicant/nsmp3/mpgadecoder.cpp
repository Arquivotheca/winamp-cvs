/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpgadecoder.cpp
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: MPEG Decoder class
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/28 21:45:29 $
 * $Id: mpgadecoder.cpp,v 1.5 2011/01/28 21:45:29 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "mpgadecoder.h"

/*-------------------------- defines --------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C M p g a D e c o d e r
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

void *CMpgaDecoder::operator new(size_t stAllocateBlock)
{
#if defined(__ANDROID__)
	void *decoder = malloc(stAllocateBlock);
#elif defined(__linux__) || defined(__APPLE__)
	void *decoder=0;
	posix_memalign(&decoder, 16, stAllocateBlock);
#elif defined(_WIN32)
   void *decoder = _aligned_malloc(stAllocateBlock, 16);
#endif
   return decoder;
}

void CMpgaDecoder::operator delete(void *decoder)
{
#if defined(__ANDROID__)
	free(decoder);
#elif defined(__linux__)  || defined(__APPLE__)
	free(decoder);
#elif defined(_WIN32)
   _aligned_free(decoder);
#endif	
}

CMpgaDecoder::CMpgaDecoder(int crcCheck) :
  m_Mbs(8192),
  m_Mp3Decode(m_Mbs, crcCheck),
  m_Mp2Decode(m_Mbs)
{
  // reset myself
  Reset();
	m_Layer=-1;
}

CMpgaDecoder::CMpgaDecoder(DecoderHooks *hooks,  int crcCheck) :
  m_Mbs(8192),
  m_Mp3Decode(m_Mbs,  crcCheck, hooks),
  m_Mp2Decode(m_Mbs,  hooks)
{
  // reset myself
  Reset();
	m_Layer=-1;

}

//-------------------------------------------------------------------------*
//   constructor with external bitbuffer memory
//-------------------------------------------------------------------------*

CMpgaDecoder::CMpgaDecoder(unsigned char *pBuf, int cbSize, int crcCheck) :
  m_Mbs(pBuf, cbSize),
  m_Mp3Decode(m_Mbs, crcCheck),
  m_Mp2Decode(m_Mbs)
{
  // reset myself
  Reset();
	  m_Layer=-1;
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CMpgaDecoder::~CMpgaDecoder()
{
}

//-------------------------------------------------------------------------*
//   Reset
//-------------------------------------------------------------------------*

void CMpgaDecoder::Reset()
{
  // no, we are not eof
  m_IsEof = false;

  // reset MPEG bitstream object
  m_Mbs.Reset();

  // reset MPEG Layer-3 decoder object
  m_Mp3Decode.Init(true);
	  // reset MPEG Layer-2 decoder object
  m_Mp2Decode.Init(true);

}

//-------------------------------------------------------------------------*
//   DecodeFrame
//-------------------------------------------------------------------------*


SSC CMpgaDecoder::DecodeFrame(float *pPcm,
                              size_t cbPcm,
                              size_t *pcbUsed,
                              unsigned char *ancData,
                              size_t *numAncBytes,
                              int oflOn,
                              unsigned int *startDelay,
                              unsigned int *totalLength)
{
	SSC dwReturn;

	//
	// reset pcbUsed
	//
	if ( pcbUsed )
		*pcbUsed = 0;

	//
	// sync to next frame
	//
	dwReturn = m_Mbs.DoSync();

	//
	// check for success
	//
	if ( SSC_SUCCESS(dwReturn) )
	{
		//
		// success, do the work
		//

		// decode one frame
		// (decoder has to eat up all data of this frame!!)
		if (m_Layer != -1) 
		{
			if (m_Mbs.GetHdr()->GetLayer() != m_Layer)
				return SSC_E_MPGA_WRONGLAYER;

		}
		else m_Layer=m_Mbs.GetHdr()->GetLayer();

		if (pPcm)
		{
			if (m_Layer==3) 
				dwReturn = m_Mp3Decode.Decode(pPcm, cbPcm, pcbUsed, ancData, numAncBytes, oflOn, startDelay, totalLength);
			else
				dwReturn = m_Mp2Decode.Decode(pPcm, cbPcm, pcbUsed);
		}

		// set streaminfo object
		SetStreamInfo(dwReturn);
	}
	else if ( dwReturn == SSC_W_MPGA_SYNCEOF )
	{
		//
		// end of file (track) reached
		//

		m_IsEof = true;
	}
	else if ( dwReturn == SSC_W_MPGA_SYNCLOST )
	{
		//
		// sync lost: reset decoder, but before flush
		//            data in dynamic buffer to receive
		//            bcc info. Attention: this ancillary
		//            data package is not valid!
		//
		m_Mp3Decode.GetLastAncData(ancData, numAncBytes);
		// CHANGED TO 'false' By JUSTIN on 5/5/99
		m_Mp3Decode.Init(false);
		m_Mp2Decode.Init(false);
	}
	else
	{
		//
		// handle all other sync states
		//
	}

	return dwReturn;
}

//-------------------------------------------------------------------------*
//   GetStreamInfo
//-------------------------------------------------------------------------*

const CMp3StreamInfo *CMpgaDecoder::GetStreamInfo() const
{
  //
  // return pointer to MP3 streaminfo object
  //
  return &m_Info;
}

//-------------------------------------------------------------------------*
//   Connect
//-------------------------------------------------------------------------*

void CMpgaDecoder::Connect(CGioBase *gf)
{
  //
  // connect "gf" with bistream object
  // data will be read automatically by MPEG bitstream object
  // do not call "Fill", if connected!!
  //
  m_Mbs.Connect(gf);
}

//-------------------------------------------------------------------------*
//   Fill
//-------------------------------------------------------------------------*

int CMpgaDecoder::Fill(const unsigned char *pBuffer, int cbBuffer)
{
  //
  // provide data to bitstream object
  //
  return m_Mbs.Fill(pBuffer, cbBuffer);
}

//-------------------------------------------------------------------------*
//   GetInputFree
//-------------------------------------------------------------------------*

int CMpgaDecoder::GetInputFree() const
{
  //
  // get number of bytes bitstream object will accept
  //
  return m_Mbs.GetFree();
}

//-------------------------------------------------------------------------*
//   GetInputLeft
//-------------------------------------------------------------------------*

int CMpgaDecoder::GetInputLeft() const
{
  //
  // get number of bytes left in bitstream object
  //
  return (m_Mbs.GetValidBits() / 8);
}

//-------------------------------------------------------------------------*
//   SetInputEof
//-------------------------------------------------------------------------*

void CMpgaDecoder::SetInputEof()
{
  //
  // indicate end-of-input-data to MPEG bitstream object
  // note: the bitstream object may still have some data,
  //       end-of-output-data will be indicated by this object
  //
  m_Mbs.SetEof();
}

//-------------------------------------------------------------------------*
//   IsEof
//-------------------------------------------------------------------------*

bool CMpgaDecoder::IsEof() const
{
  //
  // no more PCM data will be produced after EOF
  //
  return m_IsEof;
}

//-------------------------------------------------------------------------*
//   SetStreamInfo
//-------------------------------------------------------------------------*

void CMpgaDecoder::SetStreamInfo(SSC dwReturn)
{
  //
  // set streaminfo object
  //

  const CMpegHeader *hdr = m_Mbs.GetHdr();

  m_Info.SetLayer            (hdr->GetLayer());
  m_Info.SetMpegVersion      (hdr->GetMpegVersion());
  m_Info.SetBitrate          (hdr->GetBitrate());
  m_Info.SetBitrateIndex     (hdr->GetBitrateNdx());
  m_Info.SetChannels         (hdr->GetChannels());
  m_Info.SetSFreq            (hdr->GetSampleRate());
  m_Info.SetBitsPerFrame     (hdr->GetFrameLen());
  m_Info.SetDuration         (hdr->GetDuration());
	m_Info.SetSamplesPerFrame  (hdr->GetSamplesPerFrame());
  m_Info.SetCrcError         (0);
  m_Info.SetNoMainData       (0);

  switch ( dwReturn )
    {
    case SSC_I_MPGA_CRCERROR:
      m_Info.SetCrcError(1);
      break;
    case SSC_I_MPGA_NOMAINDATA:
      m_Info.SetNoMainData(1);
      break;
    default:
      break;
    }
}

//-------------------------------------------------------------------------*
//   GetScfBuffer
//-------------------------------------------------------------------------*

#ifdef KSA_DRM
int CMpgaDecoder::GetScfBuffer(const unsigned char** ppBuffer, unsigned int* pBufSize) const
{
  return m_Mp3Decode.GetScfBuffer(ppBuffer, pBufSize);
}
#endif


//-------------------------------------------------------------------------*
//   GetLastAncData
//-------------------------------------------------------------------------*

SSC CMpgaDecoder::GetLastAncData(unsigned char* ancData,
                                 size_t *numAncBytes)
{
  return m_Mp3Decode.GetLastAncData(ancData, numAncBytes);
}

//-------------------------------------------------------------------------*
//   GetOflVersion
//-------------------------------------------------------------------------*

SSC CMpgaDecoder::GetOflVersion(int* oflVersion)
{
  if (oflVersion != 0)
    {
      m_Mp3Decode.GetOflVersion(oflVersion);
      return SSC_OK;
    }
  else
    {
      return SSC_E_WRONGPARAMETER; 
    }
}

/*-------------------------------------------------------------------------*/
