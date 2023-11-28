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
*   $Id: mathmac.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   W. Schildbach                                                                    *
*   contents/description: Macros and small mathematical functions                              *
*                                                                                              *
************************************************************************************************/

#ifndef _mathmac_h
#define _mathmac_h

#include "mconfig.h"
#include <stdlib.h> /* try to find min() and max() */
/* Makros to determine the smaller/bigger value of two integers, doubles
   or floats. Beware of side effects. */

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* IEEE floating point constants */
enum IEEE {
  IEEEmantbits = 23
};

#define BITMASK(a) ((1UL<<(a))-1)
#define MANTISSA(a) ((a) & BITMASK(IEEEmantbits))

enum RSQRT {
  RSQRTresolution = 4
};

enum LOG2 {
  LOG2resolution = 5
};

typedef union {
  float f;
  signed int i;
} ieee_pattern;

__inline static float fastlog2(float a)
{
  signed int exponent;
  unsigned int index,index2 ;
  ieee_pattern b;

  static const float xx[1+(1<<LOG2resolution)] =
  {
    0.000000f, 0.044394f, 0.087463f, 0.129283f,
    0.169925f, 0.209453f, 0.247928f, 0.285402f,

    0.321928f, 0.357552f, 0.392317f, 0.426265f,
    0.459432f, 0.491853f, 0.523562f, 0.554589f,

    0.584962f, 0.614710f, 0.643856f, 0.672425f,
    0.700440f, 0.727920f, 0.754887f, 0.781360f,

    0.807355f, 0.832890f, 0.857981f, 0.882643f,
    0.906891f, 0.930737f, 0.954196f, 0.977280f,

    1.0f
  };
  static const float xx2[5] = {0.0f,0.25f,0.5f,0.75f,1.0f};

  /* x^2 */
  b.f = a*a;

  exponent = ((b.i >> IEEEmantbits) & 0xff) - 127 ;
  index    =  MANTISSA(b.i) >> (IEEEmantbits-LOG2resolution);
  index2   = (MANTISSA(b.i) >> (IEEEmantbits-LOG2resolution-2)) & 0x3;

  return (exponent+xx[index]*xx2[4-index2]+xx[index+1]*xx2[index2]) * 0.5f;
  /* times 1/2 because x^2 */
}

__inline static unsigned int
rsqrt_seed(const ieee_pattern *a, ieee_pattern *aabs)
{
  unsigned int iaabs    = (unsigned)(a->i) & BITMASK(31) ;
  unsigned int exponent = iaabs >> IEEEmantbits ;
  int index             = ((iaabs & BITMASK(IEEEmantbits+1)) >>
                           (IEEEmantbits-RSQRTresolution));

  static const unsigned int xx[1<<(1+RSQRTresolution)] =
  {
    0x00b2416a, 0x00ad166c, 0x00a85835, 0x00a3f8a2,
    0x009fec04, 0x009c2896, 0x0098a61f, 0x00955da2,
    0x00924925, 0x008f6381, 0x008ca83f, 0x008a137d,
    0x0087a1d2, 0x0085503e, 0x00831c1a, 0x0081030a,
    0x007c1764, 0x0074c867, 0x006e133e, 0x0067e3ed,
    0x006229ed, 0x005cd76e, 0x0057e0cf, 0x00533c2e,
    0x004ee116, 0x004ac83f, 0x0046eb5a, 0x004344e6,
    0x003fd012, 0x003c889f, 0x00396ace, 0x0036734a,
  };

  aabs->i = iaabs ;
  return (a->i) ? xx[index] + ((189 - exponent/2) << IEEEmantbits) : 0;
}

#endif /* _mathmac_h */
