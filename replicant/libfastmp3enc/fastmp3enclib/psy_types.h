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
*   $Id: psy_types.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PSY_TYPES_H
#define _PSY_TYPES_H

#include "psy_const.h"
#include "mp3alloc.h"



typedef union{
  float Long[MAX_PB_LONG];
  float Short[TRANS_FAC][MAX_PB_SHORT];
}PB_ENERGY;


typedef union{
  float Long[MAX_PB_LONG];
  float Short[TRANS_FAC][MAX_PB_SHORT];
}PB_SPREADED_ENERGY;


typedef union{
  float Long[MAX_PB_LONG];
  float Short[TRANS_FAC][MAX_PB_SHORT];
}PB_THRESHOLD;

typedef union{
  float Long[MAX_SFB_LONG];
  float Short[TRANS_FAC][MAX_SFB_SHORT];
}SFB_THRESHOLD;

typedef union{
  float Long[MAX_SFB_LONG];
  float Short[TRANS_FAC][MAX_SFB_SHORT];
}SFB_ENERGY;


typedef union{
  int Long[MAX_SFB_LONG];
  int Short[TRANS_FAC][MAX_SFB_SHORT];
}IS_POSITION;

typedef union{
  float Long[MAX_SFB_LONG];
  float Short[TRANS_FAC][MAX_SFB_SHORT];
}IS_DIR_COMPONENT;

typedef union{
  ALIGN_16_BYTE float Long[FRAME_LEN_LONG];
  ALIGN_16_BYTE float Short[TRANS_FAC][FRAME_LEN_SHORT];
}MDCT_SPECTRUM;

#endif /* PSY_TYPES_H */
