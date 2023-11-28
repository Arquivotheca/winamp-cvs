/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpgadecoder.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: MPEG Decoder class - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/11/29 21:04:32 $
 * $Id: mpgadecoder.h,v 1.2 2009/11/29 21:04:32 audiodsp Exp $
 */

#ifndef __MPGADECODER_H__
#define __MPGADECODER_H__

/* ------------------------ includes --------------------------------------*/

#include "mp3sscdef.h"
#include "mp3streaminfo.h"
#include "mpegbitstream.h"
#include "mp3decode.h"
#include "mp2decode.h"


/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//
// Mp3 Decoder Top Level Object.
//
// This is the main ISO/MPEG decoder object that interfaces with the
// application code.
//
// It is however recommended to use IMpgaDecoder (see mp3decifc.h) instead.
// Define USE_MP3DECIFC when planning to use IMpgaDecoder.
//

enum
{
	MPEGAUDIO_QUALITY_FULL = 0,
	MPEGAUDIO_QUALITY_HALF = 1,
	MPEGAUDIO_QUALITY_QUARTER = 2,
	MPEGAUDIO_DOWNMIX_NONE = 0,
	MPEGAUDIO_DOWNMIX_MONO = 1,
	MPEGAUDIO_CRCCHECK_OFF = 0,
	MPEGAUDIO_CRCCHECK_ON = 0,
};

class CMpgaDecoder
{
public:
	CMpgaDecoder(int Quality = MPEGAUDIO_QUALITY_FULL, int Downmix = MPEGAUDIO_DOWNMIX_NONE, int crcCheck = MPEGAUDIO_CRCCHECK_OFF);
	CMpgaDecoder(DecoderHooks *hooks, int Quality = MPEGAUDIO_QUALITY_FULL, int Downmix = MPEGAUDIO_DOWNMIX_NONE, int crcCheck = MPEGAUDIO_CRCCHECK_OFF);

	CMpgaDecoder(unsigned char *pBuf, int cbSize, int Quality = MPEGAUDIO_QUALITY_FULL, int Downmix = MPEGAUDIO_DOWNMIX_NONE, int crcCheck = MPEGAUDIO_CRCCHECK_OFF);

	~CMpgaDecoder();

	void Reset();

	SSC  DecodeFrame(float *pPcm,
	                 int cbPcm,
	                 int *pcbUsed = 0,
	                 unsigned char *ancData = 0,
	                 int *numAncBytes = 0,
	                 int oflOn = 0,
	                 unsigned int *startDelay = 0,
	                 unsigned int *totalLength = 0);

	const CMp3StreamInfo *GetStreamInfo() const;
	
	void Connect(CGioBase *gf);
	int  Fill(const unsigned char *pBuffer, int cbBuffer);
	int  GetInputFree() const;
	int  GetInputLeft() const;
	void SetInputEof();
	bool IsEof() const;
	void SetDownmix() // added by nullsoft
	{
		m_Downmix=1;
		m_Mp2Decode.m_Downmix=1;
		m_Mp3Decode.m_Downmix=1;
	}
#ifdef KSA_DRM
	int GetScfBuffer(const unsigned char** ppBuffer, unsigned int* pBufSize) const;
#endif

	SSC GetLastAncData(unsigned char* ancData = 0,
	                   int *numAncBytes = 0);

	  SSC GetOflVersion(int* oflVersion = 0);
//protected:

	void SetStreamInfo(SSC dwReturn);


	CMp3StreamInfo m_Info;
	CMpegBitStream m_Mbs;
	CMp3Decode     m_Mp3Decode;
	CMp2Decode     m_Mp2Decode;
	int            m_Quality;
	int            m_Downmix;
	bool           m_IsEof;
	int			 m_Layer;

private:

};

/*-------------------------------------------------------------------------*/
#endif
