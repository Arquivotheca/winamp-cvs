/*
  Parametric Stereo bitstream processing and decoding
*/
#include "huff_dec.h"
#include "ps_bitdec.h"
#include "ps_dec.h"
#include "sbr_rom.h"
#include "math/counters.h"
static const int aNoIidBins[3] = {NO_LOW_RES_IID_BINS, NO_IID_BINS, NO_HI_RES_BINS};
static const int aNoIccBins[3] = {NO_LOW_RES_ICC_BINS, NO_ICC_BINS, NO_HI_RES_BINS};
static const int aFixNoEnvDecode[4] = {0, 1, 2, 4};
static int limitMinMax(int i, int min, int max)
{
  int result = i;

  if (i<min)
	{
    result = min;                                                               
  }
  else
	{                                                                        
    if (i>max) 
		{
      result = max;                                                             
    }
  }
  
  return result;
}

static void
deltaDecodeArray(int enable,
                 int *aIndex,
                 int *aPrevFrameIndex,
                 int DtDf,
                 int nrElements,
                 int stride,
                 int minIdx,
                 int maxIdx)
{
  int i;
                                                                                
  if ( enable==1 ) {                                                            
    if (DtDf == 0)  {
      aIndex[0] = 0 + aIndex[0];                                                
      aIndex[0] = limitMinMax(aIndex[0],minIdx,maxIdx);                          
                                                                                
      for (i = 1; i < nrElements; i++) {
        aIndex[i] = aIndex[i-1] + aIndex[i];                                     
        aIndex[i] = limitMinMax(aIndex[i],minIdx,maxIdx);                        
      }
    }
    else {                                                                       
      for (i = 0; i < nrElements; i++) {
        aIndex[i] = aPrevFrameIndex[i*stride] + aIndex[i];                       
        aIndex[i] = limitMinMax(aIndex[i],minIdx,maxIdx);                        
      }
    }
  }
  else {                                                                         
    for (i = 0; i < nrElements; i++) {
      aIndex[i] = 0;                                                            
    }
  }
                                                                                 
  if (stride==2) {                                                                 
    for (i=nrElements*stride-1; i>0; i--) {
      aIndex[i] = aIndex[i/stride];                                             
    }
  }
  
}
static void map34IndexTo20 (int *aIndex)
{
   /* aIndex[] */
  aIndex[0] =   (2*aIndex[0]+aIndex[1])/3;                                       
  aIndex[1] =	  (aIndex[1]+2*aIndex[2])/3;                                       
  aIndex[2] =	  (2*aIndex[3]+aIndex[4])/3;                                       
  aIndex[3] =	  (aIndex[4]+2*aIndex[5])/3;                                       
  aIndex[4] =	  (aIndex[6]+aIndex[7])/2;                                         
  aIndex[5] =	  (aIndex[8]+aIndex[9])/2;                                         
  aIndex[6] =	  aIndex[10];                                                     
  aIndex[7] =	  aIndex[11];                                                     
  aIndex[8] =	  (aIndex[12]+aIndex[13])/2;                                       
  aIndex[9] =	  (aIndex[14]+aIndex[15])/2;                                       
  aIndex[10] =	aIndex[16];                                                     
  aIndex[11] =  aIndex[17];                                                     
  aIndex[12] =	aIndex[18];                                                     
  aIndex[13] =	aIndex[19];                                                     
  aIndex[14] =	(aIndex[20]+aIndex[21])/2;                                       
  aIndex[15] =	(aIndex[22]+aIndex[23])/2;                                       
  aIndex[16] =	(aIndex[24]+aIndex[25])/2;                                       
  aIndex[17] =	(aIndex[26]+aIndex[27])/2;                                       
  aIndex[18] =	(aIndex[28]+aIndex[29]+aIndex[30]+aIndex[31])/4;                 
  aIndex[19] =	(aIndex[32]+aIndex[33])/2;                                       
}
/***************************************************************************/
/*
  \brief  Decodes parametric stereo
  \return none
****************************************************************************/
void DecodePs(struct PS_DEC *h_ps_dec)
{
  int gr, env;
  int noIidSteps;
                                                                                
  if (!h_ps_dec->bPsDataAvail) {
    h_ps_dec->noEnv = 0;                                                        
  }
  noIidSteps = h_ps_dec->bFineIidQ?NO_IID_STEPS_FINE:NO_IID_STEPS;               
                                                                                 
  for (env=0; env<h_ps_dec->noEnv; env++) {
    int *aPrevIidIndex;
    int *aPrevIccIndex;
                                                                               
    if (env==0) {
      aPrevIidIndex = h_ps_dec->aIidPrevFrameIndex;                            
      aPrevIccIndex = h_ps_dec->aIccPrevFrameIndex;                            
    }
    else {
      aPrevIidIndex = h_ps_dec->aaIidIndex[env-1];                             
      aPrevIccIndex = h_ps_dec->aaIccIndex[env-1];                             
    }
    deltaDecodeArray(h_ps_dec->bEnableIid,
                     h_ps_dec->aaIidIndex[env],
                     aPrevIidIndex,
                     h_ps_dec->abIidDtFlag[env],
                     aNoIidBins[h_ps_dec->freqResIid],
                     (h_ps_dec->freqResIid)?1:2,
                     -noIidSteps,
                     noIidSteps); 
                                                                                  
    deltaDecodeArray(h_ps_dec->bEnableIcc,
                     h_ps_dec->aaIccIndex[env],
                     aPrevIccIndex,
                     h_ps_dec->abIccDtFlag[env],
                     aNoIccBins[h_ps_dec->freqResIcc],
                     (h_ps_dec->freqResIcc)?1:2,
                     0,
                     NO_ICC_STEPS-1);
                                                                                  
  }   /* for (env=0; env<h_ps_dec->noEnv; env++) */
                                                                                 
  if (h_ps_dec->noEnv==0) {
    h_ps_dec->noEnv = 1;                                                        
                                                                                
    if (h_ps_dec->bEnableIid) {                                                 
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIidIndex[h_ps_dec->noEnv-1][gr] =
          h_ps_dec->aIidPrevFrameIndex[gr];                                    
      }
    }
    else {                                                                     
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIidIndex[h_ps_dec->noEnv-1][gr] = 0;                      
      }
    }
                                                                                
    if (h_ps_dec->bEnableIcc) {                                                 
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIccIndex[h_ps_dec->noEnv-1][gr] =
          h_ps_dec->aIccPrevFrameIndex[gr];                                    
      }
    }
    else {                                                                     
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIccIndex[h_ps_dec->noEnv-1][gr] = 0;                      
      }
    }
  }
                                                                               
  for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
    h_ps_dec->aIidPrevFrameIndex[gr] =
      h_ps_dec->aaIidIndex[h_ps_dec->noEnv-1][gr];                            
  }
                                                                                 
  for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
    h_ps_dec->aIccPrevFrameIndex[gr] =
      h_ps_dec->aaIccIndex[h_ps_dec->noEnv-1][gr];                            
  }
  h_ps_dec->bPsDataAvail = 0;                                                   
                                                                                
  if (h_ps_dec->bFrameClass == 0) {
    int shift;
                                                                                 
    switch (h_ps_dec->noEnv){
    case 1:
      shift = 0;                                                                
      break;
    case 2:
      shift = 1;                                                                
      break;
    case 4:
      shift = 2;                                                                
      break;
    }
    h_ps_dec->aEnvStartStop[0] = 0;                                             
                                                                                  
    for (env=1; env<h_ps_dec->noEnv; env++) {
      h_ps_dec->aEnvStartStop[env] =
        (env * h_ps_dec->noSubSamples) >> shift;                                 
    }
    h_ps_dec->aEnvStartStop[h_ps_dec->noEnv] = h_ps_dec->noSubSamples;        
  }
  else {   /* if (h_ps_dec->bFrameClass == 0) */
    h_ps_dec->aEnvStartStop[0] = 0;                                             
                                                                                 
    if (h_ps_dec->aEnvStartStop[h_ps_dec->noEnv] <
        (int)h_ps_dec->noSubSamples) {
      h_ps_dec->noEnv++;                                                         
      h_ps_dec->aEnvStartStop[h_ps_dec->noEnv] = h_ps_dec->noSubSamples;      
                                                                                 
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIidIndex[h_ps_dec->noEnv-1][gr] =
          h_ps_dec->aaIidIndex[h_ps_dec->noEnv-2][gr];                        
      }
                                                                                 
      for (gr = 0; gr < NO_HI_RES_BINS; gr++) {
        h_ps_dec->aaIccIndex[h_ps_dec->noEnv-1][gr] =
          h_ps_dec->aaIccIndex[h_ps_dec->noEnv-2][gr];                        
      }
    }
                                                                                 
    for (env=1; env<h_ps_dec->noEnv; env++) {
      int thr;
      thr = h_ps_dec->noSubSamples - h_ps_dec->noEnv + env;                   
                                                                                 
      if (h_ps_dec->aEnvStartStop[env] > thr) {
        h_ps_dec->aEnvStartStop[env] = thr;                                    
      }
      else {
        thr = h_ps_dec->aEnvStartStop[env-1]+1;                                
                                                                                 
        if (h_ps_dec->aEnvStartStop[env] < thr) {
          h_ps_dec->aEnvStartStop[env] = thr;                                  
        }
      }
    }
  }   /* if (h_ps_dec->bFrameClass == 0) ... else */
                                                                                
  for (env=0; env<h_ps_dec->noEnv; env++) {
                                                                                
    if (h_ps_dec->freqResIid == 2)
    {
      map34IndexTo20 (h_ps_dec->aaIidIndex[env]);                              
    }
                                                                                
    if (h_ps_dec->freqResIcc == 2)
    {
      map34IndexTo20 (h_ps_dec->aaIccIndex[env]);                              
    }
  }
  
}
/***************************************************************************/
/*
  \brief  Reads IID and ICC data from bitstream
****************************************************************************/
unsigned int ReadPsData(HANDLE_PS_DEC h_ps_dec, HANDLE_BIT_BUFFER hBitBuf, int nBitsLeft)
{
  int     gr, env;
  int     dtFlag;
  int     startbits;
  Huffman CurrentTable;
  int     bEnableHeader;
                                                                                
  if (!h_ps_dec) 
	{    
    return 0;
  }
  startbits = GetNrBitsAvailable(hBitBuf);                                     
  bEnableHeader = (int) getbits (hBitBuf, 1);                                  
                                                                               
  if (bEnableHeader) {
    h_ps_dec->bEnableIid = (int) getbits (hBitBuf, 1);                           
                                                                               
    if (h_ps_dec->bEnableIid) {
      h_ps_dec->freqResIid = (int) getbits (hBitBuf, 3);                         
                                                                                
      if (h_ps_dec->freqResIid > 2){
        h_ps_dec->bFineIidQ = 1;                                                
        h_ps_dec->freqResIid -=3;                                               
      }
      else{
        h_ps_dec->bFineIidQ = 0;                                                
      }
    }
    h_ps_dec->bEnableIcc = (int) getbits (hBitBuf, 1);                           
                                                                                
    if (h_ps_dec->bEnableIcc) {
      h_ps_dec->freqResIcc = (int) getbits (hBitBuf, 3);                         
                                                                                
      if (h_ps_dec->freqResIcc > 2){
        h_ps_dec->freqResIcc -=3;                                               
      }
    }
    h_ps_dec->bEnableExt = (int) getbits (hBitBuf, 1);                           
  }
  h_ps_dec->bFrameClass = (int) getbits (hBitBuf, 1);                            
                                                                                
  if (h_ps_dec->bFrameClass == 0) {
    h_ps_dec->noEnv = aFixNoEnvDecode[(int) getbits (hBitBuf, 2)];               
  }
  else {
    h_ps_dec->noEnv = 1+(int) getbits (hBitBuf, 2);                               
                                                                                   
    for (env=1; env<h_ps_dec->noEnv+1; env++) {
      h_ps_dec->aEnvStartStop[env] = ((int) getbits (hBitBuf, 5)) + 1;             
    }
  }
                                                                                 
  if ((h_ps_dec->freqResIid > 2) || (h_ps_dec->freqResIcc > 2)) {
    h_ps_dec->bPsDataAvail = 0;                                                
    nBitsLeft -= startbits - GetNrBitsAvailable(hBitBuf);                         
                                                                                
    while (nBitsLeft) {
      int i = nBitsLeft;                                                        
                                                                                 
      if (i>8) {
        i = 8;                                                                  
      }
      getbits (hBitBuf, i);                                                     
      nBitsLeft -= i;                                                            
    }
    
    return (startbits - GetNrBitsAvailable(hBitBuf));
  }
                                                                                 
  if (h_ps_dec->bEnableIid) {
                                                                                  
    for (env=0; env<h_ps_dec->noEnv; env++) {
      dtFlag = (int)getbits (hBitBuf, 1);                                       
                                                                                
      if (!dtFlag) {
                                                                                 
        if (h_ps_dec->bFineIidQ){
          CurrentTable = (Huffman)&aBookPsIidFineFreqDecode;                    
        }
        else {
          CurrentTable = (Huffman)&aBookPsIidFreqDecode;                        
        }
      }
      else {
                                                                                 
        if (h_ps_dec->bFineIidQ){
          CurrentTable = (Huffman)&aBookPsIidFineTimeDecode;                    
        }
        else {
          CurrentTable = (Huffman)&aBookPsIidTimeDecode;                        
        }
      }
                                                                                  
      for (gr = 0; gr < aNoIidBins[h_ps_dec->freqResIid]; gr++) {
                                                                                
        h_ps_dec->aaIidIndex[env][gr] = DecodeHuffmanCW(CurrentTable,hBitBuf);  
      }
      h_ps_dec->abIidDtFlag[env] = dtFlag;                                     
    }
  }
                                                                                 
  if (h_ps_dec->bEnableIcc) {
                                                                                  
    for (env=0; env<h_ps_dec->noEnv; env++) {
      dtFlag = (int)getbits (hBitBuf, 1);                                       
                                                                                
      if (!dtFlag) {
        CurrentTable = (Huffman)&aBookPsIccFreqDecode;                          
      }
      else {
        CurrentTable = (Huffman)&aBookPsIccTimeDecode;                          
      }
                                                                                  
      for (gr = 0; gr < aNoIccBins[h_ps_dec->freqResIcc]; gr++) {
        h_ps_dec->aaIccIndex[env][gr] = DecodeHuffmanCW(CurrentTable,hBitBuf);  
      }
      h_ps_dec->abIccDtFlag[env] = dtFlag;                                     
    }
  }
                                                                                 
  if (h_ps_dec->bEnableExt) {
    int cnt, i;
    cnt = (int)getbits (hBitBuf, 4);                                            
                                                                                 
    if (cnt==15)
    {
      cnt += (int)getbits (hBitBuf, 8);                                          
    }
    
    for (i=0; i<cnt; i++)
    {
      getbits(hBitBuf, 8);                                                      
    }
  }
  h_ps_dec->bPsDataAvail = 1;                                                   
                                                                                 
  
  return (startbits - GetNrBitsAvailable(hBitBuf));
}
