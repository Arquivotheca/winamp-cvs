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
*   $Id: pre_echo_control.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PREECHO_CONTROL_
#define _PREECHO_CONTROL_

int mp3InitPreEchoControl(/*float *pbThresholdnm1*/);

void mp3PreEchoControl(float *pbThreshold_nm1,
                       /*float *pbThreshold_nm2,*/
                       int   numPb,
                       float maxAllowedIncreaseFactor,
                       float minRemainingThresholdFactor,
                       float *pbThreshold);

#endif
