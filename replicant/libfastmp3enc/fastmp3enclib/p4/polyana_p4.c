/*********************************************************************************************** 
*                                   MPEG Layer3-Audio Encoder                                  * 
*                                                                                              * 
*                                 © 1997-2005 by Fraunhofer IIS                                * 
*                                       All Rights Reserved                                    * 
*                                                                                              * 
*   This software and/or program is protected by copyright law and international treaties.     * 
*   Any reproduction or distribution of this software and/or program, or any portion of it,    * 
*   may result in severe civil and criminal penalties, and will be prosecuted to the           * 
*   maximum extent possible under law.                                                         * 
*                                                                                              * 
************************************************************************************************/
#include "mconfig.h"

#include "mp3alloc.h"
#include <math.h>
#include <assert.h>

#include "mp3ifc.h"
#include "mconfig.h"
#include "mathlib.h"
#include "psy_const.h"
#include "polyana.h"


#define DCT_LEN    POLY_PHASE_BANDS /* 32 */
#define LD_DCT_LEN 5
#define MDCT_SIZE  (FRAME_LEN_LONG/DCT_LEN) /* 18 */
#define pM64(a) ((__m64*)(a))
#define pM32(a) ((float*)(a))



#ifdef P4_INTRINSIC
#include "xmmintrin.h"

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
static void fct_iii(int n, float *a)
{
  int m, mh, mq, i, j, irev, jr, ji, kr, ki;
  /*int k=0; */
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
                  order: buffer[0..16]  = in_buf[31..15]
                         buffer[17..31] = in_buf[0..14]       
    returns:  void
    input:       
    output:      
    globals:     

*****************************************************************************/
static __inline void 
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

extern int PolyAnalyse_Opt(POLY_PHASE_HANDLE hPolyPhase,
						   const float *restrict timeSigData,
						   float *restrict polyPhaseData);
/*****************************************************************************

    functionname: PolyAnalyse
    description:  Polyphase forward transform
    returns:      error code
    input:        576 new time signal values
    output:       32 polyphase bands, containing 18 values each

*****************************************************************************/
int PolyAnalyse_Opt(POLY_PHASE_HANDLE hPolyPhase,
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
#if 0
      for(i=0; i<DCT_LEN/2;i+=4) {
        dctBuf[i  ] += polyBuf[i  ] * windowCoef[i  ] + polyBuf[i  +DCT_LEN/2] * windowCoef[i  +DCT_LEN/2];
        dctBuf[i+1] += polyBuf[i+1] * windowCoef[i+1] + polyBuf[i+1+DCT_LEN/2] * windowCoef[i+1+DCT_LEN/2];
        dctBuf[i+2] += polyBuf[i+2] * windowCoef[i+2] + polyBuf[i+2+DCT_LEN/2] * windowCoef[i+2+DCT_LEN/2];
        dctBuf[i+3] += polyBuf[i+3] * windowCoef[i+3] + polyBuf[i+3+DCT_LEN/2] * windowCoef[i+3+DCT_LEN/2];

        dctBuf[DCT_LEN/2+(i  )] += polyBuf2[i  ] * windowCoef2[i  ] - polyBuf2[i  +DCT_LEN/2] * windowCoef2[i  +DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+1)] += polyBuf2[i+1] * windowCoef2[i+1] - polyBuf2[i+1+DCT_LEN/2] * windowCoef2[i+1+DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+2)] += polyBuf2[i+2] * windowCoef2[i+2] - polyBuf2[i+2+DCT_LEN/2] * windowCoef2[i+2+DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+3)] += polyBuf2[i+3] * windowCoef2[i+3] - polyBuf2[i+3+DCT_LEN/2] * windowCoef2[i+3+DCT_LEN/2];
      }
#else
      for(i=0; i<DCT_LEN/2;i+=4) {
		__m128 mm_tmp1, mm_tmp2, mm_tmp3;
		__m128 mm_dctBuf   = _mm_load_ps(&dctBuf[i]);
		__m128 mm_polyBuf1 = _mm_load_ps(&polyBuf[          i]);
		__m128 mm_polyBuf2 = _mm_load_ps(&polyBuf[DCT_LEN/2+i]);

		mm_tmp1   = _mm_mul_ps(mm_polyBuf1, *(__m128*)&windowCoef[i          ]);
		mm_tmp2   = _mm_mul_ps(mm_polyBuf2, *(__m128*)&windowCoef[i+DCT_LEN/2]);
		mm_tmp3   = _mm_add_ps(mm_tmp1, mm_tmp2);
	    _mm_store_ps(&dctBuf[i], _mm_add_ps(mm_tmp3, mm_dctBuf));
      }

      for(i=0; i<DCT_LEN/2;i+=4) {
		__m128 mm_tmp1, mm_tmp2, mm_tmp3;
		__m128 mm_dctBuf   = _mm_load_ps(&dctBuf[DCT_LEN/2+i]);
		__m128 mm_polyBuf1 = _mm_load_ps(&polyBuf2[          i]);
		__m128 mm_polyBuf2 = _mm_load_ps(&polyBuf2[DCT_LEN/2+i]);

		mm_tmp1   = _mm_mul_ps(mm_polyBuf1, *(__m128*)&windowCoef2[i          ]);
		mm_tmp2   = _mm_mul_ps(mm_polyBuf2, *(__m128*)&windowCoef2[i+DCT_LEN/2]);
		mm_tmp3   = _mm_sub_ps(mm_tmp1, mm_tmp2);
	    _mm_store_ps(&dctBuf[DCT_LEN/2+i], _mm_add_ps(mm_tmp3, mm_dctBuf));
      }
#endif
      dctBuf[ 0] = tmpDct0  + polyBuf[ 0] * windowCoef[ 0];
      dctBuf[16] = tmpDct16 + polyBuf[16] * windowCoef[16] + polyBuf2[0] * windowCoef2[0];

      bufIndex = (bufIndex + 2*DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    }

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
#endif
