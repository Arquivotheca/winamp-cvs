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
*   $Id: block_switch.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __BLOCK_SWITCH_H__
#define __BLOCK_SWITCH_H__

struct BLOCK_SWITCHING_CONTROL;
typedef struct BLOCK_SWITCHING_CONTROL *BLOCK_SWITCHING_HANDLE ;

int   mp3BlockSwitchingInit(BLOCK_SWITCHING_HANDLE blockSwitchingControl);
int   mp3BlockSwitchingNew(BLOCK_SWITCHING_HANDLE *blockSwitchingControl);
void  mp3BlockSwitchingDelete(BLOCK_SWITCHING_HANDLE blockSwitchingControl);
extern int (*mp3BlockSwitching)(BLOCK_SWITCHING_HANDLE blockSwitchingControl,
				const float *timeSignal);

int   mp3SyncBlockSwitching(BLOCK_SWITCHING_HANDLE blockSwitchingControlLeft,
                            BLOCK_SWITCHING_HANDLE blockSwitchingControlRight, 
                            const int noOfChannels, 
                            const int commonWindow,
                            int      *windowSequenceLeft,
                            int      *windowSequenceRight);

#endif  /* #ifndef _BLOCK_SWITCH_H */
