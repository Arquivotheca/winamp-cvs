/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mdct.cpp
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: mdct class
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/20 20:58:09 $
 * $Id: mdct.cpp,v 1.5 2011/01/20 20:58:09 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "mdct.h"
#include <arm_neon.h>

/*-------------------------------------------------------------------------*/

#ifndef min
  #define min(a,b) ((a) < (b) ? (a):(b))
#endif

/*-------------------------------------------------------------------------*/

NALIGN(16) static const float hybrid_win[4][36] =
  {
    {
    0.058937661350f, 0.158918619156f, 0.232585847378f, 0.277700960636f,
    0.292893201113f, 0.277700960636f, 0.232585847378f, 0.158918619156f,
    0.058937661350f,-0.064319171011f,-0.207106769085f,-0.365086615086f,
   -0.533458590508f,-0.707106769085f,-0.880754947662f,-1.049126982689f,
   -1.207106828690f,-1.349894404411f,-1.473151206970f,-1.573132276535f,
   -1.646799445152f,-1.691914558411f,-1.707106709480f,-1.691914439201f,
   -1.646799325943f,-1.573132157326f,-1.473151206970f,-1.349894404411f,
   -1.207106828690f,-1.049126982689f,-0.880754947662f,-0.707106769085f,
   -0.533458590508f,-0.365086644888f,-0.207106769085f,-0.064319171011f
    },
    {
    0.058937661350f, 0.158918619156f, 0.232585847378f, 0.277700960636f,
    0.292893201113f, 0.277700960636f, 0.232585847378f, 0.158918619156f,
    0.058937661350f,-0.064319171011f,-0.207106769085f,-0.365086615086f,
   -0.533458590508f,-0.707106769085f,-0.880754947662f,-1.049126982689f,
   -1.207106828690f,-1.349894404411f,-1.474554657936f,-1.586706638336f,
   -1.686782836914f,-1.774021625519f,-1.847759008408f,-1.907433867455f,
   -1.935887336731f,-1.831951141357f,-1.585196495056f,-1.216364026070f,
   -0.758819043636f,-0.254864394665f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f
    },
    {
    0.079459309578f, 0.146446600556f, 0.079459309578f,-0.103553384542f,
   -0.353553384542f,-0.603553414345f,-0.786566138268f,-0.853553354740f,
   -0.786566078663f,-0.603553414345f,-0.353553384542f,-0.103553384542f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f
    },
    {
    0.000000000000f, 0.000000000000f, 0.000000000000f, 0.000000000000f,
    0.000000000000f, 0.000000000000f, 0.056502074003f, 0.099900424480f,
    0.053107600659f,-0.069211170077f,-0.241180941463f,-0.429175883532f,
   -0.601411581039f,-0.765366852283f,-0.923497200012f,-1.074599266052f,
   -1.217522859573f,-1.351180434227f,-1.473151206970f,-1.573132276535f,
   -1.646799445152f,-1.691914558411f,-1.707106709480f,-1.691914439201f,
   -1.646799325943f,-1.573132157326f,-1.473151206970f,-1.349894404411f,
   -1.207106828690f,-1.049126982689f,-0.880754947662f,-0.707106769085f,
   -0.533458590508f,-0.365086644888f,-0.207106769085f,-0.064319171011f
    }
  };

/*-------------------------------------------------------------------------*/

static const float cost9_c[9] =
  {
  1.00000000000000000000000000000000f, 0.98480775301220805936674302458952f, 0.93969262078590838405410927732473f,
  0.86602540378443864676372317075294f, 0.76604444311897803520239265055542f, 0.64278760968653932632264340990726f,
  0.50000000000000000000000000000000f, 0.34202014332566873304409961468226f, 0.17364817766693034885171662676931f
  };

/*-------------------------------------------------------------------------*/
/* 2 * cos (PI * (2i + 1) / 12)*/
static const float cost12_c1[3] =
  {
  1.9318516525781365734994863994578f, 1.4142135623730950488016887242097f, 0.5176380902050415246977976752481f
  };

/*-------------------------------------------------------------------------*/

/* 2 * cos(PI / 6)  */
static const float cost12_c2[1] =
  {
  1.7320508075688772935274463415059f
  };

/*-------------------------------------------------------------------------*/

/* 2 * cos(PI * (2i + 1) / (4 * 18)) */
static const float cost36_c0[] =
  {
  1.9980964431637156f, 1.9828897227476208f, 1.9525920142398667f,
  1.9074339014964539f, 1.8477590650225735f, 1.7740216663564434f,
  1.6867828916257714f, 1.5867066805824706f, 1.4745546736202479f,
  1.3511804152313207f, 1.2175228580174413f, 1.0745992166936478f,
  0.92349722647006771f, 0.76536686473017967f, 0.60141159900854613f,
  0.43287922787620581f, 0.26105238444010298f, 0.087238774730672014f
  };
/*-------------------------------------------------------------------------*/

/* 2 * cos(PI * (2 * i + 1) / (2 * 18)) */
static const float cost36_c1[9] =
  {
  1.9923893961834911f, 1.9318516525781366f, 1.8126155740732999f,
  1.6383040885779836f, 1.4142135623730951f, 1.1471528727020923f,
  0.84523652348139888f, 0.51763809020504192f, 0.17431148549531628f
  };

/* ------------------------------------------------------------------------*/

static void overlap_hybrid18(float *prev, float *dest, const float *even, const float *odd, const float *win);
static void cos_t_h_12(const float *vec, float *f_vec, const float *win);
static void cost9(const float *y, float *s);

//-------------------------------------------------------------------------*
//
//                   C M d c t
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CMdct::CMdct (const MPEG_INFO &_info)
 : info(_info)
{
  Init();
}

//-------------------------------------------------------------------------*
//   Init
//-------------------------------------------------------------------------*

void CMdct::Init()
{
  int i, j, k;

  for ( i=0;i<2;i++ )
    for ( j=0;j<SBLIMIT;j++ )
      for ( k=0;k<SSLIMIT;k++ )
        prevblck[i][j][k] = 0.0f;
}

//-------------------------------------------------------------------------*
//   Apply
//-------------------------------------------------------------------------*

void CMdct::Apply(int ch, const MP3SI_GRCH &SiGrCh, SPECTRUM &rs)
{
	int sb, ss;
	int bt, sblim;
	int SbLimMixed;

	//
	// gr_info->zeroSbStartNdx+1:
	// +1 because we have to process one more subband due to alias-reduction.
	// This wouldn't be neccessary for short blocks and in some cases of
	// mixed blocks. We don't care about this one extra subband in these cases.
	//
	sblim      = min(SBLIMIT, SiGrCh.zeroSbStartNdx+1);
	SbLimMixed = ((info.fhgVersion==MPG_MPEG25)&&(info.sample_rate_ndx==MPG_SF_LOW)) ? 4 : 2;

	for ( sb=0; sb<sblim; sb++ )
	{
		// get blocktype, override with long for mixed and sb<SbLimMixed
		bt = (SiGrCh.window_switching_flag&&SiGrCh.mixed_block_flag &&(sb<SbLimMixed)) ?
			0 : SiGrCh.block_type;

		// below limit -> calc i-mdct
		if ( bt == 2 )
			cos_t_h_short(prevblck[ch][sb], rs[ch][sb], hybrid_win[bt]);
		else
			cos_t_h_long (prevblck[ch][sb], rs[ch][sb], hybrid_win[bt]);

		if ( sb&1 )
			for ( ss=1; ss<SSLIMIT; ss+=2 )
				rs[ch][sb][ss] = -rs[ch][sb][ss];
	}

	for (;sb<SBLIMIT; sb++ )
	{
		// above limit -> all zero, just (easy version of) overlap and add
		for ( ss=0; ss<SSLIMIT; ss++ )
		{
			rs[ch][sb][ss]       = prevblck[ch][sb][ss];
			prevblck[ch][sb][ss] = 0.0f;
		}

		if ( sb&1 )
			for ( ss=1; ss<SSLIMIT; ss+=2 )
				rs[ch][sb][ss] = -rs[ch][sb][ss];
	}
}
/*-------------------------------------------------------------------------*/

static void cos_t_h_12 (const float *vec, float *f_vec, const float *win)
{
  double v0,v1,v2,v3,v4,v5;
  double re0,re1,re2;
  double ro0,ro1,ro2;

  /*  stage 0 */
  v0=vec[0]; v1=vec[1*3];v2=vec[2*3]; v3=vec[3*3];v4=vec[4*3];v5=vec[5*3];
  v4-=v5;
  v3-=v4;
  v2-=v3;
  v1-=v2;
  v0-=v1;

  /*  stage 2 */
  v3-=v5;
  v1-=v3;

  v2*=cost12_c2[0];
  v3*=cost12_c2[0];

  re0 = v0+v4+v2;
  re1 = v0-2.0*v4;
  re2 = v0+v4-v2;

  ro0 = (v1+v5+v3)*cost12_c1[0];
  ro1 = (v1-2.0*v5)*cost12_c1[1];
  ro2 = (v1+v5-v3)*cost12_c1[2];

  v0=(re0+ro0);   /*  *c0_0; in win[0..11] precalcd */
  v5=(re0-ro0);   /*  *c0_5; */
  v1=(re1+ro1);   /*  *c0_1; */
  v4=(re1-ro1);   /*  *c0_4; */
  v2=(re2+ro2);   /*  *c0_2; */
  v3=(re2-ro2);   /*  *c0_3; */

  f_vec[8]  += (float)v0*win[8];
  f_vec[9]  += (float)v0*win[9];
  f_vec[7]  += (float)v1*win[7];
  f_vec[10] += (float)v1*win[10];
  f_vec[6]  += (float)v2*win[6];
  f_vec[11] += (float)v2*win[11];

  f_vec[0]  +=  (float)v3*win[0];
  f_vec[5]  +=  (float)v3*win[5];
  f_vec[1]  +=  (float)v4*win[1];
  f_vec[4]  +=  (float)v4*win[4];
  f_vec[2]  +=  (float)v5*win[2];
  f_vec[3]  +=  (float)v5*win[3];
}

/*-------------------------------------------------------------------------*/

/* long block type MDCT subroutine */

void CMdct::cos_t_h_long (float *prev, float *dest, const float *win)
{
  dest[16] -= dest[17];
  dest[15] -= dest[16];
  dest[14] -= dest[15];
  dest[13] -= dest[14];
  dest[12] -= dest[13];
  dest[11] -= dest[12];
  dest[10] -= dest[11];
  dest[9]  -= dest[10];
  dest[8]  -= dest[9];
  dest[7]  -= dest[8];
  dest[6]  -= dest[7];
  dest[5]  -= dest[6];
  dest[4]  -= dest[5];
  dest[3]  -= dest[4];
  dest[2]  -= dest[3];
  dest[1]  -= dest[2];
  dest[0]  -= dest[1];

  dest[15] -= dest[17];
  dest[13] -= dest[15];
  dest[11] -= dest[13];
  dest[9]  -= dest[11];
  dest[7]  -= dest[9];
  dest[5]  -= dest[7];
  dest[3]  -= dest[5];
  dest[1]  -= dest[3];

  dest[0]  *= (float)0.5;
  dest[1]  *= (float)0.5;

  cost9(dest,   cost36_rese);
  cost9(dest+1, cost36_reso);

  /*cost36_reso[0] *= cost36_c1[0];
  cost36_reso[1] *= cost36_c1[1];
  cost36_reso[2] *= cost36_c1[2];
  cost36_reso[3] *= cost36_c1[3];
  cost36_reso[4] *= cost36_c1[4];
  cost36_reso[5] *= cost36_c1[5];
  cost36_reso[6] *= cost36_c1[6];
  cost36_reso[7] *= cost36_c1[7];*/
  cost36_reso[8] *= cost36_c1[8];

  overlap_hybrid18(prev, dest, cost36_rese, cost36_reso, win);
}

/*-------------------------------------------------------------------------*/

void CMdct::cos_t_h_short (float *prev, float *dest, const float *win)
{
  int i;

  for ( i=0; i<36; i++ )
    hybrid_res[i] = 0.0f;

  for ( i=0; i<3; i++ )
    cos_t_h_12(dest+i,&(hybrid_res[i*6+6]),win);

  for ( i=0; i<SSLIMIT; i++ )
    {
    dest[i] = hybrid_res[i]+prev[i];
    prev[i] =  hybrid_res[i+SSLIMIT];
    }
}

/*-------------------------------------------------------------------------*/

static void cost9 (const float *y, float *s)
{
  double tmp1,tmp2;

  s[4] = y[0*2]-y[2*2]+y[4*2]-y[6*2]+y[8*2];

  tmp1 = y[0*2]-y[6*2]+cost9_c[6]*(y[2*2]-y[4*2]-y[8*2]);
  tmp2 = cost9_c[3]*(y[1*2]-y[5*2]-y[7*2]);
  s[1] = (float)(tmp1+tmp2);
  s[7] = (float)(tmp1-tmp2);

  tmp1=  y[0*2]+cost9_c[2]*y[2*2]+cost9_c[4]*y[4*2]+cost9_c[6]*y[6*2]+cost9_c[8]*y[8*2];
  tmp2=  cost9_c[1]*y[1*2]+cost9_c[3]*y[3*2]+cost9_c[5]*y[5*2]+cost9_c[7]*y[7*2];
  s[0]=  (float)(tmp1+tmp2);
  s[8]=  (float)(tmp1-tmp2);

  tmp1 = y[0*2]-cost9_c[8]*y[2*2]-cost9_c[2]*y[4*2]+cost9_c[6]*y[6*2]+cost9_c[4]*y[8*2];
  tmp2 = cost9_c[5]*y[1*2]-cost9_c[3]*y[3*2]-cost9_c[7]*y[5*2]+cost9_c[1]*y[7*2];
  s[2] = (float)(tmp1+tmp2);
  s[6] = (float)(tmp1-tmp2);

  tmp1 = y[0*2]-cost9_c[4]*y[2*2]+cost9_c[8]*y[4*2]+cost9_c[6]*y[6*2]-cost9_c[2]*y[8*2];
  tmp2 = cost9_c[7]*y[1*2]-cost9_c[3]*y[3*2]+cost9_c[1]*y[5*2]-cost9_c[5]*y[7*2];
  s[3] = (float)(tmp1+tmp2);
  s[5] = (float)(tmp1-tmp2);
}

#define REVERSE_VECTOR(qvec) { \
    float32x2_t dvec_h = vget_high_f32(qvec);\
    float32x2_t dvec_l = vget_low_f32(qvec);\
    dvec_h = vrev64_f32(dvec_h);\
    dvec_l = vrev64_f32(dvec_l);\
		qvec = vcombine_f32(dvec_h, dvec_l);\
}

/*-------------------------------------------------------------------------*/
static void overlap_hybrid18(float *prev, float *dest, const float *even, const float *odd, const float *win)
{
	/*
	original result is

	for(i=0;i<9;i++)
	{
	cost36_res[i] =(even[i]+odd[i])*cost36_c0[i];
	cost36_res[17-i]=(even[i]-odd[i])*cost36_c0[17-i]
	}
	cost36_c0 Koeffs are precalced in win[0..35], so
	we save these multiplies
	*/
	float32x4_t xmm_prev;
	float32x4_t xmm_odd;
	float32x4_t xmm_even;
	float32x4_t xmm_win;
	float32x4_t xmm_interm_res;
	float32x4_t xmm_even_odd;

	/*
	dest[9]    = prev[9]+(even[0]-odd[0])*win[9];
	dest[10]   = prev[10]+(even[1]-odd[1])*win[10];
	dest[11]   = prev[11]+(even[2]-odd[2])*win[11];
	dest[12]   = prev[12]+(even[3]-odd[3])*win[12];
	*/

	xmm_prev = vld1q_f32((const float32_t *)&prev[9]);
	xmm_odd = vld1q_f32((const float32_t *)&odd[0]);
	xmm_odd = vmulq_f32(xmm_odd, vld1q_f32((const float32_t *)&cost36_c1[0]));
	xmm_even = vld1q_f32((const float32_t *)&even[0]);
	xmm_win = vld1q_f32((const float32_t *)&win[9]);
	xmm_even_odd = vsubq_f32(xmm_even, xmm_odd);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	xmm_interm_res = vaddq_f32(xmm_prev, xmm_interm_res);
	vst1q_f32((float32_t *)&dest[9], xmm_interm_res);

	/*	we have even-odd, might as well use it
	dest[5]    = prev[5]+(even[3]-odd[3])*win[5];
	dest[6]    = prev[6]+(even[2]-odd[2])*win[6];
	dest[7]    = prev[7]+(even[1]-odd[1])*win[7];
	dest[8]    = prev[8]+(even[0]-odd[0])*win[8];
	*/
	REVERSE_VECTOR(xmm_even_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[5]);
	xmm_prev = vld1q_f32((const float32_t *)&prev[5]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	xmm_interm_res = vaddq_f32(xmm_prev, xmm_interm_res);
	vst1q_f32((float32_t *)&dest[5], xmm_interm_res);

	/* since we have even and odd still, let's use it, too
	prev[9]    = (even[0]+odd[0])*win[27];
	prev[10]   = (even[1]+odd[1])*win[28];
	prev[11]   = (even[2]+odd[2])*win[29];
	prev[12]   = (even[3]+odd[3])*win[30];
	*/
	xmm_even_odd = vaddq_f32(xmm_even, xmm_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[27]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	vst1q_f32((float32_t *)&prev[9], xmm_interm_res);

	/* keep using some stuff we've already calculated
	prev[5]    = (even[3]+odd[3])*win[23];
	prev[6]    = (even[2]+odd[2])*win[24];
	prev[7]    = (even[1]+odd[1])*win[25];
	prev[8]    = (even[0]+odd[0])*win[26];
	*/
	REVERSE_VECTOR(xmm_even_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[23]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	vst1q_f32((float32_t *)&prev[5], xmm_interm_res);

	/* we'll do 4-7 (instead of 5-8) for alignment reasons 
	dest[13]   = prev[13]+(even[4]-odd[4])*win[13];
	dest[14]   = prev[14]+(even[5]-odd[5])*win[14];
	dest[15]   = prev[15]+(even[6]-odd[6])*win[15];
	dest[16]   = prev[16]+(even[7]-odd[7])*win[16];
	*/
	xmm_prev = vld1q_f32((const float32_t *)&prev[13]);
	xmm_odd = vld1q_f32((const float32_t *)&odd[4]);
	xmm_odd = vmulq_f32(xmm_odd, vld1q_f32((const float32_t *)&cost36_c1[4]));
	xmm_even = vld1q_f32((const float32_t *)&even[4]);
	xmm_win = vld1q_f32((const float32_t *)&win[13]);
	xmm_even_odd = vsubq_f32(xmm_even, xmm_odd);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	xmm_interm_res = vaddq_f32(xmm_prev, xmm_interm_res);
	vst1q_f32((float32_t *)&dest[13], xmm_interm_res);

	/*	we have even-odd, might as well use it
	dest[1]    = prev[1]+(even[7]-odd[7])*win[1];
	dest[2]    = prev[2]+(even[6]-odd[6])*win[2];
	dest[3]    = prev[3]+(even[5]-odd[5])*win[3];
	dest[4]    = prev[4]+(even[4]-odd[4])*win[4];
	*/
	REVERSE_VECTOR(xmm_even_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[1]);
	xmm_prev = vld1q_f32((const float32_t *)&prev[1]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	xmm_interm_res = vaddq_f32(xmm_prev, xmm_interm_res);
	vst1q_f32((float32_t *)&dest[1], xmm_interm_res);

	/* since we have even and odd still, let's use it, too
	prev[13]   = (even[4]+odd[4])*win[31];
	prev[14]   = (even[5]+odd[5])*win[32];
	prev[15]   = (even[6]+odd[6])*win[33];
	prev[16]   = (even[7]+odd[7])*win[34];
	*/
	xmm_even_odd = vaddq_f32(xmm_even, xmm_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[31]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	vst1q_f32((float32_t *)&prev[13], xmm_interm_res);

	/* keep using some stuff we've already calculated
	prev[1]    = (even[7]+odd[7])*win[19];
	prev[2]    = (even[6]+odd[6])*win[20];
	prev[3]    = (even[5]+odd[5])*win[21];
	prev[4]    = (even[4]+odd[4])*win[22];
	*/	
	REVERSE_VECTOR(xmm_even_odd);
	xmm_win = vld1q_f32((const float32_t *)&win[19]);
	xmm_interm_res = vmulq_f32(xmm_even_odd, xmm_win);
	vst1q_f32((float32_t *)&prev[1], xmm_interm_res);
	/* do the leftovers */

	dest[0]    = prev[0]+(even[8]-odd[8])*win[0];
	dest[17]   = prev[17]+(even[8]-odd[8])*win[17];
	prev[0]    = (even[8]+odd[8])*win[18];
	prev[17]   = (even[8]+odd[8])*win[35];
}

/*-------------------------------------------------------------------------*/
