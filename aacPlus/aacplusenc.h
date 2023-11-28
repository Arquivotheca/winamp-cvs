/****************************************************************************

 This program is protected under international copyright laws as an
 unpublished work. Do not copy.

               (C) Copyright Coding Technologies (2003 - 2007)
                 (C) Copyright Dolby Sweden AB (2008 - 2009)
                             All Rights Reserved

 This software is company confidential information and the property of
 Dolby Sweden AB, and can not be reproduced or disclosed in any form
 without written authorization of Dolby Sweden AB.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. Dolby Sweden AB has no
 liability for use of this software module or derivatives thereof in any
 implementation. Copyright is not released for any means. Dolby Sweden AB
 retains full right to use the code for its own purpose, assign or sell the
 code to a third party and to inhibit any user or third party from using the
 code. This copyright notice must be included in all copies or derivative
 works.

 $Id: aacplusenc.h,v 1.2 2009/10/12 13:27:37 audiodsp Exp $

*******************************************************************************/

/*! \addtogroup AACPLUS */
/*@{*/ 
/*!
  \file
  \brief  aacPlus encoder library interfaces $Revision: 1.2 $
  \author Stefan Gewinner, Andreas Ehret
*/

#ifndef __AACPLUSENC_H__
#define __AACPLUSENC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined _WIN32 && !defined __ICL)
  /*
   * tell linker to ignore some default libraries. If this does
   * not work for your linker, set it manually
   */
#pragma comment(linker, "-nodefaultlib:libmmt.lib")
#pragma message(__FILE__": telling linker to ignore libmmt.lib")

#pragma comment(linker, "-nodefaultlib:libirc.lib")
#pragma message(__FILE__": telling linker to ignore libirc.lib")

#pragma comment(linker, "-nodefaultlib:svml_disp.lib")
#pragma message(__FILE__": telling linker to ignore svml_disp.lib")
#endif

#ifdef _WIN32
  #pragma pack(push, 8)
  #ifndef AACPLUSENCAPI
    #define AACPLUSENCAPI __stdcall
  #endif
#else
  #ifdef macintosh
    #if defined(__MRC__) || defined(__MWERKS__)
      #pragma options align=mac68k
    #endif
  #endif

  #ifndef AACPLUSENCAPI
    #define AACPLUSENCAPI
  #endif
#endif


#define UNDEFINED 0



#undef PARAMETRICSTEREO
#define PARAMETRICSTEREO


typedef void *aacPlusEncHandle ;


/* 
  d a t a  t y p e s  
*/


typedef enum aacPlusEncIfStatusCode
{
  APEI_OK                            = 0
  ,APEI_FINISHED                     = 0x01

  ,APEI_GENERAL_ERROR                = 0x80
  ,APEI_INVALID_HANDLE               = 0x81
  ,APEI_INVALID_PARAMETER            = 0x82
  ,APEI_OUT_OF_MEMORY                = 0x83
  ,APEI_INPUT_WRONG_SAMPLERATE       = 0x84
  ,APEI_INPUT_WRONG_NCHANNELS        = 0x85
  ,APEI_UNSUPPORTED_CONFIGURATION    = 0x86
  ,APEI_UNSUPPORTED_CHANNELMODE      = 0x87
  ,APEI_UNSUPPORTED_BITRATE          = 0x88
  ,APEI_UNSUPPORTED_BITSTREAMFORMAT  = 0x89
  ,APEI_UNSUPPORTED_STREAMCONFIG     = 0x8A
  ,APEI_UNSUPPORTED_BANDWIDTH        = 0x8B
  ,APEI_MP2_MP4_MISMATCH             = 0x8C
  ,APEI_V2_NOT_ENABLED               = 0x8D
  ,APEI_BITSTREAMBUFFER_TOO_SMALL    = 0x8E
  ,APEI_CONFIGSTREAM_MISMATCH        = 0x8F

  ,APEI_ENCODING_FAILED              = 0xA0
  ,APEI_AAC_ENCODING_FAILED          = 0xA1
  ,APEI_SBR_ENCODING_FAILED          = 0xA2
  ,APEI_ODD_SAMPLES_INPUT            = 0xA3
  
} aacPlusEncIfStatusCode;



typedef enum aacPlusEncInputFormat
{
  aacPlusEncInputShort = 0,
  aacPlusEncInputFloat,
  aacPlusEncInput24Packed
} 
aacPlusEncInputFormat ;



/* stream type data types */

typedef enum bitstreamFormat
{
  BSFORMAT_ADTS = 0,
  BSFORMAT_ADIF,
  BSFORMAT_LATM,
  BSFORMAT_LOAS,
  BSFORMAT_RAW,
  BSFORMAT_ADTS_MP4
} 
bitstreamFormat ;


typedef enum sbrSignallingMode
{
  IMPLICIT = 0,
  EXPLICIT_BC,
  EXPLICIT_NON_BC
  ,EXPLICIT_BC_PS
  ,EXPLICIT_NON_BC_PS
}
sbrSignallingMode ;


typedef struct
{
  unsigned int bOriginalCopyBit ;
  unsigned int bHomeBit ;
}
mp2AdifConfig ;


typedef struct
{
  unsigned int bWriteAdtsCRCheck ;
  unsigned int bOriginalCopyBit ;
  unsigned int bHomeBit ;
  unsigned int numSubFrames ;
  unsigned int bMp2AdtsPce ;
  float        sendAdtsPceInterval ;
  float        sendAdtsPceOffset ;
}
mp2mp4AdtsConfig ;


typedef struct
{
  unsigned int bSendMuxConfigInBand ;
  unsigned int bWriteCRCheck ;
  unsigned int numSubFrames ;
  float        sendConfigDataInterval ;
  float        sendConfigDataOffset ;
  unsigned int audioMuxVersion;
}
mp4LatmLoasConfig ;


typedef struct
{
  sbrSignallingMode signallingMode;
}
ascConfig ;


typedef struct {
  mp2AdifConfig      adifConfig ;
  mp2mp4AdtsConfig   adtsConfig ;
  mp4LatmLoasConfig  latmLoasConfig ;    
  ascConfig          asConfig ;
}
bitstreamConfig ;


typedef struct aacPlusEncStreamType
{
  bitstreamFormat          bsFormat ;
  bitstreamConfig          bsConfig ;
  unsigned int             bWriteSbrCrc;
  float                    sendSbrHeaderInterval;
  float                    sendSbrHeaderOffset;
}
aacPlusEncStreamType,  *aacPlusEncStreamTypePtr;


typedef enum aacPlusEncStreamMuxConfigType
{
  TYPE_STREAM_MUX_CONFIG = 0,
  TYPE_AUDIO_SPECIFIC_CONFIG
}
aacPlusEncStreamMuxConfigType;



/* output format data types */

typedef enum aacPlusEncChannelMode
{
  UNDEFINED_CHANNEL_MODE = UNDEFINED,
  MONO,
  STEREO,
  STEREO_INDEPENDENT, 
  PARAMETRIC_STEREO,
  DUAL_CHANNEL,
  MODE_4_CHANNEL_2CPE,
  MODE_4_CHANNEL_MPEG,
  MODE_5_CHANNEL,
  MODE_5_1_CHANNEL,
  MODE_6_1_CHANNEL,
  MODE_7_1_CHANNEL
} 
aacPlusEncChannelMode ;

typedef enum eAacPlusEncSbrMode
{
  UNDEFINED_SBR_MODE = UNDEFINED,
  SBR_OFF,
  SBR_NORMAL,
  SBR_OVERSAMPLED
} 
aacPlusEncSbrMode ;


typedef enum aacPlusEncSignalType
{
  DEFAULT_SIGNAL_TYPE = UNDEFINED,
  SPEECH
} 
aacPlusEncSignalType ;


typedef enum aacPlusEncListsSortMode
{
  APELIST_SORT_BOTTOM_UP = 0,
  APELIST_SORT_TOP_DOWN,
  APELIST_BEST_MATCHING_FIRST
} 
aacPlusEncListsSortMode ;

typedef struct aacPlusEncOutputFormat
{
  unsigned int          sampleRate ;
  aacPlusEncChannelMode channelMode ;
  aacPlusEncSbrMode     sbrMode ;
  unsigned int          bitRate ;
  aacPlusEncSignalType  signalType ;
} 
aacPlusEncOutputFormat, *aacPlusEncOutputFormatPtr ;


typedef struct aacPlusEncOutputFormatList
{
  int numEntries ;
  int preferredFormatIndex ;
  aacPlusEncOutputFormat format [1] ;
} 
aacPlusEncFormatList, *aacPlusEncFormatListPtr ;


/* configuration data types */

typedef enum aacPlusEncQuality
{
  aacPlusEncQualityFast = 0,
  aacPlusEncQualityMedium,
  aacPlusEncQualityHighest
} 
aacPlusEncQualityMode ;


typedef struct aacPlusEncConfiguration
{
  /* encoding quality related configuration parameters */

  aacPlusEncQualityMode qualityMode ;
  unsigned int          allowNarrowing ;
  unsigned int          allowTns;
  unsigned int          allowPns;
  float                 requestedBandwidth ;
  
  /* read-only information */

  unsigned int codecDelay ;
  float        effectiveBandwidth ;
}

aacPlusEncConfiguration, *aacPlusEncConfigurationPtr ;


/* 
  f u n c t i o n  p r o t o t y p e s
*/


/* main top level functions */

aacPlusEncHandle AACPLUSENCAPI 
aacPlusEncOpen (unsigned int sampleRate,
                unsigned int numChannels,
                aacPlusEncInputFormat inputFormat,
                int bAllowV2Features,
                int bAllowOversampledSBR
                ) ;


aacPlusEncIfStatusCode AACPLUSENCAPI
aacPlusEncClose (aacPlusEncHandle hEncoder) ;

aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncEncode (const aacPlusEncHandle hEncoder,
                  const void *inputBuffer,
                  unsigned int bytesInput,
                  unsigned int *bytesConsumed,
                  void *outputBuffer,
                  unsigned int *bytesInBuffer,
                  const unsigned char *ancBytes,
                  unsigned int *numAncBytes,
                  unsigned int transportOverheadBits
                  ) ;


/* additional helper functions */

aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncSetStreamType(const aacPlusEncHandle  hEncoder,
                        const aacPlusEncStreamTypePtr streamType) ;


aacPlusEncStreamTypePtr AACPLUSENCAPI
aacPlusEncGetCurrentStreamType (const aacPlusEncHandle hEncoder) ;


aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncGetMPEG4Config (const aacPlusEncHandle  hEncoder,
                          unsigned char* pConfigBuffer,
                          unsigned int   nBytesBufferSize,
                          unsigned int*  pnConfigBitsOut,
                          aacPlusEncStreamMuxConfigType cfgType
                          );


aacPlusEncFormatListPtr AACPLUSENCAPI 
aacPlusEncGetFormatList (const aacPlusEncHandle hEncoder,
                         aacPlusEncOutputFormat outputFormat, 
                         unsigned int allowDownmix,
                         int bAllPossibleFormats,
                         aacPlusEncListsSortMode sortMode) ;


aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncSetOutputFormat (const aacPlusEncHandle hEncoder,
                           const aacPlusEncOutputFormatPtr pOutputFormat) ;


aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncGetOutputFormat (const aacPlusEncHandle hEncoder, 
                           const aacPlusEncOutputFormatPtr pOutputFormat);


aacPlusEncConfigurationPtr AACPLUSENCAPI 
aacPlusEncGetCurrentConfiguration (const aacPlusEncHandle hEncoder) ;


aacPlusEncIfStatusCode AACPLUSENCAPI
aacPlusEncSetConfiguration (const aacPlusEncHandle hEncoder, 
                            const aacPlusEncConfigurationPtr pConfig) ;


char* AACPLUSENCAPI 
aacPlusEncGetLibraryVersion( void);


aacPlusEncIfStatusCode AACPLUSENCAPI 
aacPlusEncGetOptimumBufferFeed (const aacPlusEncHandle hEncoder,
                                int* pOptimumBufferFeedBytes);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #ifdef macintosh
    #if defined(__MRC__) || defined(__MWERKS__)
      #pragma options align=reset
    #endif
  #endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AACPLUSENC_H__ */
/*@}*/ 
