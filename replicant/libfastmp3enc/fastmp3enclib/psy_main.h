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
*   $Id: psy_main.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PSYMAIN_H
#define _PSYMAIN_H

struct PSY_INTERNAL;
struct PSY_OUT;
struct PSY_OUT_LIST;

struct PSY_INIT
{
  long  sampleRate;
  long  bitRate;
  long  meanBitrate;
  int   nChannels;
  float bandWidth;
  int   noIntensity;
  int   useMS;
  int   useDualPass;
  int   pcmResolution;
  /* int   useLookAhead; */
};

struct MP3ENC_CONFIG;

int  PsyOutNew(struct PSY_OUT **phPsyOut);
void PsyOutDelete(struct PSY_OUT *hPsyOut);
int  PsyNew(struct PSY_INTERNAL **phpsy,
            const struct PSY_INIT     *pConfig);
int  PsyInit(struct PSY_INTERNAL *hpsy,
             const struct PSY_INIT     *pConfig);
void PsyDelete(struct PSY_INTERNAL *hpsy);

int psyFeedInputBufferFloat(struct PSY_INTERNAL *hPsy,
                            float input[][2], int nSamples);
float *psyGetInputBufferFloat(struct PSY_INTERNAL *hPsy);

int psyMain(struct PSY_INTERNAL *hpsy,
            const int            granule,
            struct PSY_OUT_LIST *firstPsyOut,
            const int            dualMono);

int GetBlockSwitchingOffset(/*struct PSY_INTERNAL *hpsy*/);

#endif /* _PSYMAIN_H */
