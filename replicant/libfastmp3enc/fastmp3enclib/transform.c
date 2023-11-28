/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: transform.c,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                  *
*                                                                                              *
************************************************************************************************/

#include <math.h>

#include "mconfig.h"
#include "polyana.h"
#include "transform.h"
#include "cmdct.h"
#include "cfftl3.h"
#include "mathlib.h"
#include "mp3alloc.h"

#define   MDCT_SIZE_LONG            18
#define   MDCT_SIZE_SHORT            6  
#define   ALIAS_COEFF                8
#define   NORM_FAC18 (1.0f/9.0f)
#define   NORM_FAC06 (1.0f/3.0f)

ALIGN_16_BYTE static float c_signal[ALIAS_COEFF] = {
    0.857492926f, 0.881741997f, 0.949628649f, 0.983314592f, 
    0.995517816f, 0.999160558f, 0.999899195f, 0.999993155f
};


ALIGN_16_BYTE static float c_alias[ALIAS_COEFF] = {
   -0.514495755f,-0.471731969f,-0.313377454f,-0.181913200f,
   -0.094574193f,-0.040965583f,-0.014198569f,-0.003699975f
};



ALIGN_16_BYTE static const float mdct_window[4][MDCT_SIZE_LONG*2]={
    {
    /*
        long 
    */
        0.043619387f,0.130526192f,0.216439614f,0.300705799f,
        0.382683432f,0.461748613f,0.537299608f,0.608761429f,
        0.675590208f,0.737277337f,0.793353340f,0.843391446f,
        0.887010833f,0.923879532f,0.953716951f,0.976296007f,
        0.991444861f,0.999048222f,0.999048222f,0.991444861f,
        0.976296007f,0.953716951f,0.923879533f,0.887010833f,
        0.843391446f,0.793353340f,0.737277337f,0.675590208f,
        0.608761429f,0.537299608f,0.461748613f,0.382683432f,
        0.300705800f,0.216439614f,0.130526192f,0.043619387f
    },
    {
    /*
        start 
    */
        0.043619387f,0.130526192f,0.216439614f,0.300705799f,
        0.382683432f,0.461748613f,0.537299608f,0.608761429f,
        0.675590208f,0.737277337f,0.793353340f,0.843391446f,
        0.887010833f,0.923879532f,0.953716951f,0.976296007f,
        0.991444861f,0.999048222f,1.000000000f,1.000000000f,
        1.000000000f,1.000000000f,1.000000000f,1.000000000f,
        0.991444861f,0.923879533f,0.793353340f,0.608761429f,
        0.382683432f,0.130526192f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f
    },
    {
    /*
        short 
    */
        0.130526192f,0.382683432f,0.608761429f,0.793353340f,
        0.923879532f,0.991444861f,0.991444861f,0.923879533f,
        0.793353340f,0.608761429f,0.382683432f,0.130526192f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.000000000f,0.000000000f
    },

    {
    /*
        stop 
    */
        0.000000000f,0.000000000f,0.000000000f,0.000000000f,
        0.000000000f,0.000000000f,0.130526192f,0.382683432f,
        0.608761429f,0.793353340f,0.923879532f,0.991444861f,
        1.000000000f,1.000000000f,1.000000000f,1.000000000f,
        1.000000000f,1.000000000f,0.999048222f,0.991444861f,
        0.976296007f,0.953716951f,0.923879533f,0.887010833f,
        0.843391446f,0.793353340f,0.737277337f,0.675590208f,
        0.608761429f,0.537299608f,0.461748613f,0.382683432f,
        0.300705800f,0.216439614f,0.130526192f,0.043619387f
    }
};

ALIGN_16_BYTE static const float trigDataLong[MDCT_SIZE_LONG/2+1]=
{
  0.0000000000f*NORM_FAC18,
  0.1736481777f*NORM_FAC18,
  0.3420201433f*NORM_FAC18,
  0.5000000000f*NORM_FAC18,
  0.6427876097f*NORM_FAC18,
  0.7660444431f*NORM_FAC18,
  0.8660254038f*NORM_FAC18,
  0.9396926208f*NORM_FAC18,
  0.9848077530f*NORM_FAC18,
  1.0000000000f*NORM_FAC18
};

ALIGN_16_BYTE static const float trigDataShort[MDCT_SIZE_SHORT/2+1]=
{
  0.0000000000f*NORM_FAC06,
  0.5000000000f*NORM_FAC06,
  0.8660254038f*NORM_FAC06,
  1.0000000000f*NORM_FAC06,
};


static void preModulationDCT(float *x,int m,const float *sineWindow)
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  
  
  for(i=0;i<m/4;i++){
    re1 = x[2*i];
    im2 = x[2*i+1];
    re2 = x[m-2-2*i];
    im1 = x[m-1-2*i];

    wim = sineWindow[i*2];
    wre = sineWindow[m-1-2*i];

    x[2*i]   = im1*wim + re1* wre;
    x[2*i+1] = im1*wre - re1* wim;

    wim = sineWindow[m-2-2*i];
    wre = sineWindow[2*i+1];

    x[m-2-2*i] = im2*wim + re2*wre;
    x[m-1-2*i] = im2*wre - re2*wim;
  }
  
  re1 = x[2*i];
  im1 = x[2*i+1];
  
  wim = sineWindow[i*2];
  wre = sineWindow[i*2+1];
  
  x[2*i]   = im1*wim + re1* wre;
  x[2*i+1] = im1*wre - re1* wim;


}

static void preModulationDST(float *x,int m,const float *sineWindow)
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  
  
  for(i=0;i<m/4;i++){
    re1 = x[2*i];
    im2 = x[2*i+1];
    re2 = x[m-2-2*i];
    im1 = x[m-1-2*i];

    wre = sineWindow[i*2];
    wim = sineWindow[m-1-2*i];

    x[2*i+1]= im1*wim + re1* wre;  
    x[2*i]  = im1*wre - re1* wim;
  
  
    wre = sineWindow[m-2-2*i];
    wim = sineWindow[2*i+1];

    x[m-1-2*i] = im2*wim + re2*wre;  
    x[m-2-2*i] = im2*wre - re2*wim;
 
  }
  re1 = x[2*i];
  im1 = x[2*i+1];
  wre = sineWindow[i*2];
  wim = sineWindow[i*2+1];
  x[2*i+1]= im1*wim + re1* wre;  
  x[2*i]  = im1*wre - re1* wim;
  



}

static void postModulationDCT(float *x,int m, const float *trigData)
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  const float *sinPtr = trigData;
  const float *cosPtr = trigData+m/2;

  wim = *sinPtr;
  wre = *cosPtr;
	
  for(i=0;i<m/4;i++){
    re1=x[2*i];
    im1=x[2*i+1];
    re2=x[m-2-2*i];
    im2=x[m-1-2*i];
    
    x[2*i]=re1*wre + im1*wim;
    x[m-1-2*i]=re1*wim - im1*wre;
  
    sinPtr++;
    cosPtr--;
    wim=*sinPtr;
    wre=*cosPtr;
  
    x[m-2-2*i] = re2*wim + im2* wre;
    x[2*i+1]   = re2*wre - im2* wim;
  }
  re1=x[2*i];
  im1=x[2*i+1];
  x[2*i]=re1*wre + im1*wim;
  x[2*i+1]=re1*wim - im1*wre;
  
  
}


static void postModulationDST(float *x,int m, const float *trigData)
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  const float *sinPtr = trigData;
  const float *cosPtr = trigData+m/2;

  wim = *sinPtr;
  wre = *cosPtr;
	
  for(i=0;i<m/4;i++){
    re1=x[2*i];
    im1=x[2*i+1];
    re2=x[m-2-2*i];
    im2=x[m-1-2*i];

    x[m-1-2*i]=-(re1*wre + im1*wim);   
    x[2*i]=-(re1*wim - im1*wre);
 
    
    sinPtr++;
    cosPtr--;
    wim=*sinPtr;
    wre=*cosPtr;
    
    x[2*i+1] = -(re2*wim + im2* wre);   
    x[m-2-2*i]   = -(re2*wre - im2* wim);
    
  }
  re1=x[2*i];
  im1=x[2*i+1];
  x[2*i+1]=-(re1*wre + im1*wim);   
  x[2*i]=-(re1*wim - im1*wre);
 
}




static void dct_iv_18(float *inData)
{
  
  preModulationDCT(inData,MDCT_SIZE_LONG,mdct_window[LONG_WINDOW]);
  cfft_9(inData);
  postModulationDCT(inData,MDCT_SIZE_LONG,trigDataLong);

}

static void dst_iv_18(float *inData)
{
  
  preModulationDST(inData,MDCT_SIZE_LONG,mdct_window[LONG_WINDOW]);
  cfft_9(inData);
  postModulationDST(inData,MDCT_SIZE_LONG,trigDataLong);

}

static void dct_iv_6(float *inData)
{
  
  preModulationDCT(inData,MDCT_SIZE_SHORT,mdct_window[SHORT_WINDOW]);
  cfft_3(inData);
  postModulationDCT(inData,MDCT_SIZE_SHORT,trigDataShort);

}

static void dst_iv_6(float *inData)
{
  
  preModulationDST(inData,MDCT_SIZE_SHORT,mdct_window[SHORT_WINDOW]);
  cfft_3(inData);
  postModulationDST(inData,MDCT_SIZE_SHORT,trigDataShort);

}


static void cmdct_long_fast(float *inData,float *outData,int windowSequence)
{
  float *dctIn=outData;
  float *dstIn=outData+FRAME_LEN_LONG;
  float ws1,ws2;
  int i;

  for(i=0;i<MDCT_SIZE_LONG/2;i++){  
      ws1 = inData[i]*mdct_window[windowSequence][i];
      ws2 = inData[MDCT_SIZE_LONG-i-1]*mdct_window[windowSequence][MDCT_SIZE_LONG-i-1];
    
      dctIn[MDCT_SIZE_LONG/2+i] = ws1-ws2;
      dstIn[MDCT_SIZE_LONG/2+i] = -(ws1+ws2);

      ws1 = inData[MDCT_SIZE_LONG+i]*mdct_window[windowSequence][MDCT_SIZE_LONG+i];
      ws2 = inData[MDCT_SIZE_LONG*2-i-1]*mdct_window[windowSequence][MDCT_SIZE_LONG*2-i-1];
    
      dctIn[MDCT_SIZE_LONG/2-i-1] = -(ws1+ws2);
      dstIn[MDCT_SIZE_LONG/2-i-1] = -(ws1-ws2);
  }
  dct_iv_18(dctIn);
  dst_iv_18(dstIn);
}


static void mdct_long_fast(float *inData,float *outData,int windowSequence)
{
  float *dctIn=outData;
  float ws1,ws2;
  int i;

  for(i=0;i<MDCT_SIZE_LONG/2;i++){  
      ws1 = inData[i]*mdct_window[windowSequence][i];
      ws2 = inData[MDCT_SIZE_LONG-i-1]*mdct_window[windowSequence][MDCT_SIZE_LONG-i-1];
    
      dctIn[MDCT_SIZE_LONG/2+i] = ws1-ws2;

      ws1 = inData[MDCT_SIZE_LONG+i]*mdct_window[windowSequence][MDCT_SIZE_LONG+i];
      ws2 = inData[MDCT_SIZE_LONG*2-i-1]*mdct_window[windowSequence][MDCT_SIZE_LONG*2-i-1];
    
      dctIn[MDCT_SIZE_LONG/2-i-1] = -(ws1+ws2);
  }
  dct_iv_18(dctIn);
}



#ifdef USE_COMPLEX_MDCT

static void cmdct_long(float *inData,float *outData,int windowSequence)
{
  int i;
  float cmplxOut[MDCT_SIZE_LONG*2];
 
  
  for(i=0;i<MDCT_SIZE_LONG*2;i++){
    cmplxOut[i]=inData[i]*mdct_window[windowSequence][i]*NORM_FAC18;
  }
    
  
  
  
  
  mp3CMDCTTransform(cmplxOut,MDCT_SIZE_LONG*2);
  
  for(i=0;i<MDCT_SIZE_LONG;i++){
    outData[i] = cmplxOut[2*i];                     /* real part */
    outData[i+FRAME_LEN_LONG] = cmplxOut[2*i+1];    /* imag part */
  }


}

static void cmdct_short(float *inData,float *outData)
{

  int i;
  float cmplxOut[MDCT_SIZE_SHORT*2];

  for(i=0;i<MDCT_SIZE_SHORT*2;i++){
    cmplxOut[i]=inData[i]*mdct_window[SHORT_WINDOW][i]*NORM_FAC06;
  }
    
  mp3CMDCTTransform(cmplxOut,MDCT_SIZE_SHORT*2);
  
  for(i=0;i<MDCT_SIZE_SHORT;i++){
    outData[i] = cmplxOut[2*i];                      /* real part */
    outData[i+FRAME_LEN_SHORT] = cmplxOut[2*i+1];    /* imag part */
  }

}

#endif


static void cmdct_short_fast(float *inData,float *outData)
{
  float *dctIn=outData;
  float *dstIn=outData+FRAME_LEN_SHORT;
  float ws1,ws2;
  int i;

  for(i=0;i<MDCT_SIZE_SHORT/2;i++){  
      ws1 = inData[i]*mdct_window[SHORT_WINDOW][i];
      ws2 = inData[MDCT_SIZE_SHORT-i-1]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT-i-1];
    
      dctIn[MDCT_SIZE_SHORT/2+i] = ws1-ws2;
      dstIn[MDCT_SIZE_SHORT/2+i] = -(ws1+ws2);

      ws1 = inData[MDCT_SIZE_SHORT+i]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT+i];
      ws2 = inData[MDCT_SIZE_SHORT*2-i-1]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT*2-i-1];
    
      dctIn[MDCT_SIZE_SHORT/2-i-1] = -(ws1+ws2);
      dstIn[MDCT_SIZE_SHORT/2-i-1] = -(ws1-ws2);
  }
  dct_iv_6(dctIn);
  dst_iv_6(dstIn);
}

static void mdct_short_fast(float *inData,float *outData)
{
  float *dctIn=outData;
  float ws1,ws2;
  int i;

  for(i=0;i<MDCT_SIZE_SHORT/2;i++){  
      ws1 = inData[i]*mdct_window[SHORT_WINDOW][i];
      ws2 = inData[MDCT_SIZE_SHORT-i-1]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT-i-1];
    
      dctIn[MDCT_SIZE_SHORT/2+i] = ws1-ws2;

      ws1 = inData[MDCT_SIZE_SHORT+i]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT+i];
      ws2 = inData[MDCT_SIZE_SHORT*2-i-1]*mdct_window[SHORT_WINDOW][MDCT_SIZE_SHORT*2-i-1];
    
      dctIn[MDCT_SIZE_SHORT/2-i-1] = -(ws1+ws2);
  }
  dct_iv_6(dctIn);
}





int InitTransform(TRANSFORM_BUFFER *transformBuffer)
{
  int i;
  for(i = 0; i < MDCT_BUF_SIZE; i++)
    transformBuffer->paMdctBuffer[i] = 0.0f;
    
  return(0);

}

int ShiftTransformBuffer(TRANSFORM_BUFFER *transformBuffer)
{

  int i;
  /*
  copy second half of mdct input buffer (paMdctBuffer) to first half.
  if this is done here, paMdctBuffer can be used for intensity mode
  since buffer contains then all 576*2 used polyphase
  samples after! MdctAnalyse
  */
  
  for(i = 0; i < FRAME_LEN_LONG; i += MDCT_SIZE_LONG){
    copyFLOAT(transformBuffer->paMdctBuffer+i*2+MDCT_SIZE_LONG,
      transformBuffer->paMdctBuffer+i*2,
      MDCT_SIZE_LONG);
    
    
  }

  return(0);
}



int Transform_LONG(TRANSFORM_BUFFER *transformBuffer,
              float *cmplxOut,
              int windowSequence)
{
  
  int i;
  
  

  switch(windowSequence){
  case LONG_WINDOW:
  case START_WINDOW:
  case STOP_WINDOW:
    for(i = 0; i < FRAME_LEN_LONG; i += MDCT_SIZE_LONG){

#ifdef OLD_TRANSFORM
      cmdct_long(transformBuffer->paMdctBuffer+i*2, cmplxOut+i, windowSequence);
#else
      cmdct_long_fast(transformBuffer->paMdctBuffer+i*2,cmplxOut+i, windowSequence);
#endif
    }
    break;
  }

  return(0);
}

int MdctTransform_LONG(TRANSFORM_BUFFER *transformBuffer,
              float *cmplxOut,
              int windowSequence)
{
  
  int i;
  
  
  
  switch(windowSequence){
  case LONG_WINDOW:
  case START_WINDOW:
  case STOP_WINDOW:
    for(i = 0; i < FRAME_LEN_LONG; i += MDCT_SIZE_LONG){

      mdct_long_fast(transformBuffer->paMdctBuffer+i*2,cmplxOut+i, windowSequence);
    }
    break;
  }

  return(0);
}



int Transform_SHORT(TRANSFORM_BUFFER *transformBuffer,
              float *cmplxOut,
              int subWindow)
{

 int i;
 
 for(i = 0; i < FRAME_LEN_SHORT; i += MDCT_SIZE_SHORT){
#ifdef OLD_TRANSFORM
   cmdct_short(transformBuffer->paMdctBuffer+MDCT_SIZE_SHORT*(subWindow+1)+i*2*TRANS_FAC, 
              cmplxOut+i);
#else
  cmdct_short_fast(transformBuffer->paMdctBuffer+MDCT_SIZE_SHORT*(subWindow+1)+i*2*TRANS_FAC, 
                   cmplxOut+i);
#endif
 }
 
 return(0);
}

int MdctTransform_SHORT(TRANSFORM_BUFFER *transformBuffer,
              float *cmplxOut,
              int subWindow)
{

 int i;
 
 for(i = 0; i < FRAME_LEN_SHORT; i += MDCT_SIZE_SHORT){
   mdct_short_fast(transformBuffer->paMdctBuffer+MDCT_SIZE_SHORT*(subWindow+1)+i*2*TRANS_FAC, 
                   cmplxOut+i);
 }
 
 return(0);
}



  

/****************************************************************************\
 *
 * butterfly_vector(float *src1, float *src2, int inc_src1, 
 * int inc_src2, FLOAT c_fwd, FLOAT c_cross, int size)
 * computes a so called butterfly for "size" vector elements
 *
\****************************************************************************/
static void butterfly_vector(float *src1, float *src2, int inc_src1,
                             int inc_src2, float c_fwd, float c_cross, int size) {
  int   i;
  float buf;
  
  for(i = 0; i < size; i++) {
    buf   = *src1;
    *src1 = (float) (  c_fwd * buf - c_cross * *src2);
    *src2 = (float) (c_cross * buf +   c_fwd * *src2);
    src1 += inc_src1;
    src2 += inc_src2;
  }
}



int AliasReduction(float *mdctData)
{
  int i,j,k;

  j = MDCT_SIZE_LONG;
  k = j - 1;
  for(i = 0; i < ALIAS_COEFF; i++){
      butterfly_vector(&(mdctData[j]),
                       &(mdctData[k]),
                       MDCT_SIZE_LONG,
                       MDCT_SIZE_LONG,
                       c_signal[i],
                       c_alias[i],
                       POLY_PHASE_BANDS-1);
      j++;
      k--;
  }
  return(0);
}



