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
*   $Id: cmdct.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <math.h>
#include "mconfig.h"
#include "cfftl3.h"
#include "cmdct.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846264338327950288
#endif

/*****************************************************************************

    functionname:PreTwiddleOdd  
    description: used for odd fft
    returns:
    input:       
    output:      
    globals:     

*****************************************************************************/
static void PreTwiddleOdd(float * indata, int len)
{
  int i;
  double c1, s1, c2, s2, t;

  c1 = 1.0;
  s1 = 0.0;
  
  c2 = cos(2.0*M_PI/len);
  s2 = sin(2.0*M_PI/len);

 
  for(i=0;i<len;i+=2){
    /*
      rotate re,im about i*2.0*PI/len
    */
    
    t = indata[i]*c1 + indata[i+1]*s1;
    indata[i+1] = (float)(indata[i+1]*c1 - indata[i]*s1);
    indata[i]   = (float)t;

    /*
       Advance trig generator
    */

    t = c1;
    c1 = c1 * c2 - s1 * s2;
    s1 = t * s2 + s1 * c2;
  }
}

/*****************************************************************************

    functionname:PostTwiddleMdct  
    description: used for cmdct
    returns:
    input:       
    output:      
    globals:     

*****************************************************************************/
static void PostTwiddleMdct(float * indata, int len)
{
  double c1, s1, c2, s2, t;
  int i;
  
  c1=cos((M_PI/len)*(0.5+len*0.25));
  s1=sin((M_PI/len)*(0.5+len*0.25));
  
  c2=c1*c1-s1*s1;
  s2=2.0*c1*s1;
  
  
  for(i=0;i<len;i+=2){
    /*
        rotate re,im about 
        (2.0*PI/len)*(i+0.5)*(0.5+len/4)
    */
    
    t = indata[i]*c1 + indata[i+1]*s1;
    indata[i+1] = (float)(indata[i+1]*c1 - indata[i]*s1);
    indata[i]   = (float)t;

    /*
       Advance trig generator
    */

    t = c1;
    c1 = c1 * c2 - s1 * s2;
    s1 = t * s2 + s1 * c2;
  }
}


/*****************************************************************************

    functionname:ZiDensity1  
    description: used for odd real fft
    returns:
    input:       
    output:      
    globals:     

*****************************************************************************/
static void ZiDensity1(float * indata, int len)
{
  int k1, k2;
  float a1, a2, a3, a4, a5, a6;
  double c1, s1, c2, s2, t;

  
  k1 = 0;
  k2 = len - 2;

  c1=cos(M_PI/len);
  s1=sin(M_PI/len);
  c2=c1*c1-s1*s1;
  s2=2.0*c1*s1;
  
  
  while (k1 < k2) {
   

    a1 = 0.5f * (indata[k1] + indata[k2]);
    a2 = 0.5f * (indata[k1 + 1] + indata[k2 + 1]);
    a3 = indata[k2] - a1;
    a4 = a2 - indata[k2 + 1];

    a5 = (float) (a2 * c1 + a3 * s1);
    a6 = (float) (a3 * c1 - a2 * s1);

    indata[k1] = a1 + a5;
    indata[k2] = a1 - a5;
    indata[k1 + 1] = a6 + a4;
    indata[k2 + 1] = a6 - a4;

     /*
       Advance trig generator
     */

    t = c1;
    c1 = c1 * c2 - s1 * s2;
    s1 = t * s2 + s1 * c2;


    k1 += 2;
    k2 -= 2;
  }
}


/*****************************************************************************

    functionname: FFTOTransform  
    description:  Real value input odd fft transform
    returns:
    input:        
    output:       
    globals:      

*****************************************************************************/
static void FFTOTransform(float * afftData, int len)
{

  PreTwiddleOdd(afftData,len);
  /* we have special cmdcts for the layer-3 blocklengths. Use these rather
     than the general routines. */
  cfft_l3(afftData,len/2); /* only forward transform implemented */
  ZiDensity1(afftData, len);

}

/*****************************************************************************

    functionname: CMDCTTransform  
    description:  real value input complex MDCT Transform
                  Note: To get the standard MDCT-Transform values,
                  take real values from this transform and multiply
                  them by 2.0 (->see def. of MDCT ISO/IEC)
    returns:     
    input:        
    output:       
    globals:      

*****************************************************************************/
void mp3CMDCTTransform(float * acmdctData, int len)
{
  
  FFTOTransform(acmdctData,len);
  PostTwiddleMdct(acmdctData,len);

}

