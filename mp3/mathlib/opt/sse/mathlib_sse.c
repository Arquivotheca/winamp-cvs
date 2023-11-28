/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: mathlib_sse.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $
   Authors:              W. Schildbach, W. Fiesel	
   contents/description: Mathlib V2.0 Vector functions, partly containing
                         SSE code.

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*include "bastypes.h" */
#include "mathlib.h"

#if defined __ICL
#define MATHALIGN16 __declspec(align(16))

#elif defined _MSC_VER
#define MATHALIGN16 __declspec(align(16))

#elif defined __GNUC__ && !defined __sparc__ && !defined __sparc_v9__ /* mul: fixing problems with gcc 3.0.4 on SunOS 5.7 */
#define MATHALIGN16  __attribute__((aligned(16)))

#else
#define MATHALIGN16
#endif

#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif

#if defined _M_IX86 || defined _M_X64 || ((defined __x86_64 || defined __i386__) && defined __OPTIMIZE__ && ((GCC_VERSION>30201) || defined __INTEL_COMPILER) )


#ifdef _MSC_VER
#pragma message(__FILE__": using SSE intrinsics")
#endif

#include <xmmintrin.h>

#define MMX2_ALIGN_CHECK(a) (0 == ((unsigned long)(a)&0xfUL))
#define M128(a) (*(__m128*)(&(a)))


#ifdef __RESTRICT
#define restrict _Restrict
#else
#define restrict
#endif

static const float ILOG2  = (float) 1.442695041f;	/* (1.0/log(2.0))  */
static const float ILOG10 = (float) 0.4342944819f;	/* (1.0/log(10.0)) */

/* prototypes */
__m128 sumup(__m128 a);
__m128 maxup(__m128 a);
__m128 minup(__m128 a);

__inline __m128 sumup(__m128 a)
{
	a = _mm_add_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,0,3,2)), a);
	return _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3,2,0,1)), a);
}

__inline __m128 maxup(__m128 a)
{
	a = _mm_max_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,0,3,2)), a);
	return _mm_max_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3,2,0,1)), a);
}

__inline __m128 minup(__m128 a)
{
	a = _mm_min_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,0,3,2)), a);
	return _mm_min_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3,2,0,1)), a);
}


/* assume no aliasing, even across function calls, enable global optimizations,
  produce fast instead of short code */
/* /#pragma optimize( "agwt", on ) */


float sumFLOAT_Opt(const float *restrict X, int n)
{
  float sum=0;
  int i=0;
  
  if(MMX2_ALIGN_CHECK((unsigned long)X)) 
  {
    __m128 acc = _mm_setzero_ps();
	int n1 = n & (~7UL);

	for(; i<n1; i+=8)
	{
	  __m128 y1, y2;

	  y1 = _mm_load_ps(X + i   );
      y2 = _mm_load_ps(X + i + 4  );
	  
	  y1 = sumup(y1);	
      y2 = sumup(y2);
	  acc = _mm_add_ss((_mm_add_ss(y1, y2)), acc);
	}

    if(i+4 <= n)
	{
      __m128 y1;
      y1 = _mm_load_ps(X + i   );
      y1 = sumup(y1);
      acc = _mm_add_ss(y1, acc);
	  i+=4;
    }
    _mm_store_ss(&sum, acc); 
  }

  for(; i<n; i++)
  {
    sum += X[i]; 
  }
  return(sum);
}

void saddFLOAT_Opt(float b, const float *restrict X, float *restrict Z, int n)
{
  int i=0;
  
  if(MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z)) 
  {
    __m128 B = _mm_set_ps1(b);
	int n1 = n & (~7UL);
    	 
	for(; i<n1; i+=8)
	{
	  __m128 y1,y2;
    
	  y1 = _mm_load_ps(X + i   );
      y2 = _mm_load_ps(X + i +4  );
	  
	  y1 = _mm_add_ps(y1,B);
      y2 =_mm_add_ps(y2,B);

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
	}

    if(i+4 <= n)
	{
      __m128 y1;
      y1 = _mm_load_ps(X + i   );
      y1 = _mm_add_ps(y1,B);
      _mm_store_ps(Z + i, y1);
      i+=4;
	}	
  }
  for(; i<n; i++)
  {
     Z[i] = X[i] + b; 
  }
}



/** $\vec Z=\vec X+\vec Y$. */
void addFLOAT_Opt(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;
  /*#pragma warning(disable:167)*/
#pragma warning(disable:4705)
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7UL);
 
    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_add_ps(_mm_load_ps(X + i    ), M128(Y[i    ]));
      y2 = _mm_add_ps(_mm_load_ps(X + i + 4), M128(Y[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_add_ps(_mm_load_ps(X + i), M128(Y[i])));
      i+=4;
    }
    for (; i<n; i++) {
      Z[i] = X[i]+Y[i];
    }
  }
  else
  {
    if (n & 1) {
      Z[0] = X[0] + Y[0];
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = X[i] + Y[i], _b = X[i + 1] + Y[i + 1];
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}

void addFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
#ifdef INTEL_PERF
  scopy_(n,Y,incY,Z,incZ);
  saxpy_(n,1.0f,X,incX,Z,incZ);
#else
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] + Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = X[ix] + Y[iy], _b = X[ix + incX] + Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
#endif
}

/** $\vec Z=\vec X-\vec Y$. */
void subFLOAT_Opt(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7);

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_sub_ps(_mm_load_ps(X + i    ), M128(Y[i    ]));
      y2 = _mm_sub_ps(_mm_load_ps(X + i + 4), M128(Y[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_sub_ps(_mm_load_ps(X + i), M128(Y[i])));
      i+=4;
    }
    for (; i<n; i++) {
      Z[i] = X[i]-Y[i];
    }
  }
  else
  {
    if (n & 1) {
      Z[0] = X[0] - Y[0];
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = X[i] - Y[i], _b = X[i + 1] - Y[i + 1];
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}

void subFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
#ifdef INTEL_PERF
  scopy_(n,X,incX,Z,incZ);
  saxpy_(n,-1.0f,Y,incY,Z,incZ);
#else
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] - Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = X[ix] - Y[iy], _b = X[ix + incX] - Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
#endif
}


/** $Z_i=X_i\cdot Y_i$. */
void multFLOAT_Opt(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7);
 
    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_mul_ps(_mm_load_ps(X + i    ), M128(Y[i    ]));
      y2 = _mm_mul_ps(_mm_load_ps(X + i + 4), M128(Y[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_mul_ps(_mm_load_ps(X + i), M128(Y[i])));
      i+=4;
    }
    for (; i<n; i++) Z[i] = X[i]*Y[i];
  }
  else
  {
    if (n & 1) {
      Z[0] = X[0] * Y[0];
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = X[i] * Y[i], _b = X[i + 1] * Y[i + 1];
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}

void multFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] * Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = X[ix] * Y[iy], _b = X[ix + incX] * Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $Z_i=\frac{X_i}{Y_i}$.
    @see Exception handling */
void divFLOAT_Approx(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7);
 
    /* use approximate division followed by one step of newton-raphson */
    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;
      __m128 d1 = _mm_load_ps(Y + i), d2 = _mm_load_ps(Y + i + 4);

      y1 = _mm_rcp_ps(d1);
      y1 = _mm_sub_ps(_mm_add_ps(y1,y1), _mm_mul_ps(d1, _mm_mul_ps(y1,y1)));
      y1 = _mm_mul_ps(y1,M128(X[i]));

      y2 = _mm_rcp_ps(d2);
      y2 = _mm_sub_ps(_mm_add_ps(y2,y2), _mm_mul_ps(d2, _mm_mul_ps(y2,y2)));
      y2 = _mm_mul_ps(y2,M128(X[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      __m128 d1 = _mm_load_ps(Y + i);
      __m128 y1 = _mm_rcp_ps(d1);
      y1 = _mm_sub_ps(_mm_add_ps(y1,y1), _mm_mul_ps(d1, _mm_mul_ps(y1,y1)));
      y1 = _mm_mul_ps(y1, M128(X[i]));
      _mm_store_ps(Z + i, y1);
      i+=4;
    }
    for (; i<n; i++) Z[i] = X[i] / Y[i];
  }
  else
  {
    if (n & 1) {
     Z[0] = X[0] / Y[0];
     i = 1;
    } else
     i = 0;
    for (; i < n; i += 2) {
     float _a = X[i] / Y[i], _b = X[i + 1] / Y[i + 1];
      Z[i] = _a;
      Z[i + 1] = _b;
   }
  }
}


void divFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] / Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = X[ix] / Y[iy], _b = X[ix + incX] / Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $\vec Z=\vec X$. */
void copyFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  if (MMX2_ALIGN_CHECK((unsigned int)X | (unsigned int)Z))
  {
    int i, n1 = n & (~7UL);

    for (i=0; i<n1; i+=8)
      {
        __m128 a,b;
        
      a = _mm_load_ps(X+i);
      b = _mm_load_ps(X+i+4);
      _mm_store_ps(Z+i,a);
      _mm_store_ps(Z+i+4,b);
      }
    if (i!=n)
    {
	for(;i<n;i++)
		Z[i] = X[i];	
    }
  }
  else
  {
	int i;
	for(i=0;i<n;i++)
		Z[i] = X[i];	
  }
}

void copyFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
#ifdef INTEL_PERF
  scopy_(n,X,incX,Z,incZ);
#else
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) + (X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) + (X[ix]), _b = (float) + (X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
#endif
}

/** $\vec Z=a\vec X$. */
void smulFLOAT_Opt(float a, const float *restrict X, float *restrict Z, int n)
{
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z))
  {
    int n1 = n & (~7UL);
    __m128 ma;

    ma = _mm_load_ps1(&a);

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_mul_ps(ma, M128(X[i    ]));
      y2 = _mm_mul_ps(ma, M128(X[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_mul_ps(ma, M128(X[i])));
      i+=4;
    }
    for (; i<n; i++) Z[i] = a*X[i];
  }
  else
  {
    int n1 = n&(~1);
    for (i=0; i < n1; i += 2) {
      float _a = (float) a * (X[i]), _b = (float) a * (X[i + 1]);
      Z[i] = _a;
      Z[i + 1] = _b;
    }
    for (; i<n; i++) {
      Z[i] = a*X[i];
    }
  }
}

void smulFLOATflex_Opt(float a, const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
#ifdef INTEL_PERF
  scopy_(n,X,incX,Z,incZ);
  sscal_(n,a,Z,incZ);
#else
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) a *(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) a * (X[ix]), _b = (float) a * (X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
#endif
}

/* simple vector operations, type int */


#if defined MMX && !defined _M_X64
typedef union {int a[2]; __m64 m;} I2;
#endif

/** $\vec Z=\vec X+\vec Y$. */
void addINT_Opt(const int *restrict X, int const *restrict Y, int *restrict Z, int n)
{
  int i;
#if defined MMX && !defined _M_X64
  int n1 = n & (~1UL);
  I2 *_X=(I2*)X,*_Y=(I2*)Y,*_Z=(I2*)Z;

  for (i=0; i<n1; i++) {
    _Z->m = _m_paddd(_X->m,_Y->m);
    _X++; _Y++; _Z++;
  }
  _mm_empty();
  for (; i<n; i++) {
    Z[i] = X[i]+Y[i];
  }
#else
  if (n & 1) {
    Z[0] = X[0] + Y[0];
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = X[i] + Y[i], _b = X[i + 1] + Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
#endif
}

void addINTflex_Opt(const int *restrict X, int incX, int const *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] + Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = X[ix] + Y[iy], _b = X[ix + incX] + Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $\vec Z=\vec X-\vec Y$. */
void subINT_Opt(const int *restrict X, int const *restrict Y, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = X[0] - Y[0];
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = X[i] - Y[i], _b = X[i + 1] - Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void subINTflex_Opt(const int *restrict X, int incX, int const *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] - Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = X[ix] - Y[iy], _b = X[ix + incX] - Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $Z_i=X_i\cdot Y_i$. */
void multINT_Opt(const int *restrict X, int const *restrict Y, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = X[0] * Y[0];
    i = 1;
  } else
    i = 0;

  for (; i < n; i += 2)
  {
    int _a = X[i] * Y[i], _b = X[i + 1] * Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

void multINTflex_Opt(const int *restrict X, int incX, int const *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] * Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = X[ix] * Y[iy], _b = X[ix + incX] * Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}
/** $Z_i=\frac{X_i}{Y_i}$.
    @see Exception handling */
void divINT_Opt(const int *restrict X, int const *restrict Y, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = X[0] / Y[0];
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = X[i] / Y[i], _b = X[i + 1] / Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

void divINTflex_Opt(const int *restrict X, int incX, int const *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = X[0] / Y[0];
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = X[ix] / Y[iy], _b = X[ix + incX] / Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $\vec Z=a\vec X$. */
void smulINT_Opt(int a, const int *restrict X, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) a *(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) a * (X[i]), _b = (int) a * (X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

void smulINTflex_Opt(int a, const int *restrict X, int incX, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) a *(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) a * (X[ix]), _b = (int) a * (X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $\vec Z=\vec X$. */
void copyINT_Opt(const int *restrict X, int *restrict Z, int n)
{
  copyFLOAT_Opt((float*)X,(float*)Z,n);
}

void copyINTflex_Opt(const int *restrict X, int incX, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) + (X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) + (X[ix]), _b = (int) + (X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/*@} */

/**@name Vector comparison operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{ */


/* type float */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
void minFLOAT_Opt(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7UL);

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_min_ps(_mm_load_ps(X+i  ), M128(Y[i    ]));
      y2 = _mm_min_ps(_mm_load_ps(X+i+4), M128(Y[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_min_ps(_mm_load_ps(X+i  ), M128(Y[i    ])));
      i+=4;
    }
    for (; i<n; i++) Z[i] = X[i] <= Y[i] ? X[i] : Y[i];
  }
  else
  {
    if (n & 1) {
     Z[0] = (float) ((X[0]) <= (Y[0]) ? (X[0]) : (Y[0]));
     i = 1;
   } else
     i = 0;
    for (; i < n; i += 2) {
     float _a = (float) ((X[i]) <= (Y[i]) ? (X[i]) : (Y[i])), _b = (float) ((X[i + 1]) <= (Y[i + 1]) ? (X[i + 1]) : (Y[i + 1]));
     Z[i] = _a;
     Z[i + 1] = _b;
    }
  }
}


void minFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ((X[0]) <= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ((X[ix]) <= (Y[iy]) ? (X[ix]) : (Y[iy])), _b = (float) ((X[ix + incX]) <= (Y[iy + incY]) ? (X[ix + incX]) : (Y[iy + incY]));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

float findminFLOAT_Opt(const float* X, int n)
{
  float min=X[0]; 
  int i=0;
  
  if (MMX2_ALIGN_CHECK((unsigned long)X ))
  {
	__m128 tmp = _mm_load_ps(X );
	int n1 = n & (~7UL);
    
	for (; i<n1; i+=8) 
	{
      __m128 y1, y2;
	  y1 = minup(_mm_load_ps(X + i  ));
      y2 = minup(_mm_load_ps(X + i + 4 ));     
      tmp = _mm_min_ss(tmp,_mm_min_ss(y1,y2));
	}
  
    if(i+4 <= n)
    {
      __m128 y1;
      y1  = minup(_mm_load_ps(X + i  ));
      tmp = _mm_min_ss(tmp,_mm_min_ss(tmp,y1));
	  i+=4;
	}
    _mm_store_ss(&min, tmp);  
  }  
  
  for(; i<n; i++)
  {
    min=(min<X[i]? min : X[i]);
  }
    
  return(min);  
}

/** $Z_i=\max\left(X_i,Y_i\right)$ */
void maxFLOAT_Opt(const float *restrict X, const float *restrict Y, float *restrict Z, int n)
{
  int i;
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y | (unsigned long)Z))
  {
    int n1 = n & (~7UL);

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_max_ps(_mm_load_ps(X+i  ), M128(Y[i    ]));
      y2 = _mm_max_ps(_mm_load_ps(X+i+4), M128(Y[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_max_ps(_mm_load_ps(X+i  ), M128(Y[i    ])));
      i+=4;
    }
    for (; i<n; i++) Z[i] = X[i] >= Y[i] ? X[i] : Y[i];
  }
  else
  {
    if (n & 1) {
      Z[0] = (float) ((X[0]) >= (Y[0]) ? (X[0]) : (Y[0]));
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = (float) ((X[i]) >= (Y[i]) ? (X[i]) : (Y[i])), _b = (float) ((X[i + 1]) >= (Y[i + 1]) ? (X[i + 1]) : (Y[i + 1]));
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}
void maxFLOATflex_Opt(const float *restrict X, int incX, const float *restrict Y, int incY, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ((X[0]) >= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ((X[ix]) >= (Y[iy]) ? (X[ix]) : (Y[iy])), _b = (float) ((X[ix + incX]) >= (Y[iy + incY]) ? (X[ix + incX]) : (Y[iy + incY]));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

float findmaxFLOAT_Opt(const float* X, int n)
{
  float max=X[0]; 
  int i=0;
    
  if (MMX2_ALIGN_CHECK((unsigned long)X ))
  {
	__m128 tmp = _mm_load_ps(X );
	int n1 = n & (~7UL);
    
	for (; i<n1; i+=8) 
	{
      __m128 y1, y2;
	  y1 = maxup(_mm_load_ps(X + i  ));
      y2 = maxup(_mm_load_ps(X + i + 4 ));     
      tmp = _mm_max_ss(tmp,_mm_max_ss(y1,y2));
	}
  
    if(i+4 <= n)
    {
      __m128 y1;
	  y1 = maxup(_mm_load_ps(X + i  ));
      tmp = _mm_max_ps(tmp,y1);
	  i+=4;
	}
  
    _mm_store_ss(&max, tmp);
  }  
  
  for(; i<n; i++)
  {
    max=(max>X[i]? max : X[i]);
  }
    
  return(max);  
}

/** $Z_i=\left\|X_i\right\|$ */
void absFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z))
  {
    int n1 = n & (~7UL);
    static MATHALIGN16 unsigned int isignmask[4] = {0x7FFFFFFFUL,0x7FFFFFFFUL,0x7FFFFFFFUL,0x7FFFFFFFUL};
    __m128 signmask = *(__m128*)isignmask;

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_and_ps(signmask, M128(X[i    ]));
      y2 = _mm_and_ps(signmask, M128(X[i + 4]));

      _mm_store_ps(Z + i, y1);
      _mm_store_ps(Z + i + 4, y2);
    }
    if (i+4 <= n) {
      _mm_store_ps(Z + i, _mm_and_ps(signmask, M128(X[i    ])));
      i+=4;
    }
    for (; i<n; i++) Z[i] = (float)fabs(X[i]);
  }
  else
  {
    if (n & 1) {
      Z[0] = (float) fabs(X[0]);
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = (float) fabs(X[i]), _b = (float) fabs(X[i + 1]);
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}
void absFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) fabs(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) fabs(X[ix]), _b = (float) fabs(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
void limitFLOAT_Opt(float a, float b, const float *restrict X, float *restrict Z, int n)
{
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z))
  {
    int n1 = n & (~3U);
    __m128 ma, mb;

    ma = _mm_load_ps1(&a);
    mb = _mm_load_ps1(&b);

    for (i=0; i<n1; i+=4) {
      __m128 x = _mm_load_ps(X + i);
      __m128 y = _mm_max_ps(x,ma);
      __m128 z = _mm_min_ps(y,mb);
      _mm_store_ps(Z+i, z);
    }
    for (; i<n; i++) {
      Z[i] = (X[i] < a ? a : (X[i] > b ? b : X[i]));
    }
  }
  else
  {
    if (n & 1) {
      Z[0] = (float) ((X[0]) < (a) ? (a) : ((X[0]) > (b) ? (b) : (X[0])));
      i = 1;
    } else
      i = 0;
    for (; i < n; i += 2) {
      float _a = (float) ((X[i]) < (a) ? (a) : ((X[i]) > (b) ? (b) : (X[i]))),
      _b = (float) ((X[i + 1]) < (a) ? (a) : ((X[i + 1]) > (b) ? (b) : (X[i + 1])));
      Z[i] = _a;
      Z[i + 1] = _b;
    }
  }
}
void limitFLOATflex_Opt(float a, float b, const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ((X[0]) < (a) ? (a) : ((X[0]) > (b) ? (b) : (X[0])));
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ((X[ix]) < (a) ? (a) : ((X[ix]) > (b) ? (b) : (X[ix]))),
     _b = (float) ((X[ix + incX]) < (a) ? (a) : ((X[ix + incX]) > (b) ? (b) : (X[ix + incX])));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
void signFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) ((X[0]) >= 0 ? 1.0 : -1.0);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) ((X[i]) >= 0 ? 1.0 : -1.0), _b = (float) ((X[i + 1]) >= 0 ? 1.0 : -1.0);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void signFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ((X[0]) >= 0 ? 1.0 : -1.0);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ((X[ix]) >= 0 ? 1.0 : -1.0), _b = (float) ((X[ix + incX]) >= 0 ? 1.0 : -1.0);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/* type int */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
void minINT_Opt(const int *restrict X, const int *restrict Y, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) ((X[0]) <= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) ((X[i]) <= (Y[i]) ? (X[i]) : (Y[i])), _b = (int) ((X[i + 1]) <= (Y[i + 1]) ? (X[i + 1]) : (Y[i + 1]));
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void minINTflex_Opt(const int *restrict X, int incX, const int *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) ((X[0]) <= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) ((X[ix]) <= (Y[iy]) ? (X[ix]) : (Y[iy])), _b = (int) ((X[ix + incX]) <= (Y[iy + incY]) ? (X[ix + incX]) : (Y[iy + incY]));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}
/** $Z_i=\max\left(X_i,Y_i\right)$ */
void maxINT_Opt(const int *restrict X, const int *restrict Y, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) ((X[0]) >= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) ((X[i]) >= (Y[i]) ? (X[i]) : (Y[i])), _b = (int) ((X[i + 1]) >= (Y[i + 1]) ? (X[i + 1]) : (Y[i + 1]));
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void maxINTflex_Opt(const int *restrict X, int incX, const int *restrict Y, int incY, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iy = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) ((X[0]) >= (Y[0]) ? (X[0]) : (Y[0]));
    i = 1;
    ix = incX;
    iy = incY;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) ((X[ix]) >= (Y[iy]) ? (X[ix]) : (Y[iy])), _b = (int) ((X[ix + incX]) >= (Y[iy + incY]) ? (X[ix + incX]) : (Y[iy + incY]));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}
/** $Z_i=\left\|X_i\right\|$ */
void absINT_Opt(const int *restrict X, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) abs(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) abs(X[i]), _b = (int) abs(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void absINTflex_Opt(const int *restrict X, int incX, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) abs(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) abs(X[ix]), _b = (int) abs(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
void limitINT_Opt(int a, int b, const int *restrict X, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) ((X[0]) < (a) ? (a) : ((X[0]) > (b) ? (b) : (X[0])));
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) ((X[i]) < (a) ? (a) : ((X[i]) > (b) ? (b) : (X[i]))),
     _b = (int) ((X[i + 1]) < (a) ? (a) : ((X[i + 1]) > (b) ? (b) : (X[i + 1])));
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void limitINTflex_Opt(int a, int b, const int *restrict X, int incX, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) ((X[0]) < (a) ? (a) : ((X[0]) > (b) ? (b) : (X[0])));
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) ((X[ix]) < (a) ? (a) : ((X[ix]) > (b) ? (b) : (X[ix]))),
     _b = (int) ((X[ix + incX]) < (a) ? (a) : ((X[ix + incX]) > (b) ? (b) : (X[ix + incX])));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
void signINT_Opt(const int *restrict X, int *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (int) ((X[0]) >= 0 ? 1.0 : -1.0);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    int _a = (int) ((X[i]) >= 0 ? 1.0 : -1.0), _b = (int) ((X[i + 1]) >= 0 ? 1.0 : -1.0);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void signINTflex_Opt(const int *restrict X, int incX, int *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (int) ((X[0]) >= 0 ? 1.0 : -1.0);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    int _a = (int) ((X[ix]) >= 0 ? 1.0 : -1.0), _b = (int) ((X[ix + incX]) >= 0 ? 1.0 : -1.0);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/*@} */

/**@name Vector quantizing operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
   These operations only operate on type float (and possibly DOUBLE).
  */
/*@{ */




/** $Z_i=\left\lfloor X_i\right\rfloor$ */
void floorFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) floor(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) floor(X[i]), _b = (float) floor(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void floorFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) floor(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) floor(X[ix]), _b = (float) floor(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** $Z_i=\left\lceil X_i\right\rceil$ */
void ceilFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) ceil(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) ceil(X[i]), _b = (float) ceil(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void ceilFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ceil(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ceil(X[ix]), _b = (float) ceil(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** round $Z_i$ to nearest integer. $Z_i=\left\lfloor X_i+\frac{1}{2}\right\rfloor$ */
void nintFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) floor(X[0] + 0.5);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) floor(X[i] + 0.5), _b = (float) floor(X[i + 1] + 0.5);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void nintFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) floor(X[0] + 0.5);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) floor(X[ix] + 0.5), _b = (float) floor(X[ix + incX] + 0.5);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/** truncate fractional part of $Z_i$ */
void truncFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) ((X[0]) >= 0.0 ? floor(X[0]) : -floor(-X[0]));
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) ((X[i]) >= 0.0 ? floor(X[i]) : -floor(-X[i])),
     _b = (float) ((X[i + 1]) >= 0.0 ? floor(X[i + 1]) : -floor(-X[i + 1]));
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void truncFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) ((X[0]) >= 0.0 ? floor(X[0]) : -floor(-X[0]));
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) ((X[ix]) >= 0.0 ? floor(X[ix]) : -floor(-X[ix])),
     _b = (float) ((X[ix + incX]) >= 0.0 ? floor(X[ix + incX]) : -floor(-X[ix + incX]));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}



void roundFLOAT2FLOAT16_Opt(const float *restrict X, float *restrict Z, int n)
{ 
  int i=0; 
#ifndef _M_X64
#pragma warning(disable:167)
  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z))
  {

	int n1 = n & (~7UL);                   
    _mm_empty();
    for (i=0; i<n1; i+=8) {
      __m64 y1,y2;
     
	  y1 = _mm_cvtps_pi16(_mm_load_ps(X + i    ));  /* Convert the four single precision FP values */
   	  y2 = _mm_cvtps_pi16(_mm_load_ps(X + i + 4));  /* in a to four signed 16-bit integer values */
 
	  _mm_store_ps(Z + i, _mm_cvtpi16_ps(y1));      /* Convert the four 16-bit signed integer values */
	  _mm_store_ps(Z + i + 4, _mm_cvtpi16_ps(y2));  /* in a to four single prec/ision FP values. */
	}

    
	if (i+4 <= n) {
      __m64 y3 = _mm_cvtps_pi16(_mm_load_ps(X + i    ));	
	  _mm_store_ps(Z + i, _mm_cvtpi16_ps(y3));
	  i+=4;
    }
    
	_mm_empty();
  }
    for (; i<n; i++) {
      if (X[i] > 32767.0f)
        Z[i] = 32767;
      else if (X[i] < -32768.0f)
        Z[i] = -32768;
      else if (X[i] > 0.0f)
        Z[i] = (signed short) (X[i] + 0.5f);
      else if (X[i] <= 0.0f)
        Z[i] = (signed short) (X[i] - 0.5f);
 	    
	  Z[i] = (float) Z[i];
	}
  }
  
#else

  for(i=0; i<n; i++){
    if(X[i] > 32767.0f)
	  Z[i] = 32767;
    else if(X[i] < -32768.0f)
      Z[i] = -32768;
    else if (X[i] > 0.0f)
      Z[i] = (signed short)(X[i] + 0.5f);
    else if(X[i] <= 0.0f)
      Z[i] = (signed short)(X[i] - 0.5f);

    Z[i] = (float)Z[i];
  }
}
#endif

/**@name Transcendent vector operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */

/** $Z_i=\sin\left(X_i\right)$. */
void sinFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) sin(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) sin(X[i]), _b = (float) sin(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void sinFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) sin(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) sin(X[ix]), _b = (float) sin(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=\cos\left(X_i\right)$. */
void cosFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) cos(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) cos(X[i]), _b = (float) cos(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void cosFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) cos(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) cos(X[ix]), _b = (float) cos(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=e^{X_i}$. */
void expFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) exp(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) exp(X[i]), _b = (float) exp(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void expFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) exp(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) exp(X[ix]), _b = (float) exp(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=X_i^a$. */
void spowFLOAT_Opt(float a, const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) pow(X[0], a);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) pow(X[i], a), _b = (float) pow(X[i + 1], a);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void spowFLOATflex_Opt(float a, const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) pow(X[0], a);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) pow(X[ix], a), _b = (float) pow(X[ix + incX], a);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=\log\left(X_i\right)$. */
void logFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) log(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) log(X[i]), _b = (float) log(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void logFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) log(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) log(X[ix]), _b = (float) log(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=\log_2\left(X_i\right)$. */
void log2FLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) log(X[0]) * ILOG2;
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) log(X[i]) * ILOG2, _b = (float) log(X[i + 1]) * ILOG2;
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void log2FLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) log(X[0]) * ILOG2;
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) log(X[ix]) * ILOG2, _b = (float) log(X[ix + incX]) * ILOG2;
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=\log_10\left(X_i\right)$. */
void log10FLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) log(X[0]) * ILOG10;
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) log(X[i]) * ILOG10, _b = (float) log(X[i + 1]) * ILOG10;
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void log10FLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) log(X[0]) * ILOG10;
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) log(X[ix]) * ILOG10, _b = (float) log(X[ix + incX]) * ILOG10;
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=a\log\left(b X_i\right)$. */
void alogbFLOAT_Opt(float a, float b, const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) (a * log(b * (X[0])));
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) (a * log(b * (X[i]))), _b = (float) (a * log(b * (X[i + 1])));
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}
void alogbFLOATflex_Opt(float a, float b, const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) (a * log(b * (X[0])));
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) (a * log(b * (X[ix]))), _b = (float) (a * log(b * (X[ix + incX])));
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}

/** $Z_i=\sqrt{X_i}$. */
void sqrtFLOAT_Opt(const float *restrict X, float *restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) sqrt(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) sqrt(X[i]), _b = (float) sqrt(X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

void sqrtFLOATflex_Opt(const float *restrict X, int incX, float *restrict Z, int incZ, int n)
{
  int i = 0, ix = 0, iz = 0;
  if (n & 1) {
    Z[0] = (float) sqrt(X[0]);
    i = 1;
    ix = incX;
    iz = incZ;
  }
  for (; i < n; i += 2) {
    float _a = (float) sqrt(X[ix]), _b = (float) sqrt(X[ix + incX]);
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iz += 2 * incZ;
  }
}
/*@} */

/**@name Vector increments
   Functions that accept vector increments can access column vectors of matrices.
   Instead of accessing $X_i$ they will access elements $X_{i\cdot \mbox{incX}}$.
  */

float dotFLOAT_Opt(const float *restrict X, const float *restrict Y, int n)
{
  float acc;
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y))
  {
    int n1;
    __m128 a1, a2;

    a1 = _mm_setzero_ps();
    a2 = _mm_setzero_ps();

    n1 = n & (~7U);
    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_mul_ps(_mm_load_ps(X+i  ), M128(Y[i    ]));
      y2 = _mm_mul_ps(_mm_load_ps(X+i+4), M128(Y[i + 4]));

      a1 = _mm_add_ps(a1, y1);
      a2 = _mm_add_ps(a2, y2);
    }
    if (i+4 <= n) {
      a1 = _mm_add_ps(a1, _mm_mul_ps(_mm_load_ps(X+i  ), M128(Y[i    ])));
      i += 4;
    }
    _mm_store_ss(&acc,sumup(_mm_add_ps(a1,a2)));

    for (; i<n; i++) acc += X[i]*Y[i];
  }
  else
  {
    acc = 0.0f;
    if (n) {
      acc = X[0] * Y[0];
    }
    for (i = 1; i < n; i++)
      acc += X[i] * Y[i];
  }
  return acc;
}

float dist2FLOAT_Opt(const float *restrict X, const float *restrict Y, int n)
{
  float t, acc;
  int i;

  if (MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Y))
  {
    int n1 = n & (~7);
    __m128 a1,a2;

    a1 = _mm_setzero_ps();
    a2 = _mm_setzero_ps();

    for (i=0; i<n1; i+=8) {
      __m128 y1,y2;

      y1 = _mm_sub_ps(_mm_load_ps(X+i  ), M128(Y[i    ]));
      y2 = _mm_sub_ps(_mm_load_ps(X+i+4), M128(Y[i + 4]));

      a1 = _mm_add_ps(a1, _mm_mul_ps(y1,y1));
      a2 = _mm_add_ps(a2, _mm_mul_ps(y2,y2));
    }
    if (i+4 <= n) {
      __m128 y1 = _mm_sub_ps(_mm_load_ps(X+i  ), M128(Y[i    ]));
      a1 = _mm_add_ps(a1, _mm_mul_ps(y1,y1));
      i+=4;
    }
    _mm_store_ss(&acc, sumup(_mm_add_ss(a1,a2)));

    for (; i<n; i++) {
      float t = X[i]-Y[i];
      acc += t*t;
    }
  }
  else
  {
    t = X[0] - Y[0];
    acc = t * t;

    for (i = 1; i < n; i++) {
      t = X[i] - Y[i];
      acc += t * t;
    }
  }
  return acc;
}

float dist2FLOATflex_Opt(const float *restrict X, int incX,
		     const float *restrict Y, int incY, int n)
{

  float t, acc = 0.0f;
  int i = 1, ix = incX, iy = incY;

  if (n) {
    t = X[0] - Y[0];
    acc = t * t;
  }
  for (i = 1; i < n; i++) {
    t = X[ix] - Y[iy];
    acc += t * t;
    ix += incX;
    iy += incY;
  }
  return acc;
}

float norm2FLOAT_Opt(const float *restrict X, int n)
{
#ifdef INTEL_PERF
  return sdot_(n,X,1,X,1);
#else
  float acc = 0.0f;
  int i;

  if (n) {
    acc = X[0] * X[0];
  }
  for (i = 1; i < n; i++)
    acc += X[i] * X[i];
  return acc;
#endif
}

void norm2FCOMPLEX_Opt(const float *restrict X, float *restrict Z, int n)
{

  int j=0,i=0;
  int n1 = 2*n;

  if(MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z)) 
  {
    int n2 = n1 & (~7UL);

	for(; i<n2; i+=8,j+=4)
	{
	  __m128 y1, y2;

	  y1 = _mm_mul_ps(_mm_load_ps(X + i    ), M128(X[i    ]));
      y2 = _mm_mul_ps(_mm_load_ps(X + i + 4), M128(X[i + 4]));
	  
      y1 = _mm_add_ps(_mm_shuffle_ps(y1, y2, _MM_SHUFFLE(2,0,2,0)), _mm_shuffle_ps(y1, y2, _MM_SHUFFLE(3,1,3,1)));

	  y1 = _mm_sqrt_ps(y1);	
      
	  _mm_store_ps(Z + j, y1);
	}
	
	if (i+4 <= n1)
	{
      __m128 y1;
	  
	  y1 = _mm_mul_ps(_mm_load_ps(X + i    ), M128(X[i    ]));
      y1 = _mm_add_ps(_mm_shuffle_ps(y1, y1, _MM_SHUFFLE(3,1,2,0)), _mm_shuffle_ps(y1, y1, _MM_SHUFFLE(2,0,3,1)));
	  y1 = _mm_sqrt_ps(y1);	
	  
	  _mm_store_ss(&(Z[j]) , y1);
      _mm_store_ss(&(Z[j+1]) , _mm_shuffle_ps(y1, y1, _MM_SHUFFLE(1,1,1,1)));
	  
	  i+=4;
      j+=2;
	}
  }

  for(; i<n1; i+=2,j++)
  {
    Z[j] = (float)sqrt(X[i]*X[i] + X[i+1]*X[i+1]);   	  
  }
}

void rad2FCOMPLEX_Opt(const float *restrict X, float *restrict Z, int n)
{

  int j=0,i=0;
  int n1 = 2*n;
  
  if(MMX2_ALIGN_CHECK((unsigned long)X | (unsigned long)Z)) 
  {
    int n2 = n1 & (~7UL);

	for(; i<n2; i+=8,j+=4)
	{
	  __m128 y1, y2;

	  y1 = _mm_mul_ps(_mm_load_ps(X + i    ), M128(X[i    ]));
      y2 = _mm_mul_ps(_mm_load_ps(X + i + 4), M128(X[i + 4]));
	  
      y1 = _mm_add_ps(_mm_shuffle_ps(y1, y2, _MM_SHUFFLE(2,0,2,0)), _mm_shuffle_ps(y1, y2, _MM_SHUFFLE(3,1,3,1)));

	  _mm_store_ps(Z + j, y1);
	}
  
	if (i+4 <= n1)
	{
      __m128 y1;
	  
	  y1 = _mm_mul_ps(_mm_load_ps(X + i    ), M128(X[i    ]));
      y1 = _mm_add_ps(_mm_shuffle_ps(y1, y1, _MM_SHUFFLE(3,1,2,0)), _mm_shuffle_ps(y1, y1, _MM_SHUFFLE(2,0,3,1)));
	 	  
	  _mm_store_ss(&(Z[j]) , y1);
      _mm_store_ss(&(Z[j+1]) , _mm_shuffle_ps(y1, y1, _MM_SHUFFLE(1,1,1,1)));
	  
	  i+=4;
      j+=2;
	}
  }

  for(; i<n1; i+=2,j++)
  {
    Z[j] = (X[i]*X[i] + X[i+1]*X[i+1]);   	  
  }
}

void setFLOAT_Opt(float a, float X[], int n)
{
  int i=0;

  if (MMX2_ALIGN_CHECK(X))
  {
    int n1 = n & ~3;
    __m128 ma;

    ma = _mm_load_ps1(&a);
    for (; i<n1; i+=4) {
      _mm_store_ps(X + i, ma);
    }
  }
  for (; i < n; i++)
    X[i] = a;
}

void setFLOATflex_Opt(float a, float X[], int incX, int n)
{
  int i, ix = 0;
  for (i = 0; i < n; i++) {
    X[ix] = a;
    ix += incX;
  }
}

void setINT_Opt(int a, int X[], int n)
{
  int i;
  for (i = 0; i < n; i++)
    X[i] = a;
}

void setINTflex_Opt(int a, int X[], int incX, int n)
{
  int i, ix = 0;
  for (i = 0; i < n; i++) {
    X[ix] = a;
    ix += incX;
  }
}

void smultFLOATip_Opt(float a, float * X, int n)
{
#ifdef INTEL_PERF
  sscal_(n,a,X,1);
#else
  int i;
  for (i = 0; i < n; i++)
    X[i] *= a;
#endif
}

void smultINTip_Opt(int a, int * X, int n)
{
  int i;
  for (i = 0; i < n; i++)
    X[i] *= a;
}

#endif /* #if defined _M_IX86 || (( defined __X86_64 || defined __i386__ && defined __OPTIMIZE__ && (GCC_VERSION>302001) ) */
