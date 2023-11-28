/*********************************************************************************************** 
*                                   MPEG Layer3-Audio Encoder                                  * 
*                                                                                              * 
*                                 © 1997-2005 by Fraunhofer IIS                                * 
*                                       All Rights Reserved                                    * 
*                                                                                              * 
*   This software and/or program is protected by copyright law and international treaties.     * 
*   Any reproduction or distribution of this software and/or program, or any portion of it,    * 
*   may result in severe civil and criminal penalties, and will be prosecuted to the           * 
*   maximum extent possible under law.                                                         * 
*                                                                                              * 
************************************************************************************************/
#ifndef _PE_H
#define _PE_H

#include "line_pe.h"

struct PSY_OUT;

typedef struct
{
  PE_CHANNEL_DATA peChannelData[MAX_CHANNELS];
  float pe;
  float noRedPe;
  float redPe;      /* store nonlinear reduction of pe calculated in psyMain */
  float constPart;
  int nActiveLines;
} PE_DATA;

#define PE_COUNT 20        /* number of linearized sections of the pe reduction formula */
#define PE_MAX   10000.0f  /* maximum pe */

/* this structure holds pe-statistics and quality data */
struct PE_STATS
{
  float pe[PE_COUNT];           /* granule-counter for pe from 0 to PE_MAX */
  float peShort[PE_COUNT];      /* same for short-blocks */
  float logPe[PE_COUNT];        /* values for linearized pe reduction formula */
  float logPeShort[PE_COUNT];   /* same for short-blocks */
  float totalPe;
  int count;                    /* total number of counted granules */
  float Q;                      /* actual quality factor */

  int TANH;                     /* signals to use the tanh, for mean bitrate vbr only */

  /* the following is only for CBR */
  float QMax;                   /* maximum allowed quality factor */
  float QMin;                   /* minimum allowed quality factor */
  float lastQMax;               /* last calculated value, used for faster convergence */
  float lastQMin;               /* last calculated value, used for faster convergence */
  int maxIndex;                 /* number of Granules in the future where QMax is reached */
  int minIndex;                 /* number of Granules in the future where QMin is reached */
  int QIndex;                   /* number of Granules in the future where new quality
                                   has to be calculated.
                                   note: QIndex holds -1, if QMax and QMin aren't reached,
                                         so only the newest granules has to be taken into
                                         account for calculation of the quality */
};

void preparePe(PE_DATA *peData,
                      struct PSY_OUT *psyOut, 
                      float sfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
               /*const int nChannels,*/
                      const int startCh,
                      const int endCh);

void calcPe(PE_DATA *peData,
            struct PSY_OUT *psyOut,
            /*const int nChannels,*/
            const int startCh,
            const int endCh);

void PeStatsInit( struct PE_STATS *peStats, float Q, int useLog );
void PeStatsAdd( struct PE_STATS *peStats, float pe, int windowSequence );
void PeStatsSub( struct PE_STATS *peStats, float pe, int windowSequence );

#endif /* PE_H */
