/****************************************************************************

 This program is protected under international copyright laws as an
 unpublished work. Do not copy.

               (C) Copyright Coding Technologies (2004 - 2007)
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

 $Id: aacplusdec.h,v 1.2 2009/10/12 13:27:37 audiodsp Exp $

*******************************************************************************/

/*!
  \file
  \brief  aacPlus Decoder Library Interface Functions
  \author Holger Hoerich
*/


#ifndef _AACPLUSDEC_H_
#define _AACPLUSDEC_H_

#include "aacplusdectypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) && defined(__AACPLUSDEC_DLL__)
  #ifdef __AACSBRDEC_ENTRYPOINT_CPP__
    #pragma message("### Building MPEG-2 aacPlus Decoder DLL ###")
    #define LINKSPEC _declspec(dllexport)
  #else
    #define LINKSPEC _declspec(dllimport)
  #endif
#else
  #define LINKSPEC
#endif

#if defined(WIN32)
  #define AACPLUS_API __stdcall
#else
  #define AACPLUS_API
#endif

#if (defined WIN32 && !defined __ICL)
  #pragma comment(linker, "-nodefaultlib:libmmt.lib")
#endif



/*** 
     OPEN / CLOSE                  *****/


/*!
  \brief     Creates And Initializes A Decoder Instance

  This function creates a decoder instance with preconfigured
  capabilities and ready to go. Use GetSettings() and SetSettings()
  in order to customize the decoder settings.
*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecEasyOpen ( HANDLE_AACPLUSDEC_DECODER *phDec,
                     AACPLUSDEC_OUTPUTFORMAT nOutputFormat,
                     int nMaxChannels );

/*!
  \brief     Creates And Initializes A Decoder Instance

  This is the one step open and configure function. An external provided
  settings structure must be filled completely. If the configuration with
  the desired settings fails, no valid instance can be created.
*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecExpertOpen( HANDLE_AACPLUSDEC_DECODER *phDec,
                      AACPLUSDEC_OUTPUTFORMAT  outputFormat, /*!< set audio output format */
                      AACPLUSDEC_EXPERTSETTINGS  settings    /*!< fill out completely */
                      );

/*!
  \brief     Destroys A Decoder Instance
*/
LINKSPEC void AACPLUS_API
aacPlusDecClose( HANDLE_AACPLUSDEC_DECODER *phDec);


/*!
  \brief     Restarts The Decoder

  Needs to be called after rewinding or skipping parts of the bitstream
  in order to clean buffers and conditions.
*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecRestart( HANDLE_AACPLUSDEC_DECODER hDec);




/*** 
     CONFIGURE                     *****/


/*!
  \brief     Get Pointer to Internal Stream Description
 */
LINKSPEC AACPLUSDEC_STREAMPROPERTIES * AACPLUS_API
aacPlusDecGetStreamPropertiesHandle( HANDLE_AACPLUSDEC_DECODER hDec);


/*!
  \brief     Set Parameter Needed For Decoding

  Some bitstream formats can be detected by the decoder e.g. ADIF, ADTS
  and LOAS. Other in contrast require the decoder need to know in advance.
  Those are e.g. LATM and RAW. For RAW it's also necessary to set the
  sample rate when not already configure by an Audio Specific Config
  or Stream Mux Config.
 */
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecPreconfigureStream( HANDLE_AACPLUSDEC_DECODER hDec,
                              AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn,  /*!< expected bitstream format, give 0 if unknown  */
                              int nAacSampleRateIn   /*!< expected sampling frequency, give 0 if unknown */
                              );


/*!
  \brief     Reads and evaluates the Audio Specific Config or the Stream Mux Config

  The stream properties are instantly updated with the already provided values. This feature
  could also be used to parse and evaluate a given config stream before decoding.

  The parameter bConfigStreamInBand should usually be set to zero! Only if you have an
  Audio Specific Config in-band with an e.g. LATM transmission but also available
  externally and you want to parse it before decoding, then set this parameter to one.
  Otherwise a call of this function prior decoding would configure the bitstream reader
  to not expect a config stream in-band.
 */
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecReadConfigStream( HANDLE_AACPLUSDEC_DECODER hDec,
                            unsigned char *pucConfigStreamBufferIn,    /*!< the config stream buffer */
                            AACPLUSDEC_BITSTREAMBUFFERINFO *hConfigStreamBufferInfoInOut,  /*!< buffer descriptor */
                            AACPLUSDEC_CONFIGTYPE nConfigTypeIn,       /*!< which config stream will be provided */
                            int    bConfigStreamInBand,                /*!< set only to one if a config stream is embedded in the bitstream also */
                            AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn  /*!< expected bitstream format, give 0 if unknown  */
                            );


/*!
  \brief     Return Pointer To Decoder Settings

  The decoder can be customized when it's opened. Especially when using the
  EasyOpen() function the settings are conveniently set to their default
  values and only those which want to be changed need to be touched. A call
  to SetDecoderSettings() is required before any changes take effect.
*/
LINKSPEC AACPLUSDEC_EXPERTSETTINGS * AACPLUS_API
aacPlusDecGetDecoderSettingsHandle( HANDLE_AACPLUSDEC_DECODER hDec);


/*!
  \brief     Apply Decoder Settings

  Changes in the decoder settings will only become effective when this
  function gets called. Then decoder will then be reconfigured according
  to the settings.

  Do not change settings after starting decoding! The result is
  unpredictable!
*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusDecSetDecoderSettings( HANDLE_AACPLUSDEC_DECODER hDec);




/*** 
     CONVENIENCE                   *****/


/*!
  \brief     Return Library Version Information
*/
LINKSPEC const char * AACPLUS_API 
aacPlusDecGetVersionInfo( void);


/*!
  \brief     Return An Error Text Based On Given Error Code
*/
LINKSPEC const char * AACPLUS_API
aacPlusDecGetErrorText( AACPLUSDEC_ERROR statusIn);




/*** 
     STREAM BASED INTERFACE        *****/

/*!
  \brief    Feed The Internal Bitstream Buffer
*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusStreamFeed( HANDLE_AACPLUSDEC_DECODER hDec,
                   unsigned char *pucBitstrmBufIn,
                   AACPLUSDEC_BITSTREAMBUFFERINFO *hBitstrmBufInfoInOut
                   );


/*!
  \brief    Decodes One Frame From The Internal Bitstream Buffer

  The bitstream buffer will be parsed for a decodable frame. This
  frame will be decoded. It will be signaled if the frame is not
  complete. Successfully read bitstream data or portions without 
  finding a synch word in the bitstream buffer is invalidated.

  The size of the output buffer limits the ability of the decoder of
  trying to decode as many elements as possible. If the decoded frame would
  not fit into the given PCM buffer, the decoder would return with an error
  and the data in the PCM buffer would not be valid.

  A frame will be decoded until the end in order to set correct
  stream description even though it would not fir into the audio buffer.

*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusStreamDecode( HANDLE_AACPLUSDEC_DECODER hDec,
                     void          *pPcmAudioBufOut,                          /*!< provide audio output buffer */
                     AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,       /*!< provide audio output buffer info */
                     unsigned char *pucDataStreamBufOut,                      /*!< provide data stream output buffer */
                     AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut /*!< provide data stream output buffer info */
                     );




/*** 
     FRAME BASED INTERFACE         *****/

/*!
  \brief    Decodes One Frame Directly Provided

  A complete frame is assumed in the frame buffer. No synchronization
  will be performed. Additional data in the frame buffer after the 
  decoding has completed will be discarded. If the frame is not complete
  it will be handled as a distorted frame. The internal bitstream buffer
  is not affected.

  When a corrupt frame is signaled, it will be handled according to the
  configured method (muting or concealment). The frame buffer will not
  be read in this case and might be NULL.

  The size of the output buffer limits the ability of the decoder of
  trying to decode as many elements as possible. If the decoded frame would
  not fit into the given PCM buffer, the decoder would return with an error
  and the data in the PCM buffer would not be valid.

*/
LINKSPEC AACPLUSDEC_ERROR AACPLUS_API
aacPlusFrameDecode( HANDLE_AACPLUSDEC_DECODER hDec,
                    void          *pPcmAudioBufOut,                           /*!< provide audio output buffer */
                    AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,        /*!< provide audio output buffer info */
                    unsigned char *pucFrameBufferIn,                          /*!< pointer to the beginning of a frame */
                    AACPLUSDEC_BITSTREAMBUFFERINFO *hFrameBufferInfoInOut,    /*!< bitstream buffer info */
                    int            bFrameCorrupt,                             /*!< signal the frame is corrupt */
                    unsigned char *pucDataStreamBufOut,                       /*!< provide data stream output buffer */
                    AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut  /*!< provide data stream output buffer info */
                    );



/*** 
     END   *****************************/


#ifdef __cplusplus
}
#endif

#endif /* _AACPLUSDEC_H_ */
