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
*   $Id: adj_thr.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __ADJ_THR_H
#define __ADJ_THR_H

#include "psy_const.h"
#include "interface.h"
#include "mp3alloc.h"


#ifdef OLD_THR_REDUCTION

void AdjustThresholds(struct PSY_OUT *psyOut, 
                      float *chBitDistribution,
                      const int nChannels,
                      const int avgBits,
                      const int bitresBits,
                      const int maxBitresBits);

#else /* #ifdef OLD_THR_REDUCTION */


typedef struct {
  float clipSaveLow, clipSaveHigh;
  float minBitSave, maxBitSave;
  float clipSpendLow, clipSpendHigh;
  float minBitSpend, maxBitSpend;
} BRES_PARAM;

typedef struct t_ah_param {
   unsigned char modifyMinSnr;
  int startSfbL;
  int startSfbS;
} AH_PARAM;

struct ADJ_THR_STATE {
   /* pe of each sfb */
ALIGN_16_BYTE  float sfbPe[MAX_CHANNELS][MAX_GROUPED_SFB];
   /* number of lines with lineEnergy > sfbThreshold/sfbWidth */
ALIGN_16_BYTE  int   sfbNActiveLines[MAX_CHANNELS][MAX_GROUPED_SFB]; 
   /* flag indicating in which sfb avoid hole was necessary */
ALIGN_16_BYTE  int   sfbAvoidHole[MAX_CHANNELS][MAX_GROUPED_SFB];
   /* parameters for bitreservoir control */
   float peMin, peMax;
   BRES_PARAM bresParamLong, bresParamShort;
   /* values for correction of pe */
   float peLast;
   int dynBitsLast;
   float peCorrectionFactor;
   /* minSnr */
ALIGN_16_BYTE  float sfbMinSnrLong[MAX_SFB_LONG];
ALIGN_16_BYTE  float sfbMinSnrShort[MAX_GROUPED_SFB];
#ifndef NO_NEW_AVOID_HOLE_STRATEGY
  ALIGN_16_BYTE float sfbMinSnr[MAX_CHANNELS][MAX_GROUPED_SFB];
  AH_PARAM ahParam;
#endif

  int granPerFrame;
  /* vbr values */
  float desiredMeanPe;     /* constant over livetime */
  int desiredTotalBits;
  int desiredBitresLevel;
  int desiredFrameBits;
  int desiredMeanBitrate;
  int usedTotalBits;       /* total number of bits used */

  /* dualPass coding */
  int totalGranules;
  struct PE_STATS *peStats;

  /*float noRedPeSum;*/
  float peSum;

};

int  AdjThrNew(struct ADJ_THR_STATE **phAdjThr);

void AdjThrDelete(struct ADJ_THR_STATE *hAdjThr);

void AdjThrInit(struct ADJ_THR_STATE *hAdjThr, 
                const float peMean, 
                const int channelBitrate
#ifndef NO_NEW_AVOID_HOLE_STRATEGY
                ,const float bitsPerSample
#endif
                );


void AdjustThresholds(struct ADJ_THR_STATE *hAdjThr,
                      struct PSY_OUT_LIST *qcPsyOut,
                      float *chBitDistribution,
                      const int nChannels,
                      const int avgBits,
                      const int bitresBits,
                      const int maxBitresBits,
                      const float maxBitFac,
                      const int startCh,
                      const int endCh);


void AdjustThresholdsVBR(struct ADJ_THR_STATE *hAdjThr,
                         /*struct PSY_OUT_LIST *firstPsyOut,*/
                         struct PSY_OUT_LIST *qcPsyOut,
                         struct PSY_OUT_LIST *lastPsyOut,
                         float *chBitDistribution,
                         const int nChannels,
                         const int maxBits,
                         const float vbrFactor,
                         const int startCh,
                         const int endCh,
                         const int granuleCnt,
			 const int predictGranules,
			 struct PE_STATS *peStats );

void AdjustThrDualPass( struct ADJ_THR_STATE *hAdjThr,
                        struct PSY_OUT_LIST *qcPsyOut,
                        float *chBitDistribution,
                        const int nChannels,
                        const int maxBits,
                        /*const float vbrFactor,*/
                        const int startCh,
                        const int endCh,
                        const int granuleCnt,
                        struct PE_STATS *peStats);

void AdjThrUpdate(struct ADJ_THR_STATE *hAdjThr,
                  const int dynBitsUsed);

float bits2pe(const float bits);

extern void (*calcThreshExp)(float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
			     struct PSY_OUT *psyOut,
			     /*const int nChannels,*/
			     const int startCh,
			     const int endCh);

#endif /* #ifdef OLD_THR_REDUCTION */

#endif
