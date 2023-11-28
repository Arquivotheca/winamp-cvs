/****************************************************************************

 This program is protected under international copyright laws as an
 unpublished work. Do not copy.

                  (C) Copyright Coding Technologies (2007)
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

 $Id: streamdescription.h,v 1.2 2009/10/12 13:27:37 audiodsp Exp $

****************************************************************************/

/*!
  \file
  \brief  public interface to streaminfo $Revision: 1.2 $
  \author Andreas Schneider
*/

#ifndef __STREAMDESCRIPTION_H__
#define __STREAMDESCRIPTION_H__

#include "aacplusdec.h"

#define MAX_NCHANNELS 8

typedef enum {
  Profile_Main = 0,
  Profile_LowComplexity = 1,
  Profile_ScalableSamplingRate = 2,
  Profile_reserved = 3
} MPEG_2_PROFILE;


typedef enum {
  TYPE_UNSPECIFIED                = (int) AACPLUSDEC_CONFIGTYPE_UNSPECIFIED,
  TYPE_DEC_STREAM_MUX_CONFIG      = (int) AACPLUSDEC_CONFIGTYPE_STREAMMUXCONFIG,
  TYPE_DEC_AUDIO_SPECIFIC_CONFIG  = (int) AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG,
  TYPE_DECODERCONFIGDESCRIPTOR    = (int) AACPLUSDEC_CONFIGTYPE_DECODERCONFIGDESCRIPTOR,
  TYPE_DECODERSPECIFICINFO        = (int) AACPLUSDEC_CONFIGTYPE_DECODERSPECIFICINFO
} CONFIG_TYPE;

typedef enum {
  BITSTREAM_FORMAT_AUTO           = (int) AACPLUSDEC_BITSTREAMFORMAT_AUTO,
  BITSTREAM_FORMAT_RAW            = (int) AACPLUSDEC_BITSTREAMFORMAT_RAW,
  BITSTREAM_FORMAT_ADIF           = (int) AACPLUSDEC_BITSTREAMFORMAT_ADIF,
  BITSTREAM_FORMAT_ADTS           = (int) AACPLUSDEC_BITSTREAMFORMAT_ADTS,
  BITSTREAM_FORMAT_LATM           = (int) AACPLUSDEC_BITSTREAMFORMAT_LATM,
  BITSTREAM_FORMAT_LOAS           = (int) AACPLUSDEC_BITSTREAMFORMAT_LOAS
} BITSTREAM_FORMAT;

typedef enum
{
  CM_UNDEFINED                    = (int) AACPLUSDEC_CHANNELMODE_UNDEFINED,
  CM_MONO                         = (int) AACPLUSDEC_CHANNELMODE_MONO,
  CM_STEREO                       = (int) AACPLUSDEC_CHANNELMODE_STEREO,
  CM_PARAMETRIC_STEREO            = (int) AACPLUSDEC_CHANNELMODE_PARAMETRIC_STEREO,
  CM_DUAL_CHANNEL                 = (int) AACPLUSDEC_CHANNELMODE_DUAL_CHANNEL,
  CM_4_CHANNEL_2CPE               = (int) AACPLUSDEC_CHANNELMODE_4_CHANNEL_2CPE,
  CM_4_CHANNEL_MPEG               = (int) AACPLUSDEC_CHANNELMODE_4_CHANNEL_MPEG,
  CM_5_CHANNEL                    = (int) AACPLUSDEC_CHANNELMODE_5_CHANNEL,
  CM_5_1_CHANNEL                  = (int) AACPLUSDEC_CHANNELMODE_5_1_CHANNEL
} AACPLUS_DEC_CHANNEL_MODE ;

/** Stream description for the C interface.

    This struct is updated on a per frame basis or whenever the data becomes
    available to the decoder. Altering this data does not influence the
    behaviour of the decoder with the exceptions of m_BitstreamFormat and
    m_AacSamplingRate prior to the first call of aacplusDecodeFrame(). The calling
    application may require this data to perform certain initializations like
    the output device.
*/

typedef struct {

  unsigned int m_AacSamplingRate;
  MPEG_2_PROFILE m_Profile;
  unsigned int m_BitRate;
  BITSTREAM_FORMAT m_BitstreamFormat;
  
  unsigned int m_StreamSbrEnabled;

  unsigned int m_OutputSamplingRate;
  unsigned int m_OutputSamplesPerFrame;
  unsigned int m_OutputChannels;
  
  AACPLUS_DEC_CHANNEL_MODE m_ChannelMode;

  unsigned int m_NumberOfFrontChannels;
  unsigned int m_NumberOfSideChannels;
  unsigned int m_NumberOfBackChannels;
  unsigned int m_NumberOfLfeChannels;
  unsigned int m_LastDecodedFrameBad;
} STREAMDESCRIPTION, * PSTREAMDESCRIPTION;


#endif
