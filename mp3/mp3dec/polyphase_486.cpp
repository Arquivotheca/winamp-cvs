#include "polyphase.h"

/* ------------------------------------------------------------------------*/

static const FHG_FLOAT cost32_c1[] =
  {
  0.50241928618816,  0.52249861493969,  0.56694403481636,  0.64682178335999,
  0.78815462345125,  1.06067768599035,  1.72244709823833,  5.10114861868916
  };

/* ------------------------------------------------------------------------*/

static const FHG_FLOAT cost32_c2[] =
  {
  0.50979557910416,  0.60134488693505,  0.89997622313642,  2.56291544774151
  };

/* ------------------------------------------------------------------------*/

static const FHG_FLOAT cost32_c3[] =
  {
  0.54119610014620,  1.30656296487638
  };

/* ------------------------------------------------------------------------*/

static const FHG_FLOAT cost32_c4[] =
  {
  0.70710678118655
  };


void cost16_486(const FHG_FLOAT *vec,FHG_FLOAT *f_vec)
{
  FHG_FLOAT tmp1_0,tmp1_1,tmp1_2,tmp1_3,tmp1_4,tmp1_5,tmp1_6,tmp1_7;
  FHG_FLOAT res1_0,res1_1,res1_2,res1_3,res1_4,res1_5,res1_6,res1_7;

  FHG_FLOAT tmp2_0,tmp2_1,tmp2_2,tmp2_3;
  FHG_FLOAT res2_0,res2_1,res2_2,res2_3;

  FHG_FLOAT tmp3_0,tmp3_1;
  FHG_FLOAT res3_0,res3_1;

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

void cost8_486(const FHG_FLOAT *vec,FHG_FLOAT *f_vec)
{
  FHG_FLOAT res1_1,res1_3,res1_5,res1_7;

  FHG_FLOAT tmp2_0,tmp2_1,tmp2_2,tmp2_3;
  FHG_FLOAT res2_0,res2_1,res2_2,res2_3;

  FHG_FLOAT tmp3_0,tmp3_1;
  FHG_FLOAT res3_0,res3_1;
  
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

