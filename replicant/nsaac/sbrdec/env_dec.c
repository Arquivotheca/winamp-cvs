/*
  envelope decoding
  This module provides envelope decoding and error concealment algorithms. The main
  entry point is decodeSbrData().
*/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "env_dec.h"
#include "sbr_const.h"
#include "env_extr.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
static void decodeEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                            HANDLE_SBR_FRAME_DATA h_sbr_data,
                            HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                            HANDLE_SBR_PREV_FRAME_DATA h_prev_data_otherChannel);
static void sbr_envelope_unmapping (HANDLE_SBR_HEADER_DATA hHeaderData,
                                    HANDLE_SBR_FRAME_DATA h_data_left,
                                    HANDLE_SBR_FRAME_DATA h_data_right);
static void requantizeEnvelopeData (HANDLE_SBR_FRAME_DATA h_sbr_data,
                                    int ampResolution);
static void deltaToLinearPcmEnvelopeDecoding (HANDLE_SBR_HEADER_DATA hHeaderData,
                                              HANDLE_SBR_FRAME_DATA h_sbr_data,
                                              HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static void decodeNoiseFloorlevels (HANDLE_SBR_HEADER_DATA hHeaderData,
                                    HANDLE_SBR_FRAME_DATA h_sbr_data,
                                    HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static void timeCompensateFirstEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                                         HANDLE_SBR_FRAME_DATA h_sbr_data,
                                         HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static int checkEnvelopeData (HANDLE_SBR_HEADER_DATA hHeaderData,
                              HANDLE_SBR_FRAME_DATA h_sbr_data,
                              HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
#define SBR_ENERGY_PAN_OFFSET   12.0f
#define SBR_MAX_ENERGY          35.0f
#define DECAY                    1.0f
#define DECAY_COUPLING           1.0f
/*
  \brief  Convert table index
*/
static int indexLow2High(int offset,
                         int index,
                         int res)
{
  
   
  if(res == LO)
  {
    
    if (offset >= 0)
    {
         
        if (index < offset)
        {
          
          return(index);
        }
        else
        {
            /* counting post-operation */
          
          return(2*index - offset);
        }
    }
    else
    {
        
        offset = -offset;
         
        if (index < offset)
        {
            /* counting post-operation */
          
          return(2*index+index);
        }
        else
        {
            /* counting post-operation */
          
          return(2*index + offset);
        }
    }
  }
  else
  {
    
    return(index);
  }
}
/*
  \brief  Update previous envelope value for delta-coding
*/
static void mapLowResEnergyVal(float  currVal,
                               float  *prevData,
                               int offset,
                               int index,
                               int res)
{
  
   
  if(res == LO)
  {
    
    if (offset >= 0)
    {
       
        if(index < offset)
        {
             
            prevData[index] = currVal;
        }
        else
        {
             
            prevData[2*index - offset] = currVal;
            prevData[2*index+1 - offset] = currVal;
        }
    }
    else
    {
        
        offset = -offset;
         
        if (index < offset)
        {
             
            prevData[3*index] = currVal;
            prevData[3*index+1] = currVal;
            prevData[3*index+2] = currVal;
        }
        else
        {
             
            prevData[2*index + offset] = currVal;
            prevData[2*index + 1 + offset] = currVal;
        }
    }
  }
  else
  {
     
    prevData[index] = currVal;
  }
  
}
/*
  \brief    Convert raw envelope and noisefloor data to energy levels
*/
void
decodeSbrData (HANDLE_SBR_HEADER_DATA hHeaderData,
               HANDLE_SBR_FRAME_DATA h_data_left,
               HANDLE_SBR_PREV_FRAME_DATA h_prev_data_left,
               HANDLE_SBR_FRAME_DATA h_data_right,
               HANDLE_SBR_PREV_FRAME_DATA h_prev_data_right)
{
  int errLeft;
  
  
  decodeEnvelope (hHeaderData, h_data_left, h_prev_data_left, h_prev_data_right);
  
  decodeNoiseFloorlevels (hHeaderData, h_data_left, h_prev_data_left);
  
  if(h_data_right != NULL) {
    
    errLeft = hHeaderData->frameErrorFlag;
    
    decodeEnvelope (hHeaderData, h_data_right, h_prev_data_right, h_prev_data_left);
    
    decodeNoiseFloorlevels (hHeaderData, h_data_right, h_prev_data_right);
      
    if (!errLeft && hHeaderData->frameErrorFlag) {
      
      decodeEnvelope (hHeaderData, h_data_left, h_prev_data_left, h_prev_data_right);
    }
     
    if (h_data_left->coupling) {
      
      sbr_envelope_unmapping (hHeaderData, h_data_left, h_data_right);
    }
  }
  
}
/*
  \brief   Convert from coupled channels to independent L/R data
*/
static void
sbr_envelope_unmapping (HANDLE_SBR_HEADER_DATA hHeaderData,
                        HANDLE_SBR_FRAME_DATA h_data_left,
                        HANDLE_SBR_FRAME_DATA h_data_right)
{
  int i;
  float tempR, tempL, newL, newR;
  const float unmapScale = (float) pow(2.0, -16.0);
  
   
   /* pointers for h_data_left->iEnvelope[],
                               h_data_right->iEnvelope[]
               */
   
  for (i = 0; i < h_data_left->nScaleFactors; i++) {
    
    tempR = h_data_right->iEnvelope[i];
    
    tempR = tempR * unmapScale;
    
    tempL = h_data_left->iEnvelope[i];
      DIV(1);
    newR = 2.0f * tempL / (tempR + 1.0f);
    
    newL = tempR * newR;
    
    h_data_left->iEnvelope[i] = newL;
    h_data_right->iEnvelope[i] = newR;
  }
   /* pointers for h_data_left->sbrNoiseFloorLevel[i],
                               h_data_right->sbrNoiseFloorLevel[i]
               */
    
  for (i = 0; i < hHeaderData->FreqBandData.nNfb * h_data_left->frameInfo.nNoiseEnvelopes; i++) {
    
    tempL = NOISE_FLOOR_OFFSET - h_data_left->sbrNoiseFloorLevel[i];
    tempR = h_data_right->sbrNoiseFloorLevel[i] - 12.0f /*SBR_ENERGY_PAN_OFFSET*/;
    
    tempL = (float) pow(2.0, tempL);
    tempR = (float) pow(2.0, tempR);
      DIV(1);
    newR = 2.0f * tempL / (1.0f + tempR);
    
    newL = tempR * newR;
    
    h_data_left->sbrNoiseFloorLevel[i] = newL;
    h_data_right->sbrNoiseFloorLevel[i] = newR;
  }
  
}
/*
  \brief    Simple alternative to the real SBR concealment
*/
static void
leanSbrConcealment(HANDLE_SBR_HEADER_DATA hHeaderData,
                   HANDLE_SBR_FRAME_DATA  h_sbr_data,
                   HANDLE_SBR_PREV_FRAME_DATA h_prev_data
                   )
{
  float  target;
  float  step;
  int i;
  int currentStartPos = h_prev_data->stopPos - hHeaderData->numberTimeSlots;
  int currentStopPos = hHeaderData->numberTimeSlots;
  
     
   
  h_sbr_data->ampResolutionCurrentFrame = h_prev_data->ampRes;
   
  h_sbr_data->coupling = h_prev_data->coupling;
   /* h_sbr_data->sbr_invf_mode[i]
                  h_prev_data->sbr_invf_mode[i]
               */
  
  for(i=0;i<MAX_INVF_BANDS;i++)
  {
    
    h_sbr_data->sbr_invf_mode[i] = h_prev_data->sbr_invf_mode[i];
  }
   
  h_sbr_data->frameInfo.nEnvelopes = 1;
  h_sbr_data->frameInfo.borders[0] = currentStartPos;
  h_sbr_data->frameInfo.borders[1] = currentStopPos;
  h_sbr_data->frameInfo.freqRes[0] = 1;
  h_sbr_data->frameInfo.tranEnv = -1;
  h_sbr_data->frameInfo.nNoiseEnvelopes = 1;
  h_sbr_data->frameInfo.bordersNoise[0] = currentStartPos;
  h_sbr_data->frameInfo.bordersNoise[1] = currentStopPos;
   
  h_sbr_data->nScaleFactors = hHeaderData->FreqBandData.nSfb[1];
   
  h_sbr_data->domain_vec[0] = TIME;
   
  if (h_sbr_data->coupling == COUPLING_BAL) {
    
    target = SBR_ENERGY_PAN_OFFSET;
    step = DECAY_COUPLING;
  }
  else {
    
    target = 0.0;
    step = DECAY;
  }
    
  if (h_sbr_data->ampResolutionCurrentFrame == SBR_AMP_RES_1_5) {
    
    target *= 2;
    step *= 2;
  }
   /* h_prev_data->sfb_nrg_prev[i]
                  h_sbr_data->iEnvelope[i]
               */
   
  for (i=0; i < h_sbr_data->nScaleFactors; i++) {
     
    if (h_prev_data->sfb_nrg_prev[i] > target)
    {
       
      h_sbr_data->iEnvelope[i] = -step;
    }
    else
    {
      
      h_sbr_data->iEnvelope[i] = step;
    }
  }
   
  h_sbr_data->domain_vec_noise[0] = TIME;
   /* h_sbr_data->sbrNoiseFloorLevel[i] */
   
  for (i=0; i < hHeaderData->FreqBandData.nNfb; i++)
  {
    
    h_sbr_data->sbrNoiseFloorLevel[i] = 0.0f;
  }
   
  for (i=0; i< MAX_FREQ_COEFFS; i++) {
    
    h_sbr_data->addHarmonics[i] = 0;
  }
  
}
/*
  \brief   Build reference energies and noise levels from bitstream elements
*/
static void
decodeEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                HANDLE_SBR_FRAME_DATA  h_sbr_data,
                HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                HANDLE_SBR_PREV_FRAME_DATA otherChannel
                )
{
  int i;
  int Error_flag;
  float tempSfbNrgPrev[MAX_FREQ_COEFFS];
  
     
  if ( (!hHeaderData->prevFrameErrorFlag) && (!hHeaderData->frameErrorFlag) &&
       (h_sbr_data->frameInfo.borders[0] != h_prev_data->stopPos - hHeaderData->numberTimeSlots) ) {
      
    if (h_sbr_data->domain_vec[0] == TIME) {
       
      hHeaderData->frameErrorFlag = 1;
    }
    else {
       
      hHeaderData->prevFrameErrorFlag = 1;
    }
  }
   
  if (hHeaderData->frameErrorFlag)
    {
      
      leanSbrConcealment(hHeaderData,
                         h_sbr_data,
                         h_prev_data);
      
      deltaToLinearPcmEnvelopeDecoding (hHeaderData, h_sbr_data, h_prev_data);
    }
  else
    {
       
      if (hHeaderData->prevFrameErrorFlag) {
        
        timeCompensateFirstEnvelope (hHeaderData, h_sbr_data, h_prev_data);
          
        if (h_sbr_data->coupling != h_prev_data->coupling) {
           /* pointers for h_prev_data->sfb_nrg_prev[i],
                                       otherChannel->sfb_nrg_prev[i]
                       */
          
          for (i = 0; i < hHeaderData->FreqBandData.nSfb[HI]; i++) {
              
            if (h_prev_data->coupling == COUPLING_BAL) {
              
              h_prev_data->sfb_nrg_prev[i] = otherChannel->sfb_nrg_prev[i];
            }
            else {
                
              if (h_sbr_data->coupling == COUPLING_LEVEL) {
                  
                h_prev_data->sfb_nrg_prev[i] = 0.5f * (h_prev_data->sfb_nrg_prev[i] + otherChannel->sfb_nrg_prev[i]);
              }
              else {
                  
                if (h_sbr_data->coupling == COUPLING_BAL) {
                  
                  h_prev_data->sfb_nrg_prev[i] = SBR_ENERGY_PAN_OFFSET;
                }
              }
            }
          }
        }
      }
          STORE(MAX_FREQ_COEFFS);
      memcpy (tempSfbNrgPrev, h_prev_data->sfb_nrg_prev,
              MAX_FREQ_COEFFS * sizeof (float));
      
      deltaToLinearPcmEnvelopeDecoding (hHeaderData, h_sbr_data, h_prev_data);
      
      Error_flag = checkEnvelopeData (hHeaderData, h_sbr_data, h_prev_data);
      
      if (Error_flag)
        {
           
          hHeaderData->frameErrorFlag = 1;
              STORE(MAX_FREQ_COEFFS);
          memcpy (h_prev_data->sfb_nrg_prev, tempSfbNrgPrev,
                  MAX_FREQ_COEFFS * sizeof (float));
          
          decodeEnvelope (hHeaderData, h_sbr_data, h_prev_data, otherChannel);
          return;
        }
    }
   
  requantizeEnvelopeData (h_sbr_data, h_sbr_data->ampResolutionCurrentFrame);
  
}
/*
  \brief   Verify that envelope energies are within the allowed range
  \return  0 if all is fine, 1 if an envelope value was too high
*/
static int
checkEnvelopeData (HANDLE_SBR_HEADER_DATA hHeaderData,
                   HANDLE_SBR_FRAME_DATA h_sbr_data,
                   HANDLE_SBR_PREV_FRAME_DATA h_prev_data
                   )
{
  float *iEnvelope = h_sbr_data->iEnvelope;
  float *sfb_nrg_prev = h_prev_data->sfb_nrg_prev;
  int    i = 0, errorFlag = 0;
  float  sbr_max_energy =
    (h_sbr_data->ampResolutionCurrentFrame == SBR_AMP_RES_3_0) ? SBR_MAX_ENERGY : (2.0f * SBR_MAX_ENERGY);
  
       
   
  for (i = 0; i < h_sbr_data->nScaleFactors; i++) {
     
    if (iEnvelope[i] > sbr_max_energy) {
      
      errorFlag = 1;
    }
    
    if (iEnvelope[i] < 0)
    {
      
      iEnvelope[i] = 0;
    }
  }
   
  for (i = 0; i < hHeaderData->FreqBandData.nSfb[HI]; i++) {
    
    if (sfb_nrg_prev[i] < 0)
    {
      
      sfb_nrg_prev[i] = 0;
    }
    else {
     
    if (sfb_nrg_prev[i] > sbr_max_energy) {
      
      sfb_nrg_prev[i] = sbr_max_energy;
    }
    }
  }
  
  return (errorFlag);
}
/*
  \brief   Verify that the noise levels are within the allowed range
*/
static void
limitNoiseLevels(HANDLE_SBR_HEADER_DATA hHeaderData,
                 HANDLE_SBR_FRAME_DATA h_sbr_data)
{
  int i;
  int nNfb = hHeaderData->FreqBandData.nNfb;
  float value;
  float lowerLimit = 0.0;
  float upperLimit = 35.0;
  
    
   /* h_sbr_data->sbrNoiseFloorLevel[i] */
    
  for (i = 0; i < h_sbr_data->frameInfo.nNoiseEnvelopes * nNfb; i++) {
    
    value = h_sbr_data->sbrNoiseFloorLevel[i];
     
    if (value > upperLimit)
    {
      
      h_sbr_data->sbrNoiseFloorLevel[i] = upperLimit;
    }
    else {
     
    if (value < lowerLimit)
    {
      
      h_sbr_data->sbrNoiseFloorLevel[i] = lowerLimit;
    }
    }
  }
  
}
/*
  \brief   Compensate for the wrong timing that might occur after a frame error.
*/
static void
timeCompensateFirstEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                             HANDLE_SBR_FRAME_DATA h_sbr_data,
                             HANDLE_SBR_PREV_FRAME_DATA h_prev_data)
{
  int i, nScalefactors;
  FRAME_INFO *pFrameInfo = &h_sbr_data->frameInfo;
  unsigned char *nSfb = hHeaderData->FreqBandData.nSfb;
  int estimatedStartPos = h_prev_data->stopPos - hHeaderData->numberTimeSlots;
  int refLen, newLen;
  float deltaExp;
  
     
   
  refLen = pFrameInfo->borders[1] - pFrameInfo->borders[0];
  
  newLen = pFrameInfo->borders[1] - estimatedStartPos;
  
  if (newLen <= 0) {
    
    newLen = refLen;
    estimatedStartPos = pFrameInfo->borders[0];
  }
  DIV(1);
  deltaExp = (float)refLen / (float)newLen;
   DIV(1);
  deltaExp = (float)( log(deltaExp)/log(2.0) );
    
  if (h_sbr_data->ampResolutionCurrentFrame == SBR_AMP_RES_1_5) /* fine resolution */
  {
    
    deltaExp *= 2.0f;
  }
   
  pFrameInfo->borders[0] = estimatedStartPos;
  pFrameInfo->bordersNoise[0] = estimatedStartPos;
    
  if (h_sbr_data->coupling != COUPLING_BAL) {
      
    nScalefactors = (pFrameInfo->freqRes[0]) ? nSfb[HI] : nSfb[LO];
     /* h_sbr_data->iEnvelope[i] */
    
    for (i = 0; i < nScalefactors; i++)
    {
       
      h_sbr_data->iEnvelope[i] = h_sbr_data->iEnvelope[i] + deltaExp;
    }
  }
  
}
/*
  \brief   Convert each envelope value from logarithmic to linear domain
*/
static void
requantizeEnvelopeData (HANDLE_SBR_FRAME_DATA h_sbr_data, int ampResolution)
{
  int i;
  float mantissa;
  int ampShift = 1 - ampResolution;
  int exponent;
  
   
   /* h_sbr_data->iEnvelope[] */
   
  for (i = 0; i < h_sbr_data->nScaleFactors; i++) {
    
    exponent = (int)h_sbr_data->iEnvelope[i];
      
    mantissa = (exponent & ampShift) ? 0.707106781186548f : 0.5f;
    
    exponent = exponent >> ampShift;
    
    exponent += 5;
      
    h_sbr_data->iEnvelope[i] = (float) (mantissa * pow(2.0, (double)exponent));
  }
  
}
/*
  \brief   Build new reference energies from old ones and delta coded data
*/
static void
deltaToLinearPcmEnvelopeDecoding (HANDLE_SBR_HEADER_DATA hHeaderData,
                                  HANDLE_SBR_FRAME_DATA h_sbr_data,
                                  HANDLE_SBR_PREV_FRAME_DATA h_prev_data)
{
  int i, domain, no_of_bands, band, freqRes, offset;
  float *sfb_nrg_prev;
  float *ptr_nrg;
  
  
  sfb_nrg_prev = h_prev_data->sfb_nrg_prev;
  ptr_nrg = h_sbr_data->iEnvelope;
    
  offset = 2 * hHeaderData->FreqBandData.nSfb[LO] - hHeaderData->FreqBandData.nSfb[HI];
   /* pointers for h_sbr_data->domain_vec[i],
                               h_sbr_data->frameInfo.freqRes[i]
               */
  
  for (i = 0; i < h_sbr_data->frameInfo.nEnvelopes; i++) {
    
    domain = h_sbr_data->domain_vec[i];
    freqRes = h_sbr_data->frameInfo.freqRes[i];
     
    no_of_bands = hHeaderData->FreqBandData.nSfb[freqRes];
     
    if (domain == FREQ)
    {
      
      mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, 0, freqRes);
      ptr_nrg++;
      
      for (band = 1; band < no_of_bands; band++)
      {
        *ptr_nrg = *ptr_nrg + *(ptr_nrg-1);
        
        mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, band, freqRes);
        ptr_nrg++;
      }
    }
    else
    {
      
      for (band = 0; band < no_of_bands; band++)
      {
          
        *ptr_nrg = *ptr_nrg + sfb_nrg_prev[indexLow2High(offset, band, freqRes)];
        
        mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, band, freqRes);
        ptr_nrg++;
      }
    }
  }
  
}
/*
  \brief   Build new noise levels from old ones and delta coded data
*/
static void
decodeNoiseFloorlevels (HANDLE_SBR_HEADER_DATA hHeaderData,
                        HANDLE_SBR_FRAME_DATA h_sbr_data,
                        HANDLE_SBR_PREV_FRAME_DATA h_prev_data)
{
  int i;
  int nNfb = hHeaderData->FreqBandData.nNfb;
  int nNoiseFloorEnvelopes = h_sbr_data->frameInfo.nNoiseEnvelopes;
  
   
   
  if (h_sbr_data->domain_vec_noise[0] == FREQ) {
     /* pointers for h_sbr_data->sbrNoiseFloorLevel[]
                  */
    
    for (i = 1; i < nNfb; i++) {
       
      h_sbr_data->sbrNoiseFloorLevel[i] =
        h_sbr_data->sbrNoiseFloorLevel[i] + h_sbr_data->sbrNoiseFloorLevel[i - 1];
    }
  }
  else {
     /* pointers for h_sbr_data->sbrNoiseFloorLevel[]
                  */
    
    for (i = 0; i < nNfb; i++) {
       
      h_sbr_data->sbrNoiseFloorLevel[i] =
        h_sbr_data->sbrNoiseFloorLevel[i] + h_prev_data->prevNoiseLevel[i];
    }
  }
   
  if (nNoiseFloorEnvelopes > 1) {
     
    if (h_sbr_data->domain_vec_noise[1] == FREQ) {
       /* pointers for h_sbr_data->sbrNoiseFloorLevel[]
                    */
       
      for (i = nNfb + 1; i < 2*nNfb; i++) {
         
        h_sbr_data->sbrNoiseFloorLevel[i] =
          h_sbr_data->sbrNoiseFloorLevel[i] + h_sbr_data->sbrNoiseFloorLevel[i - 1];
      }
    }
    else {
       /* pointers for h_sbr_data->sbrNoiseFloorLevel[]
                    */
      
      for (i = 0; i < nNfb; i++) {
         
        h_sbr_data->sbrNoiseFloorLevel[i + nNfb] =
          h_sbr_data->sbrNoiseFloorLevel[i + nNfb] + h_sbr_data->sbrNoiseFloorLevel[i];
      }
    }
  }
  
  limitNoiseLevels(hHeaderData, h_sbr_data);
     /* pointers for h_sbr_data->sbrNoiseFloorLevel[],
                                                h_prev_data->prevNoiseLevel[]
                                */
  
  for (i = 0; i < nNfb; i++) {
    
    h_prev_data->prevNoiseLevel[i] = h_sbr_data->sbrNoiseFloorLevel[i + nNfb*(nNoiseFloorEnvelopes-1)];
  }
  
  if (!h_sbr_data->coupling) {
    int nf_e;
     /* pointers for h_sbr_data->sbrNoiseFloorLevel[]
                  */
     
    for (i = 0; i < nNoiseFloorEnvelopes*nNfb; i++) {
      
      nf_e = NOISE_FLOOR_OFFSET - (long)h_sbr_data->sbrNoiseFloorLevel[i];
       
      h_sbr_data->sbrNoiseFloorLevel[i] = (float) pow(2.0, (double)nf_e);
    }
  }
  
}
