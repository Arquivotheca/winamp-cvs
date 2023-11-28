/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   
******************************************************************************/
#include "mathlib.h"

#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif

#if defined _M_IX86 || defined _M_X64 || ( (defined __x86_64 ||  defined __i386__) && defined __OPTIMIZE__ && ((GCC_VERSION>30402) || defined __INTEL_COMPILER  ) && !defined __CYGWIN__ )
#ifdef _MSC_VER
#pragma message(__FILE__": using SSE2 intrinsics")
#endif

#include <emmintrin.h>

#define M128i(a)(*(__m128i*)(&(a)))
#define MMX2_ALIGN_CHECK(a) (0 == ((unsigned long)(a)&0xfUL))

#ifdef __RESTRICT
#define restrict _Restrict
#else
#define restrict
#endif


/* ****************************************************************
 Downcast:
*******************************************************************
convert 8 int32_t from a and 8 int32_t form b to 16 int16_t in r0 */
static void _mm_cvtsi32_si16(__m128i* a, __m128i* b, __m128i* r0);

/******************************************************************
 Upcast:
*******************************************************************
convert 8 int16_t from a to 4 int32_t in r0 and 4 int32_t in r1 */
static void _mm_cvtsi16_si32(__m128i *a, __m128i *r0, __m128i *r1);




__inline void _mm_cvtsi32_si16(__m128i *a, __m128i *b, __m128i *r0)
{
  *r0 = _mm_packs_epi32(*a, *b);
}

__inline void _mm_cvtsi16_si32(__m128i *a, __m128i *r0, __m128i *r1)
{
  *r0 = _mm_srai_epi32(_mm_unpacklo_epi16(*a, *a), 16);
  *r1 = _mm_srai_epi32(_mm_unpackhi_epi16(*a, *a), 16);
}


void setINT_SSE2(int a, int *restrict A, int n)
{
  int i=0;

  if (MMX2_ALIGN_CHECK(A))
  {
    __m128i y1;
	int n1 = n & ~3;
    
	y1 = _mm_set1_epi32(a);

	for (; i<n1; i+=4)
	{
      _mm_store_si128((&M128i(A[i])),y1);
    }
  }

  for(; i<n;i++){
    A[i] = a;
  }
}


void roundFLOAT2FLOAT16_SSE2(const float *restrict A, float *restrict B, int n)
{
  int i=0; 
  __m128 y1, y2;
  __m128i y3, y4, y5;
  
  int n1 = n & (~7UL);
  
  if (MMX2_ALIGN_CHECK((unsigned long)A | (unsigned long)B))
  {
    for (; i<n1; i+=8) 
	{
      y1 =  _mm_load_ps(A + i    );
      y2 =  _mm_load_ps(A + i + 4   );

      y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
	  
      _mm_cvtsi32_si16(&y3, &y4, &y5); 	 
      _mm_cvtsi16_si32(&y5, &y3, &y4);

      y1 = _mm_cvtepi32_ps(y3);       /* Convert the four 32-bit signed integer values to four single precision FP values. */
      _mm_store_ps( B + i , y1);   
	  
      y1 = _mm_cvtepi32_ps(y4);       /* Convert the four 32-bit signed integer values to four single precision FP values.*/    
      _mm_store_ps( B + i + 4, y1);   
	} 
  }

  else
  {
    for (; i<n1; i+=8) 
	{
      y1 =  _mm_loadu_ps(A + i    );
      y2 =  _mm_loadu_ps(A + i + 4   );

      y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
	  
      _mm_cvtsi32_si16(&y3, &y4, &y5); 	 
      _mm_cvtsi16_si32(&y5, &y3, &y4);

      y1 = _mm_cvtepi32_ps(y3);       /* Convert the four 32-bit signed integer values to four single precision FP values. */
      _mm_storeu_ps( B + i , y1);   
	  
      y1 = _mm_cvtepi32_ps(y4);       /* Convert the four 32-bit signed integer values to four single precision FP values.*/    
      _mm_storeu_ps( B + i + 4, y1);   
	} 
  }

  for (; i<n; i++) 
  {
    if (A[i] > 32767.0f)
      B[i] = 32767;
    else if (A[i] < -32768.0f)
      B[i] = -32768;
    else if (A[i] > 0.0f)
      B[i] = (signed short) (A[i] + 0.5f);
    else if (A[i] <= 0.0f)
      B[i] = (signed short) (A[i] - 0.5f);
      
	B[i] = (float) B[i];
  }
}


void roundFLOAT2INT_SSE2(const float *restrict A, int *restrict B, int n)
{
  int i=0; 
  __m128 y1, y2;
  __m128i y3, y4;
  
  int n1 = n & (~7UL);   

  if (MMX2_ALIGN_CHECK((unsigned long)A | (unsigned long)B))
  {
    for (; i<n1; i+=8) 
	{
      y1 = _mm_load_ps(A + i    );
      y2 = _mm_load_ps(A + i +4 );
	  
	  y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); 

	  _mm_store_si128((__m128i *)(B + i) , y3);  
      _mm_store_si128((__m128i *)(B + i +4) , y4);     
	}

    if (i+4 <= n)
	{  
      y1 = _mm_load_ps(A + i    ); 
      y3 = _mm_cvtps_epi32(y1); 
      
      _mm_store_si128((__m128i *)(B + i) , y3);  
	 
      i += 4;
	}
  }
  
  else
  {
    for (; i<n1; i+=8) 
	{
      y1 = _mm_loadu_ps(A + i    );
      y2 = _mm_loadu_ps(A + i +4 );
	  
	  y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); 

	  _mm_storeu_si128((__m128i *)(B + i) , y3);  
      _mm_storeu_si128((__m128i *)(B + i +4) , y4);     
	}

    if (i+4 <= n)
	{  
      y1 = _mm_loadu_ps(A + i    ); 
      y3 = _mm_cvtps_epi32(y1); 
      _mm_storeu_si128((__m128i *)(B + i) , y3);  
      i+=4;
	}
  }

  for (; i<n; i++)
  {
    if (A[i] > 0.0f)
      B[i] = (int) (A[i] + 0.5f);
    else if (A[i] <= 0.0f)
      B[i] = (int) (A[i] - 0.5f);
  }
}

void roundFLOAT2SHORT_SSE2(const float *restrict A, signed short *restrict B, int n)
{
  int i=0; 
  __m128 y1, y2;
  __m128i y3, y4, y5;
  
  int n1 = n & (~7UL);   
    
  if (MMX2_ALIGN_CHECK((unsigned long)A | (unsigned long)B))
  {
    for (; i<n1; i+=8) 
	{
      y1 = _mm_load_ps(A + i    );
      y2 = _mm_load_ps(A + i +4 );
	  
	  y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); 

      _mm_cvtsi32_si16(&y3, &y4, &y5);  
	  _mm_store_si128((__m128i *)(B + i) , y5);  
    }
  }

  else
  {
    for (; i<n1; i+=8) 
	{
      y1 = _mm_loadu_ps(A + i    );
      y2 = _mm_loadu_ps(A + i +4 );
	  
	  y3 = _mm_cvtps_epi32(y1); /* Convert the four single precision FP values in y1 to four signed 32-bit integer values */
      y4 = _mm_cvtps_epi32(y2); 

      _mm_cvtsi32_si16(&y3, &y4, &y5);   
	  _mm_storeu_si128((__m128i *)(B + i) , y5);  
    }
  }

  for (; i<n; i++)
  {
    if (A[i] > 32767.0f)
      B[i] = 32767;
    else if (A[i] < -32768.0f)
      B[i] = -32768;
    else if (A[i] > 0.0f)
      B[i] = (signed short) (A[i] + 0.5f);
    else if (A[i] <= 0.0f)
      B[i] = (signed short) (A[i] - 0.5f);
  }
}

#endif

