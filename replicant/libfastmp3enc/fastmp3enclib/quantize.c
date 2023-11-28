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
*   $Id: quantize.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

/* quantizing */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>

#include "mconfig.h"
#include "mathlib.h"
#include "quantize.h"
#include "sf_estim.h"
#include "mp3alloc.h"


#define PREDEFINED_TABLES 1
static const float C0 =   4.00005f; /* -16/3*log(1.0f-0.5f-logCon)/log(2) */
static const float C1 = -69.33295f; /* -16/3*log(MAX_QUANT+1-0.5-logCon)/log(2) */
static const float C2 =   5.77078f; /* 4/log(2) */
static const float logCon = -0.0946f;

ALIGN_16_BYTE static const int preEmphasisTab[] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};

ALIGN_16_BYTE static const float quantTableQ[16]=
{
  1.000000000000f,
  0.878126080187f,
  0.771105412704f,
  0.677127773468f,
  0.594603557501f,
  0.522136891214f,
  0.458502021602f,
  0.402622582987f,
  0.353553390593f,
  0.310464453018f,
  0.272626933166f,
  0.239400820175f,
  0.210224103813f,
  0.184603268242f,
  0.162104944331f,
  0.142348579345f
};

ALIGN_16_BYTE static const float quantTableE[17]=
{
  1.6777216000000000e+007f,
  2.0971520000000000e+006f,
  2.6214400000000000e+005f,
  3.2768000000000000e+004f,
  4.0960000000000000e+003f,
  5.1200000000000000e+002f,
  6.4000000000000000e+001f,
  8.0000000000000000e+000f,
  1.0000000000000000e+000f,
  1.2500000000000000e-001f,
  1.5625000000000000e-002f,
  1.9531250000000000e-003f,
  2.4414062500000000e-004f,
  3.0517578125000000e-005f,
  3.8146972656250000e-006f,
  4.7683715820312500e-007f,
  5.9604644775390625e-008f
};


/* for inverse quantization */
static float pow4_3_tab[MAX_QUANT + 1];
static int pow4_3_init=0;

static const float invQuantTableQ[16]=
{
   1.00000000000000f,
   1.18920711500272f,
   1.41421356237310f,
   1.68179283050743f,
   2.00000000000000f,
   2.37841423000544f,
   2.82842712474619f,
   3.36358566101486f,
   4.00000000000000f,
   4.75682846001088f,
   5.65685424949238f,
   6.72717132202972f,
   8.00000000000000f,
   9.51365692002177f,
  11.31370849898476f,
  13.45434264405943f
};

static const float invQuantTableE[17]=
{
   2.32830643653870e-10f,
   3.72529029846191e-09f,
   5.96046447753906e-08f,
   9.53674316406250e-07f,
   1.52587890625000e-05f,
   2.44140625000000e-04f,
   3.90625000000000e-03f,
   6.25000000000000e-02f,
   1.00000000000000e+00f,
   1.60000000000000e+01f,
   2.56000000000000e+02f,
   4.09600000000000e+03f,
   6.55360000000000e+04f,
   1.04857600000000e+06f,
   1.67772160000000e+07f,
   2.68435456000000e+08f,
   4.29496729600000e+09f
};

#if !defined WIN32 || (defined _M_AMD64 && !defined P4_INTRINSIC)

/*****************************************************************************

    functionname:quantizeLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/

static void quantizeLines_NoOpt(const int gain,
                                const int noOfLines,
                                const float * restrict mdctSpectrum,
                                signed int * restrict quaSpectrum)
{
  float quantizer;
  float k = logCon + 0.5f;
  int line;
  /*int noline=1; */

  quantizer=quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 

  for (line = 0; line < noOfLines; line++)
  {
    float tmp  = mdctSpectrum[line];

    if (tmp < 0.0f)
    {
      tmp = (float)sqrt(-tmp);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */

      quaSpectrum[line] = -(int)(k + quantizer * tmp);
    }
    else
    {
      tmp = (float)sqrt(tmp);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */

      quaSpectrum[line] = (int)(k + quantizer * tmp);
    }
  }
}


#else /* WIN32 / IX86 code */


#define MANT_DIGITS 9
#define MANT_SIZE   (1<<MANT_DIGITS)

/*
  table of (2^-127..128)^0.75
*/
#ifndef P4_CODE
ALIGN_16_BYTE static const float eTab_3_4[256] =
{
  2.12272101e-029f, 3.56997687e-029f, 6.00396142e-029f, 1.00974196e-028f, 1.69817681e-028f, 2.85598150e-028f, 4.80316913e-028f, 8.07793567e-028f, 
  1.35854145e-027f, 2.28478520e-027f, 3.84253531e-027f, 6.46234854e-027f, 1.08683316e-026f, 1.82782816e-026f, 3.07402825e-026f, 5.16987883e-026f, 
  8.69466528e-026f, 1.46226253e-025f, 2.45922260e-025f, 4.13590306e-025f, 6.95573222e-025f, 1.16981002e-024f, 1.96737808e-024f, 3.30872245e-024f, 
  5.56458578e-024f, 9.35848017e-024f, 1.57390246e-023f, 2.64697796e-023f, 4.45166862e-023f, 7.48678413e-023f, 1.25912197e-022f, 2.11758237e-022f, 
  3.56133490e-022f, 5.98942731e-022f, 1.00729758e-021f, 1.69406589e-021f, 2.84906792e-021f, 4.79154185e-021f, 8.05838060e-021f, 1.35525272e-020f, 
  2.27925433e-020f, 3.83323348e-020f, 6.44670448e-020f, 1.08420217e-019f, 1.82340347e-019f, 3.06658678e-019f, 5.15736359e-019f, 8.67361738e-019f, 
  1.45872277e-018f, 2.45326942e-018f, 4.12589087e-018f, 6.93889390e-018f, 1.16697822e-017f, 1.96261554e-017f, 3.30071269e-017f, 5.55111512e-017f, 
  9.33582575e-017f, 1.57009243e-016f, 2.64057016e-016f, 4.44089210e-016f, 7.46866060e-016f, 1.25607395e-015f, 2.11245612e-015f, 3.55271368e-015f, 
  5.97492848e-015f, 1.00485916e-014f, 1.68996490e-014f, 2.84217094e-014f, 4.77994279e-014f, 8.03887325e-014f, 1.35197192e-013f, 2.27373675e-013f, 
  3.82395423e-013f, 6.43109860e-013f, 1.08157754e-012f, 1.81898940e-012f, 3.05916338e-012f, 5.14487888e-012f, 8.65262029e-012f, 1.45519152e-011f, 
  2.44733071e-011f, 4.11590310e-011f, 6.92209623e-011f, 1.16415322e-010f, 1.95786456e-010f, 3.29272248e-010f, 5.53767698e-010f, 9.31322575e-010f, 
  1.56629165e-009f, 2.63417799e-009f, 4.43014159e-009f, 7.45058060e-009f, 1.25303332e-008f, 2.10734239e-008f, 3.54411327e-008f, 5.96046448e-008f, 
  1.00242666e-007f, 1.68587391e-007f, 2.83529062e-007f, 4.76837158e-007f, 8.01941326e-007f, 1.34869913e-006f, 2.26823249e-006f, 3.81469727e-006f, 
  6.41553061e-006f, 1.07895930e-005f, 1.81458599e-005f, 3.05175781e-005f, 5.13242449e-005f, 8.63167443e-005f, 1.45166880e-004f, 2.44140625e-004f, 
  4.10593959e-004f, 6.90533954e-004f, 1.16133504e-003f, 1.95312500e-003f, 3.28475167e-003f, 5.52427163e-003f, 9.29068029e-003f, 1.56250000e-002f, 
  2.62780134e-002f, 4.41941731e-002f, 7.43254423e-002f, 1.25000000e-001f, 2.10224107e-001f, 3.53553385e-001f, 5.94603539e-001f, 1.00000000e+000f, 
  1.68179286e+000f, 2.82842708e+000f, 4.75682831e+000f, 8.00000000e+000f, 1.34543428e+001f, 2.26274166e+001f, 3.80546265e+001f, 6.40000000e+001f, 
  1.07634743e+002f, 1.81019333e+002f, 3.04437012e+002f, 5.12000000e+002f, 8.61077942e+002f, 1.44815466e+003f, 2.43549609e+003f, 4.09600000e+003f, 
  6.88862354e+003f, 1.15852373e+004f, 1.94839688e+004f, 3.27680000e+004f, 5.51089883e+004f, 9.26818984e+004f, 1.55871750e+005f, 2.62144000e+005f, 
  4.40871906e+005f, 7.41455188e+005f, 1.24697400e+006f, 2.09715200e+006f, 3.52697525e+006f, 5.93164150e+006f, 9.97579200e+006f, 1.67772160e+007f, 
  2.82158020e+007f, 4.74531320e+007f, 7.98063360e+007f, 1.34217728e+008f, 2.25726416e+008f, 3.79625056e+008f, 6.38450688e+008f, 1.07374182e+009f, 
  1.80581133e+009f, 3.03700045e+009f, 5.10760550e+009f, 8.58993459e+009f, 1.44464906e+010f, 2.42960036e+010f, 4.08608440e+010f, 6.87194767e+010f, 
  1.15571925e+011f, 1.94368029e+011f, 3.26886752e+011f, 5.49755814e+011f, 9.24575400e+011f, 1.55494423e+012f, 2.61509402e+012f, 4.39804651e+012f, 
  7.39660320e+012f, 1.24395538e+013f, 2.09207521e+013f, 3.51843721e+013f, 5.91728256e+013f, 9.95164307e+013f, 1.67366017e+014f, 2.81474977e+014f, 
  4.73382605e+014f, 7.96131445e+014f, 1.33892814e+015f, 2.25179981e+015f, 3.78706084e+015f, 6.36905156e+015f, 1.07114251e+016f, 1.80143985e+016f, 
  3.02964867e+016f, 5.09524125e+016f, 8.56914008e+016f, 1.44115188e+017f, 2.42371894e+017f, 4.07619300e+017f, 6.85531206e+017f, 1.15292150e+018f, 
  1.93897515e+018f, 3.26095440e+018f, 5.48424965e+018f, 9.22337204e+018f, 1.55118012e+019f, 2.60876352e+019f, 4.38739972e+019f, 7.37869763e+019f, 
  1.24094410e+020f, 2.08701082e+020f, 3.50991978e+020f, 5.90295810e+020f, 9.92755276e+020f, 1.66960865e+021f, 2.80793582e+021f, 4.72236648e+021f, 
  7.94204221e+021f, 1.33568692e+022f, 2.24634866e+022f, 3.77789319e+022f, 6.35363377e+022f, 1.06854954e+023f, 1.79707893e+023f, 3.02231455e+023f, 
  5.08290701e+023f, 8.54839630e+023f, 1.43766314e+024f, 2.41785164e+024f, 4.06632561e+024f, 6.83871704e+024f, 1.15013051e+025f, 1.93428131e+025f, 
  3.25306049e+025f, 5.47097363e+025f, 9.20104410e+025f, 1.54742505e+026f, 2.60244839e+026f, 4.37677891e+026f, 7.36083528e+026f, 1.23794004e+027f, 
  2.08195871e+027f, 3.50142313e+027f, 5.88866822e+027f, 9.90352031e+027f, 1.66556697e+028f, 2.80113850e+028f, 4.71093458e+028f, 7.92281625e+028f, 
};

/*
  table of (1.000..1.9999) ^0.75
*/

ALIGN_16_BYTE static const float mTab_3_4[512] =
{
  1.00000000e+000f, 1.00146449e+000f, 1.00292826e+000f, 1.00439131e+000f, 1.00585365e+000f, 1.00731528e+000f, 1.00877631e+000f, 1.01023650e+000f, 
  1.01169598e+000f, 1.01315486e+000f, 1.01461291e+000f, 1.01607037e+000f, 1.01752710e+000f, 1.01898313e+000f, 1.02043855e+000f, 1.02189314e+000f, 
  1.02334714e+000f, 1.02480042e+000f, 1.02625299e+000f, 1.02770483e+000f, 1.02915609e+000f, 1.03060663e+000f, 1.03205645e+000f, 1.03350568e+000f, 
  1.03495419e+000f, 1.03640199e+000f, 1.03784919e+000f, 1.03929567e+000f, 1.04074144e+000f, 1.04218662e+000f, 1.04363108e+000f, 1.04507482e+000f, 
  1.04651797e+000f, 1.04796052e+000f, 1.04940236e+000f, 1.05084348e+000f, 1.05228400e+000f, 1.05372381e+000f, 1.05516303e+000f, 1.05660152e+000f, 
  1.05803943e+000f, 1.05947661e+000f, 1.06091321e+000f, 1.06234908e+000f, 1.06378436e+000f, 1.06521904e+000f, 1.06665301e+000f, 1.06808639e+000f, 
  1.06951916e+000f, 1.07095122e+000f, 1.07238257e+000f, 1.07381344e+000f, 1.07524359e+000f, 1.07667315e+000f, 1.07810199e+000f, 1.07953024e+000f, 
  1.08095789e+000f, 1.08238494e+000f, 1.08381128e+000f, 1.08523703e+000f, 1.08666217e+000f, 1.08808672e+000f, 1.08951056e+000f, 1.09093380e+000f, 
  1.09235644e+000f, 1.09377849e+000f, 1.09519994e+000f, 1.09662068e+000f, 1.09804094e+000f, 1.09946048e+000f, 1.10087943e+000f, 1.10229778e+000f, 
  1.10371554e+000f, 1.10513270e+000f, 1.10654926e+000f, 1.10796511e+000f, 1.10938048e+000f, 1.11079526e+000f, 1.11220932e+000f, 1.11362290e+000f, 
  1.11503577e+000f, 1.11644816e+000f, 1.11785984e+000f, 1.11927104e+000f, 1.12068152e+000f, 1.12209153e+000f, 1.12350082e+000f, 1.12490964e+000f, 
  1.12631786e+000f, 1.12772548e+000f, 1.12913251e+000f, 1.13053894e+000f, 1.13194478e+000f, 1.13335001e+000f, 1.13475478e+000f, 1.13615882e+000f, 
  1.13756239e+000f, 1.13896537e+000f, 1.14036775e+000f, 1.14176953e+000f, 1.14317071e+000f, 1.14457142e+000f, 1.14597142e+000f, 1.14737093e+000f, 
  1.14876997e+000f, 1.15016830e+000f, 1.15156615e+000f, 1.15296340e+000f, 1.15436006e+000f, 1.15575624e+000f, 1.15715170e+000f, 1.15854681e+000f, 
  1.15994120e+000f, 1.16133511e+000f, 1.16272843e+000f, 1.16412115e+000f, 1.16551340e+000f, 1.16690505e+000f, 1.16829610e+000f, 1.16968668e+000f, 
  1.17107666e+000f, 1.17246616e+000f, 1.17385507e+000f, 1.17524338e+000f, 1.17663121e+000f, 1.17801845e+000f, 1.17940521e+000f, 1.18079138e+000f, 
  1.18217707e+000f, 1.18356216e+000f, 1.18494666e+000f, 1.18633068e+000f, 1.18771410e+000f, 1.18909705e+000f, 1.19047952e+000f, 1.19186139e+000f, 
  1.19324267e+000f, 1.19462347e+000f, 1.19600379e+000f, 1.19738352e+000f, 1.19876266e+000f, 1.20014143e+000f, 1.20151949e+000f, 1.20289719e+000f, 
  1.20427430e+000f, 1.20565081e+000f, 1.20702696e+000f, 1.20840240e+000f, 1.20977747e+000f, 1.21115196e+000f, 1.21252584e+000f, 1.21389937e+000f, 
  1.21527231e+000f, 1.21664464e+000f, 1.21801662e+000f, 1.21938801e+000f, 1.22075880e+000f, 1.22212923e+000f, 1.22349906e+000f, 1.22486842e+000f, 
  1.22623718e+000f, 1.22760546e+000f, 1.22897327e+000f, 1.23034060e+000f, 1.23170745e+000f, 1.23307371e+000f, 1.23443949e+000f, 1.23580480e+000f, 
  1.23716950e+000f, 1.23853385e+000f, 1.23989761e+000f, 1.24126089e+000f, 1.24262357e+000f, 1.24398589e+000f, 1.24534774e+000f, 1.24670899e+000f, 
  1.24806976e+000f, 1.24943006e+000f, 1.25078988e+000f, 1.25214911e+000f, 1.25350797e+000f, 1.25486624e+000f, 1.25622416e+000f, 1.25758147e+000f, 
  1.25893831e+000f, 1.26029468e+000f, 1.26165056e+000f, 1.26300597e+000f, 1.26436090e+000f, 1.26571536e+000f, 1.26706934e+000f, 1.26842272e+000f, 
  1.26977575e+000f, 1.27112830e+000f, 1.27248025e+000f, 1.27383184e+000f, 1.27518284e+000f, 1.27653348e+000f, 1.27788353e+000f, 1.27923322e+000f, 
  1.28058243e+000f, 1.28193104e+000f, 1.28327930e+000f, 1.28462708e+000f, 1.28597426e+000f, 1.28732109e+000f, 1.28866744e+000f, 1.29001331e+000f, 
  1.29135871e+000f, 1.29270363e+000f, 1.29404807e+000f, 1.29539216e+000f, 1.29673564e+000f, 1.29807878e+000f, 1.29942131e+000f, 1.30076349e+000f, 
  1.30210519e+000f, 1.30344641e+000f, 1.30478716e+000f, 1.30612743e+000f, 1.30746734e+000f, 1.30880666e+000f, 1.31014562e+000f, 1.31148410e+000f, 
  1.31282210e+000f, 1.31415975e+000f, 1.31549680e+000f, 1.31683350e+000f, 1.31816971e+000f, 1.31950545e+000f, 1.32084072e+000f, 1.32217562e+000f, 
  1.32351005e+000f, 1.32484400e+000f, 1.32617748e+000f, 1.32751060e+000f, 1.32884312e+000f, 1.33017540e+000f, 1.33150709e+000f, 1.33283830e+000f, 
  1.33416915e+000f, 1.33549964e+000f, 1.33682954e+000f, 1.33815908e+000f, 1.33948815e+000f, 1.34081674e+000f, 1.34214497e+000f, 1.34347272e+000f, 
  1.34480011e+000f, 1.34612691e+000f, 1.34745336e+000f, 1.34877944e+000f, 1.35010505e+000f, 1.35143018e+000f, 1.35275483e+000f, 1.35407913e+000f, 
  1.35540295e+000f, 1.35672641e+000f, 1.35804939e+000f, 1.35937202e+000f, 1.36069405e+000f, 1.36201584e+000f, 1.36333704e+000f, 1.36465800e+000f, 
  1.36597836e+000f, 1.36729836e+000f, 1.36861789e+000f, 1.36993706e+000f, 1.37125576e+000f, 1.37257409e+000f, 1.37389195e+000f, 1.37520945e+000f, 
  1.37652647e+000f, 1.37784314e+000f, 1.37915933e+000f, 1.38047504e+000f, 1.38179052e+000f, 1.38310540e+000f, 1.38441992e+000f, 1.38573408e+000f, 
  1.38704777e+000f, 1.38836110e+000f, 1.38967395e+000f, 1.39098632e+000f, 1.39229846e+000f, 1.39361000e+000f, 1.39492130e+000f, 1.39623213e+000f, 
  1.39754248e+000f, 1.39885247e+000f, 1.40016210e+000f, 1.40147126e+000f, 1.40278006e+000f, 1.40408838e+000f, 1.40539634e+000f, 1.40670383e+000f, 
  1.40801096e+000f, 1.40931773e+000f, 1.41062403e+000f, 1.41192997e+000f, 1.41323555e+000f, 1.41454065e+000f, 1.41584539e+000f, 1.41714966e+000f, 
  1.41845369e+000f, 1.41975713e+000f, 1.42106032e+000f, 1.42236304e+000f, 1.42366540e+000f, 1.42496729e+000f, 1.42626882e+000f, 1.42756999e+000f, 
  1.42887068e+000f, 1.43017113e+000f, 1.43147099e+000f, 1.43277061e+000f, 1.43406975e+000f, 1.43536854e+000f, 1.43666697e+000f, 1.43796492e+000f, 
  1.43926251e+000f, 1.44055974e+000f, 1.44185662e+000f, 1.44315302e+000f, 1.44444907e+000f, 1.44574475e+000f, 1.44704008e+000f, 1.44833493e+000f, 
  1.44962943e+000f, 1.45092356e+000f, 1.45221722e+000f, 1.45351064e+000f, 1.45480359e+000f, 1.45609617e+000f, 1.45738840e+000f, 1.45868015e+000f, 
  1.45997167e+000f, 1.46126270e+000f, 1.46255338e+000f, 1.46384370e+000f, 1.46513355e+000f, 1.46642315e+000f, 1.46771228e+000f, 1.46900105e+000f, 
  1.47028947e+000f, 1.47157753e+000f, 1.47286522e+000f, 1.47415245e+000f, 1.47543931e+000f, 1.47672594e+000f, 1.47801208e+000f, 1.47929788e+000f, 
  1.48058331e+000f, 1.48186827e+000f, 1.48315299e+000f, 1.48443723e+000f, 1.48572123e+000f, 1.48700476e+000f, 1.48828793e+000f, 1.48957074e+000f, 
  1.49085319e+000f, 1.49213529e+000f, 1.49341702e+000f, 1.49469841e+000f, 1.49597943e+000f, 1.49725997e+000f, 1.49854028e+000f, 1.49982011e+000f, 
  1.50109971e+000f, 1.50237882e+000f, 1.50365770e+000f, 1.50493610e+000f, 1.50621414e+000f, 1.50749183e+000f, 1.50876927e+000f, 1.51004624e+000f, 
  1.51132286e+000f, 1.51259911e+000f, 1.51387501e+000f, 1.51515067e+000f, 1.51642585e+000f, 1.51770067e+000f, 1.51897514e+000f, 1.52024925e+000f, 
  1.52152300e+000f, 1.52279651e+000f, 1.52406955e+000f, 1.52534223e+000f, 1.52661455e+000f, 1.52788663e+000f, 1.52915823e+000f, 1.53042960e+000f, 
  1.53170049e+000f, 1.53297114e+000f, 1.53424132e+000f, 1.53551126e+000f, 1.53678071e+000f, 1.53804994e+000f, 1.53931880e+000f, 1.54058731e+000f, 
  1.54185545e+000f, 1.54312325e+000f, 1.54439068e+000f, 1.54565775e+000f, 1.54692459e+000f, 1.54819095e+000f, 1.54945707e+000f, 1.55072272e+000f, 
  1.55198812e+000f, 1.55325317e+000f, 1.55451787e+000f, 1.55578220e+000f, 1.55704618e+000f, 1.55830991e+000f, 1.55957317e+000f, 1.56083620e+000f, 
  1.56209886e+000f, 1.56336117e+000f, 1.56462312e+000f, 1.56588471e+000f, 1.56714606e+000f, 1.56840694e+000f, 1.56966758e+000f, 1.57092786e+000f, 
  1.57218778e+000f, 1.57344735e+000f, 1.57470667e+000f, 1.57596552e+000f, 1.57722414e+000f, 1.57848239e+000f, 1.57974029e+000f, 1.58099794e+000f, 
  1.58225513e+000f, 1.58351207e+000f, 1.58476865e+000f, 1.58602500e+000f, 1.58728087e+000f, 1.58853650e+000f, 1.58979177e+000f, 1.59104669e+000f, 
  1.59230125e+000f, 1.59355557e+000f, 1.59480953e+000f, 1.59606314e+000f, 1.59731638e+000f, 1.59856939e+000f, 1.59982204e+000f, 1.60107434e+000f, 
  1.60232627e+000f, 1.60357797e+000f, 1.60482931e+000f, 1.60608029e+000f, 1.60733092e+000f, 1.60858130e+000f, 1.60983133e+000f, 1.61108100e+000f, 
  1.61233044e+000f, 1.61357951e+000f, 1.61482823e+000f, 1.61607659e+000f, 1.61732471e+000f, 1.61857247e+000f, 1.61982000e+000f, 1.62106705e+000f, 
  1.62231386e+000f, 1.62356043e+000f, 1.62480664e+000f, 1.62605250e+000f, 1.62729800e+000f, 1.62854326e+000f, 1.62978816e+000f, 1.63103271e+000f, 
  1.63227701e+000f, 1.63352096e+000f, 1.63476455e+000f, 1.63600791e+000f, 1.63725090e+000f, 1.63849354e+000f, 1.63973594e+000f, 1.64097810e+000f, 
  1.64221978e+000f, 1.64346123e+000f, 1.64470232e+000f, 1.64594316e+000f, 1.64718366e+000f, 1.64842391e+000f, 1.64966381e+000f, 1.65090334e+000f, 
  1.65214264e+000f, 1.65338159e+000f, 1.65462017e+000f, 1.65585852e+000f, 1.65709651e+000f, 1.65833426e+000f, 1.65957165e+000f, 1.66080880e+000f, 
  1.66204560e+000f, 1.66328204e+000f, 1.66451824e+000f, 1.66575408e+000f, 1.66698968e+000f, 1.66822493e+000f, 1.66945994e+000f, 1.67069459e+000f, 
  1.67192888e+000f, 1.67316294e+000f, 1.67439675e+000f, 1.67563021e+000f, 1.67686331e+000f, 1.67809618e+000f, 1.67932868e+000f, 1.68056095e+000f, 
};

static int init_tab3_4 = 1;

/*
  table of pow(2.0,0.25*q)
*/


ALIGN_16_BYTE static const float newQuantTableQ[4]={
  1.000000000000f,
  1.189207115003f,
  1.414213562373f,
  1.681792830507f
};



#ifndef PREDEFINED_TABLES
static void initTables()
{
  int i;
  int j;
  for(i=0;i<256;i++)
    eTab_3_4[i] = (float)pow(pow(2.0,(double)i-127),0.75);
  for(i=0;i<MANT_SIZE;i++)
    mTab_3_4[i] = (float)pow(1.0+(double)i/MANT_SIZE,0.75);

#if 0 /* uncomment to print out table source code */
  printf("static const float eTab_3_4[256] =\n{\n");
  for(i=0;i<32;i++)
  {
    printf("  ");
    for(j=0;j<8;j++)
    { printf("%1.8ef, ",eTab_3_4[i*8+j]); }
    printf("\n");
  }
  printf("};\n\n");

  printf("static const float mTab_3_4[%d] =\n{\n",MANT_SIZE);
  for(i=0;i<64;i++)
  {
    printf("  ");
    for(j=0;j<8;j++)
    { printf("%1.8ef, ",mTab_3_4[i*8+j]); }
    printf("\n");
  }
  printf("};\n\n");
#endif
}
#endif /* PREDEFINED_TABLES */

static void quantizeLines_NoOpt(const int gain,
                          const int noOfLines,
                          const float * restrict mdctSpectrum,
                          signed int * restrict quaSpectrum)
{
  float k = logCon + 0.5f;
  float q1;
  int   g1;
  short sav_cw,new_cw;
 
  _asm{
          fnstcw sav_cw;
          mov ax,sav_cw;
          or  ax,0x0c00;      // rounding mode to "chop"
          mov new_cw,ax;
          fldcw new_cw;

          mov eax,gain
          neg eax
          mov g1,eax
          sar g1,2
          and eax,3
          
          fild dword ptr g1
          fld1
          fscale
          fmul dword ptr newQuantTableQ[eax*4]
          fxch st(1)
          fstp st(0)                // st(0) now quantizer
  
          mov eax,mdctSpectrum
          mov ebx,quaSpectrum
          mov ecx,noOfLines

l1:       fld   dword ptr [eax]
          fmul  st(0),st(1)
          fstp  dword ptr q1
          mov   edx,q1
          mov   esi,edx
          mov   edi,edx
          sar   edx,31           // 0 positive, -1 negative
          and   esi,0x7fffffff   // clear sign bit
          and   edi,0x7fffff     // mask mantissa
          shr   esi,23
          shr   edi,(23-MANT_DIGITS)
          fld   dword ptr eTab_3_4[esi*4]   // get exp  ^0.75
          fmul  dword ptr mTab_3_4[edi*4]   // get mant ^0.75
          fadd  dword ptr k
          fistp dword ptr [ebx]
          xor   [ebx],edx       // merge sign without cond.jump
          sub   [ebx],edx
          lea   eax,[eax+4]
          lea   ebx,[ebx+4]
          dec   ecx
          jnz l1
          fstp st(0)
          fldcw sav_cw        // restore control word
  }   
}
#endif /* #ifndef P4_INTRINSIC */

#endif  /* NEW_QUANT */

extern void (*quantizeLines)(const int gain,
                             const int noOfLines,
                             const float * restrict mdctSpectrum,
                             signed int * restrict quaSpectrum);

extern void (*quantizeExpSpecLines) (const int gain,
                                     const int noOfLines,
                                     const float * restrict expSpectrum,
                                     signed int * restrict quaSpectrum);

extern void (*calcExpSpec) (      float *expSpec, 
                            const float *mdctSpectrum,
                                  int    noOfLines );


#ifndef P4_CODE
void (*quantizeLines) (const int gain,
                       const int noOfLines,
                       const float * restrict mdctSpectrum,
                       signed int * restrict quaSpectrum) = quantizeLines_NoOpt;

/*****************************************************************************

    functionname:calcExpSpec
    description: calculets pow(x,075) on spectral lines  
    input: number of lines to process, spectral data         
    output: "compressed" spectrum

*****************************************************************************/
static void calcExpSpec_NoOpt(float *expSpec, const float *mdctSpectrum, int noOfLines)
{
   int i;
   float tmp;

   for (i=0; i<noOfLines; i++)
     {
       /* x^(3/4) */
       float sign = mdctSpectrum[i]>=0.f?1.f:-1.f;
       tmp = (float)fabs(mdctSpectrum[i]);
       tmp = (float)sqrt(tmp);
       expSpec[i] = sign * (float)(tmp * sqrt(tmp));
     }
}

void (*calcExpSpec) (      float *expSpec, 
                     const float *mdctSpectrum,
                           int noOfLines ) = calcExpSpec_NoOpt;

/*****************************************************************************

    functionname:quantizeExpSpecLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/

static void quantizeExpSpecLines_NoOpt(const int gain,
                                       const int noOfLines,
                                       const float * restrict expSpectrum,
                                       signed int * restrict quaSpectrum)
{
  float quantizer;
  float k = logCon + 0.5f;
  int line;
  /*int noline=1; */

  quantizer=quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 

  for (line = 0; line < noOfLines; line++)
  {
    float tmp  = expSpectrum[line];

    if (tmp < 0.0f) {
      tmp = -tmp;
      quaSpectrum[line] = -(int)(k + quantizer * tmp);
    }
    else {
      quaSpectrum[line] = (int)(k + quantizer * tmp);
    }
  }
}

void (*quantizeExpSpecLines) (const int gain,
                              const int noOfLines,
                              const float * restrict expSpectrum,
                              signed int * restrict quaSpectrum) = quantizeExpSpecLines_NoOpt;

#endif /* #ifndef P4_CODE */

/*****************************************************************************

    functionname: calcSfbDist
    description: calculates distortion of quantized values
    returns: distortion
    input: gain, number of lines to process, spectral data
    output:

*****************************************************************************/
float calcSfbDist(const float *mdctSpectrum,
                  const float *expSpec,
                  int         *quantSpec,
                  int          noOfLines,
                  int          gain)
{
   float quantizer, invQuantizer;
   float k = logCon + 0.5f;
   int i;
   float invQuantSpec;
   float diff;
   float xfsf = 0.0f;

   quantizer    = quantTableE[(gain>>4)+8]*quantTableQ[gain & 15];
   invQuantizer = invQuantTableE[(gain>>4)+8]*invQuantTableQ[gain & 15];

   for (i=0; i<noOfLines; i++)
     {
       /* quantization */
       quantSpec[i] = (int)(k + quantizer * expSpec[i]);
       if (quantSpec[i] >= 8192)
         return FLT_MAX; /*1.0e100f;*/

       /* inverse quantization */
       invQuantSpec = pow4_3_tab[quantSpec[i]] * invQuantizer;

       if( mdctSpectrum[i] < 0.0f )
	 quantSpec[i] = -quantSpec[i];

       /* distortion */
       diff = invQuantSpec - (float)fabs(mdctSpectrum[i]);
       xfsf += diff * diff;
     }

   return xfsf;
}

int calcStepsize( int       blockType,
                  int       sfb,
                  int       scf,
                  int       scfScale,
                  int       globalGain,
                  const int subBlockGain[TRANS_FAC],
                  int       preEmphasisFlag )
{
  int scalefactor;
  if( blockType == SHORT_WINDOW )
    scalefactor = (sfb < (MAX_SFB_SHORT-1)*TRANS_FAC ?
                   (scf <<(1+scfScale)) + 8 * subBlockGain[sfb % TRANS_FAC] : 0);
  else
    scalefactor = (sfb < MAX_SFB_LONG-1 ? (scf+ (preEmphasisFlag ? preEmphasisTab[sfb]:0))<<(1+scfScale) : 0);

   return globalGain - scalefactor;
}

int improveScf( const float *spec,
		const float *expSpec,
		int         *quantSpec,
		int         *quantSpecTmp,
		int          sfbWidth,
		float        thresh,
		int          blockType,
		int          sfb,
		int          scf,
		int          maxScf,
		int          scfScale,
		int          globalGain,
		const int    subBlockGain[TRANS_FAC],
		int          preEmphasisFlag,
                float        maxSpec )
{
  float sfbDist;
  float sfbDistBest;
  int scfBest;
  int stepsize;
  int minStepsize = (int)ceil(C1 + C2*log(maxSpec));
  int maxStepsize = (int)ceil(C0 + C2*log(maxSpec));
  /*float maxStepsize = C0 + C2*log(maxSpec);*/

  while( 1 )
    {
      stepsize = calcStepsize( blockType, sfb, scf, scfScale,
                               globalGain, subBlockGain, preEmphasisFlag );
      if( stepsize >= minStepsize || scf == 0 )
        break;
      /* else: scalefactor to big, quantized values would be > 8191 */
      scf--;
      maxScf = scf;
    }

  /* check all mdct-lines in this sfb would be quantized to zero */
  if( stepsize > maxStepsize )
    {
      scfBest = SCF_DONT_CARE;
      sfbDistBest = FLT_MAX; /*1.0e100f;*/
    }
  else
    {
      /* quantize and calc distortion */
      sfbDistBest = calcSfbDist( spec, expSpec, quantSpec, sfbWidth, stepsize );
      scfBest = scf;

      /* improve by smaller scf ? */
      if( scf > 0 ) 
        {
          stepsize = calcStepsize( blockType, sfb, scf-1, scfScale,
                                   globalGain, subBlockGain, preEmphasisFlag );
          if( stepsize <= maxStepsize )
            {
              sfbDist = calcSfbDist( spec, expSpec, quantSpecTmp, sfbWidth, stepsize );
              if( sfbDist < 1.25f*thresh || sfbDist <= sfbDistBest )
                {
                  scfBest = scf-1;
                  sfbDistBest = sfbDist;
                  copyINT(quantSpecTmp, quantSpec, sfbWidth);
                }
            }
        }
    }

  /* NMR > 1dB -> try to improve NMR by bigger scf */
  if( (sfbDistBest > 1.25f*thresh) && (scf+1 < maxScf) )
    {
      stepsize = calcStepsize( blockType, sfb, scf+1, scfScale,
                               globalGain, subBlockGain, preEmphasisFlag );

      if( stepsize < minStepsize )
        return scfBest; /* quantized value > 8191 */

      if( stepsize > maxStepsize )
        return SCF_DONT_CARE; /* all lines quantized to zero, try bigger scf */

      sfbDist = calcSfbDist( spec, expSpec, quantSpecTmp, sfbWidth, stepsize );
      if( sfbDist < sfbDistBest )
        {
          scfBest = scf+1;
          sfbDistBest = sfbDist;
          copyINT(quantSpecTmp, quantSpec, sfbWidth);
        }
    }

  if( stepsize > maxStepsize )
    return SCF_DONT_CARE;  /* all lines would be quantized to zero */

  /* return best scalefactor */
  return scfBest;
}

/*****************************************************************************

    functionname:mp3QuantizeSpectrum 
    description: quantizes the entire spectrum
    input:  number of sfbs to be quantized, ...
    output: quantized spectrum

*****************************************************************************/
void 
mp3QuantizeSpectrum(const int    sfbActive,
			  const int   *sfbOffset, 
			  const float *mdctSpectrum,
			  const float *threshold,
			  const float *sfbMaxSpec,
			  const int    globalGain,
			  const int    blockType,
			  const int    subBlockGain[TRANS_FAC],
			  const int    preEmphasisFlag,
			  const int    scfScale,
			  const int   *maxScf,
			  const int    fullPsych,
			  int         *scalefactors,
			  int         *quantizedSpectrum)
{
  ALIGN_16_BYTE int quantSpecTmp[FRAME_LEN_LONG];
  ALIGN_16_BYTE float expSpec[FRAME_LEN_LONG];
  int sfb, sfbMax;

#ifndef PREDEFINED_TABLES
#ifndef OLD_QUANT
  if(!init_tab3_4){
    initTables();
    init_tab3_4=1;
  }
#endif
#endif /* PREDEFINED_TABLES */

  if( !pow4_3_init )
    {
      int i;
      for (i = 0; i <= MAX_QUANT; i++)
        pow4_3_tab[i] = (float)(pow((float)i, 4.0f/3.0f));
      pow4_3_init = 1;
    }
  
  if(blockType == SHORT_WINDOW)
    sfbMax = (MAX_SFB_SHORT-1)*TRANS_FAC;
  else
    sfbMax = MAX_SFB_LONG-1;
#if 1
  for (sfb = 0; sfb < sfbActive; sfb++)
    {
      if (scalefactors[sfb] != SCF_DONT_CARE)
        {
          if( fullPsych ){  /* improve scalefactors */
            calcExpSpec( expSpec          + sfbOffset[sfb],
                         mdctSpectrum     + sfbOffset[sfb],
                         sfbOffset[sfb+1] - sfbOffset[sfb] );
            
            scalefactors[sfb] = improveScf( mdctSpectrum      + sfbOffset[sfb],
                                            expSpec           + sfbOffset[sfb],
                                            quantizedSpectrum + sfbOffset[sfb],
                                            quantSpecTmp      + sfbOffset[sfb],
                                            sfbOffset[sfb+1]  - sfbOffset[sfb],
                                            sfb<sfbMax?threshold[sfb]:FLT_MAX/*1e100*/, blockType,
                                            sfb, sfb<sfbMax?scalefactors[sfb]:0,
                                            maxScf[sfb], scfScale, globalGain,
                                            subBlockGain, preEmphasisFlag, sfbMaxSpec[sfb] );
          }
          else {
            int scalefactor;
            
            if(blockType == SHORT_WINDOW)
              {
                scalefactor = (sfb < (MAX_SFB_SHORT-1)*TRANS_FAC ?
                               (scalefactors[sfb] <<(1+scfScale)) + 8 * subBlockGain[sfb % TRANS_FAC] : 0);
              }
            else
              {
                scalefactor = (sfb < MAX_SFB_LONG-1 ? (scalefactors[sfb]+ (preEmphasisFlag ? preEmphasisTab[sfb]:0))<<(1+scfScale) : 0);
              }
            quantizeLines(globalGain - scalefactor,
                          sfbOffset[sfb+1] - sfbOffset[sfb],
                          mdctSpectrum + sfbOffset[sfb],
                          quantizedSpectrum + sfbOffset[sfb]);
          }
        }

      if (scalefactors[sfb] == SCF_DONT_CARE)
        {
          /* if scalefactor == SCF_DONT_CARE, set spectrum to zero. */
          setINT(0, quantizedSpectrum + sfbOffset[sfb], sfbOffset[sfb+1] - sfbOffset[sfb]);
        }
    }
#else
  /* calculate 'compressed' spectrum:  expSpec = pow(spec,0.75) */ 
  calcExpSpec( expSpec,
               mdctSpectrum,
               sfbOffset[sfbActive] );
  setFLOAT ( 0.0f, expSpec + sfbOffset[sfbActive], FRAME_LEN_LONG - sfbOffset[sfbActive]);

  for (sfb = 0; sfb < sfbActive; sfb++) {

    if (scalefactors[sfb] != SCF_DONT_CARE) {
      if( fullPsych ){  /* improve scalefactors */
        scalefactors[sfb] = improveScf( mdctSpectrum      + sfbOffset[sfb],
                                        expSpec           + sfbOffset[sfb],
                                        quantizedSpectrum + sfbOffset[sfb],
                                        quantSpecTmp      + sfbOffset[sfb],
                                        sfbOffset[sfb+1]  - sfbOffset[sfb],
                                        sfb<sfbMax?threshold[sfb]:FLT_MAX/*1e100*/, blockType,
                                        sfb, sfb<sfbMax?scalefactors[sfb]:0,
                                        maxScf[sfb], scfScale, globalGain,
                                        subBlockGain, preEmphasisFlag, sfbMaxSpec[sfb] );
      }
      else {
        int scalefactor;
            
        if(blockType == SHORT_WINDOW) {
          scalefactor = (sfb < (MAX_SFB_SHORT-1)*TRANS_FAC ?
                         (scalefactors[sfb] <<(1+scfScale)) + 8 * subBlockGain[sfb % TRANS_FAC] : 0);
        }
        else {
          scalefactor = (sfb < MAX_SFB_LONG-1 ? (scalefactors[sfb]+ (preEmphasisFlag ? preEmphasisTab[sfb]:0))<<(1+scfScale) : 0);
        }
        quantizeExpSpecLines(globalGain - scalefactor,
                             sfbOffset[sfb+1] - sfbOffset[sfb],
                             expSpec + sfbOffset[sfb],
                             quantizedSpectrum + sfbOffset[sfb]);
      }
    }

    if (scalefactors[sfb] == SCF_DONT_CARE) {
      /* if scalefactor == SCF_DONT_CARE, set spectrum to zero. */
      setINT(0, quantizedSpectrum + sfbOffset[sfb], sfbOffset[sfb+1] - sfbOffset[sfb]);
    }
  }
#endif
  /*
    quench the rest of the spectrum
  */
  setINT(0, quantizedSpectrum + sfbOffset[sfbActive], FRAME_LEN_LONG - sfbOffset[sfbActive]);
}

/*****************************************************************************

    functionname:invQuantizeLines 
    description: dequantizes spectrum lines  
                 invquaSpectrum = (mdctSpectrum^(4/3))*2^((3/16)*gain)

                 This function is needed for debugging purposes only.

    input:  gain, number of lines to process, spectral data         
    output: dequantized spectrum

*****************************************************************************/
 

static void invQuantizeLines(const int gain,
                             const int noOfLines,
                             const signed int * quaSpectrum,
                             float * mdctSpectrum)
{
  float invQuantizer;
  int line;

  invQuantizer = invQuantTableE[(gain>>4)+8]*invQuantTableQ[gain & 15];

  for (line = 0; line < noOfLines; line++)
    mdctSpectrum[line] = ( quaSpectrum[line] <= 0 ?
                           -pow4_3_tab[-quaSpectrum[line]] * invQuantizer :
                            pow4_3_tab[ quaSpectrum[line]] * invQuantizer );
}



/*****************************************************************************

    functionname:mp3InvQuantizeSpectrum 
    description: dequantizes the entire spectrum
    input: number of scalefactor bands to be dequantized, ...
    output: dequantized spectrum

*****************************************************************************/

void mp3InvQuantizeSpectrum(const int    sfbCnt, 
                            const int   *sfbOffset, 
                            const int   *quaSpectrum,
                            const int    globalGain,
                            const int    blockType,
                            const int    subBlockGain[TRANS_FAC],
                            const int    preEmphasisFlag,
                            const int    scfScale,
                            const int   *scalefactors,
                            float       *invQuantizedSpectrum)
{
  int sfb;
  
  for (sfb = 0; sfb < sfbCnt; sfb++)
  {
    int scalefactor;

    if (scalefactors[sfb] != SCF_DONT_CARE)
    {
      if(blockType == SHORT_WINDOW)
      {
        scalefactor = (sfb < (MAX_SFB_SHORT-1)*TRANS_FAC ?
                       (scalefactors[sfb] <<(1+scfScale)) + 8 * subBlockGain[sfb % TRANS_FAC] : 0);
      }
      else
      {
        scalefactor = (sfb < MAX_SFB_LONG-1 ? (scalefactors[sfb]+ (preEmphasisFlag ? preEmphasisTab[sfb]:0))<<(1+scfScale) : 0);
      }

      invQuantizeLines(globalGain - scalefactor,
                       sfbOffset[sfb+1] - sfbOffset[sfb],
                       quaSpectrum + sfbOffset[sfb],
                       invQuantizedSpectrum + sfbOffset[sfb]);
    }
    else
    {
      /* if scalefactor == SCF_DONT_CARE, set spectrum to zero. */
      setFLOAT(0.0f, invQuantizedSpectrum + sfbOffset[sfb], sfbOffset[sfb+1] - sfbOffset[sfb]);
    }
  }
}
