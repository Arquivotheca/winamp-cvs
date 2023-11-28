/*
  Complex qmf analysis/synthesis
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "sbr_fft.h"
#include "qmf_dec.h"
#include "string.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "math/counters.h"
#define QMF_ANA_FILTER_STRIDE  2
/*
    Perform dct type 4
 */
static void
dct4 (float *data,
      int L,
      HANDLE_SBR_QMF_FILTER_BANK qmfBank
      )
{
  int i, M, ld;
  float wim, wre;
  float re1, im1, re2, im2;
  struct dct4Twiddle *pTwiddle;
  const float cosPiBy8  = 0.92387953251129f;
  const float cosPi3By8 = 0.38268343236509f;
  
  
  i = 1;
  
  M = L / 2;
  
  ld = -2;
   
  if (L > 2) {
    
    while(i < L) {
      
      i <<= 1;
      
      ld++;
    }
     
    pTwiddle = &qmfBank->pDct4Twiddle[ld];
     /* M / 2 */
      /* pointers for data[2 * i],
                                          data[2 * M - 2 * i],
                                          pTwiddle->sin_twiddle[],
                                          pTwiddle->cos_twiddle[],
                                          pTwiddle->sin_twiddle[M - 1 - i],
                                          pTwiddle->cos_twiddle[M - 1 - i] */
    for (i = 0; i < M / 2; i++) {
      
      re1 = data[2 * i];
      im2 = data[2 * i + 1];
      re2 = data[2 * M - 2 - 2 * i];
      im1 = data[2 * M - 1 - 2 * i];
      
      wim = pTwiddle->sin_twiddle[i];
      wre = pTwiddle->cos_twiddle[i];
        
      data[2 * i]     = im1 * wim + re1 * wre;
        
      data[2 * i + 1] = im1 * wre - re1 * wim;
      
      wim = pTwiddle->sin_twiddle[M - 1 - i];
      wre = pTwiddle->cos_twiddle[M - 1 - i];
        
      data[2 * M - 2 - 2 * i] = im2 * wim + re2 * wre;
        
      data[2 * M - 1 - 2 * i] = im2 * wre - re2 * wim;
    }
     
    if (M == 2) {
      
      re1 = data[0];
      im1 = data[1];
       
      data[0] = re1 + data[2];
      data[1] = im1 + data[3];
      data[2] = re1 - data[2];
      data[3] = im1 - data[3];
    }
    else {
      
      sbrfft(data,M);
    }
     
    wim = pTwiddle->alt_sin_twiddle[0];
    wre = pTwiddle->alt_sin_twiddle[M];
      /* pointers for data[2 * i],
                                  data[2 * M - 2 * i],
                                  pTwiddle->alt_sin_twiddle[i],
                                  pTwiddle->alt_sin_twiddle[M - 1 - i] */
    
    for (i = 0; i < M / 2; i++) {
      
      re1 = data[2 * i];
      im1 = data[2 * i + 1];
      re2 = data[2 * M - 2 - 2 * i];
      im2 = data[2 * M - 1 - 2 * i];
        
      data[2 * i]             = re1 * wre + im1 * wim;
        
      data[2 * M - 1 - 2 * i] = re1 * wim - im1 * wre;
      
      wim = pTwiddle->alt_sin_twiddle[i + 1];
      wre = pTwiddle->alt_sin_twiddle[M - 1 - i];
        
      data[2 * M - 2 - 2 * i] = re2 * wim + im2 * wre;
        
      data[2 * i + 1]         = re2 * wre - im2 * wim;
    }
  }
  else { /* 2-point transform */
    
    re1 = data[0];
    re2 = data[1];
      
    data[0] = re1 * cosPiBy8  + re2 * cosPi3By8;
      
    data[1] = re1 * cosPi3By8 - re2 * cosPiBy8;
  }
}
/*
  Perform dct type 3
 */
static void
dct3 (float *data,
      int L,
      HANDLE_SBR_QMF_FILTER_BANK qmfBank
      )
{
  int i, M, N;
  float s1, s2, s3, s4;
  float temp[16];
  const float sqrtHalf  = 0.70710678118655f;
  
  
  M = L / 2;
  
  N = L / 4;
   
  if (L > 2) {
    for(i = 0; i < N; i++) {
      
      temp[i] = data[2*i+1];
    }
      /* pointers for data[2 * i],
                                  data[i] */
    
    for(i = 1; i < M; i++) {
      
      data[i] = data[2*i];
    }
      /* pointers for data[L-1-i],
                                  data[L-1 - 2*i] */
    
    for(i = 1; i < N; i++) {
      
      data[L-1 - i] = data[L-1 - 2*i];
    }
      /* pointers for data[i+M],
                                  temp[i] */
    
    for(i = 0; i < N; i++) {
      
      data[i + M] = temp[i];
    }
    
    dct3(data,     M, qmfBank);
    
    dct4(data + M, M, qmfBank);
      /* pointers for data[i],
                                  data[i + M],
                                  data[M-1 - i],
                                  data[L-1 - i] */
    
    for(i = 0; i < N; i++) {
      
      s1 = data[i];
      s2 = data[i + M];
      s3 = data[M-1 - i];
      s4 = data[L-1 - i];
       
      data[i]       = (s1 + s2);
      data[L-1 - i] = (s1 - s2);
      data[M-1 - i] = (s3 + s4);
      data[i + M]   = (s3 - s4);
    }
  }
  else {
    
    s1 = data[1] * sqrtHalf;
     
    data[1] = (data[0] - s1);
    data[0] = (data[0] + s1);
  }
}
/*
   Perform dct type 2
 */
static void
dct2 (float *data,
      int L,
      HANDLE_SBR_QMF_FILTER_BANK qmfBank
      )
{
  int i, M, N;
  float s1, s2, s3, s4;
  float temp[16];
  const float sqrtHalf  = 0.70710678118655f;
  
  
  M = L / 2;
  
  N = L / 4;
   
  if (L > 2) {
      /* pointers for data[i],
                                  data[i + M],
                                  data[M-1 - i],
                                  data[L-1 - i] */
    
    for(i = 0; i < N; i++) {
      
      s1 = data[i];
      s2 = data[i + M];
      s3 = data[M-1 - i];
      s4 = data[L-1 - i];
       
      data[i]       = (s1 + s4);
      data[i + M]   = (s1 - s4);
      data[M-1 - i] = (s3 + s2);
      data[L-1 - i] = (s3 - s2);
    }
    
    dct2(data,     M, qmfBank);
    
    dct4(data + M, M, qmfBank);
      /* pointers for data[2*i + M],
                                  temp[i]        */
    
    for(i = 0; i < N; i++) {
      
      temp[i] = data[2*i + M];
    }
      /* pointers for data[L - 2*i],
                                  data[M - i]    */
    
    for(i = 1; i < M; i++) {
      
      data[L - 2*i] = data[M - i];
    }
      /* pointers for data[4*i - 1],
                                  data[M-1 + 2*i] */
    
    for(i = 1; i < N; i++) {
      
      data[4*i - 1] = data[M-1 + 2*i];
    }
      /* pointers for data[4*i + 1],
                                  temp[i]        */
    
    for(i = 0; i < N; i++) {
      
      data[4*i + 1] = temp[i];
    }
  }
  else { /* 2-point transform */
    
    s1 = data[0];
     
    data[0] =   (s1 + data[1]);
      
    data[1] =  ((s1 - data[1]) * sqrtHalf);
  }
}
/*
 *
 * \brief Perform real-valued forward modulation of the time domain
 *        data of timeIn and stores the real part of the subband
 *        samples in rSubband
 *
 */
static void
sbrForwardModulationLP (const float *timeIn,
                        float *rSubband,
                        HANDLE_SBR_QMF_FILTER_BANK qmfBank
                        )
{
  int i, L, M;
  
  
  L = NO_ANALYSIS_CHANNELS;
  
  M = L/2;
    /* pointers for rSubband[] */
   
  rSubband[0] = timeIn[3 * M];
    /* pointers for timeIn[3 * M - i],
                                timeIn[3 * M + i]  */
  
  for (i = 1; i < M; i++) {
     
    rSubband[i] = timeIn[3 * M - i] + timeIn[3 * M + i];
  }
  
  for (i = M; i < L; i++) {
     
    rSubband[i] = timeIn[3 * M - i] - timeIn[i - M];
  }
  
  dct3 (rSubband, L, qmfBank);
  
}
#ifndef LP_SBR_ONLY
/*
 *
 * \brief Cosine modulation of the time domain data of a subband. Performed in-place
 *
 */
static void
cosMod (float *subband,
        HANDLE_SBR_QMF_FILTER_BANK qmfBank
        )
{
  int i, M;
  float wim, wre;
  float re1, im1, re2, im2;
  float accu1,accu2;
  
   
  M = qmfBank->no_channels >> 1;
    /* pointers for subband[2 * i],
                                subband[2 * M - 2 * i],
                                qmfBank->sin_twiddle[i],
                                qmfBank->cos_twiddle[i],
                                qmfBank->sin_twiddle[M - 1 - i],
                                qmfBank->cos_twiddle[M - 1 - i]  */
   /* M/2 */ 
  for (i = 0; i < M /2; i++) {
    
    re1 = subband[2 * i];
    im2 = subband[2 * i + 1];
    re2 = subband[2 * M - 2 - 2 * i];
    im1 = subband[2 * M - 1 - 2 * i];
    
    wim = qmfBank->sin_twiddle[i];
    wre = qmfBank->cos_twiddle[i];
     
    accu1 =  im1 * wim + re1 * wre;
     
    accu2 =  im1 * wre - re1 * wim;
    
    subband[2 * i] = accu1;
    subband[2 * i + 1] = accu2;
    
    wim = qmfBank->sin_twiddle[M - 1 - i];
    wre = qmfBank->cos_twiddle[M - 1 - i];
     
    accu1 = im2 * wim + re2 * wre;
     
    accu2 = im2 * wre - re2 * wim;
    
    subband[2 * M - 2 - 2 * i] = accu1;
    subband[2 * M - 1 - 2 * i] = accu2;
  }
  
  sbrfft(subband, M);
    /* pointers for  subband[2 * i],
                                 subband[2 * M - 2 * i],
                                 qmfBank->alt_sin_twiddle[i],
                                 qmfBank->sin_twiddle[M - 1 - i] */
  
  wim = qmfBank->alt_sin_twiddle[0];
  wre = qmfBank->alt_sin_twiddle[M];
  
  for (i = 0; i < M / 2; i++) {
    
    re1 = subband[2 * i];
    im1 = subband[2 * i + 1];
    re2 = subband[2 * M - 2 - 2 * i];
    im2 = subband[2 * M - 1 - 2 * i];
     
    accu1=re1 * wre + im1 * wim;
     
    accu2=re1 * wim - im1 * wre;
    
    subband[2 * i] = accu1;
    subband[2 * M - 1 - 2 * i]= accu2;
    
    wim = qmfBank->alt_sin_twiddle[i + 1];
    wre = qmfBank->alt_sin_twiddle[M - 1 - i];
     
    accu1=re2 * wim + im2 * wre;
     
    accu2=re2 * wre - im2 * wim;
    
    subband[2 * M - 2 - 2 * i] = accu1;
    subband[2 * i + 1] = accu2;
  }
  
}
/*
 *
 * \brief Sine modulation of the time domain data of a subband. Performed in-place
 *
 */
static void
sinMod (float *subband,
        HANDLE_SBR_QMF_FILTER_BANK qmfBank
        )
{
  int i, M;
  float wre, wim;
  float re1, im1, re2, im2;
  float accu1,accu2;
  
   
  M = qmfBank->no_channels >> 1;
    /* pointers for subband[2 * i],
                                subband[2 * M - 2 * i],
                                qmfBank->sin_twiddle[i],
                                qmfBank->cos_twiddle[i],
                                qmfBank->sin_twiddle[M - 1 - i],
                                qmfBank->cos_twiddle[M - 1 - i]  */
   /* M/2 */ 
  for (i = 0; i < M / 2; i++) {
    
    re1 = subband[2 * i];
    im2 = subband[2 * i + 1];
    re2 = subband[2 * M - 2 - 2 * i];
    im1 = subband[2 * M - 1 - 2 * i];
    
    wre = qmfBank->sin_twiddle[i];
    wim = qmfBank->cos_twiddle[i];
     
    accu1 = im1 * wim + re1 * wre;
     
    accu2 = im1 * wre - re1 * wim;
    
    subband[2 * i + 1] = accu1;
    subband[2 * i] = accu2;
    
    wre = qmfBank->sin_twiddle[M - 1 - i];
    wim = qmfBank->cos_twiddle[M - 1 - i];
     
    accu1 = im2 * wim + re2 * wre;
     
    accu2 = im2 * wre - re2 * wim;
    
    subband[2 * M - 1 - 2 * i] = accu1;
    subband[2 * M - 2 - 2 * i] = accu2;
  }
  
  sbrfft(subband, M);
    /* pointers for  subband[2 * i],
                                 subband[2 * M - 2 * i],
                                 qmfBank->alt_sin_twiddle[i],
                                 qmfBank->sin_twiddle[M - 1 - i] */
  
  wim = qmfBank->alt_sin_twiddle[0];
  wre = qmfBank->alt_sin_twiddle[M];
  for (i = 0; i < M / 2; i++) {
    
    re1 = subband[2 * i];
    im1 = subband[2 * i + 1];
    re2 = subband[2 * M - 2 - 2 * i];
    im2 = subband[2 * M - 1 - 2 * i];
     
    accu1 = -(re1 * wre + im1 * wim);
     
    accu2 = -(re1 * wim - im1 * wre);
    
    subband[2 * M - 1 - 2 * i] = accu1;
    subband[2 * i] = accu2;
    
    wim = qmfBank->alt_sin_twiddle[i + 1];
    wre = qmfBank->alt_sin_twiddle[M - 1 - i];
     
    accu1 = -(re2 * wim + im2 * wre);
     
    accu2 = -(re2 * wre - im2 * wim);
    
    subband[2 * i + 1] = accu1;
    subband[2 * M - 2 - 2 * i] = accu2;
  }
  
}
/*
 *
 * \brief Perform complex-valued forward modulation of the time domain
 *        data of timeIn and stores the real part of the subband
 *        samples in rSubband, and the imaginary part in iSubband
 *
 */
static void
sbrForwardModulation (const float *timeIn,
                      float *rSubband,
                      float *iSubband,
                      HANDLE_SBR_QMF_FILTER_BANK anaQmf
                      )
{
  int i, offset;
  float real, imag;
  
  
  offset = 2 * NO_ANALYSIS_CHANNELS;
    /* pointer for timeIn[offset - 1 - i] */
  
  for (i = 0; i < NO_ANALYSIS_CHANNELS; i++) {
     
    rSubband[i] = timeIn[i] - timeIn[offset - 1 - i];
    iSubband[i] = timeIn[i] + timeIn[offset - 1 - i];
  }
  
  cosMod (rSubband, anaQmf);
  
  sinMod (iSubband, anaQmf);
    /* pointers for rSubband[i],
                                iSubband[i],
                                anaQmf->t_cos[i],
                                anaQmf->t_sin[i]  */
   
  for (i = 0; i < anaQmf->lsb; i++) {
    
    real = rSubband[i];
    imag = iSubband[i];
      
    rSubband[i] = real * anaQmf->t_cos[i] + imag * anaQmf->t_sin[i];
      
    iSubband[i] = imag * anaQmf->t_cos[i] - real * anaQmf->t_sin[i];
  }
  
}
#endif /* #ifndef LP_SBR_ONLY */
/*
 *
 * \brief Perform complex-valued subband filtering of the time domain
 *        data of timeIn and stores the real part of the subband
 *        samples in rAnalysis, and the imaginary part in iAnalysis
 *
 */
void
cplxAnalysisQmfFiltering (
                          const float *timeIn,
                          float **qmfReal,
#ifndef LP_SBR_ONLY
                          float **qmfImag,
#endif
                          HANDLE_SBR_QMF_FILTER_BANK anaQmf,
                          int   bUseLP
                          )
{
  int i, k;
  float analysisBuffer[NO_ANALYSIS_CHANNELS*2];
  const float *ptr_pf;
  const float *ptr_states;
  const float *ptr_timeIn;
  int p;
  float accu;
  
  
  ptr_timeIn = timeIn;
   
  for (i = 0; i < anaQmf->no_col; i++) {
        STORE(2*NO_ANALYSIS_CHANNELS);
    memset (analysisBuffer, 0, 2*NO_ANALYSIS_CHANNELS * sizeof (float));
     
    ptr_pf     = anaQmf->p_filter;
    ptr_states = anaQmf->FilterStatesAna;
        STORE(NO_ANALYSIS_CHANNELS);
    memcpy(anaQmf->FilterStatesAna+(QMF_FILTER_STATE_ANA_SIZE-NO_ANALYSIS_CHANNELS),ptr_timeIn,NO_ANALYSIS_CHANNELS*sizeof(float));
                                                                       
                                                                        
    for (k = 0; k < NO_ANALYSIS_CHANNELS; k++) {
      ptr_pf += NO_POLY * (QMF_ANA_FILTER_STRIDE - 1);                 
      accu=0;                                                          
      for (p = 0; p < NO_POLY; p++) {
        accu +=  *ptr_pf++ * ptr_states[2*NO_ANALYSIS_CHANNELS * p];   
      }
      analysisBuffer[2*NO_ANALYSIS_CHANNELS - 1 - k] = accu;           
      ptr_states++;
    }
    accu=0;                                                              
    for (p = 0; p < NO_POLY; p++) {
      accu +=  *ptr_pf++ * anaQmf->FilterStatesAna[2*NO_ANALYSIS_CHANNELS * p + 2*NO_ANALYSIS_CHANNELS-1]; 
    }
    analysisBuffer[0] = accu;                                          
    ptr_pf -= NO_POLY * 2;                                             
                                                                        
    for (k = 0; k < NO_ANALYSIS_CHANNELS-1; k++){
      ptr_pf -= NO_POLY * (QMF_ANA_FILTER_STRIDE - 1);                 
      accu=0;                                                          
      for (p = 0; p < NO_POLY; p++) {
        accu +=  *--ptr_pf * ptr_states[2*NO_ANALYSIS_CHANNELS * p];   
      }
      analysisBuffer[NO_ANALYSIS_CHANNELS - 1 - k] = accu;             
      ptr_states++;
    }
                                                                       
    
    if (bUseLP) {
      
      sbrForwardModulationLP (analysisBuffer,
                              qmfReal[i],
                              anaQmf);
    }
#ifndef LP_SBR_ONLY
    else {
      
      sbrForwardModulation (analysisBuffer,
                            qmfReal[i],
                            qmfImag[i],
                            anaQmf);
    }
#endif
    /*
      Shift filter states
      Should be realized with modulo adressing on a DSP instead of a true buffer shift
    */
    /*     STORE(NO_ANALYSIS_CHANNELS); */
    memmove(anaQmf->FilterStatesAna,anaQmf->FilterStatesAna+NO_ANALYSIS_CHANNELS,(QMF_FILTER_STATE_ANA_SIZE-NO_ANALYSIS_CHANNELS)*sizeof(float));
    ptr_timeIn += NO_ANALYSIS_CHANNELS;
  }
  
}
static void
inverseModulationLP (float *qmfReal,
                     float *qmfReal2,
                     HANDLE_SBR_QMF_FILTER_BANK synQmf
                     )
{
  int i, L, M;
  float timeOut[2*NO_ACTUAL_SYNTHESIS_CHANNELS];
  
  
  L = synQmf->no_channels;
   
  M = L / 2;
    /* pointers for timeOut[],
                                qmfReal[]  */
   
  for (i = 0; i < synQmf->usb; i++) {
    
    timeOut[i + M] = qmfReal[i];
  }
   
  for (i = synQmf->usb; i < L; i++) {
    
    timeOut[i + M] = 0;
  }
  
  dct2(timeOut+M, L, synQmf);
  
  timeOut[3 * M] = 0;
    /* pointers for timeOut[i]
                                timeOut[-i] */
  
  for (i = 1; i < M; i++) {
     
    timeOut[i + 3 * M] = - timeOut[3 * M - i];
  }
    /* pointer for timeOut[i],
                               timeOut[L-i] */
  
  for (i = 0; i < M; i++) {
    
    timeOut[i] = timeOut[L - i];
  }
    /* pointer for timeOut[],
                               qmfReal[]  */
  
  for (i = 0; i < L; i++) {
    
    qmfReal[i] = timeOut[i];
  }
    /* pointer for timeOut[],
                               qmfReal2[]  */
  
  for (i = 0; i < L; i++) {
    
    qmfReal2[i] = timeOut[L+i];
  }
  
}
#ifndef LP_SBR_ONLY
/*
 *
 * \brief Perform complex-valued inverse modulation of the subband
 *        samples stored in rSubband (real part) and iSubband (imaginary
 *        part) and stores the result in timeOut
 *
 */
static void
inverseModulation (float *qmfReal,
                   float *qmfImag,
                   HANDLE_SBR_QMF_FILTER_BANK synQmf
                   )
{
  int i, no_synthesis_channels, M;
  float r1, i1, r2, i2;
  
   
  no_synthesis_channels = synQmf->no_channels;
  
  M = no_synthesis_channels / 2;
    /* pointer for qmfReal[],
                               qmfImag[] */
   
  for (i = synQmf->usb; i < no_synthesis_channels; i++) {
    
    qmfReal[i]=qmfImag[i]=0;
  }
  
  cosMod (qmfReal, synQmf);
  
  sinMod (qmfImag, synQmf);
    /* pointer for qmfReal[],
                               qmfImag[],
                               qmfImag[no_synthesis_channels - 1 - i],
                               qmfReal[no_synthesis_channels - i - 1]   */
  
  for (i = 0; i < M; i++) {
    
    r1 = qmfReal[i];
    i2 = qmfImag[no_synthesis_channels - 1 - i];
    r2 = qmfReal[no_synthesis_channels - i - 1];
    i1 = qmfImag[i];
     
    qmfReal[i] = (r1 - i1);
    qmfImag[no_synthesis_channels - 1 - i] = -(r1 + i1);
    qmfReal[no_synthesis_channels - i - 1] = (r2 - i2);
    qmfImag[i] = -(r2 + i2);
  }
  
}
#endif /* #ifndef LP_SBR_ONLY */
/*
 *
 *
 * \brief Perform complex-valued subband synthesis of the
 *        low band and the high band and store the
 *        time domain data in timeOut
 *
*/
void cplxSynthesisQmfFiltering(float **qmfReal, float **qmfImag, float *timeOut, HANDLE_SBR_QMF_FILTER_BANK synQmf, int   bUseLP, HANDLE_PS_DEC h_ps_dec, int active)
{
  int i, j;
  float *ptr_time_out;
  float *filterStates;
  float accu;
  int p;
  float qmfReal2[NO_ACTUAL_SYNTHESIS_CHANNELS];
  float *imagSlot;
  int no_synthesis_channels;
  int qmf_filter_state_syn_size;
  float qmfRealTmp[NO_ACTUAL_SYNTHESIS_CHANNELS];
  float qmfImagTmp[NO_ACTUAL_SYNTHESIS_CHANNELS];
  int env;
  
  
  env = 0;
  memset(qmfRealTmp,0,NO_ACTUAL_SYNTHESIS_CHANNELS*sizeof(float));
  memset(qmfImagTmp,0,NO_ACTUAL_SYNTHESIS_CHANNELS*sizeof(float));
   
  no_synthesis_channels = synQmf->no_channels;
   
  qmf_filter_state_syn_size = synQmf->qmf_filter_state_size;
   
  filterStates = synQmf->FilterStatesSyn;
  
  ptr_time_out = timeOut;
   
  for (i = 0; i < synQmf->no_col; i++) 
	{
    const float *p_filter = synQmf->p_filter;
     
    if (bUseLP) 
		{      
      imagSlot = qmfReal2;
    }
    else 
		{
      
      imagSlot = *(qmfImag + i);
    }
    
    if(active)
		{        
      if(i == h_ps_dec-> aEnvStartStop[env])
			{         
        InitRotationEnvelope(h_ps_dec,env,synQmf->usb);
        env++;
      }
      
      ApplyPsSlot(h_ps_dec, &qmfReal[i], &qmfImag[i], qmfRealTmp, qmfImagTmp);
    }
    
    if(!bUseLP) 
		{      
      if(no_synthesis_channels == NO_SYNTHESIS_CHANNELS_DOWN_SAMPLED)
			{        
        for (j = 0; j < no_synthesis_channels; j++)
				{
          float temp;          
          temp = qmfReal[i][j];            
          qmfReal[i][j] = synQmf->t_cos[j] * qmfReal[i][j] + synQmf->t_sin[j] * imagSlot[j];            
          imagSlot[j]   = synQmf->t_cos[j] * imagSlot[j]   - synQmf->t_sin[j] * temp;
        }
      }
      for (j = 0; j < synQmf->usb; j++) 
			{         
        qmfReal[i][j] *= -1.0;
        imagSlot[j]   *= -1.0;
      }
    }

    
    if (bUseLP) 
		{      
      inverseModulationLP (qmfReal[i], imagSlot, synQmf);
    }
    else 
		{      
      inverseModulation (qmfReal[i], imagSlot, synQmf);
    }
    
    if (bUseLP) 
		{     
      for (j = 0; j < no_synthesis_channels; j++) 
			{         
        qmfReal[i][j] =  qmfReal[i][j] * 0.0625f;
        imagSlot[j]   =  imagSlot[j]   * 0.0625f;
      }
    }
    else 
		{      
      for (j = 0; j < no_synthesis_channels; j++) 
			{         
        qmfReal[i][j] =  qmfReal[i][j] * 0.03125f;
        imagSlot[j]   =  imagSlot[j]   * 0.03125f;
      }
    }
    
    for (j = 0; j < no_synthesis_channels; j++)
		{
      float newSample;
      
      newSample = imagSlot[no_synthesis_channels -1 - j];
      if(no_synthesis_channels == 32){
        p_filter += NO_POLY;
      }
      
      for (p = 0; p < NO_POLY; p++) 
			{         
        accu = filterStates[p * 2*no_synthesis_channels + j] + (*p_filter++) * newSample;
        filterStates[p * 2*no_synthesis_channels + j] = accu;
      }
    }
    
    for (p = 0; p < NO_POLY; p++) 
		{       
      accu =  filterStates[p * 2*no_synthesis_channels + no_synthesis_channels + (no_synthesis_channels-1) ] +  (*p_filter++) * qmfReal[i][0];
      filterStates[p * 2*no_synthesis_channels + no_synthesis_channels + (no_synthesis_channels-1)] = accu;
    }
    
    ptr_time_out[0] = accu;
    p_filter -= NO_POLY*2;
     
    for (j = 0; j < no_synthesis_channels-1; j++){
      float newSample;
      
      newSample = qmfReal[i][no_synthesis_channels -1 - j];
      if(no_synthesis_channels == 32)
			{
        p_filter -= NO_POLY;
      }
      
      for (p = 0; p < NO_POLY; p++) 
			{         
        accu =  filterStates[p * 2*no_synthesis_channels + no_synthesis_channels + j] + (*--p_filter) * newSample;
        filterStates[p * 2*no_synthesis_channels + no_synthesis_channels + j] = accu;
      }
      
      ptr_time_out[no_synthesis_channels - 1 - j] = accu;
    }
    ptr_time_out += no_synthesis_channels;
    /*
      Shift filter states
      Should be replaces by modulo operation if available
    */
		memmove (filterStates + no_synthesis_channels, filterStates, (qmf_filter_state_syn_size - no_synthesis_channels) * sizeof (float));
    memset (filterStates, 0, no_synthesis_channels * sizeof (float));
    
    if(active)
		{
			memcpy(qmfReal[i],qmfRealTmp,sizeof(float)*no_synthesis_channels);
      memcpy(qmfImag[i],qmfImagTmp,sizeof(float)*no_synthesis_channels);
    }
    
  }
  
}
/*
 *
 * \brief Create QMF filter bank instance
 *
 * \return 0 if succesful
 *
 */
int createCplxAnalysisQmfBank(HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf, int noCols, int lsb, int usb)
{
  memset(h_sbrQmf,0,sizeof(SBR_QMF_FILTER_BANK));
    
  h_sbrQmf->p_filter = sbr_qmf_64_640;
  h_sbrQmf->pDct4Twiddle = dct4TwiddleTable;
  
  h_sbrQmf->cos_twiddle = sbr_cos_twiddle_L32;
  h_sbrQmf->sin_twiddle = sbr_sin_twiddle_L32;
  h_sbrQmf->alt_sin_twiddle = sbr_alt_sin_twiddle_L32;
  h_sbrQmf->t_cos = sbr_t_cos_L32;
  h_sbrQmf->t_sin = sbr_t_sin_L32;
  
  h_sbrQmf->no_channels = NO_ANALYSIS_CHANNELS;
  h_sbrQmf->no_col = noCols;
  h_sbrQmf->lsb = lsb;
  h_sbrQmf->usb = usb;

  return (0);
}
/*
 *
 * \brief Create QMF filter bank instance
 *
 * \return 0 if successful
 *
 */
int createCplxSynthesisQmfBank(HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf, int noCols, int lsb, int usb, int bDownSample)
{
  int L, qmfFilterStateSize;
  
  memset(h_sbrQmf,0,sizeof(SBR_QMF_FILTER_BANK));
  
  if(bDownSample)
	{    
    L = NO_SYNTHESIS_CHANNELS_DOWN_SAMPLED;
    qmfFilterStateSize = QMF_FILTER_STATE_SYN_SIZE_DOWN_SAMPLED;
  }
  else{
    
    L = NO_SYNTHESIS_CHANNELS;
    qmfFilterStateSize = QMF_FILTER_STATE_SYN_SIZE;
  }
  
  h_sbrQmf->p_filter = sbr_qmf_64_640;
  
  h_sbrQmf->no_channels = L;
  h_sbrQmf->qmf_filter_state_size = qmfFilterStateSize;
  h_sbrQmf->no_col = noCols;
  
  h_sbrQmf->lsb = lsb;
  
  if(bDownSample)
	{    
    h_sbrQmf->usb = 32;
  }
  else
	{    
    h_sbrQmf->usb = usb;
  }
  
  h_sbrQmf->pDct4Twiddle = dct4TwiddleTable;
   
  if(L == 32)
	{
    h_sbrQmf->cos_twiddle = sbr_cos_twiddle_L32;
    h_sbrQmf->sin_twiddle = sbr_sin_twiddle_L32;
    h_sbrQmf->alt_sin_twiddle = sbr_alt_sin_twiddle_L32;
    h_sbrQmf->t_cos = sbr_cos_twiddle_ds_L32;
    h_sbrQmf->t_sin = sbr_sin_twiddle_ds_L32;
  }
  else
	{
    h_sbrQmf->cos_twiddle = sbr_cos_twiddle_L64;
    h_sbrQmf->sin_twiddle = sbr_sin_twiddle_L64;
    h_sbrQmf->alt_sin_twiddle = sbr_alt_sin_twiddle_L64;
  }

  return 0;
}
