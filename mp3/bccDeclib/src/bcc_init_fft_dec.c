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

/***********************  init_fft.c  *********************

   Initialization of constant arrays for general FTT and DFT routines

   The cos/sin constants are computed for the range k = 0,..,size-1


   Note that most FFT routine use only half of the range, that is,
   k = 1,..,siz/2
*/

#include <math.h>


void init_fft_(

int size,	/* Transform size. MUST be radix-2 */
float** w 	/* Complex FFT constants of size nsiz */
)

{
  register int k;
  float p,pk;
  float *cosn, *sins;

  cosn = w[0];
  sins = w[1];


  p = 8.f*(float)atan(1.)/size;

  for(pk=0., k=0; k < size; k++, pk += p) {
    cosn[k]  =  (float)cos(pk);
	  sins[k]  = (float)-sin(pk);
  }

}




