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
*   $Id: initSSE.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/
#include "cpuinfo.h"

extern void initSSE(void );


extern void initAdjThrSSE();
extern void initBndNrgSSE();
extern void initBlockSwitchSSE();
extern void initLinePeSSE();
extern void initQuantSSE();
extern void initQuantSSE2();
extern void initSfEstimSSE2();

void initSSE(void )
{
  if (GetCPUInfo(HAS_CPU_SSE) ) {
    initAdjThrSSE();
    initBandNrgSSE();
    initBlockSwitchSSE();
    initLinePeSSE();
    initQuantSSE();
  }

  if (GetCPUInfo(HAS_CPU_SSE2) ) {
    initQuantSSE2();
    initSfEstimSSE2();
  }
}
