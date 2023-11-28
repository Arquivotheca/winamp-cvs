/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: polyphase.cpp
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: polyphase class
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/11/29 21:04:32 $
 * $Id: polyphase.cpp,v 1.2 2009/11/29 21:04:32 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "polyphase.h"

/*-------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------
 *
 * function to convert a Polyphase Window according to ISO/MPEG into a
 * polyphase window used in _this_ decoder
 *
 *
 *  int    i, j, index = 0;
 *  double tmp;
 *  float  window[512];    // ISO/MPEG Window
 *  float  my_window[512]; // window used in _this_ decoder
 *
 *  for ( i=0; i<512; i+=32 )
 *    for ( j=1; j<8; j++ )
 *      {
 *      tmp            = window[i+j+16];
 *      window[i+j+16] = window[i+32-j];
 *      window[i+32-j] = tmp;
 *      }
 *
 *  for ( j=0; j<16; j++ )
 *     for ( i=0; i<512; i+=64 )
 *       {
 *       window[i+j+16] *= -1.0;
 *       window[i+j+32] *= -1.0;
 *       window[i+j+48] *= -1.0;
 *       }
 *
 *  for ( j=0; j<16; j++ )
 *    for ( i=0; i<512; i+=64 )
 *      {
 *      my_window[index]    = window[i+j];
 *      my_window[index+1]  = window[i+j+16];
 *      my_window[index+2]  = window[i+j+32];
 *      my_window[index+3]  = window[i+j+48];
 *      index += 4;
 *      }
 * ------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------*/

static const float syn_f_window[HAN_SIZE] =
{
  0.000000000f, 0.000076294f, 0.000442505f, 0.001586914f,
  0.003250122f,-0.002227783f, 0.007003784f, 0.023910522f,
  0.031082153f, 0.000686646f, 0.078628540f, 0.148422241f,
  0.100311279f, 0.152206421f, 0.572036743f, 0.976852417f,
  1.144989014f,-0.976852417f,-0.572036743f,-0.152206421f,
  0.100311279f,-0.148422241f,-0.078628540f,-0.000686646f,
  0.031082153f,-0.023910522f,-0.007003784f, 0.002227783f,
  0.003250122f,-0.001586914f,-0.000442505f,-0.000076294f,

 -0.000015259f, 0.000396729f, 0.000473022f, 0.003173828f,
  0.003326416f, 0.006118774f, 0.007919312f, 0.031478882f,
  0.030517578f, 0.073059082f, 0.084182739f, 0.108856201f,
  0.090927124f, 0.543823242f, 0.600219727f, 1.144287109f,
  1.144287109f,-0.600219727f,-0.543823242f, 0.090927124f,
  0.108856201f,-0.084182739f,-0.073059082f, 0.030517578f,
  0.031478882f,-0.007919312f,-0.006118774f, 0.003326416f,
  0.003173828f,-0.000473022f,-0.000396729f,-0.000015259f,

 -0.000015259f, 0.000366211f, 0.000534058f, 0.003082275f,
  0.003387451f, 0.005294800f, 0.008865356f, 0.031738281f,
  0.029785156f, 0.067520142f, 0.089706421f, 0.116577148f,
  0.080688477f, 0.515609741f, 0.628295898f, 1.142211914f,
  1.142211914f,-0.628295898f,-0.515609741f, 0.080688477f,
  0.116577148f,-0.089706421f,-0.067520142f, 0.029785156f,
  0.031738281f,-0.008865356f,-0.005294800f, 0.003387451f,
  0.003082275f,-0.000534058f,-0.000366211f,-0.000015259f,

 -0.000015259f, 0.000320435f, 0.000579834f, 0.002990723f,
  0.003433228f, 0.004486084f, 0.009841919f, 0.031845093f,
  0.028884888f, 0.061996460f, 0.095169067f, 0.123474121f,
  0.069595337f, 0.487472534f, 0.656219482f, 1.138763428f,
  1.138763428f,-0.656219482f,-0.487472534f, 0.069595337f,
  0.123474121f,-0.095169067f,-0.061996460f, 0.028884888f,
  0.031845093f,-0.009841919f,-0.004486084f, 0.003433228f,
  0.002990723f,-0.000579834f,-0.000320435f,-0.000015259f,

 -0.000015259f, 0.000289917f, 0.000625610f, 0.002899170f,
  0.003463745f, 0.003723145f, 0.010848999f, 0.031814575f,
  0.027801514f, 0.056533813f, 0.100540161f, 0.129577637f,
  0.057617187f, 0.459472656f, 0.683914185f, 1.133926392f,
  1.133926392f,-0.683914185f,-0.459472656f, 0.057617187f,
  0.129577637f,-0.100540161f,-0.056533813f, 0.027801514f,
  0.031814575f,-0.010848999f,-0.003723145f, 0.003463745f,
  0.002899170f,-0.000625610f,-0.000289917f,-0.000015259f,

 -0.000015259f, 0.000259399f, 0.000686646f, 0.002792358f,
  0.003479004f, 0.003005981f, 0.011886597f, 0.031661987f,
  0.026535034f, 0.051132202f, 0.105819702f, 0.134887695f,
  0.044784546f, 0.431655884f, 0.711318970f, 1.127746582f,
  1.127746582f,-0.711318970f,-0.431655884f, 0.044784546f,
  0.134887695f,-0.105819702f,-0.051132202f, 0.026535034f,
  0.031661987f,-0.011886597f,-0.003005981f, 0.003479004f,
  0.002792358f,-0.000686646f,-0.000259399f,-0.000015259f,

 -0.000015259f, 0.000244141f, 0.000747681f, 0.002685547f,
  0.003479004f, 0.002334595f, 0.012939453f, 0.031387329f,
  0.025085449f, 0.045837402f, 0.110946655f, 0.139450073f,
  0.031082153f, 0.404083252f, 0.738372803f, 1.120223999f,
  1.120223999f,-0.738372803f,-0.404083252f, 0.031082153f,
  0.139450073f,-0.110946655f,-0.045837402f, 0.025085449f,
  0.031387329f,-0.012939453f,-0.002334595f, 0.003479004f,
  0.002685547f,-0.000747681f,-0.000244141f,-0.000015259f,

 -0.000030518f, 0.000213623f, 0.000808716f, 0.002578735f,
  0.003463745f, 0.001693726f, 0.014022827f, 0.031005859f,
  0.023422241f, 0.040634155f, 0.115921021f, 0.143264771f,
  0.016510010f, 0.376800537f, 0.765029907f, 1.111373901f,
  1.111373901f,-0.765029907f,-0.376800537f, 0.016510010f,
  0.143264771f,-0.115921021f,-0.040634155f, 0.023422241f,
  0.031005859f,-0.014022827f,-0.001693726f, 0.003463745f,
  0.002578735f,-0.000808716f,-0.000213623f,-0.000030518f,

 -0.000030518f, 0.000198364f, 0.000885010f, 0.002456665f,
  0.003417969f, 0.001098633f, 0.015121460f, 0.030532837f,
  0.021575928f, 0.035552979f, 0.120697021f, 0.146362305f,
  0.001068115f, 0.349868774f, 0.791213989f, 1.101211548f,
  1.101211548f,-0.791213989f,-0.349868774f, 0.001068115f,
  0.146362305f,-0.120697021f,-0.035552979f, 0.021575928f,
  0.030532837f,-0.015121460f,-0.001098633f, 0.003417969f,
  0.002456665f,-0.000885010f,-0.000198364f,-0.000030518f,

 -0.000030518f, 0.000167847f, 0.000961304f, 0.002349854f,
  0.003372192f, 0.000549316f, 0.016235352f, 0.029937744f,
  0.019531250f, 0.030609131f, 0.125259399f, 0.148773193f,
 -0.015228271f, 0.323318481f, 0.816864014f, 1.089782715f,
  1.089782715f,-0.816864014f,-0.323318481f,-0.015228271f,
  0.148773193f,-0.125259399f,-0.030609131f, 0.019531250f,
  0.029937744f,-0.016235352f,-0.000549316f, 0.003372192f,
  0.002349854f,-0.000961304f,-0.000167847f,-0.000030518f,

 -0.000030518f, 0.000152588f, 0.001037598f, 0.002243042f,
  0.003280640f, 0.000030518f, 0.017349243f, 0.029281616f,
  0.017257690f, 0.025817871f, 0.129562378f, 0.150497437f,
 -0.032379150f, 0.297210693f, 0.841949463f, 1.077117920f,
  1.077117920f,-0.841949463f,-0.297210693f,-0.032379150f,
  0.150497437f,-0.129562378f,-0.025817871f, 0.017257690f,
  0.029281616f,-0.017349243f,-0.000030518f, 0.003280640f,
  0.002243042f,-0.001037598f,-0.000152588f,-0.000030518f,

 -0.000045776f, 0.000137329f, 0.001113892f, 0.002120972f,
  0.003173828f,-0.000442505f, 0.018463135f, 0.028533936f,
  0.014801025f, 0.021179199f, 0.133590698f, 0.151596069f,
 -0.050354004f, 0.271591187f, 0.866363525f, 1.063217163f,
  1.063217163f,-0.866363525f,-0.271591187f,-0.050354004f,
  0.151596069f,-0.133590698f,-0.021179199f, 0.014801025f,
  0.028533936f,-0.018463135f, 0.000442505f, 0.003173828f,
  0.002120972f,-0.001113892f,-0.000137329f,-0.000045776f,

 -0.000045776f, 0.000122070f, 0.001205444f, 0.002014160f,
  0.003051758f,-0.000869751f, 0.019577026f, 0.027725220f,
  0.012115479f, 0.016708374f, 0.137298584f, 0.152069092f,
 -0.069168091f, 0.246505737f, 0.890090942f, 1.048156738f,
  1.048156738f,-0.890090942f,-0.246505737f,-0.069168091f,
  0.152069092f,-0.137298584f,-0.016708374f, 0.012115479f,
  0.027725220f,-0.019577026f, 0.000869751f, 0.003051758f,
  0.002014160f,-0.001205444f,-0.000122070f,-0.000045776f,

 -0.000061035f, 0.000106812f, 0.001296997f, 0.001907349f,
  0.002883911f,-0.001266479f, 0.020690918f, 0.026840210f,
  0.009231567f, 0.012420654f, 0.140670776f, 0.151962280f,
 -0.088775635f, 0.221984863f, 0.913055420f, 1.031936646f,
  1.031936646f,-0.913055420f,-0.221984863f,-0.088775635f,
  0.151962280f,-0.140670776f,-0.012420654f, 0.009231567f,
  0.026840210f,-0.020690918f, 0.001266479f, 0.002883911f,
  0.001907349f,-0.001296997f,-0.000106812f,-0.000061035f,

 -0.000061035f, 0.000106812f, 0.001388550f, 0.001785278f,
  0.002700806f,-0.001617432f, 0.021789551f, 0.025909424f,
  0.006134033f, 0.008316040f, 0.143676758f, 0.151306152f,
 -0.109161377f, 0.198059082f, 0.935195923f, 1.014617920f,
  1.014617920f,-0.935195923f,-0.198059082f,-0.109161377f,
  0.151306152f,-0.143676758f,-0.008316040f, 0.006134033f,
  0.025909424f,-0.021789551f, 0.001617432f, 0.002700806f,
  0.001785278f,-0.001388550f,-0.000106812f,-0.000061035f,

 -0.000076294f, 0.000091553f, 0.001480103f, 0.001693726f,
  0.002487183f,-0.001937866f, 0.022857666f, 0.024932861f,
  0.002822876f, 0.004394531f, 0.146255493f, 0.150115967f,
 -0.130310059f, 0.174789429f, 0.956481934f, 0.996246338f,
  0.996246338f,-0.956481934f,-0.174789429f,-0.130310059f,
  0.150115967f,-0.146255493f,-0.004394531f, 0.002822876f,
  0.024932861f,-0.022857666f, 0.001937866f, 0.002487183f,
  0.001693726f,-0.001480103f,-0.000091553f,-0.000076294f
};


#ifndef USE_ASM
static const float cost32_c0[] =
  {
  0.50060299823520f,  0.50547095989754f,  0.51544730992262f,  0.53104259108978f,
  0.55310389603444f,  0.58293496820613f,  0.62250412303566f,  0.67480834145501f,
  0.74453627100230f,  0.83934964541553f,  0.97256823786196f,  1.16943993343288f,
  1.48416461631417f,  2.05778100995341f,  3.40760841846872f, 10.19000812354803f
  };
#endif

/* ------------------------------------------------------------------------*/

static const float cost32_c1[] =
  {
  0.50241928618816f,  0.52249861493969f,  0.56694403481636f,  0.64682178335999f,
  0.78815462345125f,  1.06067768599035f,  1.72244709823833f,  5.10114861868916f
  };

/* ------------------------------------------------------------------------*/

static const float cost32_c2[] =
  {
  0.50979557910416f,  0.60134488693505f,  0.89997622313642f,  2.56291544774151f
  };

/* ------------------------------------------------------------------------*/

static const float cost32_c3[] =
  {
  0.54119610014620f,  1.30656296487638f
  };

/* ------------------------------------------------------------------------*/

static const float cost32_c4[] =
  {
  0.70710678118655f
  };

/*-------------------------------------------------------------------------*/

static void cost32(const float *vec,float *f_vec);

/*-------------------------------------------------------------------------*/

static void cost16(const float *vec,float *f_vec)
{
  float tmp1_0,tmp1_1,tmp1_2,tmp1_3,tmp1_4,tmp1_5,tmp1_6,tmp1_7;
  float res1_0,res1_1,res1_2,res1_3,res1_4,res1_5,res1_6,res1_7;

  float tmp2_0,tmp2_1,tmp2_2,tmp2_3;
  float res2_0,res2_1,res2_2,res2_3;

  float tmp3_0,tmp3_1;
  float res3_0,res3_1;

  tmp1_0 = vec[0]+vec[15];
  tmp1_1 = vec[1]+vec[14];
  tmp1_2 = vec[2]+vec[13];
  tmp1_3 = vec[3]+vec[12];
  tmp1_4 = vec[4]+vec[11];
  tmp1_5 = vec[5]+vec[10];
  tmp1_6 = vec[6]+vec[9];
  tmp1_7 = vec[7]+vec[8];

  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;

  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  f_vec[0]  = tmp3_0+tmp3_1;
  f_vec[16] = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  f_vec[8]  = res3_0+res3_1;
  f_vec[24] = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;

  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;

  f_vec[12] = res1_1;
  f_vec[4]  = res1_3;
  f_vec[20] = res1_5;
  f_vec[28] = res1_7;

  tmp1_0 = (vec[0]-vec[15])*cost32_c1[0];
  tmp1_1 = (vec[1]-vec[14])*cost32_c1[1];
  tmp1_2 = (vec[2]-vec[13])*cost32_c1[2];
  tmp1_3 = (vec[3]-vec[12])*cost32_c1[3];
  tmp1_4 = (vec[4]-vec[11])*cost32_c1[4];
  tmp1_5 = (vec[5]-vec[10])*cost32_c1[5];
  tmp1_6 = (vec[6]-vec[9])*cost32_c1[6];
  tmp1_7 = (vec[7]-vec[8])*cost32_c1[7];

  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;

  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res1_0 = tmp3_0+tmp3_1;
  res1_4 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res1_2 = res3_0+res3_1;
  res1_6 = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;
  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;

  f_vec[14] = res1_0+res1_1;
  f_vec[10] = res1_1+res1_2;
  f_vec[6] = res1_2+res1_3;
  f_vec[2] = res1_3+res1_4;
  f_vec[18] = res1_4+res1_5;
  f_vec[22] = res1_5+res1_6;
  f_vec[26] = res1_6+res1_7;
  f_vec[30] = res1_7;
}

/*-------------------------------------------------------------------------*/

static void cost8(const float *vec,float *f_vec)
{
  float res1_1,res1_3,res1_5,res1_7;

  float tmp2_0,tmp2_1,tmp2_2,tmp2_3;
  float res2_0,res2_1,res2_2,res2_3;

  float tmp3_0,tmp3_1;
  float res3_0,res3_1;

  tmp2_0 = vec[0]+vec[7];
  tmp2_1 = vec[1]+vec[6];
  tmp2_2 = vec[2]+vec[5];
  tmp2_3 = vec[3]+vec[4];

  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  f_vec[0]  = tmp3_0+tmp3_1;
  f_vec[16] = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  f_vec[8]  = res3_0+res3_1;
  f_vec[24] = res3_1;

  tmp2_0 = (vec[0]-vec[7])*cost32_c2[0];
  tmp2_1 = (vec[1]-vec[6])*cost32_c2[1];
  tmp2_2 = (vec[2]-vec[5])*cost32_c2[2];
  tmp2_3 = (vec[3]-vec[4])*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;

  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;

  f_vec[12] = res1_1;
  f_vec[4]  = res1_3;
  f_vec[20] = res1_5;
  f_vec[28] = res1_7;
}

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C P o l y p h a s e
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CPolyphase::CPolyphase (const MPEG_INFO &_info,
                        int              _qual,
                        int              _downMix) :
  info (_info),
  qual (_qual),
  downMix (_downMix)
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
  int nChannels    = (downMix ? 1:info.stereo);
  int nIncrement   = (16<<nChannels)>>(qual);
  int fShortWindow = (downMix && (info.stereo==2)) ? 1 : 0;

  int j,k;

  for ( k=0; k<frms; k++ )
    {
    bufOffset = (bufOffset-32)&(HAN_SIZE-1);

    for ( j=0; j<nChannels; j++ )
      {
      switch ( qual )
        {
        case 0:
          cost32(sample[j][k], &(syn_buf[j][bufOffset]));
          break;

        case 1:
          cost16(sample[j][k], &(syn_buf[j][bufOffset]));
          break;

        case 2:
          cost8(sample[j][k], &(syn_buf[j][bufOffset]));
          break;
        }
      }

    if ( nChannels == 1 )
      window_band_m(bufOffset, pPcm, fShortWindow);
    else
      window_band_s(bufOffset, pPcm, fShortWindow);

    pPcm += nIncrement;
    }

  return pPcm;
}

/*-------------------------------------------------------------------------*/

void CPolyphase::window_band_m(int bufOffset,float *out_samples, int /* short_window */)
{
  const float *winPtr = syn_f_window;
  double       sum1,sum2;
  int          i,j;

  /* sum 0 and sum 16, 8, 4 (full, half, quarter spectrum) */
  sum1 = sum2 = 0;

  for ( i=0; i<512; i+=64 )
    {
    sum1 += syn_buf[0][(bufOffset+i+16)    & (HAN_SIZE-1)]*winPtr[0];
    sum2 += syn_buf[0][(bufOffset+i+32)    & (HAN_SIZE-1)]*winPtr[3];
    sum1 += syn_buf[0][(bufOffset+i+32+16) & (HAN_SIZE-1)]*winPtr[2];
    winPtr += 4;
    }

  out_samples[0]          = float(sum1 / 32768.0);
  out_samples[16 >> qual] = float(sum2 / 32768.0);

  /* sum 1-15, 1-7, 1-3 and 17-31, 9-15, 5-7 (full, half, quarter spectrum) */

  for ( j=1; j<(16>>qual); j++ )
    {
    sum1 = sum2 = 0;

    winPtr += (1<<qual)*32 - 32;

    for ( i=0;i<512;i+=64 )
      {
      sum1 += syn_buf[0][(bufOffset+i+j*(1<<qual)+16) & (HAN_SIZE-1)]*winPtr[0];
      sum2 += syn_buf[0][(bufOffset+i+j*(1<<qual)+16) & (HAN_SIZE-1)]*winPtr[1];
      sum1 += syn_buf[0][(bufOffset+i+j*(1<<qual)+32) & (HAN_SIZE-1)]*winPtr[2];
      sum2 += syn_buf[0][(bufOffset+i+j*(1<<qual)+32) & (HAN_SIZE-1)]*winPtr[3];
      winPtr += 4;
      }

    out_samples[j]            = float(sum1 / 32768.0);
    out_samples[(32>>qual)-j] = float(sum2 / 32768.0);
    }
}

/*-------------------------------------------------------------------------*/

void CPolyphase::window_band_s(int bufOffset, float *out_samples, int /* short_window */)
{
  const float *winPtr = syn_f_window;
  double       sum1l,sum2l,sum1r,sum2r;
  int          i,j,bufPtr;

  /* sum 0 and sum 16, 8, 4 (full, half, quarter spectrum) */
  sum1l = sum2l = sum1r = sum2r = 0;

  bufPtr = bufOffset;

  for ( i=0; i<512; i+=64 )
    {
    sum1l += syn_buf[0][bufPtr+16] * winPtr[0];
    sum1r += syn_buf[1][bufPtr+16] * winPtr[0];

    bufPtr = (bufPtr+32)&(HAN_SIZE-1);

    sum1l += syn_buf[0][bufPtr+16] * winPtr[2];
    sum1r += syn_buf[1][bufPtr+16] * winPtr[2];
    sum2l += syn_buf[0][bufPtr   ] * winPtr[3];
    sum2r += syn_buf[1][bufPtr   ] * winPtr[3];

    bufPtr = (bufPtr+32)&(HAN_SIZE-1);

    winPtr+=4;
    }

  out_samples[0]            = float(sum1l / 32768.0);
  out_samples[32>>qual]     = float(sum2l / 32768.0);
  out_samples[1]            = float(sum1r / 32768.0);
  out_samples[(32>>qual)+1] = float(sum2r / 32768.0);

  /* sum 1-15, 1-7, 1-3 and 17-31, 9-15, 5-7 (full, half, quarter spectrum) */

  for ( j=1; j<(16>>qual); j++ )
    {
    sum1l = sum2l = sum1r = sum2r = 0;

    bufPtr  = bufOffset+j*(1<<qual);
    winPtr += (1<<qual)*32 - 32;

    for ( i=0; i<512; i+=64 )
      {
      sum1l += syn_buf[0][bufPtr+16]*winPtr[0];
      sum1r += syn_buf[1][bufPtr+16]*winPtr[0];
      sum2l += syn_buf[0][bufPtr+16]*winPtr[1];
      sum2r += syn_buf[1][bufPtr+16]*winPtr[1];

      bufPtr = (bufPtr+32)&(HAN_SIZE-1);

      sum1l += syn_buf[0][bufPtr]*winPtr[2];
      sum1r += syn_buf[1][bufPtr]*winPtr[2];
      sum2l += syn_buf[0][bufPtr]*winPtr[3];
      sum2r += syn_buf[1][bufPtr]*winPtr[3];

      bufPtr = (bufPtr+32)&(HAN_SIZE-1);

      winPtr += 4;
      }

    out_samples[j*2]                = float(sum1l / 32768.0);
    out_samples[((32>>qual)-j)*2]   = float(sum2l / 32768.0);
    out_samples[j*2+1]              = float(sum1r / 32768.0);
    out_samples[((32>>qual)-j)*2+1] = float(sum2r / 32768.0);
    }
}

/*-------------------------------------------------------------------------*/

#ifndef USE_ASM

/*-------------------------------------------------------------------------*/

static void cost32(const float *vec,float *f_vec)
{
  float tmp0_0,tmp0_1,tmp0_2,tmp0_3,tmp0_4,tmp0_5,tmp0_6,tmp0_7;
  float tmp0_8,tmp0_9,tmp0_10,tmp0_11,tmp0_12,tmp0_13,tmp0_14,tmp0_15;
  float res0_0,res0_1,res0_2,res0_3,res0_4,res0_5,res0_6,res0_7;
  float res0_8,res0_9,res0_10,res0_11,res0_12,res0_13,res0_14,res0_15;

  float tmp1_0,tmp1_1,tmp1_2,tmp1_3,tmp1_4,tmp1_5,tmp1_6,tmp1_7;
  float res1_0,res1_1,res1_2,res1_3,res1_4,res1_5,res1_6,res1_7;

  float tmp2_0,tmp2_1,tmp2_2,tmp2_3;
  float res2_0,res2_1,res2_2,res2_3;

  float tmp3_0,tmp3_1;
  float res3_0,res3_1;

  tmp0_0 = vec[0]+vec[31];
  tmp0_1 = vec[1]+vec[30];
  tmp0_2 = vec[2]+vec[29];
  tmp0_3 = vec[3]+vec[28];
  tmp0_4 = vec[4]+vec[27];
  tmp0_5 = vec[5]+vec[26];
  tmp0_6 = vec[6]+vec[25];
  tmp0_7 = vec[7]+vec[24];
  tmp0_8 = vec[8]+vec[23];
  tmp0_9 = vec[9]+vec[22];
  tmp0_10 = vec[10]+vec[21];
  tmp0_11 = vec[11]+vec[20];
  tmp0_12 = vec[12]+vec[19];
  tmp0_13 = vec[13]+vec[18];
  tmp0_14 = vec[14]+vec[17];
  tmp0_15 = vec[15]+vec[16];

  tmp1_0 = tmp0_0+tmp0_15;
  tmp1_1 = tmp0_1+tmp0_14;
  tmp1_2 = tmp0_2+tmp0_13;
  tmp1_3 = tmp0_3+tmp0_12;
  tmp1_4 = tmp0_4+tmp0_11;
  tmp1_5 = tmp0_5+tmp0_10;
  tmp1_6 = tmp0_6+tmp0_9;
  tmp1_7 = tmp0_7+tmp0_8;

  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;

  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  f_vec[0]  = tmp3_0+tmp3_1;
  f_vec[16] = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  f_vec[8]  = res3_0+res3_1;
  f_vec[24] = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;

  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;

  f_vec[12]  = res1_1;
  f_vec[4] = res1_3;
  f_vec[20] = res1_5;
  f_vec[28] = res1_7;

  tmp1_0 = (tmp0_0-tmp0_15)*cost32_c1[0];
  tmp1_1 = (tmp0_1-tmp0_14)*cost32_c1[1];
  tmp1_2 = (tmp0_2-tmp0_13)*cost32_c1[2];
  tmp1_3 = (tmp0_3-tmp0_12)*cost32_c1[3];
  tmp1_4 = (tmp0_4-tmp0_11)*cost32_c1[4];
  tmp1_5 = (tmp0_5-tmp0_10)*cost32_c1[5];
  tmp1_6 = (tmp0_6-tmp0_9)*cost32_c1[6];
  tmp1_7 = (tmp0_7-tmp0_8)*cost32_c1[7];
  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res1_0 = tmp3_0+tmp3_1;
  res1_4 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res1_2 = res3_0+res3_1;
  res1_6 = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];

  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];

  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;
  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;

  f_vec[14] = res1_0+res1_1;
  f_vec[10] = res1_1+res1_2;
  f_vec[6] = res1_2+res1_3;
  f_vec[2] = res1_3+res1_4;
  f_vec[18] = res1_4+res1_5;
  f_vec[22] = res1_5+res1_6;
  f_vec[26] = res1_6+res1_7;
  f_vec[30] = res1_7;

  /*  Odd Terms */
  tmp0_0 = (vec[0]-vec[31])*cost32_c0[0];
  tmp0_1 = (vec[1]-vec[30])*cost32_c0[1];
  tmp0_2 = (vec[2]-vec[29])*cost32_c0[2];
  tmp0_3 = (vec[3]-vec[28])*cost32_c0[3];
  tmp0_4 = (vec[4]-vec[27])*cost32_c0[4];
  tmp0_5 = (vec[5]-vec[26])*cost32_c0[5];
  tmp0_6 = (vec[6]-vec[25])*cost32_c0[6];
  tmp0_7 = (vec[7]-vec[24])*cost32_c0[7];
  tmp0_8 = (vec[8]-vec[23])*cost32_c0[8];
  tmp0_9 = (vec[9]-vec[22])*cost32_c0[9];
  tmp0_10 = (vec[10]-vec[21])*cost32_c0[10];
  tmp0_11 = (vec[11]-vec[20])*cost32_c0[11];
  tmp0_12 = (vec[12]-vec[19])*cost32_c0[12];
  tmp0_13 = (vec[13]-vec[18])*cost32_c0[13];
  tmp0_14 = (vec[14]-vec[17])*cost32_c0[14];
  tmp0_15 = (vec[15]-vec[16])*cost32_c0[15];

  tmp1_0 = tmp0_0+tmp0_15;
  tmp1_1 = tmp0_1+tmp0_14;
  tmp1_2 = tmp0_2+tmp0_13;
  tmp1_3 = tmp0_3+tmp0_12;
  tmp1_4 = tmp0_4+tmp0_11;
  tmp1_5 = tmp0_5+tmp0_10;
  tmp1_6 = tmp0_6+tmp0_9;
  tmp1_7 = tmp0_7+tmp0_8;
  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res0_0 = tmp3_0+tmp3_1;
  res0_8 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];
  res0_4 = res3_0+res3_1;
  res0_12 = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;
  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];
  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;
  res0_2 = res2_0+res2_1;
  res0_6 = res2_1+res2_2;
  res0_10 = res2_2+res2_3;
  res0_14 = res2_3;

  tmp1_0 = (tmp0_0-tmp0_15)*cost32_c1[0];
  tmp1_1 = (tmp0_1-tmp0_14)*cost32_c1[1];
  tmp1_2 = (tmp0_2-tmp0_13)*cost32_c1[2];
  tmp1_3 = (tmp0_3-tmp0_12)*cost32_c1[3];
  tmp1_4 = (tmp0_4-tmp0_11)*cost32_c1[4];
  tmp1_5 = (tmp0_5-tmp0_10)*cost32_c1[5];
  tmp1_6 = (tmp0_6-tmp0_9)*cost32_c1[6];
  tmp1_7 = (tmp0_7-tmp0_8)*cost32_c1[7];
  tmp2_0 = tmp1_0+tmp1_7;
  tmp2_1 = tmp1_1+tmp1_6;
  tmp2_2 = tmp1_2+tmp1_5;
  tmp2_3 = tmp1_3+tmp1_4;
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;

  res1_0 = tmp3_0+tmp3_1;
  res1_4 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];
  res1_2 = res3_0+res3_1;
  res1_6 = res3_1;

  tmp2_0 = (tmp1_0-tmp1_7)*cost32_c2[0];
  tmp2_1 = (tmp1_1-tmp1_6)*cost32_c2[1];
  tmp2_2 = (tmp1_2-tmp1_5)*cost32_c2[2];
  tmp2_3 = (tmp1_3-tmp1_4)*cost32_c2[3];
  tmp3_0 = tmp2_0+tmp2_3;
  tmp3_1 = tmp2_1+tmp2_2;
  res2_0 = tmp3_0+tmp3_1;
  res2_2 = (tmp3_0-tmp3_1)*cost32_c4[0];

  tmp3_0 = (tmp2_0-tmp2_3)*cost32_c3[0];
  tmp3_1 = (tmp2_1-tmp2_2)*cost32_c3[1];
  res3_0 = tmp3_0+tmp3_1;
  res3_1 = (tmp3_0-tmp3_1)*cost32_c4[0];
  res2_1 = res3_0+res3_1;
  res2_3 = res3_1;
  res1_1 = res2_0+res2_1;
  res1_3 = res2_1+res2_2;
  res1_5 = res2_2+res2_3;
  res1_7 = res2_3;
  res0_1 = res1_0+res1_1;
  res0_3 = res1_1+res1_2;
  res0_5 = res1_2+res1_3;
  res0_7 = res1_3+res1_4;
  res0_9 = res1_4+res1_5;
  res0_11 = res1_5+res1_6;
  res0_13 = res1_6+res1_7;
  res0_15 = res1_7;

  f_vec[15] = res0_0+res0_1;
  f_vec[13] = res0_1+res0_2;
  f_vec[11] = res0_2+res0_3;
  f_vec[9] = res0_3+res0_4;
  f_vec[7] = res0_4+res0_5;
  f_vec[5] = res0_5+res0_6;
  f_vec[3] = res0_6+res0_7;
  f_vec[1] = res0_7+res0_8;
  f_vec[17] = res0_8+res0_9;
  f_vec[19] = res0_9+res0_10;
  f_vec[21] = res0_10+res0_11;
  f_vec[23] = res0_11+res0_12;
  f_vec[25] = res0_12+res0_13;
  f_vec[27] = res0_13+res0_14;
  f_vec[29] = res0_14+res0_15;
  f_vec[31] = res0_15;
}

/*-------------------------------------------------------------------------*/

#else /* ifndef USE_ASM */
#pragma message (__FILE__": using Intel X86 inline assembler")

#define C0_OFF 0*4
#define C1_OFF 16*4
#define C2_OFF 24*4
#define C3_OFF 28*4
#define C4_OFF 30*4

/*-------------------------------------------------------------------------*/

/* Even DCT-4 Transform */
#define COST4E(Src,Res0,Res1,Res2,Res3) _asm \
                                             \
{                                            \
  _asm fld  dword ptr Src                    \
  _asm fadd dword ptr Src+7*4                \
  _asm fld  dword ptr Src+1*4                \
  _asm fadd dword ptr Src+6*4                \
  _asm fld  dword ptr Src+3*4                \
  _asm fadd dword ptr Src+4*4                \
  _asm fld  dword ptr Src+2*4                \
  _asm fadd dword ptr Src+5*4                \
  _asm fld    st(3)                          \
  _asm fadd   st(0),st(2)                    \
  _asm fxch   st(4)                          \
  _asm fsubrp st(2),st(0)                    \
  _asm fld    st(2)                          \
  _asm fadd   st(0),st(1)                    \
  _asm fxch   st(3)                          \
  _asm fsubrp st(1),st(0)                    \
  _asm fld    st(3)                          \
  _asm fadd   st(0),st(3)                    \
  _asm fxch   st(4)                          \
  _asm fsubrp st(3),st(0)                    \
  _asm fmul   dword ptr [ecx+C3_OFF+1*4]     \
  _asm fxch   st(1)                          \
  _asm fmul   dword ptr [ecx+C3_OFF+0*4]     \
  _asm fxch   st(2)                          \
  _asm fmul   dword ptr [ecx+C4_OFF+0*4]     \
  _asm fld    st(2)                          \
  _asm fadd   st(0),st(2)                    \
  _asm fxch   st(3)                          \
  _asm fsubrp st(2),st(0)                    \
  _asm fstp   dword ptr Res1                 \
  _asm fmul   dword ptr [ecx+C4_OFF+0*4]     \
  _asm fxch   st(2)                          \
  _asm fstp   dword ptr Res0                 \
  _asm fadd   st(0),st(1)                    \
  _asm fxch   st(1)                          \
  _asm fstp   dword ptr Res3                 \
  _asm fstp   dword ptr Res2                 \
}

/*-------------------------------------------------------------------------*/

/* ODD DCT-4 Transform */
#define COST4O(Src,Res0,Res1,Res2,Res3) _asm \
                                             \
{                                            \
  _asm fld  dword ptr Src                    \
  _asm fsub dword ptr Src+7*4                \
  _asm fmul dword ptr [ecx+C2_OFF+0*4]       \
  _asm fld  dword ptr Src+1*4                \
  _asm fsub dword ptr Src+6*4                \
  _asm fmul dword ptr [ecx+C2_OFF+1*4]       \
  _asm fld  dword ptr Src+3*4                \
  _asm fsub dword ptr Src+4*4                \
  _asm fmul dword ptr [ecx+C2_OFF+3*4]       \
  _asm fld  dword ptr Src+2*4                \
  _asm fsub dword ptr Src+5*4                \
  _asm fmul dword ptr [ecx+C2_OFF+2*4]       \
  _asm fld    st(3)                          \
  _asm fadd   st(0),st(2)                    \
  _asm fxch   st(4)                          \
  _asm fsubrp st(2),st(0)                    \
  _asm fld    st(2)                          \
  _asm fadd   st(0),st(1)                    \
  _asm fxch   st(3)                          \
  _asm fsubrp st(1),st(0)                    \
  _asm fld    st(3)                          \
  _asm fadd   st(0),st(3)                    \
  _asm fxch   st(4)                          \
  _asm fsubrp st(3),st(0)                    \
  _asm fmul   dword ptr [ecx+C3_OFF+1*4]     \
  _asm fxch   st(1)                          \
  _asm fmul   dword ptr [ecx+C3_OFF+0*4]     \
  _asm fxch   st(2)                          \
  _asm fmul   dword ptr [ecx+C4_OFF+0*4]     \
  _asm fld    st(2)                          \
  _asm fadd   st(0),st(2)                    \
  _asm fxch   st(3)                          \
  _asm fsubrp st(2),st(0)                    \
  _asm fxch   st(1)                          \
  _asm fmul   dword ptr [ecx+C4_OFF+0*4]     \
  _asm fadd   st(2),st(0)                    \
  _asm fxch   st(2)                          \
  _asm fadd   st(3),st(0)                    \
  _asm fadd   st(0),st(1)                    \
  _asm fxch   st(2)                          \
  _asm fadd   st(1),st(0)                    \
  _asm fstp   dword ptr Res3                 \
  _asm fstp   dword ptr Res1                 \
  _asm fstp   dword ptr Res2                 \
  _asm fstp   dword ptr Res0                 \
}

/*-------------------------------------------------------------------------*/

static void cost32 (const float *vec,float *f_vec)
{
  static const float cost32_const[31] =
  {
    /* c0 */
    0.50060299823520f,0.50547095989754f,0.51544730992262f,0.53104259108978f,
    0.55310389603444f,0.58293496820613f,0.62250412303566f,0.67480834145501f,
    0.74453627100230f,0.83934964541553f,0.97256823786196f,1.16943993343288f,
    1.48416461631417f,2.05778100995341f,3.40760841846872f,10.19000812354803f,

    /* c1 */
    0.50241928618816f,0.52249861493969f,0.56694403481636f,0.64682178335999f,
    0.78815462345125f,1.06067768599035f,1.72244709823833f,5.10114861868916f,

    /* c2 */
    0.50979557910416f,0.60134488693505f,0.89997622313642f,2.56291544774151f,

    /* c3 */
    0.54119610014620f,1.30656296487638f,

    /* c4 */
    0.70710678118655f
  };

  float tmp[16];
  float res[16];

  _asm
    {
    mov ebx,vec
    mov esi,f_vec

    lea eax,tmp          /* to produce a smaller code size */
    lea edx,res          /*           ""                   */
    lea ecx,cost32_const /*           ""                   */

    /* even Part */

    fld   dword ptr [ebx]
    fadd  dword ptr [ebx+124]

    fld   dword ptr [ebx+60]
    fadd  dword ptr [ebx+64]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+0*4]
    fxch  st(1)
    fstp  dword ptr [eax+0*4]
    fstp  dword ptr [eax+8*4+0*4]


    fld   dword ptr [ebx+4]
    fadd  dword ptr [ebx+120]

    fld   dword ptr [ebx+56]
    fadd  dword ptr [ebx+68]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+1*4]
    fxch  st(1)
    fstp  dword ptr [eax+1*4]
    fstp  dword ptr [eax+8*4+1*4]

    fld   dword ptr [ebx+8]
    fadd  dword ptr [ebx+116]

    fld   dword ptr [ebx+52]
    fadd  dword ptr [ebx+72]

    fld st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+2*4]
    fxch  st(1)
    fstp  dword ptr [eax+2*4]
    fstp  dword ptr [eax+8*4+2*4]

    fld   dword ptr [ebx+12]
    fadd  dword ptr [ebx+112]

    fld   dword ptr [ebx+48]
    fadd  dword ptr [ebx+76]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+3*4]
    fxch  st(1)
    fstp  dword ptr [eax+3*4]
    fstp  dword ptr [eax+8*4+3*4]

    fld   dword ptr [ebx+16]
    fadd  dword ptr [ebx+108]

    fld   dword ptr [ebx+44]
    fadd  dword ptr [ebx+80]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+4*4]
    fxch  st(1)
    fstp  dword ptr [eax+4*4]
    fstp  dword ptr [eax+8*4+4*4]

    fld   dword ptr [ebx+20]
    fadd  dword ptr [ebx+104]

    fld   dword ptr [ebx+40]
    fadd  dword ptr [ebx+84]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+5*4]
    fxch  st(1)
    fstp  dword ptr [eax+5*4]
    fstp  dword ptr [eax+8*4+5*4]

    fld   dword ptr [ebx+24]
    fadd  dword ptr [ebx+100]

    fld   dword ptr [ebx+36]
    fadd  dword ptr [ebx+88]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+6*4]
    fxch  st(1)
    fstp  dword ptr [eax+6*4]
    fstp  dword ptr [eax+8*4+6*4]

    fld   dword ptr [ebx+28]
    fadd  dword ptr [ebx+96]

    fld   dword ptr [ebx+32]
    fadd  dword ptr [ebx+92]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+7*4]
    fxch  st(1)
    fstp  dword ptr [eax+7*4]
    fstp  dword ptr [eax+8*4+7*4]

    COST4E([eax],[esi],[esi+64],[esi+32],[esi+96])
    COST4O([eax],[esi+48],[esi+80],[esi+16],[esi+112])

    COST4E([eax+8*4],[edx+0*4],[edx+4*4],[edx+2*4],[edx+6*4])
    COST4O([eax+8*4],[edx+1*4],[edx+5*4],[edx+3*4],[esi+120])

    fld   dword ptr [edx+0*4]
    fld   dword ptr [edx+1*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+2*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+3*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+4*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+5*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+6*4]
    fadd  st(1),st(0)
    fadd  dword ptr [esi+120]
    fxch  st(6)
    fstp  dword ptr [esi+56]
    fstp  dword ptr [esi+88]
    fstp  dword ptr [esi+72]
    fstp  dword ptr [esi+8]
    fstp  dword ptr [esi+24]
    fstp  dword ptr [esi+40]
    fstp  dword ptr [esi+104]


    /* odd part */

    fld   dword ptr [ebx]
    fsub  dword ptr [ebx+124]
    fmul  dword ptr [ecx+C0_OFF+0*4]

    fld   dword ptr [ebx+60]
    fsub  dword ptr [ebx+64]
    fmul  dword ptr [ecx+C0_OFF+15*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+0*4]
    fxch  st(1)
    fstp  dword ptr [eax+0*4]
    fstp  dword ptr [eax+8*4+0*4]


    fld   dword ptr [ebx+4]
    fsub  dword ptr [ebx+120]
    fmul  dword ptr [ecx+C0_OFF+1*4]

    fld   dword ptr [ebx+56]
    fsub  dword ptr [ebx+68]
    fmul  dword ptr [ecx+C0_OFF+14*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+1*4]
    fxch  st(1)
    fstp  dword ptr [eax+1*4]
    fstp  dword ptr [eax+8*4+1*4]

    fld   dword ptr [ebx+8]
    fsub  dword ptr [ebx+116]
    fmul  dword ptr [ecx+C0_OFF+2*4]

    fld   dword ptr [ebx+52]
    fsub  dword ptr [ebx+72]
    fmul  dword ptr [ecx+C0_OFF+13*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+2*4]
    fxch  st(1)
    fstp  dword ptr [eax+2*4]
    fstp  dword ptr [eax+8*4+2*4]

    fld   dword ptr [ebx+12]
    fsub  dword ptr [ebx+112]
    fmul  dword ptr [ecx+C0_OFF+3*4]

    fld   dword ptr [ebx+48]
    fsub  dword ptr [ebx+76]
    fmul  dword ptr [ecx+C0_OFF+12*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+3*4]
    fxch  st(1)
    fstp  dword ptr [eax+3*4]
    fstp  dword ptr [eax+8*4+3*4]

    fld   dword ptr [ebx+16]
    fsub  dword ptr [ebx+108]
    fmul  dword ptr [ecx+C0_OFF+4*4]

    fld   dword ptr [ebx+44]
    fsub  dword ptr [ebx+80]
    fmul  dword ptr [ecx+C0_OFF+11*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+4*4]
    fxch  st(1)
    fstp  dword ptr [eax+4*4]
    fstp  dword ptr [eax+8*4+4*4]

    fld   dword ptr [ebx+20]
    fsub  dword ptr [ebx+104]
    fmul  dword ptr [ecx+C0_OFF+5*4]

    fld   dword ptr [ebx+40]
    fsub  dword ptr [ebx+84]
    fmul  dword ptr [ecx+C0_OFF+10*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+5*4]
    fxch  st(1)
    fstp  dword ptr [eax+5*4]
    fstp  dword ptr [eax+8*4+5*4]

    fld   dword ptr [ebx+24]
    fsub  dword ptr [ebx+100]
    fmul  dword ptr [ecx+C0_OFF+6*4]

    fld   dword ptr [ebx+36]
    fsub  dword ptr [ebx+88]
    fmul  dword ptr [ecx+C0_OFF+9*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+6*4]
    fxch  st(1)
    fstp  dword ptr [eax+6*4]
    fstp  dword ptr [eax+8*4+6*4]

    fld   dword ptr [ebx+28]
    fsub  dword ptr [ebx+96]
    fmul  dword ptr [ecx+C0_OFF+7*4]

    fld   dword ptr [ebx+32]
    fsub  dword ptr [ebx+92]
    fmul  dword ptr [ecx+C0_OFF+8*4]

    fld   st(1)
    fsub  st(0),st(1)
    fxch  st(1)
    faddp st(2),st(0)
    fmul  dword ptr [ecx+C1_OFF+7*4]
    fxch  st(1)
    fstp  dword ptr [eax+7*4]
    fstp  dword ptr [eax+8*4+7*4]

    COST4E([eax],[edx+8*4+0*4],[edx+8*4+4*4],[edx+8*4+2*4],[edx+8*4+6*4])
    COST4O([eax],[edx+8*4+1*4],[edx+8*4+5*4],[edx+8*4+3*4],[edx+8*4+7*4])

    COST4E([eax+8*4],[edx+0*4],[edx+4*4],[edx+2*4],[edx+6*4])
    COST4O([eax+8*4],[edx+1*4],[edx+5*4],[edx+3*4],[esi+124])


    fld   dword ptr [esi+124]
    fld   dword ptr [edx+6*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+5*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+4*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+3*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+2*4]
    fadd  st(1),st(0)
    fld   dword ptr [edx+1*4]
    fadd  st(1),st(0)
    fadd  dword ptr [edx+0*4]


    fld   dword ptr [edx+8*4+0*4]
    fadd  st(0),st(1)
    fstp  dword ptr [esi+60]


    fld   dword ptr [edx+8*4+1*4]
    fadd  st(1),st(0)
    fadd  st(0),st(2)
    fxch  st(1)
    fstp  dword ptr [esi+52]
    fstp  dword ptr [esi+44]


    fld   dword ptr [edx+8*4+2*4]
    fadd  st(1),st(0)
    fadd  st(0),st(2)

    fld   dword ptr [edx+8*4+3*4]
    fadd  st(3),st(0)
    fadd  st(0),st(4)

    fxch  st(2)
    fstp  dword ptr [esi+36]
    fstp  dword ptr [esi+28]
    fstp  dword ptr [esi+12]
    fstp  dword ptr [esi+20]


    fld   dword ptr [edx+8*4+4*4]
    fadd  st(1),st(0)
    fadd  st(0),st(2)

    fld   dword ptr [edx+8*4+5*4]
    fadd  st(3),st(0)
    fadd  st(0),st(4)

    fxch  st(2)
    fstp  dword ptr [esi+4]
    fstp  dword ptr [esi+68]
    fstp  dword ptr [esi+84]
    fstp  dword ptr [esi+76]


    fld   dword ptr [edx+8*4+6*4]
    fadd  st(1),st(0)
    fadd  st(0),st(2)

    fld   dword ptr [edx+8*4+7*4]
    fadd  st(3),st(0)
    fadd  dword ptr [esi+124]

    fxch  st(2)
    fstp  dword ptr [esi+92]
    fstp  dword ptr [esi+100]
    fstp  dword ptr [esi+116]
    fstp  dword ptr [esi+108]
    }
}

/*-------------------------------------------------------------------------*/

#endif /* ifndef USE_ASM */

/*-------------------------------------------------------------------------*/
