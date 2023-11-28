/*
  decoding of short blocks
*/
#include <stdlib.h>
#include <math.h>
#include "block.h"
#include "aac_rom.h"
#include "imdct.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
void CPns_Read (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                HANDLE_BIT_BUF bs,
                const CodeBookDescription *hcb,
                unsigned char global_gain,
                int band,
                int group);
void CShortBlock_Init(CAacDecoderChannelInfo *pAacDecoderChannelInfo)
{
  int group,band;
  char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  short *pScaleFactor = pAacDecoderChannelInfo->aScaleFactor;
  
  for (group=0; group<MaximumGroups; group++)
  {
    
    for (band=0; band<MaximumScaleFactorBandsShort; band++)
    {
      
      pCodeBook[group*MaximumScaleFactorBandsShort+band] = 0;
      pScaleFactor[group*MaximumScaleFactorBandsShort+band] = 0;
    }
  }
  
}
int CShortBlock_ReadSectionData(HANDLE_BIT_BUF bs,
                                CAacDecoderChannelInfo *pAacDecoderChannelInfo)
{
  int top;
  int band;
  int group;
  char sect_cb;
  int sect_len;
  int sect_len_incr;
  int sect_esc_val = (1 << 3) - 1 ;
  char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  int ErrorStatus = AAC_DEC_OK;
    
     
     
  for (group=0; group<GetWindowGroups(&pAacDecoderChannelInfo->IcsInfo); group++)
  {
     
    for (band=0; band<GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); )
    {
      
      sect_len = 0 ;
      
      sect_cb = (char) GetBits(bs,4) ;
      
      sect_len_incr = GetBits(bs,3);
      
      while (sect_len_incr == sect_esc_val)
      {
        
        sect_len += sect_esc_val;
        
        sect_len_incr = GetBits(bs,3);
      }
      
      sect_len += sect_len_incr;
      
      top = band + sect_len;
        
      if (top + group*MaximumScaleFactorBandsShort > (MAX_WINDOWS * MAX_SFB_SHORT)) {
        
        return (AAC_DEC_DECODE_FRAME_ERROR);
      }
      
      for (; band < top; band++)
      {
        
        pCodeBook[group*MaximumScaleFactorBandsShort+band] = sect_cb;
         
        if (pCodeBook[group*MaximumScaleFactorBandsShort+band] == BOOKSCL)
        {
          
          return (AAC_DEC_INVALID_CODE_BOOK);
        }
      }
    }
     
    for ( ; band < GetScaleFactorBandsTotal(&pAacDecoderChannelInfo->IcsInfo); band++)
    {
      
      pCodeBook[group*MaximumScaleFactorBandsShort+band] = ZERO_HCB;
    }
  }
  
  return (ErrorStatus);
}
void CShortBlock_ReadScaleFactorData(HANDLE_BIT_BUF bs,
                                     CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                                     unsigned char global_gain)
{
  int temp;
  int band;
  int group;
  int position = 0;
  int factor = global_gain;
  const char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  short *pScaleFactor = pAacDecoderChannelInfo->aScaleFactor;
  const CodeBookDescription *hcb = &HuffmanCodeBooks[BOOKSCL];
  
     
     
  for (group=0; group < GetWindowGroups(&pAacDecoderChannelInfo->IcsInfo); group++)
  {
    for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
    {
      
      switch (pCodeBook[group*MaximumScaleFactorBandsShort+band])
      {
        case ZERO_HCB: /* zero book */
          
          pScaleFactor[group*MaximumScaleFactorBandsShort+band] = 0;
          break;
        default: /* decode scale factor */
           
          temp = CBlock_DecodeHuffmanWord(bs,hcb->CodeBook);
          
          factor += temp - 60; /* MIDFAC 1.5 dB */
           
          pScaleFactor[group*MaximumScaleFactorBandsShort+band] = factor - 100;
          break;
        case INTENSITY_HCB: /* intensity steering */
        case INTENSITY_HCB2:
           
          temp = CBlock_DecodeHuffmanWord(bs,hcb->CodeBook);
          
          position += temp - 60;
           
          pScaleFactor[group*MaximumScaleFactorBandsShort+band] = position - 100;
          break;
        case NOISE_HCB: /* PNS */
          
          CPns_Read(pAacDecoderChannelInfo, bs, hcb, global_gain, band, group);
          break;
      }
    }
  }
  
}
int  CShortBlock_ReadSpectralData(HANDLE_BIT_BUF bs,
                                  CAacDecoderChannelInfo *pAacDecoderChannelInfo)
{
  int i,index,step;
  int window,group,groupwin,groupoffset,band;
  int scfExp,scfMod;
  int *QuantizedCoef;
  const char *pCodeBook = pAacDecoderChannelInfo->aCodeBook;
  const short *pScaleFactor = pAacDecoderChannelInfo->aScaleFactor;
  float *pSpectralCoefficient = pAacDecoderChannelInfo->aSpectralCoefficient;
  const short *BandOffsets = GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->IcsInfo);
  const CodeBookDescription *hcb;
  
  QuantizedCoef = (int*)pSpectralCoefficient;
     
  
  for (window=0; window < MaximumWindows; window++)
  {
     /* pointer for QuantizedCoef[] */
    
    for (index=0; index < MaximumBinsShort; index++) {
      
      QuantizedCoef[window*MaximumBinsShort+index] = 0;
    }
  }
  
  groupoffset = 0;
    
  for (group=0; group < GetWindowGroups(&pAacDecoderChannelInfo->IcsInfo); group++)
  {
     /* pointer for pCodeBook[] */
      
    for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
    {
      
      hcb = &HuffmanCodeBooks[pCodeBook[group*MaximumScaleFactorBandsShort+band]];
        
      for (groupwin=0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->IcsInfo,group); groupwin++)
      {
        
        window = groupoffset + groupwin;
          
        if (  (pCodeBook[group*MaximumScaleFactorBandsShort+band] == ZERO_HCB)
            ||(pCodeBook[group*MaximumScaleFactorBandsShort+band] == INTENSITY_HCB)
            ||(pCodeBook[group*MaximumScaleFactorBandsShort+band] == INTENSITY_HCB2)
            ||(pCodeBook[group*MaximumScaleFactorBandsShort+band] == NOISE_HCB))
          continue;
        
        step = 0 ;
         /* pointer for BandOffsets[],
                                    QuantizedCoef[] */
        
        for (index=BandOffsets[band]; index < BandOffsets[band+1]; index+=step)
        {
             
          step = CBlock_UnpackIndex(CBlock_DecodeHuffmanWord(bs,hcb->CodeBook),&QuantizedCoef[window*MaximumBinsShort+index],hcb);
           
          if (hcb->Offset == 0)
          {
             /* pointer for QuantizedCoef[] */
            
            for (i=0; i < step; i++)
            {
              
              if (QuantizedCoef[window*MaximumBinsShort+index+i])
              {
                
                if (GetBits(bs,1)) /* sign bit */
                {
                   
                  QuantizedCoef [window*MaximumBinsShort+index+i] = -QuantizedCoef [window*MaximumBinsShort+index+i];
                }
              }
            }
          }
           
          if (pCodeBook[group*MaximumScaleFactorBandsShort+band] == ESCBOOK)
          {
             
            QuantizedCoef[window*MaximumBinsShort+index] = CBlock_GetEscape(bs,QuantizedCoef[window*MaximumBinsShort+index]);
             
            QuantizedCoef[window*MaximumBinsShort+index+1] = CBlock_GetEscape(bs,QuantizedCoef[window*MaximumBinsShort+index+1]);
  
                
            if (abs(QuantizedCoef[window*MaximumBinsShort+index]) > MAX_QUANTIZED_VALUE || abs(QuantizedCoef[window*MaximumBinsShort+index+1]) > MAX_QUANTIZED_VALUE) {
              
              return (AAC_DEC_DECODE_FRAME_ERROR);
            }
          }
        }
      }
    }
     
    groupoffset += GetWindowGroupLength(&pAacDecoderChannelInfo->IcsInfo,group);
  }
    
  for (window=0, group=0; group < GetWindowGroups(&pAacDecoderChannelInfo->IcsInfo); group++)
  {
      
    for (groupwin=0; groupwin < GetWindowGroupLength(&pAacDecoderChannelInfo->IcsInfo,group); groupwin++, window++)
    {
      
      index = 0;
       /* pointer for BandOffsets[],
                                  pScaleFactor     */
        LOOP(1) ;
      /* quantize & apply scalefactors */
      for (band=0; band < GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo); band++)
      {
        /* scalefactor exponents and scalefactor mantissa for current band */
        
        scfExp = pScaleFactor[group*MaximumScaleFactorBandsShort+band] >> 2;
        
        scfMod = pScaleFactor[group*MaximumScaleFactorBandsShort+band] & 3;
         /* pointer for QuantizedCoef[], 
                                    pSpectralCoefficient */
        
        for (index=BandOffsets[band]; index < BandOffsets[band+1] ;index++)
        {
            
          pSpectralCoefficient[window*MaximumBinsShort+index] = CBlock_Quantize(QuantizedCoef[window*MaximumBinsShort+index],scfMod,scfExp-6);
        }
      }
       /* pointer for pSpectralCoefficient */
      
      for (; index < MaximumBinsShort; index++) {
        
        pSpectralCoefficient[window*MaximumBinsShort+index] = 0.0;
      }
    }
  }
  
  return (AAC_DEC_OK);
}
static void 
LongShortLapIllegal(float *current,
                    float *prev,
                    float *out,
                    const float *shortWindow,
                    const float *shortWindowPrev,
                    const float *longWindowPrev,
                    int stride)
{
  int i;
  
  /* 0,...,Size07-1 */
   /* out[stride*i]
                  longWindowPrev[Size16-1-i]
                  prev[Size08-1-i]
               */
  
  for (i=0; i<Size07; i++) {
     
    out[stride*i] = -longWindowPrev[Size16-1-i]*prev[Size08-1-i];
  }
  /* Size07,...,Size08-1 */
   /* out[stride*(Size07+i)]
                  current[Size01+i]
                  shortWindowPrev[i]
                  longWindowPrev[Size09-1-i]
                  prev[Size01-1-i]
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(Size07+i)] = (current[Size01+i]*shortWindowPrev[i]) - (longWindowPrev[Size09-1-i]*prev[Size01-1-i]);
  }
  /* Size08,...,Size09-1 */
   /* out[stride*(Size08+i)]
                  current[Size02-1-i]
                  shortWindowPrev[Size01+i]
                  longWindowPrev[Size08-1-i]
                  prev[i]
               */
  
  for (i=0; i<Size01; i++) {
      
    out[stride*(Size08+i)] = (-current[Size02-1-i]*shortWindowPrev[Size01+i]) - (longWindowPrev[Size08-1-i]*prev[i]);
  }
  /* Size09,...,Size10-1 */
   /* out[stride*(Size09+i)]
                  current[Size03+i]
                  current[Size01-1-i]
                  shortWindow[i]
                  shortWindow[Size02-1-i]
                  longWindowPrev[Size07-1-i]
                  prev[i+Size01]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size09+i)] = (current[Size03+i]*shortWindow[i]) - (shortWindow[Size02-1-i]*current[Size01-1-i]) - (longWindowPrev[Size07-1-i]*prev[i+Size01]);
  }
  /* Size10,...,Size11-1 */
   /* out[stride*(Size10+i)]
                  current[Size04-1-i]
                  current[i]
                  shortWindow[Size01+i]
                  shortWindow[Size01-1-i]
                  longWindowPrev[Size06-1-i]
                  prev[i+Size02]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size10+i)] = (-current[Size04-1-i]*shortWindow[Size01+i]) - (shortWindow[Size01-1-i]*current[i]) - (longWindowPrev[Size06-1-i]*prev[i+Size02]);
  }
  /* Size11,...,Size12-1 */
   /* out[stride*(Size11+i)]
                  current[Size05+i]
                  current[Size03-1-i]
                  shortWindow[i]
                  shortWindow[Size02-1-i]
                  longWindowPrev[Size05-1-i]
                  prev[i+Size03]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2);  /* calculating: (-1 * (current[Size05+i]*shortWindow[i])) + (shortWindow[Size02-1-i]*current[Size03-1-i]) + (longWindowPrev[Size05-1-i]*prev[i+Size03]) */
    out[stride*(Size11+i)] = (current[Size05+i]*shortWindow[i]) - (shortWindow[Size02-1-i]*current[Size03-1-i]) - (longWindowPrev[Size05-1-i]*prev[i+Size03]);
  }
  /* Size12,...,Size13-1 */
   /* out[stride*(Size12+i)]
                  current[Size06-1-i]
                  current[Size02+i]
                  shortWindow[Size01+i]
                  shortWindow[Size01-1-i]
                  longWindowPrev[Size04-1-i]
                  prev[i+Size04]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size12+i)] = (-current[Size06-1-i]*shortWindow[Size01+i]) - (shortWindow[Size01-1-i]*current[Size02+i]) - (longWindowPrev[Size04-1-i]*prev[i+Size04]);
  }
  /* Size13,...,Size14-1 */
   /* out[stride*(Size13+i)]
                  current[Size07+i]
                  current[Size05-1-i]
                  shortWindow[i]
                  shortWindow[Size02-1-i]
                  longWindowPrev[Size03-1-i]
                  prev[i+Size05]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size13+i)] = (current[Size07+i]*shortWindow[i]) - (shortWindow[Size02-1-i]*current[Size05-1-i]) - (longWindowPrev[Size03-1-i]*prev[i+Size05]);
  }
  /* Size14,...,Size15-1 */
   /* out[stride*(Size14+i)]
                  current[Size08-1-i]
                  shortWindow[Size01+i]
                  shortWindow[Size01-1-i]
                  current[Size04+i]
                  longWindowPrev[Size02-1-i]
                  prev[i+Size06]
               */
  
  for(i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size14+i)] = (-current[Size08-1-i]*shortWindow[Size01+i]) - (shortWindow[Size01-1-i]*current[Size04+i]) - (longWindowPrev[Size02-1-i]*prev[i+Size06]);
  }
  /* Size15,...,Size16-1 */
   /* out[stride*(Size15+i)]
                  current[Size09+i]
                  current[Size07-1-i]
                  shortWindow[i]
                  shortWindow[Size02-1-i]
                  longWindowPrev[Size01-1-i]
                  prev[i+Size07]
               */
  
  for (i=0; i<Size01; i++) {
     MAC(2); 
    out[stride*(Size15+i)] = (current[Size09+i]*shortWindow[i]) - (shortWindow[Size02-1-i]*current[Size07-1-i]) - (longWindowPrev[Size01-1-i]*prev[i+Size07]);
  }
   /* prev[i]
                  current[Size10-1-i]
                  current[Size06+i]
                  shortWindow[Size01+i]
                  shortWindow[Size01-1-i]
               */
  
  for (i=0; i<Size01; i++) {
      
    prev[i] = -current[Size10-1-i]*shortWindow[Size01+i] - shortWindow[Size01-1-i]*current[Size06+i];
  }
  
}
void CShortBlock_FrequencyToTime(CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
                                 CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                                 float outSamples[],
                                 const int stride)
{
  int i;
  COverlapAddData *pOverlapAddData = &pAacDecoderStaticChannelInfo->OverlapAddData;
  const float *shortWindow = pAacDecoderStaticChannelInfo->pShortWindow[GetWindowShape(&pAacDecoderChannelInfo->IcsInfo)];
  const float *shortWindowPrev = pAacDecoderStaticChannelInfo->pShortWindow[pOverlapAddData->WindowShape];
  const float *longWindowPrev = pAacDecoderStaticChannelInfo->pLongWindow[pOverlapAddData->WindowShape];
  float *pSpectralCoefficient = pAacDecoderChannelInfo->aSpectralCoefficient;
  
  /* Inverse MDCT */
  
  for (i=0; i<MaximumWindows; i++) {
    
    CShortBlock_InverseTransform(&pSpectralCoefficient[i*Size02]);
  }
  /* Overlap & Add */
   
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
        
      Lap1(&pSpectralCoefficient[0],&pOverlapAddData->pOverlapBuffer[Size07],
           &outSamples[stride*Size07],shortWindowPrev,Size01,stride);
        
      Lap1(&pSpectralCoefficient[Size02],&pSpectralCoefficient[0],
           &outSamples[stride*Size09],shortWindow,Size01,stride);
        
      Lap1(&pSpectralCoefficient[Size04],&pSpectralCoefficient[Size02],
           &outSamples[stride*Size11],shortWindow,Size01,stride);
        
      Lap1(&pSpectralCoefficient[Size06],&pSpectralCoefficient[Size04],
           &outSamples[stride*Size13],shortWindow,Size01,stride);
       
      Lap2(&pSpectralCoefficient[Size08],&pSpectralCoefficient[Size06],
           pOverlapAddData->pOverlapBuffer,shortWindow,Size01,1);
       /* outSamples[stride*(Size15+i)]
                      pOverlapAddData->pOverlapBuffer[i]
                      pOverlapAddData->pOverlapBuffer[i+Size01]
                   */
      
      for (i=0; i<Size01; i++)
      {
        
        outSamples[stride*(Size15+i)] = pOverlapAddData->pOverlapBuffer[i];
        pOverlapAddData->pOverlapBuffer[i] = pOverlapAddData->pOverlapBuffer[i+Size01];
      }
    break;
    case OnlyLongSequence:
    case LongStopSequence:
       
      LongShortLapIllegal(pSpectralCoefficient,pOverlapAddData->pOverlapBuffer,outSamples,
                          shortWindow,shortWindowPrev,longWindowPrev,stride);
    break;
  }
   
  Lap2(&pSpectralCoefficient[Size10],&pSpectralCoefficient[Size08],
       &pOverlapAddData->pOverlapBuffer[Size01],shortWindow,Size01,1);
   
  Lap2(&pSpectralCoefficient[Size12],&pSpectralCoefficient[Size10],
       &pOverlapAddData->pOverlapBuffer[Size03],shortWindow,Size01,1);
   
  Lap2(&pSpectralCoefficient[Size14],&pSpectralCoefficient[Size12],
       &pOverlapAddData->pOverlapBuffer[Size05],shortWindow,Size01,1);
   /* pOverlapAddData->pOverlapBuffer[i+Size07]
                  pSpectralCoefficient[Size14+i]
               */
  
  for (i=0; i<Size01; i++)
  {
    
    pOverlapAddData->pOverlapBuffer[i+Size07] = pSpectralCoefficient[Size14+i];
  }
      
  pOverlapAddData->WindowShape = GetWindowShape(&pAacDecoderChannelInfo->IcsInfo);
  pOverlapAddData->WindowSequence = GetWindowSequence(&pAacDecoderChannelInfo->IcsInfo);
  
}
