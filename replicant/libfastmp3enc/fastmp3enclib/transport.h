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
*   $Id: transport.h,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                  *
*                                                                                              *
************************************************************************************************/

#ifndef _TRANSPORT_H
#define _TRANSPORT_H

#include "psy_const.h"

typedef struct{
  int	sampfreq;
  int	bitrate;
  int	profile;
  int	mode;
}STAT_CONF_DATA;

typedef struct{
  int	modeExtension;
  int	commonIteration;
  int	protection;
  int	copyright;
  int	original;
}DYNA_CONF_DATA;

typedef struct{
  int	blockType;
  int	windowShape;
  int	noOfGroups;
  int	sumPe;
  int	psychGain[TRANS_FAC];
  int	groupLength[TRANS_FAC];
  int	sfbPe[MAX_GROUPED_SFB];
  float sfbEnergy[MAX_GROUPED_SFB];
  float sfbThreshold[MAX_GROUPED_SFB];
}CHANNEL_SI;

typedef struct{
  STAT_CONF_DATA statConfData;
  DYNA_CONF_DATA dynaConfData;
  CHANNEL_SI	 channelSi[2];
  float			 psyOutSpec[2][FRAME_LEN_LONG];
} TRANSPORT;


int BuildTransport(/* to be defined ... */);

#endif /* _TRANSPORT_H */
