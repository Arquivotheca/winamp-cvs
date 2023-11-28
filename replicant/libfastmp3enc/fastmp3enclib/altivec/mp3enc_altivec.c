/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 2005 by Fraunhofer IIS                                     *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3enc_altivec.c,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   N. Rettelbach                                                                    *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>

#include "mconfig.h"
#include "sf_estim.h"
#include "sf_cmprs.h"
#include "interface.h"
#include "mathmac.h"
#include "mathlib.h"
#include "mp3alloc.h"
#include "quantize.h"

/* Some usefull constants and masks */
#define AltiVecFltZeros    ((vector float)(-0.0f))
#define AltiVecFltOnes     ((vector float)( 1.0f))
#define AltiVecFltMin      ((vector float)(FLT_MIN))
#define AltiVecFltAbsMask  ((vector unsigned int)(0X7FFFFFFFUL))
#define AltiVecFltSignMask ((vector unsigned int)(0X80000000UL))
#define AltiVecFltReverseOrderMask ((vector unsigned char)(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3))
#define AltiVecFltAbs(v_x) ((vector float)vec_and((vector unsigned int)v_x, AltiVecFltAbsMask))


static void
CalcFormFactorChannel_Opt(float * restrict sfbFormFactor,
                          float * restrict sfbMaxSpec,
                          const struct PSY_OUT_CHANNEL *psyOutChan)
{
  int i, j;
  ALIGN_16_BYTE float tmpSpec[FRAME_LEN_LONG];

  for(i=0; i<(psyOutChan->sfbOffsets[psyOutChan->sfbActive]);i+=4) {
    vector unsigned char v_validMask;
    vector float v_x, v_y, v_est, v_t1, v_t2;

    /* Make all values non negative */
    v_x = AltiVecFltAbs(*(vector float*)(psyOutChan->mdctSpectrum + i));
    /* Check if there are components smaller than FLT_MIN (avoid denormalized numbers) ... */
    v_validMask = (vector unsigned char)vec_cmpge(v_x, AltiVecFltMin);
    /* ... and replace these values with FLT_MIN for the reciprocal estimate */
    v_y = (vector float)vec_sel((vector unsigned char)AltiVecFltMin, (vector unsigned char)v_x, v_validMask);
    /* ... replace these values with 0.0f */
    v_x = (vector float)vec_sel((vector unsigned char)AltiVecFltZeros, (vector unsigned char)v_x, v_validMask);
    /* Reciprocal sqrt estimate:  ~ 12 bits precision */
    v_est = vec_rsqrte(v_y);
    /* Newton-Raphson iteration */
    v_t1  = vec_madd(v_y, (vector float)(-0.5f), AltiVecFltZeros); 
    v_t2  = vec_madd(v_est, v_est, AltiVecFltZeros);
    v_t2  = vec_madd(v_t1, v_t2, (vector float)(1.5f));
    v_est = vec_madd(v_est, v_t2, AltiVecFltZeros);
    /* Multiply reciprocal sqrt with argument and accumulate results */
    *((vector float*)(tmpSpec+i)) = vec_madd(v_est, v_x, AltiVecFltZeros);
  }

  for (i=0; i<psyOutChan->sfbActive; i++) {
    float maxSpec;
    maxSpec          = 0.0f;
    sfbFormFactor[i] = 1e-10f;

    for(j = psyOutChan->sfbOffsets[i]; j < psyOutChan->sfbOffsets[i+1]; j+=2)  {
      sfbFormFactor[i] += tmpSpec[j] + tmpSpec[j+1];

      if (tmpSpec[j]   > maxSpec) 
        maxSpec = tmpSpec[j];
      if (tmpSpec[j+1] > maxSpec) 
        maxSpec = tmpSpec[j+1];
    }
    sfbMaxSpec[i] = maxSpec*maxSpec;
  }
}



ALIGN_16_BYTE static const float quantTableQ[16]=
{
  1.000000000000f,
  0.878126080187f,
  0.771105412704f,
  0.677127773468f,
  0.594603557501f,
  0.522136891214f,
  0.458502021602f,
  0.402622582987f,
  0.353553390593f,
  0.310464453018f,
  0.272626933166f,
  0.239400820175f,
  0.210224103813f,
  0.184603268242f,
  0.162104944331f,
  0.142348579345f
};

ALIGN_16_BYTE static const float quantTableE[17]=
{
  1.6777216000000000e+007f,
  2.0971520000000000e+006f,
  2.6214400000000000e+005f,
  3.2768000000000000e+004f,
  4.0960000000000000e+003f,
  5.1200000000000000e+002f,
  6.4000000000000000e+001f,
  8.0000000000000000e+000f,
  1.0000000000000000e+000f,
  1.2500000000000000e-001f,
  1.5625000000000000e-002f,
  1.9531250000000000e-003f,
  2.4414062500000000e-004f,
  3.0517578125000000e-005f,
  3.8146972656250000e-006f,
  4.7683715820312500e-007f,
  5.9604644775390625e-008f
};

/*****************************************************************************

    functionname:calcExpSpec 
    description: compresses spectrum lines  
                 expSpectrum = mdctSpectrum^(3/4)    
    input: number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/

static void
calcExpSpec_Opt (float       *expSpec, 
                 const float *mdctSpectrum,
                 int          noOfLines)
{
  static const vector float v_5_4      = (vector float)( 1.2500f);
  static const vector float v_min_1_4  = (vector float)(-0.2500f);
  static const vector float v_one      = (vector float)( 1.0000f);

  vector unsigned int v_sign;
  vector float v_est, v_t1, v_t2, v_x, v_scale;
  vector unsigned char v_validMask;
  int line;
  int preCnt;
  int offset    = (int) ((size_t)expSpec&(size_t)0xf);
  int floatSize = sizeof(float);
  preCnt = min( noOfLines, (4-(offset/floatSize))%4);

  assert( (((size_t)expSpec)&0xf) == (((size_t)mdctSpectrum)&0xf) );

  for(; line<preCnt; line++) {
    /* x^(3/4) */
    float tmp;
    float sign = mdctSpectrum[line]>=0.f?1.f:-1.f;
    tmp = (float)fabs(mdctSpectrum[line]);
    tmp = (float)sqrt(tmp);
    expSpec[line] = sign * (float)(tmp * sqrt(tmp));
  }

  for(line = 0; line < noOfLines-3; line+=4) {

    v_x = *((const vector float*)&mdctSpectrum[line]);
    /* Get sign of vector values */
    v_sign = vec_and(AltiVecFltSignMask, (vector unsigned int)v_x);
    /* Make non negative values */
    v_x = (vector float)vec_andc((vector unsigned int)v_x, AltiVecFltSignMask);
    /* Check if there are components smaller than FLT_MIN (avoid denormalized numbers) ... */
    v_validMask = (vector unsigned char)vec_cmpge(v_x, AltiVecFltMin);
    /* ... replace these values with 0.0f */
    /* v_x = (vector float)vec_sel((vector unsigned char)AltiVecFltZeros, (vector unsigned char)v_x, v_validMask); */

    /* Coarse estimate of reciprocal of fourth root */
    v_est = vec_rsqrte(v_x);
    v_est = vec_madd(v_est, v_x, AltiVecFltZeros);
    v_est = vec_rsqrte(v_est);
    /* Check if there are components smaller than FLT_MIN (avoid denormalized numbers) ... */
    /* ... replace these values with 0.0f */
    v_est = (vector float)vec_sel((vector unsigned char)AltiVecFltZeros, (vector unsigned char)v_est, v_validMask);
  
    /* Newton-Raphson iteration
      Multiply x value by -0.5 */
    v_t1 = vec_madd(v_x, v_min_1_4, AltiVecFltZeros);
    /* Square estimate */
    v_t2 = vec_madd(v_est, v_est, AltiVecFltZeros);
    v_t2 = vec_madd(v_t2, v_t2, AltiVecFltZeros);
    /* Multiply estimate with -x/2.0 and add 3.0/2.0 */
    v_t2 = vec_madd(v_t1, v_t2, v_5_4);
 
    /* Multiply with estimate */
    v_est = vec_madd(v_est, v_t2, AltiVecFltZeros);
    /* Multiply with x value yields x^(3/4) */
    v_est = vec_madd(v_est, v_x, AltiVecFltZeros);
    /* Merge signs */
    v_est = (vector float)vec_or(v_sign, (vector unsigned int)v_est);

    /* Convert to signed fixed point */
    *((vector float*) &expSpec[line]) = v_est;
  }

  for(; line<noOfLines; line++) {
    /* x^(3/4) */
    float tmp;
    float sign = mdctSpectrum[line]>=0.f?1.f:-1.f;
    tmp = (float)fabs(mdctSpectrum[line]);
    tmp = (float)sqrt(tmp);
    expSpec[line] = sign * (float)(tmp * sqrt(tmp));
  }
}

static const vector float v_logCon   = (vector float)(.4054f); /*(logCon+0.5f)=0.5f-0.0946f;*/
static const float k = .4054f;

typedef union {
  float f[4];
  vector float v_f;
} altivec_f;

static void quantizeExpSpecLines_Opt(const int gain,
                                     const int noOfLines,
                                     const float * restrict expSpec,
                                     signed int * restrict quaSpectrum)
{
  altivec_f v_scale;
  vector unsigned int v_sign;
  vector float v_est, v_x;
  vector unsigned char v_validMask;
  int line;
  int preCnt;
  int offset    = (int) ((size_t)expSpec&(size_t)0xf);
  int floatSize = sizeof(float);
  preCnt = (4-(offset/floatSize))%4;

  v_scale.f[0] = quantTableE[(gain>>4)+8]*quantTableQ[gain & 15];
  v_scale.v_f  = vec_splat(v_scale.v_f, 0);

  assert( (((size_t)expSpec)&0xf) == (((size_t)quaSpectrum)&0xf));

  /* align source and destination pointers to 16 byte border */
  for(line=0; line<preCnt; line++) {
    float tmp  = expSpec[line];
    if (tmp < 0.0f) {
      tmp = -tmp;
      quaSpectrum[line] = -(int)(k + v_scale.f[0] * tmp);
    }
    else {
      quaSpectrum[line] = (int)(k + v_scale.f[0] * tmp);
    }
  }

  /* main loop */
  for( ; line<noOfLines; line+=4) {
    v_x = *((const vector float*) &expSpec[line]);
    /* Get sign of vector values */
    v_sign = vec_and(AltiVecFltSignMask,  (vector unsigned int)v_x);
    /* Make non negative values */
    v_x = (vector float)vec_and(AltiVecFltAbsMask, (vector unsigned int)v_x);
    /* Multiply with scaling factor and add rounding constant */
    v_x = vec_madd(v_x, v_scale.v_f, v_logCon);
    /* Merge signs */
    v_x = (vector float)vec_or(v_sign, (vector unsigned int) v_x);
    /* Convert to signed fixed point */
    *((vector signed int*) &quaSpectrum[line]) = vec_cts(v_x, 0);
  }

  /* EPILOG */
  for(line=0; line<noOfLines; line++) {
    float tmp  = expSpec[line];
    if (tmp < 0.0f) {
      tmp = -tmp;
      quaSpectrum[line] = -(int)(k + v_scale.f[0] * tmp);
    }
    else {
      quaSpectrum[line] = (int)(k + v_scale.f[0] * tmp);
    }
  }
}

/*****************************************************************************

    functionname:quantizeLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/

static void quantizeLines_Opt(const int gain,
                              const int noOfLines,
                              const float * restrict mdctSpectrum,
                              signed int * restrict quaSpectrum)
{
  static const vector float v_5_4      = (vector float)(1.25f);
  static const vector float v_min_1_4  = (vector float)(-.25f);
  vector float v_est, v_t1, v_t2, v_x;
  vector unsigned int v_sign;
  altivec_f v_scale;
  int line;


  v_scale.f[0] = quantTableE[(gain>>4)+8]*quantTableQ[gain & 15];
  v_scale.v_f = vec_splat(v_scale.v_f, 0);

  line=0;
  if( (((size_t)mdctSpectrum)&0xf) == (((size_t)quaSpectrum)&0xf)) {
    /* align target and destination pointer to 16 byte border */
    int preCnt;
    int offset    = (int) ((size_t)mdctSpectrum&(size_t)0xf);
    int floatSize = sizeof(float);
    preCnt = (4-(offset/floatSize))%4;

    for(line=0; line<preCnt; line++) {
      float tmp  = mdctSpectrum[line];
      if (tmp < 0.0f) {
        tmp = (float)sqrt(-tmp);
        tmp *= (float)sqrt(tmp); /* x^(3/4) */
        quaSpectrum[line] = -(int)(k + v_scale.f[0] * tmp);
      }
      else {
        tmp = (float)sqrt(tmp);
        tmp *= (float)sqrt(tmp); /* x^(3/4) */
        quaSpectrum[line] = (int)(k + v_scale.f[0] * tmp);
      }
    }

    for( ; line < noOfLines-3; line+=4) {
      v_x = *(const vector float*) &mdctSpectrum[line];
      /* Get sign of vector values */
      v_sign = vec_and(AltiVecFltSignMask,  (vector unsigned int)v_x);
      /* Make non negative values */
      v_x = (vector float)vec_andc((vector unsigned int)v_x, AltiVecFltSignMask);
      /* Coarse estimate of reciprocal of fourth root */
      v_est = vec_rsqrte(v_x);
      v_est = vec_madd(v_est, v_x, AltiVecFltZeros);
      v_est = vec_rsqrte(v_est);
      /* Newton-Raphson iteration
         Multiply x value by -0.5 */
      v_t1 = vec_madd(v_x, v_min_1_4, AltiVecFltZeros);
      /* Square estimate */
      v_t2 = vec_madd(v_est, v_est, AltiVecFltZeros);
      v_t2 = vec_madd(v_t2, v_t2, AltiVecFltZeros);
      /* Multiply estimate with -x/2.0 and add 3.0/2.0 */
      v_t2 = vec_madd(v_t1, v_t2, v_5_4);
      /* Multiply with estimate */
      v_est = vec_madd(v_est, v_t2, AltiVecFltZeros);
      /* Multiply with x value yields x^(3/4) */
      v_est = vec_madd(v_est, v_x, AltiVecFltZeros);
      /* Multiply with scaling factor and add rounding constant */
      v_est = vec_madd(v_est, v_scale.v_f, v_logCon);
      /* Merge signs */
      v_est = (vector float)vec_or(v_sign, (vector unsigned int)v_est);
      /* Convert to signed fixed point */
      *(vector signed int*) &quaSpectrum[line] = vec_cts(v_est, 0);
    }
  }
  /* EPILOG */
  for( ;line<noOfLines; line++) {
    float tmp  = mdctSpectrum[line];
    if (tmp < 0.0f) {
      tmp = (float)sqrt(-tmp);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */
      quaSpectrum[line] = -(int)(k + v_scale.f[0] * tmp);
    }
    else {
      tmp = (float)sqrt(tmp);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */
      quaSpectrum[line] = (int)(k + v_scale.f[0] * tmp);
    }
  }
}


#include "polyana.h"

#define DCT_LEN    POLY_PHASE_BANDS /* 32 */
#define LD_DCT_LEN 5
#define MDCT_SIZE  (FRAME_LEN_LONG/DCT_LEN) /* 18 */

static ALIGN_16_BYTE const float dct_iii_32_trig_data[44] =
{
  0.9238795325F,0.3826834324F,0.7071067812F,0.7071067812F,
  0.9807852804F,0.1950903220F,0.8314696123F,0.5555702330F,
  0.9238795325F,0.3826834324F,0.3826834324F,0.9238795325F,
  0.9951847267F,0.0980171403F,0.8819212643F,0.4713967368F,
  0.9569403357F,0.2902846773F,0.7730104534F,0.6343932842F,
  0.9807852804F,0.1950903220F,0.5555702330F,0.8314696123F,
  0.8314696123F,0.5555702330F,0.1950903220F,0.9807852804F,
  0.9987954562F,0.0490676743F,0.9039892931F,0.4275550934F,
  0.9700312532F,0.2429801799F,0.8032075315F,0.5956993045F,
  0.9891765100F,0.1467304745F,0.8577286100F,0.5141027442F,
  0.9415440652F,0.3368898534F,0.7409511254F,0.6715589548F
};


/*****************************************************************************

    functionname: fct_iii
    description:  DCT Type III Transform
                  a[k]=sum {j=0..n-1} F[j]*cos(pi*j*(k+1/2)/n),0<=k<n

                  optimized version with trig data table look up for n=32
                  trigdata generation for general case commented out
    input:        32 time signal values
    output:       32 transformed values

*****************************************************************************/
static inline void fct_iii(int n, float *a)
{
  int m, mh, mq, i, j, irev, jr, ji, kr, ki;
  int k=0;
  float  wr, wi, xr, xi;
  const float *tPtr;

  assert(n==32);
  
  tPtr = dct_iii_32_trig_data;
  /*
    theta = 4 * atan(1.0) / n;
  */
  if (n > 1)
  {
    m = n >> 1;
         
    /*
      wr=cos(0.5 * theta * m);
    */
    wr = tPtr[2];
    xr = a[m] * wr;
      
    a[m] = a[0] - xr;
    a[0] += xr;
  }
  for (m = n; (mh = m >> 1) >= 2; m = mh)
  {
         
    mq = mh >> 1;
    /* ---- real & complex to real butterflies ---- */
    
    /*
      irev = 0;
    */

    for (jr = 0; jr < n; jr += m)
    {
      float t1,t2,t3,t4 ;
      /* 
         wr = cos(0.5 * theta * (irev + mq));
         wi = sin(0.5 * theta * (irev + mq));
         for (k = n >> 2; k > (irev ^= k); k >>= 1);
      */
      wr=*tPtr++;
      wi=*tPtr++;
      
      ki = jr + mq;
      kr = n - ki;
      ji = kr - mq;
      xr = wr * a[ki] + wi * a[kr];
      xi = wr * a[kr] - wi * a[ki];

      t1 = a[jr] - xr;
      t2 = a[ji] + xi;
      t3 = a[jr] + xr;
      t4 = a[ji] - xi;

      a[kr] = t1;
      a[ki] = t2;
      a[jr] = t3;
      a[ji] = t4;
    }

    if (mh == 2) continue;
    /* ---- complex to complex butterflies ---- */
    irev = 0;
    for (i = 0; i < n; i += m)
    {
      /*
        wr = cos(theta * (irev + mq));
        wi = sin(theta * (irev + mq));
        for (k = n >> 2; k > (irev ^= k); k >>= 1);
      */

      wr=*tPtr++;
      wi=*tPtr++;
             
      for (j = 1; j < mq; j++)
      {
        float t1,t2,t3,t4 ;
        jr = i + j;
        ki = i + mh - j;
        ji = n - jr;
        kr = n - ki;
        xr = wr * a[ki] + wi * a[kr];
        xi = wr * a[kr] - wi * a[ki];

        t1 = a[jr] - xr;
        t2 = -a[ji] - xi;
        t3 = a[jr] + xr;
        t4 = a[ji] - xi;

        a[kr] = t1;
        a[ki] = t2;
        a[jr] = t3;
        a[ji] = t4;

      }
    }
  }

  /* ---- unscrambler ---- */
#if 0
  i = 0;
  for (j = 1; j < n - 1; j++)
  {
    for (k = n >> 1; k > (i ^= k); k >>= 1)
	  (void) 0;
    if (j < i)
    {
	  xr = a[j];
      a[j] = a[i];
      a[i] = xr;
    }
  }
#else
   #define swap_2(a1,a2) xr=a[a1]; a[a1]=a[a2]; a[a2]=xr;
   swap_2(1,16);
   swap_2(2,8);
   swap_2(3,24);
   swap_2(5,20);
   swap_2(6,12);
   swap_2(7,28);
   swap_2(9,18);
   swap_2(11,26);
   swap_2(13,22);
   swap_2(15,30);
   swap_2(19,25);
   swap_2(23,29);
#endif
}

/*****************************************************************************

    functionname: feedInput  
    description:  replace 32 oldest samples with 32 new time samples in the 
                  polyphase input buffer;
                  reorder time signal input to allow for fast windowing and
                  lapping
                  order: 
                         buffer1[0..16]  = in_buf[31..15]
                         buffer1[17..31] = in_buf[0..14]       

                         buffer2[0..16]  = in_buf[15..31]
                         buffer2[17..31] = in_buf[14.. 0]       

    returns:  void
    input:       
    output:      
    globals:     

*****************************************************************************/
static inline void 
feedInput( POLY_PHASE_HANDLE hPolyPhase,
           const float *timeSignal)
{
  int i;
  float *buffer  = hPolyPhase->polyBuffer  + hPolyPhase->curNdx;
  float *buffer2 = hPolyPhase->polyBuffer2 + hPolyPhase->curNdx;

  /*
    replace 32 oldest samples with 32 new samples 

    order: 
    buffer[0..16]  = in_buf[31..15]
    buffer[17..31] = in_buf[0..14]       
  */
  for (i=0; i<=(DCT_LEN/2); i++) {
    buffer[i] = timeSignal[2*(DCT_LEN-1-i)];
  }
  for (i=0; i<(DCT_LEN/2-1); i++) {
    buffer[i + 1 + DCT_LEN/2] = timeSignal[2*i];
  }
  /*
    replace 32 oldest samples with 32 new samples 

    order: 
    buffer[0..16]  = in_buf[15..31]
    buffer[17..31] = in_buf[14.. 0]       
  */
  for (i=0; i<=(DCT_LEN/2); i++) {
    buffer2[i] = buffer[DCT_LEN/2-i];
  }
  for (i=0; i<(DCT_LEN/2-1); i++) {
    buffer2[i + 1 + DCT_LEN/2] = buffer[DCT_LEN-1-i];
  }
}

/*****************************************************************************

    functionname: PolyAnalyse
    description:  Polyphase forward transform
    returns:      error code
    input:        576 new time signal values
    output:       32 polyphase bands, containing 18 values each

*****************************************************************************/
static int PolyAnalyse_Opt(POLY_PHASE_HANDLE hPolyPhase,
                           const float *restrict timeSigData,
                           float *restrict polyPhaseData)
{
  /* DCT input */
  ALIGN_16_BYTE float dctBuf[DCT_LEN] ;
  float  mdctLineSign;
  int mdctLine = 0;
  int offset ;

  for (offset = 0; offset < FRAME_LEN_LONG; offset += DCT_LEN) {
    int  bufIndex, winIndex, j;

    /* copy 32 new time signal samples into polyphase buffer */
    feedInput(hPolyPhase, timeSigData+(2*offset));

    /* apply polyphase filter window */
    setFLOAT(0.0f, dctBuf, DCT_LEN);

    bufIndex = hPolyPhase->curNdx;

#if 0
    for (winIndex=0; winIndex<POLY_WINDOW_SIZE; winIndex+=(DCT_LEN*2) ) {
      int i;
      float tmpDct0  = dctBuf[ 0];
      float tmpDct16 = dctBuf[16];
      float *polyBuf, *polyBuf2;
      const float *windowCoef;
      const float *windowCoef2;

      windowCoef  = &hPolyPhase->anaWindow[winIndex];
      polyBuf     = hPolyPhase->polyBuffer2 +   bufIndex;
      windowCoef2 = &hPolyPhase->anaWindow[winIndex+DCT_LEN];
      polyBuf2    = hPolyPhase->polyBuffer  + ((bufIndex + DCT_LEN) & (POLY_WINDOW_SIZE - 1));

      for(i=0; i<DCT_LEN/2;i+=4) {
        vector float poly1_1, poly1_2;
        vector float poly2_1, poly2_2;
        vector float win1_1, win1_2;
        vector float win2_1, win2_2;
        vector float accu1, accu2;
        accu1   = *(vector float*) &dctBuf[i];
        accu2   = *(vector float*) &dctBuf[DCT_LEN/2+i];
        poly1_1 = *(vector float*) &polyBuf[i];
        poly1_2 = *(vector float*) &polyBuf[i+DCT_LEN/2];
        poly2_1 = *(vector float*) &polyBuf2[i];
        poly2_2 = *(vector float*) &polyBuf2[i+DCT_LEN/2];
        win1_1  = *(vector float*) &windowCoef[i];
        win1_2  = *(vector float*) &windowCoef[i+DCT_LEN/2];
        win2_1  = *(vector float*) &windowCoef2[i];
        win2_2  = *(vector float*) &windowCoef2[i+DCT_LEN/2];

        accu1 = vec_madd(poly1_1, win1_1, accu1);
        accu2 = vec_madd(poly2_1, win2_1, accu2);
        *((vector float*) &dctBuf[i])           = vec_madd( poly1_2, win1_2, accu1);
        *((vector float*) &dctBuf[DCT_LEN/2+i]) = vec_nmsub(poly2_2, win2_2, accu2);
      }
      dctBuf[ 0] = tmpDct0  + polyBuf[ 0] * windowCoef[ 0];
      dctBuf[16] = tmpDct16 + polyBuf[16] * windowCoef[16] + polyBuf2[0] * windowCoef2[0];

      bufIndex = (bufIndex + 2*DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    }
#else
    for (winIndex=0; winIndex<POLY_WINDOW_SIZE; winIndex+=(DCT_LEN*2) ) {
      const vector float *v_windowCoef   = (const vector float*) ( &hPolyPhase->anaWindow   [winIndex] );
      const vector float *v_windowCoef2  = (const vector float*) ( &hPolyPhase->anaWindow   [winIndex + DCT_LEN] );
      vector float *v_polyBuf            = (      vector float*) ( &hPolyPhase->polyBuffer2 [bufIndex] );
      vector float *v_polyBuf2           = (      vector float*) ( &hPolyPhase->polyBuffer  [((bufIndex + DCT_LEN) & (POLY_WINDOW_SIZE - 1))] );
      vector float *v_dctBuf             = (      vector float*) dctBuf;
      float tmpDct0  = dctBuf[ 0];
      float tmpDct16 = dctBuf[16];

      v_dctBuf[0]  = vec_madd(v_polyBuf[0], v_windowCoef[0], v_dctBuf[0]);
      v_dctBuf[1]  = vec_madd(v_polyBuf[1], v_windowCoef[1], v_dctBuf[1]);
      v_dctBuf[2]  = vec_madd(v_polyBuf[2], v_windowCoef[2], v_dctBuf[2]);
      v_dctBuf[3]  = vec_madd(v_polyBuf[3], v_windowCoef[3], v_dctBuf[3]);

      v_dctBuf[0]  = vec_madd(v_polyBuf[4], v_windowCoef[4], v_dctBuf[0]);
      v_dctBuf[1]  = vec_madd(v_polyBuf[5], v_windowCoef[5], v_dctBuf[1]);
      v_dctBuf[2]  = vec_madd(v_polyBuf[6], v_windowCoef[6], v_dctBuf[2]);
      v_dctBuf[3]  = vec_madd(v_polyBuf[7], v_windowCoef[7], v_dctBuf[3]);
    
      v_dctBuf[4]  = vec_madd(v_polyBuf2[0], v_windowCoef2[0], v_dctBuf[4]);
      v_dctBuf[5]  = vec_madd(v_polyBuf2[1], v_windowCoef2[1], v_dctBuf[5]);
      v_dctBuf[6]  = vec_madd(v_polyBuf2[2], v_windowCoef2[2], v_dctBuf[6]);
      v_dctBuf[7]  = vec_madd(v_polyBuf2[3], v_windowCoef2[3], v_dctBuf[7]);

      v_dctBuf[4]  = vec_nmsub(v_polyBuf2[4], v_windowCoef2[4], v_dctBuf[4]);
      v_dctBuf[5]  = vec_nmsub(v_polyBuf2[5], v_windowCoef2[5], v_dctBuf[5]);
      v_dctBuf[6]  = vec_nmsub(v_polyBuf2[6], v_windowCoef2[6], v_dctBuf[6]);
      v_dctBuf[7]  = vec_nmsub(v_polyBuf2[7], v_windowCoef2[7], v_dctBuf[7]);

      dctBuf[ 0] = tmpDct0  + ((float*)v_polyBuf)[ 0] * ((float*)v_windowCoef)[ 0];
      dctBuf[16] = tmpDct16 + ((float*)v_polyBuf)[16] * ((float*)v_windowCoef)[16] 
                            + ((float*)v_polyBuf2)[0] * ((float*)v_windowCoef2)[0];

      bufIndex = (bufIndex + 2*DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    }
#endif

    /*
      DCT-Type III Transform
    */
    fct_iii(DCT_LEN,dctBuf);

    /*
        Insert poly phase data into second half of mdct buffer
        and invert every odd coeff in each odd mdct spectrum
        poly0/0.............poly0/17
        poly1/0
        .
        .
        .
        poly31/0...........poly31/17
    */
    mdctLineSign = (mdctLine&0x1)?(-1.f):(1.f);
    for(j=0;j<DCT_LEN;j+=4) {
      polyPhaseData[MDCT_SIZE+mdctLine+(j  )*(2*MDCT_SIZE)] =                dctBuf[j  ];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+1)*(2*MDCT_SIZE)] = mdctLineSign * dctBuf[j+1];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+2)*(2*MDCT_SIZE)] =                dctBuf[j+2];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+3)*(2*MDCT_SIZE)] = mdctLineSign * dctBuf[j+3];
    }

    /* increment  poly phase buffer index */
    hPolyPhase->curNdx = (hPolyPhase->curNdx - DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    mdctLine++;
  }

  return (TRUE);
}



#include <sys/sysctl.h>

int IsAltiVecAvailable( void )
{
  int selectors[2] = { CTL_HW, HW_VECTORUNIT };
  int hasVectorUnit = 0;
  size_t length = sizeof(hasVectorUnit);
  int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0); 

  if( 0 == error ) return hasVectorUnit != 0;

  return 0;
}

extern void (*CalcFormFactorChannel)(float * restrict sfbFormFactor,
                                     float * restrict sfbMaxSpec,
                                     const struct PSY_OUT_CHANNEL *psyOutChan);

extern void (*calcExpSpec) (float       *expSpec, 
                            const float *mdctSpectrum,
                            int          noOfLines);

extern int (*PolyAnalyse) (POLY_PHASE_HANDLE hPolyPhase,
                           const float *restrict timeSigData,
                           float *restrict polyPhaseData);

extern void (*quantizeExpSpecLines)(const int gain,
                                    const int noOfLines,
                                    const float * restrict expSpec,
                                    signed int * restrict quaSpectrum);

extern void (*quantizeLines) (const int gain,
                              const int noOfLines,
                              const float * restrict mdctSpectrum,
                              signed int * restrict quaSpectrum);

void initAltivec()
{
  if ( IsAltiVecAvailable() ) {
    CalcFormFactorChannel   = CalcFormFactorChannel_Opt;
    calcExpSpec             = calcExpSpec_Opt;
    quantizeExpSpecLines    = quantizeExpSpecLines_Opt;
    quantizeLines           = quantizeLines_Opt;
    PolyAnalyse             = PolyAnalyse_Opt;
  }
}
