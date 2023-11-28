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

#include "bcc_offt.h"

/***********************  rff2.c  *********************

              Fast FFT of a real time sequence based on viewing
	      the full-size real-data transform as an half-size
	      complex-data transform.

	      Similar to rff1.c except that initialization of cosin[][]
	      and bitrev[]  is done externally, e.g., by init_fft.c

	      irff2.c is the inverse of rff2.c   The two routines
	      the same  constants in cosin[2][] and bitrev[]

	      Y. Shoham  5/95        94/95 p. 178


The folowing are assumed to be initialized to the size used by
rff2.c and irff2.c: (See rff1.c or init_fft.c);
*/


void rff2_(

float* x,		/* Real input data of size nsiz */
float* re,		/* Complex output data (FFT of x) of size nsiz/2 + 1 */
float* im,		/* Complex output data (FFT of x) of size nsiz/2 + 1 */
int nsiz,   	/* Transform size */
float** cosin	/* fft constants */
)

{
  register int k,kw,n4;
  float pn;
  float t0,t1,u0,u1,w0,w1;
  int nsizh;
  float *cosn, *sins;

  cosn = cosin[0];
  sins = cosin[1];

  nsizh = nsiz>>1;



  splitRadixComplexFFT(nsiz/2, x);



  /* For 0 , nsizh/2 , nsizh terms :
  *********************************/
  t0 = x[0];
  t1 = x[1];
  re[0] = t0 + t1;
  im[0] = 0.0;
  re[nsizh] = t0 - t1;
  im[nsizh] = 0.0;
  n4 = nsizh>>1;
  re[n4] =  x[nsizh];
  im[n4] = -x[nsizh+1];

  /*Other terms
  *************/
  for(k=1,kw=nsizh-1; k < n4; k++, kw--) {
	  w0 = x[2*k];
    w1 = x[2*kw];
	  t0 = w0 + w1;
	  u0 = w1 - w0;
	  w0 = x[2*k+1];
    w1 = x[2*kw+1];
	  t1 = w0 - w1;
	  u1 = w0 + w1;
	  w0 = cosn[k];
    w1 = sins[k];
	  pn = u0;
	  u0 = w0*u1 - w1*u0;
	  u1 = w0*pn + w1*u1;

	  re[k] = 0.5f * (t0 + u0);
	  im[k] = 0.5f * (t1 + u1);

	  re[kw] = 0.5f * (t0 - u0);
	  im[kw] = 0.5f * (u1 - t1);
  }
}
