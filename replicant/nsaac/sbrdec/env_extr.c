/*
  Envelope extraction
*/
#include <string.h>
#include <assert.h>
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "env_extr.h"
#include "huff_dec.h"
#include "sbr_const.h"       /* Various defines */
#include "math/FloatFR.h"
#include "ps_bitdec.h"
#include "ps_dec.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
static int extractFrameInfo (HANDLE_BIT_BUFFER hBitBuf,
                             HANDLE_SBR_HEADER_DATA hHeaderData,
                             HANDLE_SBR_FRAME_DATA h_frame_data);
static int sbrGetEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                           HANDLE_SBR_FRAME_DATA h_frame_data,
                           HANDLE_BIT_BUFFER hBitBuf);
static void sbrGetDirectionControlData (HANDLE_SBR_FRAME_DATA hFrameData,
                                        HANDLE_BIT_BUFFER hBitBuf);
static void sbrGetNoiseFloorData (HANDLE_SBR_HEADER_DATA hHeaderData,
                                  HANDLE_SBR_FRAME_DATA h_frame_data,
                                  HANDLE_BIT_BUFFER hBitBuf);
static int checkFrameInfo (FRAME_INFO *pFrameInfo, int numberOfTimeSlots);
/*
  \brief     Initialize SBR header data
*/
void
initHeaderData (SBR_HEADER_DATA *hHeaderData,
                int sampleRate,
                int samplesPerFrame)
{
  HANDLE_FREQ_BAND_DATA hFreq = &hHeaderData->FreqBandData;
  
  memcpy(hHeaderData, &sbr_defaultHeader, sizeof(SBR_HEADER_DATA));
   
  hFreq->freqBandTable[LOW_RES]  = hFreq->freqBandTableLo;
  hFreq->freqBandTable[HIGH_RES] = hFreq->freqBandTableHi;
   
   
  hHeaderData->codecFrameSize = samplesPerFrame;
    
  hHeaderData->outSampleRate = SBR_UPSAMPLE_FAC * sampleRate;
     
  hHeaderData->numberTimeSlots = samplesPerFrame >> (4 + hHeaderData->timeStep);
  
}
/*
  \brief   Initialize the SBR_PREV_FRAME_DATA struct
*/
void
initSbrPrevFrameData (HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                      int timeSlots)
{
  int i;
  
  
  for (i=0; i < MAX_FREQ_COEFFS; i++)
  {
    
    h_prev_data->sfb_nrg_prev[i] = 0.0f;
  }
  
  for (i=0; i < MAX_NOISE_COEFFS; i++)
  {
    
    h_prev_data->prevNoiseLevel[i] = 0.0f;
  }
  
  for (i=0; i < MAX_INVF_BANDS; i++)
  {
    
    h_prev_data->sbr_invf_mode[i] = INVF_OFF;
  }
   
  h_prev_data->stopPos = timeSlots;
  h_prev_data->coupling = COUPLING_OFF;
  h_prev_data->ampRes = 0;
  h_prev_data->xposCtrl = -1;
  
}
/*
  \brief   Read header data from bitstream
  \return  error status - 0 if ok
*/
SBR_HEADER_STATUS
sbrGetHeaderData (SBR_HEADER_DATA *h_sbr_header,
                  HANDLE_BIT_BUFFER hBitBuf,
                  SBR_ELEMENT_ID id_sbr)
{
  SBR_HEADER_DATA lastHeader;
  int dummy;
  int headerExtra1, headerExtra2;
  
      STORE(sizeof(SBR_HEADER_DATA));
  memcpy (&lastHeader, h_sbr_header, sizeof(SBR_HEADER_DATA));
    
  h_sbr_header->ampResolution = (unsigned char)getbits (hBitBuf, SI_SBR_AMP_RES_BITS);
    
  h_sbr_header->startFreq = (unsigned char)getbits (hBitBuf, SI_SBR_START_FREQ_BITS);
    
  h_sbr_header->stopFreq = (unsigned char)getbits (hBitBuf, SI_SBR_STOP_FREQ_BITS);
    
  h_sbr_header->xover_band = (unsigned char)getbits (hBitBuf, SI_SBR_XOVER_BAND_BITS);
   
  dummy = (unsigned char)getbits (hBitBuf, SI_SBR_RESERVED_BITS_HDR);
   
  headerExtra1 = (unsigned char)getbits (hBitBuf, SI_SBR_HEADER_EXTRA_1_BITS);
   
  headerExtra2 = (unsigned char)getbits (hBitBuf, SI_SBR_HEADER_EXTRA_2_BITS);
   
  if (id_sbr == SBR_ID_SCE) {
     
    h_sbr_header->channelMode = MONO;
  }
  else {
     
    h_sbr_header->channelMode = STEREO;
  }
  
  if(headerExtra1) {
      
    h_sbr_header->freqScale = (unsigned char)getbits (hBitBuf, SI_SBR_FREQ_SCALE_BITS);
      
    h_sbr_header->alterScale = (unsigned char)getbits (hBitBuf, SI_SBR_ALTER_SCALE_BITS);
      
    h_sbr_header->noise_bands = (unsigned char)getbits (hBitBuf, SI_SBR_NOISE_BANDS_BITS);
  }
  else {
     
    h_sbr_header->freqScale   = SBR_FREQ_SCALE_DEFAULT;
    h_sbr_header->alterScale  = SBR_ALTER_SCALE_DEFAULT;
    h_sbr_header->noise_bands = SBR_NOISE_BANDS_DEFAULT;
  }
  
  if(headerExtra2) {
      
    h_sbr_header->limiterBands = (unsigned char)getbits (hBitBuf, SI_SBR_LIMITER_BANDS_BITS);
      
    h_sbr_header->limiterGains = (unsigned char)getbits (hBitBuf, SI_SBR_LIMITER_GAINS_BITS);
      
    h_sbr_header->interpolFreq = (unsigned char)getbits (hBitBuf, SI_SBR_INTERPOL_FREQ_BITS);
      
    h_sbr_header->smoothingLength = (unsigned char)getbits (hBitBuf, SI_SBR_SMOOTHING_LENGTH_BITS);
  }
  else {
     
    h_sbr_header->limiterBands    = SBR_LIMITER_BANDS_DEFAULT;
    h_sbr_header->limiterGains    = SBR_LIMITER_GAINS_DEFAULT;
    h_sbr_header->interpolFreq    = SBR_INTERPOL_FREQ_DEFAULT;
    h_sbr_header->smoothingLength = SBR_SMOOTHING_LENGTH_DEFAULT;
  }
    
  if(h_sbr_header->syncState != SBR_ACTIVE ||
     lastHeader.startFreq   != h_sbr_header->startFreq   ||
     lastHeader.stopFreq    != h_sbr_header->stopFreq    ||
     lastHeader.xover_band  != h_sbr_header->xover_band  ||
     lastHeader.freqScale   != h_sbr_header->freqScale   ||
     lastHeader.alterScale  != h_sbr_header->alterScale  ||
     lastHeader.noise_bands != h_sbr_header->noise_bands) {
    
    return HEADER_RESET; /* New settings */
  }
  
  return HEADER_OK; /* Everything ok */
}
/*
  \brief   Get missing harmonics parameters
  \return  error status - 0 if ok
*/
static int
sbrGetSyntheticCodedData(HANDLE_SBR_HEADER_DATA hHeaderData,
                         HANDLE_SBR_FRAME_DATA hFrameData,
                         HANDLE_BIT_BUFFER     hBitBuf)
{
  int i, bitsRead = 0;
  int flag = getbits(hBitBuf,1);
  
    
  
  bitsRead++;
  
  if(flag){
     /* hFrameData->addHarmonics[] */
     
    for(i=0;i<hHeaderData->FreqBandData.nSfb[HI];i++){
       
      hFrameData->addHarmonics[i]  = (unsigned char)getbits (hBitBuf, SI_SBR_ADDITIONAL_HARMONICS_BITS );
      
      bitsRead++;
    }
  }
  else {
     /* hFrameData->addHarmonics[] */
    
    for(i=0; i<MAX_FREQ_COEFFS; i++)
    {
      
      hFrameData->addHarmonics[i]  = 0;
    }
  }
  
  return(bitsRead);
}
/*
  \brief      Reads extension data from the bitstream
*/
static int extractExtendedData(HANDLE_BIT_BUFFER     hBitBuf,
                               HANDLE_SBR_HEADER_DATA hHeaderData,
                               HANDLE_PS_DEC       hPs
                               )
{
  int extended_data;
  int i,nBitsLeft;
  
  
  extended_data = getbits(hBitBuf, SI_SBR_EXTENDED_DATA_BITS);
  
  if (extended_data) {
    int cnt, bPsRead;
    
    bPsRead = 0;
    
    cnt = getbits(hBitBuf, SI_SBR_EXTENSION_SIZE_BITS);
     
    if (cnt == (1<<SI_SBR_EXTENSION_SIZE_BITS)-1)
    {
       
      cnt += getbits(hBitBuf, SI_SBR_EXTENSION_ESC_COUNT_BITS);
    }
    
    nBitsLeft = 8 * cnt;
    
    while (nBitsLeft > 7) {
      int extension_id = getbits(hBitBuf, SI_SBR_EXTENSION_ID_BITS);
       
      
      nBitsLeft -= SI_SBR_EXTENSION_ID_BITS;
      
      switch(extension_id) {
#ifndef MONO_ONLY
      case EXTENSION_ID_PS_CODING:
         
        if (!hPs) {
          
          return 0;
        }
          
        if (!hPs->bForceMono && !bPsRead) {
           
          nBitsLeft -= ReadPsData(hPs, hBitBuf, nBitsLeft);
          
          bPsRead = 1;
           
          hHeaderData->channelMode = PS_STEREO;
          break;
        }
#endif /* #ifndef MONO_ONLY */
      default:
        
        cnt = nBitsLeft >> 3;
        
        for (i=0; i<cnt; i++)
        {
          
          getbits(hBitBuf, 8);
        }
         
        nBitsLeft -= cnt * 8;
        break;
      }
    }
    
    getbits(hBitBuf, nBitsLeft);
  }
  
  return 1;
}
/*
  \brief   Read bitstream elements of one channel
  \return  SbrFrameOK:  1=ok, 0=error
*/
int
sbrGetSingleChannelElement (HANDLE_SBR_HEADER_DATA hHeaderData,
                            HANDLE_SBR_FRAME_DATA  hFrameData,
                            HANDLE_PS_DEC          hPs,
                            HANDLE_BIT_BUFFER      hBitBuf)
{
  int i, bit;
  
   
  hFrameData->coupling = COUPLING_OFF;
  
  bit = getbits(hBitBuf, 1);
  
  if (bit)
  {
    
    getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
  }
   
  if ( !extractFrameInfo (hBitBuf, hHeaderData, hFrameData) )
  {
    
    return 0;
  }
     
  if ( !checkFrameInfo (&hFrameData->frameInfo, hHeaderData->numberTimeSlots) )
  {
    
    return 0;
  }
  
  sbrGetDirectionControlData (hFrameData, hBitBuf);
    
  if (hFrameData->domain_vec[0]==FREQ)
  {
     
    hHeaderData->frameErrorFlag = 0;
  }

  for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) 
	{     
    hFrameData->sbr_invf_mode[i] =
      (INVF_MODE) getbits (hBitBuf, SI_SBR_INVF_MODE_BITS);
  }
   
  if ( !sbrGetEnvelope (hHeaderData, hFrameData, hBitBuf) )
  {
    
    return 0;
  }
  
  sbrGetNoiseFloorData (hHeaderData, hFrameData, hBitBuf);
  
  sbrGetSyntheticCodedData(hHeaderData, hFrameData, hBitBuf);
   
  if ( !extractExtendedData(hBitBuf, hHeaderData, hPs) )
  {
    
    return 0;
  }
  
  return 1;
}
/*
  \brief      Read bitstream elements of a channel pair
  \return     SbrFrameOK
*/
int
sbrGetChannelPairElement (HANDLE_SBR_HEADER_DATA hHeaderData,
                          HANDLE_SBR_FRAME_DATA hFrameDataLeft,
                          HANDLE_SBR_FRAME_DATA hFrameDataRight,
                          HANDLE_BIT_BUFFER hBitBuf)
{
  int i, bit;
  
  
  bit = getbits(hBitBuf, 1);
  
  if (bit) {
    
    getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
    
    getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
  }
    
  if (hHeaderData->channelMode != STEREO) {
     
    hHeaderData->syncState = UPSAMPLING;
    
    return 0;
  }
  
  bit = getbits (hBitBuf, SI_SBR_COUPLING_BITS);
  
  if (bit) {
     
    hFrameDataLeft->coupling = COUPLING_LEVEL;
    hFrameDataRight->coupling = COUPLING_BAL;
  }
  else {
     
    hFrameDataLeft->coupling = COUPLING_OFF;
    hFrameDataRight->coupling = COUPLING_OFF;
  }
   
  if ( !extractFrameInfo (hBitBuf, hHeaderData, hFrameDataLeft) )
  {
    
    return 0;
  }
     
  if ( !checkFrameInfo (&hFrameDataLeft->frameInfo, hHeaderData->numberTimeSlots) )
  {
    
    return 0;
  }
   
  if (hFrameDataLeft->coupling) {
        STORE(sizeof(FRAME_INFO));
    memcpy (&hFrameDataRight->frameInfo, &hFrameDataLeft->frameInfo, sizeof(FRAME_INFO));
  }
  else {
     
    if ( !extractFrameInfo (hBitBuf, hHeaderData, hFrameDataRight) )
    {
      
      return 0;
    }
       
    if ( !checkFrameInfo (&hFrameDataRight->frameInfo, hHeaderData->numberTimeSlots) )
    {
      
      return 0;
    }
  }
  
  sbrGetDirectionControlData (hFrameDataLeft, hBitBuf);
  
  sbrGetDirectionControlData (hFrameDataRight, hBitBuf);
     
  if ((hFrameDataLeft->domain_vec[0]==FREQ) && (hFrameDataRight->domain_vec[0]==FREQ))
  {
     
    hHeaderData->frameErrorFlag = 0;
  }
   /* hFrameDataLeft->sbr_invf_mode[] */
   
  for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) {
     
    hFrameDataLeft->sbr_invf_mode[i] = (INVF_MODE) getbits (hBitBuf, SI_SBR_INVF_MODE_BITS);
  }
   
  if (hFrameDataLeft->coupling) {
     /* hFrameDataRight->sbr_invf_mode[]
                    hFrameDataLeft->sbr_invf_mode[]
                 */
     
    for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) {
      
      hFrameDataRight->sbr_invf_mode[i] = hFrameDataLeft->sbr_invf_mode[i];
    }
     
    if ( !sbrGetEnvelope (hHeaderData, hFrameDataLeft, hBitBuf) ) {
      
      return 0;
    }
    
    sbrGetNoiseFloorData (hHeaderData, hFrameDataLeft, hBitBuf);
     
    if ( !sbrGetEnvelope (hHeaderData, hFrameDataRight, hBitBuf) ) {
      
      return 0;
    }
  }
  else {
     /* hFrameDataRight->sbr_invf_mode[] */
     
    for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) {
       
      hFrameDataRight->sbr_invf_mode[i] = (INVF_MODE) getbits (hBitBuf, SI_SBR_INVF_MODE_BITS);
    }
     
    if ( !sbrGetEnvelope (hHeaderData, hFrameDataLeft, hBitBuf ) )
    {
      
      return 0;
    }
     
    if ( !sbrGetEnvelope (hHeaderData, hFrameDataRight, hBitBuf) )
    {
      
      return 0;
    }
    
    sbrGetNoiseFloorData (hHeaderData, hFrameDataLeft, hBitBuf);
  }
  
  sbrGetNoiseFloorData (hHeaderData, hFrameDataRight, hBitBuf);
  
  sbrGetSyntheticCodedData(hHeaderData, hFrameDataLeft, hBitBuf);
  
  sbrGetSyntheticCodedData(hHeaderData, hFrameDataRight, hBitBuf);
   
  if ( !extractExtendedData(hBitBuf, hHeaderData, NULL) )
  {
    
    return 0;
  }
  
  return 1;
}
/*
  \brief   Read direction control data from bitstream
*/
void
sbrGetDirectionControlData (HANDLE_SBR_FRAME_DATA h_frame_data,
                            HANDLE_BIT_BUFFER hBitBuf)
{
  int i;
   
  for (i = 0; i < h_frame_data->frameInfo.nEnvelopes; i++) {
     
    h_frame_data->domain_vec[i] = (unsigned char)getbits (hBitBuf, SI_SBR_DOMAIN_BITS);
  }
   
  for (i = 0; i < h_frame_data->frameInfo.nNoiseEnvelopes; i++) {
     
    h_frame_data->domain_vec_noise[i] = (unsigned char)getbits (hBitBuf, SI_SBR_DOMAIN_BITS);
  }
}
/*
  \brief   Read noise-floor-level data from bitstream
*/
void
sbrGetNoiseFloorData (HANDLE_SBR_HEADER_DATA hHeaderData,
                      HANDLE_SBR_FRAME_DATA h_frame_data,
                      HANDLE_BIT_BUFFER hBitBuf)
{
  int i,j;
  int delta;
  COUPLING_MODE coupling;
  int noNoiseBands = hHeaderData->FreqBandData.nNfb;
  Huffman hcb_noiseF;
  Huffman hcb_noise;
  int envDataTableCompFactor;
  int start_bits = SI_SBR_START_NOISE_BITS_AMP_RES_3_0;
  int start_bits_balance = SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0;
  
    
   
  coupling = h_frame_data->coupling;
   
  if (coupling == COUPLING_BAL) {
     
    hcb_noise = (Huffman)&sbr_huffBook_NoiseBalance11T;
    hcb_noiseF = (Huffman)&sbr_huffBook_EnvBalance11F;
    envDataTableCompFactor = 1;
  }
  else {
     
    hcb_noise = (Huffman)&sbr_huffBook_NoiseLevel11T;
    hcb_noiseF = (Huffman)&sbr_huffBook_EnvLevel11F;
    envDataTableCompFactor = 0;
  }
   /* h_frame_data->domain_vec_noise[i]
                  h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands]
               */
  
  for (i=0; i<h_frame_data->frameInfo.nNoiseEnvelopes; i++) {
     
    if (h_frame_data->domain_vec_noise[i] == FREQ) {
       
      if (coupling == COUPLING_BAL) {
          
        h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands] =
          (float) (((int)getbits (hBitBuf,start_bits_balance)) << envDataTableCompFactor);
      }
      else {
         
        h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands] =
          (float) getbits (hBitBuf, start_bits);
      }
       /* h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands+j] */
      
      for (j = 1; j < noNoiseBands; j++) {
        
        delta = DecodeHuffmanCW(hcb_noiseF, hBitBuf);
         
        h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands+j] = (float) (delta << envDataTableCompFactor);
      }
    }
    else {
       /* h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands+j] */
      
      for (j = 0; j < noNoiseBands; j++) {
        
        delta = DecodeHuffmanCW(hcb_noise, hBitBuf);
         
        h_frame_data->sbrNoiseFloorLevel[i*noNoiseBands+j] = (float) (delta << envDataTableCompFactor);
      }
    }
  }
  
}
/*
  \brief   Read envelope data from bitstream
*/
int
sbrGetEnvelope (HANDLE_SBR_HEADER_DATA hHeaderData,
                HANDLE_SBR_FRAME_DATA h_frame_data,
                HANDLE_BIT_BUFFER hBitBuf)
{
  int i, j;
  unsigned char no_band[MAX_ENVELOPES];
  int delta = 0;
  int offset = 0;
  COUPLING_MODE coupling = h_frame_data->coupling;
  int ampRes = hHeaderData->ampResolution;
  int nEnvelopes = h_frame_data->frameInfo.nEnvelopes;
  int envDataTableCompFactor;
  int start_bits, start_bits_balance;
  Huffman hcb_t, hcb_f;
  
    
   
  h_frame_data->nScaleFactors = 0;
       
    if ( (h_frame_data->frameInfo.frameClass == FIXFIX) && (nEnvelopes == 1) ) {
      
      ampRes = SBR_AMP_RES_1_5;
    }
   
  h_frame_data->ampResolutionCurrentFrame = ampRes;
   
  if(ampRes == SBR_AMP_RES_3_0)
  {
    
    start_bits = SI_SBR_START_ENV_BITS_AMP_RES_3_0;
    start_bits_balance = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0;
  }
  else
  {
    
    start_bits = SI_SBR_START_ENV_BITS_AMP_RES_1_5;
    start_bits_balance = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5;
  }
   /* no_band[]
                  h_frame_data->frameInfo.freqRes[]
               */
  
  for (i = 0; i < nEnvelopes; i++) {
     
    no_band[i] = hHeaderData->FreqBandData.nSfb[h_frame_data->frameInfo.freqRes[i]];
    
    h_frame_data->nScaleFactors += no_band[i];
  }
    /* h_frame_data->nScaleFactors */
   
  if (h_frame_data->nScaleFactors > MAX_NUM_ENVELOPE_VALUES)
  {
    
    return 0;
  }
   
  if (coupling == COUPLING_BAL) {
    
    envDataTableCompFactor = 1;
     
    if (ampRes == SBR_AMP_RES_1_5) {
      
      hcb_t = (Huffman)&sbr_huffBook_EnvBalance10T;
      hcb_f = (Huffman)&sbr_huffBook_EnvBalance10F;
    }
    else {
      
      hcb_t = (Huffman)&sbr_huffBook_EnvBalance11T;
      hcb_f = (Huffman)&sbr_huffBook_EnvBalance11F;
    }
  }
  else {
    
    envDataTableCompFactor = 0;
     
    if (ampRes == SBR_AMP_RES_1_5) {
      
      hcb_t = (Huffman)&sbr_huffBook_EnvLevel10T;
      hcb_f = (Huffman)&sbr_huffBook_EnvLevel10F;
    }
    else {
      
      hcb_t = (Huffman)&sbr_huffBook_EnvLevel11T;
      hcb_f = (Huffman)&sbr_huffBook_EnvLevel11F;
    }
  }
   /* h_frame_data->domain_vec[]
                  no_band[]
                  h_frame_data->iEnvelope[]
               */
  
  for (j = 0, offset = 0; j < nEnvelopes; j++) {
     
    if (h_frame_data->domain_vec[j] == FREQ) {
       
      if (coupling == COUPLING_BAL) {
          
        h_frame_data->iEnvelope[offset] =
          (float) (( (int)getbits(hBitBuf, start_bits_balance)) << envDataTableCompFactor);
      }
      else {
         
        h_frame_data->iEnvelope[offset] =
          (float) getbits(hBitBuf, start_bits);
      }
    }
     /* h_frame_data->iEnvelope[] */
    
    for (i = (1 - h_frame_data->domain_vec[j]); i < no_band[j]; i++) {
       
      if (h_frame_data->domain_vec[j] == FREQ) {
        
        delta = DecodeHuffmanCW(hcb_f, hBitBuf);
      }
      else {
        
        delta = DecodeHuffmanCW(hcb_t, hBitBuf);
      }
       
      h_frame_data->iEnvelope[offset + i] = (float) (delta << envDataTableCompFactor);
    }
    
    offset += no_band[j];
  }
  
  return 1;
}
/*
  \brief   Extract the frame information (structure FRAME_INFO) from the bitstream
  \return  Zero for bitstream error, one for correct.
*/
int
extractFrameInfo (HANDLE_BIT_BUFFER hBitBuf,
                  HANDLE_SBR_HEADER_DATA hHeaderData,
                  HANDLE_SBR_FRAME_DATA h_frame_data)
{
  int pointer_bits = 0, nEnv = 0, b = 0, border, i, frameClass, n = 0,
    k, p, aL, aR, nL, nR,
    temp = 0, staticFreqRes;
  FRAME_INFO * pFrameInfo = &h_frame_data->frameInfo;
  int numberTimeSlots = hHeaderData->numberTimeSlots;
  
    
  
  frameClass = getbits (hBitBuf, SBR_CLA_BITS);
   
  pFrameInfo->frameClass = frameClass;
  
  switch (frameClass) {
  case FIXFIX:
    
    temp = getbits (hBitBuf, SBR_ENV_BITS);
    
    staticFreqRes = getbits (hBitBuf, SBR_RES_BITS);
    
    nEnv = (int) (1 << temp);
     
    if (nEnv > MAX_ENVELOPES)
    {
      
      return 0;
    }
    
    b = nEnv + 1;
    
    switch (nEnv) {
    case 1:
      
      switch (numberTimeSlots) {
        case 16:
              STORE(sizeof(FRAME_INFO));
          memcpy (pFrameInfo, &sbr_frame_info1_16, sizeof(FRAME_INFO));
          break;
      }
      break;
    case 2:
      switch (numberTimeSlots) {
        case 16:
              STORE(sizeof(FRAME_INFO));
          memcpy (pFrameInfo, &sbr_frame_info2_16, sizeof(FRAME_INFO));
          break;
      }
      break;
    case 4:
      switch (numberTimeSlots) {
        case 16:
              STORE(sizeof(FRAME_INFO));
          memcpy (pFrameInfo, &sbr_frame_info4_16, sizeof(FRAME_INFO));
          break;
      }
      break;
    case 8:
#if (MAX_ENVELOPES >= 8)
      
      switch (numberTimeSlots) {
        case 16:
              STORE(sizeof(FRAME_INFO));
          memcpy (pFrameInfo, &sbr_frame_info8_16, sizeof(FRAME_INFO));
          break;
      }
      break;
#else
      
      return 0;
#endif
    }
    
    if (!staticFreqRes) {
       /* pFrameInfo->freqRes[] */
      
      for (i = 0; i < nEnv ; i++)
      {
        
        pFrameInfo->freqRes[i] = 0;
      }
    }
    break;
  case FIXVAR:
  case VARFIX:
    
    temp = getbits (hBitBuf, SBR_ABS_BITS);
    
    n    = getbits (hBitBuf, SBR_NUM_BITS);
    
    nEnv = n + 1;
    b = nEnv + 1;
    break;
  }
  
  switch (frameClass) {
  case FIXVAR:
     
    pFrameInfo->borders[0] = 0;
    
    border = temp + numberTimeSlots;
    i = b-1;
     /* pFrameInfo->borders[] */
    
    pFrameInfo->borders[i] = border;
    
    for (k = 0; k < n; k++) {
      
      temp = getbits (hBitBuf, SBR_REL_BITS);
       
      border -= (2 * temp + 2);
      
      pFrameInfo->borders[--i] = border;
    }
     
    pointer_bits = (int)(FloatFR_logDualis(n+2) + 0.992188);
    
    p = getbits (hBitBuf, pointer_bits);
     
    if (p > n+1)
    {
      
      return 0;
    }
    
    if (p) {
        
      pFrameInfo->tranEnv = n + 2 - p;
    } else {
       
      pFrameInfo->tranEnv = -1;
    }
     /* pFrameInfo->freqRes[] */
    
    for (k = n; k >= 0; k--) {
       
      pFrameInfo->freqRes[k] = (unsigned char)getbits (hBitBuf, SBR_RES_BITS); /* f = F [SBR_RES_BITS bits] */
    }
      
    if (p == 0 || p == 1)
    {
       
      pFrameInfo->bordersNoise[1] = pFrameInfo->borders[n];
    }
    else
    {
       
      pFrameInfo->bordersNoise[1] = pFrameInfo->borders[pFrameInfo->tranEnv];
    }
    break;
  case VARFIX:
     
    border = temp;
    pFrameInfo->borders[0] = border;
     /* pFrameInfo->borders[] */
    
    for (k = 1; k <= n; k++) {
      
      temp = getbits (hBitBuf, SBR_REL_BITS);
       
      border += (2 * temp + 2);
      
      pFrameInfo->borders[k] = border;
    }
    
    pFrameInfo->borders[k] = numberTimeSlots;
     
    pointer_bits = (int)(FloatFR_logDualis(n+2) + 0.992188);
    
    p = getbits (hBitBuf, pointer_bits);
     
    if (p > n+1)
    {
      
      return 0;
    }
      
    if (p == 0 || p == 1)
    {
       
      pFrameInfo->tranEnv = -1;
    }
    else
    {
        
      pFrameInfo->tranEnv = p - 1;
    }
     /* pFrameInfo->freqRes[k] */
    
    for (k = 0; k <= n; k++) {
       
      pFrameInfo->freqRes[k] = (unsigned char)getbits(hBitBuf, SBR_RES_BITS);
    }
    
    switch (p) {
    case 0:
       
      pFrameInfo->bordersNoise[1] = pFrameInfo->borders[1];
      break;
    case 1:
       
      pFrameInfo->bordersNoise[1] = pFrameInfo->borders[n];
      break;
    default:
       
      pFrameInfo->bordersNoise[1] = pFrameInfo->borders[pFrameInfo->tranEnv];
      break;
    }
    break;
  case VARVAR:
    
    aL = getbits (hBitBuf, SBR_ABS_BITS);
    
    aR = getbits (hBitBuf, SBR_ABS_BITS) + numberTimeSlots;
    
    nL = getbits (hBitBuf, SBR_NUM_BITS);
    
    nR = getbits (hBitBuf, SBR_NUM_BITS);
    
    nEnv = nL + nR + 1;
     
    if (nEnv > MAX_ENVELOPES)
    {
      
      return 0;
    }
    
    b = nEnv + 1;
     
    border            = aL;
    pFrameInfo->borders[0] = border;
     /* pFrameInfo->borders[] */
    
    for (k = 1; k <= nL; k++) {
      
      temp = getbits (hBitBuf, SBR_REL_BITS);
       
      border += (2 * temp + 2);
      
      pFrameInfo->borders[k] = border;
    }
    
    border = aR;
    i      = nEnv;
     /* pFrameInfo->borders[] */
    
    pFrameInfo->borders[i] = border;
    
    for (k = 0; k < nR; k++) {
      
      temp = getbits (hBitBuf, SBR_REL_BITS);
       
      border -= (2 * temp + 2);
      
      pFrameInfo->borders[--i] = border;
    }
     
    pointer_bits = (int)(FloatFR_logDualis(nL+nR+2) + 0.992188);
    
    p = getbits (hBitBuf, pointer_bits);
     
    if (p > nL+nR+1)
    {
      
      return 0;
    }
    
    if (p) {
        
      pFrameInfo->tranEnv = b - p;
    } else {
       
      pFrameInfo->tranEnv = -1;
    }
     /* pFrameInfo->freqRes[] */
    
    for (k = 0; k < nEnv; k++) {
      
      pFrameInfo->freqRes[k] = (unsigned char)getbits(hBitBuf, SBR_RES_BITS);
    }
     
    pFrameInfo->bordersNoise[0] = aL;
     
    if (nEnv == 1) {
       
      pFrameInfo->bordersNoise[1] = aR;
    }
    else {
           
      if (p == 0 || p == 1)
        pFrameInfo->bordersNoise[1] = pFrameInfo->borders[nEnv - 1];
      else
        pFrameInfo->bordersNoise[1] = pFrameInfo->borders[pFrameInfo->tranEnv];
       
      pFrameInfo->bordersNoise[2] = aR;
    }
    break;
  }
   
  pFrameInfo->nEnvelopes = nEnv;
     
  if (nEnv == 1)
    pFrameInfo->nNoiseEnvelopes = 1;
  else
    pFrameInfo->nNoiseEnvelopes = 2;
    
  if (frameClass == VARFIX || frameClass == FIXVAR) {
     
    pFrameInfo->bordersNoise[0] = pFrameInfo->borders[0];
     
    pFrameInfo->bordersNoise[pFrameInfo->nNoiseEnvelopes] = pFrameInfo->borders[nEnv];
  }
  
  return 1;
}
/*
  \brief   Check if the frameInfo vector has reasonable values.
  \return  Zero for error, one for correct
*/
int
checkFrameInfo (FRAME_INFO * pFrameInfo,
                int numberOfTimeSlots)
{
  int maxPos,i,j;
  int startPos;
  int stopPos;
  int tranEnv;
  int startPosNoise;
  int stopPosNoise;
  int nEnvelopes = pFrameInfo->nEnvelopes;
  int nNoiseEnvelopes = pFrameInfo->nNoiseEnvelopes;
  
    
    
  if(nEnvelopes < 1 || nEnvelopes > MAX_ENVELOPES)
  {
    
    return 0;
  }
   
  if(nNoiseEnvelopes > MAX_NOISE_ENVELOPES)
  {
    
    return 0;
  }
   
  startPos        = pFrameInfo->borders[0];
  stopPos         = pFrameInfo->borders[nEnvelopes];
  tranEnv         = pFrameInfo->tranEnv;
  startPosNoise   = pFrameInfo->bordersNoise[0];
  stopPosNoise    = pFrameInfo->bordersNoise[nNoiseEnvelopes];
  
  switch(numberOfTimeSlots) {
  case 16:
    
    maxPos = 19;
    break;
  default:
    
    return 0;
  }
    
  if( (startPos < 0) || (startPos >= stopPos) )
  {
    
    return 0;
  }
   
  if( startPos > maxPos-numberOfTimeSlots )
  {
    
    return 0;
  }
   
  if( stopPos < numberOfTimeSlots )
  {
    
    return 0;
  }
   
  if(stopPos > maxPos)
  {
    
    return 0;
  }
   /* pFrameInfo->borders[] */
  
  for(i=0;i<nEnvelopes;i++) {
     
    if(pFrameInfo->borders[i] >= pFrameInfo->borders[i+1])
    {
      
      return 0;
    }
  }
   
  if(tranEnv>nEnvelopes)
  {
    
    return 0;
  }
    
  if(nEnvelopes==1 && nNoiseEnvelopes>1)
  {
    
    return 0;
  }
    
  if(startPos != startPosNoise || stopPos != stopPosNoise)
  {
    
    return 0;
  }
   /* pFrameInfo->bordersNoise[] */
  
  for(i=0; i<nNoiseEnvelopes; i++) {
     
    if(pFrameInfo->bordersNoise[i] >= pFrameInfo->bordersNoise[i+1])
    {
      
      return 0;
    }
  }
   /* pFrameInfo->bordersNoise[] */
  
  for(i=0; i<nNoiseEnvelopes; i++) {
    
    startPosNoise = pFrameInfo->bordersNoise[i];
     /* pFrameInfo->borders[] */
    
    for(j=0; j<nEnvelopes; j++) {
       
      if(pFrameInfo->borders[j] == startPosNoise)
        break;
    }
     
    if(j==nEnvelopes)
    {
      
      return 0;
    }
  }
  
  return 1;
}
