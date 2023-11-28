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
*   $Id: interface.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "stprepro.h"
#include "psy_types.h"
#include "pe.h"
#include "mp3alloc.h"

struct MP3ENC_CONFIG;

struct PSY_OUT_CHANNEL {
  int   windowSequence;
  int   nWindows; /* a convenience variable. 1 for long,start,stop; 3 else */
  int   sfbCnt;   /* number of sfbs required to cover entire spectrum */
  int   sfbActive;/* number of sfbs containing energy after lowpass operation */
  ALIGN_16_BYTE  int   sfbOffsets[MAX_GROUPED_SFB+1]; /* holds a sentinel */
  ALIGN_16_BYTE  float sfbEnergy[MAX_GROUPED_SFB];
  ALIGN_16_BYTE  float sfbEnergyMS[MAX_GROUPED_SFB]; /* for stereo preprocessing */
  ALIGN_16_BYTE  float sfbThreshold[MAX_GROUPED_SFB];
  ALIGN_16_BYTE  float sfbFormFactor[MAX_GROUPED_SFB];
  ALIGN_16_BYTE  float sfbMaxSpec[MAX_GROUPED_SFB];
  ALIGN_16_BYTE  float mdctSpectrum[FRAME_LEN_LONG];

  /* intensity limit is held for both channels because it makes scalefactor
     handling more straightforward further down the code. The intensity limit
     for the left channel will equal sfbActive always, so that we can loop
     from 0 to isLimit in each channel */

  ALIGN_16_BYTE  int isLimit[TRANS_FAC] ;
  float pe;   /* pe sum for sterei preprocessing */
};

struct PSY_OUT {
  /* information shared by both channels */
  int msUsed;  /* 0 = no MS; 1 = MS on */
  int isUsed;  /* Guaranteed to be equal in both granules (MPEG 1) */
  ALIGN_16_BYTE  int sfbIsPositions[MAX_GROUPED_SFB];

  /* information specific to each channel */
  ALIGN_16_BYTE  struct PSY_OUT_CHANNEL psyOutChannel[2];

  HANDLE_STEREO_PREPRO hStereoPrePro;
  int nSamples;    /* number of samples in time-buffer */
};

/* used for linked list in mbr and cbr mode */
struct PSY_OUT_LIST {
  struct PSY_OUT      *psyOut;
  struct PSY_OUT_LIST *next;
  struct PSY_OUT_LIST *prev;
  int                  psyOutValid;
  int                  windowSequence;
  PE_DATA              peData;
#ifdef IISMP3_USE_THREADS
  int                  numAncDataBytes;
  char                 ancDataBytes[256];
#endif
#if 0
  ALIGN_16_BYTE  struct PSY_OUT_CHANNEL psyOutChannel[2];
#endif
};

void BuildInterface(const MDCT_SPECTRUM    *mdctSpectrum,
                    const SFB_THRESHOLD    *sfbThreshold,
                    const SFB_ENERGY       *sfbEnergy,
                    const SFB_ENERGY       *sfbEnergyMS,
                    const int               windowSequence,
                    const int               sfbCnt,
                    const int               sfbActive,
                    const int              *sfbOffset,
                    struct PSY_OUT_CHANNEL *psyOutCh);

int CreatePsyList( struct PSY_OUT_LIST **firstPsyOut,
                   struct PSY_OUT_LIST **lastPsyOut,
                   struct PSY_OUT_LIST **qcPsyOut,
                   const struct MP3ENC_CONFIG* pConfig,
                   int predictGranules );

void DelPsyList( struct PSY_OUT_LIST *firstPsyOut );

void FeedPsyList( struct PSY_OUT_LIST **firstPsyOut,
                  struct PSY_OUT_LIST **lastPsyOut,
                  struct PSY_OUT_LIST **qcPsyOut );

#endif /* _INTERFACE_H */
