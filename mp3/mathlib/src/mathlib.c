/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: mathlib.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $
   Initial author:       W. Schildbach
   contents/description: Mathlib V2.0 Vector functions

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/*
  restrict is the proposed keyword to signal to a C compiler that a
  function parameter is not aliased to any other parameter in that function.

  To this date, only the SUN CC seems to support this. It is signalled by
  the presence of the __RESTRICT preprocessor variable.
*/

#ifdef __RESTRICT
#define restrict _Restrict
#else
#define restrict
#endif

#include "random.h"
#include "mathlib.h"
#include "cpuinfo.h"


#define MAKE_VERSION(m,n,b) (((m&0xff)<<24)|((n&0xff)<<16)|(b&0xffff))
#define BUILD_DATA(v) ((unsigned)(v & 0xffffffff))

#define VERSION MAKE_VERSION(0x1, 0x0, 0x0)
#define BUILD BUILD_DATA(0x04262000)


static const float ILOG2 = (float) 1.442695041f;
static const float ILOG10 = (float) 0.4342944819f;


/** $\vec Z=\vec X+\vec Y$. */
static float sumFLOAT_NoOpt(const float* X, int n)
{
  int i;
  float sum = 0; 

  for(i=0; i<n; i++)
  {
    sum +=X[i];
  }
  return sum;
}


static void addFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] + Y[i];
  }
  for (; i < n; i += 2) {
    float _a = X[i] + Y[i], _b = X[i + 1] + Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void addFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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
    float _a = X[ix] + Y[iy], _b = X[ix + incX] + Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

static void saddFLOAT_NoOpt(float b, const float *restrict X, float *restrict Z, int n)
{
  int i;
  for(i=0; i<n; i++)
  {
    Z[i] = X[i] + b;
  }
}

/** $\vec Z=\vec X-\vec Y$. */
static void subFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] - Y[i];
  }
  for (; i < n; i += 2) {
    float _a = X[i] - Y[i], _b = X[i + 1] - Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void subFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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
    float _a = X[ix] - Y[iy], _b = X[ix + incX] - Y[iy + incY];
    Z[iz] = _a;
    Z[iz + incZ] = _b;
    ix += 2 * incX;
    iy += 2 * incY;
    iz += 2 * incZ;
  }
}

/** $Z_i=X_i\cdot Y_i$. */
static void multFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] * Y[i];
  }
  for (; i < n; i += 2) {
    float _a = X[i] * Y[i], _b = X[i + 1] * Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void multFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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
static void divFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] / Y[i];
  }
  for (; i < n; i += 2) {
    float _a = X[i] / Y[i], _b = X[i + 1] / Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void divFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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
static void copyFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
{
	memcpy(Z,X,sizeof(float)*n);
}

static void copyFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
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
}

/** $\vec Z=a\vec X$. */
static void smulFLOAT_NoOpt(float a, const float * restrict X, float * restrict Z, int n)
{
  int i;
  if (n & 1) {
    Z[0] = (float) a *(X[0]);
    i = 1;
  } else
    i = 0;
  for (; i < n; i += 2) {
    float _a = (float) a * (X[i]), _b = (float) a * (X[i + 1]);
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void smulFLOATflex_NoOpt(float a, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
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
}

/* simple vector operations, type int */

/** $\vec Z=\vec X+\vec Y$. */
static void addINT_NoOpt(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] + Y[i];
  }
  for (; i < n; i += 2) {
    int _a = X[i] + Y[i], _b = X[i + 1] + Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void addINTflex_NoOpt(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void subINT_NoOpt(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] - Y[i];
  }
  for (; i < n; i += 2) {
    int _a = X[i] - Y[i], _b = X[i + 1] - Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void subINTflex_NoOpt(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void multINT_NoOpt(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] * Y[i];
  }
  for (; i < n; i += 2) {
    int _a = X[i] * Y[i], _b = X[i + 1] * Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void multINTflex_NoOpt(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void divINT_NoOpt(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
  int i;
  for (i = 0; i < (n & 1); i++) {
    Z[i] = X[i] / Y[i];
  }
  for (; i < n; i += 2) {
    int _a = X[i] / Y[i], _b = X[i + 1] / Y[i + 1];
    Z[i] = _a;
    Z[i + 1] = _b;
  }
}

static void divINTflex_NoOpt(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void smulINT_NoOpt(int a, const int * restrict X, int * restrict Z, int n)
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

static void smulINTflex_NoOpt(int a, const int * restrict X, int incX, int * restrict Z, int incZ, int n)
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
static void copyINT_NoOpt(const int * restrict X, int * restrict Z, int n)
{
	memcpy(Z,X,sizeof(int)*n);
}

static void copyINTflex_NoOpt(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
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

/* simple vector operations, type byte */
/** $\vec Z=\vec X$. */
/* this function might replace a memcpy, it should only be used if the 
   nice little Intel-Compiler uses it's own vec_memcpy instead, and you would like to 
   avoid the use of the Intel-library */

static void copyCHAR_NoOpt(const char * restrict X, char * restrict Z, int n)
{
  int i = 0;
  for (; i < n; i ++) {
    Z[i] = X[i];
  }
}

/**@name Vector comparison operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{ */

/* type float */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
static void minFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
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

static void minFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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

static float findminFLOAT_NoOpt(const float* X, int n)
{
  int i=0;
  float min=X[0];
  for(; i<n; i++)
  {
	 if(min>X[i])
	   min=X[i];
	/*min=(min<X[i]? min : X[i]);*/
  }
  return min;  
}

/** $Z_i=\max\left(X_i,Y_i\right)$ */
static void maxFLOAT_NoOpt(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
  int i;
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

static void maxFLOATflex_NoOpt(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
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


static float findmaxFLOAT_NoOpt(const float* X, int n)
{
  int i=0;
  float max=X[0];
  for(; i<n; i++)
  {
	  max=(max>X[i]? max : X[i]);
  }
  return max;  
}


/** $Z_i=\left\|X_i\right\|$ */
static void absFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
{
  int i;
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

static void absFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void limitFLOAT_NoOpt(float a, float b, const float * restrict X, float * restrict Z, int n)
{
  int i;
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

static void limitFLOATflex_NoOpt(float a, float b, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void signFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void signFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void minINT_NoOpt(const int * restrict X, const int * restrict Y, int * restrict Z, int n)
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

static void minINTflex_NoOpt(const int * restrict X, int incX, const int * restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void maxINT_NoOpt(const int * restrict X, const int * restrict Y, int * restrict Z, int n)
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

static void maxINTflex_NoOpt(const int * restrict X, int incX, const int * restrict Y, int incY, int * restrict Z, int incZ, int n)
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
static void absINT_NoOpt(const int * restrict X, int * restrict Z, int n)
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

static void absINTflex_NoOpt(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
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
static void limitINT_NoOpt(int a, int b, const int * restrict X, int * restrict Z, int n)
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

static void limitINTflex_NoOpt(int a, int b, const int * restrict X, int incX, int * restrict Z, int incZ, int n)
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
static void signINT_NoOpt(const int * restrict X, int * restrict Z, int n)
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

static void signINTflex_NoOpt(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
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
static void floorFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void floorFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void ceilFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void ceilFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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

/*!
  \brief  efficient implementation of b=ceil(log(a)/log(2))
  \return (int)ceil(log(a)/log(2))
  This function is useful to obtain number bits used to store a number
  with maximum range given.
*/
static int ceillog2_NoOpt( int a )   /*!< range value (make sure it's not negative) */
{
  int b = 0;

  a--;
  while (a>0) {
    b++;
    a = a >> 1;
  }

  return b;
}

/** round $Z_i$ to nearest integer. $Z_i=\left\lfloor X_i+\frac{1}{2}\right\rfloor$ */
static void nintFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void nintFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void truncFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void truncFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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


/** 
rounds float value to 16 bit float
*/
static void roundFLOAT2FLOAT16_NoOpt(const float *restrict X, float *restrict Y, int n)                           
{
  
  int i=0;
  for(; i<n; i++){
		
    if(X[i] > 32767.0f)
      Y[i] = 32767;
    else if(X[i] < -32768.0f)
      Y[i] = -32768;
    else if (X[i] > 0.0f)
      Y[i] = (signed short)(X[i] + 0.5f);
    else if(X[i] <= 0.0f)
      Y[i] = (signed short)(X[i] - 0.5f);

    Y[i] = (float)Y[i];

  }
}

static void roundFLOAT2INT_NoOpt(const float *restrict X, int *restrict Y, int n)
{

  int i=0;
  for(; i<n; i++)
  {
    if (X[i] > 0.0f)
      Y[i] = (int) (X[i] + 0.5f);
    else if (X[i] <= 0.0f)
      Y[i] = (int) (X[i] - 0.5f);
  }
}

static void roundFLOAT2SHORT_NoOpt(const float *restrict X, signed short *restrict Y, int n)
{

  int i=0;
  for(; i<n; i++)
  {
    if (X[i] > 32767.0f)
      Y[i] = 32767;
    else if (X[i] < -32768.0f)
      Y[i] = -32768;
    else if (X[i] > 0.0f)
      Y[i] = (signed short) (X[i] + 0.5f);
    else if (X[i] <= 0.0f)
      Y[i] = (signed short) (X[i] - 0.5f);
  }
}



/*@} */

/**@name Transcendent vector operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{ */

/** $Z_i=\sin\left(X_i\right)$. */
static void sinFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void sinFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void cosFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void cosFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void expFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void expFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void spowFLOAT_NoOpt(float a, const float * restrict X, float * restrict Z, int n)
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

static void spowFLOATflex_NoOpt(float a, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void logFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void logFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void log2FLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void log2FLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void log10FLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void log10FLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void alogbFLOAT_NoOpt(float a, float b, const float * restrict X, float * restrict Z, int n)
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

static void alogbFLOATflex_NoOpt(float a, float b, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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
static void sqrtFLOAT_NoOpt(const float * restrict X, float * restrict Z, int n)
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

static void sqrtFLOATflex_NoOpt(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
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

static float dotFLOAT_NoOpt(const float * restrict X, const float * restrict Y, int n)
{
  float acc=0.0f;
  int i;

  if (n) {
    acc = X[0]*Y[0];
  }

  for (i=1; i<n; i++) acc += X[i]*Y[i];
  return acc;
}

static float dotFLOATflex_NoOpt(const float * restrict X, int incX, 
							const float * restrict Y, int incY, int n)
{
  float acc=0.0f;
  int i, iX=incX, iY=incY;

  if (n) {
    acc = X[0]*Y[0];
  }

  for (i=1; i<n; i++) 
  {
	  acc += X[iX]*Y[iY];
	  iX+=incX; iY+=incY;
  }
  return acc;
}

static float dist2FLOAT_NoOpt(const float * restrict X, const float * restrict Y, int n)
{
  float t,acc;
  int i;

  t = X[0]-Y[0];
  acc = t*t;

  for (i=1; i<n; i++) {
    t = X[i]-Y[i];
    acc += t*t;
  }
  return acc;
}

static float dist2FLOATflex_NoOpt(const float * restrict X, int incX,
                     const float * restrict Y, int incY, int n)
{
  float t,acc=0.0f;
  int i=1,ix=incX,iy=incY;

  if (n) {
    t = X[0]-Y[0];
    acc = t*t;
  }

  for (i=1; i<n; i++) {
    t = X[ix]-Y[iy];
    acc += t*t;
    ix+=incX; iy+=incY;
  }
  return acc;
}

static float norm2FLOAT_NoOpt(const float * restrict X, int n)
{
  float acc=0.0f;
  int i;

  if (n) {
    acc = X[0]*X[0];
  }

  for (i=1; i<n; i++) acc += X[i]*X[i];
  return acc;
}

static void norm2FCOMPLEX_NoOpt(const float *restrict X, float *restrict Z, int n)
{
  int i=0, j=0;

  for(; j<n; i+=2,j++)
  {
    Z[j] = (float)sqrt(X[i]*X[i] + X[i+1]*X[i+1]);   	  
  }
}

static void rad2FCOMPLEX_NoOpt(const float * restrict X, float * restrict Z, int n)
{
  int i=0;
  if (n&1) {
    Z[0] = X[0]*X[0]+X[1]*X[1];
    i+=1;
  }
  if (n&2) {
    float _a = X[2*i]*X[2*i]+X[2*i+1]*X[2*i+1];
    float _b = X[2*i+2]*X[2*i+2]+X[2*i+3]*X[2*i+3];
    Z[i] = _a; Z[i+1] = _b;
    i+=2;
  }
  for (; i<n; i+=4) {
    float _a = X[2*i  ]*X[2*i  ]+X[2*i+1]*X[2*i+1];
    float _b = X[2*i+2]*X[2*i+2]+X[2*i+3]*X[2*i+3];
    float _c = X[2*i+4]*X[2*i+4]+X[2*i+5]*X[2*i+5];
    float _d = X[2*i+6]*X[2*i+6]+X[2*i+7]*X[2*i+7];
    Z[i] = _a; Z[i+1] = _b; Z[i+2] = _c; Z[i+3] = _d;
  }
}

static void randFLOAT_NoOpt(float Z[], int n)
{
  double d[10];
  int i,j;

  for (i=0; i+10<n; i+=10) {
    zufall(10,d);
    for (j=0; j<10; j++) Z[i+j] = (float)d[j];
  }
  n-=i;
  zufall(n,d);
  for (j=0; j<n; j++) Z[i+j] = (float)d[j];
}

static void randDOUBLE_NoOpt(double Z[], int n)
{
  zufall(n,Z);
}

static void randseed_NoOpt(int seed)
{
  seed %= 31328;
  zufalli(seed);
}

static void randsave_NoOpt(double mem[])
{
  zufallsv(mem);
}

static void randrestore_NoOpt(double mem[])
{
  zufallrs(mem);
}

static void quantFLOATtoUINT_NoOpt(float a, float b, const float * restrict X, unsigned int * restrict Z, int n)
{
  int i=0;
  if (n&1) {
    Z[0] = (unsigned int)(a+b*X[i]);
    i++;
  }
  if (n&2) {
    int _a,_b;
    _a = (unsigned int)(a+b*X[i]); _b = (unsigned int)(a+b*X[i+1]);
    Z[i]=_a; Z[i+1]=_b;
    i+=2;
  }
  for (; i < n; i += 4) {
    float t1 = a+b*X[i];
    float t2 = a+b*X[i + 1];
    float t3 = a+b*X[i + 2];
    float t4 = a+b*X[i + 3];
    Z[i    ] = (unsigned int) (t1);
    Z[i + 1] = (unsigned int) (t2);
    Z[i + 2] = (unsigned int) (t3);
    Z[i + 3] = (unsigned int) (t4);
  }
}

static void setFLOAT_NoOpt(float a, float X[], int n)
{
  int i;
  for (i=0; i<n; i++) X[i]=a;
}

static void setFLOATflex_NoOpt(float a, float X[], int incX, int n)
{
  int i,ix=0;
  for (i=0; i<n; i++) {
    X[ix]=a; ix += incX;
  }
}

static void setINT_NoOpt(int a, int X[], int n)
{
  int i;
  for (i=0; i<n; i++) X[i]=a;
}

static void setINTflex_NoOpt(int a, int X[], int incX, int n)
{
  int i,ix=0;
  for (i=0; i<n; i++) {
    X[ix]=a; ix += incX;
  }
}

static void smultFLOATip_NoOpt(float a, float *X, int n)
{
  int i;
  for (i=0; i<n; i++) X[i] *= a;
}

static void smultINTip_NoOpt(int a, int *X, int n)
{
  int i;
  for (i=0; i<n; i++) X[i] *= a;
}

#if 0 /* currently unused */
static unsigned int GetBuildDateMath(){
  return (unsigned int)BUILD ;
}
#endif

#if 0 /* currently unused */
static unsigned int GetVersionMath(){
  return (unsigned int)VERSION ;
}
#endif

/* default definition of all mathlib pointer */
static float (*sumFLOAT_Ptr)(const float* X, int n)
            = sumFLOAT_NoOpt;

static void (*addFLOAT_Ptr)(const float *restrict X, const float *restrict Y, float *restrict Z, int n)	
			= addFLOAT_NoOpt;

static void (*addFLOATflex_Ptr)(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n) 
			= addFLOATflex_NoOpt;

static void (*saddFLOAT_Ptr)(float b, const float *restrict X, float *restrict Z, int n)
            = saddFLOAT_NoOpt;

static void (*subFLOAT_Ptr)(const float X[], const float Y[], float Z[], int n)	
			= subFLOAT_NoOpt;

static void (*subFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) 
			= subFLOATflex_NoOpt;

static void (*multFLOAT_Ptr)(const float X[], const float Y[], float Z[], int n) 
			= multFLOAT_NoOpt;

static void (*multFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) 
			= multFLOATflex_NoOpt;

static void (*divFLOAT_Ptr)(const float X[], const float Y[], float Z[], int n) 
			= divFLOAT_NoOpt;

static void (*divFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) 
			= divFLOATflex_NoOpt;

static void (*copyFLOAT_Ptr)(const float X[], float Z[], int n) 
			= copyFLOAT_NoOpt;

static void (*copyFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n) 
			= copyFLOATflex_NoOpt;

static void (*smulFLOAT_Ptr)(float a, const float X[], float Z[], int n) 
			= smulFLOAT_NoOpt;

static void (*smulFLOATflex_Ptr)(float a, const float X[], int incX, float Z[], int incZ, int n) 
			= smulFLOATflex_NoOpt;

static void (*setFLOAT_Ptr)(float a, float X[], int n) 
			= setFLOAT_NoOpt;

static void (*setFLOATflex_Ptr)(float a, float X[], int incX, int n) 
			= setFLOATflex_NoOpt;

static float (*dotFLOAT_Ptr)(const float X[], const float Y[], int n) 
			= dotFLOAT_NoOpt;

static float (*dotFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, int n) 
			= dotFLOATflex_NoOpt;

static float (*dist2FLOAT_Ptr)(const float X[], const float Y[], int n) 
			= dist2FLOAT_NoOpt;

static float (*dist2FLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, int n) 
			= dist2FLOATflex_NoOpt;
	
static float (*norm2FLOAT_Ptr)(const float X[], int n) 
			= norm2FLOAT_NoOpt;

static void (*norm2FCOMPLEX_Ptr)(const float *restrict X, float *restrict Z, int n)
            = norm2FCOMPLEX_NoOpt;

static void (*rad2FCOMPLEX_Ptr)(const float X[], float Z[], int n) 
			= rad2FCOMPLEX_NoOpt;

static void (*addINT_Ptr)(const int X[], int const Y[], int Z[], int n) 
			= addINT_NoOpt;

static void (*addINTflex_Ptr)(const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n)
			= addINTflex_NoOpt;

static void (*subINT_Ptr)(const int X[], int const Y[], int Z[], int n) 
			= subINT_NoOpt;

static void (*subINTflex_Ptr)(const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) 
			= subINTflex_NoOpt;

static void (*multINT_Ptr)(const int X[],int const Y[], int Z[], int n) 
			= multINT_NoOpt;

static void (*multINTflex_Ptr)(const int X[],int incX, int const Y[], int incY, int Z[], int incZ, int n) 
			= multINTflex_NoOpt;

static void (*divINT_Ptr)(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
			= divINT_NoOpt;

static void (*divINTflex_Ptr)(const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) 
			= divINTflex_NoOpt;

static void (*smulINT_Ptr)(int a, const int X[], int Z[], int n) 
			= smulINT_NoOpt;

static void (*smulINTflex_Ptr)(int a, const int X[], int incX, int Z[], int incZ, int n) 
			= smulINTflex_NoOpt;

static void (*smultFLOATip_Ptr)(float a, float *X, int n)
			= smultFLOATip_NoOpt;

static void (*smultINTip_Ptr)(int a, int *X, int n)	
			= smultINTip_NoOpt;

static void (*copyINT_Ptr)(const int X[], int Z[], int n) 
			= copyINT_NoOpt;

static void (*copyINTflex_Ptr)(const int X[], int incX, int Z[], int incZ, int n) 
			= copyINTflex_NoOpt;

static void (*setINT_Ptr)(int a, int X[], int n)
			= setINT_NoOpt;

static void (*setINTflex_Ptr)(int a, int X[], int incX, int n)
			= setINTflex_NoOpt;

static void (*copyCHAR_Ptr)(const char X[], char Z[], int n)
			= copyCHAR_NoOpt;

static void (*minFLOAT_Ptr)(const float X[], const float Y[], float Z[], int n) 
			= minFLOAT_NoOpt;

static void (*minFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n)
			= minFLOATflex_NoOpt;

static float (*findminFLOAT_Ptr)(const float* X, int n)
            = findminFLOAT_NoOpt;

static void (*maxFLOAT_Ptr)(const float X[], const float Y[], float Z[], int n)
			= maxFLOAT_NoOpt;

static void (*maxFLOATflex_Ptr)(const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n)
			= maxFLOATflex_NoOpt;

static float (*findmaxFLOAT_Ptr)(const float* X, int n)
            = findmaxFLOAT_NoOpt;

static void (*absFLOAT_Ptr)(const float X[], float Z[], int n)
			= absFLOAT_NoOpt;

static void (*absFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= absFLOATflex_NoOpt;

static void (*limitFLOAT_Ptr)(float a, float b, const float X[], float Z[], int n)
			= limitFLOAT_NoOpt;

static void (*limitFLOATflex_Ptr)(float a, float b, const float X[], int incX, float Z[], int incZ, int n)
			= limitFLOATflex_NoOpt;

static void (*signFLOAT_Ptr)(const float X[], float Z[], int n)
			= signFLOAT_NoOpt;

static void (*signFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= signFLOATflex_NoOpt;

static void (*minINT_Ptr)(const int X[], const int Y[], int Z[], int n)
			= minINT_NoOpt;

static void (*minINTflex_Ptr)  (const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n)
			= minINTflex_NoOpt;

static void (*maxINT_Ptr)(const int X[], const int Y[], int Z[], int n)
			= maxINT_NoOpt;

static void (*maxINTflex_Ptr)(const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n)
			= maxINTflex_NoOpt;

static void (*absINT_Ptr)(const int X[], int Z[], int n)
			= absINT_NoOpt;

static void (*absINTflex_Ptr)(const int X[], int incX, int Z[], int incZ, int n)
			= absINTflex_NoOpt;

static void (*limitINT_Ptr)(int a, int b, const int X[], int Z[], int n)
			= limitINT_NoOpt;

static void (*limitINTflex_Ptr)(int a, int b, const int X[],int incX,  int Z[], int incZ, int n)
			= limitINTflex_NoOpt;

static void (*signINT_Ptr)(const int * restrict X, int * restrict Z, int n)
			= signINT_NoOpt;

static void (*signINTflex_Ptr)(const int X[], int incX, int Z[], int incZ, int n)
			= signINTflex_NoOpt;

static void (*floorFLOAT_Ptr)(const float X[], float Z[], int n)
			= floorFLOAT_NoOpt;

static void (*floorFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= floorFLOATflex_NoOpt;

static void (*ceilFLOAT_Ptr)(const float X[], float Z[], int n)
			= ceilFLOAT_NoOpt;

static void (*ceilFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= ceilFLOATflex_NoOpt;

static int (*ceillog2_Ptr)( int a )
			= ceillog2_NoOpt;

static void (*nintFLOAT_Ptr)(const float X[], float Z[], int n)
			= nintFLOAT_NoOpt;

static void (*nintFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= nintFLOATflex_NoOpt;

static void (*truncFLOAT_Ptr)(const float X[], float Z[], int n)
			= truncFLOAT_NoOpt;

static void (*truncFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= truncFLOATflex_NoOpt;

static void (*roundFLOAT2FLOAT16_Ptr)(const float A[], float B[], int n)
			= roundFLOAT2FLOAT16_NoOpt;

static void (*roundFLOAT2INT_Ptr)(const float A[], int B[], int n)
			= roundFLOAT2INT_NoOpt;

static void (*roundFLOAT2SHORT_Ptr)(const float A[], signed short B[], int n)
			= roundFLOAT2SHORT_NoOpt;

static void (*sinFLOAT_Ptr)(const float X[], float Z[], int n)
			= sinFLOAT_NoOpt;

static void (*sinFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= sinFLOATflex_NoOpt;

static void (*cosFLOAT_Ptr)(const float X[], float Z[], int n)
			= cosFLOAT_NoOpt;

static void (*cosFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= cosFLOATflex_NoOpt;

static void (*expFLOAT_Ptr)(const float X[], float Z[], int n)
			= expFLOAT_NoOpt;

static void (*expFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= expFLOATflex_NoOpt;

static void (*spowFLOAT_Ptr)(float a, const float X[], float Z[], int n)
			= spowFLOAT_NoOpt;

static void (*spowFLOATflex_Ptr)(float a, const float X[], int incX, float Z[], int incZ, int n)
			= spowFLOATflex_NoOpt;

static void (*logFLOAT_Ptr)(const float X[], float Z[], int n)
			= logFLOAT_NoOpt;

static void (*logFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= logFLOATflex_NoOpt;

static void (*log2FLOAT_Ptr)(const float X[], float Z[], int n)
			= log2FLOAT_NoOpt;

static void (*log2FLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= log2FLOATflex_NoOpt;

static void (*log10FLOAT_Ptr)(const float X[], float Z[], int n)
			= log10FLOAT_NoOpt;

static void (*log10FLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= log10FLOATflex_NoOpt;

static void (*alogbFLOAT_Ptr)(float a, float b, const float X[], float Z[], int n)
			= alogbFLOAT_NoOpt;

static void (*alogbFLOATflex_Ptr)(float a, float b, const float X[], int incX, float Z[], int incZ, int n)
			= alogbFLOATflex_NoOpt;

static void (*sqrtFLOAT_Ptr)(const float X[], float Z[], int n)	
			= sqrtFLOAT_NoOpt;

static void (*sqrtFLOATflex_Ptr)(const float X[], int incX, float Z[], int incZ, int n)
			= sqrtFLOATflex_NoOpt;

static void (*randFLOAT_Ptr)(float Z[], int n)	
			= randFLOAT_NoOpt;

static void (*randDOUBLE_Ptr)(double Z[], int n)
			= randDOUBLE_NoOpt;

static void (*randseed_Ptr)(int seed)
			= randseed_NoOpt;

static void (*randsave_Ptr)(double mem[])
			= randsave_NoOpt;

static void (*randrestore_Ptr)(double mem[])
			= randrestore_NoOpt;

static void (*quantFLOATtoUINT_Ptr)(float a, float b, const float * restrict X, unsigned int * restrict Z, int n)
			= quantFLOATtoUINT_NoOpt;

/* detects CPU features */
void InitMathOpt(void)
{

/************* MATHLIB optimisation **************/
#ifdef __GNUC__
#define GCC_VERSION (   __GNUC__             * 10000 \
                      + __GNUC_MINOR__       *   100 \
                      + __GNUC_PATCHLEVEL__  *     1)
#endif
#if defined _M_IX86 || defined _M_X64 || ((defined __x86_64 ||  defined __i386__)  && defined __OPTIMIZE__ && ( (GCC_VERSION>30201) || defined __INTEL_COMPILER )  && !defined __CYGWIN__ )
#ifdef _MSC_VER
#pragma message(__FILE__": using SSE intrinsics")
#endif
/* change pointer to optimised version */
   if (GetCPUInfo(HAS_CPU_SSE)) 
   {

#ifdef DEBUG
     fprintf(stderr,"\n%s: CPU supports SSE", __FILE__);
#endif
	sumFLOAT_Ptr        = sumFLOAT_Opt;
	addFLOAT_Ptr		= addFLOAT_Opt;
	addFLOATflex_Ptr	= addFLOATflex_Opt;
	saddFLOAT_Ptr       = saddFLOAT_Opt;
	subFLOAT_Ptr		= subFLOAT_Opt;
	subFLOATflex_Ptr	= subFLOATflex_Opt;
	multFLOAT_Ptr		= multFLOAT_Opt;
	multFLOATflex_Ptr	= multFLOATflex_Opt;
	divFLOAT_Ptr		= divFLOAT_NoOpt;
	divFLOATflex_Ptr	= divFLOATflex_Opt;
	copyFLOAT_Ptr		= copyFLOAT_Opt;
	copyFLOATflex_Ptr	= copyFLOATflex_Opt;
	smulFLOAT_Ptr		= smulFLOAT_Opt;
	smulFLOATflex_Ptr	= smulFLOATflex_Opt;
	addINT_Ptr	        = addINT_Opt;
	addINTflex_Ptr		= addINTflex_Opt;
	subINT_Ptr		    = subINT_Opt;
	subINTflex_Ptr		= subINTflex_Opt;
	multINT_Ptr		    = multINT_Opt;
	multINTflex_Ptr		= multINTflex_Opt;
	divINT_Ptr		    = divINT_Opt;
	divINTflex_Ptr		= divINTflex_Opt;
	smulINT_Ptr		    = smulINT_Opt;
	smulINTflex_Ptr		= smulINTflex_Opt;
	copyINT_Ptr		    = copyINT_Opt;
	copyINTflex_Ptr		= copyINTflex_Opt;
	minFLOAT_Ptr		= minFLOAT_Opt;
	minFLOATflex_Ptr	= minFLOATflex_Opt;
	findminFLOAT_Ptr    = findminFLOAT_Opt;
	maxFLOAT_Ptr		= maxFLOAT_Opt;
	maxFLOATflex_Ptr	= maxFLOATflex_Opt;
	findmaxFLOAT_Ptr    = findmaxFLOAT_Opt;
 	absFLOAT_Ptr		= absFLOAT_Opt;
	absFLOATflex_Ptr	= absFLOATflex_Opt;
	limitFLOAT_Ptr		= limitFLOAT_Opt;
	limitFLOATflex_Ptr	= limitFLOATflex_Opt;
	signFLOAT_Ptr		= signFLOAT_Opt;
	signFLOATflex_Ptr	= signFLOATflex_Opt;
	minINT_Ptr		    = minINT_Opt;
	minINTflex_Ptr		= minINTflex_Opt;
	maxINT_Ptr		    = maxINT_Opt;
	maxINTflex_Ptr		= maxINTflex_Opt;
	absINT_Ptr		    = absINT_Opt;
	absINTflex_Ptr		= absINTflex_Opt;
	limitINT_Ptr		= limitINT_Opt;
	limitINTflex_Ptr	= limitINTflex_Opt;
	signINT_Ptr		    = signINT_Opt;
	signINTflex_Ptr		= signINTflex_Opt;
	floorFLOAT_Ptr		= floorFLOAT_Opt;
	floorFLOATflex_Ptr	= floorFLOATflex_Opt;
	ceilFLOAT_Ptr		= ceilFLOAT_Opt;
	ceilFLOATflex_Ptr	= ceilFLOATflex_Opt;
	nintFLOAT_Ptr		= nintFLOAT_Opt;
	nintFLOATflex_Ptr	= nintFLOATflex_Opt;
	truncFLOAT_Ptr		= truncFLOAT_Opt;
	truncFLOATflex_Ptr	= truncFLOATflex_Opt;
	roundFLOAT2FLOAT16_Ptr  = roundFLOAT2FLOAT16_Opt;
    sinFLOAT_Ptr		= sinFLOAT_Opt;
	sinFLOATflex_Ptr	= sinFLOATflex_Opt;
	cosFLOAT_Ptr		= cosFLOAT_Opt;
	cosFLOATflex_Ptr	= cosFLOATflex_Opt;
	expFLOAT_Ptr		= expFLOAT_Opt;
	expFLOATflex_Ptr	= expFLOATflex_Opt;
	spowFLOAT_Ptr		= spowFLOAT_Opt;
	spowFLOATflex_Ptr	= spowFLOATflex_Opt;
	logFLOAT_Ptr		= logFLOAT_Opt;
	logFLOATflex_Ptr	= logFLOATflex_Opt;
	log2FLOAT_Ptr		= log2FLOAT_Opt;
	log2FLOATflex_Ptr	= log2FLOATflex_Opt;
	log10FLOAT_Ptr		= log10FLOAT_Opt;
	log10FLOATflex_Ptr	= log10FLOATflex_Opt;
	alogbFLOAT_Ptr		= alogbFLOAT_Opt;
	alogbFLOATflex_Ptr	= alogbFLOATflex_Opt;
	sqrtFLOAT_Ptr		= sqrtFLOAT_Opt;
	sqrtFLOATflex_Ptr	= sqrtFLOATflex_Opt;
	dotFLOAT_Ptr		= dotFLOAT_Opt;
	dist2FLOAT_Ptr		= dist2FLOAT_Opt;
	dist2FLOATflex_Ptr	= dist2FLOATflex_Opt;	
	norm2FLOAT_Ptr		= norm2FLOAT_Opt;
    norm2FCOMPLEX_Ptr   = norm2FCOMPLEX_Opt;
 	rad2FCOMPLEX_Ptr	= rad2FCOMPLEX_Opt;
	setFLOAT_Ptr		= setFLOAT_Opt;
	setFLOATflex_Ptr	= setFLOATflex_Opt;
	setINT_Ptr	        = setINT_Opt;
	setINTflex_Ptr		= setINTflex_Opt;
	smultFLOATip_Ptr	= smultFLOATip_Opt;
	smultINTip_Ptr		= smultINTip_Opt;
   }

#endif
   
/*change pointer to SSE2 optimised version*/
#if defined _M_IX86 || defined _M_X64  || ( (defined __x86_64 || defined __i386__) && defined __OPTIMIZE__ && ((GCC_VERSION>30402) || defined __INTEL_COMPILER ) && !defined __CYGWIN__ )
#ifdef _MSC_VER
#pragma message(__FILE__": using SSE2 intrinsics")
#endif

if (GetCPUInfo(HAS_CPU_SSE2)){
  roundFLOAT2FLOAT16_Ptr = roundFLOAT2FLOAT16_SSE2;
  roundFLOAT2INT_Ptr     = roundFLOAT2INT_SSE2;
  roundFLOAT2SHORT_Ptr   = roundFLOAT2SHORT_SSE2;
  setINT_Ptr	         = setINT_SSE2;
}

#endif

#if 0
#if defined _M_IX86 || ( defined __i386__ && !defined __CYGWIN__  )
   if (GetCPUInfo(HAS_CPU_FPU)) {
#ifdef DEBUG
     fprintf(stderr,"\n%s: CPU supports FPU", __FILE__);
#endif
     quantFLOATtoUINT_Ptr    = quantFLOATtoUINT_Opt;
   }
#endif
#endif

#if defined  __ALTIVEC__ && defined __OPTIMIZE__
  if (GetCPUInfo( HAS_CPU_ALTIVEC)){
    roundFLOAT2FLOAT16_Ptr = roundFLOAT2FLOAT16_ALTIVEC;     
}
#endif
}

/* standard mathlib call */


float sumFLOAT(const float* X, int n)
{
  return(sumFLOAT_Ptr(X, n));
}

void addFLOAT(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
	addFLOAT_Ptr(X, Y, Z, n);
}

void addFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	addFLOATflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void saddFLOAT(float b, const float *restrict X, float *restrict Z, int n)
{
    saddFLOAT_Ptr(b, X, Z, n);
}

void subFLOAT(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
	subFLOAT_Ptr(X, Y, Z, n);
}

void subFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	subFLOATflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void multFLOAT(const float * X, const float * Y, float * restrict Z, int n)
{
	multFLOAT_Ptr(X, Y, Z, n);
}

void multFLOATflex(const float * restrict X, int incX, const float *restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	multFLOATflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void divFLOAT(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
	divFLOAT_Ptr(X, Y, Z, n);
}

void divFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	divFLOATflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void copyFLOAT(const float * restrict X, float * restrict Z, int n)
{
	copyFLOAT_Ptr(X, Z, n);
}

void copyFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	copyFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void smulFLOAT(float a, const float * restrict X, float * restrict Z, int n)
{
	smulFLOAT_Ptr(a, X, Z, n);
}

void smulFLOATflex(float a, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	smulFLOATflex_Ptr(a, X, incX, Z, incZ, n);
}

void addINT(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
	addINT_Ptr(X, Y, Z, n);
}

void addINTflex(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	addINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void subINT(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
	subINT_Ptr(X, Y, Z, n);
}

void subINTflex(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	subINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void multINT(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
	multINT_Ptr(X, Y, Z, n);
}

void multINTflex(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	multINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void divINT(const int * restrict X, int const *restrict Y, int * restrict Z, int n)
{
	divINT_Ptr(X, Y, Z, n);
}

void divINTflex(const int * restrict X, int incX, int const *restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	divINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void smulINT(int a, const int * restrict X, int * restrict Z, int n)
{
	smulINT_Ptr(a, X, Z, n);
}

void smulINTflex(int a, const int * restrict X, int incX, int * restrict Z, int incZ, int n)
{
	smulINTflex_Ptr(a, X, incX, Z, incZ, n);
}

void copyINT(const int * restrict X, int * restrict Z, int n)
{
	copyINT_Ptr(X, Z, n);
}

void copyINTflex(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
{
	copyINTflex_Ptr(X, incX, Z, incZ, n);
}

void copyCHAR(const char * restrict X, char * restrict Z, int n)
{
	copyCHAR_Ptr(X, Z, n);
}

void minFLOAT(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
	minFLOAT_Ptr(X, Y, Z, n);
}

void minFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	minFLOATflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

float findminFLOAT(const float* X, int n)
{
    return(findminFLOAT_Ptr(X, n));   
}

void maxFLOAT(const float * restrict X, const float * restrict Y, float * restrict Z, int n)
{
	maxFLOAT_Ptr(X, Y, Z, n);
}

void maxFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, float * restrict Z, int incZ, int n)
{
	maxFLOATflex_Ptr(X,incX, Y, incY, Z, incZ, n);
}

float findmaxFLOAT(const float *X, int n)
{
    return(findmaxFLOAT_Ptr(X, n));
}

void absFLOAT(const float *restrict X, float *restrict Z, int n)
{
	absFLOAT_Ptr(X, Z, n);
}

void absFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	absFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void limitFLOAT(float a, float b, const float * restrict X, float * restrict Z, int n)
{
	limitFLOAT_Ptr(a, b, X, Z, n);
}

void limitFLOATflex(float a, float b, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	limitFLOATflex_Ptr(a, b, X, incX, Z, incZ, n);
}

void signFLOAT(const float * restrict X, float * restrict Z, int n)
{
	signFLOAT_Ptr(X, Z, n);
}

void signFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	signFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void minINT(const int * restrict X, const int * restrict Y, int * restrict Z, int n)
{
	minINT_Ptr(X, Y, Z, n);
}

void minINTflex(const int * restrict X, int incX, const int * restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	minINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void maxINT(const int * restrict X,  const int * restrict Y, int * restrict Z, int n)
{
	maxINT_Ptr( X, Y, Z, n);
}

void maxINTflex(const int * restrict X, int incX, const int * restrict Y, int incY, int * restrict Z, int incZ, int n)
{
	maxINTflex_Ptr(X, incX, Y, incY, Z, incZ, n);
}

void absINT(const int * restrict X, int * restrict Z, int n)
{
	absINT_Ptr(X, Z, n);
}

void absINTflex(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
{
	absINTflex_Ptr(X, incX, Z, incZ, n);
}

void limitINT(int a, int b, const int * restrict X, int * restrict Z, int n)
{
	limitINT_Ptr(a, b, X, Z, n);
}

void limitINTflex(int a, int b, const int * restrict X, int incX, int * restrict Z, int incZ, int n)
{
	limitINTflex_Ptr(a, b, X, incX, Z, incZ, n);
}

void signINT(const int * restrict X, int * restrict Z, int n)
{
	signINT_Ptr(X, Z, n);
}

void signINTflex(const int * restrict X, int incX, int * restrict Z, int incZ, int n)
{
	signINTflex_Ptr(X, incX, Z, incZ, n);
}

void floorFLOAT(const float * restrict X, float * restrict Z, int n)
{
	floorFLOAT_Ptr(X, Z, n);
}

void floorFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	floorFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void ceilFLOAT(const float * restrict X, float * restrict Z, int n)
{
	ceilFLOAT_Ptr(X, Z, n);
}

void ceilFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	ceilFLOATflex_Ptr(X, incX, Z, incZ, n);
}

int ceillog2( int a )   /*!< range value (make sure it's not negative) */
{
	return (ceillog2_Ptr( a ));
}

void nintFLOAT(const float * restrict X, float * restrict Z, int n)
{
	nintFLOAT_Ptr(X, Z, n);
}

void nintFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	nintFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void truncFLOAT(const float * restrict X, float * restrict Z, int n)
{
	truncFLOAT_Ptr(X, Z, n);
}

void truncFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	truncFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void roundFLOAT2FLOAT16(const float *restrict A, float *restrict B, int n)                           
{
   roundFLOAT2FLOAT16_Ptr(A, B, n);                           
}

void roundFLOAT2INT(const float *restrict A, int *restrict B, int n)                           
{
   roundFLOAT2INT_Ptr(A, B, n);                           
}

void roundFLOAT2SHORT(const float *restrict A, signed short *restrict B, int n)                           
{
   roundFLOAT2SHORT_Ptr(A, B, n);                           
}

void sinFLOAT(const float * restrict X, float * restrict Z, int n)
{
	sinFLOAT_Ptr(X, Z, n);
}

void sinFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	sinFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void cosFLOAT(const float * restrict X, float * restrict Z, int n)
{
	cosFLOAT_Ptr(X, Z, n);
}

void cosFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	cosFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void expFLOAT(const float * restrict X, float * restrict Z, int n)
{
	expFLOAT_Ptr(X, Z, n);
}

void expFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	expFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void spowFLOAT(float a, const float * restrict X, float * restrict Z, int n)
{
	spowFLOAT_Ptr(a, X, Z, n);
}

void spowFLOATflex(float a, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	spowFLOATflex_Ptr(a, X, incX, Z, incZ, n);
}

void logFLOAT(const float * restrict X, float * restrict Z, int n)
{
	logFLOAT_Ptr(X, Z, n);
}

void logFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	logFLOATflex_Ptr(X, incX, Z, incZ, n);
}

void log2FLOAT(const float * restrict X, float * restrict Z, int n)
{
	log2FLOAT_Ptr(X, Z, n);
}

void log2FLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	log2FLOATflex_Ptr(X, incX, Z, incZ, n);
}

void log10FLOAT(const float * restrict X, float * restrict Z, int n)
{
	log10FLOAT_Ptr(X, Z, n);
}

void log10FLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	log10FLOATflex_Ptr(X, incX, Z, incZ, n);
}

void alogbFLOAT(float a, float b, const float * restrict X, float * restrict Z, int n)
{
	alogbFLOAT_Ptr(a, b, X, Z, n);
}

void alogbFLOATflex(float a, float b, const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	alogbFLOATflex_Ptr(a, b, X, incX, Z, incZ, n);
}

void sqrtFLOAT(const float * restrict X, float * restrict Z, int n)
{
	sqrtFLOAT_Ptr(X, Z, n);
}

void sqrtFLOATflex(const float * restrict X, int incX, float * restrict Z, int incZ, int n)
{
	sqrtFLOATflex_Ptr(X, incX, Z, incZ, n);
}

float dotFLOAT(const float * restrict X, const float * restrict Y, int n)
{
	return (dotFLOAT_Ptr(X, Y, n));
}

float dotFLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, int n)
{
	return(dotFLOATflex_Ptr(X, incX, Y, incY, n));
}

float dist2FLOAT(const float * restrict X, const float * restrict Y, int n)
{
	return(dist2FLOAT_Ptr(X, Y, n));
}

float dist2FLOATflex(const float * restrict X, int incX, const float * restrict Y, int incY, int n)
{
	return(dist2FLOATflex_Ptr(X, incX, Y, incY, n));
}

float norm2FLOAT(const float * restrict X, int n)
{
	return(norm2FLOAT_Ptr(X, n));
}

void norm2FCOMPLEX(const float *restrict X, float *restrict Z, int n)
{
    norm2FCOMPLEX_Ptr(X, Z, n);
}

void rad2FCOMPLEX(const float * restrict X, float * restrict Z, int n)
{
	rad2FCOMPLEX_Ptr(X, Z, n);
}

void randFLOAT(float Z[], int n)
{
	randFLOAT_Ptr(Z, n);
}

void randDOUBLE(double Z[], int n)
{
	randDOUBLE_Ptr(Z, n);
}

void randseed(int seed)
{
	randseed_Ptr(seed);
}

/* save internal state of pseudo random number generator */
void randsave(double mem[])  /* size of mem array has to be at least 608 */
{
	randsave_Ptr(mem);
}

/* restore internal state of pseudo random number generator */
void randrestore(double mem[])  /* size of mem array has to be at least 608 */
{
	randrestore_Ptr(mem);
}

void quantFLOATtoUINT(float a, float b, const float * restrict X, unsigned int * restrict Z, int n)
{
	quantFLOATtoUINT_Ptr(a, b, X, Z, n);
}

void setFLOAT(float a, float X[], int n)
{
	setFLOAT_Ptr( a, X, n);
}

void setFLOATflex(float a, float X[], int incX, int n)
{
	setFLOATflex_Ptr(a, X, incX, n);
}

void setINT(int a, int X[], int n)
{
	setINT_Ptr(a, X, n);
}

void setINTflex(int a, int X[], int incX, int n)
{
	setINTflex_Ptr(a, X, incX, n);
}

void smultFLOATip(float a, float *X, int n)
{
	smultFLOATip_Ptr(a, X, n);
}

void smultINTip(int a, int *X, int n)
{
	smultINTip_Ptr(a, X, n);
}






