/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1996 - 2008 Fraunhofer IIS
 *          (c) 2004 Fraunhofer IIS and Agere Systems Inc.
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#include <math.h>
#include "bcc_offt.h"

/***********************  irff2.c  *********************

              Fast inverse FFT of a real time sequence based on viewing
              the full-size real-data transform as an half-size
              complex-data transform.

              Similar to irff1.c except that initialization of cosin[][]
              and bitrev[]  is done externally, e.g., by init_fft.c

              irff2.c is the inverse of rff2.c. Both use same constants
              cosin[][] and bitrev[].


              Y. Shoham  5/95      94/95 p. 182


The folowing are assumed to be initialized to the size used by
rff2.c and irff2.c: (See rff1.c or init_fft.c);
*/


void irff2_(

float* re,		/* Complex output data (FFT of x) of size nsiz/2 + 1 */
float* im,		/* Complex output data (FFT of x) of size nsiz/2 + 1 */
float* y,		/* Real output data (IFFT of x) of size nsiz */
int nsiz,		/* Transform size */
float** cosin	/* fft constants */
)

{
  register int k,kw,kww,n2,n4;
  float pn;
  float t0,t1,u0,u1,w0,w1;
  static float s;
  int nsizh;
  float *cosn, *sins;

  cosn = cosin[0];
  sins = cosin[1];

  nsizh = nsiz>>1;
  s = 1.f/nsiz;





  /*DC and nsiz/4 terms (and bit reversal)
  **
  **************************************/
  t0 = re[0];
  t1 = re[nsizh];
  y[0] = t0 + t1;
  y[1] = t0 - t1;
  n4 = nsizh>>1;
  n2 = 2*n4;
  y[n2] = 2.f*re[n4];
  y[n2+1] = -2.f*im[n4];

  /* Other terms (and bit reversal)
  ********************************/
  for(k=1, kw=nsizh-1; k < n4; k++, kw--) {
	  w0 = re[kw];
    w1 = re[k];
	  t0 = w0 + w1;
    u0 = w1 - w0;
	  w0 = im[kw];
    w1 = im[k];
	  t1 = w1 - w0;
    u1 = w1 + w0;
	  w0 = cosn[k];
    w1 = -sins[k];
	  pn = u0;
	  u0 = w1*pn + w0*u1;
	  u1 = w0*pn - w1*u1;
	  n2 = 2*k;
	  kww =2*kw;

    y[kww] = t0 - u0;
	  y[kww+1] = t1 + u1;
	  y[n2] = t0 + u0;
	  y[n2+1] = u1 - t1;
  }

  splitRadixComplexFFT(nsizh,y);

  /*Scale it
  **********/

  for(k=0; k < nsiz; k++)
    y[k] *= s;
}
