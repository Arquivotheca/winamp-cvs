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
*   $Id: polyana.c,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"

#include "mp3alloc.h"
#include <math.h>
#include <assert.h>

#include "mathlib.h"
#include "psy_const.h"
#include "polyana.h"

#define DCT_LEN    POLY_PHASE_BANDS /* 32 */
#define LD_DCT_LEN 5
#define MDCT_SIZE  (FRAME_LEN_LONG/DCT_LEN) /* 18 */
#define pM64(a) ((__m64*)(a))
#define pM32(a) ((float*)(a))

/*
  The origin of these coefficients is not entirely clear; they are not equal
  to the coefficients in the standard.
  However, if you round these coefficients to 17 bits, you will get the
  coefficients used in the standard.

  Also, Coeffs (17..31),(47..63) , (81..95), ) (111,127),... of the analysis
  filter have been reversed.

  a dummy coefficient has been inserted at the start of the table such that most
  float*2 accesses happen on 8-byte boundaries
*/


ALIGN_16_BYTE static const float anaWindow[POLY_WINDOW_SIZE] =
{
  +0.0000000000e+000F,-2.8706250532e-007F,-3.5053125202e-007F,-4.2056248617e-007F,
  -4.9878127584e-007F,-5.8506248024e-007F,-6.8125001462e-007F,-7.8746876397e-007F,
  -9.0578123491e-007F,-1.0366875358e-006F,-1.1825000001e-006F,-1.3440625253e-006F,
  -1.5241249685e-006F,-1.7238124883e-006F,-1.9463436729e-006F,-2.1932812615e-006F,
  -2.4682499316e-006F,-1.2405875168e-005F,-1.1249562704e-005F,-1.0184375242e-005F,
  -9.2062809927e-006F,-8.3092809291e-006F,-7.4893123383e-006F,-6.7404689617e-006F,
  -6.0587813095e-006F,-5.4385623116e-006F,-4.8760939535e-006F,-4.3660625124e-006F,
  -3.9050623855e-006F,-3.4881875308e-006F,-3.1124061479e-006F,-2.7732812669e-006F,
  -1.3659156139e-005F,-1.5013280972e-005F,-1.6473719370e-005F,-1.8043874661e-005F,
  -1.9728593543e-005F,-2.1530562663e-005F,-2.3453718313e-005F,-2.5499719413e-005F,
  -2.7671281714e-005F,-2.9968656236e-005F,-3.2392967114e-005F,-3.4942688217e-005F,
  -3.7617061025e-005F,-4.0412374801e-005F,-4.3325562729e-005F,-4.6350374760e-005F,
  -4.9481001042e-005F,-9.9233686342e-005F,-9.6532341558e-005F,-9.3615002697e-005F,
  -9.0518311481e-005F,-8.7275097030e-005F,-8.3917751908e-005F,-8.0474907008e-005F,
  -7.6974683907e-005F,-7.3441406130e-005F,-6.9899033406e-005F,-6.6367691034e-005F,
  -6.2867220549e-005F,-5.9413781855e-005F,-5.6023342040e-005F,-5.2708281146e-005F,
  +1.0168224981e-004F,+1.0383734480e-004F,+1.0565834236e-004F,+1.0710106289e-004F,
  +1.0812150140e-004F,+1.0867234232e-004F,+1.0870687402e-004F,+1.0817537259e-004F,
  +1.0702919099e-004F,+1.0521706281e-004F,+1.0268924962e-004F,+9.9394063000e-005F,
  +9.5281844551e-005F,+9.0301655291e-005F,+8.4405124653e-005F,+7.7543438238e-005F,
  +6.9671128585e-005F,-1.9143077952e-004F,-1.6523090017e-004F,-1.4025041310e-004F,
  -1.1651884415e-004F,-9.4059440016e-005F,-7.2885595728e-005F,-5.3005249356e-005F,
  -3.4417469578e-005F,-1.7116624804e-005F,-1.0892186992e-006F,+1.3681968994e-005F,
  +2.7221156415e-005F,+3.9555376134e-005F,+5.0717626436e-005F,+6.0742844653e-005F,
  -2.1880993154e-004F,-2.4732109159e-004F,-2.7690574643e-004F,-3.0749753932e-004F,
  -3.3901844290e-004F,-3.7138155312e-004F,-4.0448844084e-004F,-4.3823156739e-004F,
  -4.7249093768e-004F,-5.0713779638e-004F,-5.4203061154e-004F,-5.7701906189e-004F,
  -6.1194063164e-004F,-6.4662407385e-004F,-6.8088626722e-004F,-7.1453658165e-004F,
  -7.4737216346e-004F,-9.8377501126e-004F,-9.9169567693e-004F,-9.9513528403e-004F,
  -9.9434028380e-004F,-9.8956434522e-004F,-9.8106218502e-004F,-9.6909282729e-004F,
  -9.5391372452e-004F,-9.3578471569e-004F,-9.1496156529e-004F,-8.9169904822e-004F,
  -8.6624658434e-004F,-8.3885126514e-004F,-8.0975220772e-004F,-7.7918404713e-004F,
  +9.7113155061e-004F,+9.5353252254e-004F,+9.3075342011e-004F,+9.0258219279e-004F,
  +8.6881720927e-004F,+8.2927377662e-004F,+7.8377971658e-004F,+7.3218345642e-004F,
  +6.7434966331e-004F,+6.1016622931e-004F,+5.3954095347e-004F,+4.6240750817e-004F,
  +3.7872343091e-004F,+2.8847393696e-004F,+1.9167178834e-004F,+8.8360720838e-005F,
  -2.1386218577e-005F,-2.2833053954e-003F,-2.1098139696e-003F,-1.9373561954e-003F,
  -1.7665550113e-003F,-1.5979950549e-003F,-1.4322253410e-003F,-1.2697553029e-003F,
  -1.1110581690e-003F,-9.5656688791e-004F,-8.0667814473e-004F,-6.6174875246e-004F,
  -5.2209873684e-004F,-3.8800999755e-004F,-2.5972820004e-004F,-1.3746206241e-004F,
  -2.4571737740e-003F,-2.6307257358e-003F,-2.8032362461e-003F,-2.9739462771e-003F,
  -3.1420688611e-003F,-3.3067844342e-003F,-3.4672531765e-003F,-3.6226001102e-003F,
  -3.7719407119e-003F,-3.9143622853e-003F,-4.0489342064e-003F,-4.1747125797e-003F,
  -4.2907469906e-003F,-4.3960749172e-003F,-4.4897217304e-003F,-4.5707216486e-003F,
  -4.6381060965e-003F,-3.4019530285e-003F,-3.6430594046e-003F,-3.8586657029e-003F,
  -4.0492843837e-003F,-4.2154751718e-003F,-4.3578529730e-003F,-4.4770813547e-003F,
  -4.5738751069e-003F,-4.6489844099e-003F,-4.7032064758e-003F,-4.7373627312e-003F,
  -4.7523155808e-003F,-4.7489469871e-003F,-4.7281687148e-003F,-4.6909060329e-003F,
  +3.1349093188e-003F,+2.8415385168e-003F,+2.5215274654e-003F,+2.1746265702e-003F,
  +1.8006606260e-003F,+1.3995256741e-003F,+9.7119563725e-004F,+5.1572092343e-004F,
  +3.3233154682e-005F,-4.7605688451e-004F,-1.0118538048e-003F,-1.5737812500e-003F,
  -2.1613771096e-003F,-2.7740967926e-003F,-3.4113093279e-003F,-4.0723029524e-003F,
  -4.7562778927e-003F,-1.6994385049e-002F,-1.6112888232e-002F,-1.5233700164e-002F,
  -1.4358599670e-002F,-1.3489328325e-002F,-1.2627578340e-002F,-1.1774993502e-002F,
  -1.0933165438e-002F,-1.0103630833e-002F,-9.2878621072e-003F,-8.4872655571e-003F,
  -7.7031874098e-003F,-6.9368905388e-003F,-6.1895716935e-003F,-5.4623531178e-003F,
  -1.7876377329e-002F,-1.8757028505e-002F,-1.9634462893e-002F,-2.0506802946e-002F,
  -2.1372143179e-002F,-2.2228583694e-002F,-2.3074213415e-002F,-2.3907124996e-002F,
  -2.4725424126e-002F,-2.5527236983e-002F,-2.6310702786e-002F,-2.7073994279e-002F,
  -2.7815310284e-002F,-2.8532896191e-002F,-2.9225043952e-002F,-2.9890084639e-002F,
  -3.0526412651e-002F,-3.5759095103e-002F,-3.5694155842e-002F,-3.5586155951e-002F,
  -3.5435311496e-002F,-3.5242062062e-002F,-3.5006813705e-002F,-3.4730218351e-002F,
  -3.4412968904e-002F,-3.4055873752e-002F,-3.3659782261e-002F,-3.3225689083e-002F,
  -3.2754719257e-002F,-3.2248001546e-002F,-3.1706813723e-002F,-3.1132483855e-002F,
  +3.5780750215e-002F,+3.5759095103e-002F,+3.5694155842e-002F,+3.5586155951e-002F,
  +3.5435311496e-002F,+3.5242062062e-002F,+3.5006813705e-002F,+3.4730218351e-002F,
  +3.4412968904e-002F,+3.4055873752e-002F,+3.3659782261e-002F,+3.3225689083e-002F,
  +3.2754719257e-002F,+3.2248001546e-002F,+3.1706813723e-002F,+3.1132483855e-002F,
  +3.0526412651e-002F,+1.8757028505e-002F,+1.9634462893e-002F,+2.0506802946e-002F,
  +2.1372143179e-002F,+2.2228583694e-002F,+2.3074213415e-002F,+2.3907124996e-002F,
  +2.4725424126e-002F,+2.5527236983e-002F,+2.6310702786e-002F,+2.7073994279e-002F,
  +2.7815310284e-002F,+2.8532896191e-002F,+2.9225043952e-002F,+2.9890084639e-002F,
  +1.7876377329e-002F,+1.6994385049e-002F,+1.6112888232e-002F,+1.5233700164e-002F,
  +1.4358599670e-002F,+1.3489328325e-002F,+1.2627578340e-002F,+1.1774993502e-002F,
  +1.0933165438e-002F,+1.0103630833e-002F,+9.2878621072e-003F,+8.4872655571e-003F,
  +7.7031874098e-003F,+6.9368905388e-003F,+6.1895716935e-003F,+5.4623531178e-003F,
  +4.7562778927e-003F,-2.8415385168e-003F,-2.5215274654e-003F,-2.1746265702e-003F,
  -1.8006606260e-003F,-1.3995256741e-003F,-9.7119563725e-004F,-5.1572092343e-004F,
  -3.3233154682e-005F,+4.7605688451e-004F,+1.0118538048e-003F,+1.5737812500e-003F,
  +2.1613771096e-003F,+2.7740967926e-003F,+3.4113093279e-003F,+4.0723029524e-003F,
  +3.1349093188e-003F,+3.4019530285e-003F,+3.6430594046e-003F,+3.8586657029e-003F,
  +4.0492843837e-003F,+4.2154751718e-003F,+4.3578529730e-003F,+4.4770813547e-003F,
  +4.5738751069e-003F,+4.6489844099e-003F,+4.7032064758e-003F,+4.7373627312e-003F,
  +4.7523155808e-003F,+4.7489469871e-003F,+4.7281687148e-003F,+4.6909060329e-003F,
  +4.6381060965e-003F,+2.6307257358e-003F,+2.8032362461e-003F,+2.9739462771e-003F,
  +3.1420688611e-003F,+3.3067844342e-003F,+3.4672531765e-003F,+3.6226001102e-003F,
  +3.7719407119e-003F,+3.9143622853e-003F,+4.0489342064e-003F,+4.1747125797e-003F,
  +4.2907469906e-003F,+4.3960749172e-003F,+4.4897217304e-003F,+4.5707216486e-003F,
  +2.4571737740e-003F,+2.2833053954e-003F,+2.1098139696e-003F,+1.9373561954e-003F,
  +1.7665550113e-003F,+1.5979950549e-003F,+1.4322253410e-003F,+1.2697553029e-003F,
  +1.1110581690e-003F,+9.5656688791e-004F,+8.0667814473e-004F,+6.6174875246e-004F,
  +5.2209873684e-004F,+3.8800999755e-004F,+2.5972820004e-004F,+1.3746206241e-004F,
  +2.1386218577e-005F,-9.5353252254e-004F,-9.3075342011e-004F,-9.0258219279e-004F,
  -8.6881720927e-004F,-8.2927377662e-004F,-7.8377971658e-004F,-7.3218345642e-004F,
  -6.7434966331e-004F,-6.1016622931e-004F,-5.3954095347e-004F,-4.6240750817e-004F,
  -3.7872343091e-004F,-2.8847393696e-004F,-1.9167178834e-004F,-8.8360720838e-005F,
  +9.7113155061e-004F,+9.8377501126e-004F,+9.9169567693e-004F,+9.9513528403e-004F,
  +9.9434028380e-004F,+9.8956434522e-004F,+9.8106218502e-004F,+9.6909282729e-004F,
  +9.5391372452e-004F,+9.3578471569e-004F,+9.1496156529e-004F,+8.9169904822e-004F,
  +8.6624658434e-004F,+8.3885126514e-004F,+8.0975220772e-004F,+7.7918404713e-004F,
  +7.4737216346e-004F,+2.4732109159e-004F,+2.7690574643e-004F,+3.0749753932e-004F,
  +3.3901844290e-004F,+3.7138155312e-004F,+4.0448844084e-004F,+4.3823156739e-004F,
  +4.7249093768e-004F,+5.0713779638e-004F,+5.4203061154e-004F,+5.7701906189e-004F,
  +6.1194063164e-004F,+6.4662407385e-004F,+6.8088626722e-004F,+7.1453658165e-004F,
  +2.1880993154e-004F,+1.9143077952e-004F,+1.6523090017e-004F,+1.4025041310e-004F,
  +1.1651884415e-004F,+9.4059440016e-005F,+7.2885595728e-005F,+5.3005249356e-005F,
  +3.4417469578e-005F,+1.7116624804e-005F,+1.0892186992e-006F,-1.3681968994e-005F,
  -2.7221156415e-005F,-3.9555376134e-005F,-5.0717626436e-005F,-6.0742844653e-005F,
  -6.9671128585e-005F,-1.0383734480e-004F,-1.0565834236e-004F,-1.0710106289e-004F,
  -1.0812150140e-004F,-1.0867234232e-004F,-1.0870687402e-004F,-1.0817537259e-004F,
  -1.0702919099e-004F,-1.0521706281e-004F,-1.0268924962e-004F,-9.9394063000e-005F,
  -9.5281844551e-005F,-9.0301655291e-005F,-8.4405124653e-005F,-7.7543438238e-005F,
  +1.0168224981e-004F,+9.9233686342e-005F,+9.6532341558e-005F,+9.3615002697e-005F,
  +9.0518311481e-005F,+8.7275097030e-005F,+8.3917751908e-005F,+8.0474907008e-005F,
  +7.6974683907e-005F,+7.3441406130e-005F,+6.9899033406e-005F,+6.6367691034e-005F,
  +6.2867220549e-005F,+5.9413781855e-005F,+5.6023342040e-005F,+5.2708281146e-005F,
  +4.9481001042e-005F,+1.5013280972e-005F,+1.6473719370e-005F,+1.8043874661e-005F,
  +1.9728593543e-005F,+2.1530562663e-005F,+2.3453718313e-005F,+2.5499719413e-005F,
  +2.7671281714e-005F,+2.9968656236e-005F,+3.2392967114e-005F,+3.4942688217e-005F,
  +3.7617061025e-005F,+4.0412374801e-005F,+4.3325562729e-005F,+4.6350374760e-005F,
  +1.3659156139e-005F,+1.2405875168e-005F,+1.1249562704e-005F,+1.0184375242e-005F,
  +9.2062809927e-006F,+8.3092809291e-006F,+7.4893123383e-006F,+6.7404689617e-006F,
  +6.0587813095e-006F,+5.4385623116e-006F,+4.8760939535e-006F,+4.3660625124e-006F,
  +3.9050623855e-006F,+3.4881875308e-006F,+3.1124061479e-006F,+2.7732812669e-006F,
  +2.4682499316e-006F,+2.8706250532e-007F,+3.5053125202e-007F,+4.2056248617e-007F,
  +4.9878127584e-007F,+5.8506248024e-007F,+6.8125001462e-007F,+7.8746876397e-007F,
  +9.0578123491e-007F,+1.0366875358e-006F,+1.1825000001e-006F,+1.3440625253e-006F,
  +1.5241249685e-006F,+1.7238124883e-006F,+1.9463436729e-006F,+2.1932812615e-006F
};



static ALIGN_16_BYTE const float dct_iii_32_trig_data[44] =
{
  0.9238795325F,0.3826834324F,0.7071067812F,0.7071067812F,
  0.9807852804F,0.1950903220F,0.8314696123F,0.5555702330F,
  0.9238795325F,0.3826834324F,0.3826834324F,0.9238795325F,
  0.9951847267F,0.0980171403F,0.8819212643F,0.4713967368F,
  0.9569403357F,0.2902846773F,0.7730104534F,0.6343932842F,
  0.9807852804F,0.1950903220F,0.5555702330F,0.8314696123F,
  0.8314696123F,0.5555702330F,0.1950903220F,0.9807852804F,
  0.9987954562F,0.0490676743F,0.9039892931F,0.4275550934F,
  0.9700312532F,0.2429801799F,0.8032075315F,0.5956993045F,
  0.9891765100F,0.1467304745F,0.8577286100F,0.5141027442F,
  0.9415440652F,0.3368898534F,0.7409511254F,0.6715589548F
};


/*****************************************************************************

    functionname: fct_iii
    description:  DCT Type III Transform
                  a[k]=sum {j=0..n-1} F[j]*cos(pi*j*(k+1/2)/n),0<=k<n

                  optimized version with trig data table look up for n=32
                  trigdata generation for general case commented out
    input:        32 time signal values
    output:       32 transformed values

*****************************************************************************/
static void fct_iii(int n, float *a)
{
  int m, mh, mq, i, j, irev, jr, ji, kr, ki;
  /*int k=0; */
  float  wr, wi, xr, xi;
  const float *tPtr;

  assert(n==32);
  
  tPtr = dct_iii_32_trig_data;
  /*
    theta = 4 * atan(1.0) / n;
  */
  if (n > 1)
  {
    m = n >> 1;
         
    /*
      wr=cos(0.5 * theta * m);
    */
    wr = tPtr[2];
    xr = a[m] * wr;
      
    a[m] = a[0] - xr;
    a[0] += xr;
  }
  for (m = n; (mh = m >> 1) >= 2; m = mh)
  {
         
    mq = mh >> 1;
    /* ---- real & complex to real butterflies ---- */
    
    /*
      irev = 0;
    */

    for (jr = 0; jr < n; jr += m)
    {
      float t1,t2,t3,t4 ;
      /* 
         wr = cos(0.5 * theta * (irev + mq));
         wi = sin(0.5 * theta * (irev + mq));
         for (k = n >> 2; k > (irev ^= k); k >>= 1);
      */
      wr=*tPtr++;
      wi=*tPtr++;
      
      ki = jr + mq;
      kr = n - ki;
      ji = kr - mq;
      xr = wr * a[ki] + wi * a[kr];
      xi = wr * a[kr] - wi * a[ki];

      t1 = a[jr] - xr;
      t2 = a[ji] + xi;
      t3 = a[jr] + xr;
      t4 = a[ji] - xi;

      a[kr] = t1;
      a[ki] = t2;
      a[jr] = t3;
      a[ji] = t4;
    }

    if (mh == 2) continue;
    /* ---- complex to complex butterflies ---- */
    irev = 0;
    for (i = 0; i < n; i += m)
    {
      /*
        wr = cos(theta * (irev + mq));
        wi = sin(theta * (irev + mq));
        for (k = n >> 2; k > (irev ^= k); k >>= 1);
      */

      wr=*tPtr++;
      wi=*tPtr++;
             
      for (j = 1; j < mq; j++)
      {
        float t1,t2,t3,t4 ;
        jr = i + j;
        ki = i + mh - j;
        ji = n - jr;
        kr = n - ki;
        xr = wr * a[ki] + wi * a[kr];
        xi = wr * a[kr] - wi * a[ki];

        t1 = a[jr] - xr;
        t2 = -a[ji] - xi;
        t3 = a[jr] + xr;
        t4 = a[ji] - xi;

        a[kr] = t1;
        a[ki] = t2;
        a[jr] = t3;
        a[ji] = t4;

      }
    }
  }

  /* ---- unscrambler ---- */
#if 0
  i = 0;
  for (j = 1; j < n - 1; j++)
  {
    for (k = n >> 1; k > (i ^= k); k >>= 1)
	  (void) 0;
    if (j < i)
    {
	  xr = a[j];
      a[j] = a[i];
      a[i] = xr;
    }
  }
#else
   #define swap_2(a1,a2) xr=a[a1]; a[a1]=a[a2]; a[a2]=xr;
   swap_2(1,16);
   swap_2(2,8);
   swap_2(3,24);
   swap_2(5,20);
   swap_2(6,12);
   swap_2(7,28);
   swap_2(9,18);
   swap_2(11,26);
   swap_2(13,22);
   swap_2(15,30);
   swap_2(19,25);
   swap_2(23,29);
#endif
}

static void resortAnaWindow(float *anaWindowNew, const float *anaWindowRef)
{
  int i, winIndex;
  for (winIndex=0; winIndex<POLY_WINDOW_SIZE; winIndex+=(2*DCT_LEN)) {
    const float *src;
    float *dst;

    src = &anaWindowRef[winIndex];
    dst = &anaWindowNew[winIndex];

    for (i=0; i<=DCT_LEN/2; i++) {
      dst[i] = src[DCT_LEN/2-i];
    }
    for (i=0; i<(DCT_LEN/2-1); i++) {
      dst[(DCT_LEN/2)+1+i] = src[DCT_LEN-1-i];
    }

    for (i=0;i<DCT_LEN;i++) {
      dst[DCT_LEN+i] = src[DCT_LEN+i];
    }
  }
}


#ifndef P4_CODE
/*****************************************************************************

    functionname: feedInput  
    description:  replace 32 oldest samples with 32 new time samples in the 
                  polyphase input buffer;
                  reorder time signal input to allow for fast windowing and
                  lapping
                  order: 
                         buffer1[0..16]  = in_buf[31..15]
                         buffer1[17..31] = in_buf[0..14]       

                         buffer2[0..16]  = in_buf[15..31]
                         buffer2[17..31] = in_buf[14.. 0]       

    returns:  void
    input:       
    output:      
    globals:     

*****************************************************************************/
static __inline void 
feedInput( POLY_PHASE_HANDLE hPolyPhase,
           const float *timeSignal)
{
  int i;
  float *buffer  = hPolyPhase->polyBuffer  + hPolyPhase->curNdx;
  float *buffer2 = hPolyPhase->polyBuffer2 + hPolyPhase->curNdx;

  /*
    replace 32 oldest samples with 32 new samples 

    order: 
    buffer[0..16]  = in_buf[31..15]
    buffer[17..31] = in_buf[0..14]       
  */
  for (i=0; i<=(DCT_LEN/2); i++) {
    buffer[i] = timeSignal[2*(DCT_LEN-1-i)];
  }
  for (i=0; i<(DCT_LEN/2-1); i++) {
    buffer[i + 1 + DCT_LEN/2] = timeSignal[2*i];
  }
  /*
    replace 32 oldest samples with 32 new samples 

    order: 
    buffer[0..16]  = in_buf[15..31]
    buffer[17..31] = in_buf[14.. 0]       
  */
  for (i=0; i<=(DCT_LEN/2); i++) {
    buffer2[i] = buffer[DCT_LEN/2-i];
  }
  for (i=0; i<(DCT_LEN/2-1); i++) {
    buffer2[i + 1 + DCT_LEN/2] = buffer[DCT_LEN-1-i];
  }
}

/*****************************************************************************

    functionname: PolyAnalyse
    description:  Polyphase forward transform
    returns:      error code
    input:        576 new time signal values
    output:       32 polyphase bands, containing 18 values each

*****************************************************************************/
static int PolyAnalyse_NoOpt(POLY_PHASE_HANDLE hPolyPhase,
                      const float *restrict timeSigData,
                      float *restrict polyPhaseData)
{
  /* DCT input */
  ALIGN_16_BYTE float dctBuf[DCT_LEN] ;
  float  mdctLineSign;
  int mdctLine = 0;
  int offset ;

  for (offset = 0; offset < FRAME_LEN_LONG; offset += DCT_LEN) {
    int  bufIndex, winIndex, j;

    /* copy 32 new time signal samples into polyphase buffer */
    feedInput(hPolyPhase, timeSigData+(2*offset));

    /* apply polyphase filter window */
    setFLOAT(0.0f, dctBuf, DCT_LEN);

    bufIndex = hPolyPhase->curNdx;

    for (winIndex=0; winIndex<POLY_WINDOW_SIZE; winIndex+=(DCT_LEN*2) ) {
      int i;
      float tmpDct0  = dctBuf[ 0];
      float tmpDct16 = dctBuf[16];
      float *polyBuf, *polyBuf2;
      const float *windowCoef;
      const float *windowCoef2;

      windowCoef  = &hPolyPhase->anaWindow[winIndex];
      polyBuf     = hPolyPhase->polyBuffer2 +   bufIndex;
      windowCoef2 = &hPolyPhase->anaWindow[winIndex+DCT_LEN];
      polyBuf2    = hPolyPhase->polyBuffer  + ((bufIndex + DCT_LEN) & (POLY_WINDOW_SIZE - 1));

      for(i=0; i<DCT_LEN/2;i+=4) {
        dctBuf[i  ] += polyBuf[i  ] * windowCoef[i  ] + polyBuf[i  +DCT_LEN/2] * windowCoef[i  +DCT_LEN/2];
        dctBuf[i+1] += polyBuf[i+1] * windowCoef[i+1] + polyBuf[i+1+DCT_LEN/2] * windowCoef[i+1+DCT_LEN/2];
        dctBuf[i+2] += polyBuf[i+2] * windowCoef[i+2] + polyBuf[i+2+DCT_LEN/2] * windowCoef[i+2+DCT_LEN/2];
        dctBuf[i+3] += polyBuf[i+3] * windowCoef[i+3] + polyBuf[i+3+DCT_LEN/2] * windowCoef[i+3+DCT_LEN/2];

        dctBuf[DCT_LEN/2+(i  )] += polyBuf2[i  ] * windowCoef2[i  ] - polyBuf2[i  +DCT_LEN/2] * windowCoef2[i  +DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+1)] += polyBuf2[i+1] * windowCoef2[i+1] - polyBuf2[i+1+DCT_LEN/2] * windowCoef2[i+1+DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+2)] += polyBuf2[i+2] * windowCoef2[i+2] - polyBuf2[i+2+DCT_LEN/2] * windowCoef2[i+2+DCT_LEN/2];
        dctBuf[DCT_LEN/2+(i+3)] += polyBuf2[i+3] * windowCoef2[i+3] - polyBuf2[i+3+DCT_LEN/2] * windowCoef2[i+3+DCT_LEN/2];
      }
      dctBuf[ 0] = tmpDct0  + polyBuf[ 0] * windowCoef[ 0];
      dctBuf[16] = tmpDct16 + polyBuf[16] * windowCoef[16] + polyBuf2[0] * windowCoef2[0];

      bufIndex = (bufIndex + 2*DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    }

    /*
      DCT-Type III Transform
    */
    fct_iii(DCT_LEN,dctBuf);

    /*
        Insert poly phase data into second half of mdct buffer
        and invert every odd coeff in each odd mdct spectrum
        poly0/0.............poly0/17
        poly1/0
        .
        .
        .
        poly31/0...........poly31/17
    */
    mdctLineSign = (mdctLine&0x1)?(-1.f):(1.f);
    for(j=0;j<DCT_LEN;j+=4) {
      polyPhaseData[MDCT_SIZE+mdctLine+(j  )*(2*MDCT_SIZE)] =                dctBuf[j  ];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+1)*(2*MDCT_SIZE)] = mdctLineSign * dctBuf[j+1];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+2)*(2*MDCT_SIZE)] =                dctBuf[j+2];
      polyPhaseData[MDCT_SIZE+mdctLine+(j+3)*(2*MDCT_SIZE)] = mdctLineSign * dctBuf[j+3];
    }

    /* increment  poly phase buffer index */
    hPolyPhase->curNdx = (hPolyPhase->curNdx - DCT_LEN) & (POLY_WINDOW_SIZE - 1);
    mdctLine++;
  }

  return (TRUE);
}
#endif


#ifdef GP_CODE
int (* PolyAnalyse_GP ) (POLY_PHASE_HANDLE hPolyPhase,
                        const float *restrict timeSigData,
                        float *restrict polyPhaseData) = PolyAnalyse_NoOpt;
#elif defined P4_CODE
extern int PolyAnalyse_Opt(POLY_PHASE_HANDLE hPolyPhase,
						   const float *restrict timeSigData,
						   float *restrict polyPhaseData);

int (* PolyAnalyse_P4 ) (POLY_PHASE_HANDLE hPolyPhase,
                      const float *restrict timeSigData,
                      float *restrict polyPhaseData) = PolyAnalyse_Opt;

#else

int (* PolyAnalyse ) (POLY_PHASE_HANDLE hPolyPhase,
                      const float *restrict timeSigData,
                      float *restrict polyPhaseData) = PolyAnalyse_NoOpt;
#endif


/*****************************************************************************

    functionname: PolyPhaseNew
    description:  Allocate memory for a polyphase handle
    returns:      an error code
    input:        a pointer to a handle

*****************************************************************************/
int PolyPhaseNew(POLY_PHASE_HANDLE *phPolyPhase)
{
  if (phPolyPhase) {
    *phPolyPhase = (POLY_PHASE_HANDLE)mp3Calloc(1,sizeof(struct POLY_PHASE));
    if (*phPolyPhase) {
      (*phPolyPhase)->polyBuffer  = (float*)mp3Alloc(sizeof(float)*(POLY_WINDOW_SIZE));
      (*phPolyPhase)->polyBuffer2 = (float*)mp3Alloc(sizeof(float)*(POLY_WINDOW_SIZE));
      (*phPolyPhase)->anaWindow   = (float*)mp3Alloc(sizeof(float)*(POLY_WINDOW_SIZE));
    }
  }

  return ((phPolyPhase && *phPolyPhase && (*phPolyPhase)->polyBuffer) ? 0 : 1);
}

/*****************************************************************************

    functionname: PolyPhaseInit
    description:  Initialize a polyphase handle
    returns:      an error code
    input:        an allocated polyphase handle

*****************************************************************************/
int PolyPhaseInit(POLY_PHASE_HANDLE hPolyPhase)
{
  setFLOAT(0.0f, hPolyPhase->polyBuffer,  POLY_WINDOW_SIZE);
  setFLOAT(0.0f, hPolyPhase->polyBuffer2, POLY_WINDOW_SIZE);
  resortAnaWindow(hPolyPhase->anaWindow, anaWindow);
  hPolyPhase->curNdx = 0;

  return(0);
}

/*****************************************************************************

    functionname: PolyPhaseDelete
    description:  Release all memory allocated by PolyPhaseNew()

*****************************************************************************/
void PolyPhaseDelete(POLY_PHASE_HANDLE hPolyPhase)
{
  if (hPolyPhase) {
    if (hPolyPhase->polyBuffer) 
      mp3Free(hPolyPhase->polyBuffer);
    if (hPolyPhase->polyBuffer2) 
      mp3Free(hPolyPhase->polyBuffer2);
    if (hPolyPhase->anaWindow) 
      mp3Free(hPolyPhase->anaWindow);
    mp3Free(hPolyPhase);
  }
}

