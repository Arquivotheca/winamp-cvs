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
*   $Id: mconfig.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:  machine dependent configuration items                               *
*                                                                                              *
************************************************************************************************/

#ifndef _MCONFIG_H_
#define _MCONFIG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>

#ifndef WORDS_BIGENDIAN    /* if we're not big-endian, ... */
#define WORDS_LITTLEENDIAN /* ... we must be little-endian. */
#endif

#endif /* HAVE_CONFIG_H */

#ifndef HAVE_CONFIG_H

#ifdef _MSC_VER /* Microsoft Visual C */
#define inline __inline
#define popen _popen
#define pclose _pclose
#ifndef __ICL
#define restrict /* MSVC does not support restrict, but Intel compiler does */
#endif
/* INTEL SSE2 support */
#ifdef P4_CODE
#define P4_INTRINSIC
#endif
#endif

/* Open MP based Encoder Threading */ 
#if defined _OPENMP
#ifndef IISMP3_USE_THREADS
#define IISMP3_USE_THREADS
#endif
#endif

#if defined P4_CODE || defined GP_CODE
/* rename all internal functions */
#define MP3ENCAPI
#include "mp3ifc.h"
#endif

#ifdef __MRC__
#define inline
#define restrict
#endif

#ifdef __GNUC__
#define restrict
#endif

#ifdef __QNX__
#define inline
#define __inline
#define restrict
#define BOOL_BROKEN
#ifndef QNX4
#define HAVE_LOG2
#endif
#endif /* __QNX__ */

#ifdef _M_IX86
#define WORDS_LITTLEENDIAN
#endif

#endif /* HAVE_CONFIG_H */

/* define some mathematical constants */

#if defined(macintosh) && defined(__MRC__)
#include <float.h>
#undef FLT_MAX
#define FLT_MAX	3.402823466e+38F
#endif

/* #include <math.h> */
#ifndef M_SQRT2 /* sqrt(2) */
#define M_SQRT2         1.41421356237309504880
#endif

#ifndef M_SQRT1_2 /* 1 / sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440
#endif

#ifndef M_LN2 /* ln(2) */
#define M_LN2           0.69314718055994530942
#endif

#ifndef M_LN10 /* ln(10) */
#define M_LN10          2.30258509299404568402
#endif

#ifndef M_LOG2E /* log2(e) */
#define M_LOG2E         1.4426950408889634074
#endif

#ifndef M_PI /* PI */
#define M_PI            3.14159265358979323846
#endif

/* support for C++ compilers without datatype bool */
#if defined(__cplusplus) && defined(BOOL_BROKEN)

class bool
{
 public:
  bool(int e=0) {val = e;}
  bool(const bool &a) {val = a.val;}

  bool operator&&(const bool &a) const {return bool(val && a.val);}
  bool operator||(const bool &a) const {return bool(val || a.val);}

  int operator==(const bool &a) const {return ((int)*this) == ((int)a);}

  operator int () const {return val ? 1 : 0;}

  static bool true()   {return bool(1);}
  static bool false()  {return bool(0);}
 private:
  int val;
};

#define true  (bool::true())
#define false (bool::false())

#endif

#endif /* _MCONFIG_H_ */

