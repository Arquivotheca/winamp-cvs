/*
  long/short decoding
*/
#include <assert.h>
#include <math.h>
#include "aac_rom.h"
#include "bitbuffer.h"
#include "math/FloatFR.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
void CTns_ReadDataPresentFlag(HANDLE_BIT_BUF bs, CAacDecoderChannelInfo *pAacDecoderChannelInfo);
void CTns_Read(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo);
void CTns_Apply(CAacDecoderChannelInfo *pAacDecoderChannelInfo);
void CPns_Apply(CAacDecoderChannelInfo pAacDecoderChannelInfo[], int channel);
float CBlock_Quantize(int value, int scfMod, int scale)
{
  if (value != 0)
  {
    float scalefactor = (float) pow (2.0F, 0.25F * scfMod + scale);
    if (value >= 0.0) 
		{      
      return (float) (pow (value, 4.0 / 3.0) * scalefactor) ;
    }
    else 
		{      
      return (float) (-pow (-value, 4.0 / 3.0) * scalefactor) ;
    }
  }
  else 
	{    
    return (0.0);
  }
}
/*
  The function reads the huffman codeword from the bitstream and
  returns the index value.
  return:  index value
*/
int CBlock_DecodeHuffmanWord(HANDLE_BIT_BUF bs,                                 /*!< pointer to bitstream */
                             const unsigned short (*CodeBook) [HuffmanEntries]) /*!< pointer to sideinfo of the current codebook */
{
  unsigned int val;
  unsigned int index;
  
  
  index = 0;
  
  while (1)
  {
     
    val = CodeBook[index][GetBits(bs,HuffmanBits)];
 
     
    if ((val & 1) == 0)
    {
      
      index = val>>2;
      continue;
    }
    else
    {
       
      if (val & 2)
      {
        
        PushBack(bs,1);
      }
      
      val >>= 2;
      break;
    }
  }
  
  return val;
}
/*
  The function reads the escape sequence from the bitstream,
  if the absolute value of the quantized coefficient has the
  value 16.
  return:  quantized coefficient
*/
int CBlock_GetEscape(HANDLE_BIT_BUF bs,  /*!< pointer to bitstream */
                     const int q)        /*!< quantized coefficient */
{
  int i, off, neg ;
  
  
  if (q < 0)
  {
     
    if (q != -16) {
      
      return q;
    }
    
    neg = 1;
  }
  else
  {
     
    if (q != +16) {
      
      return q;
    }
    
    neg = 0;
  }
  
  for (i=4; ; i++)
  {
     
    if (GetBits(bs,1) == 0)
      break;
  }
   
  if (i > 16)
  {
     
    if (i - 16 > (int) LongSize) { /* cannot read more than "LongSize" bits at once in the function ReadBits() */
      
      return (MAX_QUANTIZED_VALUE + 1); /* returning invalid value that will be captured later */
    }
      
    off = GetBits(bs,i-16) << 16;
     
    off |= GetBits(bs,16);
  }
  else
  {
    
    off = GetBits(bs,i);
  }
   
  i = off + (1 << i);
  
  if (neg) {
    
    i = -i;
  }
  
  return i;
}
/*
  The function converts the index values to quantized coefficients.
  return:  dimension
*/
int CBlock_UnpackIndex(int idx,                        /*!< pointer to index */
                       int *qp,                        /*!< pointer to quantized coefficients */
                       const CodeBookDescription *hcb) /*!< pointer to sideinfo of the current codebook */
{
  int offset, bits, mask;
  
   
  offset = hcb->Offset;
  bits = hcb->numBits;
   
  mask = (1<<bits)-1;
    
  if (hcb->Dimension == 4)
  {
      
    qp[0] = (idx & mask)-offset;
    
    idx >>= bits;
      
    qp[1] = (idx & mask)-offset;
    
    idx >>= bits;
      
    qp[2] = (idx & mask)-offset;
    
    idx >>= bits;
      
    qp[3] = (idx & mask)-offset;;
  }
  else
  {
      
    qp[0] = (idx & mask)-offset;
    
    idx >>= bits;
      
    qp[1] = (idx & mask)-offset;
  }
  return hcb->Dimension;
}
/*
  The function reads the element of the individual channel stream for
  long blocks.
  return:  none
*/
int CLongBlock_Read(HANDLE_BIT_BUF bs,                              /*!< pointer to bitstream */
                    CAacDecoderChannelInfo *pAacDecoderChannelInfo, /*!< pointer to aac decoder channel info */
                    unsigned char global_gain)                      /*!< global gain */
{
  int GainControlDataPresent;
  int ErrorStatus = AAC_DEC_OK;
  
   
  
    
  if ((ErrorStatus = CLongBlock_ReadSectionData(bs,pAacDecoderChannelInfo)))
  {
    
    return (ErrorStatus);
  }
  
  CLongBlock_ReadScaleFactorData(bs,pAacDecoderChannelInfo,global_gain);
    
  CPulseData_Read(bs,&pAacDecoderChannelInfo->PulseData);
  
  CTns_ReadDataPresentFlag(bs,pAacDecoderChannelInfo);
  
  CTns_Read(bs,pAacDecoderChannelInfo);
  
  GainControlDataPresent = GetBits(bs,1);
  
  if (GainControlDataPresent)
  {
    
    return (AAC_DEC_UNIMPLEMENTED_GAIN_CONTROL_DATA);
  }
    
  if ((ErrorStatus = CLongBlock_ReadSpectralData(bs, pAacDecoderChannelInfo)))
  {
    
    return (ErrorStatus);
  }
  
  return (ErrorStatus);
}
/*
  The function reads the element of the individual channel stream for
  short blocks. Gain control data is not supported.
*/
int CShortBlock_Read(HANDLE_BIT_BUF bs,                              /*!< pointer to bitstream */
                     CAacDecoderChannelInfo *pAacDecoderChannelInfo, /*!< pointer to aac decoder channel info */
                     unsigned char global_gain)                      /*!< global gain */
{
  int GainControlDataPresent;
  int ErrorStatus = AAC_DEC_OK;
  
  
   
    
  if ((ErrorStatus = CShortBlock_ReadSectionData(bs,pAacDecoderChannelInfo)))
  {
    
    return (ErrorStatus);
  }
  
  CShortBlock_ReadScaleFactorData(bs,pAacDecoderChannelInfo,global_gain);
    
  CPulseData_Read(bs,&pAacDecoderChannelInfo->PulseData);
  
  CTns_ReadDataPresentFlag(bs,pAacDecoderChannelInfo);
  
  CTns_Read(bs,pAacDecoderChannelInfo);
  
  GainControlDataPresent = GetBits(bs,1);
  
  if (GainControlDataPresent)
  {
    
    return (AAC_DEC_UNIMPLEMENTED_GAIN_CONTROL_DATA);
  }
    
  if ((ErrorStatus = CShortBlock_ReadSpectralData(bs,pAacDecoderChannelInfo)))
  {
    
    return (ErrorStatus);   
  }
  
  return (ErrorStatus);
}
void ApplyTools(CAacDecoderChannelInfo pAacDecoderChannelInfo[],
                int channel)
{
  
  
  CPns_Apply(pAacDecoderChannelInfo, channel);
   
  CTns_Apply(&pAacDecoderChannelInfo[channel]);
  
}
void 
Lap1(float *coef,         /*!< pointer to current spectrum */
     float *prev,         /*!< pointer to previous spectrum */
     float *out,          /*!< pointer to output time samples */
     const float *window, /*!< pointer to window coefficients */
     int size,            /*!< number of spectral lines */
     int stride)          /*!< stride */
{
  int i;
  
  for (i=0; i<size; i++) {
      
    out[stride*i] = (coef[size+i]*window[i]) - (window[size*2-1-i]*prev[size-1-i]);
  }

  
  for (i=0; i<size; i++) {
      
    out[stride*(i+size)] = (-coef[size*2-1-i]*window[size+i]) - (window[size-1-i]*prev[i]);
  }
  
}
void 
Lap2(float *coef,         /*!< pointer to current spectrum */
     float *prev,         /*!< pointer to previous spectrum */
     float *out,          /*!< pointer to output time samples */
     const float *window, /*!< pointer to window coefficients */
     int size,            /*!< number of spectral lines */
     int stride)          /*!< stride */
{
  int i;
  
  
  for (i=0; i<size; i++) {
      
    out[stride*i] = coef[size+i]*window[i] - window[size*2-1-i]*prev[size-1-i];
  }

  
  for (i=0; i<size; i++) {
      
    out[stride*(i+size)] = -coef[size*2-1-i]*window[size+i] - window[size-1-i]*prev[i];
  }
  
}
