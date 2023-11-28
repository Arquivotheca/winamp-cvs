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
*   $Id: spreading.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <math.h>
#include <stdio.h>
#include "psy_const.h"
#include "spreading.h"
#include "mathmac.h"
#include "mathlib.h"


static const float magic_const = 1e-20f;

static const int MAX_SAMPLES_HIGH = 500;
static const int MAX_SAMPLES_LOW = 150;
static const float MAX_BARC_DIFF_HIGH = 5.0f;
static const float MAX_BARC_DIFF_LOW = -1.5f;
static const float MASK_LOW = 2.0f;
static const float MASK_HIGH = 1.5f;

static const float MAX_BARC_VALUE = 24.0f;

static const float slope = 7.5f;

/* for dummy psych */
static const float maskLow  = 30.0f; /* in dB/bark */
static const float maskHigh = 15.0f; /* in dB/bark */

/*****************************************************************************

    functionname: BarcLineValue
    description:  Calculates barc value for one frequency line
    returns:      barc value of line
    input:        number of lines in transform, index of line to check, Fs
    output:       

*****************************************************************************/
static float BarcLineValue(const int noOfLines, const int lineIndex, const long samplingFreq ) {

  float center_freq, temp, bvalFFTLine;
    
  /*
    center frequency of fft line 
  */
  center_freq = (float) lineIndex * ((float)samplingFreq * 0.5f)/(float)noOfLines; 
  temp = (float) atan(1.3333333e-4f * center_freq);
  bvalFFTLine = (float) (13.3f * atan(0.00076f * center_freq) + 3.5f * temp * temp);
  /*
    Note: this approx seems not to be correct e.g:
    512    noOfLines
    48000  samplingFreq
    ==> bvalLine[511] = 25,78
  */

  return(bvalFFTLine);
}


void InitBarcValues(const int numLines, 
                    const long samplingFrequency, 
                    const int numPb, 
                    const int *pbOffset,
                    int *barcValues,
                    int *barcValScaling,
                    float *pbSpreadNorm,
                    float *pScratchSpreadNorm,
                    const float *spreadingFunction,
                    float *maskLoFactor,
                    float *maskHiFactor) {

  float barcValueFloat, barcValueFloatLast=0.0f;
  int i;

  for(i=0; i<numPb; i++) {
    barcValueFloat = min((BarcLineValue(numLines, pbOffset[i], samplingFrequency) + 
      BarcLineValue(numLines, pbOffset[i+1], samplingFrequency)) * 0.5f, MAX_BARC_VALUE);
    barcValues[i] = (int)((MAX_SAMPLES_HIGH + MAX_SAMPLES_LOW) * 
      barcValueFloat / (MAX_BARC_DIFF_HIGH - MAX_BARC_DIFF_LOW));

    pScratchSpreadNorm[i] = 1.0f;

    /* spreading factors for dummy psych */
    if (i > 0) {
       float dbVal;
       dbVal = maskHigh * (barcValueFloat-barcValueFloatLast);
       maskHiFactor[i] = (float)pow(10.0f, -dbVal/10.0f);
       dbVal = maskLow * (barcValueFloat-barcValueFloatLast);
       maskLoFactor[i-1] = (float)pow(10.0f, -dbVal/10.0f);
    }
    else {
       maskHiFactor[i] = 0.0f;
       maskLoFactor[numPb-1] = 0.0f;
    }

    barcValueFloatLast = barcValueFloat;
  }

  *barcValScaling = (int)((MAX_SAMPLES_HIGH + MAX_SAMPLES_LOW) / (MAX_BARC_DIFF_HIGH - MAX_BARC_DIFF_LOW));
  Spreading (pScratchSpreadNorm, 0, numPb, /*pbOffset,*/ spreadingFunction, barcValues,
             pbSpreadNorm, 0);
  for(i=0; i<numPb; i++) {
     pbSpreadNorm[i] = 1.0f / pbSpreadNorm[i];
  }
}


/*****************************************************************************

  functionname: InitSpreadingFunction 
  description:  allocates memory fro spreading function and initializes array
  returns:      ptr to spreading function
  input:        
  output:       
  
*****************************************************************************/
float *InitSpreadingFunction() {
  int i;
  float mask, adj_mask, diff, diffIncrement;
  float sprdval, temp;
  float *spreadingFunction;
  /*FILE *f;*/
  
  /*f=fopen("spread.txt","w");*/
  /* allocate memory for spreading function */
  spreadingFunction = (float *) mp3Alloc(sizeof(float) * (MAX_SAMPLES_HIGH + MAX_SAMPLES_LOW + 1));

  /* compute spreading function */
  diff = MAX_BARC_DIFF_LOW;
  diffIncrement = (MAX_BARC_DIFF_HIGH - MAX_BARC_DIFF_LOW) / (float)(MAX_SAMPLES_HIGH + MAX_SAMPLES_LOW);
  i = 0;
  while(diff < MAX_BARC_DIFF_HIGH) {
    if(diff >= 0.0f) {
      mask = diff * MASK_HIGH;
    } else {
      mask = diff * MASK_LOW;
    }

    if(mask >= .5f && mask <= 2.5f) { 
      temp = mask - 0.5f; 
      temp = 8.0f*(temp*temp - 2*temp); 
    } else {
      temp = 0.0f;
    }
    
    adj_mask = mask + .474f;
    sprdval  = 15.811f + slope*adj_mask;
    sprdval  = sprdval - 17.5f*(float)sqrt(1.0 + adj_mask * adj_mask);
    if(sprdval <= -100.0f) {
      spreadingFunction[i] = 0.0f;
    } else {
      spreadingFunction[i] = (float)pow(10.0f, 0.1f*(temp+sprdval));        
    }
    /*fprintf(f,"%f\n",spreadingFunction[i]);*/
    i++;
    diff += diffIncrement;
  }
  /*fclose(f);*/
  return(spreadingFunction);
}

/*****************************************************************************

  functionname: FreeSpreadingFunction 
  description:  frees up memory allocated for spreading function
  returns:
  input:        ptr to spreading function
  output:       
  
*****************************************************************************/
void FreeSpreadingFunction(float *spreadingFunction) 
{
  if(spreadingFunction != 0)
  {
    mp3Free(spreadingFunction);
  }
}


/*****************************************************************************

  functionname: Spreading 
  description:  spreads out energy and tonality weighted energy according to 
  pre computed spreading function
  returns:
  input:        ptr to pb wise energies, ptr to pb wise chaos measure, number
  of pb, ptr to spreading function
  output:       spreaded energies, spreaded tonality (chaos measure)
  
*****************************************************************************/
void Spreading (const float *pbEnergy,
                const float *pbWeightedChaosMeasure,
                const int    pbCnt,
                /*const int   *pbOffset,*/
                const float *spreadingFunction,
                const int   *barcValues,
                float       *pbSpreadedEnergy,
                float       *pbSpreadedChaosMeasure)
{
  int i, j, diff;
  float accuSpreadEnergy, accuSpreadCm;
  float spreadingCoefficient;  
  int barcValueCurrentBand;
  
  if (pbWeightedChaosMeasure != 0) {
    for (i=0; i<pbCnt; i++) {
      accuSpreadEnergy = magic_const;
      accuSpreadCm = 0.0f;

      barcValueCurrentBand = barcValues[i];

      diff = 0;
      j = i;
      while((j >= 0) && (diff < MAX_SAMPLES_HIGH)) {
        spreadingCoefficient = spreadingFunction[MAX_SAMPLES_LOW + diff];
        accuSpreadEnergy += pbEnergy[j] * spreadingCoefficient;
        accuSpreadCm += pbWeightedChaosMeasure[j] * spreadingCoefficient;
        j--;
        diff = barcValueCurrentBand - barcValues[j];
      }
      j = i + 1;
      diff = barcValueCurrentBand - barcValues[j];
      while((j < pbCnt) && (diff >  -MAX_SAMPLES_LOW)) {
        spreadingCoefficient = spreadingFunction[MAX_SAMPLES_LOW + diff];
        accuSpreadEnergy += pbEnergy[j] * spreadingCoefficient;
        accuSpreadCm += pbWeightedChaosMeasure[j] * spreadingCoefficient;
        j++;
        diff = barcValueCurrentBand - barcValues[j];
      }

      pbSpreadedEnergy[i] = accuSpreadEnergy;
      pbSpreadedChaosMeasure[i] = accuSpreadCm;
    }
  } else {			/* use this to only spread the energy */
    for (i = 0; i < pbCnt; i++) {
      accuSpreadEnergy = magic_const;
      
      barcValueCurrentBand = barcValues[i];

      diff = 0;
      j = i;
      while((j >= 0) && (diff < MAX_SAMPLES_HIGH)) {
        spreadingCoefficient = spreadingFunction[MAX_SAMPLES_LOW + diff];
        /*spreadingCoefficient = spreadValue( i, j, pbOffset );*/
        accuSpreadEnergy += pbEnergy[j] * spreadingCoefficient;
        j--;
        diff = barcValueCurrentBand - barcValues[j];
      }
      j = i + 1;
      diff = barcValueCurrentBand - barcValues[j];
      while((j < pbCnt) && (diff >  -MAX_SAMPLES_LOW)) {
        spreadingCoefficient = spreadingFunction[MAX_SAMPLES_LOW + diff];
        /*spreadingCoefficient = spreadValue( i, j, pbOffset );*/
        accuSpreadEnergy += pbEnergy[j] * spreadingCoefficient;
        j++;
        diff = barcValueCurrentBand - barcValues[j];
      }

      pbSpreadedEnergy[i] = accuSpreadEnergy;
    }
  }
}

/* for dummy psych */
void SpreadingMax(const int    pbCnt,
                  const float *maskLowFactor,
                  const float *maskHighFactor,
                  float       *pbSpreadedEnergy)
{
   int i;

   /* slope to higher frequencies */
   for (i=1; i<pbCnt; i++) {
      pbSpreadedEnergy[i] = max(pbSpreadedEnergy[i],
                                maskHighFactor[i] * pbSpreadedEnergy[i-1]);
   }
   /* slope to lower frequencies */
   for (i=pbCnt-2; i>=0; i--) {
      pbSpreadedEnergy[i] = max(pbSpreadedEnergy[i],
                                maskLowFactor[i] * pbSpreadedEnergy[i+1]);
   }

   /* this should replace the use of the minimum in adapt pb to sfb */
   for (i=1; i<pbCnt-1; i++) {
      float en1, en2;
      en1 = max(pbSpreadedEnergy[i-1], pbSpreadedEnergy[i+1]);
      en2 = 0.2f * pbSpreadedEnergy[i];
      if (en1 < pbSpreadedEnergy[i]) {
         pbSpreadedEnergy[i] = max(en1, en2);
      }
   }

}
