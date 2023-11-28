/*
  Hybrid Filter Bank
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "hybrid.h"
#include "math/cfftn.h"
#include "sbr_rom.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
static void twoChannelFiltering( const float *pQmf,
                                 float *mHybrid )
{
  int n;
  float cum0, cum1;
  
   
  
  cum0 = 0.5f * pQmf[HYBRID_FILTER_DELAY];
  
  cum1 = 0;
   
  for(n = 0; n < 6; n++) {
     
    cum1 += p2_6[n] * pQmf[2*n+1];
  }
   
  mHybrid[0] = cum0 + cum1;
   
  mHybrid[1] = cum0 - cum1;
  
}
static void eightChannelFiltering( const float *pQmfReal,
                                   const float *pQmfImag,
                                   float *mHybridReal,
                                   float *mHybridImag )
{
  int n;
  float real, imag;
  float cum[16];
  
  
   
  real = p8_13[4]  * pQmfReal[4] +
         p8_13[12] * pQmfReal[12];
   
  imag = p8_13[4]  * pQmfImag[4] +
         p8_13[12] * pQmfImag[12];
    
  cum[4] =  (imag - real) * 0.70710678118655f;
    
  cum[5] = -(imag + real) * 0.70710678118655f;
   
  real = p8_13[3]  * pQmfReal[3] +
         p8_13[11] * pQmfReal[11];
   
  imag = p8_13[3]  * pQmfImag[3] +
         p8_13[11] * pQmfImag[11];
    
  cum[6] =   imag * 0.92387953251129f - real * 0.38268343236509f;
    
  cum[7] = -(imag * 0.38268343236509f + real * 0.92387953251129f);
     
  cum[9] = -( p8_13[2]  * pQmfReal[2] +
              p8_13[10] * pQmfReal[10] );
    
  cum[8] =    p8_13[2]  * pQmfImag[2] +
              p8_13[10] * pQmfImag[10];
   
  real = p8_13[1]  * pQmfReal[1] +
         p8_13[9] * pQmfReal[9];
   
  imag = p8_13[1]  * pQmfImag[1] +
         p8_13[9] * pQmfImag[9];
    
  cum[10] = imag * 0.92387953251129f + real * 0.38268343236509f;
    
  cum[11] = imag * 0.38268343236509f - real * 0.92387953251129f;
   
  real = p8_13[0]  * pQmfReal[0] +
         p8_13[8] * pQmfReal[8];
   
  imag = p8_13[0]  * pQmfImag[0] +
         p8_13[8] * pQmfImag[8];
    
  cum[12] = (imag + real) * 0.70710678118655f;
    
  cum[13] = (imag - real) * 0.70710678118655f;
  
  real = p8_13[7]  * pQmfReal[7];
  imag = p8_13[7]  * pQmfImag[7];
    
  cum[14] = imag * 0.38268343236509f + real * 0.92387953251129f;
    
  cum[15] = imag * 0.92387953251129f - real * 0.38268343236509f;
   
  cum[0] = p8_13[HYBRID_FILTER_DELAY]  * pQmfReal[HYBRID_FILTER_DELAY];
  cum[1] = p8_13[HYBRID_FILTER_DELAY]  * pQmfImag[HYBRID_FILTER_DELAY];
  
  real = p8_13[5]  * pQmfReal[5];
  imag = p8_13[5]  * pQmfImag[5];
    
  cum[2] = real * 0.92387953251129f - imag * 0.38268343236509f;
    
  cum[3] = real * 0.38268343236509f + imag * 0.92387953251129f;
  
  CFFTN(cum, 8, 1);
   
  for(n = 0; n < 8; n++) {
    
    mHybridReal[n] = cum[2*n];
    mHybridImag[n] = cum[2*n+1];
  }
  
}
/**************************************************************************/
/*
  \brief HybridAnalysis
  \return none.
*/
/**************************************************************************/
void HybridAnalysis(const float **mQmfReal, const float **mQmfImag, float *mHybridReal, float *mHybridImag, HANDLE_HYBRID hHybrid)
{
  int band, oddQmf;
  HYBRID_RES hybridRes;
  int  chOffset = 0;
   
  for(band = 0; band < hHybrid->nQmfBands; band++) 
	{    
    oddQmf = (band & 1);
     
    hybridRes = (HYBRID_RES)hHybrid->pResolution[band];
     
    memcpy(hHybrid->pWorkReal, hHybrid->mQmfBufferReal[band], hHybrid->qmfBufferMove * sizeof(float));
    memcpy(hHybrid->pWorkImag, hHybrid->mQmfBufferImag[band], hHybrid->qmfBufferMove * sizeof(float));
    
    hHybrid->pWorkReal[hHybrid->qmfBufferMove] = mQmfReal[HYBRID_FILTER_DELAY][band];
    hHybrid->pWorkImag[hHybrid->qmfBufferMove] = mQmfImag[HYBRID_FILTER_DELAY][band];
    memcpy(hHybrid->mQmfBufferReal[band], hHybrid->pWorkReal + 1, hHybrid->qmfBufferMove * sizeof(float));
    memcpy(hHybrid->mQmfBufferImag[band], hHybrid->pWorkImag + 1, hHybrid->qmfBufferMove * sizeof(float));
    switch(hybridRes) 
		{
    case HYBRID_2_REAL:
       
      
      twoChannelFiltering( hHybrid->pWorkReal,
                           hHybrid->mTempReal );
      
      twoChannelFiltering( hHybrid->pWorkImag,
                           hHybrid->mTempImag );
      
      
      mHybridReal[chOffset] = hHybrid->mTempReal [0];
      mHybridImag[chOffset] = hHybrid->mTempImag [0];
      mHybridReal[chOffset + 1] = hHybrid->mTempReal [1];
      mHybridImag [chOffset + 1] = hHybrid->mTempImag [1];
      
      chOffset += 2;
      break;
    case HYBRID_8_CPLX:
       
      
      eightChannelFiltering( hHybrid->pWorkReal,
                             hHybrid->pWorkImag,
                             hHybrid->mTempReal,
                             hHybrid->mTempImag );
      
       
      mHybridReal[chOffset + 0] = hHybrid->mTempReal [0];
      mHybridImag[chOffset + 0] = hHybrid->mTempImag [0];
      mHybridReal[chOffset + 1] = hHybrid->mTempReal [1];
      mHybridImag[chOffset + 1] = hHybrid->mTempImag [1];
      mHybridReal[chOffset + 2] = hHybrid->mTempReal [2] + hHybrid->mTempReal [5];
      mHybridImag[chOffset + 2] = hHybrid->mTempImag [2] + hHybrid->mTempImag [5];
      mHybridReal[chOffset + 3] = hHybrid->mTempReal [3] + hHybrid->mTempReal [4];
      mHybridImag[chOffset + 3] = hHybrid->mTempImag [3] + hHybrid->mTempImag [4];
      mHybridReal[chOffset + 4] = hHybrid->mTempReal [6];
      mHybridImag[chOffset + 4] = hHybrid->mTempImag [6];
      mHybridReal[chOffset + 5] = hHybrid->mTempReal [7];
      mHybridImag[chOffset + 5] = hHybrid->mTempImag [7];
      
      chOffset += 6;
      break;
    default:
      assert(0);
    }
  }
  
}
/**************************************************************************/
/*
  \brief
  \return none.
*/
/**************************************************************************/
void HybridSynthesis(const float *mHybridReal, const float *mHybridImag, float *mQmfReal, float *mQmfImag, HANDLE_HYBRID hHybrid)
{
  int  k, band;
  HYBRID_RES hybridRes;
  int  chOffset = 0;
     
  for(band = 0; band < hHybrid->nQmfBands; band++)
	{     
    hybridRes = (HYBRID_RES)min(hHybrid->pResolution[band],6);    
    mQmfReal[band] = mQmfImag[band] = 0;
     
    for(k = 0; k < hybridRes; k++) 
		{      
      mQmfReal[band] += mHybridReal[chOffset + k];
      mQmfImag[band] += mHybridImag[chOffset + k];
    }    
    
    chOffset += hybridRes;
  }
  
}
/**************************************************************************/
/*
  \brief    CreateHybridFilterBank
  \return   errorCode, noError if successful.
*/
/**************************************************************************/
int CreateHybridFilterBank(HANDLE_HYBRID hs, int noBands, const int *pResolution)
{
  int i;
  int maxNoChannels = 0;
   
  for (i = 0; i < noBands; i++) 
	{    
    hs->pResolution[i] = pResolution[i];
      
    if( pResolution[i] != HYBRID_8_CPLX &&
        pResolution[i] != HYBRID_2_REAL &&
        pResolution[i] != HYBRID_4_CPLX )
    {      
      return 1;
    }
     
    if(pResolution[i] > maxNoChannels)
    {      
      maxNoChannels = pResolution[i];
    }
  }
   
  hs->nQmfBands     = noBands;
  hs->qmfBufferMove = HYBRID_FILTER_LENGTH - 1;
    
  for (i = 0; i < noBands; i++) 
	{
    memset(hs->mQmfBufferReal[i],0,hs->qmfBufferMove*sizeof(float));
    memset(hs->mQmfBufferImag[i],0,hs->qmfBufferMove*sizeof(float));
  }
   
  memset(hs->mTempReal,0,maxNoChannels*sizeof(float));
  memset(hs->mTempImag,0,maxNoChannels*sizeof(float));
  
  return 0;
}
