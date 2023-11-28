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
*   $Id: adj_thr.c,v 1.1 2007/05/29 16:02:26 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <math.h>
#include <limits.h>
#include <float.h>
#include <assert.h>

#include "mathlib.h"
#include "mp3alloc.h"
#include "interface.h"
#include "adj_thr.h"
#include "line_pe.h"
#include "utillib.h"
#include "mathmac.h"

static const float   redExp = 0.25f; 
static const float   invRedExp = 1.0f/0.25f;
static const float   minSNR = 0.63f; /* 2 dB */

#ifdef OLD_THR_REDUCTION

static const float   avgFillLevel = 0.9f;
static const float   shortRedFactor = 0.57735027f; /* 1/pow(TRANS_FAC,2*0.25) */

static int pe2bits(const int pe) 
{
   return (int)(2.0e4f * log(1.0f + pe/2.0e4f));
}

static int bits2pe(const int bits)
{
   return (int)(2.0e4f * (exp(bits/2.0e4f) - 1.0f));
}


/* empty bitreservoir -> big reduction value, 
   full bitres -> small reduction value */
static float 
redValFromBres(const float fillLevel, const float avgLoudness)
{
   float rv;

   if (fillLevel > avgFillLevel)
      rv = 0.0f;
   else
      rv = 1.5f * avgLoudness / (avgFillLevel * avgFillLevel) * 
           (avgFillLevel-fillLevel) * (avgFillLevel-fillLevel);

   return rv;
}



/* estimate reduction value that reduces pe by peFactor */
static float 
redValFromPe(const struct PSY_OUT *psyOut,
             const int nChannels,
             const float peFactor)
{
   float en, thr, thr2;
   float rv;
   int i, j;
   int sfbWidth, noOfActiveLines;
   float avgEn, avgThr;

   noOfActiveLines = 0;
   avgEn  = 0.0f;
   avgThr = 0.0f;
   for(j=0; j<nChannels; j++) {
      for (i=0; i<psyOut->psyOutChannel[j].sfbActive; i++) {
         sfbWidth = psyOut->psyOutChannel[j].sfbOffsets[i+1] -
                    psyOut->psyOutChannel[j].sfbOffsets[i];
         en  = psyOut->psyOutChannel[j].sfbEnergy[i];
         thr = psyOut->psyOutChannel[j].sfbThreshold[i];
         if (en > thr) {
            avgEn  += sfbWidth * en;
            avgThr += sfbWidth * thr;
            noOfActiveLines += sfbWidth;
         }
      }
   }
   avgEn  /= noOfActiveLines;
   avgThr /= noOfActiveLines;
   thr2 = avgEn * (float)pow(avgThr/avgEn, peFactor);
   rv = (float)pow(thr2, redExp) - (float)pow(avgThr, redExp);

   return rv;
}


/* average loudness of granule */
static float 
avgLoudness(const struct PSY_OUT *psyOut, const int nChannels)
{
   float avgLoud;
   int i, j;

   avgLoud = 0.0f;
   for(j=0; j<nChannels; j++) {
      for (i=0; i<psyOut->psyOutChannel[j].sfbActive; i++) {
         avgLoud += psyOut->psyOutChannel[j].sfbEnergy[i];
      }
   }
   avgLoud /= (nChannels*psyOut->psyOutChannel[0].sfbActive);
   avgLoud = (float)pow(avgLoud, redExp);

   return avgLoud;
}

static float
calcThresholdReduction(const struct PSY_OUT *psyOut, 
                       const int nChannels,
                       const int avgBits, 
                       const int bitresBits,
                       const int maxBitresBits)
{
   int    j;
   int    totalPe, estimBits;
   float  reductionValue, redValPe;
   float  curFillLevel;
   const struct PSY_OUT_CHANNEL *psyOutChan;
   float  avgLoud;

   /* calc pe by line == estimation for required bits without reduction */
   totalPe = 0;
   for(j=0; j<nChannels; j++) {
      (void)setDeboutVars(-1,-1,j,-1);
      psyOutChan = &(psyOut->psyOutChannel[j]);
      (void)sendDebout("estScalInp",psyOutChan->sfbActive,1,"thresBeforeAdj",
                       MTV_FLOAT,psyOutChan->sfbThreshold);

      totalPe += CalcPeSfbByLine(psyOutChan->mdctSpectrum, 
                                 psyOutChan->sfbEnergy, 
                                 psyOutChan->sfbThreshold, 
                                 psyOutChan->sfbOffsets, 
                                 psyOutChan->sfbActive);
   }
   /* pe->bits */
   estimBits = pe2bits(totalPe);

   /* avg loudness */
   avgLoud = avgLoudness(psyOut, nChannels);

   /* current fillLevel */
   curFillLevel = (float)bitresBits / maxBitresBits;

   /* calc reduction value from fillLevel */
   reductionValue = redValFromBres(curFillLevel, avgLoud);

   /* no reduction if less than 80% of average Bits are needed */
   if (estimBits < 0.8f*avgBits) {
      reductionValue = 0.0f;
   }
   /* do not use more than 150% of average Bits */
   if (estimBits > 1.5f*avgBits) {
      float maxPe = (float)bits2pe((int)(1.5f*avgBits));
      float peFactor = maxPe / totalPe;
      redValPe = redValFromPe(psyOut, nChannels, peFactor);
      reductionValue = max(reductionValue, redValPe);
   }

   return reductionValue;
}


#if (0)
static void
recalcThresholdsChannel(struct PSY_OUT_CHANNEL *psyOutChan, 
                        const float reductionValue)
{
   int i;
   float modConst, maxConst;
   float thrPowRedExp;
   float modSfbThr, sfbEn;
   float sfbEnm1, sfbEnm2;

   sfbEnm1 = sfbEnm2 = 0.0f;
   for (i=0; i<psyOutChan->sfbActive; i++) {

      sfbEn = psyOutChan->sfbEnergy[i];

      if (psyOutChan->sfbThreshold[i] < sfbEn) {

         thrPowRedExp = 
            (float)pow((double)psyOutChan->sfbThreshold[i], (double)redExp);
         /* calculate upper limit for const to avoid holes */
         maxConst = (float)pow((double)sfbEn, (double)redExp) - thrPowRedExp;
         /* modify const to avoid holes */
         if (maxConst > 0.0f) {
            modConst = maxConst *
                       (1.0f - (float)exp((double)(-reductionValue/maxConst)));
         }
         else {
            modConst = 0.0f;
         }
         /* recalc threshold with modified reduction const */
         modSfbThr = 
            (float)pow((double)(thrPowRedExp + modConst), (double)invRedExp);
         /* avoid hole really necessary in this band ? */
         if ( ((float)pow((double)(thrPowRedExp + reductionValue), 
                          (double)invRedExp) > sfbEn) ) {
            if ((sfbEn < 0.1f * sfbEnm1) || (sfbEn < 0.01f * sfbEnm2)) {
               /* do not avoid holes -> force threshold greater energy */
               modSfbThr = 2.0f * sfbEn;
            }
         } 
         else {
            /* force minimum of 1dB SNR; necessary for fastenc ? */
            if (modSfbThr > minSNR * sfbEn) modSfbThr = minSNR * sfbEn;
         }
         psyOutChan->sfbThreshold[i] = modSfbThr;
      }

      sfbEnm2 = sfbEnm1;
      sfbEnm1 = sfbEn;
   }
}
#endif


/* recalc threshold for all channels */
static void recalcThresholds(struct PSY_OUT *psyOut,
                             const int nChannels,
                             const float reductionValue)
{
   int ch, i;
   struct PSY_OUT_CHANNEL *psyOutChan;
   float sfbEn, thrExp, modSfbThr;
   float sfbEnm1, sfbEnm2;
   int ahFlagStereo, ahFlagMask;
   int sfbW, sfbWLast;
   float snr;
   float realRedVal;

   for (ch=0; ch<nChannels; ch++) {
      psyOutChan = &(psyOut->psyOutChannel[ch]);
      sfbEnm1 = sfbEnm2 = 0.0f;
      sfbWLast = psyOutChan->sfbOffsets[psyOutChan->sfbCnt] -
                 psyOutChan->sfbOffsets[psyOutChan->sfbCnt-1];
      for (i=0; i<psyOutChan->sfbActive; i++) {
         sfbEn = psyOutChan->sfbEnergy[i];
         if (psyOutChan->sfbThreshold[i] < sfbEn) {
            /* threshold to the power of 0.25 */
            thrExp = (float)pow(psyOutChan->sfbThreshold[i], redExp);

            realRedVal = reductionValue;
            /* use less reduction in case of a short block */
            if (psyOutChan->windowSequence == SHORT_WINDOW)
               realRedVal *= shortRedFactor;
            /* recalc threshold with reduction value */
            modSfbThr = (float)pow(thrExp + realRedVal, invRedExp);
            
            /* do not avoid hole because of dominant other channel ? */
            ahFlagStereo = 1;
            if (psyOut->msUsed) {
               struct PSY_OUT_CHANNEL *psyOutChan2;
               psyOutChan2 = &(psyOut->psyOutChannel[1-ch]);
               if (1.e2f*sfbEn < psyOutChan2->sfbEnergy[i]) {
                  ahFlagStereo = 0;
               }
            }
            /* do not avoid hole because of masking */
            ahFlagMask = 1;
            if ((psyOutChan->windowSequence != SHORT_WINDOW) && 
                ((sfbEn < 0.02f * sfbEnm1) || (sfbEn < 0.002f * sfbEnm2))) {
               ahFlagMask = 0;
            }
            /* evtl avoid hole */
            if (ahFlagStereo && ahFlagMask) {
               sfbW = psyOutChan->sfbOffsets[i+1] - psyOutChan->sfbOffsets[i];
               snr = (float)pow(minSNR, sqrt((float)sfbWLast/sfbW));
               if (modSfbThr > snr * sfbEn) {
                  modSfbThr = snr * sfbEn;
               }
               /* interpolate thr between snr*energy and energy 
                  (for thr==1.e2*energy) */
               if (modSfbThr > snr * sfbEn) {
                  modSfbThr = ((1.0f-snr)*modSfbThr + (1.e2f-1.0f)*snr*sfbEn) /
                               (1.e2f-snr);
               }
            }

            /* set threshold to new calculated value */
            psyOutChan->sfbThreshold[i] = modSfbThr;
         }
         sfbEnm2 = sfbEnm1;
         sfbEnm1 = sfbEn;
      }
   }
}

void AdjustThresholds(struct PSY_OUT *psyOut, 
                      float *chBitDistribution,
                      const int nChannels,
                      const int avgBits,
                      const int bitresBits,
                      const int maxBitresBits)
{
   float reductionValue;
   int ch;

   /* calc threshold reduction value from avg bits and bitres fullness */
   reductionValue = calcThresholdReduction(psyOut, 
                                           nChannels,
                                           avgBits, 
                                           bitresBits,
                                           maxBitresBits);

   /* apply  threshold reduction factor to thresholds */
   recalcThresholds(psyOut, nChannels, reductionValue);

   /* calculate relative distribution of bits among the channels */
   for (ch=0; ch<nChannels; ch++) {
      chBitDistribution[ch] = 1.0f / nChannels;
   }
}

#else /* #ifdef OLD_THR_REDUCTION */


/* values for avoid hole flag */
enum {
   NO_AH              =0,
   AH_INACTIVE        =1,
   AH_ACTIVE          =2
};

static __inline float pow_4(float x)
{
   return (x*x)*(x*x);
}

static __inline float pow_1_4(float x)
{
   return (float)sqrt(sqrt(x));
}

/* convert from bits to pe */
float bits2pe(const float bits) {
   return (bits * 1.18f);
}

float pe2bits(const float bits) {
   return (bits * 0.8474576f);
}

/* loudness calculation (threshold to the power of redExp) */
#ifndef P4_CODE
static void calcThreshExp_NoOpt(float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
				struct PSY_OUT *psyOut,
				/*const int nChannels, */
				const int startCh,
				const int endCh)
{
   int ch, sfb;
   for (ch=startCh; ch<endCh; ch++) {
     struct PSY_OUT_CHANNEL *psyOutChan = &(psyOut->psyOutChannel[ch]);
      for (sfb=0; sfb<psyOutChan->sfbActive; sfb++) {
         thrExp[ch][sfb] = pow_1_4(psyOutChan->sfbThreshold[sfb]);
      }
   }
}

void (*calcThreshExp)(float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
		      struct PSY_OUT *psyOut,
		      /*const int nChannels, */
		      const int startCh,
		      const int endCh) =  calcThreshExp_NoOpt;
#endif

/* determine bands where avoid hole is not necessary resp. possible */
static void initAvoidHoleFlag(int ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                              struct PSY_OUT *psyOut,
                              const int nChannels,
                              const int startCh,
                              const int endCh)
{
   int ch, sfb;
   float sfbEn, sfbEnm1, sfbEnm2, sfbEnm3;

   /* init ahFlag (0: no ah necessary, 1: ah possible, 2: ah active */
   for(ch=startCh; ch<endCh; ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
      sfbEnm1 = sfbEnm2 = sfbEnm3 = 0.0f;

      for (sfb=0; sfb<psyOutChan->sfbActive; sfb++) {
         sfbEn  = psyOutChan->sfbEnergy[sfb];
         /* avoid hole necessary for this band ? */
         ahFlag[ch][sfb] = AH_INACTIVE;
         /* do not avoid hole because of dominant other channel ? */
         if ((nChannels==2) && (psyOut->msUsed)) {
            struct PSY_OUT_CHANNEL *psyOutChan2 = &psyOut->psyOutChannel[1-ch];
            if (1.e1f*sfbEn < psyOutChan2->sfbEnergy[sfb]) {
               ahFlag[ch][sfb] = NO_AH;
            }
         }
         /* do not avoid hole because of masking (only for long blocks) */
         if ((psyOutChan->windowSequence != SHORT_WINDOW) && 
             ((sfbEn < 0.05f * sfbEnm1) || (sfbEn < 0.005f * sfbEnm2))) {
            ahFlag[ch][sfb] = NO_AH;
         }
         /* do not avoid hole because of masking part 2 */
         /* consider energies previous (in freq) to sfbEnm2 with a 8dB decay per band */
         if ((psyOutChan->windowSequence != SHORT_WINDOW) && 
             (sfbEn < sfbEnm3*0.158f) && (sfb>10) ) {
             ahFlag[ch][sfb] = NO_AH;
         }

         /* allow holes when signal energy is below threshold */
         if (psyOut->psyOutChannel[ch].sfbEnergy[sfb] < psyOut->psyOutChannel[ch].sfbThreshold[sfb])
           ahFlag[ch][sfb] = NO_AH;

         /* update energies of last sfbs sfbEnm1, sfbEnm2 and sfbEnm3 */
         sfbEnm3 = max(sfbEnm2*0.005f, sfbEnm3*0.158f);
         sfbEnm2 = sfbEnm1;
         sfbEnm1 = sfbEn;
      }
   }
}


/* constants that do not change during successive pe calculations */
void preparePe( PE_DATA *peData,
                struct PSY_OUT *psyOut, 
                float sfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
                /*const int nChannels,*/
                const int startCh,
                const int endCh )
{
   int ch;

   for(ch=startCh; ch<endCh; ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
      prepareSfbPe(&peData->peChannelData[ch],
                   psyOutChan->sfbEnergy, 
                   psyOutChan->sfbThreshold, 
                   sfbFormFactor[ch],
                   psyOutChan->sfbOffsets, 
                   psyOutChan->sfbActive,
                   psyOutChan->windowSequence);
   }
}

/* calculate pe for both channels */
void calcPe(PE_DATA *peData,
            struct PSY_OUT *psyOut,
            /*const int nChannels,*/
            const int startCh,
            const int endCh)
{
   int ch;

   peData->pe = 0.0f;
   peData->constPart = 0.0f;
   peData->nActiveLines = 0;
   for(ch=startCh; ch<endCh; ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
      PE_CHANNEL_DATA *peChanData = &peData->peChannelData[ch];
      calcSfbPe(&peData->peChannelData[ch],
                psyOutChan->sfbEnergy, 
                psyOutChan->sfbThreshold, 
                psyOutChan->sfbActive);
      peData->pe += peChanData->pe;
      peData->constPart += peChanData->constPart;
      peData->nActiveLines += peChanData->nActiveLines;
      psyOut->psyOutChannel[ch].pe = peChanData->pe ;
   }
}
 


/* sum the pe data only for bands where avoid hole is inactive */
static void calcPeNoAH(float *pe,
                       float *constPart,
                       int   *nActiveLines,
                       PE_DATA *peData,
                       int ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                       struct PSY_OUT *psyOut,
                       /*const int nChannels,*/
                       const int startCh,
                       const int endCh)
{
   int ch, sfb;

   *pe = 0.0f;
   *constPart = 0.0f;
   *nActiveLines = 0;
   for(ch=startCh; ch<endCh; ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
      PE_CHANNEL_DATA *peChanData = &peData->peChannelData[ch];
      for (sfb=0; sfb<psyOutChan->sfbActive; sfb++) {
         if (ahFlag[ch][sfb] < AH_ACTIVE) {
            *pe += peChanData->sfbPe[sfb];
            *constPart += peChanData->sfbConstPart[sfb];
            *nActiveLines += peChanData->sfbNActiveLines[sfb];
         }
      }
   }
}


/* apply reduction formula */
static void reduceThresholds(struct PSY_OUT *psyOut,
                             int ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                             float sfbMinSnrTmp[MAX_CHANNELS][MAX_GROUPED_SFB],
                             float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
                             /*const int nChannels,*/
                             const float redVal,
                             const int startCh,
                             const int endCh)
{
   int ch, sfb;
   float sfbEn, sfbThr;
   const float *sfbMinSnr;

   for(ch=startCh; ch<endCh; ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
         sfbMinSnr = sfbMinSnrTmp[ch];
      for (sfb=0; sfb<psyOutChan->sfbActive; sfb++) {
         sfbEn  = psyOutChan->sfbEnergy[sfb];
         sfbThr = psyOutChan->sfbThreshold[sfb];
         if (sfbEn > sfbThr) {
            /* threshold reduction formula */
            sfbThr = pow_4(thrExp[ch][sfb]+redVal);
            /* avoid holes */
            if ((sfbThr > sfbMinSnr[sfb]*sfbEn) && (ahFlag[ch][sfb] != NO_AH)){
               sfbThr = max(psyOutChan->sfbThreshold[sfb], 
                            sfbMinSnr[sfb] * sfbEn);
               ahFlag[ch][sfb] = AH_ACTIVE;
            }
            psyOutChan->sfbThreshold[sfb] = sfbThr;
         }
      }
   }
}


/* if pe difference deltaPe between desired pe and real pe is small enough,
   the difference can be distributed among the scale factor bands.
   New thresholds can be derived from this pe-difference */
static void correctThresh(struct PSY_OUT *psyOut, 
                          int ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
			  float sfbMinSnrTmp[MAX_CHANNELS][MAX_GROUPED_SFB],
                          PE_DATA *peData,
                          float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
                          const float redVal,
                          /*const int nChannels, */
                          const float deltaPe,
                          const int startCh,
                          const int endCh)
{
   int ch, i;
   struct PSY_OUT_CHANNEL *psyOutChan;
   PE_CHANNEL_DATA *peChanData;
   float deltaSfbPe;
   float thrFactor;
   float sfbPeFactors[MAX_CHANNELS][MAX_GROUPED_SFB], normFactor;
   const float *sfbMinSnr;

   /* for each sfb calc relative factors for pe changes */
   normFactor = 0.0f;
   for(ch=startCh; ch<endCh; ch++) {
      psyOutChan = &(psyOut->psyOutChannel[ch]);
      peChanData = &peData->peChannelData[ch];
      for (i=0; i<psyOutChan->sfbActive; i++) {
         if ((ahFlag[ch][i] < AH_ACTIVE) || (deltaPe > 0)) {
            sfbPeFactors[ch][i] = peChanData->sfbNActiveLines[i] /
                                  (thrExp[ch][i] + redVal);
            normFactor += sfbPeFactors[ch][i];
         }
         else {
            sfbPeFactors[ch][i] = 0.0f;
         }
      }
   }
   if (normFactor > 0.0f) {

      normFactor = 1.0f/normFactor;

      /* distribute the pe difference to the scalefactors
         and calculate the according thresholds */
      for(ch=startCh; ch<endCh; ch++) {
         psyOutChan = &(psyOut->psyOutChannel[ch]);
         peChanData = &peData->peChannelData[ch];
         sfbMinSnr = sfbMinSnrTmp[ch];
         for (i=0; i<psyOutChan->sfbActive; i++) {
            /* pe difference for this sfb */
            deltaSfbPe = sfbPeFactors[ch][i] * normFactor * deltaPe;
            if (peChanData->sfbNActiveLines[i] > 0) {
               /* new threshold */
               /* limit thrFactor to 60dB */
               thrFactor = min(-deltaSfbPe/peChanData->sfbNActiveLines[i],20.f);
               thrFactor = (float)pow(2.0f, thrFactor);
               psyOutChan->sfbThreshold[i] *= thrFactor;
               /* avoid hole */
               if ((psyOutChan->sfbThreshold[i] > 
                        sfbMinSnr[i]*psyOutChan->sfbEnergy[i]) 
                     && (ahFlag[ch][i] == AH_INACTIVE)) {
                  psyOutChan->sfbThreshold[i] = sfbMinSnr[i] *
                     psyOutChan->sfbEnergy[i];
                  ahFlag[ch][i] = AH_ACTIVE;
               }
            }
         }
      }
   }
}



#define NUM_NRG_LEVS 4 /* for allow more holes */
static const float minSnrLimit = 0.8f; /* 1dB */
static const int   nrHighSfbAhPreserve = 0; /* Number of sfb counting 
                                               from highest which will 
                                               preserve the AH-Flag */

static void initMinSnr( struct PSY_OUT *psyOut, 
                        const float *sfbMinSnrLong, 
                        const float *sfbMinSnrShort, 
                              float  sfbMinSnr [MAX_CHANNELS] [MAX_GROUPED_SFB],
                        const int nChannels,
                        const int startCh
                        /*,const int endCh */)
{
  int ch;
  for(ch=startCh; ch<(startCh+nChannels); ch++) {
    struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
    if (psyOutChan->windowSequence != SHORT_WINDOW) {
      copyFLOAT(sfbMinSnrLong, sfbMinSnr[ch],  MAX_SFB_LONG);
    }
    else{
      copyFLOAT(sfbMinSnrShort, sfbMinSnr[ch],  MAX_GROUPED_SFB);
    }
  }
}


/* if the desired pe can not be reached, reduce pe by reducing minSnr */
static void reduceMinSnr(struct PSY_OUT *psyOutElement,
			 PE_DATA *peData,
                         int   ahFlag    [MAX_CHANNELS] [MAX_GROUPED_SFB],
			 float sfbMinSnrTmp [MAX_CHANNELS] [MAX_GROUPED_SFB],
                         const int startCh,
                         const int nChannels,
                         const float desiredPe)
{
  int ch, sfb;
  float deltaPe;

#ifdef PLOTMTV
  for (ch=startCh;ch<(startCh+nChannels); ch++) {
    setDeboutVars(-1, -1, ch ,-1);
    sendDebout("minSNR", psyOutElement->psyOutChannel[ch].sfbActive, 1,"org",MTV_FLOAT, sfbMinSnrTmp);
  }
#endif
  /* handle 1 channel or 2 channels with same window */
  if ( nChannels==1 || 
       (  nChannels==2   &&
          (psyOutElement->psyOutChannel[startCh  ].windowSequence == 
           psyOutElement->psyOutChannel[startCh+1].windowSequence)  )) {

    struct PSY_OUT_CHANNEL *psyOutChannel = psyOutElement->psyOutChannel;
    /* start at highest freq down to 0 */
    sfb = psyOutChannel[startCh].sfbActive-1;
    while ( (peData->pe>desiredPe) && (sfb>=0) ) {
      /* loop over all channels */
      for (ch=startCh; ch<(startCh+nChannels); ch++) {
        float *weightedEn = psyOutChannel[ch].sfbEnergy;
        float *sfbMinSnr  = sfbMinSnrTmp[ch];

	if (ahFlag[ch][sfb] != NO_AH &&
	    sfbMinSnr[sfb] < minSnrLimit) {
	  /* increase threshold to new minSnr of 1dB */
	  sfbMinSnr[sfb] = minSnrLimit;
	  psyOutChannel[ch].sfbThreshold[sfb] =
	    weightedEn[sfb] * sfbMinSnr[sfb];

	  /* calc new pe */
	  /* C2 + C3*ld(1/0.8) = 1.5 (see line_pe.c) */
	  deltaPe = peData->peChannelData[ch].sfbNLines[sfb] * 1.5f -
	    peData->peChannelData[ch].sfbPe[sfb];
	  peData->pe += deltaPe;
	  peData->peChannelData[ch].pe += deltaPe;
	}
      }
      /* stop if enough has been saved */
      if (peData->pe <= desiredPe)
	break;
      sfb--;
    }
  }
  else {
    /* loop over all channels */
    for (ch=startCh; ch<(startCh+nChannels); ch++) {
      struct PSY_OUT_CHANNEL *psyOutChannel = psyOutElement->psyOutChannel;
      float *weightedEn = psyOutChannel[ch].sfbEnergy;
      float *sfbMinSnr  = sfbMinSnrTmp[ch];

      /* start at highest freq down to 0 */
      sfb = psyOutChannel[startCh].sfbActive-1;
      while ( (peData->pe>desiredPe) && (sfb>=0) ) {
	if (ahFlag[ch][sfb] != NO_AH &&
	    sfbMinSnr[sfb] < minSnrLimit) {
	  /* increase threshold to new minSnr of 1dB */
	  sfbMinSnr[sfb] = minSnrLimit;
	  psyOutChannel[ch].sfbThreshold[sfb] =
	    weightedEn[sfb] * sfbMinSnr[sfb];

	  /* calc new pe */
	  /* C2 + C3*ld(1/0.8) = 1.5 (see line_pe.c) */
	  deltaPe = peData->peChannelData[ch].sfbNLines[sfb] * 1.5f -
	    peData->peChannelData[ch].sfbPe[sfb];
	  peData->pe += deltaPe;
	  peData->peChannelData[ch].pe += deltaPe;
	}
        sfb--;
      }
      /* stop if enough has been saved */
      if (peData->pe <= desiredPe)
	break;
    }
  }
#ifdef PLOTMTV
  for (ch=startCh;ch<(startCh+nChannels); ch++) {
    setDeboutVars(-1, -1, ch ,-1);
    sendDebout("minSNR", psyOutElement->psyOutChannel[ch].sfbActive, 1,"reduced1",MTV_FLOAT, sfbMinSnrTmp[ch]);
  }
#endif
}


/* if the desired pe can not be reached, some more scalefactor bands have to be 
   quantized to zero */
static void allowMoreHoles(struct PSY_OUT *psyOut,
                           PE_DATA *peData, 
                           const AH_PARAM *ahParam,
                           int   ahFlag    [MAX_CHANNELS] [MAX_GROUPED_SFB],
                           float sfbMinSnrTmp [MAX_CHANNELS] [MAX_GROUPED_SFB],
                           const int startCh,
                           const int nChannels,
                           const float desiredPe)
{
  int ch, sfb;
  float actPe;

  actPe = peData->pe;
  
  /* for MS allow hole in the channel with less energy */
  if ( (nChannels==2)             &&
       ( psyOut->msUsed == 1 )    &&
       ( psyOut->psyOutChannel[0].windowSequence == psyOut->psyOutChannel[1].windowSequence) ) {

    struct PSY_OUT_CHANNEL *psyOutChanL = &psyOut->psyOutChannel[0];
    struct PSY_OUT_CHANNEL *psyOutChanR = &psyOut->psyOutChannel[1];
    float *sfbMinSnr  = sfbMinSnrTmp[startCh];

    for (sfb=0; sfb<psyOutChanL->sfbActive; sfb++) {
      /* allow hole in side channel ? */
      if (ahFlag[1][sfb] != NO_AH &&
	  0.4f*sfbMinSnr[sfb]*psyOutChanL->sfbEnergy[sfb] >
	  psyOutChanR->sfbEnergy[sfb]) {
	ahFlag[1][sfb] = NO_AH;
	psyOutChanR->sfbThreshold[sfb] = 2.0f * psyOutChanR->sfbEnergy[sfb];
	actPe -= peData->peChannelData[1].sfbPe[sfb];
      }
      /* allow hole in mid channel ? */
      else if (ahFlag[0][sfb] != NO_AH &&
	       0.4f*sfbMinSnr[sfb]*psyOutChanR->sfbEnergy[sfb] >
	       psyOutChanL->sfbEnergy[sfb]) {
	ahFlag[0][sfb] = NO_AH;
	psyOutChanL->sfbThreshold[sfb] = 2.0f * psyOutChanL->sfbEnergy[sfb];
	actPe -= peData->peChannelData[0].sfbPe[sfb];
      }
      if (actPe < desiredPe)
	break;
    }
  }

  /* more holes necessary? subsequently erase bands 
       starting with low energies */
  if (actPe > desiredPe) {
    int startSfb[2];
    float avgEn, minEn;
    int ahCnt;
    int enIdx;
    float en[NUM_NRG_LEVS];
    int minSfb, maxSfb;
    int done;

    /* do not go below startSfb */
    for (ch=startCh; ch<(startCh+nChannels); ch++) {
      if (psyOut->psyOutChannel[ch].windowSequence != SHORT_WINDOW)
        startSfb[ch] = ahParam->startSfbL;
      else
        startSfb[ch] = ahParam->startSfbS;
    }

    /* calc avg and min energies of bands that avoid holes */
    avgEn = 0.0f;
    minEn = FLT_MAX;
    ahCnt = 0;
    for (ch=startCh; ch<(startCh+nChannels); ch++) {
      struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
      for (sfb=startSfb[ch]; sfb<psyOutChan->sfbActive; sfb++){
        if ((ahFlag[ch][sfb]!=NO_AH) &&
            (psyOutChan->sfbEnergy[sfb] > psyOutChan->sfbThreshold[sfb])){
          minEn = min(minEn, psyOutChan->sfbEnergy[sfb]);
          avgEn += psyOutChan->sfbEnergy[sfb];
          ahCnt++;
        }
      }
    }
    avgEn = (float)min ( FLT_MAX , (double)avgEn /((double)ahCnt+FLT_MIN));

    /* calc some energy borders between minEn and avgEn */
    for (enIdx=0; enIdx<NUM_NRG_LEVS; enIdx++)
      en[enIdx] = minEn * (float)pow(avgEn/(minEn+FLT_MIN), (2.0f*enIdx+1.0f)/(2.0f*NUM_NRG_LEVS-1.0f));

    /* start with lowest energy border at highest sfb */
    maxSfb = psyOut->psyOutChannel[0].sfbActive - 1 - nrHighSfbAhPreserve;
    minSfb = startSfb[0];
    if (nChannels==2) {
      maxSfb = max(maxSfb, psyOut->psyOutChannel[1].sfbActive - 1 - nrHighSfbAhPreserve);
      minSfb = min(minSfb, startSfb[1]);
    }
    /* loop through channels and sfb and allow hole where possible */
    sfb = maxSfb;
    enIdx = 0;
    done = 0;
    while (!done) {
      for (ch=0; ch<nChannels; ch++) {
        struct PSY_OUT_CHANNEL *psyOutChan = &psyOut->psyOutChannel[ch];
        if (sfb>=startSfb[ch] && sfb < psyOutChan->sfbActive) {
          /* sfb energy below border ? */
          if (ahFlag[ch][sfb] != NO_AH && psyOutChan->sfbEnergy[sfb] < en[enIdx]){
            /* allow hole */
            ahFlag[ch][sfb] = NO_AH;
            psyOutChan->sfbThreshold[sfb] = 2.0f * psyOutChan->sfbEnergy[sfb];
            actPe -= peData->peChannelData[ch].sfbPe[sfb];
          }
          if (actPe < desiredPe) {
            done = 1;
            break;
          }
        }
      }
      sfb--;
      if (sfb < minSfb) {
        /* restart with next energy border */
        sfb = maxSfb;
        enIdx++;
        if (enIdx >= NUM_NRG_LEVS)
          done = 1;
      }
    }
  }
}


/* two guesses for the reduction value and one final correction of the
   thresholds */
static void adaptThresholdsToPe(struct PSY_OUT *psyOut, 
                                PE_DATA *peData,
                                float *sfbMinSnrLong,
                                float *sfbMinSnrShort,
                                float sfbMinSnr[MAX_CHANNELS][MAX_GROUPED_SFB],
                                AH_PARAM ahParam,
                                const int nChannels,
                                const float desiredPe,
                                const int startCh,
                                const int endCh)
{
   float noRedPe, redPe1, redPe1NoAH, redPe2;
   float constPart, constPartNoAH;
   int   nActiveLines, nActiveLinesNoAH;
   float desiredPeNoAH; 
#if defined _MSC_VER && !defined __ICL && ( _MSC_VER < 1400 )
   /* There seems to be a bug in VS6/7 compiler's optimizer,
      avgThrExp is not saved from FP stack before calling redVal = pow(...)
      avgThrExp might be overwritten when using the Intel compiler with no SSE 
      or AMD machines (<= Ver8.1)
   */
   volatile float avgThrExp;
   float redVal;
#else
   float avgThrExp, redVal;
#endif
   int   ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB];
   float thrExp[MAX_CHANNELS][MAX_GROUPED_SFB];

   /* thresholds to the power of redExp */
   calcThreshExp(thrExp, psyOut, /*nChannels,*/ startCh, endCh);

   /* init ahFlag (0: no ah necessary, 1: ah possible, 2: ah active */
   initAvoidHoleFlag(ahFlag, psyOut, nChannels, startCh, endCh);

   initMinSnr( psyOut, sfbMinSnrLong, sfbMinSnrShort, sfbMinSnr,
               nChannels, startCh /*, endCh*/);
   /* pe without reduction */
   noRedPe = peData->pe;
   constPart = peData->constPart;
   nActiveLines = peData->nActiveLines;

   /* first guess of reduction value */
   avgThrExp = (float)pow(2.0f, (constPart - noRedPe)/(invRedExp*nActiveLines));
   redVal    = (float)pow(2.0f, (constPart - desiredPe)/(invRedExp*nActiveLines)) - 
               avgThrExp;

   /* reduce thresholds */
   reduceThresholds(psyOut, ahFlag, sfbMinSnr, thrExp,
                    /*nChannels,*/ redVal, startCh, endCh);
   /* pe after first guess */
   calcPe(peData, psyOut, /*nChannels,*/ startCh, endCh);

   redPe1 = peData->pe;

   /*   fprintf( f_plot_4, "%i\t%f\n", count, redPe1 );*/

   /* pe for bands where avoid hole is inactive */
   calcPeNoAH(&redPe1NoAH, &constPartNoAH, &nActiveLinesNoAH, 
              peData, ahFlag, psyOut, /*nChannels,*/ startCh, endCh);

   /* new desired pe without bands where avoid hole is active */
   desiredPeNoAH = desiredPe - (redPe1 - redPe1NoAH);

   /* second guess (only if there are bands left where avoid hole is inactive)*/
   if (nActiveLinesNoAH > 0) {
      avgThrExp = (float)pow(2.0f, (constPartNoAH - redPe1NoAH)/
                                   (invRedExp*nActiveLinesNoAH));
      redVal   += (float)pow(2.0f, (constPartNoAH - desiredPeNoAH)/
                                   (invRedExp*nActiveLinesNoAH)) -
                  avgThrExp;

      /* reduce thresholds */
      reduceThresholds(psyOut, ahFlag, sfbMinSnr, thrExp, 
                       /*nChannels,*/ redVal, startCh, endCh);
   }

   /* pe after second guess */
   calcPe(peData, psyOut, /*nChannels,*/ startCh, endCh);
   redPe2 = peData->pe;

   /*   fprintf( f_plot_5, "%i\t%f\n", count, redPe2 );*/

   if (redPe2 < 1.15f*desiredPe) {
     /* correct thresholds to get closer to the desired pe */
     correctThresh(psyOut, ahFlag, sfbMinSnr, peData, thrExp, 
                   redVal, /*nChannels,*/ desiredPe - redPe2, startCh, endCh);
   }
   else {
     /* reduce pe by reducing minSnr requirements */
     reduceMinSnr(psyOut, peData, ahFlag, sfbMinSnr,
		  startCh, nChannels, 1.05f*desiredPe);
     /* reduce pe by allowing additional spectral holes */ 
     allowMoreHoles(psyOut, peData, &ahParam, ahFlag, sfbMinSnr,
		    startCh, nChannels, 1.05f*desiredPe);
   }

#ifdef PLOTMTV
   /* pe after correction (only necessary for debugging) */
   calcPe(peData, psyOut, /* nChannels, */ startCh, endCh);
   sendDeboutHist( "pe", "redPe_tmp1", MTV_FLOAT, &redPe1 );
   sendDeboutHist( "pe", "redPe_tmp2", MTV_FLOAT, &redPe2 );
   sendDeboutHist( "pe", "redPe", MTV_FLOAT, &(peData->pe) );
   /*   fprintf( f_plot_6, "%i\t%f\n", count, peData->pe );*/
#endif
}


/*****************************************************************************

    functionname: calcBitSave 
    description:  Calculates percentage of bit save, see figure below  
    returns:
    input:        parameters and bitres-fullness
    output:       percentage of bit save

*****************************************************************************/


/*
        bitsave
                    maxBitSave(%)|   clipLow
                                 |---\
                                 |    \
                                 |     \
                                 |      \
                                 |       \
                                 |--------\--------------> bitres
                                 |         \
                    minBitSave(%)|          \------------
                                          clipHigh      maxBitres
*/

static float calcBitSave(float fillLevel,
                         const float clipLow,
                         const float clipHigh,
                         const float minBitSave,
                         const float maxBitSave)
{
   float bitsave;

   fillLevel = max(fillLevel, clipLow);
   fillLevel = min(fillLevel, clipHigh);

   bitsave = maxBitSave - ((maxBitSave-minBitSave) / (clipHigh-clipLow)) *
                          (fillLevel-clipLow);  

   return (bitsave);
}       



/*****************************************************************************

    functionname: calcBitSpend 
    description:  Calculates percentage of bit spend, see figure below   
    returns:
    input:        parameters and bitres-fullness
    output:       percentage of bit spend

*****************************************************************************/


/*
                              bitspend      clipHigh
                   maxBitSpend(%)|          /-----------maxBitres
                                 |         /
                                 |        /
                                 |       /
                                 |      /
                                 |     / 
                                 |----/-----------------> bitres
                                 |   /       
                   minBitSpend(%)|--/         
                                   clipLow      
*/

static float calcBitSpend(float fillLevel,
                          const float clipLow,
                          const float clipHigh,
                          const float minBitSpend,
                          const float maxBitSpend)
{
   float bitspend;

   fillLevel = max(fillLevel, clipLow);
   fillLevel = min(fillLevel, clipHigh);

   bitspend = minBitSpend + ((maxBitSpend-minBitSpend) / (clipHigh-clipLow)) *
                            (fillLevel-clipLow);      

   return (bitspend);
}


/*****************************************************************************

    functionname: adjustPeMinMax()
    description:  adjusts peMin and peMax parameters over time
    returns:
    input:        current pe, peMin, peMax, bitres size 
    output:       adjusted peMin/peMax

*****************************************************************************/
static void adjustPeMinMax(const float currPe,
                           float      *peMin,
                           float      *peMax
                           /*, const int   bitresMax*/)
{
  float minFacHi = 0.3f, maxFacHi = 1.0f, minFacLo = 0.14f, maxFacLo = 0.07f;
  float diff;
  float minDiff = currPe / 4.0f;

  if (currPe > *peMax) {
     diff = (currPe-*peMax) ;
     *peMin += diff * minFacHi;
     *peMax += diff * maxFacHi;
  } else if (currPe < *peMin) {
     diff = (*peMin-currPe) ;
     *peMin -= diff * minFacLo;
     *peMax -= diff * maxFacLo;
  } else {
     *peMin += (currPe - *peMin) * minFacHi;
     *peMax -= (*peMax - currPe) * maxFacLo;
  }
  
  if ((*peMax - *peMin) < minDiff) {
     float partLo, partHi;

     partLo = max(0.0f, currPe - *peMin);
     partHi = max(0.0f, *peMax - currPe); 

     *peMax = currPe + partHi/(partLo+partHi) * minDiff;
     *peMin = currPe - partLo/(partLo+partHi) * minDiff;
     *peMin = max(0.0f, *peMin);
  }

}


/*
                     bitfac(%)            pemax
                   bitspend(%)   |          /-----------maxBitres
                                 |         /
                                 |        /
                                 |       /
                                 |      /
                                 |     / 
                                 |----/-----------------> pe
                                 |   /       
                   bitsave(%)    |--/         
                                    pemin      
*/

        
        
        
/*****************************************************************************

    functionname: BitresCalcBitFac
    description:  calculates factor of spending bits for one frame
                    1.0 : take all frame dynpart bits
                   >1.0 : take all frame dynpart bits + bitres
                   <1.0 : put bits in bitreservoir
    returns:      BitFac
    input:        bitres-fullness, pe, blockType, parameter-settings
    output:       

*****************************************************************************/
static float bitresCalcBitFac(const int bitresBits, 
                              const int maxBitresBits, 
                              const int avgBits,
                              const float maxBitFac,
                              const float pe,
                              const int windowSequence,
                              struct ADJ_THR_STATE *hAdjThr)
{
   BRES_PARAM *bresParam;
   float pex;
   float fillLevel;
   float bitSave, bitSpend, bitresFac;
   float relBresSize;

   if (maxBitresBits > avgBits)
      relBresSize = 1.0f;
   else
      relBresSize = (float)maxBitresBits / avgBits;

   if (maxBitresBits > 0) {

      fillLevel = (float)bitresBits / maxBitresBits;

      if (windowSequence != SHORT_WINDOW) 
         bresParam = &(hAdjThr->bresParamLong);
      else
         bresParam = &(hAdjThr->bresParamShort);

      pex = max(pe, hAdjThr->peMin);
      pex = min(pex,hAdjThr->peMax);

      bitSave = relBresSize * calcBitSave(fillLevel,
            bresParam->clipSaveLow, bresParam->clipSaveHigh,
            bresParam->minBitSave, bresParam->maxBitSave);

      bitSpend = relBresSize * calcBitSpend(fillLevel,
            bresParam->clipSpendLow, bresParam->clipSpendHigh,
            bresParam->minBitSpend, bresParam->maxBitSpend);

      bitresFac = 1.0f - bitSave + 
         ((bitSpend + bitSave) / (hAdjThr->peMax - hAdjThr->peMin)) * 
         (pex - hAdjThr->peMin);

      adjustPeMinMax(pe, &hAdjThr->peMin, &hAdjThr->peMax /*, maxBitresBits*/);

   }
   else {
      bitresFac = 1.0f;
   }

   /* limit bitresFac to not spend more bits than available */
   bitresFac = min(bitresFac, 1.0f + 0.95f*(float)bitresBits/avgBits - 0.10f);
   /* limit bitresFac for high bitrates */
   bitresFac = min(bitresFac, maxBitFac);

   return bitresFac;
}


int AdjThrNew(struct ADJ_THR_STATE **phAdjThr)
{
  struct ADJ_THR_STATE *hAdjThr = 
                 (struct ADJ_THR_STATE *)mp3Alloc(sizeof(struct ADJ_THR_STATE));
  *phAdjThr = hAdjThr;
  return (hAdjThr == 0);
}

void AdjThrDelete(struct ADJ_THR_STATE *hAdjThr)
{
  if (hAdjThr)
  {
    mp3Free(hAdjThr);
  }
}


static const float sfbMinSnrL[MAX_SFB_LONG] = {
   0.004f, 0.004f, 0.006f, 0.011f, 0.020f,
   0.032f, 0.050f, 0.100f, 0.200f, 0.320f,
   0.440f, 0.630f, 0.630f, 0.630f, 0.630f, 
   0.630f, 0.630f, 0.630f, 0.630f, 0.630f,
   0.630f, 0.630f
};
static const float sfbMinSnrS[MAX_SFB_SHORT] = {
   0.004f, 0.004f, 0.004f, 0.006f, 0.016f, 
   0.050f, 0.250f, 0.500f, 0.630f, 0.630f,
   0.630f, 0.630f, 0.630f
};

void AdjThrInit(struct ADJ_THR_STATE *hAdjThr, 
                const float meanPe,
                const int channelBitrate,
                const float bitsPerSample
                )
{
   int sfb;
   float minSnrLimit, minSnrLimit2;

   /* bitres control */
   hAdjThr->peMin = 0.8f * meanPe;
   hAdjThr->peMax = 1.2f * meanPe;

   hAdjThr->bresParamLong.clipSaveLow   =  0.20f;
   hAdjThr->bresParamLong.clipSaveHigh  =  0.90f;
   hAdjThr->bresParamLong.minBitSave    = -0.05f;
   hAdjThr->bresParamLong.maxBitSave    =  0.30f;

   hAdjThr->bresParamLong.clipSpendLow  =  0.20f;
   hAdjThr->bresParamLong.clipSpendHigh =  0.90f;
   hAdjThr->bresParamLong.minBitSpend   = -0.03f;
   hAdjThr->bresParamLong.maxBitSpend   =  0.50f;

   hAdjThr->bresParamShort.clipSaveLow   =  0.20f;
   hAdjThr->bresParamShort.clipSaveHigh  =  0.90f;
   hAdjThr->bresParamShort.minBitSave    = -0.05f;
   hAdjThr->bresParamShort.maxBitSave    =  0.20f;

   hAdjThr->bresParamShort.clipSpendLow  =  0.20f;
   hAdjThr->bresParamShort.clipSpendHigh =  0.80f;
   hAdjThr->bresParamShort.minBitSpend   = -0.03f;
   hAdjThr->bresParamShort.maxBitSpend   =  1.00f;

   /* pe correction */
   hAdjThr->peLast = 0.0f;
   hAdjThr->dynBitsLast = 0;
   hAdjThr->peCorrectionFactor = 1.0f;
   
   /* minSnr */
   minSnrLimit = (float) pow(10.0f,-channelBitrate/32000.0f*10.0f/10.0f);
   minSnrLimit2 = 0.63f; /* 2dB */
   if (channelBitrate >= 48000)
      minSnrLimit2 = 0.5f; /* 3dB */
   for (sfb=0; sfb<MAX_SFB_LONG; sfb++) {
      hAdjThr->sfbMinSnrLong[sfb] = max(sfbMinSnrL[sfb], minSnrLimit);
      hAdjThr->sfbMinSnrLong[sfb] = min(hAdjThr->sfbMinSnrLong[sfb], 
                                        minSnrLimit2);
   }
   minSnrLimit = (float) pow(10.0f,-channelBitrate/32000.0f*12.0f/10.0f);
   for (sfb=0; sfb<MAX_SFB_SHORT; sfb++) {
      hAdjThr->sfbMinSnrShort[TRANS_FAC*sfb]=max(sfbMinSnrS[sfb], minSnrLimit);
      hAdjThr->sfbMinSnrShort[TRANS_FAC*sfb+1] = 
         hAdjThr->sfbMinSnrShort[TRANS_FAC*sfb];
      hAdjThr->sfbMinSnrShort[TRANS_FAC*sfb+2] = 
         hAdjThr->sfbMinSnrShort[TRANS_FAC*sfb];
   }

   if(bitsPerSample > 1.5f) {
     hAdjThr->ahParam.modifyMinSnr = FALSE;
     hAdjThr->ahParam.startSfbL    = 22;
     hAdjThr->ahParam.startSfbS    = 39;
   }
   else if(bitsPerSample > 1.0f) {
     hAdjThr->ahParam.modifyMinSnr = FALSE;
     hAdjThr->ahParam.startSfbL    = 8;
     hAdjThr->ahParam.startSfbS    = 9;
   }
   else if(bitsPerSample > 0.5f) {
     hAdjThr->ahParam.modifyMinSnr = TRUE;
     hAdjThr->ahParam.startSfbL    = 8;
     hAdjThr->ahParam.startSfbS    = 9;
   }
   else{
     hAdjThr->ahParam.modifyMinSnr = TRUE;
     hAdjThr->ahParam.startSfbL    = 0;
     hAdjThr->ahParam.startSfbS    = 0;
   }

   hAdjThr->desiredTotalBits = 0;
   hAdjThr->desiredFrameBits = 0;
   hAdjThr->usedTotalBits    = 0;
   hAdjThr->peSum            = 0.0f;
   hAdjThr->desiredBitresLevel = 0;
}

static void calcPeCorrection(float *correctionFac,
                             const float peAct,
                             const float peLast, 
                             const int bitsLast) 
{
   if ((bitsLast > 0) && (peAct < 1.5f*peLast) && (peAct > 0.7f*peLast) &&
       (1.2f*bits2pe((float)bitsLast) > peLast) && (0.8f*bits2pe((float)bitsLast) < peLast))
   {
      float newFac = peLast / bits2pe((float)bitsLast);
      if (((newFac > 1.0f) && (*correctionFac < 1.0f)) ||
          ((newFac < 1.0f) && (*correctionFac > 1.0f))) {
         *correctionFac = 1.0f;
      }
      *correctionFac = 0.8f * (*correctionFac) + 0.2f * newFac;
      *correctionFac = min(*correctionFac, 1.15f);
      *correctionFac = max(*correctionFac, 0.85f);
   }
   else {
      *correctionFac = 1.0f;
   }
}

#define Q_SHORT 1.5f    /* additional quality factor for short-blocks */

void PeStatsInit( struct PE_STATS *peStats, float Q, int useLog )
{
  int i;
  for( i=0; i<PE_COUNT; i++ )
    {
      peStats->pe[i] = 0.0f;
      peStats->peShort[i] = 0.0f;
    }
  peStats->count = 0;
  peStats->Q = Q;
  peStats->lastQMax = 0.0f;
  peStats->lastQMin = 0.0f;
  peStats->QMax = 1.0e5f;
  peStats->QMin = 0.0f;
  peStats->totalPe = 0.0f;
  peStats->maxIndex = 0;
  peStats->minIndex = 0;
  peStats->minIndex = 0;
  peStats->QIndex = 0;
  peStats->TANH = !useLog;
}

void PeStatsAdd( struct PE_STATS *peStats, float pe, int windowSequence )
{
  float floatIndex;
  int index;
  float factor;
  float *peTab;
  if ( peStats->TANH )
    peTab = ( windowSequence != SHORT_WINDOW ) ? peStats->pe : peStats->peShort;
  else
    peTab = peStats->pe;

  floatIndex = min( pe, PE_MAX-(PE_MAX/PE_COUNT)-1.0f ) / (PE_MAX/PE_COUNT);
  index = (int)floatIndex;
  factor = floatIndex - (float)index;
  assert( index+1 < PE_COUNT );
  peTab[index]   += 1.0f - factor;
  peTab[index+1] += factor;
  peStats->totalPe += pe;
  peStats->count++;
}

void PeStatsSub( struct PE_STATS *peStats, float pe, int windowSequence )
{
  float floatIndex;
  int index;
  float factor;
  float *peTab;
  if (peStats->TANH)
    peTab = ( windowSequence != SHORT_WINDOW ) ? peStats->pe : peStats->peShort;
  else
    peTab = peStats->pe;

  floatIndex = min( pe, PE_MAX-(PE_MAX/PE_COUNT)-1.0f ) / (PE_MAX/PE_COUNT);
  index = (int)floatIndex;
  factor = floatIndex - (float)index;
  assert( index+1 < PE_COUNT );
  peTab[index]   -= 1.0f - factor;
  peTab[index+1] -= factor;
  peStats->totalPe -= pe;
  peStats->count--;
}

void initPeTab( struct PE_STATS *peStats, float Q )
{
  int i;
  peStats->logPe[0] = 0.0f;
  for( i=1; i<PE_COUNT; i++ )
    {
      if ( peStats->TANH ) {
        peStats->logPe[i] = Q * (float)tanh( (PE_MAX/PE_COUNT) * (float)i / Q );
        peStats->logPeShort[i] = Q_SHORT * Q * (float)tanh( (PE_MAX/PE_COUNT) * (float)i / (Q_SHORT*Q) );
      }
      else{
        peStats->logPe[i] = Q * (float)log( 1.0f + (PE_MAX/PE_COUNT) * (float)i / Q );
      }
    }
}

float reducePe( struct PE_STATS *peStats, float pe, int windowSequence )
{
  float floatIndex = min( pe, PE_MAX-(PE_MAX/PE_COUNT)-1.0f ) / (PE_MAX/PE_COUNT);
  int index = (int)floatIndex;
  float factor = floatIndex - (float)index;
  assert( index+1 < PE_COUNT );
  if (peStats->TANH){
    if( windowSequence == SHORT_WINDOW ){
      return ( ( 1.0f - factor ) * peStats->logPeShort[index]
               +        factor   * peStats->logPeShort[index+1] );
    }
  }
  return ( ( 1.0f - factor ) * peStats->logPe[index]
           +        factor   * peStats->logPe[index+1] );
}

/*
  calculate quality with Newton formula:
  Q(n+1) = Q(n) * ( f(n) / f'(n) )
*/
float adjustQualityCBR( struct PE_STATS *peStats, float constPart, float maxBitresBits, float *QStart )
{
  int i, j=0;
  float pe, logPe, logPeShort;
  float f, f1;
  float Q = *QStart;
  if( peStats->count == 0 )
    return Q;

  /* find best quality by iteration */
  while( 1 )
    {
      f = f1 = 0.0f;
      Q = max( Q, 1.0f );
      for( i=1; i<PE_COUNT; i++ )
	{
	  pe = (PE_MAX/PE_COUNT) * (float)i;
          if (peStats->TANH){
            logPe = (float)tanh( pe/Q );
            logPeShort = (float)tanh( pe/(Q_SHORT*Q) );
            f  -= Q * ( peStats->pe[i] * logPe + Q_SHORT * peStats->peShort[i] * logPeShort );
            f1 -= peStats->pe[i] * ( logPe - (1.0f-logPe*logPe)*pe/Q )
              + peStats->peShort[i] * ( Q_SHORT * logPeShort - (1.0f-logPeShort*logPeShort)*pe/Q );
          }
          else {
            logPe = (float)log( 1.0f + pe/Q );
            f -= peStats->pe[i] * Q * logPe;
            f1 -= peStats->pe[i] * ( logPe - 1.0f / ( Q/pe + 1.0f ) );
          }
	}
      f = f / 1.18f + constPart;

      /* check if result is ok */
      if( fabs( f ) < maxBitresBits/10000.0f )
	break;

      /* calculate new Quality */
      f1 /= 1.18f;
      Q = Q - f / f1;

      if( Q > 30000.0f )
	{
	  Q = 30000.0f;
	  break;
	}

      if( Q < 1.0f && j != 0 )
	return *QStart = 1.0f;

      assert( j < 100 ); /* no convergence ? */

      j++;
    }
  return *QStart = Q;
}

float adjustQualityVBR( struct PE_STATS *peStats, float desiredPe )
{
  int i,j=0;
  float pe, logPe, logPeShort;
  float f, f1;
  float Q = peStats->Q;
  if( peStats->count == 0 )
    return Q;

  /* find best quality by iteration */
  while( 1 )
    {
      f = f1 = 0.0;
      Q = max( Q, 1.0f );
      for( i=1; i<PE_COUNT; i++ )
	{
	  pe = (PE_MAX/PE_COUNT) * (float)i;
          if (peStats->TANH){
            logPe = (float)tanh( pe/Q );
            logPeShort = (float)tanh( pe/(Q_SHORT*Q) );
            f  += Q * ( peStats->pe[i] * logPe + Q_SHORT * peStats->peShort[i] * logPeShort );
            f1 += peStats->pe[i] * ( logPe - (1-logPe*logPe)*pe/Q )
              + peStats->peShort[i] * ( Q_SHORT * logPeShort - (1-logPeShort*logPeShort)*pe/Q );
          }
          else {
            logPe = (float)log( 1.0f + pe/Q );
            f  += peStats->pe[i] * Q * logPe;
            f1 += peStats->pe[i] * ( logPe - 1.0f / ( Q/pe + 1.0f ) );
          }
	}
      f = f / peStats->count - desiredPe;

      /* check if result is ok */
      if( fabs( f ) < desiredPe/1000.0f )
        break;

      /* calculate new Quality */
      f1 /= peStats->count;
      Q = Q - f / f1;

      if( Q > 30000.0f )
	{
	  Q = 30000.0f;
	  break;
	}

      assert( Q >= 1.0f || !j ); /* no convergence, shouldn't happen */
      j++;
    }
  return peStats->Q = Q;
}



/* old threshold-reduction for CBR mode */
void AdjustThresholds(struct ADJ_THR_STATE *hAdjThr,
                      struct PSY_OUT_LIST *qcPsyOut, 
                      float *chBitDistribution,
                      const int nChannels,
                      const int avgBits,
                      const int bitresBits,
                      const int maxBitresBits,
                      const float maxBitFac,
                      const int startCh,
                      const int endCh)
{
   float noRedPe, grantedPe, grantedPeCorr;
   int curWindowSequence;
   float bitFactor;
   int ch;
   struct PSY_OUT *psyOut = qcPsyOut->psyOut;

   noRedPe = qcPsyOut->peData.pe;
#ifdef PLOTMTV
   setDeboutVars(-1,-1,0,-1);
   sendDeboutHist( "pe", "noRedPe", MTV_FLOAT, &(noRedPe) );
#endif
   /* prefer short windows for calculation of bitFactor */
   curWindowSequence = LONG_WINDOW;
   for(ch=startCh;ch<startCh+nChannels;ch++) {
     if (psyOut->psyOutChannel[ch].windowSequence == SHORT_WINDOW) {
       curWindowSequence = SHORT_WINDOW;
     }
   }
   /* factor dependend on current fill level and pe */
   bitFactor = bitresCalcBitFac(bitresBits, maxBitresBits, avgBits, maxBitFac,
                                noRedPe, curWindowSequence, hAdjThr);
#ifdef PLOTMTV
   {
     int grantedBits = bitFactor * avgBits;
     int blockType   = 1000 + 1000 * (psyOut->psyOutChannel[startCh].windowSequence == SHORT_WINDOW);
     setDeboutVars(-1,-1,0,-1);
     sendDeboutHist( "bitres", "bitresBits",  MTV_INT, &bitresBits );
     sendDeboutHist( "bitres", "grantedBits", MTV_INT, &grantedBits );
     sendDeboutHist( "bitres", "avgBits",     MTV_INT, &avgBits );
     sendDeboutHist( "bitres", "blockType",   MTV_INT, &blockType );
   }
#endif
   /* desired pe for actual frame */
   grantedPe = bits2pe(bitFactor * avgBits);
   /* correction of pe value */
   calcPeCorrection(&(hAdjThr->peCorrectionFactor), min(grantedPe, noRedPe),
                    hAdjThr->peLast, hAdjThr->dynBitsLast);
   grantedPeCorr = grantedPe * hAdjThr->peCorrectionFactor;

#ifdef PLOTMTV
   sendDeboutHist( "bits2pe", "peCorrectionFactor",  MTV_FLOAT, &hAdjThr->peCorrectionFactor );
   sendDeboutHist( "pe", "grantedPe", MTV_FLOAT, &(grantedPeCorr) );
#endif

   if (grantedPeCorr < noRedPe) {
      /* calc threshold necessary for desired pe */
      adaptThresholdsToPe(psyOut, &(qcPsyOut->peData),
                          hAdjThr->sfbMinSnrLong, hAdjThr->sfbMinSnrShort,
                          hAdjThr->sfbMinSnr, hAdjThr->ahParam,
                          nChannels, grantedPeCorr, startCh, endCh);
   }
#ifdef PLOTMTV
   else {
     sendDeboutHist( "pe", "redPe_tmp1", MTV_FLOAT, &(noRedPe) );
     sendDeboutHist( "pe", "redPe_tmp2", MTV_FLOAT, &(noRedPe) );
     sendDeboutHist( "pe", "redPe",      MTV_FLOAT, &(noRedPe) );
   }
#endif
   /* calculate relative distribution of bits among the channels */
   for (ch=startCh; ch<endCh; ch++) {
#if 1
     int   minBits   = 60;
     float minBitDis = (float)minBits/(float)(bitFactor * avgBits);
    
     minBitDis = min(minBitDis,0.5f);
     minBitDis = max(minBitDis,0.1f);
     
     chBitDistribution[ch] = qcPsyOut->peData.pe ? minBitDis + (1.0f-nChannels*minBitDis) * 
       (qcPsyOut->peData.peChannelData[ch].pe/qcPsyOut->peData.pe) : 1.0f / nChannels;
#else 
     /* old version: causes endless loop in some intensity cases
        with low bitrate */
     chBitDistribution[ch] = qcPsyOut->peData.pe ? 0.1f + (1.0f-nChannels*0.1f) * 
       (qcPsyOut->peData.peChannelData[ch].pe/qcPsyOut->peData.pe) : 1.0f / nChannels;
#endif
   }

   /* update last pe */
   hAdjThr->peLast = grantedPe;
}


void AdjustThresholdsVBR(struct ADJ_THR_STATE *hAdjThr,
                         /*struct PSY_OUT_LIST *firstPsyOut, */
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
			 struct PE_STATS *peStats )
{
  int ch;
  float corrFactor;
  /*struct PSY_OUT_LIST *tempPsyOut; */
  float redPe;
  float noRedPe          = qcPsyOut->peData.noRedPe;
  float desiredMeanPe    = hAdjThr->desiredMeanPe;
  /*int desiredMeanBitrate = hAdjThr->desiredMeanBitrate; */
  int usedTotalBits      = hAdjThr->usedTotalBits;
  int desiredTotalBits   = hAdjThr->desiredTotalBits;
  float Q;

  /*sendDeboutHist( "pe", "redPe", MTV_FLOAT, &(grantedPe) );*/

  if( hAdjThr->desiredMeanBitrate )     /* mbr mode */
    {
      Q = adjustQualityVBR( peStats, desiredMeanPe );

      if (peStats->TANH){
        if( qcPsyOut->windowSequence != SHORT_WINDOW )
          redPe = Q * (float)tanh( noRedPe / Q );
        else
          redPe = Q_SHORT * Q * (float)tanh( noRedPe / (Q_SHORT*Q) );
      }
      else
	redPe = Q * (float)log( 1.0f + noRedPe / Q );


      hAdjThr->desiredFrameBits += (int) pe2bits( redPe );

      /*
        calculate correction factor
      */
      corrFactor = 0.000002f * (float)( desiredTotalBits - usedTotalBits );
      corrFactor = min(  0.15f, corrFactor );
      corrFactor = max( -0.15f, corrFactor );

      redPe *= 1.0f /*+ corrFactor*/;
      sendDeboutHist( "pe", "Q", MTV_FLOAT, &Q );

      if( granuleCnt >= predictGranules/2 )
        PeStatsSub( peStats, lastPsyOut->peData.noRedPe, lastPsyOut->windowSequence );
    }
  else /* "normal" vbr mode */
    {
      if( vbrFactor >= 9000.0f ) /* threshold simulator */
        redPe = noRedPe;
      else
        {
          if(peStats->TANH){
            if( qcPsyOut->windowSequence != SHORT_WINDOW )
              redPe = vbrFactor * (float)tanh( noRedPe / vbrFactor );
            else
              redPe = Q_SHORT * vbrFactor * (float)tanh( noRedPe / (Q_SHORT*vbrFactor) );
          }
          else{
            if( qcPsyOut->windowSequence != SHORT_WINDOW )
              redPe =  vbrFactor * (float)log( 1.0f + noRedPe / ( vbrFactor) ); 
            else
              redPe =  2 * Q_SHORT *vbrFactor * (float)log( 1.0f + noRedPe / ( 2 * Q_SHORT *vbrFactor) );
          }
        }
    }

  /* limit pe to maximum allowed value */

  redPe = min( redPe, 0.8f * bits2pe((const float)maxBits) );
  //redPe = min( redPe, 0.7 * bits2pe((const float)maxBits) );


  if( redPe < noRedPe )
    {
      /* calc threshold necessary for desired pe */
      adaptThresholdsToPe(qcPsyOut->psyOut, &(qcPsyOut->peData),
                          hAdjThr->sfbMinSnrLong, hAdjThr->sfbMinSnrShort, 
                          hAdjThr->sfbMinSnr, hAdjThr->ahParam,
                          nChannels, redPe, startCh, endCh);
    }

  sendDeboutHist( "pe", "noRedPe", MTV_FLOAT, &noRedPe );
  sendDeboutHist( "pe", "redPe", MTV_FLOAT, &redPe );

  /* calculate relative distribution of bits among the channels */
  for (ch=0; ch<nChannels; ch++)
    {
      chBitDistribution[ch] = qcPsyOut->peData.pe ? 0.1f + (1.0f-nChannels*0.1f) * 
        (qcPsyOut->peData.peChannelData[ch].pe/qcPsyOut->peData.pe) : 1.0f / nChannels;
    }
}

void AdjustThrDualPass( struct ADJ_THR_STATE *hAdjThr,
                        struct PSY_OUT_LIST *qcPsyOut,
                        float *chBitDistribution,
                        const int nChannels,
                        const int maxBits,
                        /*const float vbrFactor,*/
                        const int startCh,
                        const int endCh,
                        const int granuleCnt,
                        struct PE_STATS *peStats )
{
  /*PE_DATA peData;*/
  int   ch;
  float corrFactor;
  /*int   desiredMeanBitrate = hAdjThr->desiredMeanBitrate; */
  float desiredMeanPe      = hAdjThr->desiredMeanPe;
  int   desiredTotalBits   = hAdjThr->desiredTotalBits;
  int   usedTotalBits      = hAdjThr->usedTotalBits;
  float noRedPe            = qcPsyOut->peData.noRedPe;
  float redPe;

  /*
    calculate correction factor
  */
  corrFactor = 0.000002f * (float)( desiredTotalBits - usedTotalBits );
  corrFactor = min(  0.15f, corrFactor );
  corrFactor = max( -0.15f, corrFactor );

  if( !granuleCnt )
    {
      adjustQualityVBR( peStats, desiredMeanPe );
      initPeTab( peStats, peStats->Q );
    }

  redPe = reducePe( peStats, noRedPe, qcPsyOut->windowSequence );
  hAdjThr->desiredFrameBits += (int) pe2bits( redPe );
  redPe *= 1.0f + corrFactor;

  sendDeboutHist( "pe", "noRedPe", MTV_FLOAT, &noRedPe );
  sendDeboutHist( "corrFactor", "corr", MTV_FLOAT, &corrFactor );

  /* limit pe to maximum allowed value */
  redPe = min( redPe, 0.9f * bits2pe((const float)maxBits) );

  if( redPe < noRedPe )
    {
      /* calc threshold necessary for desired pe */
      adaptThresholdsToPe( qcPsyOut->psyOut, &(qcPsyOut->peData),
                           hAdjThr->sfbMinSnrLong, hAdjThr->sfbMinSnrShort, 
                           hAdjThr->sfbMinSnr, hAdjThr->ahParam,
                           nChannels, redPe, startCh, endCh );
    }

  sendDeboutHist( "pe", "redPe", MTV_FLOAT, &(qcPsyOut->peData.pe) );

  /* calculate relative distribution of bits among the channels */
  for (ch=0; ch<nChannels; ch++)
    {
      chBitDistribution[ch] = qcPsyOut->peData.pe ? 0.1f + (1.0f-nChannels*0.1f) * 
        (qcPsyOut->peData.peChannelData[ch].pe/qcPsyOut->peData.pe) : 1.0f / nChannels;
    }
}

void AdjThrUpdate(struct ADJ_THR_STATE *hAdjThr,
                  const int dynBitsUsed)
{
  hAdjThr->dynBitsLast = dynBitsUsed;
}

#endif
