/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3tools.cpp
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: Layer III-processing-functions
 *                         stereo-processing, antialias & reordering
 *                         hybrid synthese
 *
 * SSE implementation
 *
 \***************************************************************************/

/*
 * $Date: 2011/01/21 21:28:46 $
 * $Id: mp3tools.cpp,v 1.3 2011/01/21 21:28:46 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "nsmp3/mp3tools.h"
#include "nsmp3/l3table.h"
#include <emmintrin.h>
/* ------------------------------------------------------------------------*/

#ifndef min
  #define min(a,b) ((a) < (b) ? (a):(b))
#endif

#ifndef max
  #define max(a,b) ((a) > (b) ? (a):(b))
#endif

/*-------------------------------------------------------------------------*/

static const double M_SQRT_2 = 0.70710678118654752440084436210485;

/*-------------------------------------------------------------------------*/
/* tan(i * (PI / 12)) / (1 + tan(i * (PI / 12)))*/
static const float tan12_tab1[7] =
  {
  0.000000000000f, 0.21132486540518708f, 0.36602540378443865f,
	0.500000000000f, 0.63397459621556129f, 0.78867513459481264f,
  1.000000000000f
  };

/*-------------------------------------------------------------------------*/

static const float tan12_tab2[7] =
  {
  1.000000000000f, 0.78867513459481264f, 0.63397459621556129f,
	0.500000000000f, 0.36602540378443865f, 0.21132486540518708f,
  0.000000000000f
  };

/*-------------------------------------------------------------------------*/

static const float mpeg2_itab[2][16] =
{
	/* (1 / sqrt(sqrt(2)))^i */
  {
  1.00000000000000000f, 0.84089641525371461f, 0.70710678118654768f, 0.59460355750136074f,
  0.50000000000000000f, 0.42044820762685747f, 0.35355339059327395f, 0.29730177875068042f,
  0.25000000000000000f, 0.21022410381342879f, 0.17677669529663703f, 0.14865088937534027f,
  0.12500000000000000f, 0.10511205190671444f, 0.088388347648318544f, 0.074325444687670161f
  },
	/* (1 / sqrt(2))^i */
  {
  1.000000000f, 0.70710678118654746f, 0.500000000f, 0.35355339059327368f,
  0.250000000f, 0.17677669529663681f, 0.125000000f, 0.088388347648318391f,
  0.062500000f, 0.044194173824159182f, 0.031250000f, 0.022097086912079587f,
  0.015625000f, 0.011048543456039792f, 0.007812500f, 0.0055242717280198951f
  }
};

/* c[]  = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 } */

/*-------------------------------------------------------------------------*/
/* 1 / sqrt(1 + c[i]^2) */
NALIGN(16) static const float but_cs[8] =
  {
	0.85749292571254432f,  0.8817419973177052f,  0.94962864910273281f,  0.98331459249179021f,
	0.99551781606758583f,  0.99916055817814753f,  0.99989919524444715f,  0.9999931550702803f
  };

/*-------------------------------------------------------------------------*/

NALIGN(16) static const float but_ca[8] =
  {
 -0.51449575542752657f, -0.47173196856497235f, -0.31337745420390184f, -0.18191319961098118f,
 -0.094574192526420658f, -0.040965582885304053f, -0.01419856857247115f, -0.0036999746737600373f
  };

/*-------------------------------------------------------------------------*/

/* Stereo ms mode routine */
static void III_process_stereo_ms(float *pLeft, float *pRight, int startNdx, int endNdx)
{
	int    indx;
	double temp;

	pLeft += startNdx;
	pRight += startNdx;
	int count = endNdx-startNdx;

		float sqrt_2 = (float)M_SQRT_2;
		__m128 xmm_sqrt_2 = _mm_load1_ps(&sqrt_2);
		// benski> empirical testing shows that it's not worth aligning
		while (count > 4)
		{
			__m128 xmm_mid = _mm_loadu_ps(pLeft);
			__m128 xmm_side = _mm_loadu_ps(pRight);
			__m128 xmm_left = _mm_add_ps(xmm_mid, xmm_side);
			xmm_left = _mm_mul_ps(xmm_left, xmm_sqrt_2);
			__m128 xmm_right = _mm_sub_ps(xmm_mid, xmm_side);
			xmm_right = _mm_mul_ps(xmm_right, xmm_sqrt_2);
			_mm_storeu_ps(pLeft, xmm_left);
			_mm_storeu_ps(pRight, xmm_right);
			count-=4;
			pLeft+=4;
			pRight+=4;
		}
		while (count--)
		{
			temp         = ( *pLeft + *pRight) * M_SQRT_2;
			*pRight++ = (float)(( *pLeft - *pRight ) * M_SQRT_2);
			*pLeft++ = (float)(temp);
		}

}

/*-------------------------------------------------------------------------*/

/* Stereo intensity mode routine */
static void III_process_stereo_intens
    (
    float           *pLeft,
    float           *pRight,
    int              startNdx,
    int              endNdx,
    int              position,
    int              intensity_scale,
    const MPEG_INFO &Info
    )
{
  int   indx;
  float tmp;

  if ( Info.IsMpeg1 )
    {
    /* MPEG 1 */
      for ( indx=startNdx; indx<endNdx; indx++ )
        {
        tmp          = pLeft[indx];
        pLeft[indx]  = tmp * tan12_tab1[position];
        pRight[indx] = tmp * tan12_tab2[position];
        }

    }
  else
    {
    /* MPEG 2 */
      if ( position & 1 )
        {
        position = (position+1)>>1;

        for ( indx=startNdx; indx<endNdx; indx++ )
          {
          pRight[indx]  = pLeft[indx];
          pLeft[indx]  *= mpeg2_itab[intensity_scale][position];
          }
        }
      else
        {
        position>>=1;

        for ( indx=startNdx; indx<endNdx; indx++ )
          {
          pRight[indx] = pLeft[indx] * mpeg2_itab[intensity_scale][position];
          }
        }
		
    }
}

/*-------------------------------------------------------------------------*/

static void III_process_stereo_lr
    (
    float *pLeft,
    float *pRight,
    int    startNdx,
    int    endNdx
    )
{
  int indx;

}

/*-------------------------------------------------------------------------*/

/* Mixed block stereo processing */
static void III_stereo_mixed
    (
    float            *pLeft,
    float            *pRight,
    const MP3SI_GRCH &SiL,
    const MP3SI_GRCH &SiR,
    const MP3SCF     &ScaleFac, /* right channel!! */
    const MPEG_INFO  &Info
    )
{
  int ms_stereo = Info.mode_ext & 0x2;
  int i_stereo  = Info.mode_ext & 0x1;

  int cbZero;
  int cbMaxBand;
  int pt, cb, width;
  int startNdx,endNdx;
  int Position;
  int IllegalPos;

  cbMaxBand = max(SiL.zeroSfbStartNdxSMax,
                  SiR.zeroSfbStartNdxSMax);

  if ( i_stereo && SiR.zeroSfbStartNdxIsLong )
    cbZero = SiR.zeroSfbStartNdxL;
  else
    cbZero = 22;

  /* long-block part */
  for ( cb=0; cb<(Info.IsMpeg1 ? 8:6); cb++ )
    {
    Position   = ScaleFac.l[cb];
    IllegalPos = ScaleFac.l_iip[cb];

    startNdx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].l[cb];
    endNdx   = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].l[cb+1];

    if ( (cb<cbZero) || (i_stereo && (Position == IllegalPos)) )
      {
      if ( ms_stereo )
        III_process_stereo_ms(pLeft, pRight, startNdx, endNdx);
      else
        III_process_stereo_lr(pLeft, pRight, startNdx, endNdx);
      }

    if ( i_stereo && (cb>=cbZero) && (Position != IllegalPos) )
      III_process_stereo_intens(pLeft, pRight,
                                startNdx, endNdx,
                                Position,
                                SiR.intensity_scale,
                                Info);
    }

  /* short-block part */
  for ( pt=0; pt<3; pt++ )
    {
    /* get intensity-border */
    if ( i_stereo )
      {
      if ( SiR.zeroSfbStartNdxIsLong )
        cbZero = 0;
      else
        cbZero = SiR.zeroSfbStartNdxS[pt];
      }
    else
      {
      cbZero=13;
      }

    for ( cb=3; cb<cbMaxBand; cb++ )
      {
      Position   = ScaleFac.s[pt][cb];
      IllegalPos = ScaleFac.s_iip[cb];

      width = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb+1]-
              sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb];

      startNdx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb]*3+width*pt;
      endNdx   = startNdx+width;

      if ( (cb<cbZero) || (i_stereo && (Position == IllegalPos)) )
        {
        if ( ms_stereo )
          III_process_stereo_ms(pLeft, pRight, startNdx, endNdx);
        else
          III_process_stereo_lr(pLeft, pRight, startNdx, endNdx);
        }

      if ( i_stereo && (cb>=cbZero) && (Position != IllegalPos) )
        III_process_stereo_intens(pLeft, pRight,
                                  startNdx, endNdx,
                                  Position,
                                  SiR.intensity_scale,
                                  Info);
      }
    }
}

/*-------------------------------------------------------------------------*/

/* short block stereo processing */
static void III_stereo_short
    (
    float            *pLeft,
    float            *pRight,
    const MP3SI_GRCH &SiL,
    const MP3SI_GRCH &SiR,
    const MP3SCF     &ScaleFac, /* right channel!! */
    const MPEG_INFO  &Info )
{
  int ms_stereo = Info.mode_ext & 0x2;
  int i_stereo  = Info.mode_ext & 0x1;

  int cbZero;
  int cbMaxBand;
  int pt, cb, width;
  int startNdx, endNdx;
  int Position;
  int IllegalPos;

  cbMaxBand = max(SiL.zeroSfbStartNdxSMax,
                  SiR.zeroSfbStartNdxSMax);

  for ( pt=0; pt<3; pt++ )
    {
    /* get intensity-border */
    if ( i_stereo )
      cbZero = SiR.zeroSfbStartNdxS[pt];
    else
      cbZero=13;

    for ( cb=0; cb<cbMaxBand; cb++ )
      {
      Position   = ScaleFac.s[pt][cb];
      IllegalPos = ScaleFac.s_iip[cb];

      width = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb+1]-
              sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb];

      startNdx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[cb]*3+width*pt;
      endNdx   = startNdx+width;

      if ( (cb<cbZero) || (i_stereo && (Position == IllegalPos)) )
        {
        /* sfb below intensity-border or illegal intensity-position */
        if ( ms_stereo )
          III_process_stereo_ms(pLeft, pRight, startNdx, endNdx);
        else
          III_process_stereo_lr(pLeft, pRight, startNdx, endNdx);
        }

      if ( i_stereo && (cb>=cbZero) && (Position != IllegalPos) )
        {
        /* sfb above intensitiy-border and legal intensity-position */
        III_process_stereo_intens(pLeft, pRight,
                                  startNdx, endNdx,
                                  Position,
                                  SiR.intensity_scale,
                                  Info);
        }
      }
    }
}
/*-------------------------------------------------------------------------*/

/* long block stereo processing */
static void III_stereo_long
    (
    float            *pLeft,
    float            *pRight,
    const MP3SI_GRCH &SiL,
    const MP3SI_GRCH &SiR,
    const MP3SCF     &ScaleFac, /* right channel!! */
    const MPEG_INFO  &Info )
{
  int ms_stereo = Info.mode_ext & 0x2;
  int i_stereo  = Info.mode_ext & 0x1;

  int cbZero;
  int cbMaxBand;
  int cb;
  int startNdx,endNdx;
  int Position;
  int IllegalPos;

  /* get intensity-border */
  if ( i_stereo )
    cbZero = SiR.zeroSfbStartNdxL;
  else
    cbZero = 22;

  cbMaxBand = max(SiL.zeroSfbStartNdxL,
                  SiR.zeroSfbStartNdxL);

  for ( cb=0; cb<cbMaxBand; cb++ )
    {
    Position   = ScaleFac.l[cb];
    IllegalPos = ScaleFac.l_iip[cb];

    startNdx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].l[cb];
    endNdx   = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].l[cb+1];

    if ( (cb<cbZero) || (i_stereo && (Position == IllegalPos)) )
      {
      /* sfb below intensity-border or illegal intensity-position */
      if ( ms_stereo )
        III_process_stereo_ms(pLeft, pRight, startNdx, endNdx);
      else
        III_process_stereo_lr(pLeft, pRight, startNdx, endNdx);
      }

    if ( i_stereo && (cb>=cbZero) && (Position != IllegalPos) )
      {
      /* sfb above intensitiy-border and legal intensity-position */
      III_process_stereo_intens(pLeft, pRight,
                                startNdx, endNdx,
                                Position,
                                SiR.intensity_scale,
                                Info);
      }
    }
}

/*-------------------------------------------------------------------------*/

void mp3StereoProcessing
    (
    float            *pLeft,
    float            *pRight,
    MP3SI_GRCH       &SiL,
    MP3SI_GRCH       &SiR,
    const MP3SCF     &ScaleFac, /* right channel!! */
    const MPEG_INFO  &Info
    )
{
  if ( (Info.stereo == STEREO) && (Info.mode == MPG_MD_JOINT_STEREO) )
    {
    /* joint stereo */
    if ( SiL.window_switching_flag && (SiL.block_type == 2) )
      {
      if ( SiL.mixed_block_flag )
        III_stereo_mixed(pLeft, pRight, SiL, SiR, ScaleFac, Info);
      else
        III_stereo_short(pLeft, pRight, SiL, SiR, ScaleFac, Info);
      }
    else
      III_stereo_long(pLeft, pRight, SiL, SiR, ScaleFac, Info);

    //
    //  copy _max_ of zero information to left _and_ right
    //
    SiL.zeroStartNdx        = max(SiL.zeroStartNdx,        SiR.zeroStartNdx);
    SiL.zeroSfbStartNdxL    = max(SiL.zeroSfbStartNdxL,    SiR.zeroSfbStartNdxL);
    SiL.zeroSfbStartNdxSMax = max(SiL.zeroSfbStartNdxSMax, SiR.zeroSfbStartNdxSMax);
    SiL.zeroSfbStartNdxS[0] = max(SiL.zeroSfbStartNdxS[0], SiR.zeroSfbStartNdxS[0]);
    SiL.zeroSfbStartNdxS[1] = max(SiL.zeroSfbStartNdxS[1], SiR.zeroSfbStartNdxS[1]);
    SiL.zeroSfbStartNdxS[2] = max(SiL.zeroSfbStartNdxS[2], SiR.zeroSfbStartNdxS[2]);

    SiR.zeroStartNdx        = SiL.zeroStartNdx;
    SiR.zeroSfbStartNdxL    = SiL.zeroSfbStartNdxL;
    SiR.zeroSfbStartNdxSMax = SiL.zeroSfbStartNdxSMax;
    SiR.zeroSfbStartNdxS[0] = SiL.zeroSfbStartNdxS[0];
    SiR.zeroSfbStartNdxS[1] = SiL.zeroSfbStartNdxS[1];
    SiR.zeroSfbStartNdxS[2] = SiL.zeroSfbStartNdxS[2];
    }  
}

/*-------------------------------------------------------------------------*/

/*
 * reorder one sfb
 */
static void reorder_sfb(float *pData, int startNdx, int endNdx)
{
  float fTmp[3*256];

  int indxr = 0;
  int width = (endNdx-startNdx)/3;
  int indx;

  for ( indx=startNdx; indx<startNdx+width; indx++ )
    {
    fTmp[indxr++] = pData[indx];
    fTmp[indxr++] = pData[indx+width];
    fTmp[indxr++] = pData[indx+width*2];
    }

  for ( indx=0; indx<width*3; indx++ )
    pData[startNdx+indx] = fTmp[indx];
}

/*-------------------------------------------------------------------------*/

void mp3Reorder
    (
    float            *pData,
    const MP3SI_GRCH &Si,
    const MPEG_INFO  &Info
    )
{
  int sfb;
  int startNdx;
  int endNdx;

  if ( Si.window_switching_flag && (Si.block_type == 2) )
    {
    /* mixed/short Block */
    for ( sfb=(Si.mixed_block_flag ? 3:0); sfb<13; sfb++ )
      {
      startNdx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[sfb]*3;
      endNdx   = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[sfb+1]*3;
      reorder_sfb(pData, startNdx, endNdx);
      }
    }
}

/*-------------------------------------------------------------------------*/

static void III_calcSbLimit(MP3SI_GRCH &SiGrCh, const MPEG_INFO &Info)
{
  //
  // pSi->zeroSbStartNdx:
  // index of first subband with all zero
  //
  if  ( SiGrCh.window_switching_flag && (SiGrCh.block_type == 2) )
    {
    // might not work for mixed blocks, never tested
    int ndx = sfBandIndex[Info.fhgVersion][Info.sample_rate_ndx].s[SiGrCh.zeroSfbStartNdxSMax];

    SiGrCh.zeroSbStartNdx = (ndx*3)/SSLIMIT + ( (((ndx*3)%SSLIMIT)!=0) ? 1:0 );
    }
  else
    {
    SiGrCh.zeroSbStartNdx = SiGrCh.zeroStartNdx / SSLIMIT +
      ( ((SiGrCh.zeroStartNdx % SSLIMIT)!= 0) ? 1:0 );
    }
}

/*-------------------------------------------------------------------------*/
static void antialias_sb(float *sb1,float *sb2)
{
	__m128 xmm_sb1, xmm_sb2, xmm_but_cs, xmm_but_ca;
	__m128 results;

	xmm_sb1 = _mm_loadu_ps(&sb1[14]);
	xmm_sb1 = _mm_shuffle_ps(xmm_sb1, xmm_sb1, 0x1B); 
	xmm_sb2 = _mm_loadu_ps(&sb2[0]);
	xmm_but_cs = _mm_load_ps(&but_cs[0]);
	xmm_but_ca = _mm_load_ps(&but_ca[0]);
	
	results = _mm_add_ps(_mm_mul_ps(xmm_sb2, xmm_but_cs), _mm_mul_ps(xmm_sb1, xmm_but_ca));
	_mm_storeu_ps(&sb2[0], results);

	results = _mm_sub_ps(_mm_mul_ps(xmm_sb1, xmm_but_cs), _mm_mul_ps(xmm_sb2, xmm_but_ca));
	results = _mm_shuffle_ps(results, results, 0x1B); 
	_mm_storeu_ps(&sb1[14], results);
	
	xmm_sb1 = _mm_loadu_ps(&sb1[10]);
	xmm_sb1 = _mm_shuffle_ps(xmm_sb1, xmm_sb1, 0x1B); 
	xmm_sb2 = _mm_loadu_ps(&sb2[4]);
	xmm_but_cs = _mm_load_ps(&but_cs[4]);
	xmm_but_ca = _mm_load_ps(&but_ca[4]);
	
	results = _mm_add_ps(_mm_mul_ps(xmm_sb2, xmm_but_cs), _mm_mul_ps(xmm_sb1, xmm_but_ca));
	_mm_storeu_ps(&sb2[4], results);

	results = _mm_sub_ps(_mm_mul_ps(xmm_sb1, xmm_but_cs), _mm_mul_ps(xmm_sb2, xmm_but_ca));
	results = _mm_shuffle_ps(results, results, 0x1B); 
	_mm_storeu_ps(&sb1[10], results);
}

/*-------------------------------------------------------------------------*/

void mp3Antialias
    (
    float           *pData,
    MP3SI_GRCH      &SiGrCh,
    const MPEG_INFO &Info
    )
{
  int sb;
  int sblim;

  III_calcSbLimit(SiGrCh, Info);

  if ( SiGrCh.window_switching_flag &&
      (SiGrCh.block_type == 2) &&
      !SiGrCh.mixed_block_flag
     )
    {
    /* short blocks, but no mixed blocks -> no antialias */
    return;
    }

  if ( SiGrCh.window_switching_flag &&
       SiGrCh.mixed_block_flag &&
      (SiGrCh.block_type == 2)
     )
    {
    /* short blocks AND mixed blocks -> antialias for lowest 2 (4) subbands */
    sblim = ((Info.fhgVersion==MPG_MPEG25)&&(Info.sample_rate_ndx==MPG_SF_LOW)) ? 3 : 1;
    }
  else
    {
    sblim = min(SBLIMIT-1,SiGrCh.zeroSbStartNdx);
    }

  /*
   * max 31 alias-reduction operations between each pair of sub-bands
   * with 8 butterflies between each pair
   */
  for ( sb=0; sb<sblim; sb++ )
    {
    antialias_sb(&pData[sb*SSLIMIT], &pData[(sb+1)*SSLIMIT]);
    }
}

/*-------------------------------------------------------------------------*/
