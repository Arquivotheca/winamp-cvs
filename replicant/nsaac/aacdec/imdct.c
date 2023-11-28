/*
  inverse transformation
*/
#include <math.h>
#include "aac_rom.h"
#include "imdct.h"
#include "math/FloatFR.h"
#include "math/cfftn.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function performs the pre modulation in the inverse
  transformation.
*/
static void 
preModulation(float *x,                /*!< pointer to spectrum */
              int m,                   /*!< number of lines in spectrum */
              const float *sineWindow) /*!< pointer to modulation coefficients */
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  
    /* pointers for x[2*i], x[m-2-2*i],
                                sineWindow[i*2],
                                sineWindow[m-1-2*i] */
   
  for (i = 0; i < m/4; i++)
  {
    
    re1 = x[2*i];
    im2 = x[2*i+1];
    re2 = x[m-2-2*i];
    im1 = x[m-1-2*i];
    
    wim = sineWindow[i*2];
    wre = sineWindow[m-1-2*i];
      
    x[2*i] = 0.5f * (im1*wim + re1*wre);
      
    x[2*i+1] = 0.5f * (im1*wre - re1*wim);
    
    wim = sineWindow[m-2-2*i];
    wre = sineWindow[2*i+1];
         
    x[m-2-2*i] = 0.5f * (im2*wim + re2*wre);
      
    x[m-1-2*i] = 0.5f * (im2*wre - re2*wim);
  }
  
}
/*
  The function performs the post modulation in the inverse
  transformation.
*/
static void 
postModulation(float *x,              /*!< pointer to spectum */
               int m,                 /*!< number of lines in spectrum */
               const float *trigData, /*!< pointer to trigData */
               int step,              /*!< steps */
               int trigDataSize)      /*!< size of trigData */
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  const float *sinPtr = trigData;
  const float *cosPtr = trigData+trigDataSize;
  
   /* previous initialization */
  
  wim = *sinPtr;
  wre = *cosPtr;
    /* pointers for x[2*i], x[m-2-2*i] */
   
  for (i = 0; i < m/4; i++)
  {
    
    re1=x[2*i];
    im1=x[2*i+1];
    re2=x[m-2-2*i];
    im2=x[m-1-2*i];
      
    x[2*i] = re1*wre + im1*wim;
      
    x[m-1-2*i] = re1*wim - im1*wre;
    
    sinPtr+=step;
    cosPtr-=step;
    
    wim=*sinPtr;
    wre=*cosPtr;
         
    x[m-2-2*i] = re2*wim + im2* wre;
      
    x[2*i+1] = re2*wre - im2* wim;
  }
  
}
/*
  The calculation of the imdct is divided into three steps, the pre modulation,
  the complex fft and the post modulation. The imdct is calculated in-place.
*/
void CLongBlock_InverseTransform(float *pData)      /*!< pointer to input/output data */
{
  int trigDataSize;
  
  
  trigDataSize = 512;
  /* calculate imdct */
  
  preModulation(pData,FRAME_SIZE,OnlyLongWindowSine);
  
  CFFTN(pData, trigDataSize, -1);
  
  postModulation(pData,FRAME_SIZE,trigData,1,trigDataSize);
  
}
/*!
  The calculation of the imdct is divided into three steps, the pre modulation,
  the complex fft and the post modulation. The imdct is calculated in-place.
*/
void CShortBlock_InverseTransform(float *pData)       /*!< pointer to input/output data */
{
  int trigDataSize;
  
  trigDataSize = 512;
  /* calculate imdct */
  
  preModulation(pData,FRAME_SIZE/8,OnlyShortWindowSine);
  
  CFFTN(pData, trigDataSize/8, -1);
  
  postModulation(pData,FRAME_SIZE/8,trigData,8,trigDataSize);
  
}
