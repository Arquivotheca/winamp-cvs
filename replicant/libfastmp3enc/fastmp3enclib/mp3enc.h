/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3enc.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:       W. Schildbach                                                                *
*   contents/description: Fast MPEG Layer-3 encoder interface file                             *
*                                                                                              *
************************************************************************************************/

#ifndef _mp3enc_h_
#define _mp3enc_h_

#include "mconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------ structure alignment ---------------------------*/

#ifdef WIN32
#pragma pack(push, 1)
#endif

/*-------------------------- defines --------------------------------------*/

/*
 * calling convention
 */

#ifndef MP3ENCAPI
#ifdef WIN32
#define MP3ENCAPI __stdcall
#else
#define MP3ENCAPI
#endif
#endif

/*
 * macros to create and extract version numbers
 */

#ifndef MP3ENC_VERSION
#define MP3ENC_MAKE_VERSION(m,n,b) (((m&0xff)<<24)|((n&0xff)<<16)|(b&0xffff))
#define MP3ENC_MAJOR_VERSION(v) ((unsigned)(v & 0xff000000) >> 24)
#define MP3ENC_MINOR_VERSION(v) ((unsigned)(v & 0xff0000) >> 16)
#define MP3ENC_BUILD_NUMBER(v) ((unsigned)(v & 0xffff))

/*
 * the version of library these include files are used for
 *
 * FORMAT OF VERSION: mmnnbbbb
 *                    mm  : major (8bit)
 *                    nn  : minor (8bit)
 *                    bbbb: build (16bit)
 */

#define MP3ENC_VERSION MP3ENC_MAKE_VERSION(0, 99, 17)
#endif

/*-------------------------------------------------------------------------*/

#define MP3ENC_OK        0   /* function successful */
#define MP3ENC_READY     -2
#define MP3ENC_BLOCKSIZE 576 /* encoder only takes BLOCKSIZE samples at a time */

/*-------------------------------------------------------------------------*/

  enum
  {
    MP3ENC_PADDING_ISO,
    MP3ENC_PADDING_NEVER,
    MP3ENC_PADDING_ALWAYS,
    MP3ENC_PADDING_NEAREST
  } ;

/*-------------------- structure definitions ------------------------------*/

  typedef enum mp3_ancillary_mode
  { 
    ANC_RAW_MODE,
    ANC_FHG_MODE
  }mp3_ancillary_mode;

  typedef struct mp3_ancillary
  {
    mp3_ancillary_mode anc_Mode;
    unsigned int anc_Rate;  
  }mp3_ancillary, *mp3_ancillaryPtr;

  struct MP3_ENCODER;

  struct MP3ENC_CONFIG
  {
  int   sampleRate;      /* encoder sample rate */
  int   bitRate;         /* encoder bit rate in bits/sec */
  int   nChannelsIn;     /* number of channels on input (1,2) */
  int   nChannelsOut;    /* number of channels on output (1,2) */

  float bandWidth;       /* targeted audio bandwidth in Hz */
  int   fNoIntensity;    /* forbid usage of intensity stereo */
  int   fVbrMode;        /* set to true to run variable bitrate */
  int   vbrQuality;      /* quality for variable bitrate 0: best, 4: worst */
  int   fFullHuffman;    /* enable/disable full huffman search */
  
  int   useMS;           /* flag for MS stereo usage */

  int   paddingMode;     /* padding on/off/ISO/nearest */

  int   fCrc;            /* set to true to write CRC checks */
  int   privateBit;      /* private bit of MPEG Header */
  int   copyRightBit;    /* copyright bit of MPEG Header */
  int   originalCopyBit; /* original bit of MPEG Header */

  int   stPreProFlag;     /* use stereo pre processing */

  int   dualMonoFlag;    /* use Dual-Mono-Mode: i.e independent channels */

  int   pcmResolution;   /* Input Signal resolution */

  mp3_ancillary ancillary; /* ancillary data struct */

  int useDualPass;

  int meanBitrate;
  int predictGranules;    /* number of granules to predict in mbr mode */
  /* for dual-pass coding */
  int dualPassLoop;
  struct PE_STATS *peStats;
};

struct MP3ENC_INFO
{
  float bandWidth;       /* audio bandwidth in Hz */
  int   nDelay;          /* encoder delay in units of sample frames */
  int   cbBufsizeMin;    /* minimum size of output buffer (bytes) */
  int   nFramesInMeta;   /* ISO/MPEG Layer 3 frames contained in meta frame */
  int   cbMetaframeSize; /* number of bytes per meta frame */
  int   granPerFrame;    /* granules per frame */
  int   ancBitsPerFrame; /* Ancillary bits per frame */
};

/*---------------------------------------------------------------------------

    functionname:mp3encInit
    description: allocate global resources for the layer-3 library
    returns:     MP3ENC_OK if success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encInit
(
 unsigned long needVersion, /* pass MP3ENC_VERSION to check compatibility */
 unsigned long *pVersion    /* contains library version on exit */
 );

/*---------------------------------------------------------------------------

    functionname:mp3encInitDefaultConfig
    description: initializes the MP3ENC_CONFIG to zero
    returns:     void

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encInitDefaultConfig(struct MP3ENC_CONFIG* pConfig);

/*---------------------------------------------------------------------------

    functionname:mp3encOpen
    description: allocate and initialize a new encoder instance
    returns:     MP3ENC_OK if success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encOpen
    (
    struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
    const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
    );

/*---------------------------------------------------------------------------

    functionname:mp3encClose
    description: deallocate an encoder instance

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encClose
    (
    struct MP3_ENCODER*        hMp3Enc  /* an encoder handle */
    );

/*---------------------------------------------------------------------------

    functionname: mp3encFlush
    returns:      calls BSFlush

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encFlush
    (
    struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
    unsigned char *bitStreamOutData,
    int *outBytes
    );


/*---------------------------------------------------------------------------

    functionname: mp3encGetDualPassState
    returns:      state of the first dualPass coding loop

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encGetDualPassState
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
     );

/*---------------------------------------------------------------------------

    functionname:mp3encGetOflOffset
    description: 
    returns:

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetOflOffset(const struct MP3_ENCODER* hMp3Enc) ;

/*---------------------------------------------------------------------------

    functionname:mp3encGetGranCnt
    description: 
    returns:

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetGranCnt(const struct MP3_ENCODER* hMp3Enc) ;

/*---------------------------------------------------------------------------

    functionname:mp3encEncode
    description: encode BLOCKSIZE*nChannels samples
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/
int  MP3ENCAPI mp3encEncode
    (
    struct MP3_ENCODER * const hMp3Enc,  /* an encoder handle */
    const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
    const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
    unsigned char* const       pOutput,  /* pointer to bitstream buffer; receives complete frames only */
    int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
    int* const                 cbOut,    /* number of bytes in bitstream buffer */   
    unsigned char * ancDataBytes,
    int* numAncDataBytes,
    unsigned int* writeOflOnce 
    );

/*---------------------------------------------------------------------------

    functionname:mp3encGetInfo
    description: get information about the encoding process
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encGetInfo
    (
    const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
    struct MP3ENC_INFO*       pInfo     /* receives information */
    );

/*---------------------------------------------------------------------------

    functionname:mp3encGetFrameStats
    description: 
    returns:

  ---------------------------------------------------------------------------*/

const int * MP3ENCAPI mp3encGetFrameStats
    (
    const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
    );

/*-------------------------------------------------------------------------*/

#ifdef WIN32
#pragma pack(pop)
#endif

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/
#endif /* _mp3enc_h_ */

