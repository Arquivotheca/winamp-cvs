/***************************************************************************\
*
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
*                      (c) 2010 Nullsoft, Inc.
*                        All Rights Reserved
*
*   filename: polyphase.cpp
*   project : ISO/MPEG-Decoder
*   author  : Ben Allison
*   date    : 05 Jan 2011
*   contents/description: polyphase class, x86 SSE implementation
*
*
\***************************************************************************/

#include "../polyphase.h"
#include <xmmintrin.h>
/* ------------------------------------------------------------------------
*
* function to convert the polyphase window in unoptimized code to an SSE-friendly version
*
const float *winPtr = syn_f_window;
int          i,j=0;

float new_window[8][4][16];

for ( i=0; i<8; i++ )
{
syn_f_window_sse[i][0][j]=winPtr[0];
syn_f_window_sse[i][1][j]=0;
syn_f_window_sse[i][2][j]=winPtr[2];
syn_f_window_sse[i][3][j]=winPtr[3];

winPtr+=4;
}

for ( j=1; j<16; j++ )
{
for ( i=0; i<8; i++ )
{
syn_f_window_sse[i][0][j]=winPtr[0];
syn_f_window_sse[i][1][j]=winPtr[1];
syn_f_window_sse[i][2][j]=winPtr[2];
syn_f_window_sse[i][3][j]=winPtr[3];

winPtr += 4;
}
}

* ------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------*/

NALIGN(16) static const float syn_f_window_sse[8][4][16] =
{
	{
		{
			0,      -1.52587890625e-005f,      -1.52587890625e-005f,      -1.52587890625e-005f,
				-1.52587890625e-005f,      -1.52587890625e-005f,      -1.52587890625e-005f,       -3.0517578125e-005f,
				-3.0517578125e-005f,       -3.0517578125e-005f,       -3.0517578125e-005f,      -4.57763671875e-005f,
				-4.57763671875e-005f,        -6.103515625e-005f,        -6.103515625e-005f,      -7.62939453125e-005f,
		},
		{
			0,        0.000396728515625f,          0.0003662109375f,       0.0003204345703125f,
				0.0002899169921875f,       0.0002593994140625f,           0.000244140625f,        0.000213623046875f,
				0.0001983642578125f,       0.0001678466796875f,        0.000152587890625f,       0.0001373291015625f,
				0.0001220703125f,       0.0001068115234375f,       0.0001068115234375f,        9.1552734375e-005f,
			},
			{
				0.0004425048828125f,       0.0004730224609375f,       0.0005340576171875f,        0.000579833984375f,
					0.0006256103515625f,       0.0006866455078125f,       0.0007476806640625f,       0.0008087158203125f,
					0.000885009765625f,       0.0009613037109375f,         0.00103759765625f,       0.0011138916015625f,
					0.0012054443359375f,       0.0012969970703125f,       0.0013885498046875f,       0.0014801025390625f,
			},
			{
				0.0015869140625f,           0.003173828125f,        0.003082275390625f,         0.00299072265625f,
					0.002899169921875f,       0.0027923583984375f,           0.002685546875f,       0.0025787353515625f,
					0.0024566650390625f,        0.002349853515625f,       0.0022430419921875f,       0.0021209716796875f,
					0.00201416015625f,       0.0019073486328125f,       0.0017852783203125f,       0.0016937255859375f,
				},
	},
	{
		{
			0.0032501220703125f,        0.003326416015625f,        0.003387451171875f,       0.0034332275390625f,
				0.0034637451171875f,         0.00347900390625f,         0.00347900390625f,       0.0034637451171875f,
				0.00341796875f,       0.0033721923828125f,       0.0032806396484375f,           0.003173828125f,
				0.0030517578125f,       0.0028839111328125f,       0.0027008056640625f,       0.0024871826171875f,
		},
		{
			0,       0.0061187744140625f,       0.0052947998046875f,        0.004486083984375f,
				0.00372314453125f,       0.0030059814453125f,       0.0023345947265625f,       0.0016937255859375f,
				0.0010986328125f,         0.00054931640625f,        3.0517578125e-005f,      -0.0004425048828125f,
				-0.0008697509765625f,      -0.0012664794921875f,       -0.001617431640625f,      -0.0019378662109375f,
			},
			{
				0.0070037841796875f,       0.0079193115234375f,       0.0088653564453125f,       0.0098419189453125f,
					0.0108489990234375f,       0.0118865966796875f,           0.012939453125f,       0.0140228271484375f,
					0.0151214599609375f,          0.0162353515625f,       0.0173492431640625f,        0.018463134765625f,
					0.0195770263671875f,         0.02069091796875f,         0.02178955078125f,        0.022857666015625f,
			},
			{
				0.0239105224609375f,       0.0314788818359375f,            0.03173828125f,       0.0318450927734375f,
					0.0318145751953125f,       0.0316619873046875f,       0.0313873291015625f,           0.031005859375f,
					0.0305328369140625f,        0.029937744140625f,       0.0292816162109375f,        0.028533935546875f,
					0.0277252197265625f,       0.0268402099609375f,        0.025909423828125f,        0.024932861328125f,
				},
				},
				{
					{
						0.0310821533203125f,           0.030517578125f,            0.02978515625f,       0.0288848876953125f,
							0.027801513671875f,       0.0265350341796875f,         0.02508544921875f,       0.0234222412109375f,
							0.021575927734375f,               0.01953125f,       0.0172576904296875f,        0.014801025390625f,
							0.012115478515625f,       0.0092315673828125f,        0.006134033203125f,       0.0028228759765625f,
					},
					{
						0,         0.07305908203125f,       0.0675201416015625f,       0.0619964599609375f,
							0.0565338134765625f,       0.0511322021484375f,         0.04583740234375f,       0.0406341552734375f,
							0.035552978515625f,        0.030609130859375f,         0.02581787109375f,         0.02117919921875f,
							0.0167083740234375f,        0.012420654296875f,       0.0083160400390625f,            0.00439453125f,
						},
						{
							0.0786285400390625f,       0.0841827392578125f,       0.0897064208984375f,       0.0951690673828125f,
								0.1005401611328125f,       0.1058197021484375f,       0.1109466552734375f,       0.1159210205078125f,
								0.120697021484375f,       0.1252593994140625f,       0.1295623779296875f,       0.1335906982421875f,
								0.137298583984375f,       0.1406707763671875f,          0.1436767578125f,       0.1462554931640625f,
						},
						{
							0.1484222412109375f,        0.108856201171875f,          0.1165771484375f,         0.12347412109375f,
								0.12957763671875f,          0.1348876953125f,       0.1394500732421875f,       0.1432647705078125f,
								0.1463623046875f,        0.148773193359375f,       0.1504974365234375f,       0.1515960693359375f,
								0.152069091796875f,       0.1519622802734375f,         0.15130615234375f,        0.150115966796875f,
							},
				},
				{
					{
						0.100311279296875f,       0.0909271240234375f,          0.0806884765625f,       0.0695953369140625f,
							0.0576171875f,       0.0447845458984375f,       0.0310821533203125f,        0.016510009765625f,
							0.001068115234375f,       -0.015228271484375f,       -0.032379150390625f,        -0.05035400390625f,
							-0.0691680908203125f,       -0.088775634765625f,       -0.109161376953125f,        -0.13031005859375f,
					},
					{
						0,          0.5438232421875f,       0.5156097412109375f,       0.4874725341796875f,
							0.45947265625f,       0.4316558837890625f,        0.404083251953125f,        0.376800537109375f,
							0.3498687744140625f,       0.3233184814453125f,        0.297210693359375f,       0.2715911865234375f,
							0.2465057373046875f,         0.22198486328125f,         0.19805908203125f,       0.1747894287109375f,
						},
						{
							0.5720367431640625f,          0.6002197265625f,          0.6282958984375f,        0.656219482421875f,
								0.6839141845703125f,       0.7113189697265625f,        0.738372802734375f,       0.7650299072265625f,
								0.7912139892578125f,        0.816864013671875f,        0.841949462890625f,        0.866363525390625f,
								0.8900909423828125f,        0.913055419921875f,       0.9351959228515625f,         0.95648193359375f,
						},
						{
							0.9768524169921875f,           1.144287109375f,          1.1422119140625f,        1.138763427734375f,
								1.1339263916015625f,         1.12774658203125f,       1.1202239990234375f,       1.1113739013671875f,
								1.1012115478515625f,         1.08978271484375f,        1.077117919921875f,       1.0632171630859375f,
								1.04815673828125f,       1.0319366455078125f,        1.014617919921875f,        0.996246337890625f,
							},
							},
							{
								{
									1.144989013671875f,           1.144287109375f,          1.1422119140625f,        1.138763427734375f,
										1.1339263916015625f,         1.12774658203125f,       1.1202239990234375f,       1.1113739013671875f,
										1.1012115478515625f,         1.08978271484375f,        1.077117919921875f,       1.0632171630859375f,
										1.04815673828125f,       1.0319366455078125f,        1.014617919921875f,        0.996246337890625f,
								},
								{
									0,         -0.6002197265625f,         -0.6282958984375f,       -0.656219482421875f,
										-0.6839141845703125f,      -0.7113189697265625f,       -0.738372802734375f,      -0.7650299072265625f,
										-0.7912139892578125f,       -0.816864013671875f,       -0.841949462890625f,       -0.866363525390625f,
										-0.8900909423828125f,       -0.913055419921875f,      -0.9351959228515625f,        -0.95648193359375f,
									},
									{
										-0.5720367431640625f,         -0.5438232421875f,      -0.5156097412109375f,      -0.4874725341796875f,
											-0.45947265625f,      -0.4316558837890625f,       -0.404083251953125f,       -0.376800537109375f,
											-0.3498687744140625f,      -0.3233184814453125f,       -0.297210693359375f,      -0.2715911865234375f,
											-0.2465057373046875f,        -0.22198486328125f,        -0.19805908203125f,      -0.1747894287109375f,
									},
									{
										-0.1522064208984375f,       0.0909271240234375f,          0.0806884765625f,       0.0695953369140625f,
											0.0576171875f,       0.0447845458984375f,       0.0310821533203125f,        0.016510009765625f,
											0.001068115234375f,       -0.015228271484375f,       -0.032379150390625f,        -0.05035400390625f,
											-0.0691680908203125f,       -0.088775634765625f,       -0.109161376953125f,        -0.13031005859375f,
										},
							},
							{
								{
									0.100311279296875f,        0.108856201171875f,          0.1165771484375f,         0.12347412109375f,
										0.12957763671875f,          0.1348876953125f,       0.1394500732421875f,       0.1432647705078125f,
										0.1463623046875f,        0.148773193359375f,       0.1504974365234375f,       0.1515960693359375f,
										0.152069091796875f,       0.1519622802734375f,         0.15130615234375f,        0.150115966796875f,
								},
								{
									0,      -0.0841827392578125f,      -0.0897064208984375f,      -0.0951690673828125f,
										-0.1005401611328125f,      -0.1058197021484375f,      -0.1109466552734375f,      -0.1159210205078125f,
										-0.120697021484375f,      -0.1252593994140625f,      -0.1295623779296875f,      -0.1335906982421875f,
										-0.137298583984375f,      -0.1406707763671875f,         -0.1436767578125f,      -0.1462554931640625f,
									},
									{
										-0.0786285400390625f,        -0.07305908203125f,      -0.0675201416015625f,      -0.0619964599609375f,
											-0.0565338134765625f,      -0.0511322021484375f,        -0.04583740234375f,      -0.0406341552734375f,
											-0.035552978515625f,       -0.030609130859375f,        -0.02581787109375f,        -0.02117919921875f,
											-0.0167083740234375f,       -0.012420654296875f,      -0.0083160400390625f,           -0.00439453125f,
									},
									{
										-0.0006866455078125f,           0.030517578125f,            0.02978515625f,       0.0288848876953125f,
											0.027801513671875f,       0.0265350341796875f,         0.02508544921875f,       0.0234222412109375f,
											0.021575927734375f,               0.01953125f,       0.0172576904296875f,        0.014801025390625f,
											0.012115478515625f,       0.0092315673828125f,        0.006134033203125f,       0.0028228759765625f,
										},
										},
										{
											{
												0.0310821533203125f,       0.0314788818359375f,            0.03173828125f,       0.0318450927734375f,
													0.0318145751953125f,       0.0316619873046875f,       0.0313873291015625f,           0.031005859375f,
													0.0305328369140625f,        0.029937744140625f,       0.0292816162109375f,        0.028533935546875f,
													0.0277252197265625f,       0.0268402099609375f,        0.025909423828125f,        0.024932861328125f,
											},
											{
												0,      -0.0079193115234375f,      -0.0088653564453125f,      -0.0098419189453125f,
													-0.0108489990234375f,      -0.0118865966796875f,          -0.012939453125f,      -0.0140228271484375f,
													-0.0151214599609375f,         -0.0162353515625f,      -0.0173492431640625f,       -0.018463134765625f,
													-0.0195770263671875f,        -0.02069091796875f,        -0.02178955078125f,       -0.022857666015625f,
												},
												{
													-0.0070037841796875f,      -0.0061187744140625f,      -0.0052947998046875f,       -0.004486083984375f,
														-0.00372314453125f,      -0.0030059814453125f,      -0.0023345947265625f,      -0.0016937255859375f,
														-0.0010986328125f,        -0.00054931640625f,       -3.0517578125e-005f,       0.0004425048828125f,
														0.0008697509765625f,       0.0012664794921875f,        0.001617431640625f,       0.0019378662109375f,
												},
												{
													0.002227783203125f,        0.003326416015625f,        0.003387451171875f,       0.0034332275390625f,
														0.0034637451171875f,         0.00347900390625f,         0.00347900390625f,       0.0034637451171875f,
														0.00341796875f,       0.0033721923828125f,       0.0032806396484375f,           0.003173828125f,
														0.0030517578125f,       0.0028839111328125f,       0.0027008056640625f,       0.0024871826171875f,
													},
										},
										{
											{
												0.0032501220703125f,           0.003173828125f,        0.003082275390625f,         0.00299072265625f,
													0.002899169921875f,       0.0027923583984375f,           0.002685546875f,       0.0025787353515625f,
													0.0024566650390625f,        0.002349853515625f,       0.0022430419921875f,       0.0021209716796875f,
													0.00201416015625f,       0.0019073486328125f,       0.0017852783203125f,       0.0016937255859375f,
											},
											{
												0,      -0.0004730224609375f,      -0.0005340576171875f,       -0.000579833984375f,
													-0.0006256103515625f,      -0.0006866455078125f,      -0.0007476806640625f,      -0.0008087158203125f,
													-0.000885009765625f,      -0.0009613037109375f,        -0.00103759765625f,      -0.0011138916015625f,
													-0.0012054443359375f,      -0.0012969970703125f,      -0.0013885498046875f,      -0.0014801025390625f,
												},
												{
													-0.0004425048828125f,       -0.000396728515625f,         -0.0003662109375f,      -0.0003204345703125f,
														-0.0002899169921875f,      -0.0002593994140625f,          -0.000244140625f,       -0.000213623046875f,
														-0.0001983642578125f,      -0.0001678466796875f,       -0.000152587890625f,      -0.0001373291015625f,
														-0.0001220703125f,      -0.0001068115234375f,      -0.0001068115234375f,       -9.1552734375e-005f,
												},
												{
													-7.62939453125e-005f,      -1.52587890625e-005f,      -1.52587890625e-005f,      -1.52587890625e-005f,
														-1.52587890625e-005f,      -1.52587890625e-005f,      -1.52587890625e-005f,       -3.0517578125e-005f,
														-3.0517578125e-005f,       -3.0517578125e-005f,       -3.0517578125e-005f,      -4.57763671875e-005f,
														-4.57763671875e-005f,        -6.103515625e-005f,        -6.103515625e-005f,      -7.62939453125e-005f,
													},
													},

};

NALIGN(16) static const float cost32_c0[] =
{
	0.50060299823520f,  0.50547095989754f,  0.51544730992262f,  0.53104259108978f,
	0.55310389603444f,  0.58293496820613f,  0.62250412303566f,  0.67480834145501f,
	0.74453627100230f,  0.83934964541553f,  0.97256823786196f,  1.16943993343288f,
	1.48416461631417f,  2.05778100995341f,  3.40760841846872f, 10.19000812354803f
};

/* ------------------------------------------------------------------------*/

NALIGN(16) static const float cost32_c1[] =
{
	0.50241928618816f,  0.52249861493969f,  0.56694403481636f,  0.64682178335999f,
	0.78815462345125f,  1.06067768599035f,  1.72244709823833f,  5.10114861868916f
};

/* ------------------------------------------------------------------------*/

NALIGN(16) static const float cost32_c2[] =
{
	0.50979557910416f,  0.60134488693505f,  0.89997622313642f,  2.56291544774151f
};

/* ------------------------------------------------------------------------*/

NALIGN(16) static const float cost32_c3[] =
{
	0.54119610014620f,  1.30656296487638f
};

/* ------------------------------------------------------------------------*/

NALIGN(16) static const float cost32_c4[] =
{
	0.70710678118655f
};

/*-------------------------------------------------------------------------*/

static void cost32(const float *vec,float *f_vec);

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C P o l y p h a s e
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CPolyphase::CPolyphase (const MPEG_INFO &_info) : info (_info)
{
	Init() ;
}

//-------------------------------------------------------------------------*
//   Init
//-------------------------------------------------------------------------*

void CPolyphase::Init(void)
{
	int i,j;

	for ( j=0; j<2; j++ )
		for ( i=0; i<HAN_SIZE; i++ )
			syn_buf[j][i] = 0.0f;

	bufOffset = 32;
}

//-------------------------------------------------------------------------*
//   Apply (float)
//-------------------------------------------------------------------------*

float *CPolyphase::Apply(POLYSPECTRUM &sample, float *pPcm, int frms)
{
	int nChannels    = info.stereo;
	int nIncrement   = 16<<nChannels;
	int fShortWindow = (info.stereo==2) ? 1 : 0;

	int k;

	if (nChannels == 1) // mono
	{
		for (k=0;k<frms;k++)
		{
			bufOffset = (bufOffset-32)&(HAN_SIZE-1);
			cost32(sample[0][k], &(syn_buf[0][bufOffset]));
			window_band_m(bufOffset, pPcm);
			pPcm += nIncrement;
		}
	}
	else
	{
		for (k=0;k<frms;k++)
		{
			bufOffset = (bufOffset-32)&(HAN_SIZE-1);
			cost32(sample[0][k], &(syn_buf[0][bufOffset]));
			cost32(sample[1][k], &(syn_buf[1][bufOffset]));
			window_band_s(bufOffset, pPcm);
			pPcm += nIncrement;
		}
	}

	return pPcm;
}

/*-------------------------------------------------------------------------*/
void CPolyphase::window_band_m(int bufOffset, float *out_samples) const
{
	int bufPtr;

	float extra1=0,extra2=0;
	__m128 sum1_0=_mm_setzero_ps(), sum1_1=_mm_setzero_ps(), sum1_2=_mm_setzero_ps(), sum1_3=_mm_setzero_ps();
	__m128 sum2_0=_mm_setzero_ps(),sum2_1=_mm_setzero_ps(), sum2_2=_mm_setzero_ps(), sum2_3=_mm_setzero_ps();
	__m128 xmm_left, xmm_right;
	__m128 xmm_win0, xmm_win1;

	/* sum 0 and sum 16, 8, 4 (full, half, quarter spectrum) */
	bufPtr = bufOffset;

	for (int i=0; i<8; i++)
	{
		extra1 += syn_buf[0][bufPtr+16] * syn_f_window_sse[i][0][0];
		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][0]);

		sum1_0 = _mm_add_ps(sum1_0, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_0 = _mm_add_ps(sum2_0, _mm_mul_ps(xmm_left, xmm_win1));

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][4]);

		sum1_1 = _mm_add_ps(sum1_1, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_1 = _mm_add_ps(sum2_1, _mm_mul_ps(xmm_left, xmm_win1));

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+8]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][8]);

		sum1_2 = _mm_add_ps(sum1_2, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_2 = _mm_add_ps(sum2_2, _mm_mul_ps(xmm_left, xmm_win1));

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][12]);

		sum1_3 = _mm_add_ps(sum1_3, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_3 = _mm_add_ps(sum2_3, _mm_mul_ps(xmm_left, xmm_win1));

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);

		// special part for 0
		extra1 += syn_buf[0][bufPtr+16] * syn_f_window_sse[i][2][0];
		extra2 += syn_buf[0][bufPtr] * syn_f_window_sse[i][3][0];

		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][0]);

		sum1_0 = _mm_add_ps(sum1_0, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_0 = _mm_add_ps(sum2_0, _mm_mul_ps(xmm_left, xmm_win1));

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][4]);

		sum1_1 = _mm_add_ps(sum1_1, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_1 = _mm_add_ps(sum2_1, _mm_mul_ps(xmm_left, xmm_win1));

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][8]);

		sum1_2 = _mm_add_ps(sum1_2, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_2 = _mm_add_ps(sum2_2, _mm_mul_ps(xmm_left, xmm_win1));

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][12]);

		sum1_3 = _mm_add_ps(sum1_3, _mm_mul_ps(xmm_left, xmm_win0));
		sum2_3 = _mm_add_ps(sum2_3, _mm_mul_ps(xmm_left, xmm_win1));

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);
	}
	float tmp = 1.0f/32768.0f;
	__m128 div32768 = _mm_load1_ps(&tmp);

	out_samples[16] = extra2*tmp;

	sum1_0 = _mm_mul_ps(sum1_0, div32768);
	sum2_0 = _mm_mul_ps(sum2_0, div32768);

	_mm_storeu_ps(out_samples, sum1_0);
	sum2_0 = _mm_shuffle_ps(sum2_0, sum2_0, 0x6C); // [ 0 3 2 1]
	_mm_storeu_ps(out_samples+28, sum2_0);

	out_samples[0] = extra1*tmp;

	sum1_1 = _mm_mul_ps(sum1_1, div32768);
	sum2_1 = _mm_mul_ps(sum2_1, div32768);
	_mm_storeu_ps(out_samples+4, sum1_1);
	sum2_1 = _mm_shuffle_ps(sum2_1, sum2_1, 0x1B); // reverse
	_mm_storeu_ps(out_samples+25, sum2_1);

	sum1_2 = _mm_mul_ps(sum1_2, div32768);
	sum2_2 = _mm_mul_ps(sum2_2, div32768);
	_mm_storeu_ps(out_samples+8, sum1_2);
	sum2_2 = _mm_shuffle_ps(sum2_2, sum2_2, 0x1B); // reverse
	_mm_storeu_ps(out_samples+21, sum2_2);

	sum1_3 = _mm_mul_ps(sum1_3, div32768);
	sum2_3 = _mm_mul_ps(sum2_3, div32768);
	_mm_storeu_ps(out_samples+12, sum1_3);
	sum2_3 = _mm_shuffle_ps(sum2_3, sum2_3, 0x1B); // reverse
	_mm_storeu_ps(out_samples+17, sum2_3);
}

/*-------------------------------------------------------------------------*/

void CPolyphase::window_band_s(int bufOffset, float *out_samples) const
{
	int          j,bufPtr;
	NALIGN(16) float sum2l[4],sum2r[4];
	float extra_left, extra_right;
	float extra_left2, extra_right2;

	__m128 sum1l_0, sum1l_1, sum1l_2, sum1l_3;
	__m128 sum1r_0,sum1r_1, sum1r_2, sum1r_3;
	__m128 sum2l_0,sum2l_1, sum2l_2, sum2l_3;
	__m128 sum2r_0,sum2r_1, sum2r_2, sum2r_3;
	__m128 xmm_left, xmm_right;
	__m128 xmm_win0, xmm_win1;
	__m128 unpacked;

	/* sum 0 and sum 16, 8, 4 (full, half, quarter spectrum) */
	bufPtr = bufOffset;

	//for (int i=0; i<8; i++)
	{
		extra_left = syn_buf[0][bufPtr+16] * syn_f_window_sse[0][0][0];
		extra_right = syn_buf[1][bufPtr+16] * syn_f_window_sse[0][0][0];

		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][0][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][1][0]);

		sum1l_0 = _mm_mul_ps(xmm_left, xmm_win0);
		sum1r_0 = _mm_mul_ps(xmm_right, xmm_win0);
		sum2l_0 = _mm_mul_ps(xmm_left, xmm_win1);
		sum2r_0 = _mm_mul_ps(xmm_right, xmm_win1);

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+4]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][0][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][1][4]);

		sum1l_1 = _mm_mul_ps(xmm_left, xmm_win0);
		sum1r_1 = _mm_mul_ps(xmm_right, xmm_win0);
		sum2l_1 = _mm_mul_ps(xmm_left, xmm_win1);
		sum2r_1 = _mm_mul_ps(xmm_right, xmm_win1);

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+8]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][0][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][1][8]);

		sum1l_2 = _mm_mul_ps(xmm_left, xmm_win0);
		sum1r_2 = _mm_mul_ps(xmm_right, xmm_win0);
		sum2l_2 = _mm_mul_ps(xmm_left, xmm_win1);
		sum2r_2 = _mm_mul_ps(xmm_right, xmm_win1);

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+12]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][0][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][1][12]);

		sum1l_3 = _mm_mul_ps(xmm_left, xmm_win0);
		sum1r_3 = _mm_mul_ps(xmm_right, xmm_win0);
		sum2l_3 = _mm_mul_ps(xmm_left, xmm_win1);
		sum2r_3 = _mm_mul_ps(xmm_right, xmm_win1);

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);

		// special part for 0
		extra_left += syn_buf[0][bufPtr+16] * syn_f_window_sse[0][2][0];
		extra_right += syn_buf[1][bufPtr+16] * syn_f_window_sse[0][2][0];
		extra_left2 = syn_buf[0][bufPtr] * syn_f_window_sse[0][3][0];
		extra_right2 = syn_buf[1][bufPtr] * syn_f_window_sse[0][3][0];
		//sum1l_0 = _mm_add_ps(sum1l_0, _mm_load_ss(&extra_left));
		//sum1r_0 = _mm_add_ps(sum1r_0, _mm_load_ss(&extra_right));

		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][2][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][3][0]);

		sum1l_0 = _mm_add_ps(sum1l_0, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_0 = _mm_add_ps(sum1r_0, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_0 = _mm_add_ps(sum2l_0, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_0 = _mm_add_ps(sum2r_0, _mm_mul_ps(xmm_right, xmm_win1));

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+4]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][2][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][3][4]);

		sum1l_1 = _mm_add_ps(sum1l_1, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_1 = _mm_add_ps(sum1r_1, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_1 = _mm_add_ps(sum2l_1, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_1 = _mm_add_ps(sum2r_1, _mm_mul_ps(xmm_right, xmm_win1));

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+8]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][2][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][3][8]);

		sum1l_2 = _mm_add_ps(sum1l_2, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_2 = _mm_add_ps(sum1r_2, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_2 = _mm_add_ps(sum2l_2, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_2 = _mm_add_ps(sum2r_2, _mm_mul_ps(xmm_right, xmm_win1));

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+12]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[0][2][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[0][3][12]);

		sum1l_3 = _mm_add_ps(sum1l_3, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_3 = _mm_add_ps(sum1r_3, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_3 = _mm_add_ps(sum2l_3, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_3 = _mm_add_ps(sum2r_3, _mm_mul_ps(xmm_right, xmm_win1));

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);
	}

	for (int i=1; i<8; i++)
	{
		extra_left += syn_buf[0][bufPtr+16] * syn_f_window_sse[i][0][0];
		extra_right += syn_buf[1][bufPtr+16] * syn_f_window_sse[i][0][0];

		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][0]);

		sum1l_0 = _mm_add_ps(sum1l_0, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_0 = _mm_add_ps(sum1r_0, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_0 = _mm_add_ps(sum2l_0, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_0 = _mm_add_ps(sum2r_0, _mm_mul_ps(xmm_right, xmm_win1));

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+4]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][4]);

		sum1l_1 = _mm_add_ps(sum1l_1, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_1 = _mm_add_ps(sum1r_1, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_1 = _mm_add_ps(sum2l_1, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_1 = _mm_add_ps(sum2r_1, _mm_mul_ps(xmm_right, xmm_win1));

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+8]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][8]);

		sum1l_2 = _mm_add_ps(sum1l_2, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_2 = _mm_add_ps(sum1r_2, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_2 = _mm_add_ps(sum2l_2, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_2 = _mm_add_ps(sum2r_2, _mm_mul_ps(xmm_right, xmm_win1));

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+16+12]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+16+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][0][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][1][12]);

		sum1l_3 = _mm_add_ps(sum1l_3, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_3 = _mm_add_ps(sum1r_3, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_3 = _mm_add_ps(sum2l_3, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_3 = _mm_add_ps(sum2r_3, _mm_mul_ps(xmm_right, xmm_win1));

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);

		// special part for 0
		extra_left += syn_buf[0][bufPtr+16] * syn_f_window_sse[i][2][0];
		extra_right += syn_buf[1][bufPtr+16] * syn_f_window_sse[i][2][0];
		extra_left2 += syn_buf[0][bufPtr] * syn_f_window_sse[i][3][0];
		extra_right2 += syn_buf[1][bufPtr] * syn_f_window_sse[i][3][0];
		//sum1l_0 = _mm_add_ps(sum1l_0, _mm_load_ss(&extra_left));
		//sum1r_0 = _mm_add_ps(sum1r_0, _mm_load_ss(&extra_right));

		// 0-4
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][0]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][0]);

		sum1l_0 = _mm_add_ps(sum1l_0, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_0 = _mm_add_ps(sum1r_0, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_0 = _mm_add_ps(sum2l_0, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_0 = _mm_add_ps(sum2r_0, _mm_mul_ps(xmm_right, xmm_win1));

		// 4-7
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+4]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+4]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][4]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][4]);

		sum1l_1 = _mm_add_ps(sum1l_1, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_1 = _mm_add_ps(sum1r_1, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_1 = _mm_add_ps(sum2l_1, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_1 = _mm_add_ps(sum2r_1, _mm_mul_ps(xmm_right, xmm_win1));

		// 8-11
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+8]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+8]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][8]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][8]);

		sum1l_2 = _mm_add_ps(sum1l_2, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_2 = _mm_add_ps(sum1r_2, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_2 = _mm_add_ps(sum2l_2, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_2 = _mm_add_ps(sum2r_2, _mm_mul_ps(xmm_right, xmm_win1));

		// 12-16
		xmm_left = _mm_load_ps(&syn_buf[0][bufPtr+12]);
		xmm_right = _mm_load_ps(&syn_buf[1][bufPtr+12]);
		xmm_win0 = _mm_load_ps(&syn_f_window_sse[i][2][12]);
		xmm_win1 = _mm_load_ps(&syn_f_window_sse[i][3][12]);

		sum1l_3 = _mm_add_ps(sum1l_3, _mm_mul_ps(xmm_left, xmm_win0));
		sum1r_3 = _mm_add_ps(sum1r_3, _mm_mul_ps(xmm_right, xmm_win0));
		sum2l_3 = _mm_add_ps(sum2l_3, _mm_mul_ps(xmm_left, xmm_win1));
		sum2r_3 = _mm_add_ps(sum2r_3, _mm_mul_ps(xmm_right, xmm_win1));

		bufPtr = (bufPtr+32)&(HAN_SIZE-1);
	}


	float tmp = 1.0f/32768.0f;
	__m128 div32768 = _mm_load1_ps(&tmp);

	out_samples[32] = extra_left2*tmp;	
	out_samples[32+1] =extra_right2*tmp;

	sum1l_0 = _mm_mul_ps(sum1l_0, div32768);
	sum1r_0 = _mm_mul_ps(sum1r_0, div32768);
	unpacked=_mm_unpacklo_ps(sum1l_0, sum1r_0);
	_mm_storeu_ps(&out_samples[0], unpacked);
	unpacked=_mm_unpackhi_ps(sum1l_0, sum1r_0);
	_mm_storeu_ps(&out_samples[4], unpacked);

	out_samples[0] = extra_left*tmp;
	out_samples[1] = extra_right*tmp;

	sum2l_0 = _mm_mul_ps(sum2l_0, div32768);
	sum2r_0 = _mm_mul_ps(sum2r_0, div32768);
	_mm_store_ps(sum2l, sum2l_0);
	_mm_store_ps(sum2r, sum2r_0);

	for (j=1;j<4;j++)
	{
		out_samples[(32-j)*2] = float(sum2l[j]);		
		out_samples[(32-j)*2+1] = float(sum2r[j]);
	}

	/* --- */
	sum1l_1 = _mm_mul_ps(sum1l_1, div32768);
	sum1r_1 = _mm_mul_ps(sum1r_1, div32768);
	unpacked=_mm_unpacklo_ps(sum1l_1, sum1r_1);
	_mm_storeu_ps(&out_samples[8], unpacked);
	unpacked=_mm_unpackhi_ps(sum1l_1, sum1r_1);
	_mm_storeu_ps(&out_samples[12], unpacked);

	sum2l_1 = _mm_mul_ps(sum2l_1, div32768);
	sum2r_1 = _mm_mul_ps(sum2r_1, div32768);
	unpacked=_mm_unpackhi_ps(sum2r_1, sum2l_1);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[50], unpacked);
	unpacked=_mm_unpacklo_ps(sum2r_1, sum2l_1);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[54], unpacked);

	/* --- */
	sum1l_2 = _mm_mul_ps(sum1l_2, div32768);
	sum1r_2 = _mm_mul_ps(sum1r_2, div32768);
	unpacked=_mm_unpacklo_ps(sum1l_2, sum1r_2);
	_mm_storeu_ps(&out_samples[16], unpacked);
	unpacked=_mm_unpackhi_ps(sum1l_2, sum1r_2);
	_mm_storeu_ps(&out_samples[20], unpacked);

	sum2l_2 = _mm_mul_ps(sum2l_2, div32768);
	sum2r_2 = _mm_mul_ps(sum2r_2, div32768);
	unpacked=_mm_unpackhi_ps(sum2r_2, sum2l_2);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[42], unpacked);
	unpacked=_mm_unpacklo_ps(sum2r_2, sum2l_2);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[46], unpacked);

	/* --- */
	sum1l_3 = _mm_mul_ps(sum1l_3, div32768);
	sum1r_3 = _mm_mul_ps(sum1r_3, div32768);
	unpacked=_mm_unpacklo_ps(sum1l_3, sum1r_3);
	_mm_storeu_ps(&out_samples[24], unpacked);
	unpacked=_mm_unpackhi_ps(sum1l_3, sum1r_3);
	_mm_storeu_ps(&out_samples[28], unpacked);

	sum2l_3 = _mm_mul_ps(sum2l_3, div32768);
	sum2r_3 = _mm_mul_ps(sum2r_3, div32768);
	unpacked=_mm_unpackhi_ps(sum2r_3, sum2l_3);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[34], unpacked);
	unpacked=_mm_unpacklo_ps(sum2r_3, sum2l_3);
	unpacked = _mm_shuffle_ps(unpacked, unpacked, 0x1B); // reverse
	_mm_storeu_ps(&out_samples[38], unpacked);
}

/*-------------------------------------------------------------------------*/
static void cost32(const float *vec,float *f_vec)
{
	NALIGN(16) float b0[4], b1[4], b2[4], b3[4];
	__m128 a0, a1, xmm_b0, xmm_b1, xmm_b2, xmm_b3, c0, c1, d0;
	__m128 r0, r1, r2, r3; // matrix rotation temp variables

	__m128 tmp0_0, tmp0_1, tmp0_2, tmp0_3;
	__m128 tmp1_0, tmp1_1;
	__m128 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
	__m128 load0, load1, load2, load3;

	__m128 final0, final1, final2, final3;

	vec0 = _mm_load_ps(vec); // 0-3
	vec1 = _mm_load_ps(vec+4); // 4-7
	vec2 = _mm_load_ps(vec+8); // 8-11
	vec3 = _mm_load_ps(vec+12); // 12-15

	vec4 = _mm_load_ps(vec+16); // 16-19
	vec5 = _mm_load_ps(vec+20); // 20-23
	vec6 = _mm_load_ps(vec+24); // 24-27
	vec7 = _mm_load_ps(vec+28); // 28-31

	// reverse positions of 16-31
	vec7 = _mm_shuffle_ps(vec7, vec7, 0x1B); // vec7 now holds [31 30 29 28]
	vec6 = _mm_shuffle_ps(vec6, vec6, 0x1B); // vec6 now holds [27 26 25 24]
	vec5 = _mm_shuffle_ps(vec5, vec5, 0x1B); // vec5 now holds [23 22 21 20]
	vec4 = _mm_shuffle_ps(vec4, vec4, 0x1B); // vec4 now holds [19 18 17 16]

	// add vec[n] + vec[31-n]
	tmp0_0 = _mm_add_ps(vec7, vec0);
	tmp0_1 = _mm_add_ps(vec6, vec1);
	tmp0_2 = _mm_add_ps(vec5, vec2);
	tmp0_3 = _mm_add_ps(vec4, vec3);

	// reverse positions of 8-15
	tmp0_3 = _mm_shuffle_ps(tmp0_3, tmp0_3, 0x1B); // tmp0_2 now holds [15 14 13 12]
	tmp0_2 = _mm_shuffle_ps(tmp0_2, tmp0_2, 0x1B); // tmp0_3 now holds [11 10 9 8]

	// add vec[n] + vec[15-n]
	tmp1_0 = _mm_add_ps(tmp0_0, tmp0_3);
	tmp1_1 = _mm_add_ps(tmp0_1, tmp0_2);

	// reverse positions of 4-7
	tmp1_1 = _mm_shuffle_ps(tmp1_1, tmp1_1, 0x1B); // tmp1_1 now holds [7 6 5 4]

	// add vec[n] + vec[7-n]
	final0 = _mm_add_ps(tmp1_0, tmp1_1);

	final1 = _mm_sub_ps(tmp1_0, tmp1_1);
	load0 = _mm_load_ps(cost32_c2);
	final1 = _mm_mul_ps(final1, load0);

	load0 = _mm_load_ps(cost32_c1);
	load1 = _mm_load_ps(cost32_c1+4);

	tmp1_0 = _mm_sub_ps(tmp0_0, tmp0_3);
	tmp1_0 = _mm_mul_ps(tmp1_0, load0);
	tmp1_1 = _mm_sub_ps(tmp0_1, tmp0_2);
	tmp1_1 = _mm_mul_ps(tmp1_1, load1);
	tmp1_1 = _mm_shuffle_ps(tmp1_1, tmp1_1, 0x1B); // tmp0_3 now holds [7 6 5 4]
	final2 = _mm_add_ps(tmp1_0, tmp1_1);

	load0 = _mm_load_ps(cost32_c2);
	final3 = _mm_sub_ps(tmp1_0, tmp1_1);
	final3 = _mm_mul_ps(final3, load0);

	// rotate 4x4 matrix
	r0 = _mm_unpacklo_ps(final0, final2); // 00 20 01 21
	r1 = _mm_unpacklo_ps(final1, final3); // 10 30 11 31
	r2 = _mm_unpackhi_ps(final0, final2); // 02 22 03 23
	r3 = _mm_unpackhi_ps(final1, final3); // 12 32 13 33
	final0 = _mm_unpacklo_ps(r0, r1); // 00 10 20 30
	final1 = _mm_unpackhi_ps(r0, r1); // 01 11 21 31
	final2 = _mm_unpacklo_ps(r2, r3); // 02 12 22 32
	final3 = _mm_unpackhi_ps(r2, r3); // 03 13 23 33

	a0 = _mm_add_ps(final0, final3);
	a1 = _mm_add_ps(final1, final2);

	xmm_b0 = _mm_add_ps(a0, a1);
	xmm_b2 = _mm_mul_ps(_mm_sub_ps(a0, a1), _mm_load1_ps(&cost32_c4[0]));

	c0 = _mm_mul_ps(_mm_sub_ps(final0, final3), _mm_load1_ps(&cost32_c3[0]));
	c1 = _mm_mul_ps(_mm_sub_ps(final1, final2), _mm_load1_ps(&cost32_c3[1]));

	d0 = _mm_add_ps(c0, c1);
	xmm_b3 = _mm_mul_ps(_mm_sub_ps(c0, c1), _mm_load1_ps(&cost32_c4[0]));
	xmm_b1 = _mm_add_ps(d0, xmm_b3);

	_mm_store_ps(b0, xmm_b0);
	_mm_store_ps(b1, xmm_b1);
	_mm_store_ps(b2, xmm_b2);
	_mm_store_ps(b3, xmm_b3);

	f_vec[0] =  b0[0];
	f_vec[16] = b2[0];
	f_vec[8] =  b1[0];
	f_vec[24] = b3[0];

	f_vec[12] = b0[1]+b1[1];
	f_vec[4] =  b1[1]+b2[1];
	f_vec[20] = b2[1]+b3[1];
	f_vec[28] = b3[1];

	f_vec[14] = b0[2]+b0[3]+b1[3];
	f_vec[10] = b0[3]+b1[3]+b1[2];
	f_vec[6] =  b1[2]+b1[3]+b2[3];
	f_vec[2] =  b1[3]+b2[3]+b2[2];
	f_vec[18] = b2[2]+b2[3]+b3[3];
	f_vec[22] = b2[3]+b3[3]+b3[2];
	f_vec[26] = b3[2]+b3[3];
	f_vec[30] = b3[3];

	/*  --- Odd Terms --- */
	load0 = _mm_load_ps(cost32_c0);
	load1 = _mm_load_ps(cost32_c0+4);
	load2 = _mm_load_ps(cost32_c0+8);
	load3 = _mm_load_ps(cost32_c0+12);

	// (vec[n] - vec[31-n]) * cost32_c0[n]
	tmp0_0 = _mm_sub_ps(vec0, vec7);
	tmp0_0 = _mm_mul_ps(tmp0_0, load0);
	tmp0_1 = _mm_sub_ps(vec1, vec6);
	tmp0_1 = _mm_mul_ps(tmp0_1, load1);
	tmp0_2 = _mm_sub_ps(vec2, vec5);
	tmp0_2 = _mm_mul_ps(tmp0_2, load2);
	tmp0_3 = _mm_sub_ps(vec3, vec4);
	tmp0_3 = _mm_mul_ps(tmp0_3, load3);

	// reverse positions of 8-15
	tmp0_3 = _mm_shuffle_ps(tmp0_3, tmp0_3, 0x1B); // tmp0_3 now holds [15 14 13 12]
	tmp0_2 = _mm_shuffle_ps(tmp0_2, tmp0_2, 0x1B); // tmp0_2 now holds [11 10 9 8]

	// add vec[n] + vec[15-n]
	tmp1_0 = _mm_add_ps(tmp0_0, tmp0_3);
	tmp1_1 = _mm_add_ps(tmp0_1, tmp0_2);

	// reverse positions of 4-7
	tmp1_1 = _mm_shuffle_ps(tmp1_1, tmp1_1, 0x1B); // tmp1_1 now holds [7 6 5 4]

	// add vec[n] + vec[7-n]
	final0 = _mm_add_ps(tmp1_0, tmp1_1);

	load0 = _mm_load_ps(cost32_c2);
	final1 = _mm_sub_ps(tmp1_0, tmp1_1);
	final1 = _mm_mul_ps(final1, load0);

	load0 = _mm_load_ps(cost32_c1);
	load1 = _mm_load_ps(cost32_c1 + 4);
	tmp1_0 = _mm_sub_ps(tmp0_0, tmp0_3);
	tmp1_0 = _mm_mul_ps(tmp1_0, load0);
	tmp1_1 = _mm_sub_ps(tmp0_1, tmp0_2);
	tmp1_1 = _mm_mul_ps(tmp1_1, load1);

	// reverse positions of 4-7
	tmp1_1 = _mm_shuffle_ps(tmp1_1, tmp1_1, 0x1B); // tmp1_1 now holds [7 6 5 4]
	final2 = _mm_add_ps(tmp1_0, tmp1_1);

	load0 = _mm_load_ps(cost32_c2);
	final3 = _mm_sub_ps(tmp1_0, tmp1_1);
	final3 = _mm_mul_ps(final3, load0);

	// rotate 4x4 matrix
	r0 = _mm_unpacklo_ps(final0, final2); // 00 20 01 21
	r1 = _mm_unpacklo_ps(final1, final3); // 10 30 11 31
	r2 = _mm_unpackhi_ps(final0, final2); // 02 22 03 23
	r3 = _mm_unpackhi_ps(final1, final3); // 12 32 13 33
	final0 = _mm_unpacklo_ps(r0, r1); // 00 10 20 30
	final1 = _mm_unpackhi_ps(r0, r1); // 01 11 21 31
	final2 = _mm_unpacklo_ps(r2, r3); // 02 12 22 32
	final3 = _mm_unpackhi_ps(r2, r3); // 03 13 23 33

	a0 = _mm_add_ps(final0, final3);
	a1 = _mm_add_ps(final1, final2);

	xmm_b0 = _mm_add_ps(a0, a1);
	xmm_b2 = _mm_mul_ps(_mm_sub_ps(a0, a1), _mm_load1_ps(&cost32_c4[0]));

	c0 = _mm_mul_ps(_mm_sub_ps(final0, final3), _mm_load1_ps(&cost32_c3[0]));
	c1 = _mm_mul_ps(_mm_sub_ps(final1, final2), _mm_load1_ps(&cost32_c3[1]));

	d0 = _mm_add_ps(c0, c1);
	xmm_b3 = _mm_mul_ps(_mm_sub_ps(c0, c1), _mm_load1_ps(&cost32_c4[0]));
	xmm_b1 = _mm_add_ps(d0, xmm_b3);

	_mm_store_ps(b0, xmm_b0);
	_mm_store_ps(b1, xmm_b1);
	_mm_store_ps(b2, xmm_b2);
	_mm_store_ps(b3, xmm_b3);

	f_vec[15] = b0[0]+b0[2]+b0[3]+b1[3];
	f_vec[13] = b0[2]+b0[3]+b1[3]+b0[1]+b1[1];
	f_vec[11] = b0[1]+b1[1]+b0[3]+b1[3]+b1[2];
	f_vec[9] =  b0[3]+b1[3]+b1[2]+b1[0];
	f_vec[7] =  b1[0]+b1[2]+b1[3]+b2[3];
	f_vec[5] =  b1[2]+b1[3]+b2[3]+b1[1]+b2[1];
	f_vec[3] =  b1[1]+b2[1]+b1[3]+b2[3]+b2[2];
	f_vec[1] =  b1[3]+b2[3]+b2[2]+b2[0];
	f_vec[17] = b2[0]+b2[2]+b2[3]+b3[3];
	f_vec[19] = b2[2]+b2[3]+b3[3]+b2[1]+b3[1];
	f_vec[21] = b2[1]+b3[1]+b2[3]+b3[3]+b3[2];
	f_vec[23] = b2[3]+b3[3]+b3[2]+b3[0];
	f_vec[25] = b3[0]+b3[2]+b3[3];
	f_vec[27] = b3[2]+b3[3]+b3[1];
	f_vec[29] = b3[1]+b3[3];
	f_vec[31] = b3[3];
}

static void Rotate4x4(float *output, const float *input)
{
	__m128 p0, p1, p2, p3;
	__m128 r0, r1, r2, r3;
	__m128 t0, t1, t2, t3;

	// load registers in vertical mode, we'll rotate them next
	p0 = _mm_load_ps(&input[0*SSLIMIT]);  // 00 01 02 03
	p1 = _mm_loadu_ps(&input[1*SSLIMIT]); // 10 11 12 13
	p2 = _mm_load_ps(&input[2*SSLIMIT]);  // 20 21 22 23
	p3 = _mm_loadu_ps(&input[3*SSLIMIT]); // 30 31 32 33

	// rotate 4x4 matrix
	r0 = _mm_unpacklo_ps(p0, p2); // 00 20 01 21
	r1 = _mm_unpacklo_ps(p1, p3); // 10 30 11 31
	r2 = _mm_unpackhi_ps(p0, p2); // 02 22 03 23
	r3 = _mm_unpackhi_ps(p1, p3); // 12 32 13 33
	t0 = _mm_unpacklo_ps(r0, r1); // 00 10 20 30
	t1 = _mm_unpackhi_ps(r0, r1); // 01 11 21 31
	t2 = _mm_unpacklo_ps(r2, r3); // 02 12 22 32
	t3 = _mm_unpackhi_ps(r2, r3); // 03 13 23 33

	_mm_store_ps(&output[0*SBLIMIT], t0);
	_mm_store_ps(&output[1*SBLIMIT], t1);
	_mm_store_ps(&output[2*SBLIMIT], t2);
	_mm_store_ps(&output[3*SBLIMIT], t3);
}

void CPolyphase::Reorder(int channels, POLYSPECTRUM &output, const SPECTRUM &input)
{
	int ch, sb, ss;

	for ( ch=0; ch<channels; ch++ )
	{
		for ( ss=0; ss<SSLIMIT-2; ss+=4 )
			for ( sb=0; sb<SBLIMIT; sb+=4 )
				Rotate4x4(&output[ch][ss][sb], &input[ch][sb][ss]);

		// handle leftovers
		for ( ss=16; ss<SSLIMIT; ss++ )
			for ( sb=0; sb<SBLIMIT; sb++ )
				output[ch][ss][sb] = input[ch][sb][ss];

	}
}