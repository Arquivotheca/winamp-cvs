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
*   $Id: ratectrl.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

/* bitrate control */

#include "mconfig.h"
#include "mp3alloc.h"
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "ratectrl.h"

struct RATE_CONTROL
{
  long dx,dy,D;
  long decision;
#ifndef NO_PADDING
  int  padding;
  int  paddingMode;
#endif
  int  idx0,idx1;
  long len0,len1;
};

int RCNew(struct RATE_CONTROL **ratectrl)
{
  struct RATE_CONTROL *rc = (struct RATE_CONTROL *) mp3Alloc(sizeof(struct RATE_CONTROL));
  *ratectrl = rc;
  return (rc == 0);
}

int RCInit(struct RATE_CONTROL *rc,
           long dy, long dx, const int n, const int *table, int padding)
{
  int i;

  /* factor out common factors between nominator and denominator */

  RCCancelFraction(&dx,&dy);

  rc->dx = dx;
  rc->dy = dy;

  rc->D = 0;
  rc->decision = 0; /* use higher bitrate first */
#ifndef NO_PADDING
  rc->padding  = 0;
#endif

#if 1
    /* Check for VBR */
  if( (dx==1) && (dy==0) ) {
    rc->idx0 = 0;
    rc->idx1 = 0;
    rc->len0 = 0;
    rc->len1 = 0;
#ifndef NO_PADDING
    rc->paddingMode = padding;
#endif
    return 0;
  }
#endif

#ifndef _NO_FRAMESIZE_FIX
  /* 
     check for first valid bitrate entry which is found at index 1, 
     index 0 is reserved for free format and hence corresponds to bitrate zero
  */
  if (dy < dx*table[1])
#else
  if (dy < dx*table[0])
#endif
  {
    return 1; /* error: cannot achieve this bitrate - bitrate too low */
  }
#ifndef NO_PADDING
  if (dy > dx*(table[n-1]+8))
  {
    return 1; /* error: cannot achieve this high bitrate even with padding */
  }
#else
  if (dy > dx*(table[n-1]))
  {
    return 1; /* error: cannot achieve this high bitrate */
  }
#endif

  rc->idx0 = -1;
  for ( i = 0; i < n; i++ )
  {
    if (table[i] * dx == dy)
    {
      /* perfect match, no bitrate switching, no padding byte */
      rc->idx0 = i;
      rc->idx1 = i;
      break;
    }
    else if (table[i] * dx <= dy && (i+1 == n || table[i+1] * dx > dy))
    {
      rc->idx0 = i;
      rc->idx1 = (i+1 != n ? i+1 : i);
      break;
    }
  }
  assert( rc->idx0 >= 0 );

#if 0
  if (padding != RC_PADDING_ISO)
  {
    /* no ISO padding, presumably no bitrate switching as well. */

    rc->idx1 = rc->idx0;
  }
#endif
  /**************************/

  rc->len0 = table[rc->idx0];
  rc->len1 = table[rc->idx1];

#ifndef NO_PADDING
  rc->paddingMode = padding;
#endif

  return 0;
}

void RCDelete(struct RATE_CONTROL *rc)
{
  if (rc)
  {
    mp3Free(rc);
  }
}

/*
  this frame's size
 */

int RCBitrateIdx(const struct RATE_CONTROL *rc)
{
  return rc->decision ? rc->idx1 : rc->idx0 ;
}

int RCNuansdroMode(const struct RATE_CONTROL *rc)
{
#ifndef NO_PADDING
  return rc->paddingMode; /* we don't need padding */
#else
  return 0;
#endif
}

/* was RCPaddingByte */
int RCNuansdroByte(const struct RATE_CONTROL *rc)
{
#ifndef NO_PADDING
  return rc->padding; /* we don't need padding */
#else
  return 0;
#endif
}

/*
  advance to next frame.

  we want to minimize rc->D + frameSize * rc->dx - rc->dy.
 */

#if defined(_ILP32)
#undef LONG_MAX
#define LONG_MAX INT_MAX
#endif

int RCAdvance(struct RATE_CONTROL *rc)
{
  long minDiff = LONG_MAX;
  long diff;

  rc->D -= rc->dy;

#ifndef NO_PADDING
  /* len0, no padding */
  diff = abs(rc->D + rc->dx * rc->len0);
  if (rc->paddingMode != RC_PADDING_ALWAYS && diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 0;
    rc->padding = 0;
  }

  /* len0 + padding? */
  diff = abs(rc->D + rc->dx * (rc->len0 + 8));
  if (rc->paddingMode != RC_PADDING_NEVER && diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 0;
    rc->padding = 1;
  }

  /* len1 no padding? */
  diff = abs(rc->D + rc->dx * rc->len1);
  if (rc->paddingMode != RC_PADDING_ALWAYS && diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 1;
    rc->padding = 0;
  }

  /* len1 + padding? */
  diff = abs(rc->D + rc->dx * (rc->len1 + 8));
  if (rc->paddingMode != RC_PADDING_NEVER && diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 1;
    rc->padding = 1;
  }

  /* assert that one choice has been made */
  assert(minDiff != LONG_MAX);

  rc->D += rc->dx * ((rc->padding ? 8 : 0) + (rc->decision ? rc->len1 : rc->len0));
#else
  /* len0, no padding */
  diff = abs(rc->D + rc->dx * rc->len0);
  if (diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 0;
  }

  /* len1 no padding? */
  diff = abs(rc->D + rc->dx * rc->len1);
  if (diff < minDiff)
  {
    minDiff = diff;
    rc->decision = 1;
  }

  /* assert that one choice has been made */
  assert(minDiff != LONG_MAX);

  rc->D += rc->dx * ((rc->decision ? rc->len1 : rc->len0));
#endif

  return 0; /* OK */
}

void RCCancelFraction(long *nom, long *den)
{
  /* cancel out common factors between nominator and denominator */

  unsigned int i;
#ifndef _NO_FRAMESIZE_FIX
  static const int factor[] = {2,3,5,7,11,13,17,19,31,61,89,107,127 };
#else
  static const int factor[] = {2,3,5,7,11,13,17,19};
#endif

  for (i=0; i<sizeof(factor)/sizeof(factor[0]); i++)
  {
    do
    {
      long nom1 = *nom / factor[i];
      long den1 = *den / factor[i];
      if (nom1 * factor[i] != *nom || den1 * factor[i] != *den)
        break;
      *nom = nom1; *den = den1;
    }
    while(1);
  }
}
