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
*   $Id: channelelement.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "channelelement.h"
#include "mp3alloc.h"

int CCNew(CHANNEL_CONFIGURATION ** channelConf){

  CHANNEL_CONFIGURATION * hCC;

  hCC = (CHANNEL_CONFIGURATION *) mp3Alloc(sizeof(CHANNEL_CONFIGURATION));
  *channelConf = hCC;

  return ( hCC == 0 );

}

int CENew( CHANNEL_ELEMENT  ** ce ){

  CHANNEL_ELEMENT * hCE ;

  hCE = (CHANNEL_ELEMENT *) mp3Alloc (sizeof(CHANNEL_ELEMENT));

  *ce = hCE ;
  return (hCE == 0);

}

void InitCE ( CHANNEL_ELEMENT  * ce, int no ){

  ce->noOfChannelConfigurations = no ;
  if ( no == 2 ) {
    ce->channel[0] = 0;
    ce->channel[1] = 1;
  }
  else {
    ce->channel[0] = 0;
    ce->channel[1] = 0;
  }

}

void CCDelete ( CHANNEL_CONFIGURATION * channelConf[2]){

  int i ;

  for ( i = 0 ; i < 2; i++){
    if ( channelConf[i] )
      mp3Free(channelConf[i]);
  }

}

void CEDelete (  CHANNEL_ELEMENT  * ce ){

  if ( ce ){

    mp3Free(ce);

  }
}
