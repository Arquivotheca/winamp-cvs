/*
  decoding of long blocks
*/
#include <stdlib.h>
#include <math.h>
#include "aac_rom.h"
#include "imdct.h"
#include "bitbuffer.h"
#include "block.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
void CPns_Read (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                HANDLE_BIT_BUF bs,
                const CodeBookDescription *hcb,
                unsigned char global_gain,
                int band,
                int group);
int CLongBlock_ReadSectionData(HANDLE_BIT_BUF bs,
                               CAacDecoderChannelInfo *pAacDecoderChannelInfo)
{
  int top;
  int band;
  char sect_cb;
  int sect_len;
  int sect_len_incr;
  int sect_esc_val = (1 << 5) - 1;
  char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  int ErrorStatus = AAC_DEC_OK;
  
  
     
   /* pCodeBook[] */
     
  for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); )
  {
    
    sect_len = 0;
    
    sect_cb = (char) GetBits(bs,4);
    
    sect_len_incr = GetBits(bs,5);
    
    while (sect_len_incr == sect_esc_val)
    {
      
      sect_len += sect_esc_val;
      
      sect_len_incr = GetBits(bs,5);
    }
    
    sect_len += sect_len_incr;
    
    top = band + sect_len;
     
    if (top > MAX_SFB_LONG) {
      
      return (AAC_DEC_DECODE_FRAME_ERROR);
    }
    
    for (; band < top; band++)
    {
      
      pCodeBook[band] = sect_cb;
       
      if (pCodeBook[band] == BOOKSCL)
      {
        
        return (AAC_DEC_INVALID_CODE_BOOK);
      }
    }
  }
   /* pCodeBook[] */
     
  for (; band < GetScaleFactorBandsTotal(&pAacDecoderChannelInfo->IcsInfo); band++)
  {
    
    pCodeBook[band] = ZERO_HCB;
  }
  
  return (ErrorStatus);
}
void CLongBlock_ReadScaleFactorData(HANDLE_BIT_BUF bs,
                                    CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                                    unsigned char global_gain)
{
  int temp;
  int band;
  int position = 0;
  int factor = global_gain;
  const char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  short *pScaleFactor = pAacDecoderChannelInfo->aScaleFactor;
  const CodeBookDescription *hcb = &HuffmanCodeBooks[BOOKSCL];
  
     
   /* pCodeBook[] 
                  pScaleFactor[]
               */
     
  for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
  {
    
    switch (pCodeBook[band])
    {
      case ZERO_HCB: /* zero book */
        
        pScaleFactor[band] = 0;
        break;
      default: /* regular scale factor */
         
        temp = CBlock_DecodeHuffmanWord(bs,hcb->CodeBook);
        
        factor += temp - 60; /* MIDFAC 1.5 dB */
         
        pScaleFactor[band] = factor - 100;
        break;
      case INTENSITY_HCB: /* intensity steering */
      case INTENSITY_HCB2:
         
        temp = CBlock_DecodeHuffmanWord(bs,hcb->CodeBook);
        
        position += temp - 60;
         
        pScaleFactor[band] = position - 100;
        break;
      case NOISE_HCB:
        
        CPns_Read(pAacDecoderChannelInfo, bs, hcb, global_gain, band, 0);
        break;
    }
  }
  
}
int  CLongBlock_ReadSpectralData(HANDLE_BIT_BUF bs,
                                 CAacDecoderChannelInfo *pAacDecoderChannelInfo)
{
  int i,index,band,step;
  int scfExp,scfMod;
  int *QuantizedCoef;
  const char *pCodeBook;
  const short *pScaleFactor;
  float *pSpectralCoefficient;
  const short *BandOffsets = GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->IcsInfo);
  const CodeBookDescription *hcb;
  
    
  pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  pScaleFactor = pAacDecoderChannelInfo->aScaleFactor;
  pSpectralCoefficient = pAacDecoderChannelInfo->aSpectralCoefficient;
  QuantizedCoef = (int*)pSpectralCoefficient;
  /* including initialization of BandOffsets */
  
  for (index=0; index < MaximumBinsLong; index++)
  {
    
    QuantizedCoef[index] = 0;
  }
   /* pointer for pCodeBook[] */
   
  for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
  {
      
    if ((pCodeBook[band] == ZERO_HCB) || (pCodeBook[band] == INTENSITY_HCB) || (pCodeBook[band] == INTENSITY_HCB2) || (pCodeBook[band] == NOISE_HCB))
      continue;
    
    hcb = &HuffmanCodeBooks[pCodeBook[band]] ;
    
    step = 0;
     /* pointer for BandOffsets[],
                                QuantizedCoef[] */
    
    for (index=BandOffsets[band]; index < BandOffsets[band+1]; index += step)
    {
        
      step = CBlock_UnpackIndex(CBlock_DecodeHuffmanWord(bs,hcb->CodeBook),&QuantizedCoef[index],hcb);
       
      if (hcb->Offset == 0)
      {
         /* pointer for QuantizedCoef[] */
        
        for (i=0; i < step; i++)
        {
          
          if (QuantizedCoef[index+i])
          {
            
            if (GetBits(bs,1)) /* sign bit */
            {
               
              QuantizedCoef[index+i] = -QuantizedCoef[index+i];
            }
          }
        }
      }
       
      if (pCodeBook[band] == ESCBOOK)
      {
         
        QuantizedCoef[index] = CBlock_GetEscape(bs,QuantizedCoef[index]);
         
        QuantizedCoef[index+1] = CBlock_GetEscape(bs,QuantizedCoef[index+1]);
            
        if (abs(QuantizedCoef[index]) > MAX_QUANTIZED_VALUE || abs(QuantizedCoef[index+1]) > MAX_QUANTIZED_VALUE) {
          
          return (AAC_DEC_DECODE_FRAME_ERROR);
        }
      }
    }
  }
  
  /* apply pulse data */
   
  CPulseData_Apply(&pAacDecoderChannelInfo->PulseData,
                   GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->IcsInfo),
                   QuantizedCoef);
   /* pointer for BandOffsets[],
                              pScaleFactor     */
   
  for (index=0, band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
  {
    /* scalefactor exponents and scalefactor mantissa for current band */
    
    scfExp = pScaleFactor[band] >> 2;
    
    scfMod = pScaleFactor[band] & 3;
     /* pointer for QuantizedCoef[], 
                                pSpectralCoefficient */
    
    for (index=BandOffsets[band]; index < BandOffsets[band+1]; index++)
    {
        
      pSpectralCoefficient[index] = CBlock_Quantize(QuantizedCoef[index],scfMod,scfExp-9);
    }
  }
  /* zero out spectral data beyond max_sfb; index is now first bin of max_sfb+1 */
   /* pointer for pSpectralCoefficient */
  
  for ( ; index < MaximumBinsLong; index++)
  {
    
    pSpectralCoefficient[index] = 0;
  }
  
  return (AAC_DEC_OK);
}
static void 
NoLap1(float *coef,
       float *out,
       int stride)
{
  int i;
  
   /* out[stride*i]
                  coef[Size07-1-i]
               */
  
  for (i=0; i<Size07; i++)
  {
     
    out[stride*i] = -coef[Size07-1-i];
  }
  
}
static void 
NoLap2(float *coef,
       float *out,
       int stride)
{
  int i;
  
   /* out[stride*i]
                  coef[Size07-1-i]
               */
  
  for(i=0; i<Size07; i++)
  {
     
    out[stride*i] = -coef[Size07-1-i];
  }
  
}
static void 
StartLap(float *coef,
         float *out,
         int stride)
{
  int i;
  
   /* out[stride*i]
                  coef[i]
               */
  
  for (i=0; i<Size01; i++)
  {
    
    out[stride*i] = coef[i];
  }
  
}
static void 
StopLap(float *coef,
        float *prev,
        float *out,
        const float *window,
        int stride)
{
  int i;
  
   /* out[stride*(Size07+i)]
                  coef[Size15+i]
                  window[i]
                  window[Size01*2-1-i]
                  prev[Size08-1-i])
               */
  
  for (i=0; i<Size01; i++)
  {
      
    out[stride*(Size07+i)] = (coef[Size15+i]*window[i]) - (window[Size01*2-1-i]*prev[Size08-1-i]);
  }
   /* out[stride*(Size08+i)]
                  coef[Size08*2-1-i]
                  window[Size01+i]
                  window[Size01-1-i]
                  prev[Size07+i]
               */
  
  for (i=0; i<Size01; i++)
  {
      
    out[stride*(Size08+i)] = (-coef[Size08*2-1-i]*window[Size01+i]) - (window[Size01-1-i]*prev[Size07+i]);
  }
  
}
static void 
ShortLongLapIllegal(float *coef,
                    float *prev,
                    float *out,
                    const float *window_long,
                    const float *window_short,
                    int stride)
{
  int i;
  
  /* 0,...,Size07-1 */
   /* out[stride*i]
                  coef[Size08+i]
                  window_long[i]
                  prev[i]
               */
  
  for (i=0; i<Size07; i++) {
      
    out[stride*i] = (coef[Size08+i]*window_long[i]) + prev[i];
  }
  /* Size07,...,Size08-1 */
   /* out[stride*(i+Size07)]
                  coef[Size15+i]
                  window_long[Size07+i]
                  window_short[Size02-1-i]
                  prev[Size08-1-i])
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(i+Size07)] = (coef[Size15+i]*window_long[Size07+i]) - (window_short[Size02-1-i]*prev[Size08-1-i]);
  }
  /* Size08,...,Size09-1 */
   /* out[stride*(i+Size08)]
                  coef[Size16-1-i]
                  window_long[Size08+i]
                  window_short[Size01-1-i]
                  prev[Size07+i]
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(i+Size08)] = (-coef[Size16-1-i]*window_long[Size08+i]) - (window_short[Size01-1-i]*prev[Size07+i]);
  }
  /* Size09,...,Size16-1 */
   /* out[stride*(i+Size09)]
                  coef[Size15-1-i]
                  window_long[Size09+i]
               */
  
  for (i=0; i<Size07; i++) {
     
    out[stride*(i+Size09)] = -coef[Size15-1-i]*window_long[Size09+i];
  }
  
}
static void 
LongShortLapIllegal(float *coef,
                    float *prev,
                    float *out,
                    const float *window_long,
                    const float *window_short,
                    int stride)
{
  int i;
  
  /* 0,...,Size07-1 */
   /* out[stride*i]
                  window_long[Size16-1-i]
                  prev[Size08-1-i]
               */
  
  for (i=0; i<Size07; i++) {
     
    out[stride*i] = -window_long[Size16-1-i]*prev[Size08-1-i];
  }
  /* Size07,...,Size08-1 */
   /* out[stride*(Size07+i)]
                  coef[Size15+i]
                  window_short[i]
                  window_long[Size09-1-i]
                  prev[Size01-1-i]
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(Size07+i)] = (coef[Size15+i]*window_short[i]) - (window_long[Size09-1-i]*prev[Size01-1-i]);
  }
  /* Size08,...,Size09-1 */
   /* out[stride*(Size08+i)]
                  coef[Size16-1-i]
                  window_short[Size01+i]
                  window_long[Size08-1-i]
                  prev[i]
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(Size08+i)] = (-coef[Size16-1-i]*window_short[Size01+i]) - (window_long[Size08-1-i]*prev[i]);
  }
  /* Size09-Size16-1 */
   /* out[stride*(Size09+i)]
                  coef[Size15-1-i]
                  window_long[Size07-1-i]
                  prev[i+Size01]
               */
  
  for (i=0; i<Size07; i++) {
      
    out[stride*(Size09+i)] = -coef[Size15-1-i] - (window_long[Size07-1-i]*prev[i+Size01]);
  }
  
}
void CLongBlock_FrequencyToTime(CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
                                CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                                float outSamples[],
                                const int stride)
{
  int i;
  COverlapAddData *pOverlapAddData = &pAacDecoderStaticChannelInfo->OverlapAddData;
  const float *pLongWindow = pAacDecoderStaticChannelInfo->pLongWindow[pOverlapAddData->WindowShape];
  const float *pShortWindow = pAacDecoderStaticChannelInfo->pShortWindow[pOverlapAddData->WindowShape];
  float *pSpectralCoefficient = pAacDecoderChannelInfo->aSpectralCoefficient;
  
    
  /* Inverse IMDCT */
  
  CLongBlock_InverseTransform(pSpectralCoefficient);
  /* Overlap&Add */
     
  switch(GetWindowSequence(&pAacDecoderChannelInfo->IcsInfo))
  {
    case OnlyLongSequence:
       
      switch(pOverlapAddData->WindowSequence)
      {
        case OnlyLongSequence:
        case LongStopSequence:
           
          Lap1(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,pLongWindow,Size08,stride);
        break;
        case LongStartSequence:
        case EightShortSequence:
           
          ShortLongLapIllegal(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,pLongWindow,pShortWindow,stride);
        break;
      }
       /* pOverlapAddData->pOverlapBuffer[i]
                      pSpectralCoefficient[i]
                   */
      
      for (i=0; i<Size08; i++)
      {
        
        pOverlapAddData->pOverlapBuffer[i] = pSpectralCoefficient[i];
      }
    break;
    case LongStartSequence:
       
      switch(pOverlapAddData->WindowSequence)
      {
        case OnlyLongSequence:
        case LongStopSequence:
           
          Lap1(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,pLongWindow,Size08,stride);
        break;
        case LongStartSequence:
        case EightShortSequence:
           
          ShortLongLapIllegal(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,pLongWindow,pShortWindow,stride);
        break;
      }
        
      NoLap1(&pSpectralCoefficient[Size01],pOverlapAddData->pOverlapBuffer,1);
            
      StartLap(pSpectralCoefficient,&(pOverlapAddData->pOverlapBuffer[Size07]),1);
    break;
    case LongStopSequence:
       
      switch(pOverlapAddData->WindowSequence)
      {
        case EightShortSequence:
        case LongStartSequence:
         /* outSamples[stride*i]
                        pOverlapAddData->pOverlapBuffer[i]
                     */
        
        for (i=0; i<Size07; i++)
        {
          
          outSamples[stride*i] = pOverlapAddData->pOverlapBuffer[i];
        }
         
        StopLap(pSpectralCoefficient, pOverlapAddData->pOverlapBuffer,outSamples,pShortWindow,stride);
          
        NoLap2(&pSpectralCoefficient[Size08],&outSamples[stride*Size09],stride);
        break;
        case OnlyLongSequence:
        case LongStopSequence:
         
        LongShortLapIllegal(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,pLongWindow,pShortWindow,stride);
        break;
      }
       /* pOverlapAddData->pOverlapBuffer[i]
                      pSpectralCoefficient[i]
                   */
      
      for (i=0; i<Size08; i++)
      {
        
        pOverlapAddData->pOverlapBuffer[i] = pSpectralCoefficient[i];
      }
    break;
  }
  
    
  pOverlapAddData->WindowShape = GetWindowShape(&pAacDecoderChannelInfo->IcsInfo);
  pOverlapAddData->WindowSequence = GetWindowSequence(&pAacDecoderChannelInfo->IcsInfo);
  
}
