/*
  SBR decoder frontend
*/
#include <string.h>
#include <assert.h>
#include "sbr_const.h"
#include "sbr_bitb.h"
#include "sbrdec/sbrdecoder.h"
#include "freq_sca.h"
#include "env_extr.h"
#include "sbr_dec.h"
#include "env_dec.h"
#include "sbr_crc.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "math/FloatFR.h"
#include "ps_bitdec.h"
#include "ps_dec.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
struct SBR_DECODER_INSTANCE
{
  SBR_CHANNEL      SbrChannel[MAXNRSBRCHANNELS];
  SBR_HEADER_DATA  sbr_header;
  struct PS_DEC ParametricStereoDec;
  SBR_CONCEAL_DATA SbrConcealData;
}sbrDecoderInstance;
static void map2channelSbrData2one(HANDLE_SBR_FRAME_DATA hFrameDataLeft, HANDLE_SBR_FRAME_DATA hFrameDataRight, HANDLE_SBR_HEADER_DATA hHeaderData);
/*!
  \brief     Set up SBR decoder phase 1
  \return    Handle
*/
SBRDECODER openSBR(int sampleRate, int samplesPerFrame, int bDownSample, int bApplyQmfLp)
{
  int i, err;
  SBR_CHANNEL *SbrChannel = &sbrDecoderInstance.SbrChannel[0];
  HANDLE_SBR_HEADER_DATA hHeaderData = &sbrDecoderInstance.sbr_header;
  HANDLE_SBR_CONCEAL_DATA hSbrConcealData = &sbrDecoderInstance.SbrConcealData;
  
  FloatFR_Init();  /* Not needed for a DSP implementation */
  
  initHeaderData(hHeaderData, sampleRate, samplesPerFrame);
  
  for (i = 0; i < MAXNRSBRCHANNELS; i++) 
	{
    err = createSbrDec (&(SbrChannel[i]), hHeaderData, i, bApplyQmfLp, sampleRate);
    
    if (err) 
		{      
      return NULL;
    }
  } 
  for (i = 0; i < MAXNRQMFCHANNELS; i++) 
	{
    err = createSbrQMF (&(SbrChannel[i]), hHeaderData, i, bDownSample);
    
    if (err) 
		{      
      return NULL;
    }
  }
  
   
  err = CreatePsDec(&sbrDecoderInstance.ParametricStereoDec, SbrChannel[0].SbrDec.SynthesisQmfBank.no_col);
  
  if (err) 
	{    
    return NULL;
  }
  
  hSbrConcealData->FrameOk = 1;
  hSbrConcealData->Bitstream.NrElements     = 1;
  hSbrConcealData->Bitstream.NrElementsCore = 1;
  
  for (i = 0; i < MAXNRELEMENTS; i++) {
    
    hSbrConcealData->Bitstream.sbrElement[i].ElementID     = 0;
    hSbrConcealData->Bitstream.sbrElement[i].ExtensionType = SBR_EXTENSION;
    hSbrConcealData->Bitstream.sbrElement[i].Payload       = 0;
  }
 
  return &sbrDecoderInstance;
}
/*!
  \brief     Delay SBR bitstream
  \return none
*/
static void
DelaySbrBitstr(HANDLE_SBR_CONCEAL_DATA hSbrConcealData,
               SBRBITSTREAM *hSbrBS,
               int *SbrFrameOK)
{
  int tmp;
  SBRBITSTREAM bs_tmp;
  
   
  tmp       = hSbrConcealData->Bitstream.sbrElement[0].ElementID;
   
  bs_tmp    = hSbrBS[0];
  hSbrBS[0] = hSbrConcealData->Bitstream;
  hSbrConcealData->Bitstream = bs_tmp;
  
  if(!(*SbrFrameOK))
  {
    
    hSbrConcealData->Bitstream.sbrElement[0].ElementID = tmp;
  }
   
  tmp = *SbrFrameOK;
  *SbrFrameOK = hSbrConcealData->FrameOk;
  
  hSbrConcealData->FrameOk = tmp;
  
}
/*!
  \brief     SBR decoder processing
 \return SBRDEC_OK if successfull, else error code
*/
SBR_ERROR
applySBR (SBRDECODER self,
          SBRBITSTREAM * Bitstr,
          float *timeData,
          int *numChannels,
          int SbrFrameOK,
          int bDownSample,
          int bBitstreamDownMix
          )
{
  unsigned char i;
  unsigned char dualMono = 0;
  int stereo = 0;
  int CRCLen = 0;
  int crcEnable = 0;
  int readHeader = 0;
  int err = 0;
  SBR_CHANNEL *SbrChannel = &self->SbrChannel[0];
  BIT_BUFFER bitBuf;
  HANDLE_SBR_HEADER_DATA hHeaderData = &self->sbr_header;
  SBR_HEADER_STATUS headerStatus = HEADER_NOT_INITIALIZED;
  int codecFrameSize = hHeaderData->codecFrameSize;
  SBR_SYNC_STATE initialSyncState = hHeaderData->syncState;
  HANDLE_SBR_CONCEAL_DATA hConcealData = &self->SbrConcealData;
  float *pWorkBuffer1 = &timeData[2*codecFrameSize];
  SBR_FRAME_DATA hFrameDataLeft;
  SBR_FRAME_DATA hFrameDataRight;
   
  DelaySbrBitstr(hConcealData, Bitstr, &SbrFrameOK);
   
  if (Bitstr->NrElements) 
	{     
    for (i=0; i<Bitstr->NrElements; i++) {
      /* Save last error flag */
      
      hHeaderData->prevFrameErrorFlag = hHeaderData->frameErrorFlag;
       
      if (Bitstr->NrElements == 2) {
        
        dualMono = 1;
      }
      else {
        
        switch (Bitstr->sbrElement[i].ElementID) {
          case SBR_ID_SCE:
            
            stereo = 0;
            break;
          case SBR_ID_CPE:
            
            stereo = 1;
            break;
          default:
            
            SbrFrameOK = 0;
        }
      }
       
      initBitBuffer (&bitBuf,
                     Bitstr->sbrElement[i].Data,
                     Bitstr->sbrElement[i].Payload * 8) ;
      
      getbits (&bitBuf, LEN_NIBBLE);
      
      if (SbrFrameOK) {
         
        if (Bitstr->sbrElement[i].ExtensionType == SBR_EXTENSION_CRC) {
          
          crcEnable = 1;
           
          CRCLen = 8*(Bitstr->sbrElement[i].Payload-1)+4 - SI_SBR_CRC_BITS;
          
          if (CRCLen < 0) {
            
            crcEnable = 0;
            SbrFrameOK = 0;
          }
        }
        
        if (crcEnable)
        {
          
          SbrFrameOK = SbrCrcCheck (&bitBuf,
                                    CRCLen);
        }
        
        readHeader = getbits (&bitBuf, 1);
        
        if (SbrFrameOK){
          int lr;
          
          if (readHeader) {
            
            headerStatus = sbrGetHeaderData (hHeaderData,
                                             &bitBuf,
                                             (SBR_ELEMENT_ID)Bitstr->sbrElement[i].ElementID);
             
            if (headerStatus == HEADER_NOT_INITIALIZED) {
              
              return SBRDEC_NOT_INITIALIZED;
            }
             
            if (headerStatus == HEADER_RESET) {
              
              err = resetFreqBandTables(hHeaderData);
              
              for (lr = 0 ; lr < MAXNRSBRCHANNELS; lr++) {
                  
                resetSbrEnvelopeCalc (&(SbrChannel[lr].SbrDec.SbrCalculateEnvelope));
              }
              
              for (lr = 0 ; lr < MAXNRQMFCHANNELS; lr++) {
                 
                err |= resetSbrQMF (&(SbrChannel[lr].SbrDec),
                                    hHeaderData,
                                    lr,
                                    *numChannels,
                                    &SbrChannel[lr].PrevFrameData);
              }
              
              if (err==0) {
                
                hHeaderData->syncState = SBR_ACTIVE;
              }
            }
          } // if (readHeader)
            
          if (err || hHeaderData->syncState == SBR_NOT_INITIALIZED) {
              
            if (err && hHeaderData->syncState == SBR_NOT_INITIALIZED) {
              
              return SBRDEC_NOT_INITIALIZED;
            }
             
            initHeaderData( hHeaderData,
                            hHeaderData->outSampleRate >> 1,
                            codecFrameSize);
            
            err = resetFreqBandTables(hHeaderData);
            
            hHeaderData->FreqBandData.lowSubband = NO_ANALYSIS_CHANNELS;
            hHeaderData->FreqBandData.highSubband = NO_ANALYSIS_CHANNELS;
             /* SbrChannel[lr] */
            
            for (lr = 0 ; lr < MAXNRSBRCHANNELS; lr++) {
                
              resetSbrEnvelopeCalc (&(SbrChannel[lr].SbrDec.SbrCalculateEnvelope));
            }
             /* SbrChannel[lr] */
            
            for (lr = 0 ; lr < MAXNRQMFCHANNELS; lr++) {
               
              err |= resetSbrQMF (&(SbrChannel[lr].SbrDec),
                                  hHeaderData,
                                  lr,
                                  *numChannels,
                                  &SbrChannel[lr].PrevFrameData);
            }
            
            hHeaderData->syncState = UPSAMPLING;
          }
           
          if (hHeaderData->syncState == SBR_ACTIVE) {
            
            if (dualMono) {
              
              if (i == 0) {
                 
                hFrameDataLeft.xposCtrl = max(0, SbrChannel[i].PrevFrameData.xposCtrl);
                
                SbrFrameOK = sbrGetSingleChannelElement(hHeaderData,
                                                        &hFrameDataLeft,
                                                        NULL,
                                                        &bitBuf);
              }
              else {
                 
                hFrameDataRight.xposCtrl = max(0, SbrChannel[i].PrevFrameData.xposCtrl);
                
                SbrFrameOK = sbrGetSingleChannelElement(hHeaderData,
                                                        &hFrameDataRight,
                                                        NULL,
                                                        &bitBuf);
              }
            }
            else {
               
              hFrameDataLeft.xposCtrl = max(0, SbrChannel[i].PrevFrameData.xposCtrl);
              
              if (stereo) {
                 
                hFrameDataRight.xposCtrl = max(0, SbrChannel[i+1].PrevFrameData.xposCtrl);
                
                SbrFrameOK = sbrGetChannelPairElement(hHeaderData,
                                                      &hFrameDataLeft,
                                                      &hFrameDataRight,
                                                      &bitBuf);
              }
              else
              {
                
                if (bBitstreamDownMix)
                {
                  
                  sbrDecoderInstance.ParametricStereoDec.bForceMono = 1;
                }
                else
                {
                  
                  sbrDecoderInstance.ParametricStereoDec.bForceMono = 0;
                }
                
                SbrFrameOK = sbrGetSingleChannelElement(hHeaderData,
                                                        &hFrameDataLeft,
                                                        &sbrDecoderInstance.ParametricStereoDec,
                                                        &bitBuf);
              }
            }
            {
              int payloadbits = GetNrBitsRead (&bitBuf);
              int fillbits = (8 - (payloadbits & 7)) & 7;
                 
                
              if ((payloadbits + fillbits) != 8 * Bitstr->sbrElement[i].Payload)
              {
                
                SbrFrameOK = 0;
              }
            }
           }
        }
      }
        
      if (!SbrFrameOK || headerStatus == CONCEALMENT) {
        
        hHeaderData->frameErrorFlag = 1;
      }
    }
  }
  else {
     
    if (hHeaderData->syncState != SBR_NOT_INITIALIZED)
    {
      
      hHeaderData->syncState = UPSAMPLING;
    }
      
    stereo = ( *numChannels == 2) ? 1 : 0 ;
  }
    
  if ((hHeaderData->syncState == SBR_NOT_INITIALIZED) ||
      (initialSyncState == SBR_NOT_INITIALIZED && SbrFrameOK==0) ) {
    
    hHeaderData->syncState = SBR_NOT_INITIALIZED;
    
    return SBRDEC_NOT_INITIALIZED;
  }
   
  if (hHeaderData->syncState == SBR_ACTIVE) {
    
    err = 0;
       
    if ( (headerStatus == HEADER_RESET) ||
         (hFrameDataLeft.xposCtrl != SbrChannel[0].PrevFrameData.xposCtrl) ) {
       
      err |= resetLppTransposer (&SbrChannel[0].SbrDec.LppTrans,
                                 hFrameDataLeft.xposCtrl,
                                 hHeaderData->FreqBandData.lowSubband,
                                 hHeaderData->FreqBandData.v_k_master,
                                 hHeaderData->FreqBandData.numMaster,
                                 hHeaderData->FreqBandData.freqBandTableNoise,
                                 hHeaderData->FreqBandData.nNfb,
                                 hHeaderData->FreqBandData.highSubband,
                                 hHeaderData->outSampleRate);
         
      err |= ResetLimiterBands ( hHeaderData->FreqBandData.limiterBandTable,
                                 &hHeaderData->FreqBandData.noLimiterBands,
                                 hHeaderData->FreqBandData.freqBandTable[LO],
                                 hHeaderData->FreqBandData.nSfb[LO],
                                 SbrChannel[0].SbrDec.LppTrans.Settings.patchParam,
                                 SbrChannel[0].SbrDec.LppTrans.Settings.noOfPatches,
                                 hHeaderData->limiterBands);
    }
      
    if ( (MAXNRQMFCHANNELS>1) && 
         (stereo || dualMono) &&
         ((headerStatus == HEADER_RESET) ||
          (hFrameDataRight.xposCtrl != SbrChannel[1].PrevFrameData.xposCtrl)) ) {
       
      err |= resetLppTransposer (&SbrChannel[1].SbrDec.LppTrans,
                                 hFrameDataRight.xposCtrl,
                                 hHeaderData->FreqBandData.lowSubband,
                                 hHeaderData->FreqBandData.v_k_master,
                                 hHeaderData->FreqBandData.numMaster,
                                 hHeaderData->FreqBandData.freqBandTableNoise,
                                 hHeaderData->FreqBandData.nNfb,
                                 hHeaderData->FreqBandData.highSubband,
                                 hHeaderData->outSampleRate);
         
      err |= ResetLimiterBands ( hHeaderData->FreqBandData.limiterBandTable,
                                 &hHeaderData->FreqBandData.noLimiterBands,
                                 hHeaderData->FreqBandData.freqBandTable[LO],
                                 hHeaderData->FreqBandData.nSfb[LO],
                                 SbrChannel[1].SbrDec.LppTrans.Settings.patchParam,
                                 SbrChannel[1].SbrDec.LppTrans.Settings.noOfPatches,
                                 hHeaderData->limiterBands);
    }
    
    if (err) {
      
      hHeaderData->syncState = UPSAMPLING;
    }
    else {
      
      if (dualMono)  {
        
        decodeSbrData (hHeaderData,
                       &hFrameDataLeft,
                       &SbrChannel[0].PrevFrameData,
                       NULL,
                       NULL);
        
        decodeSbrData (hHeaderData,
                       &hFrameDataRight,
                       &SbrChannel[1].PrevFrameData,
                       NULL,
                       NULL);
      }
      else {
         
        decodeSbrData (hHeaderData,
                       &hFrameDataLeft,
                       &SbrChannel[0].PrevFrameData,
                       (stereo) ? &hFrameDataRight : NULL,
                       (stereo) ? &SbrChannel[1].PrevFrameData : NULL);
        
#ifndef MONO_ONLY
         
        if(hHeaderData->channelMode == PS_STEREO ) {
           
          DecodePs(&sbrDecoderInstance.ParametricStereoDec);
        }
#endif /* #ifndef MONO_ONLY */
        
      }
    }
  }
    
  if ( initialSyncState == SBR_NOT_INITIALIZED && hHeaderData->frameErrorFlag ) {
    
    hHeaderData->syncState = SBR_NOT_INITIALIZED;
    
    return SBRDEC_NOT_INITIALIZED;
  }
   
  if(hHeaderData->syncState == SBR_ACTIVE){
     
    SbrChannel[0].PrevFrameData.stopPos = hFrameDataLeft.frameInfo.borders[hFrameDataLeft.frameInfo.nEnvelopes];
     
    if(stereo || dualMono)
    {
       
      SbrChannel[1].PrevFrameData.stopPos = hFrameDataRight.frameInfo.borders[hFrameDataRight.frameInfo.nEnvelopes];
    }
  }
   
  if(stereo && bBitstreamDownMix){
    int kk;
     
    if(hHeaderData->syncState == SBR_ACTIVE){
      
      map2channelSbrData2one(&hFrameDataLeft,&hFrameDataRight,hHeaderData);
    }
     /* timeData[kk] */
    
    for(kk = 0; kk < codecFrameSize; kk++){
       DIV(1); 
      timeData[kk] = (timeData[kk] + timeData[kk + codecFrameSize])/2.0f;
      
      timeData[kk + codecFrameSize] = timeData[kk];
    }
  }
    
  sbr_dec(&SbrChannel[0].SbrDec,
           timeData,
           pWorkBuffer1,
           InterimResult,
           hHeaderData,
           &hFrameDataLeft,
           &SbrChannel[0].PrevFrameData,
           (hHeaderData->syncState == SBR_ACTIVE),
           &sbrDecoderInstance.ParametricStereoDec,
           &SbrChannel[1].SbrDec.SynthesisQmfBank, 
           *numChannels);
   
  if (!bBitstreamDownMix && (stereo || dualMono)) {
        
    memcpy( timeData, pWorkBuffer1, codecFrameSize*sizeof(float) );
        
    memcpy( &hFrameDataLeft, &hFrameDataRight, sizeof(SBR_FRAME_DATA) );
    
    hFrameDataRight = hFrameDataLeft;
        STORE(codecFrameSize);
    memcpy( InterimResult, pWorkBuffer1+codecFrameSize, codecFrameSize*sizeof(float) );
      
    sbr_dec(&SbrChannel[1].SbrDec,
             timeData + codecFrameSize,
             pWorkBuffer1,
             InterimResult,
             hHeaderData,
             &hFrameDataRight,
             &SbrChannel[1].PrevFrameData,
             (hHeaderData->syncState == SBR_ACTIVE),
             NULL,
             NULL,
             *numChannels);
    
    if(bDownSample)
		{
      memcpy( timeData+codecFrameSize, pWorkBuffer1, codecFrameSize*sizeof(float) );
    }
    else
		{
      memcpy( &(timeData[codecFrameSize]), InterimResult, codecFrameSize*sizeof(float) );
    }
  }
  else {
     
    if(hHeaderData->channelMode == PS_STEREO) {
      
      *numChannels = 2;
    }
    else  {
      
      if(bDownSample){
            STORE(codecFrameSize);
        memcpy( timeData, pWorkBuffer1, codecFrameSize*sizeof(float) );
        
            STORE(codecFrameSize);
        memcpy( timeData+codecFrameSize, pWorkBuffer1, codecFrameSize*sizeof(float) );
      }
      else{
            STORE(2*codecFrameSize);
        memcpy( timeData, pWorkBuffer1, 2*codecFrameSize*sizeof(float) );
      }
    }
  }
  
  
  return SBRDEC_OK;
}
/*!
  \brief     SBR bitstream data conversion from stereo to mono.
 \return none.
*/
static void
map2channelSbrData2one(HANDLE_SBR_FRAME_DATA hFrameDataLeft,
                       HANDLE_SBR_FRAME_DATA hFrameDataRight,
                       HANDLE_SBR_HEADER_DATA hHeaderData){
  int band, env, envLeft, envRight, offset, offsetLeft,offsetRight, nScaleFactors;
  HANDLE_FREQ_BAND_DATA hFreq  = &hHeaderData->FreqBandData;
  FRAME_INFO *frameInfoLeft    = &hFrameDataLeft->frameInfo;
  FRAME_INFO *frameInfoRight   = &hFrameDataRight->frameInfo;
  FRAME_INFO frameInfoMerge;
  char addHarmonicsMerge[MAX_FREQ_COEFFS];
  unsigned char bordersTemp[MAX_ENVELOPES*2 +2];
  int nEnvelopes = 1;
  INVF_MODE sbr_invf_modeMerge[MAX_INVF_BANDS];
  float  iEnvelopeMerge[MAX_NUM_ENVELOPE_VALUES];
  float  sbrNoiseFloorLevelMerge[MAX_NUM_NOISE_VALUES];
  float*  iEnvelopeLeft           = hFrameDataLeft->iEnvelope;
  float*  iEnvelopeRight          = hFrameDataRight->iEnvelope;
  float*  sbrNoiseFloorLevelLeft  = hFrameDataLeft->sbrNoiseFloorLevel;
  float*  sbrNoiseFloorLevelRight = hFrameDataRight->sbrNoiseFloorLevel;
  unsigned char noSubbands   = hFreq->highSubband - hFreq->lowSubband;
  unsigned char noNoiseBands = hFreq->nNfb;
  unsigned char * nSfb       = hFreq->nSfb;
  
     
   /* sbr_invf_modeMerge[]
                  hFrameDataLeft->sbr_invf_mode[]
                  hFrameDataRight->sbr_invf_mode[]
               */
  
  for(band = 0; band < noNoiseBands; band++){
      
    sbr_invf_modeMerge[band] = max(hFrameDataLeft->sbr_invf_mode[band],
                                   hFrameDataRight->sbr_invf_mode[band]);
  }
   /* addHarmonicsMerge[]
                  hFrameDataLeft->addHarmonics[]
                  hFrameDataRight->addHarmonics[]
               */
  
  for(band = 0; band < noSubbands; band++){
     
    addHarmonicsMerge[band] = (hFrameDataLeft->addHarmonics[band] ||
                               hFrameDataRight->addHarmonics[band]);
  }
     
  bordersTemp[0] = max(frameInfoLeft->borders[0], frameInfoRight->borders[0]);
     
  bordersTemp[nEnvelopes] = max(frameInfoLeft->borders[frameInfoLeft->nEnvelopes],
                                frameInfoRight->borders[frameInfoRight->nEnvelopes]);
  
  envLeft = 0;
   /* frameInfoLeft->borders[] */
  
  while(frameInfoLeft->borders[envLeft] <= bordersTemp[0]) {
     /* while() condition */
    envLeft++;
  }
  
  envRight = 0;
   /* frameInfoRight->borders[] */
  
  while(frameInfoRight->borders[envRight] <= bordersTemp[0]) {
     /* while() condition */
    envRight++;
  }
   /* bordersTemp[] */
   
  while(envLeft <= frameInfoLeft->nEnvelopes && envRight <= frameInfoRight->nEnvelopes){
    unsigned char borderLeftChannel  = frameInfoLeft->borders[envLeft];
    unsigned char borderRightChannel = frameInfoRight->borders[envRight];
     
      /* while() condition */
    
    bordersTemp[nEnvelopes + 1] = bordersTemp[nEnvelopes];
     
    if (borderLeftChannel < borderRightChannel){
      
      bordersTemp[nEnvelopes] = frameInfoLeft->borders[envLeft];
      envLeft++;
    }
    else{
      
      bordersTemp[nEnvelopes] = frameInfoRight->borders[envRight];
      envRight++;
    }
    
    nEnvelopes++;
  }
   /* bordersTemp[] */
  
  for(env = 0; env < nEnvelopes - 1; env++){
    
    while(bordersTemp[env + 1] - bordersTemp[env] < 2 && env < nEnvelopes - 1){
        /* while() condition */
          STORE(nEnvelopes - env - 1);
      memcpy(bordersTemp + env + 1, bordersTemp + env  + 2, (nEnvelopes - env - 1)*sizeof(unsigned char));
      
      nEnvelopes--;
    }
  }
    
  if(bordersTemp[nEnvelopes] - bordersTemp[nEnvelopes - 1] < 2){
    
    bordersTemp[nEnvelopes - 1] = bordersTemp[nEnvelopes];
    
    nEnvelopes--;
  }
  
  while(nEnvelopes > 5){
     /* bordersTemp[] */
    
    for(env = nEnvelopes; env > 0; env--){
       
      if(bordersTemp[env] - bordersTemp[env - 1] < 4)
        break;
    }
        STORE(nEnvelopes - env - 1);
    memcpy(bordersTemp + env - 1, bordersTemp + env, (nEnvelopes - env + 1 )*sizeof(unsigned char));
    
    nEnvelopes--;
  }
  
  frameInfoMerge.nEnvelopes = nEnvelopes;
   /* frameInfoMerge.borders[]
                  bordersTemp[]
               */
  
  for(env = 0; env <= nEnvelopes; env++){
    
    frameInfoMerge.borders[env] = bordersTemp[env];
  }
    
  if(frameInfoLeft->tranEnv == -1){
    
    frameInfoMerge.tranEnv = frameInfoRight->tranEnv;
  }
  else{
      
    if(frameInfoRight->tranEnv == -1){
      
      frameInfoMerge.tranEnv = frameInfoLeft->tranEnv;
    }
    else{
        
      frameInfoMerge.tranEnv = min(frameInfoLeft->tranEnv, frameInfoRight->tranEnv);
    }
  }
     
  frameInfoMerge.nNoiseEnvelopes = max(frameInfoLeft->nNoiseEnvelopes,frameInfoRight->nNoiseEnvelopes);
   
  if(frameInfoMerge.nNoiseEnvelopes > 1){
     
    if(frameInfoLeft->nNoiseEnvelopes < frameInfoRight->nNoiseEnvelopes){
      
      frameInfoMerge.bordersNoise[0] = frameInfoMerge.borders[0];
      
      frameInfoMerge.bordersNoise[1] = frameInfoRight->bordersNoise[1];
       
      frameInfoMerge.bordersNoise[2] = frameInfoMerge.borders[frameInfoMerge.nEnvelopes];
    }
    else{
       
      if(frameInfoRight->nNoiseEnvelopes < frameInfoLeft->nNoiseEnvelopes){
        
        frameInfoMerge.bordersNoise[0] = frameInfoMerge.borders[0];
        
        frameInfoMerge.bordersNoise[1] = frameInfoLeft->bordersNoise[1];
         
        frameInfoMerge.bordersNoise[2] = frameInfoMerge.borders[frameInfoMerge.nEnvelopes];
      }
      else{
        
        frameInfoMerge.bordersNoise[0] = frameInfoMerge.borders[0];
           
        frameInfoMerge.bordersNoise[1] = min(frameInfoLeft->bordersNoise[1],frameInfoRight->bordersNoise[1]);
         
        frameInfoMerge.bordersNoise[2] = frameInfoMerge.borders[frameInfoMerge.nEnvelopes];
      }
    }
  }
  else{
    
    frameInfoMerge.bordersNoise[0] = frameInfoMerge.borders[0];
     
    frameInfoMerge.bordersNoise[1] = frameInfoMerge.borders[frameInfoMerge.nEnvelopes];
  }
   
  if(frameInfoMerge.nNoiseEnvelopes > 1){
     /* frameInfoMerge.borders[] */
    
    for(env = 1; env < frameInfoMerge.nEnvelopes; env++){
       
      if(frameInfoMerge.borders[env] >= frameInfoMerge.bordersNoise[1]){
        break;
      }
    }
    
    frameInfoMerge.bordersNoise[1] = frameInfoMerge.borders[env];
  }
  
  envLeft     = 0;
  envRight    = 0;
  offset      = 0;
  offsetRight = 0;
  offsetLeft  = 0;
   /* frameInfoLeft->freqRes[]
                  frameInfoRight->freqRes]
                  frameInfoMerge.borders[]
                  frameInfoMerge.freqRes[]
               */
  
  for(env = 0; env < frameInfoMerge.nEnvelopes;env++){
    unsigned char freqResLeft  = frameInfoLeft->freqRes[envLeft];
    unsigned char freqResRight = frameInfoRight->freqRes[envRight];
    unsigned char freqRes;
    int bandLeft  = 0;
    int bandRight = 0;
    int band;
     
     
    if(frameInfoMerge.borders[env + 1] > frameInfoLeft->borders[envLeft+1]){
       
      if(envLeft < frameInfoLeft->nEnvelopes - 1){
         
        offsetLeft += nSfb[freqResLeft];
        envLeft++;
      }
    }
     
    if(frameInfoMerge.borders[env + 1] > frameInfoRight->borders[envRight+1]){
       
      if(envRight < frameInfoRight->nEnvelopes - 1){
         
        offsetRight += nSfb[freqResRight];
        envRight++;
      }
    }
    
    freqResLeft  = frameInfoLeft->freqRes[envLeft];
    freqResRight = frameInfoRight->freqRes[envRight];
    
    freqRes      = freqResLeft || freqResRight;
    
    frameInfoMerge.freqRes[env] = freqRes;
     /* iEnvelopeMerge[]
                    iEnvelopeLeft[]
                    iEnvelopeRight[]
                 */
     
    for(band = 0; band < nSfb[freqRes]; band++){
      unsigned char ui      = hFreq->freqBandTable[freqRes     ][band+1];
      unsigned char uiLeft  = hFreq->freqBandTable[freqResLeft ][bandLeft+1];
      unsigned char uiRight = hFreq->freqBandTable[freqResRight][bandRight+1];
        
       
      if(ui > uiLeft) {
        
        bandLeft++;
      }
       
      if(ui > uiRight) {
        
        bandRight++;
      }
       DIV(1); 
      iEnvelopeMerge[band + offset] = (iEnvelopeLeft[bandLeft + offsetLeft] +
                                       iEnvelopeRight[bandRight + offsetRight])/2.0f;
    }
     
    offset      +=  nSfb[freqRes];
  }
  
  nScaleFactors = offset;
  
  envLeft     = 0;
  envRight    = 0;
  offset      = 0;
  offsetRight = 0;
  offsetLeft  = 0;
   /* frameInfoMerge.bordersNoise[]
                  frameInfoLeft->bordersNoise[]
                  frameInfoRight->bordersNoise[]
               */
  
  for(env = 0; env < frameInfoMerge.nNoiseEnvelopes; env++){
    int band;
      
    if(frameInfoMerge.bordersNoise[env] > frameInfoLeft->bordersNoise[envLeft] &&
       envLeft < frameInfoLeft->nNoiseEnvelopes - 1){
      
      envLeft++;
      offsetLeft += noNoiseBands;
    }
      
    if(frameInfoMerge.bordersNoise[env] > frameInfoRight->bordersNoise[envRight] &&
      envRight < frameInfoRight->nNoiseEnvelopes - 1){
      
      envRight++;
      offsetRight += noNoiseBands;
    }
     /* sbrNoiseFloorLevelMerge[]
                    sbrNoiseFloorLevelLeft[]
                    sbrNoiseFloorLevelRight[]
                 */
    
    for(band = 0; band < noNoiseBands; band++){
       DIV(1); 
      sbrNoiseFloorLevelMerge[band + offset] = (sbrNoiseFloorLevelLeft[band + offsetLeft] +
                                                sbrNoiseFloorLevelRight[band + offsetRight])/2.0f;
    }
    
    offset += noNoiseBands;
  }
      STORE(sizeof(FRAME_INFO));
  memcpy(frameInfoLeft,&frameInfoMerge,sizeof(FRAME_INFO));
      STORE(MAX_ENVELOPES+1);
  memcpy(frameInfoLeft->borders, frameInfoMerge.borders, (MAX_ENVELOPES+1)*sizeof(unsigned char));
      STORE(MAX_ENVELOPES);
  memcpy(frameInfoLeft->freqRes, frameInfoMerge.freqRes, (MAX_ENVELOPES)*sizeof(unsigned char));
      STORE(MAX_NOISE_ENVELOPES+1);
  memcpy(frameInfoLeft->bordersNoise, frameInfoMerge.bordersNoise, (MAX_NOISE_ENVELOPES+1)*sizeof(unsigned char));
      STORE(MAX_NUM_ENVELOPE_VALUES);
  memcpy(iEnvelopeLeft, iEnvelopeMerge, MAX_NUM_ENVELOPE_VALUES*sizeof(float));
      STORE(MAX_NUM_NOISE_VALUES);
  memcpy(sbrNoiseFloorLevelLeft, sbrNoiseFloorLevelMerge, MAX_NUM_NOISE_VALUES*sizeof(float));
      STORE(MAX_FREQ_COEFFS);
  memcpy(hFrameDataLeft->addHarmonics, addHarmonicsMerge,MAX_FREQ_COEFFS*sizeof(char));
      STORE(MAX_INVF_BANDS);
  memcpy(hFrameDataLeft->sbr_invf_mode, sbr_invf_modeMerge, MAX_INVF_BANDS*sizeof(INVF_MODE));
   
  hFrameDataLeft->nScaleFactors = nScaleFactors;
  
}
