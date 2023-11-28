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
*   $Id: lean_psy.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _LEAN_PSY_H
#define _LEAN_PSY_H

#include "mconfig.h"

#ifdef LEAN_PSYCH

#include "psy_configuration.h"

int advancePsychLongLean(int ch, /* channel */
                         struct PSY_DATA psyData[MAX_CHANNELS],
                         const PSY_CONFIGURATION psyConf[2],
                         float *lineEnergy,
                         float *pScratchLineChaosMeasure);
int advancePsychShortLean(int ch, /* channel */
                          struct PSY_DATA psyData[MAX_CHANNELS],
                          const PSY_CONFIGURATION psyConf[2],
                          float *lineEnergy,
                          float *pScratchLineChaosMeasure);

#endif
#endif
