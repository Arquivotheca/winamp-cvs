/*
  Envelope calculation
*/
#include <assert.h>
#include "env_calc.h"
#include "sbr_const.h"
#include "freq_sca.h"
#include "env_extr.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "math/FloatFR.h"
#include <math.h>
#include "math/counters.h" /* the 3GPP instrumenting tool */
#define EPS           1e-12
static void updateFiltBuffer(float *filtBuffer,
                             float *filtBufferNoise,
                             float *nrgGain,
                             float *noiseLevel,
                             int    noSubbands);
static void calcNrgPerSubband(float  **analysBufferReal,
#ifndef LP_SBR_ONLY
                              float  **analysBufferImag,
#endif
                              int    lowSubband, int highSubband,
                              int    start_pos,  int next_pos,
                              float  *nrgEst,
                              int    bUseLP);
static void calcNrgPerSfb(float  **analysBufferReal,
#ifndef LP_SBR_ONLY
                          float  **analysBufferImag,
#endif
                          int    nSfb,
                          unsigned char *freqBandTable,
                          int    start_pos,  int next_pos,
                          float  *nrg_est,
                          int    bUseLP);
static void calcSubbandGain(float nrgRef,
                            float nrgEst,
                            float tmpNoise,
                            float *ptrNrgGain,
                            float *ptrNoiseLevel,
                            float *ptrNrgSine,
                            char   sinePresentFlag,
                            char   sineMapped,
                            int    noNoiseFlag);
static void calcAvgGain(float  *nrgRef,
                        float  *nrgEst,
                        int     lowSubband,
                        int     highSubband,
                        float  *sumRef,
                        float  *ptrAvgGain);
static void adjustTimeSlotLP(float  *ptrReal,
                             float  *gain,
                             float  *noiseLevel,
                             float  *sineLevel,
                             char   *ptrHarmIndex,
                             int    lowSubbands,
                             int    noSubbands,
                             int    noNoiseFlag,
                             int    *ptrPhaseIndex);
#ifndef LP_SBR_ONLY
static void adjustTimeSlot(float  *ptrReal,
                           float  *ptrImag,
                           float  *filtBuffer,
                           float  *filtBufferNoise,
                           float  *gain,
                           float  *noiseLevel,
                           float  *sineLevel,
                           char   *ptrHarmIndex,
                           int    lowSubbands,
                           int    noSubbands,
                           float  smooth_ratio,
                           int    noNoiseFlag,
                           int    *ptrPhaseIndex);
#endif
static void mapSineFlags(unsigned char *freqBandTable,
                         int nSfb,
                         char *addHarmonics,
                         int *harmFlagsPrev,
                         int tranEnv,
                         char *sineMapped)
{
  unsigned int mask;
  int qmfband2, li, ui, i;
  int lowSubband2 = 2 * freqBandTable[0];
  int bitcount = 0;
  int oldflags = 0;
  int newflags = 0;
  
    
   /* pointers for sineMapped[]
                */
  
  for (i=0; i<MAX_FREQ_COEFFS; i++) {
    
    sineMapped[i] = MAX_ENVELOPES;
  }
   /* pointers for freqBandTable[]
                */
  
  for (i=nSfb-1; i>=0; i--) {
    
    li = freqBandTable[i];
    ui = freqBandTable[i+1];
    
    mask = 1 << bitcount;
    
    if (bitcount == 0) {
      
      oldflags = *harmFlagsPrev;
      newflags = 0;
    }
    
    if ( addHarmonics[i] ) {
      
      newflags |= mask;
      
      qmfband2 = ui+li-lowSubband2;
         
      sineMapped[qmfband2 >> 1] = ( oldflags & mask ) ? 0 : tranEnv;
    }
      
    if ((++bitcount == 16) || i==0) {
      
      bitcount = 0;
      
      *harmFlagsPrev++ = newflags;
    }
  }
  
}
/*!
  \brief     Reduce gain-adjustment induced aliasing for real valued filterbank.
*/
static void
aliasingReduction(float*  degreeAlias,
                  float*  nrgGain,
                  float*  nrgEst,
                  int*    useAliasReduction,
                  int noSubbands)
{
  int group, grouping = 0, index = 0, noGroups, k;
  int groupVector[MAX_FREQ_COEFFS];
  
   
   /* pointers for degreeAlias[],
                               useAliasReduction[],
                               groupVector[]
                */
  
  for (k = 0; k < noSubbands-1; k++ ){
     
    if ( (degreeAlias[k + 1] != 0) && useAliasReduction[k] ) {
      
      if(grouping==0){
        
        groupVector[index] = k;
        grouping = 1;
        index++;
      }
      else{
         
        if(groupVector[index-1] + 3 == k){
           
          groupVector[index] = k + 1;
          
          grouping = 0;
          index++;
        }
      }
    }
    else{
      
      if(grouping){
        
        if(useAliasReduction[k]) {
           
          groupVector[index] = k + 1;
        }
        else {
          
          groupVector[index] = k;
        }
        
        grouping = 0;
        index++;
      }
    }
  }
  
  if(grouping){
    
    groupVector[index] = noSubbands;
    index++;
  }
  
  noGroups = index >> 1;
   /* pointers for groupVector[]
                */
  
  for (group = 0; group < noGroups; group ++) {
    float  nrgOrig = 0.0f;
    float  nrgAmp = 0.0f;
    float  nrgMod = 0.0f;
    float  groupGain;
    float  compensation;
    int startGroup = groupVector[2*group];
    int stopGroup  = groupVector[2*group+1];
     
     /* pointers for nrgEst[],
                                 nrgGain[]
                 */
    
    for(k = startGroup; k < stopGroup; k++){
      float tmp;
      
      tmp = nrgEst[k];
      
      nrgOrig += tmp;
      
      nrgAmp += tmp * nrgGain[k];
    }
     DIV(1);
    groupGain = nrgAmp / (nrgOrig + (float)EPS);
     /* pointers for degreeAlias[],
                                 nrgGain[],
                                 nrgEst[]
                 */
    
    for(k = startGroup; k < stopGroup; k++){
      float alpha;
      
      alpha = degreeAlias[k];
       
      if (k < noSubbands - 1) {
         
        if (degreeAlias[k + 1] > alpha) {
          
          alpha = degreeAlias[k + 1];
        }
      }
         
      nrgGain[k] = alpha*groupGain + (1.0f-alpha)*nrgGain[k];
      
      nrgMod += nrgGain[k] * nrgEst[k];
    }
      DIV(1);
    compensation = nrgAmp / (nrgMod + (float)EPS);
     /* pointers for nrgGain[k]
                  */
    
    for(k = startGroup; k < stopGroup; k++){
       
      nrgGain[k] = nrgGain[k] * compensation;
    }
  }
  
}
/*!
  \brief  Apply spectral envelope to subband samples
*/
void
calculateSbrEnvelope (HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env,
                      HANDLE_SBR_HEADER_DATA hHeaderData,
                      HANDLE_SBR_FRAME_DATA  hFrameData,
                      float **analysBufferReal_m,
#ifndef LP_SBR_ONLY
                      float **analysBufferImag_m,
#endif
                      float *degreeAlias,
                      int   bUseLP
                      )
{
  int c, cc, li, ui, i, j, k, l, m, envNoise;
  unsigned char start_pos, stop_pos, freq_res;
  unsigned char nEnvelopes;
  unsigned char*   borders;
  float *noiseLevels;
  HANDLE_FREQ_BAND_DATA hFreq;
  int noNoiseFlag;
  int smooth_length;
  const unsigned char *nSfb;
  int timeStep;
  int lowSubband;
  int highSubband;
  int noSubbands;
  int    noNoiseBands;
  float  nrgRef[MAX_FREQ_COEFFS];
  float  nrgEst[MAX_FREQ_COEFFS];
  float  noiseLevel[MAX_FREQ_COEFFS];
  float  nrgGain[MAX_FREQ_COEFFS];
  float  nrgSine[MAX_FREQ_COEFFS];
  char   sineMapped[MAX_FREQ_COEFFS];
  int useAliasReduction[64];
  float  limiterGain;
  
  
  m = 0, envNoise = 0;
   
  nEnvelopes = hFrameData->frameInfo.nEnvelopes;
  borders = hFrameData->frameInfo.borders;
   
  noiseLevels = hFrameData->sbrNoiseFloorLevel;
  hFreq = &hHeaderData->FreqBandData;
   
  nSfb = hFreq->nSfb;
   
  timeStep = hHeaderData->timeStep;
  lowSubband = hFreq->lowSubband;
  highSubband = hFreq->highSubband;
  
  noSubbands = highSubband - lowSubband;
   
  noNoiseBands = hFreq->nNfb;
   
  limiterGain = sbr_limGains[hHeaderData->limiterGains];
   
  mapSineFlags(hFreq->freqBandTable[HI],
               hFreq->nSfb[HI],
               hFrameData->addHarmonics,
               h_sbr_cal_env->harmFlagsPrev,
               hFrameData->frameInfo.tranEnv,
               sineMapped);
   /* pointers for hFrameData->frameInfo.freqRes[],
                                hFrameData->frameInfo.bordersNoise[],
                                hFrameData->frameInfo.tranEnv,
                                h_sbr_cal_env->prevTranEnv,
                                hHeaderData->smoothingLength,
                                hHeaderData->interpolFreq,
                                hFreq->freqBandTableNoise[1],
                                useAliasReduction[],
                                hFreq->noLimiterBands,
                                h_sbr_cal_env->startUp,
                                h_sbr_cal_env->filtBuffer,
                                h_sbr_cal_env->filtBufferNoise
                */
  
  for (i = 0; i < nEnvelopes; i++) {
    
    start_pos = timeStep * borders[i];
    stop_pos = timeStep * borders[i+1];
    
    freq_res = hFrameData->frameInfo.freqRes[i];
     /* pointers for nSfb[freq_res],
                                 hFreq->freqBandTable[freq_res]
                 */
     
    if (borders[i] == hFrameData->frameInfo.bordersNoise[envNoise+1]){
      
      noiseLevels += noNoiseBands;
      envNoise++;
    }
      
    if(i==hFrameData->frameInfo.tranEnv || i==h_sbr_cal_env->prevTranEnv) {
      
      noNoiseFlag = 1;
      smooth_length = 0;
    }
    else {
      
      noNoiseFlag = 0;
       
      smooth_length = (1 - hHeaderData->smoothingLength) << 2;
    }
    
    if (hHeaderData->interpolFreq) {
      
      calcNrgPerSubband(analysBufferReal_m,
#ifndef LP_SBR_ONLY
                        analysBufferImag_m,
#endif
                        lowSubband, highSubband,
                        start_pos, stop_pos,
                        nrgEst,
                        bUseLP);
    }
    else {
      
      calcNrgPerSfb(analysBufferReal_m,
#ifndef LP_SBR_ONLY
                    analysBufferImag_m,
#endif
                    nSfb[freq_res],
                    hFreq->freqBandTable[freq_res],
                    start_pos, stop_pos,
                    nrgEst,
                    bUseLP);
    }
    {
      unsigned char * table;
      unsigned char uiNoise;
      unsigned char noiseBandIndex;
      
      table = hFreq->freqBandTable[freq_res];
      
      uiNoise = hFreq->freqBandTableNoise[1];
      noiseBandIndex = 0;
      
      c = 0, cc = 0;
       /* pointer for table[] */
      
      for (j = 0; j < nSfb[freq_res]; j++) {
        char sinePresentFlag;
        
        li = table[j];
        ui = table[j+1];
        
        sinePresentFlag = 0;
         /* pointer for sineMapped[] */
        
        for (k=li; k<ui; k++) {
           
          if(i >= sineMapped[cc]) {
            
            sinePresentFlag = 1;
          }
          
          cc++;
        }
         /* pointers for hFreq->freqBandTableNoise[],
                                     useAliasReduction[],
                                     hFrameData->iEnvelope[],
                                     nrgRef[],
                                     nrgSine[],
                                     nrgEst[],
                                     nrgGain[],
                                     noiseLevel[]
                     */
        
        for (k=li; k<ui; k++) {
           
          if(k >= uiNoise){
            
            noiseBandIndex++;
            
            uiNoise = hFreq->freqBandTableNoise[noiseBandIndex+1];
          }
          
          if (bUseLP) {
             
            if (sinePresentFlag)
              useAliasReduction[k-lowSubband] = 0;
            else
              useAliasReduction[k-lowSubband] = 1;
          }
          
          nrgRef[c] = hFrameData->iEnvelope[m];
          
          nrgSine[c] = 0;
          {
             
            calcSubbandGain(nrgRef[c],
                            nrgEst[c],
                            noiseLevels[noiseBandIndex],
                            &nrgGain[c],
                            &noiseLevel[c],
                            &nrgSine[c],
                            sinePresentFlag, i >= sineMapped[c],
                            noNoiseFlag);
          }
          
          c++;
        }
        
        m++;
      }
    }
     /* pointer for hFreq->limiterBandTable[] */
    
    for (c = 0; c < hFreq->noLimiterBands; c++) {
      float maxGain, sumRef;
      const float SBR_MAX_GAIN = 1.0e10;
       /* previous initialization */
       
      calcAvgGain(nrgRef,
                  nrgEst,
                  hFreq->limiterBandTable[c], hFreq->limiterBandTable[c+1],
                  &sumRef,
                  &maxGain);
      /* Multiply maxGain with limiterGain: */
      
      maxGain *= limiterGain;
       
      if (maxGain > SBR_MAX_GAIN) {
        
        maxGain = SBR_MAX_GAIN;
      }
       /* pointer for nrgGain[],
                                  noiseLevel[],
                                  nrgEst[],
                                  nrgSine[]
                   */
      
      for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c+1]; k++) {
         
        if (nrgGain[k] > maxGain) {
          DIV(1);  
          noiseLevel[k] *= (maxGain / nrgGain[k]);
          
          nrgGain[k] = maxGain;
        }
      }
      {
        float  boostGain, accu;
        
        accu = 0.0f;
         /* pointer for nrgGain[],
                                    noiseLevel[],
                                    nrgEst[],
                                    nrgSine[]
                     */
        
        for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c + 1]; k++) {
          
          accu += nrgGain[k] * nrgEst[k];
          
          if(nrgSine[k] != 0.0f) {
            
            accu += nrgSine[k];
          }
          else {
            
            if(noNoiseFlag == 0) {
              
              accu += noiseLevel[k];
            }
          }
        }
        
        if (accu == 0) {
          
          accu = (float)EPS;
        }
        DIV(1);
        boostGain = sumRef / accu;
         
        if(boostGain > 2.51188643f) {
          
          boostGain = 2.51188643f;
        }
         /* pointer for nrgGain[],
                                    noiseLevel[],
                                    nrgEst[],
                                    nrgSine[]
                     */
        
        for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c + 1]; k++) {
           
          nrgGain[k] = nrgGain[k] * boostGain;
          nrgSine[k] = nrgSine[k] * boostGain;
          noiseLevel[k] = noiseLevel[k] * boostGain;
        }
      }
    }
    
    if (bUseLP) {
       
      aliasingReduction(degreeAlias+lowSubband,
                        nrgGain,
                        nrgEst,
                        useAliasReduction,
                        noSubbands);
    }
     /* pointer for nrgGain[],
                                noiseLevel[],
                                nrgEst[],
                                nrgSine[]
                 */
    
    for (k=0; k<noSubbands; k++) {
       
      nrgSine[k] = (float) sqrt(nrgSine[k]);
      nrgGain[k] = (float) sqrt(nrgGain[k]);
      noiseLevel[k] = (float) sqrt(noiseLevel[k]);
    }
    
    if (h_sbr_cal_env->startUp) {
      
       /* pointer for h_sbr_cal_env->filtBufferNoise[],
                      h_sbr_cal_env->filtBuffer[],
                      nrgGain[],
                      noiseLevel[]
                   */
      
      for (k = 0; k < noSubbands; k++) {
        
        
        h_sbr_cal_env->filtBufferNoise[k] = noiseLevel[k];
        h_sbr_cal_env->filtBuffer[k] = nrgGain[k];
      }
      
      
      h_sbr_cal_env->startUp = 0;
    }
    
    
     /* pointer for sbr_smoothFilter[] */
    
    for (l = start_pos; l < stop_pos; l++) {
      
      
      if (bUseLP) {
        
         
        adjustTimeSlotLP( *(analysBufferReal_m + l) + lowSubband,
                          nrgGain,
                          noiseLevel,
                          nrgSine,
                          &h_sbr_cal_env->harmIndex,
                          lowSubband,
                          noSubbands,
                          noNoiseFlag,
                          &h_sbr_cal_env->phaseIndex);
      }
#ifndef LP_SBR_ONLY
      else {
        
        float smooth_ratio;
        
         
        if (l-start_pos < smooth_length) {
          
          
          smooth_ratio = sbr_smoothFilter[l-start_pos];
        }
        else {
          
          
          smooth_ratio = 0;
        }
        
         
        adjustTimeSlot( *(analysBufferReal_m + l) + lowSubband,
                        *(analysBufferImag_m + l) + lowSubband,
                        h_sbr_cal_env->filtBuffer,
                        h_sbr_cal_env->filtBufferNoise,
                        nrgGain,
                        noiseLevel,
                        nrgSine,
                        &h_sbr_cal_env->harmIndex,
                        lowSubband,
                        noSubbands,
                        smooth_ratio,
                        noNoiseFlag,
                        &h_sbr_cal_env->phaseIndex);
      }
#endif
    }
    
    updateFiltBuffer(h_sbr_cal_env->filtBuffer,
                     h_sbr_cal_env->filtBufferNoise,
                     nrgGain,
                     noiseLevel,
                     noSubbands);
  }
   
  if(hFrameData->frameInfo.tranEnv == nEnvelopes) {
    
    h_sbr_cal_env->prevTranEnv = 0;
  }
  else {
    
    h_sbr_cal_env->prevTranEnv = -1;
  }
  
}
/*!
  \brief   Create envelope instance
  \return  errorCode, 0 if successful
*/
int createSbrEnvelopeCalc(HANDLE_SBR_CALCULATE_ENVELOPE hs, HANDLE_SBR_HEADER_DATA hHeaderData, int chan)
{
  int err = 0;
  int i;
   
  for (i=0; i<(MAX_FREQ_COEFFS+15)/16; i++) {
    
    hs->harmFlagsPrev[i] = 0;
  }
  
  hs->harmIndex = 0;
  
  hs->prevTranEnv = -1;
  
  resetSbrEnvelopeCalc(hs);
  
  if (chan==0) {
    
    err = resetFreqBandTables(hHeaderData);
  }
  
  return err;
}
/*!
  \brief   Reset envelope instance
  \return  errorCode, 0 if successful
*/
void
resetSbrEnvelopeCalc (HANDLE_SBR_CALCULATE_ENVELOPE hCalEnv)
{
  
  
  hCalEnv->phaseIndex = 0;
  hCalEnv->startUp = 1;
  
}
/*!
  \brief  Update buffers for gains and noise levels
*/
static void updateFiltBuffer(float *filtBuffer,
                             float *filtBufferNoise,
                             float *nrgGain,
                             float *noiseLevel,
                             int     noSubbands)
{
  int k;
  
  
  for (k=0; k<noSubbands; k++) {
    
    *filtBuffer++ = *nrgGain++;
    *filtBufferNoise++ = *noiseLevel++;
  }
  
}
/*!
  \brief  Estimates the mean energy per subband
*/
static void calcNrgPerSubband(float  **analysBufferReal,
#ifndef LP_SBR_ONLY
                              float  **analysBufferImag,
#endif
                              int    lowSubband,
                              int    highSubband,
                              int    start_pos,
                              int    next_pos,
                              float  *nrgEst,
                              int    bUseLP
                              )
{
  float  temp, accu, invWidth;
  int    k,l;
  
    
  invWidth = sbr_invIntTable[next_pos - start_pos];
   /* pointer for analysBufferReal[l][k],
                              analysBufferImag[l][k]
               */
  
  for (k=lowSubband; k<highSubband; k++) {
    
    accu = 0.0f;
    
    for (l=start_pos; l<next_pos; l++) {
      
      temp = analysBufferReal[l][k];
      
      accu += temp * temp;
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
        
        temp = analysBufferImag[l][k];
        
        accu += temp * temp;
      }
#endif
    }
    
    if (bUseLP) {
      
      accu *= 2.0;
    }
    
    *nrgEst++ = accu * invWidth;
  }
  
}
/*!
  \brief   Estimates the mean energy of each Scale factor band.
*/
static void calcNrgPerSfb(float  **analysBufferReal,
#ifndef LP_SBR_ONLY
                          float  **analysBufferImag,
#endif
                          int    nSfb,
                          unsigned char *freqBandTable,
                          int    start_pos,
                          int    next_pos,
                          float  *nrgEst,
                          int   bUseLP
                          )
{
  float  temp, accu, invWidth;
  int    j,k,l,li,ui;
  
    
  invWidth = sbr_invIntTable[next_pos - start_pos];
   /* pointer for freqBandTable[],
                              analysBufferReal[][],
                              analysBufferImag[][]
               */
  
  for(j=0; j<nSfb; j++) {
    
    li = freqBandTable[j];
    ui = freqBandTable[j+1];
    
    accu = 0.0f;
    
    for (k=li; k<ui; k++) {
      
      for (l=start_pos; l<next_pos; l++) {
        
        temp   = analysBufferReal[l][k];
        
        accu += temp * temp;
#ifndef LP_SBR_ONLY
        
        if (!bUseLP) {
          
          temp  = analysBufferImag[l][k];
          
          accu += temp * temp;
        }
#endif
      }
    }
    
    if (bUseLP) {
      
      accu *= 2.0;
    }
    
    accu *= invWidth;
     
    accu *= sbr_invIntTable[ui-li];
    
    for (k=li; k<ui; k++)
    {
      
      *nrgEst++ = accu;
    }
  }
  
}
/*!
  \brief  Calculate gain, noise, and additional sine level for one subband.
*/
static void calcSubbandGain(float  nrgRef,
                            float  nrgEst,
                            float  tmpNoise,
                            float  *ptrNrgGain,
                            float  *ptrNoiseLevel,
                            float  *ptrNrgSine,
                            char   sinePresentFlag,
                            char   sineMapped,
                            int    noNoiseFlag)
{
  float a,b;
  float c;
  
  
  nrgEst += 1.0f;
  
  a = nrgRef * tmpNoise;
  
  b = 1.0f + tmpNoise;
  DIV(1); 
  *ptrNoiseLevel = a / b;
  
  if (sinePresentFlag) {
    
    c = b * nrgEst;
    DIV(1); 
    *ptrNrgGain = a / c;
    
    if (sineMapped) {
      DIV(1); 
      *ptrNrgSine = nrgRef / b;
    }
  }
  else {
    
    if (noNoiseFlag) {
      
      b = nrgEst;
    }
    else {
      
      b = b * nrgEst;
    }
    DIV(1); 
    *ptrNrgGain = nrgRef / b;
  }
  
}
/*!
  \brief  Calculate "average gain" for the specified subband range.
*/
static void calcAvgGain(float  *nrgRef,
                        float  *nrgEst,
                        int    lowSubband,
                        int    highSubband,
                        float  *ptrSumRef,
                        float  *ptrAvgGain)
{
  float sumRef = 1.0;
  float sumEst = 1.0;
  int    k;
  
   
   /* pointer for nrgRef[],
                              nrgEst[]
               */
  
  for (k=lowSubband; k<highSubband; k++){
    
    sumRef += nrgRef[k];
    
    sumEst += nrgEst[k];
  }
  DIV(1); 
  *ptrAvgGain = sumRef / sumEst;
  
  *ptrSumRef = sumRef;
  
}
/*!
  \brief   Amplify one timeslot of the signal with the calculated gains
           and add the noisefloor.
*/
static void adjustTimeSlotLP(float  *ptrReal,
                             float  *gain,
                             float  *noiseLevel,
                             float  *nrgSine,
                             char   *ptrHarmIndex,
                             int    lowSubband,
                             int    noSubbands,
                             int    noNoiseFlag,
                             int    *ptrPhaseIndex)
{
  int    k;
  float  signalReal;
  float  noiseReal;
  int    index = *ptrPhaseIndex;
  char   harmIndex = *ptrHarmIndex;
  char   freqInvFlag = (lowSubband & 1);
  float  sineLevel;
  float  sineLevelNext;
  float  sineLevelPrev;
  int    tone_count= 0;
  
     
   
  index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
   
  signalReal = *ptrReal * gain[0];
   
  sineLevel = nrgSine[0];
   
  if (noSubbands == 1) {
    
    sineLevelNext = 0;
  }
  else {
     
    sineLevelNext = nrgSine[1];
  }
  
  if (sineLevel != 0.0f) {
    
    tone_count++;
  }
  else {
     
    if (!noNoiseFlag) {
        
      noiseReal = sbr_randomPhase[index][0] * noiseLevel[0];
       
      signalReal  = signalReal + noiseReal;
    }
  }
  
  switch(harmIndex) {
  case 0:
     
    signalReal = signalReal + sineLevel;
    break;
  case 2:
     
    signalReal = signalReal - sineLevel;
    break;
  case 1:
    
    if (freqInvFlag) {
       
      *(ptrReal-1) = *(ptrReal-1) + 0.00815f * sineLevel;
        
      signalReal   = signalReal - 0.00815f * sineLevelNext;
    }
    else {
       
      *(ptrReal-1) = *(ptrReal-1) - 0.00815f * sineLevel;
        
      signalReal   = signalReal + 0.00815f * sineLevelNext;
    }
    break;
  case 3:
    
    if (freqInvFlag) {
       
      *(ptrReal-1) = *(ptrReal-1) - 0.00815f * sineLevel;
        
      signalReal   = signalReal + 0.00815f * sineLevelNext;
    }
    else {
       
      *(ptrReal-1) = *(ptrReal-1) + 0.00815f * sineLevel;
        
      signalReal   = signalReal - 0.00815f * sineLevelNext;
    }
    break;
  }
   
  *ptrReal++ = signalReal;
  
  freqInvFlag = !freqInvFlag;
   /* pointer for gain[],
                              nrgSine[],
                              sbr_randomPhase[][0],
                              noiseLevel[]          */
  
  switch(harmIndex) {
  case 0:
     
    for (k=1; k<noSubbands-1; k++) {
       
      index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
      
      signalReal = *ptrReal * gain[k];
      
      sineLevelPrev = sineLevel;
      sineLevel =  sineLevelNext;
      sineLevelNext = nrgSine[k+1];
       
      if (sineLevel == 0.0f && !noNoiseFlag) {
          
        noiseReal = sbr_randomPhase[index][0] * noiseLevel[k];
         
        signalReal  = signalReal + noiseReal;
      }
       
      *ptrReal++ = signalReal + sineLevel;
      
      freqInvFlag = !freqInvFlag;
    }
    break;
  case 1:
     
    for (k=1; k<noSubbands-1; k++) {
       
      index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
      
      signalReal = *ptrReal * gain[k];
      
      sineLevelPrev = sineLevel;
      sineLevel =  sineLevelNext;
      sineLevelNext = nrgSine[k+1];
      
      if (sineLevel != 0.0f) {
        
        tone_count++;
      }
      else {
         
        if (!noNoiseFlag) {
            
          noiseReal = sbr_randomPhase[index][0] * noiseLevel[k];
           
          signalReal  = signalReal + noiseReal;
        }
      }
       
      if (tone_count <= 16) {
        float addSine;
         
        addSine = (sineLevelPrev - sineLevelNext) * 0.00815f;
        
        if (freqInvFlag) {
           
          signalReal = signalReal + addSine;
        }
        else {
           
          signalReal = signalReal - addSine;
        }
      }
       
      *ptrReal++ = signalReal;
      
      freqInvFlag = !freqInvFlag;
    }
    break;
  case 2:
     
    for (k=1; k<noSubbands-1; k++) {
       
      index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
      
      signalReal = *ptrReal * gain[k];
      
      sineLevelPrev = sineLevel;
      sineLevel =  sineLevelNext;
      sineLevelNext = nrgSine[k+1];
       
      if (sineLevel == 0.0f && !noNoiseFlag) {
          
        noiseReal = sbr_randomPhase[index][0] * noiseLevel[k];
         
        signalReal  = signalReal + noiseReal;
      }
       
      *ptrReal++ = signalReal - sineLevel;
      
      freqInvFlag = !freqInvFlag;
    }
    break;
  case 3:
     
    for (k=1; k<noSubbands-1; k++) {
       
      index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
      
      signalReal = *ptrReal * gain[k];
      
      sineLevelPrev = sineLevel;
      sineLevel =  sineLevelNext;
      sineLevelNext = nrgSine[k+1];
      
      if (sineLevel != 0.0f) {
        
        tone_count++;
      }
      else {
         
        if (!noNoiseFlag) {
            
          noiseReal = sbr_randomPhase[index][0] * noiseLevel[k];
           
          signalReal  = signalReal + noiseReal;
        }
      }
       
      if (tone_count <= 16) {
        float addSine;
         
        addSine = (sineLevelPrev - sineLevelNext) * 0.00815f;
        
        if (freqInvFlag) {
           
          signalReal = signalReal - addSine;
        }
        else {
           
          signalReal = signalReal + addSine;
        }
      }
       
      *ptrReal++ = signalReal;
      
      freqInvFlag = !freqInvFlag;
    }
    break;
  }
   
  if (noSubbands > 1) 
	{     
    index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
    
    signalReal = *ptrReal * gain[k];
    
    sineLevelPrev = sineLevel * 0.00815f;
    
    sineLevel =  sineLevelNext;
    
    if (sineLevel != 0.0f) {
      
      tone_count++;
    }
    else {
       
      if (!noNoiseFlag) {
          
        noiseReal = sbr_randomPhase[index][0] * noiseLevel[k];
         
        signalReal  = signalReal + noiseReal;
      }
    }
    
    switch(harmIndex) {
    case 0:
       
      *ptrReal = signalReal + sineLevel;
      break;
    case 2:
       
      *ptrReal = signalReal - sineLevel;
      break;
    case 1:
       
      if(tone_count <= 16) {
        
        if (freqInvFlag) {
           
          *ptrReal++   = signalReal + sineLevelPrev;
           
          if (k + lowSubband < 62) {
             
            *ptrReal     = *ptrReal   - 0.00815f*sineLevel;
          }
        }
        else {
           
          *ptrReal++   = signalReal - sineLevelPrev;
           
          if (k + lowSubband < 62) {
             
            *ptrReal     = *ptrReal   + 0.00815f*sineLevel;
          }
        }
      }
      else {
        
        *ptrReal = signalReal;
      }
      break;
    case 3:
       
      if(tone_count <= 16) {
        
        if (freqInvFlag) {
           
          *ptrReal++   = signalReal - sineLevelPrev;
           
          if (k + lowSubband < 62) {
             
            *ptrReal     = *ptrReal   + 0.00815f*sineLevel;
          }
        }
        else {
           
          *ptrReal++   = signalReal + sineLevelPrev;
           
          if (k + lowSubband < 62) {
             
            *ptrReal     = *ptrReal - 0.00815f*sineLevel;
          }
        }
      }
      else {
        
        *ptrReal = signalReal;
      }
      break;
    }
  }
    
  *ptrHarmIndex  = (harmIndex + 1) & 3;
  
  *ptrPhaseIndex = index;
  
}
#ifndef LP_SBR_ONLY
static void adjustTimeSlot(float  *ptrReal,
                           float  *ptrImag,
                           float  *filtBuffer,
                           float  *filtBufferNoise,
                           float  *gain,
                           float  *noiseLevel,
                           float  *nrgSine,
                           char   *ptrHarmIndex,
                           int    lowSubband,
                           int    noSubbands,
                           float  smooth_ratio,
                           int    noNoiseFlag,
                           int    *ptrPhaseIndex)
{
  int    k;
  float  signalReal, signalImag;
  float  noiseReal,  noiseImag;
  float  smoothedGain, smoothedNoise;
  float  direct_ratio = 1.0f - smooth_ratio;
  int    index = *ptrPhaseIndex;
  char   harmIndex = *ptrHarmIndex;
  char   freqInvFlag = (lowSubband & 1);
  float  sineLevel;
  
     
   /* pointers for gain[],
                               nrgSine[],
                               sbr_randomPhase[][0],
                               noiseLevel[],
                               filtBuffer[]          */
  
  for (k=0; k<noSubbands; k++) {
    
    if (smooth_ratio > 0) {
       
      smoothedGain =
        smooth_ratio * filtBuffer[k] +
        direct_ratio * gain[k];
       
      smoothedNoise =
        smooth_ratio * filtBufferNoise[k] +
        direct_ratio * noiseLevel[k];
    }
    else {
      
      smoothedGain  = gain[k];
      smoothedNoise = noiseLevel[k];
    }
    
    signalReal = *ptrReal * smoothedGain;
    signalImag = *ptrImag * smoothedGain;
     
    index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);
    
    if (nrgSine[k] != 0.0f) {
      
      sineLevel = nrgSine[k];
      
      switch(harmIndex) {
      case 0:
         
        *ptrReal = signalReal + sineLevel;
        
        *ptrImag = signalImag;
        break;
      case 2:
         
        *ptrReal = signalReal - sineLevel;
        
        *ptrImag = signalImag;
        break;
      case 1:
        
        *ptrReal = signalReal;
        
        if (freqInvFlag) {
           
          *ptrImag = signalImag - sineLevel;
        }
        else {
           
          *ptrImag = signalImag + sineLevel;
        }
        break;
      case 3:
        
        *ptrReal = signalReal;
        
        if (freqInvFlag) {
           
          *ptrImag = signalImag + sineLevel;
        }
        else {
           
          *ptrImag = signalImag - sineLevel;
        }
        break;
      }
    }
    else {
      
      if (noNoiseFlag) {
        
        *ptrReal = signalReal;
        *ptrImag = signalImag;
      }
      else {
        
        noiseReal = sbr_randomPhase[index][0] * smoothedNoise;
        noiseImag = sbr_randomPhase[index][1] * smoothedNoise;
         
        *ptrReal  = signalReal + noiseReal;
        *ptrImag  = signalImag + noiseImag;
      }
    }
    
    freqInvFlag = !freqInvFlag;
    
    ptrReal++;
    ptrImag++;
  }
    
  *ptrHarmIndex = (harmIndex + 1) & 3;
  
  *ptrPhaseIndex = index;
  
}
#endif
/*!
  \brief   Reset limiter bands.
*/
int
ResetLimiterBands ( unsigned char *limiterBandTable,
                    unsigned char *noLimiterBands,
                    unsigned char *freqBandTable,
                    int noFreqBands,
                    const PATCH_PARAM *patchParam,
                    int noPatches,
                    int limiterBands)
{
  int i, k, isPatchBorder[2], loLimIndex, hiLimIndex, tempNoLim, nBands;
  unsigned char workLimiterBandTable[MAX_FREQ_COEFFS / 2 + MAX_NUM_PATCHES + 1];
  int patchBorders[MAX_NUM_PATCHES + 1];
  int kx, k2;
  float temp;
  int lowSubband = freqBandTable[0];
  int highSubband = freqBandTable[noFreqBands];
  
    
  
  if(limiterBands == 0) {
    
    limiterBandTable[0] = 0;
     
    limiterBandTable[1] = highSubband - lowSubband;
    
    nBands = 1;
  } else {
     /* patchBorders[i]
                    patchParam[i]
                 */
    
    for (i = 0; i < noPatches; i++) {
       
      patchBorders[i] = patchParam[i].guardStartBand - lowSubband;
    }
     
    patchBorders[i] = highSubband - lowSubband;
     /* workLimiterBandTable[k]
                    freqBandTable[k]
                 */
    
    for (k = 0; k <= noFreqBands; k++) {
       
      workLimiterBandTable[k] = freqBandTable[k] - lowSubband;
    }
     /* workLimiterBandTable[noFreqBands + k]
                    patchBorders[k]
                 */
    
    for (k = 1; k < noPatches; k++) {
      
      workLimiterBandTable[noFreqBands + k] = patchBorders[k];
    }
     
    tempNoLim = nBands = noFreqBands + noPatches - 1;
     
    shellsort(workLimiterBandTable, tempNoLim + 1);
    
    loLimIndex = 0;
    hiLimIndex = 1;
    
    while (hiLimIndex <= tempNoLim) {
       
      k2 = workLimiterBandTable[hiLimIndex] + lowSubband;
      kx = workLimiterBandTable[loLimIndex] + lowSubband;
      
      temp = FloatFR_getNumOctaves(kx,k2);
       
      temp = temp * sbr_limiterBandsPerOctave[limiterBands];
       
      if (temp < 0.49f) {
         
        if (workLimiterBandTable[hiLimIndex] == workLimiterBandTable[loLimIndex]) {
          
          workLimiterBandTable[hiLimIndex] = highSubband;
          
          nBands--;
          hiLimIndex++;
          continue;
        }
        
        isPatchBorder[0] = isPatchBorder[1] = 0;
         /* patchBorders[k] */
         /* workLimiterBandTable[hiLimIndex] */
        
        for (k = 0; k <= noPatches; k++) {
           
          if (workLimiterBandTable[hiLimIndex] == patchBorders[k]) {
            
            isPatchBorder[1] = 1;
            break;
          }
        }
        
        if (!isPatchBorder[1]) {
          
          workLimiterBandTable[hiLimIndex] = highSubband;
          
          nBands--;
          hiLimIndex++;
          continue;
        }
         /* patchBorders[k] */
         /* workLimiterBandTable[loLimIndex] */
        
        for (k = 0; k <= noPatches; k++) {
           
          if (workLimiterBandTable[loLimIndex] == patchBorders[k]) {
            
            isPatchBorder[0] = 1;
            break;
          }
        }
        
        if (!isPatchBorder[0]) {
          
          workLimiterBandTable[loLimIndex] = highSubband;
          
          nBands--;
        }
      }
      
      loLimIndex = hiLimIndex;
      
      hiLimIndex++;
    }
     
    shellsort(workLimiterBandTable, tempNoLim + 1);
      
    if( nBands > MAX_NUM_LIMITERS || nBands <= 0) {
      
      return -1;
    }
     /* limiterBandTable[k]
                    workLimiterBandTable[k]
                 */
    
    for (k = 0; k <= nBands; k++) {
      
      limiterBandTable[k] = workLimiterBandTable[k];
    }
  }
  
  *noLimiterBands = nBands;
  
  return 0;
}
