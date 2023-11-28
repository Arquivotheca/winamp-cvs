/*
  Sbr decoder
  This module provides the actual decoder implementation
*/
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "sbr_ram.h"
#include "sbr_dec.h"
#include "sbrdecsettings.h"
#include "env_extr.h"
#include "env_calc.h"
#include "math/FloatFR.h"
#include "ps_dec.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  \brief      SBR decoder core function for one channel
*/
void
sbr_dec (HANDLE_SBR_DEC hSbrDec,
         float *timeIn,
         float *timeOut,
         float *interimResult,
         HANDLE_SBR_HEADER_DATA hHeaderData,
         HANDLE_SBR_FRAME_DATA hFrameData,
         HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData,
         int applyProcessing
         ,HANDLE_PS_DEC h_ps_d,
         HANDLE_SBR_QMF_FILTER_BANK hSynthesisQmfBankRight, 
         int nChannels)
{
  int i, k, slot;
  int ov_len;
  int bUseLP=1;
  float  *QmfBufferReal[MAX_ENV_COLS];
  float  *QmfBufferImag[MAX_ENV_COLS];
  float  *ptr;
  int noCols = hHeaderData->numberTimeSlots * hHeaderData->timeStep;
  int halflen = noCols >> 1;
  int islots = noCols >> 2;
  
  if (nChannels == 1) 
	{    
    bUseLP = 0;
  }

  if(hHeaderData->channelMode==MONO) {
    
    bUseLP = 0;
  }
  
  ov_len = 6;
  
  if (bUseLP) 
	{     
    ptr = hSbrDec->sbr_OverlapBuffer;
    
    for(slot=0; slot<ov_len; slot++) 
		{
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
    }
      
    ptr = timeOut + islots * 2 * NO_SYNTHESIS_CHANNELS;
    
		for(i=0; i<2*(halflen-islots); i++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }
    
    ptr = timeIn;
     
    for(i=0; i<2*islots; i++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }
    assert( sizeof(SBR_FRAME_DATA) <= islots * 2 * NO_SYNTHESIS_CHANNELS * sizeof(float) );
  }

  else {
    
    ptr = hSbrDec->sbr_OverlapBuffer;
    
    for(slot=0; slot<ov_len; slot++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      
      QmfBufferImag[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
    }
      
    ptr = timeOut + islots * 2 * NO_SYNTHESIS_CHANNELS;
     
    for(i=0; i<halflen-islots; i++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      
      QmfBufferImag[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }
    
    ptr = &WorkBuffer2[0];
    
    for(i=0; i<halflen; i++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      
      QmfBufferImag[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }
      
    if(hHeaderData->channelMode==PS_STEREO)
    {
      
      ptr=interimResult;
    }
    else
    {
      
      ptr = timeIn;
    }
    
    for(i=0; i<islots; i++) 
		{      
      QmfBufferReal[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      
      QmfBufferImag[slot] = ptr;
      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }
    assert( sizeof(SBR_FRAME_DATA) <= islots * 2 * NO_SYNTHESIS_CHANNELS * sizeof(float) );
  }

  assert(slot == noCols+ov_len);

  cplxAnalysisQmfFiltering(timeIn, &QmfBufferReal[ov_len], &QmfBufferImag[ov_len], &hSbrDec->CodecQmfBank, bUseLP);
   
  for (slot = ov_len; slot < noCols+ov_len; slot++) {
    
    for (k=NO_ANALYSIS_CHANNELS; k<NO_SYNTHESIS_CHANNELS; k++){
      
      QmfBufferReal[slot][k] = 0;
     
      if (!bUseLP)
      {
        QmfBufferImag[slot][k] = 0;
      }
    }
  }
  
  if (applyProcessing)
  {
    float degreeAlias[NO_SYNTHESIS_CHANNELS];
    unsigned char * borders = hFrameData->frameInfo.borders;
    
    if (bUseLP) 
		{
      memset (degreeAlias, 0, NO_SYNTHESIS_CHANNELS * sizeof (float));
    }

    lppTransposer(&hSbrDec->LppTrans,
                    QmfBufferReal,
                    QmfBufferImag,
                    degreeAlias,
                    hHeaderData->timeStep,
                    borders[0],
                    borders[hFrameData->frameInfo.nEnvelopes] - hHeaderData->numberTimeSlots,
                    hHeaderData->FreqBandData.nInvfBands,
                    hFrameData->sbr_invf_mode,
                    hPrevFrameData->sbr_invf_mode,
                    bUseLP
                    );

    calculateSbrEnvelope(&hSbrDec->SbrCalculateEnvelope,
                          hHeaderData,
                          hFrameData,
                          QmfBufferReal,
                          QmfBufferImag,
                          degreeAlias,
                          bUseLP
                          );
     
     
    for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) {
      
      hPrevFrameData->sbr_invf_mode[i] = hFrameData->sbr_invf_mode[i];
    }
     
    hPrevFrameData->coupling = hFrameData->coupling;
    hPrevFrameData->xposCtrl = hFrameData->xposCtrl;
     
    hPrevFrameData->ampRes = hFrameData->ampResolutionCurrentFrame;
  }
  else 
	{
    memset(hSbrDec->LppTrans.lpcFilterStatesReal, 0, sizeof(hSbrDec->LppTrans.lpcFilterStatesReal));
		
    if (!bUseLP)
    {
      memset(hSbrDec->LppTrans.lpcFilterStatesImag, 0, sizeof(hSbrDec->LppTrans.lpcFilterStatesImag));
		
    }
  }
   
  if(hSbrDec->bApplyQmfLp)
	{    
    for (i = 0; i < noCols; i++){
      memset(QmfBufferReal[i] + hSbrDec->qmfLpChannel,0,(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel)*sizeof(float));
    
    if (!bUseLP)
    {
      memset(QmfBufferImag[i] + hSbrDec->qmfLpChannel,0,(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel)*sizeof(float));
    }
    }
  }
  if(hHeaderData->channelMode==PS_STEREO) 
	{
    cplxSynthesisQmfFiltering(QmfBufferReal,
                               QmfBufferImag,
                               timeOut-noCols*NO_SYNTHESIS_CHANNELS,
                               &hSbrDec->SynthesisQmfBank,
                               bUseLP,
                               h_ps_d,
                               1);
      
    cplxSynthesisQmfFiltering(QmfBufferReal,
                               QmfBufferImag,
                               timeOut-noCols*(NO_SYNTHESIS_CHANNELS - hSbrDec->SynthesisQmfBank.no_channels),
                               hSynthesisQmfBankRight,
                               bUseLP,
                               h_ps_d,
                               0);
  }
  else 
	{
    cplxSynthesisQmfFiltering(QmfBufferReal,
                               QmfBufferImag,
                               timeOut,
                               &hSbrDec->SynthesisQmfBank,
                               bUseLP,
                               h_ps_d,
                               0
                               );
  }

  
  for ( i=0; i<ov_len; i++ ) 
	{    
    for ( k=0; k<NO_SYNTHESIS_CHANNELS; k++ ) 
		{      
      QmfBufferReal[i][k] = QmfBufferReal[i+noCols][k];      
      if (!bUseLP)
      {        
        QmfBufferImag[i][k] = QmfBufferImag[i+noCols][k];
      }
    }
  }
  
}
/*
  \brief     Creates sbr decoder structure
  \return    errorCode
*/
int
createSbrDec (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA hHeaderData,
              int chan,
              int bApplyQmfLp,
              int sampleFreq
              )
{
  int err;
  int timeSlots = hHeaderData->numberTimeSlots;
  HANDLE_SBR_DEC hs = &(hSbrChannel->SbrDec);
  
   
  hs->bApplyQmfLp  = bApplyQmfLp;
   
  if(bApplyQmfLp == 8)
  {
    DIV(1);   
    hs->qmfLpChannel = (unsigned char)(8000.0f/sampleFreq * NO_SYNTHESIS_CHANNELS);
  }
   
  if(bApplyQmfLp == 4)
  {
    DIV(1);   
    hs->qmfLpChannel = (unsigned char)(4000.0f/sampleFreq * NO_SYNTHESIS_CHANNELS);
  }
    
  err = createSbrEnvelopeCalc (&hs->SbrCalculateEnvelope,
                               hHeaderData,
                               chan);
  
  if (err) {
    
    return (-1);
  }
   
  initSbrPrevFrameData(&hSbrChannel->PrevFrameData, timeSlots);
  
  return 0;
}
/*
  \brief     Creates sbr decoder structure
  \return    errorCode
*/
int
createSbrQMF (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA hHeaderData,
              int chan,
              int bDownSample
              )
{
  int err;
  int timeSlots = hHeaderData->numberTimeSlots;
  int noCols = timeSlots * hHeaderData->timeStep;
  HANDLE_SBR_DEC hs = &(hSbrChannel->SbrDec);
  
      
    
  createCplxAnalysisQmfBank(&hs->CodecQmfBank, noCols, hHeaderData->FreqBandData.lowSubband, hHeaderData->FreqBandData.highSubband);
    
  createCplxSynthesisQmfBank(&hs->SynthesisQmfBank, noCols, hHeaderData->FreqBandData.lowSubband, hHeaderData->FreqBandData.highSubband, bDownSample);
    
  err = createLppTransposer (&hs->LppTrans,
                             hHeaderData->FreqBandData.lowSubband,
                             hHeaderData->FreqBandData.v_k_master,
                             hHeaderData->FreqBandData.numMaster,
                             hs->SynthesisQmfBank.usb,
                             hs->CodecQmfBank.no_col,
                             hHeaderData->FreqBandData.freqBandTableNoise,
                             hHeaderData->FreqBandData.nNfb,
                             hHeaderData->outSampleRate,
                             chan);
  
  if (err) {
    
    return (-1);
  }
   
	memset(hs->sbr_OverlapBuffer, 0, sizeof(hs->sbr_OverlapBuffer));
  
  return 0;
}
/*
  \brief     resets sbr QMF structure
  \return    errorCode,
*/
int
resetSbrQMF (HANDLE_SBR_DEC hSbrDec,
             HANDLE_SBR_HEADER_DATA hHeaderData,
             int sbrChannel,
             int nChannels,
             HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData)
{
  int old_lsb = hSbrDec->SynthesisQmfBank.lsb;
  int new_lsb = hHeaderData->FreqBandData.lowSubband;
  int k,l, startBand, stopBand, startSlot;
  float * ptr;
  int bUseLP=1;
  float  *OverlapBufferReal[6];
#ifndef LP_SBR_ONLY
  float  *OverlapBufferImag[6];
#endif
  
    
     
  if (nChannels == 1) {
    
    bUseLP = 0;
  }
   
  ptr = hSbrDec->sbr_OverlapBuffer;
  
  for(l=0; l<6; l++) {
    
    OverlapBufferReal[l] = ptr;
    
    ptr += NO_SYNTHESIS_CHANNELS;
#ifndef LP_SBR_ONLY
    
    if (!bUseLP) {
      
      OverlapBufferImag[l] = ptr;
      
      ptr += NO_SYNTHESIS_CHANNELS;
    }
#endif
  }
   
  hSbrDec->SynthesisQmfBank.lsb = hHeaderData->FreqBandData.lowSubband;
  hSbrDec->SynthesisQmfBank.usb = hHeaderData->FreqBandData.highSubband;
   
  hSbrDec->CodecQmfBank.lsb = hSbrDec->SynthesisQmfBank.lsb;
  hSbrDec->CodecQmfBank.usb = hSbrDec->SynthesisQmfBank.usb;
  
  startBand = old_lsb;
  stopBand =  new_lsb;
    
  startSlot = hHeaderData->timeStep * (hPrevFrameData->stopPos - hHeaderData->numberTimeSlots);
  
  for (l=startSlot; l<6 ; l++) 
	{
   
    for (k=startBand; k<stopBand; k++) {
      
      OverlapBufferReal[l][k] = 0;
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
        
        OverlapBufferImag[l][k] = 0;
      }
#endif
    }
  }
  
  if (sbrChannel == 0) {
      
    startBand = min(old_lsb,new_lsb);
      
    stopBand =  max(old_lsb,new_lsb);
    
    
    for (l=startBand; l<stopBand; l++) {
      
      hSbrDec->LppTrans.lpcFilterStatesReal[0][l] =  hSbrDec->LppTrans.lpcFilterStatesReal[1][l] = 0;
     hSbrDec->LppTrans.lpcFilterStatesImag[0][l] =  hSbrDec->LppTrans.lpcFilterStatesImag[1][l] = 0;
    }
  }
  
  return 0;
}
