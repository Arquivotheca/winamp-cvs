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
*   $Id: sf_estim.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>

#include "mconfig.h"
#include "sf_estim.h"
#include "sf_cmprs.h"
#include "interface.h"
#include "mathmac.h"
#include "mathlib.h"
#include "utillib.h"
#include "mp3alloc.h"

static const float C0 =   4.00005f;    /* -16/3*log(1-0.5-logCon)/log(2) */
static const float C1 = -69.33295f; /* -16/3*log(MAX_QUANT+1-0.5-logCon)/log(2) */
static const float C2 =   5.77078f; /* 4/log(2) */

/* if the following is defined, sqrt() is calculated via Newton-Raphson */
#define USE_FAST_SQRT

/* if the following is defined, log2() is only estimated */
#ifndef macintosh
#define USE_FAST_LOG2
#endif

#define alpha 0.75f          /* non-linear quantizer exponent */

static void findscfMinMax(const int *scf, int sfbCnt, int scan,
                          int *scfMin, int *scfMax);

#ifndef P4_CODE
void CalcFormFactorChannel_NoOpt (float * restrict sfbFormFactor,
				  float * restrict sfbMaxSpec,
				  const struct PSY_OUT_CHANNEL *psyOutChan)
{
  int i, j;

  /* calc sum of sqrt(spec) */

  for (i=0; i<psyOutChan->sfbActive; i++)
  {
#ifndef USE_FAST_SQRT
	float maxSpec, ep1, ep2 ;
    maxSpec = ep1 = ep2 = 0.0f;

    for(j = psyOutChan->sfbOffsets[i]; j < psyOutChan->sfbOffsets[i+1]; j+=2)
    {
      ep1 += (float)sqrt( fabs(psyOutChan->mdctSpectrum[j]) );
      if ((float)fabs(psyOutChan->mdctSpectrum[j]) > maxSpec)
        maxSpec = (float)fabs(psyOutChan->mdctSpectrum[j]);

      ep2 += (float)sqrt( fabs(psyOutChan->mdctSpectrum[j+1]) );
      if ((float)fabs(psyOutChan->mdctSpectrum[j+1]) > maxSpec)
        maxSpec = (float)fabs(psyOutChan->mdctSpectrum[j+1]);
    }
#else
    float maxSpec, ep1, ep2 ;
    ieee_pattern iMaxSpec;
    iMaxSpec.i = INT_MIN;
    ep1 = ep2 = 0.0f;

    for(j = psyOutChan->sfbOffsets[i]; j < psyOutChan->sfbOffsets[i+1]; j+=2)
    {
      ieee_pattern t[2];
      ieee_pattern y[2];
      float y0f, y1f;

      t[0].i = rsqrt_seed((ieee_pattern*)&psyOutChan->mdctSpectrum[j  ],
                          (ieee_pattern*)&y[0] );
      t[1].i = rsqrt_seed((ieee_pattern*)&psyOutChan->mdctSpectrum[j+1],
                          (ieee_pattern*)&y[1] );

      if (y[0].i > iMaxSpec.i) iMaxSpec.i = y[0].i;
      if (y[1].i > iMaxSpec.i) iMaxSpec.i = y[1].i;

      y0f = y[0].f;
      y1f = y[1].f;

      t[0].f = 0.5f*t[0].f*(3.0f-t[0].f*t[0].f*y0f);
      t[1].f = 0.5f*t[1].f*(3.0f-t[1].f*t[1].f*y1f);

      t[0].f = 0.5f*t[0].f*(3.0f-t[0].f*t[0].f*y0f);
      t[1].f = 0.5f*t[1].f*(3.0f-t[1].f*t[1].f*y1f);

      ep1 += y0f*t[0].f;
      ep2 += y1f*t[1].f;
    }
    maxSpec = iMaxSpec.f;
#endif
    sfbFormFactor[i] = 1e-10f + ep1 + ep2;
    sfbMaxSpec[i] = maxSpec;
  }
}
#endif

#ifndef P4_CODE
void (*CalcFormFactorChannel) (float * restrict sfbFormFactor,
                               float * restrict sfbMaxSpec,
                               const struct PSY_OUT_CHANNEL *psyOutChan) = CalcFormFactorChannel_NoOpt;
#endif


/*
  estimate scalefactors. These are negative numbers, ranging
  from 0 to -somewhere.
 */
void
EstimateScaleFactorsChannel(const struct PSY_OUT_CHANNEL *psyOutChan,
                            int   *scf,
                            float *sfbFormFactor,
                            float *sfbMaxSpec)
{
  int   i;

  sendDebout("estScalInp",psyOutChan->sfbActive,1,"thres",MTV_FLOAT,psyOutChan->sfbThreshold);
  sendDebout("estScalInp",psyOutChan->sfbActive,1,"ener",MTV_FLOAT,psyOutChan->sfbEnergy);

  /* by going up to sfbActive, we will estimate scalefactors even for the right
     channel where intensity may be active. These scalefactors will be
     disregarded later. */

  for(i=0; i<psyOutChan->sfbActive; i++)
  {
    float threshold  = psyOutChan->sfbThreshold[i];
    float energy     = psyOutChan->sfbEnergy[i];
    float scfFloat;
    int   scfInt;
    int   scfMin;

    /* calc estimation formula */
    /*
      influence of allowed distortion
     */

#ifndef USE_FAST_LOG2
    scfFloat = 2.0f*(float)log(12.0f*alpha*alpha*threshold / sfbFormFactor[i]) /
                  (alpha*M_LN2);
#else
    scfFloat = 2.0f/alpha*(float)fastlog2(12.0f*alpha*alpha * threshold / sfbFormFactor[i]) ;
#endif

    scfInt = (int)floor(scfFloat + 0.5f);          /* integer scalefactor */

    /*
      avoid quantized values bigger than MAX_QUANT
     */
    if (sfbMaxSpec[i] > 0.0f)
    {
#ifndef USE_FAST_LOG2
      scfMin = (int)ceil(C1 + C2*log(sfbMaxSpec[i]));
#else
      scfMin = (int)ceil(C1 + C2*M_LN2*fastlog2(sfbMaxSpec[i]));
#endif
      scfInt = max( scfInt, scfMin );
    }

    /*
      in all bands where threshold > energy, mark the corresponding scalefactors as DONT_CARE
    */
    scf[i] = (energy > 0.0f  && energy > threshold) ?
      -scfInt : SCF_DONT_CARE;
  }

  /* set scfs for bands above lowpass to DONT_CARE */
  for (; i < psyOutChan->sfbCnt; i++)
    scf[i] = SCF_DONT_CARE ;

  sendDebout("estScalFac",psyOutChan->sfbCnt,1,"scaleFac",MTV_INT,scf);
}

ALIGN_16_BYTE static const int preEmphasisTab[] =
{0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};

void
AdaptScfToMp3Channel(/*const struct PSY_OUT_CHANNEL *psyOutChan,*/
                     const int     sfbActive,
                     const int     blockType,
                     /*const int     mpegVersion,*/
                     const int     fullPsych,
                     int           scf[MAX_GROUPED_SFB],
                     int          *globalGain,
                     int           subBlockGain[TRANS_FAC],
                     unsigned int *scfScale,
                     /*unsigned int *scfCompress,*/
                     const int     scfCntPerPartition[SCF_PARTITIONS],
                     const int     scfBitsPerPartition[SCF_PARTITIONS],
                     const int     allowPreEmphasis,
                     int          *preEmphasisFlag,
                     int          *maxScf,
                     float        *sfbMaxSpec)
{
  int i;
  int sfb;
  int block;
  int offset;
  int sfbCnt, sfbCntAll;

  int sfMin0;
  int sfMax0;
  ALIGN_16_BYTE int sfMax[SCF_PARTITIONS];
  ALIGN_16_BYTE int scfTmp[MAX_GROUPED_SFB]; /* temporary work array */

  int continueFlag;

  int deltaSf;
  int meanScf = 0;
  int scfActive = 0;
  int globalGainTmp;
  int globalGainDiff = 0;

  /* at this point, scalefactors are in the range from 0 to -somewhere. Larger scalefactors
     mean larger multiplication factors in the encoder, meaning lower stepsize */

  if (blockType != SHORT_WINDOW)
  {
    /* ignore sfb 22 */
    sfbCnt = min(sfbActive, MAX_SFB_LONG-1);
    sfbCntAll = MAX_SFB_LONG;
  }
  else
  {
    /* ignore sfb 22 */
    sfbCnt = min(sfbActive, TRANS_FAC * (MAX_SFB_SHORT-1));
    sfbCntAll = TRANS_FAC * MAX_SFB_SHORT;
  }

  sendDebout("scf", sfbCntAll, 1, "0", MTV_INT, scf );

  /* calculate mean value of all scalefactors */
  for( i=0; i < sfbCnt; i++ )
    if( scf[i] != SCF_DONT_CARE )
      {
        meanScf += scf[i];
        scfActive++;
      }
  if( scfActive )
    meanScf /= scfActive;

  for ( *scfScale = 0; *scfScale < 2; (*scfScale)++)
  {
    int nScf;
    int nDist;

    /* copy scalefactors to tmp array. */
    copyINT(scf, scfTmp, sfbCnt);

    /* set scfs for bands above lowpass to DONT_CARE */
    for (i=sfbCnt; i < sfbCntAll; i++)
       scfTmp[i] = SCF_DONT_CARE ;

    /*
      find the total range of scalefactors
     */
    findscfMinMax(scfTmp,sfbCnt,1,&sfMin0,&sfMax0);

    /*
      we would usually put the smallest scalefactor into global gain. However, if
      truncating the scalefactor resolution will throw away low-order bits for more
      than half of the scalefactors, we should use a larger globalGain instead.
     */

    *globalGain = (sfMin0 < INT_MAX) ? -sfMin0 : 0; /* first try */
    if ( fullPsych ){
      globalGainDiff = *globalGain ;
      /*
        we take care of the last scalefactor that is always 0 for mp3 and we try to
        adjust globalGain a bit for it
      */
      if( blockType != SHORT_WINDOW )
        {
          if( scf[sfbCntAll-1] != SCF_DONT_CARE )
            {
              globalGainTmp = min( *globalGain,
                                   -min( scf[sfbCntAll-1], (int)meanScf+2 ) );
              if( globalGainTmp <= (int)ceil( C0 + C2*log(sfbMaxSpec[sfbCntAll-1]) ) )
                *globalGain = globalGainTmp;
              else
                scf[sfbCntAll-1] = SCF_DONT_CARE;
            }
        }
      
      /* this formula trys to save bits for the coding of scalefactors
         by using a bigger globalGain */
      if( sfMin0 < INT_MAX )
        *globalGain = min( *globalGain, -(int)(0.2f*meanScf+0.8f*sfMin0) );
      
      globalGainDiff -= *globalGain ;
#if 1 /* Obiges funktioniert nur mit 112kBps; Ausnullen zur Bitidentitaet */
      if ( globalGainDiff != 0 ){
        for(sfb = 0; sfb < sfbCnt; sfb++)
          scfTmp[sfb] -= globalGainDiff ;
      }
#endif
    }

    nScf = nDist = 0;
    for(sfb = 0; sfb < sfbCnt; sfb++)
    {
      if (scfTmp[sfb] != SCF_DONT_CARE)
      {
        float qScf = (float)(scfTmp[sfb] + *globalGain) / (1<<(1 + *scfScale));
        nScf++;

        if (qScf-floor(qScf) >= 0.5f)
        {
          nDist++;
        }
      }
    }
    if (nDist >= nScf / 2)
    {
      *globalGain += (1 << *scfScale);
    }

    /*
      adjust scalefactors by globalgain
     */

    for(sfb = 0; sfb < sfbCnt; sfb++)
    {
      if (scfTmp[sfb] != SCF_DONT_CARE)
      {
        scfTmp[sfb] += *globalGain;
        scfTmp[sfb] = max( scfTmp[sfb], 0 );
      }
    }

    if (blockType != SHORT_WINDOW)
    {
      /*
        LONG BLOCK.
        check if we can apply preemphasis
       */

      if (allowPreEmphasis)
      {
        for (sfb = 0; sfb < sfbCnt; sfb++)
        {
          if (scfTmp[sfb] != SCF_DONT_CARE && scfTmp[sfb] < (preEmphasisTab[sfb]<<(1+*scfScale))) break;
        }
        *preEmphasisFlag = (sfb == sfbCnt);
      }
      else
      {
        *preEmphasisFlag = 0;
      }

      if (*preEmphasisFlag)
      {
        for (sfb = 0; sfb < sfbCnt; sfb++)
        {
          if (scfTmp[sfb] != SCF_DONT_CARE)
            scfTmp[sfb] -= (preEmphasisTab[sfb]<<(1+*scfScale));
        }
      }
      for (block = 0; block < TRANS_FAC; block++)
      {
        subBlockGain[block] = 0; /* no sub block gains */
      }
    }
    else
    {
      /*
        SHORT BLOCK.
        "decide" on subblock gains.
       */

      for (block = 0; block < TRANS_FAC; block++)
      {
        findscfMinMax(scfTmp+block, sfbCnt, TRANS_FAC, &sfMin0, &sfMax0);
        if (sfMin0 == INT_MAX)
        {
          sfMin0 = 0; /* if there is no energy in this subwindow */
        }
        subBlockGain[block] = sfMin0 / 8;
        if(subBlockGain[block] >= 8) {
#ifdef DEBUGPRINT
          fprintf(stderr, "\n!!subblock %d: limiting subblockgain from %d to 7 sfMin0 was: %d\n", block, subBlockGain[block], sfMin0);
#endif
          subBlockGain[block] = 7;
        }
        /*
          adjust scalefactors by subblock gains
         */
        for ( i = block; i < sfbCnt; i += TRANS_FAC )
        {
          if (scfTmp[i] != SCF_DONT_CARE) scfTmp[i] -= 8 * subBlockGain[block];
        }
      }
      *preEmphasisFlag = 0; /* no preemphasis in short blocks */
    }

    /*
      adjust scaleFactors by scfScale.
     */
    for(sfb = 0; sfb < sfbCnt; sfb++)
    {
      if (scfTmp[sfb] != SCF_DONT_CARE)
      {
        /* downscale scalefactors with rounding */
        scfTmp[sfb] = (scfTmp[sfb]+(1 << *scfScale)) >> (1+*scfScale);
      }
    }

    if (*scfScale == 1)
      break; /* we are done, no coarser scalefactor resolution possible */

    /*
      check if the scalefactors can be coded at all.
     */

    offset = 0;
    continueFlag = 0;
    for ( i = 0; i < SCF_PARTITIONS; i++ )
    {
      findscfMinMax(scfTmp+offset,scfCntPerPartition[i],1,&sfMin0,&sfMax[i]);
      if (sfMax[i] >= (1 << scfBitsPerPartition[i]))
         continueFlag = 1; /* can not be coded, try coarser scf scale */
      offset += scfCntPerPartition[i];
    }
    if (!continueFlag)
       break;
  }
  /* copy scalefactors from tmp array. */
  copyINT(scfTmp, scf, sfbCnt);

  /*
    limit scaleFactors to available range.
   */
  deltaSf = 0;
  offset = 0;
  for ( i = 0; i < SCF_PARTITIONS; i++ )
  {
    int maxScfHere = (1 << scfBitsPerPartition[i]) - 1;
    for(sfb = offset; sfb < offset+scfCntPerPartition[i]; sfb++)
    {
      if ( fullPsych )
        maxScf[sfb] = maxScfHere;

      if (scf[sfb] != SCF_DONT_CARE && scf[sfb] > maxScfHere)
        deltaSf = max(deltaSf, scf[sfb]-maxScfHere);
    }
    offset += scfCntPerPartition[i];
  }
  for (sfb=0; sfb<sfbCnt; sfb++) {
     if (scf[sfb] != SCF_DONT_CARE)
        scf[sfb] = max(0, scf[sfb]-deltaSf);
  }
  *globalGain -= (1<<(1+*scfScale)) * deltaSf;
}

static void findscfMinMax(const int *scf, int sfbCnt, int scan,
                          int *scfMin, int *scfMax)
{
  int i;
  *scfMin = INT_MAX;
  *scfMax = INT_MIN;

  for (i = 0; i < sfbCnt; i+=scan)
  {
    if (scf[i] != SCF_DONT_CARE)
    {
      if (scf[i] > *scfMax) *scfMax = scf[i];
      if (scf[i] < *scfMin) *scfMin = scf[i];
    }
  }
}
