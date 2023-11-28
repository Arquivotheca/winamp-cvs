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
*   $Id: channelelement.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _CHANNEL_ELEMENT_H_
#define _CHANNEL_ELEMENT_H_


typedef struct {
  int channelBitrate;
  int bitRes;
  int maxBits;
  int averageBits;
  int averageBitsPerGranule;
  int deltaBitres ;
  int nChannels;
  float maxBitFac;
} CHANNEL_CONFIGURATION;


typedef struct {

  int noOfChannelConfigurations ;
  int channel[2];

} CHANNEL_ELEMENT ;

int  CCNew(CHANNEL_CONFIGURATION ** channelConf);
int  CENew( CHANNEL_ELEMENT  ** ce );
void InitCE ( CHANNEL_ELEMENT  * ce, int no );
void CCDelete ( CHANNEL_CONFIGURATION * channelConf[2]);
void CEDelete (  CHANNEL_ELEMENT  * ce );


#endif
