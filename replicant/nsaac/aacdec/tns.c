/*
  temporal noise shaping tool
*/
#include <math.h>
#include "aac_rom.h"
#include "bitbuffer.h"
#include "tns.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function returns the minimum of 3 values.
  return:  minimum value
*/
static int Minimum(int a, /*!< value 1 */
                   int b, /*!< value 2 */
                   int c) /*!< value 3 */
{
  int t;
      
  
  t = (a < b ) ? a : b;
  return (t < c) ? t : c;
}
/*
  The function returns the tns data present bit
*/
int TnsPresent(CTnsData *pTnsData) /*!< pointer to tns side info */
{
    
  
  return (pTnsData->TnsDataPresent);
}
/*
  The function converts the decoded index values into
  parcor coefficients, which are used in the lattice
  filter.
*/
static void TnsDecodeCoefficients(CFilter *filter, /*!< pointer to filter side info */
                                  float *a)        /*!< pointer to parcor coefficients */
{
  int i;
  
  for (i=0; i < filter->Order; i++)
  {
       
    if (filter->Resolution == 3)
      a[i+1] = tnsCoeff3[filter->Coeff[i]+4];
    else
      a[i+1] = tnsCoeff4[filter->Coeff[i]+8];
  }
  
}
/*
  The function applies the conversion of the parcor coefficients
  to the lpc coefficients.
*/
static void 
TnsParcor2Lpc(float *parcor, /*!< pointer to parcor coefficients */
              float *lpc,    /*!< pointer to lpc coefficients */
              int order)     /*!< filter order */
{
  int i,j;
  float z1;
  float z[MaximumOrder+1];
  float w[MaximumOrder+1];
  float accu;
  
  {  
    for (i=0; i<MaximumOrder+1; i++)
    {
      
      z[i] = 0.0;
      w[i] = 0.0;
    }
    
    for (i=0; i<=order; i++)
    {
       
      if (i == 0)
        accu = 1.0;
      else
        accu = 0.0;
      
      
      z1 = accu;
      
      
      for (j=0; j<order; j++)
      {
        
        w[j] = accu;
        
        accu += parcor[j] * z[j];
      }
      
      for (j=order-1; j>=0; j--)
      {
          
        z[j+1] = parcor[j] * w[j] + z[j];
      }
      
      z[0] = z1;
      lpc[i] = accu;
    }
  }
  
}
/*
  The function applies the tns filtering to the
  spectrum.
*/
static void 
TnsFilterIIR(float *spec,   /*!< pointer to spectrum */
             float *lpc,    /*!< pointer to lpc coefficients */
             float *state,  /*!< pointer to states */
             int size,      /*!< nunber of filtered spectral lines */
             char inc,      /*!< increment or decrement */
             int order)     /*!< filter order */
{
  int i,j;
  float accu;
  
  
  for (i=0; i<order; i++)
  {
    
    state[i] = 0.0F;
  }
   
  if (inc == -1)
  {
    
    spec += size-1;
  }
  
  for (i=0; i<size; i++)
  {
    
    accu = *spec * lpc[0];
    
    for (j=0; j<order; j++)
    {
       
      accu -= lpc[j+1] * state[j];
    }
    
    for (j=order-1; j>0; j--)
    {
      
      state[j] = state[j-1];
    }
    
    state[0] = accu;
    *spec = accu;
    
    spec += inc;
  }
  
}
/*
  The function reads the data-present flag for tns from
  the bitstream.
*/
void CTns_ReadDataPresentFlag(HANDLE_BIT_BUF bs,                              /*!< pointer to bitstream */
                              CAacDecoderChannelInfo *pAacDecoderChannelInfo) /*!< pointer to aac decoder channel info */
{
  CTnsData *pTnsData = &pAacDecoderChannelInfo->TnsData;
  
  pTnsData->TnsDataPresent = (char) GetBits(bs,1);
  
}
/*
  The function reads the elements for tns from
  the bitstream.
*/
void CTns_Read(HANDLE_BIT_BUF bs,                              /*!< pointer to bitstream */
               CAacDecoderChannelInfo *pAacDecoderChannelInfo) /*!< pointer to aac decoder channel info */
{
  char window,n_filt,order;
  char length,coef_res,coef_compress;  
  CTnsData *pTnsData = &pAacDecoderChannelInfo->TnsData;
  
    
   
  if (!pTnsData->TnsDataPresent) {
    
    return;
  }
     
  for (window = 0; window < GetWindowsPerFrame(&pAacDecoderChannelInfo->IcsInfo); window++)
  {
       
    pTnsData->NumberOfFilters[window] = n_filt = (char) GetBits(bs,IsLongBlock(&pAacDecoderChannelInfo->IcsInfo) ? 2 : 1);
    
    if (n_filt)
    {
      char index;
      char nextstopband;
      
      coef_res = (char) GetBits(bs,1);
      
      nextstopband = GetScaleFactorBandsTotal(&pAacDecoderChannelInfo->IcsInfo);
      
      for (index=0; index < n_filt; index++)
      {
        CFilter *filter = &(pTnsData->Filter[window][index]);
          
        length = (char) GetBits(bs,IsLongBlock(&pAacDecoderChannelInfo->IcsInfo) ? 6 : 4);
        
        filter->StartBand = nextstopband - length;
        
        filter->StopBand  = nextstopband;
        
        nextstopband = filter->StartBand;
           
        filter->Order = order = (char) GetBits(bs,IsLongBlock(&pAacDecoderChannelInfo->IcsInfo) ? 5 : 3);
        
        if (order)
        {
          char i,coef,s_mask,n_mask;
          static const char sgn_mask[] = {  0x2,  0x4,  0x8 };
          static const char neg_mask[] = { ~0x3, ~0x7, ~0xF };
            
          filter->Direction = GetBits(bs,1) ? -1 : 1;
          
          coef_compress = (char) GetBits(bs,1);
           
          filter->Resolution = coef_res + 3;
            
          s_mask = sgn_mask[coef_res + 1 - coef_compress];
          n_mask = neg_mask[coef_res + 1 - coef_compress];
          
          for (i=0; i < order; i++)
          {
            coef = (char) GetBits(bs,filter->Resolution - coef_compress);
             
            if (coef & s_mask) {
               
              filter->Coeff[i] =  (coef | n_mask);
            } else {
              
              filter->Coeff[i] = coef;
            }
          }
        }
      }
    }
  }
  
}
/*
  The function applies the tns to the spectrum,
*/
void CTns_Apply(CAacDecoderChannelInfo *pAacDecoderChannelInfo) /*!< pointer to aac decoder info */
{
  float tnsState[MaximumOrder];
  CTnsData *pTnsData = &pAacDecoderChannelInfo->TnsData;
  float *pSpectrum = pAacDecoderChannelInfo->aSpectralCoefficient;
  int window,index,start,stop,size;
  float lpc[MaximumOrder+1];
  float CoeffParc[MaximumOrder+1];
  
    
   
  if (!pTnsData->TnsDataPresent) {
    
    return;
  }
     
  for (window=0; window < GetWindowsPerFrame(&pAacDecoderChannelInfo->IcsInfo); window++)
  {
    for (index=0; index < pTnsData->NumberOfFilters[window]; index++)
    {
      CFilter *filter = &(pTnsData->Filter[window][index]);
      
      TnsDecodeCoefficients(filter,CoeffParc);
        
      start = Minimum(filter->StartBand,GetMaximumTnsBands(&pAacDecoderChannelInfo->IcsInfo),
                      GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo));
        
      start = GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->IcsInfo)[start];
      
      stop = Minimum(filter->StopBand, GetMaximumTnsBands(&pAacDecoderChannelInfo->IcsInfo),
                     GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->IcsInfo));
       
      stop = GetScaleFactorBandOffsets(&pAacDecoderChannelInfo->IcsInfo)[stop];
      
      size = stop - start;
      
      if (size <= 0) continue;
      
      if (filter->Order <= 0) continue;
       
      TnsParcor2Lpc(&CoeffParc[1],lpc,filter->Order);
        
      TnsFilterIIR(&pSpectrum[window*FRAME_SIZE/8+start],lpc,tnsState,size,
                   filter->Direction,filter->Order);
    }
  }
  
}
