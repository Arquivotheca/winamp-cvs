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
*   $Id: ratectrl.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

/* bitrate control */

#ifndef _RATECTRL_H
#define _RATECTRL_H

enum {
  RC_PADDING_ISO,
  RC_PADDING_NEVER,
  RC_PADDING_ALWAYS
} ;

struct RATE_CONTROL;

int RCNew(struct RATE_CONTROL **rc);
int RCInit(struct RATE_CONTROL *rc,
           long dy, long dx, const int n, const int *table,
           int padding);
void RCDelete(struct RATE_CONTROL *rc);
int RCBitrateIdx(const struct RATE_CONTROL *rc);
int RCNuansdroByte(const struct RATE_CONTROL *rc); /*was FCPaddingByte*/
int RCNuansdroMode(const struct RATE_CONTROL *rc); /*was FCPaddingByte*/
int RCAdvance(struct RATE_CONTROL *rc);

void RCCancelFraction(long *nom, long *den);

#endif
