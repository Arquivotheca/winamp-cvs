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
*   $Id: quantize_p4.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W.Fiesel                                                                         *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "mp3ifc.h"
#include "mconfig.h"
#include "mathlib.h"
#include "quantize.h"
#include "sf_estim.h"
#include "mp3alloc.h"

#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif

#if defined P4_INTRINSIC  && !defined(__GNUC__) || (GCC_VERSION>30202)

#include "xmmintrin.h"
#include "emmintrin.h"

#define PREDEFINED_TABLES 1

ALIGN_16_BYTE static const int preEmphasisTab[] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};

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

static const float logCon = -0.0946f;




typedef union {
  __m128i I;
  __m128  F;
} _my_m128;
/*****************************************************************************

    functionname:calcExpSpec
    description: calculets pow(x,075) on spectral lines  
    input: number of lines to process, spectral data         
    output: "compressed" spectrum

*****************************************************************************/
static void calcExpSpec_Opt(float *expSpec, const float *mdctSpectrum, int noOfLines)
{
#if 0
   int i;
   float tmp;

   for (i=0; i<noOfLines; i++) {
       /* x^(3/4) */
       float sign = mdctSpectrum[i]>=0.f?1.f:-1.f;
       tmp = (float)fabs(mdctSpectrum[i]);
       tmp = (float)sqrt(tmp);
       expSpec[i] = sign * (float)(tmp * sqrt(tmp));
   }
#else
  int line;
  float tmpF;
  __m128 input, tmp, absval, signval, res34;
  __m128 sqrt_val1, sqrt_val2;
  __m128 result; 
  _my_m128 abs_mask, sign_mask;

  /* align target and destination pointers */
  int preCnt;
  int offset    = (int) ((size_t)expSpec&(size_t)0xf);
  int floatSize = sizeof(float);
  preCnt = min(noOfLines, (4-(offset/floatSize))%4);
  
  abs_mask.I = _mm_set1_epi32(0x7fffffff);
  sign_mask.I = _mm_set1_epi32(0x80000000);	


  for (line = 0; line < preCnt; line++) {
    /* x^(3/4) */
    float sign = mdctSpectrum[line]>=0.f?1.f:-1.f;
    tmpF = (float)fabs(mdctSpectrum[line]);
    tmpF = (float)sqrt(tmpF);
    expSpec[line] = sign * (float)(tmpF * sqrt(tmpF));
  }

  for ( ; line < noOfLines; line+=4) {
    /* take 4 values each time */
    input = _mm_load_ps(mdctSpectrum+line);
	/* get sign */
	signval = _mm_and_ps(input, sign_mask.F);
	/* get abs() */     
	absval = _mm_and_ps(input, abs_mask.F);

    /* get sprt()  - full resolution */
    sqrt_val1 = _mm_sqrt_ps(absval);
    sqrt_val2 = _mm_sqrt_ps(sqrt_val1);
    res34     = _mm_mul_ps(sqrt_val1, sqrt_val2);
	/* merge the sign into result  */
	result    = _mm_or_ps(res34, signval);
	_mm_store_ps(expSpec+line, result);
  }
#endif 
}

/*****************************************************************************

    functionname:quantizeExpSpecLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/

static void quantizeExpSpecLines_Opt(const int gain,
                                     const int noOfLines,
                                     const float * restrict expSpectrum,
                                     signed int * restrict quaSpectrum)
{
#if 0
  float quantizer;
  float k = logCon + 0.5f;
  int line;
  /*int noline=1; */

  quantizer=quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 

  for (line = 0; line < noOfLines; line++) {
    float tmp  = expSpectrum[line];

    if (tmp < 0.0f) {
      tmp = -tmp;
      quaSpectrum[line] = -(int)(k + quantizer * tmp);
    }
    else {
      quaSpectrum[line] = (int)(k + quantizer * tmp);
    }
  }
#else
  float quantizer;
  float k = logCon + 0.5f;
  float c1_5 = 1.5;
  float c0_5 = 0.5;
  int line;

  __m128 input, tmp, absval, signval, sqrt_val1, sqrt_val2, res34;
  __m128 quantfac, kfac, const1_5, const0_5;
  __m128i result; 
  _my_m128 abs_mask, sign_mask;
  /* __m128 app, part1, part2, part3, part4;  */
  
  /* align target and destination pointers */
  int preCnt;
  int offset    = (int) ((size_t)expSpectrum&(size_t)0xf);
  int floatSize = sizeof(float);
  preCnt = min(noOfLines, (4-(offset/floatSize))%4);


  quantizer=quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 

  abs_mask.I = _mm_set1_epi32(0x7fffffff);
  sign_mask.I = _mm_set1_epi32(0x80000000);	
  const1_5 = _mm_set_ps1(c1_5);
  const0_5 = _mm_set_ps1(c0_5);
  quantfac = _mm_set_ps1(quantizer);
  kfac = _mm_set_ps1(k);

  for (line = 0; line < preCnt; line++) {
    float tmp  = expSpectrum[line];
    if (tmp < 0.0f) {
      tmp = -tmp;
      quaSpectrum[line] = -(int)(k + quantizer * tmp);
    }
    else {
      quaSpectrum[line] = (int)(k + quantizer * tmp);
    }
  }

  for (; line < noOfLines; line+=4) {
    /* take 4 values each time */
    input = _mm_load_ps(expSpectrum+line);
    /* get sign */
    signval = _mm_and_ps(input, sign_mask.F);
    /* get abs() */     
    absval = _mm_and_ps(input, abs_mask.F);
    /* scale with quantizer step size */
    tmp = _mm_mul_ps(quantfac, absval);  
    tmp = _mm_add_ps(tmp, kfac);
    /*  merge the sign into result */
    tmp = _mm_or_ps(tmp, signval);
    /* float to fix conversion */
    result = _mm_cvttps_epi32(tmp);    
    _mm_storeu_si128 ((__m128i*)(quaSpectrum+line), result);
  }
#endif
}

static void quantizeLines_Opt (const int gain,
			       const int noOfLines,
			       const float * restrict mdctSpectrum,
			       signed int * restrict quaSpectrum)
{
  float quantizer;
  float k = logCon + 0.5f;
  float c1_5 = 1.5;
  float c0_5 = 0.5;
  int line;

  __m128 input, tmp, absval, signval, sqrt_val1, sqrt_val2, res34;
  __m128 quantfac, kfac, const1_5, const0_5;
  __m128i result; 
  /* __m128 app, part1, part2, part3, part4;	 */
  
  union {
	__m128i I;
	__m128  F;
  } abs_mask;

  union {
	  __m128i I;
	  __m128  F;
  } sign_mask;

  quantizer=quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 

  abs_mask.I = _mm_set1_epi32(0x7fffffff);
  sign_mask.I = _mm_set1_epi32(0x80000000);	
  const1_5 = _mm_set_ps1(c1_5);
  const0_5 = _mm_set_ps1(c0_5);
  quantfac = _mm_set_ps1(quantizer);
  kfac = _mm_set_ps1(k);

  for (line = 0; line < noOfLines; line+=4) {

    /* take 4 values each time */
    input = _mm_loadu_ps(mdctSpectrum+line);
    /* input = _mm_load_ps(mdctSpectrum+line); */
 
	/*  get sign */
	signval = _mm_and_ps(input, sign_mask.F);
    
	/* get abs() */     
	absval = _mm_and_ps(input, abs_mask.F);

    /* get sprt()  - full resolution */
    sqrt_val1 = _mm_sqrt_ps(absval);
    sqrt_val2 = _mm_sqrt_ps(sqrt_val1);
    res34 = _mm_mul_ps(sqrt_val1, sqrt_val2);
	
	/* newton- rapson iteration */
 	/*sqrt_val1 = _mm_rsqrt_ps(absval); */
 	/*app = _mm_rsqrt_ps(absval); */
 	/*part1 = _mm_mul_ps(app,app); */
 	/*part2 = _mm_mul_ps(absval, const0_5); */
 	/*part3 = _mm_mul_ps(part2, part1); */
 	/*part4 = _mm_sub_ps(const1_5, part3); */
 	/*sqrt_val1 = _mm_mul_ps(app, part4); */

 	/*sqrt_val2 = _mm_rsqrt_ps(sqrt_val1); */
 	/*app = _mm_rsqrt_ps(sqrt_val1); */
 	/*part1 = _mm_mul_ps(app,app); */
 	/*part2 = _mm_mul_ps(sqrt_val1, const0_5); */
 	/*part3 = _mm_mul_ps(part2, part1); */
 	/*part4 = _mm_sub_ps(const1_5, part3); */
 	/*sqrt_val2 = _mm_mul_ps(app, part4); */

 	/*tmp = _mm_mul_ps(sqrt_val1, sqrt_val2); */
 	/*res34 = _mm_mul_ps(tmp, absval); */

    tmp = _mm_mul_ps(quantfac, res34);  
    tmp = _mm_add_ps(tmp, kfac);
 
    /* merge the sign into result */
	tmp = _mm_or_ps(tmp, signval);

	/* float to fix conversion */
    result = _mm_cvttps_epi32(tmp);    
    _mm_storeu_si128 ((__m128i*)(quaSpectrum+line), result);
    /* _mm_store_si128 ((__m128i*)(quaSpectrum+line), result); */
  }
 
}

extern void (*calcExpSpec) (      float *expSpec, 
			    const float *mdctSpectrum,
				  int    noOfLines );
extern void (*quantizeExpSpecLines) (const int gain,
				     const int noOfLines,
				     const float * restrict expSpectrum,
				     signed int * restrict quaSpectrum);
extern void (*quantizeLines) (const int gain,
			      const int noOfLines,
			      const float * restrict mdctSpectrum,
			      signed int * restrict quaSpectrum);


#ifdef P4_CODE
void (*calcExpSpec) (      float *expSpec, 
                     const float *mdctSpectrum,
                           int    noOfLines ) = calcExpSpec_Opt;

void (*quantizeExpSpecLines) (const int gain,
                              const int noOfLines,
                              const float * restrict expSpectrum,
                              signed int * restrict quaSpectrum) = quantizeExpSpecLines_Opt;

void (*quantizeLines) (const int gain,
		       const int noOfLines,
		       const float * restrict mdctSpectrum,
		       signed int * restrict quaSpectrum) = quantizeLines_Opt;
#endif
#endif


void initQuantSSE(void) 
{
#if !defined(__GNUC__) || (GCC_VERSION>30202)
  calcExpSpec       = calcExpSpec_Opt;
#endif
}

void initQuantSSE2(void) 
{
#if !defined(__GNUC__) || (GCC_VERSION>30202)
  quantizeExpSpecLines = quantizeExpSpecLines_Opt;
  quantizeLines = quantizeLines_Opt;
#endif
}
