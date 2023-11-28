/*
  individual channel stream info
*/
#include "aac_rom.h"
#include "streaminfo.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
unsigned char IsValid(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->Valid;
}
unsigned char IsLongBlock(CIcsInfo *pIcsInfo)
{
      
  
  return (pIcsInfo->WindowSequence != EightShortSequence);
}
unsigned char IsShortBlock(CIcsInfo *pIcsInfo)
{
      
  
  return (pIcsInfo->WindowSequence == EightShortSequence);
}
unsigned char IsMainProfile(CIcsInfo *pIcsInfo)
{
      
  
  return (pIcsInfo->Profile == ProfileMain);
}
int GetProfile(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->Profile;
}
char GetWindowShape(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->WindowShape;
}
char GetWindowSequence(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->WindowSequence;
}
int GetWindowsPerFrame(CIcsInfo *pIcsInfo)
{
      
  
  return (pIcsInfo->WindowSequence == EightShortSequence) ? 8 : 1;
}
char GetWindowGroups(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->WindowGroups;
}
char GetWindowGroupLength(CIcsInfo *pIcsInfo, int index)
{
    
  
  return pIcsInfo->WindowGroupLength[index];
}
char *GetWindowGroupLengthTable(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->WindowGroupLength;
}
char GetScaleFactorBandsTransmitted(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->MaxSfBands;
}
char GetScaleFactorBandsTotal(CIcsInfo *pIcsInfo)
{
    
  
  return pIcsInfo->TotalSfBands;
}
int SamplingRateFromIndex(int index)
{
    
  
  return SamplingRateInfoTable[index].SamplingFrequency;
}
int GetSamplingFrequency(CIcsInfo *pIcsInfo)
{
     
  
  return SamplingRateFromIndex(pIcsInfo->SamplingRateIndex);
}
char GetMaximumTnsBands(CIcsInfo *pIcsInfo)
{
  int idx = IsLongBlock(pIcsInfo) ? 0 : 1;
      
   /* counting post operation */
  
  return tns_max_bands_tbl[pIcsInfo->SamplingRateIndex][idx];
}
const short *GetScaleFactorBandOffsets(CIcsInfo *pIcsInfo)
{
  
   
  if (IsLongBlock(pIcsInfo))
  {
    
    
    return SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Long;
  }
  else
  {
    
    
    return SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Short;
  }
}
void IcsReset(CIcsInfo *pIcsInfo, CStreamInfo *pStreamInfo)
{
  
   
  pIcsInfo->Valid = 0;
  pIcsInfo->TotalSfBands = 0;
   
  pIcsInfo->SamplingRateIndex = pStreamInfo->SamplingRateIndex;
  pIcsInfo->Profile = pStreamInfo->Profile;
  
}
int IcsRead(HANDLE_BIT_BUF bs,
             CIcsInfo *pIcsInfo)
{
  int i;
  char mask;
  char PredictorDataPresent;
  int ErrorStatus = AAC_DEC_OK;
  
  
   
    
  pIcsInfo->IcsReservedBit = (char) GetBits(bs,1);
    
  pIcsInfo->WindowSequence = (char) GetBits(bs,2);
    
  pIcsInfo->WindowShape = (char) GetBits(bs,1);
   
  if (IsLongBlock(pIcsInfo))
  {
     
    pIcsInfo->TotalSfBands = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Long;
      
    pIcsInfo->MaxSfBands = (char) GetBits(bs,6);
    
     
    if ((PredictorDataPresent = (char) GetBits(bs,1)))
    {
      
      return (AAC_DEC_PREDICTION_NOT_SUPPORTED_IN_LC_AAC);
    }
     
    pIcsInfo->WindowGroups = 1;
    pIcsInfo->WindowGroupLength[0] = 1;
  }
  else
  {
     
    pIcsInfo->TotalSfBands = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Short;
      
    pIcsInfo->MaxSfBands = (char) GetBits(bs,4);
      
    pIcsInfo->ScaleFactorGrouping = (char) GetBits(bs,7);
     
    pIcsInfo->WindowGroups = 0 ;
     /* pIcsInfo->WindowGroupLength[] */
    
    for (i=0; i < 7; i++)
    {
       
      mask = 1 << (6 - i);
      
      pIcsInfo->WindowGroupLength[i] = 1;
        
      if (pIcsInfo->ScaleFactorGrouping & mask)
      {
          
        pIcsInfo->WindowGroupLength[pIcsInfo->WindowGroups]++;
      }
      else
      {
          
        pIcsInfo->WindowGroups++;
      }
    }
    /* loop runs to i < 7 only */
     
    pIcsInfo->WindowGroupLength[7] = 1;
      
    pIcsInfo->WindowGroups++;
  }
   
  pIcsInfo->Valid = 1;
  
  return ErrorStatus;
}
