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
*   $Id: bit_cnt.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include "mconfig.h"
#include "bit_cnt.h"
#include "mathmac.h"
#include "mathlib.h"
#include "mp3alloc.h"


#define MAX_PAIR_TABLE 32
#define MAX_QUAD_TABLE  2   


typedef struct{ 
  void (*encodePairFunction)(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream);
  int linbits;
}ENCODE_PAIR_FUNC;

    

typedef void (*ENCODE_QUAD_FUNC)(int q,int sq,int sqBits,HANDLE_BIT_BUF hBitstream);




#ifndef min

#define min(a,b) (((a) < (b)) ? (a) : (b))

#endif


#define HD(a,b) ((a<<16)+b)
#define HT(a,b,c) ((a<<20)+(b<<10)+c)

ALIGN_16_BYTE static const unsigned long len_1_tab[2][2] =
{
	{1,4},
	{3,5}
};

ALIGN_16_BYTE static const unsigned long len_2_3_tab[3][3] =
{
	{ HD(1,2),HD(4,3),HD(7,7)},
	{ HD(4,4),HD(5,4),HD(7,7)},
	{ HD(6,6),HD(7,7),HD(8,8)}
};

ALIGN_16_BYTE static const unsigned long len_5_6_tab[4][4] =
{
	{HD(1,3),HD(4,4),HD(7,6),HD(8,8)},
	{HD(4,4),HD(5,4),HD(8,6),HD(9,7)},
	{HD(7,5),HD(8,6),HD(9,7),HD(10,8)},
	{HD(8,7),HD(8,7),HD(9,8),HD(10,9)}
};

ALIGN_16_BYTE static const unsigned long len_7_8_9_tab[6][6]=
{
	{HT(1,2,3),HT(4,4,4),HT(7,7,6),HT(9,9,7),HT(9,9,9),HT(10,10,10)},
	{HT(4,4,4),HT(6,4,5),HT(8,6,6),HT(9,10,7),HT(9,10,8),HT(10,10,10)},
	{HT(7,7,5),HT(7,6,6),HT(9,8,7),HT(10,10,8),HT(10,10,9),HT(11,11,10)},
	{HT(8,9,7),HT(9,10,7),HT(10,10,8),HT(11,11,9),HT(11,11,9),HT(11,12,10)},
	{HT(8,9,8),HT(9,9,8),HT(10,10,9),HT(11,11,9),HT(11,12,10),HT(12,12,11)},
	{HT(9,10,9),HT(10,10,9),HT(11,11,10),HT(12,11,10),HT(12,13,11),HT(12,13,11)},
};

ALIGN_16_BYTE static const unsigned long len_10_11_12_tab[8][8]=
{
	{HT(1,2,4),HT(4,4,4),HT(7,6,6),HT(9,8,8),HT(10,9,9),HT(10,10,10),HT(10,9,10),HT(11,10,10)},
	{HT(4,4,4),HT(6,5,5),HT(8,6,6),HT(9,8,7),HT(10,10,9),HT(11,10,9),HT(10,9,10),HT(10,10,10)},
	{HT(7,6,6),HT(8,7,6),HT(9,8,7),HT(10,9,8),HT(11,10,9),HT(12,11,10),HT(11,10,9),HT(11,10,10)},
	{HT(8,8,7),HT(9,8,7),HT(10,9,8),HT(11,11,8),HT(12,10,9),HT(12,12,10),HT(11,10,10),HT(12,11,10)},
	{HT(9,9,8),HT(10,10,8),HT(11,10,9),HT(12,11,9),HT(12,11,10),HT(12,12,10),HT(12,11,10),HT(12,12,11)},
	{HT(10,9,9),HT(11,10,9),HT(12,11,10),HT(12,12,10),HT(13,12,10),HT(13,13,11),HT(12,12,10),HT(13,13,11)},
	{HT(9,9,9),HT(10,9,9),HT(11,9,9),HT(12,10,10),HT(12,11,10),HT(12,12,11),HT(13,12,11),HT(13,12,12)},
	{HT(10,9,10),HT(10,9,10),HT(11,10,10),HT(12,11,11),HT(12,12,11),HT(13,12,11),HT(13,12,11),HT(13,12,12)}
};

ALIGN_16_BYTE static const unsigned long len_13_15_tab[16][16]=
{
	{HD(1,3),HD(5,5),HD(7,6),HD(8,8),HD(9,8),HD(10,9),HD(10,10),HD(11,10),HD(10,10),HD(11,11),HD(12,11),HD(12,12),HD(13,12),HD(13,12),HD(14,13),HD(14,14)},
	{HD(4,5),HD(6,5),HD(8,7),HD(9,8),HD(10,9),HD(10,9),HD(11,10),HD(11,10),HD(11,10),HD(11,11),HD(12,11),HD(12,12),HD(13,12),HD(14,12),HD(14,13),HD(14,13)},
	{HD(7,6),HD(8,7),HD(9,7),HD(10,8),HD(11,9),HD(11,9),HD(12,10),HD(12,10),HD(11,10),HD(12,11),HD(12,11),HD(13,12),HD(13,12),HD(14,13),HD(15,13),HD(15,13)},
	{HD(8,7),HD(9,8),HD(10,8),HD(11,9),HD(11,9),HD(12,10),HD(12,10),HD(12,11),HD(12,11),HD(13,11),HD(13,12),HD(13,12),HD(13,12),HD(14,13),HD(15,13),HD(15,13)},
	{HD(9,8),HD(9,8),HD(11,9),HD(11,9),HD(12,10),HD(12,10),HD(13,11),HD(13,11),HD(12,11),HD(13,11),HD(13,12),HD(14,12),HD(14,12),HD(15,13),HD(15,13),HD(16,13)},
	{HD(10,9),HD(10,9),HD(11,9),HD(12,10),HD(12,10),HD(12,10),HD(13,11),HD(13,11),HD(13,11),HD(13,11),HD(14,12),HD(13,12),HD(15,13),HD(15,13),HD(16,13),HD(16,14)},
	{HD(10,10),HD(11,9),HD(12,10),HD(12,10),HD(13,10),HD(13,11),HD(13,11),HD(13,11),HD(13,11),HD(14,12),HD(14,12),HD(14,12),HD(15,13),HD(15,13),HD(16,14),HD(16,14)},
	{HD(11,10),HD(11,10),HD(12,10),HD(13,11),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(15,13),HD(16,13),HD(18,13),HD(18,14)},
	{HD(10,10),HD(10,10),HD(11,10),HD(12,11),HD(12,11),HD(13,11),HD(13,11),HD(14,12),HD(14,12),HD(14,12),HD(14,12),HD(15,13),HD(15,13),HD(16,14),HD(17,14),HD(17,14)},
	{HD(11,10),HD(11,10),HD(12,11),HD(12,11),HD(13,11),HD(13,11),HD(13,12),HD(15,12),HD(14,12),HD(15,13),HD(15,13),HD(16,13),HD(16,13),HD(16,14),HD(18,14),HD(17,14)},
	{HD(11,11),HD(12,11),HD(12,11),HD(13,11),HD(13,12),HD(14,12),HD(14,12),HD(15,12),HD(14,12),HD(15,13),HD(16,13),HD(15,13),HD(16,13),HD(17,14),HD(18,15),HD(19,14)},
	{HD(12,11),HD(12,11),HD(12,11),HD(13,11),HD(14,12),HD(14,12),HD(14,12),HD(14,12),HD(15,13),HD(15,13),HD(15,13),HD(16,13),HD(17,14),HD(17,14),HD(17,14),HD(18,15)},
	{HD(12,12),HD(13,12),HD(13,11),HD(14,12),HD(14,12),HD(15,12),HD(14,13),HD(15,13),HD(16,13),HD(16,13),HD(17,13),HD(17,13),HD(17,14),HD(18,14),HD(18,15),HD(18,15)},
	{HD(13,12),HD(13,12),HD(14,12),HD(15,12),HD(15,12),HD(15,13),HD(16,13),HD(16,13),HD(16,13),HD(16,14),HD(16,14),HD(17,14),HD(18,14),HD(17,14),HD(18,15),HD(18,15)},
	{HD(14,13),HD(14,13),HD(14,13),HD(15,13),HD(15,13),HD(15,13),HD(17,13),HD(16,13),HD(16,14),HD(19,14),HD(17,14),HD(17,14),HD(17,15),HD(19,15),HD(18,14),HD(18,15)},
	{HD(13,13),HD(14,13),HD(15,13),HD(16,13),HD(16,13),HD(16,13),HD(17,13),HD(16,14),HD(17,14),HD(17,14),HD(18,14),HD(18,14),HD(21,15),HD(20,15),HD(21,15),HD(18,15)}
};

ALIGN_16_BYTE static const unsigned long len_16_24_tab[16][16] =
{
	{HD(1,4),HD(5,5),HD(7,7),HD(9,8),HD(10,9),HD(10,10),HD(11,10),HD(11,11),HD(12,11),HD(12,12),HD(12,12),HD(13,12),HD(13,12),HD(13,12),HD(14,13),HD(10,10)},
	{HD(4,5),HD(6,6),HD(8,7),HD(9,8),HD(10,9),HD(11,10),HD(11,10),HD(11,11),HD(12,11),HD(12,11),HD(12,12),HD(13,12),HD(14,12),HD(13,12),HD(14,12),HD(10,10)},
	{HD(7,7),HD(8,7),HD(9,8),HD(10,9),HD(11,9),HD(11,10),HD(12,10),HD(12,11),HD(13,11),HD(12,11),HD(13,11),HD(13,12),HD(13,12),HD(14,12),HD(14,13),HD(11,9)},
	{HD(9,8),HD(9,8),HD(10,9),HD(11,9),HD(11,10),HD(12,10),HD(12,10),HD(12,11),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(12,9)},
	{HD(10,9),HD(10,9),HD(11,9),HD(11,10),HD(12,10),HD(12,10),HD(13,10),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(15,13),HD(11,9)},
	{HD(10,10),HD(10,9),HD(11,10),HD(11,10),HD(12,10),HD(13,10),HD(13,11),HD(14,11),HD(13,11),HD(14,11),HD(14,12),HD(15,12),HD(15,12),HD(15,12),HD(16,12),HD(12,9)},
	{HD(11,10),HD(11,10),HD(11,10),HD(12,10),HD(13,10),HD(13,11),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(16,13),HD(12,9)},
	{HD(11,11),HD(11,10),HD(12,10),HD(12,10),HD(13,11),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(15,12),HD(15,12),HD(15,12),HD(15,12),HD(17,13),HD(17,13),HD(12,10)},
	{HD(11,11),HD(12,11),HD(12,11),HD(13,11),HD(13,11),HD(13,11),HD(14,11),HD(14,11),HD(15,11),HD(15,12),HD(15,12),HD(15,12),HD(16,12),HD(16,13),HD(16,13),HD(12,10)},
	{HD(12,11),HD(12,11),HD(12,11),HD(13,11),HD(13,11),HD(14,11),HD(14,11),HD(15,12),HD(15,12),HD(15,12),HD(15,12),HD(16,12),HD(15,13),HD(16,13),HD(15,13),HD(13,10)},
	{HD(12,12),HD(13,11),HD(12,11),HD(13,11),HD(14,11),HD(14,12),HD(14,12),HD(14,12),HD(15,12),HD(16,12),HD(16,12),HD(16,13),HD(17,13),HD(17,13),HD(16,13),HD(12,10)},
	{HD(13,12),HD(13,12),HD(13,11),HD(13,11),HD(14,11),HD(14,12),HD(15,12),HD(16,12),HD(16,12),HD(16,12),HD(16,12),HD(16,13),HD(16,13),HD(15,13),HD(16,13),HD(13,10)},
	{HD(13,12),HD(14,12),HD(14,12),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(15,12),HD(15,12),HD(17,12),HD(16,13),HD(16,13),HD(16,13),HD(16,13),HD(18,13),HD(13,10)},
	{HD(15,12),HD(14,12),HD(14,12),HD(14,12),HD(15,12),HD(15,12),HD(16,12),HD(16,12),HD(16,13),HD(18,13),HD(17,13),HD(17,13),HD(17,13),HD(19,13),HD(17,13),HD(13,10)},
	{HD(14,13),HD(15,12),HD(13,12),HD(14,12),HD(16,12),HD(16,12),HD(15,12),HD(16,13),HD(16,13),HD(17,13),HD(18,13),HD(17,13),HD(19,13),HD(17,13),HD(16,13),HD(13,10)},
	{HD(10,9),HD(10,9),HD(10,9),HD(11,9),HD(11,9),HD(12,9),HD(12,9),HD(12,9),HD(13,9),HD(13,9),HD(13,9),HD(13,10),HD(13,10),HD(13,10),HD(13,10),HD(10,6)}
};



void mp3CountFunc_0(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  (void)no_of_pairs; /* silence compiler warnings */
  (void)quant_ptr;
 
  *bitsum = 0;
}


void mp3CountFunc_1(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i;
  unsigned long bit_cnt=0;

  for(i=0;i<no_of_pairs;i++)
    bit_cnt+=len_1_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	
  *bitsum = bit_cnt;
}

void
mp3CountFunc_2_3(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i;
  unsigned long bit_cnt=0;

  for(i=0;i<no_of_pairs;i++)
    bit_cnt+=len_2_3_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	

  bitsum[0] = (bit_cnt>>16);
  bitsum[1] = (bit_cnt & 0xffff);
}

void
mp3CountFunc_5_6(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i;
  unsigned long bit_cnt=0;

  for(i=0;i<no_of_pairs;i++)
    bit_cnt+=len_5_6_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	

  bitsum[0] = (bit_cnt>>16);
  bitsum[1] = (bit_cnt & 0xffff);
}

void
mp3CountFunc_7_8_9(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i,ov_l;
  unsigned long bit_cnt;

  bitsum[0]=0;
  bitsum[1]=0;
  bitsum[2]=0;

  while(no_of_pairs > 0)
  {
    ov_l = min(no_of_pairs,78);	 /* 78*13 < 1024, max counter */
    no_of_pairs-=ov_l;
    bit_cnt=0;

    for(i=0;i<ov_l;i++)
      bit_cnt+=len_7_8_9_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];

    quant_ptr+=ov_l*2;

    bitsum[0]+=(bit_cnt >> 20);
    bitsum[1]+=(bit_cnt >> 10) & 0x3ff;
    bitsum[2]+=(bit_cnt)&0x3ff;
  }
}		 

void
mp3CountFunc_10_11_12(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i,ov_l;
  unsigned long bit_cnt;
  bitsum[0]=0;
  bitsum[1]=0;
  bitsum[2]=0;

  while(no_of_pairs > 0)
  {
    ov_l = min(no_of_pairs,78);	 /* 78*13 < 1024 ==> max counter */
    no_of_pairs-=ov_l;
    bit_cnt=0;

    for(i=0;i<ov_l;i++)
      bit_cnt+=len_10_11_12_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];

    quant_ptr+=ov_l*2;

    bitsum[0]+=(bit_cnt >> 20);
    bitsum[1]+=(bit_cnt >> 10) & 0x3ff;
    bitsum[2]+=(bit_cnt)&0x3ff;
  }
}

void
mp3CountFunc_13_15(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  int i;
  unsigned long bit_cnt=0;

  for(i=0;i<no_of_pairs;i++)
    bit_cnt+=len_13_15_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	

  bitsum[0] = (bit_cnt>>16);
  bitsum[1] = (bit_cnt & 0xffff);
}

static void
mp3CountFunc_Escape_Generic(const unsigned int *quant_ptr,
                         int no_of_pairs,
                         int *bitsum,
                         unsigned long linb1)
{
  int i;
  unsigned long bit_cnt=0;
  unsigned long linb2=linb1<<1;

  for(i=0;i<no_of_pairs;i++)
  {
    if(quant_ptr[i*2] >= 15)
    {
      if(quant_ptr[i*2+1]>=15)
        bit_cnt+=len_16_24_tab[15][15]+linb2;
      else
        bit_cnt+=len_16_24_tab[15][quant_ptr[i*2+1]]+linb1;
    }
    else
    {
      if(quant_ptr[i*2+1]>=15)
        bit_cnt+=len_16_24_tab[quant_ptr[i*2]][15]+linb1;
      else
        bit_cnt+=len_16_24_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];
    }
  }
  bitsum[0] = (bit_cnt>>16);
  bitsum[1] = (bit_cnt & 0xffff);
}

void
mp3CountFunc_16_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00010004L);
}


void mp3CountFunc_17_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00020004L);
}

void mp3CountFunc_18_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00030004L);
}

void mp3CountFunc_19_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00040004L);
}

void mp3CountFunc_20_25(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00060005L);
}

void mp3CountFunc_20_26(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00060006L);
}

void mp3CountFunc_21_27(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00080007L);
}

void mp3CountFunc_21_28(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00080008L);
}

void mp3CountFunc_22_29(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000a0009L);
}

void mp3CountFunc_22_30(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000a000bL);
}

void mp3CountFunc_23_30(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000d000bL);
}

void mp3CountFunc_23_31(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
  mp3CountFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000d000dL);
}


/********************* Encoding *********************/

/*
    Pair tables
*/

ALIGN_16_BYTE static const int huff_cod_tab_01[2][2] = {      /* 1(0) */
    {0x0001, 0x0001},
    {0x0001, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_02[3][3] = {      /* 2(0) */
    {0x0001, 0x0002, 0x0001},
    {0x0003, 0x0001, 0x0001},
    {0x0003, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_03[3][3] = {      /* 3(0) */
    {0x0003, 0x0002, 0x0001},
    {0x0001, 0x0001, 0x0001},
    {0x0003, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_05[4][4] = {      /* 5(0) */
    {0x0001, 0x0002, 0x0006, 0x0005},
    {0x0003, 0x0001, 0x0004, 0x0004},
    {0x0007, 0x0005, 0x0007, 0x0001},
    {0x0006, 0x0001, 0x0001, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_06[4][4] = {      /* 6(0) */
    {0x0007, 0x0003, 0x0005, 0x0001},
    {0x0006, 0x0002, 0x0003, 0x0002},
    {0x0005, 0x0004, 0x0004, 0x0001},
    {0x0003, 0x0003, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_07[6][6] = {      /* 7(0) */
    {0x0001, 0x0002, 0x000a, 0x0013, 0x0010, 0x000a},
    {0x0003, 0x0003, 0x0007, 0x000a, 0x0005, 0x0003},
    {0x000b, 0x0004, 0x000d, 0x0011, 0x0008, 0x0004},
    {0x000c, 0x000b, 0x0012, 0x000f, 0x000b, 0x0002},
    {0x0007, 0x0006, 0x0009, 0x000e, 0x0003, 0x0001},
    {0x0006, 0x0004, 0x0005, 0x0003, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_08[6][6] = {      /* 8(0) */
    {0x0003, 0x0004, 0x0006, 0x0012, 0x000c, 0x0005},
    {0x0005, 0x0001, 0x0002, 0x0010, 0x0009, 0x0003},
    {0x0007, 0x0003, 0x0005, 0x000e, 0x0007, 0x0003},
    {0x0013, 0x0011, 0x000f, 0x000d, 0x000a, 0x0004},
    {0x000d, 0x0005, 0x0008, 0x000b, 0x0005, 0x0001},
    {0x000c, 0x0004, 0x0004, 0x0001, 0x0001, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_09[6][6] = {      /* 9(0) */
    {0x0007, 0x0005, 0x0009, 0x000e, 0x000f, 0x0007},
    {0x0006, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007},
    {0x0007, 0x0006, 0x0008, 0x0008, 0x0008, 0x0005},
    {0x000f, 0x0006, 0x0009, 0x000a, 0x0005, 0x0001},
    {0x000b, 0x0007, 0x0009, 0x0006, 0x0004, 0x0001},
    {0x000e, 0x0004, 0x0006, 0x0002, 0x0006, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_10[8][8] = {      /* 10(0) */
    {0x0001, 0x0002, 0x000a, 0x0017, 0x0023, 0x001e, 0x000c, 0x0011},
    {0x0003, 0x0003, 0x0008, 0x000c, 0x0012, 0x0015, 0x000c, 0x0007},
    {0x000b, 0x0009, 0x000f, 0x0015, 0x0020, 0x0028, 0x0013, 0x0006},
    {0x000e, 0x000d, 0x0016, 0x0022, 0x002e, 0x0017, 0x0012, 0x0007},
    {0x0014, 0x0013, 0x0021, 0x002f, 0x001b, 0x0016, 0x0009, 0x0003},
    {0x001f, 0x0016, 0x0029, 0x001a, 0x0015, 0x0014, 0x0005, 0x0003},
    {0x000e, 0x000d, 0x000a, 0x000b, 0x0010, 0x0006, 0x0005, 0x0001},
    {0x0009, 0x0008, 0x0007, 0x0008, 0x0004, 0x0004, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_11[8][8] = {      /* 11(0) */
    {0x0003, 0x0004, 0x000a, 0x0018, 0x0022, 0x0021, 0x0015, 0x000f},
    {0x0005, 0x0003, 0x0004, 0x000a, 0x0020, 0x0011, 0x000b, 0x000a},
    {0x000b, 0x0007, 0x000d, 0x0012, 0x001e, 0x001f, 0x0014, 0x0005},
    {0x0019, 0x000b, 0x0013, 0x003b, 0x001b, 0x0012, 0x000c, 0x0005},
    {0x0023, 0x0021, 0x001f, 0x003a, 0x001e, 0x0010, 0x0007, 0x0005},
    {0x001c, 0x001a, 0x0020, 0x0013, 0x0011, 0x000f, 0x0008, 0x000e},
    {0x000e, 0x000c, 0x0009, 0x000d, 0x000e, 0x0009, 0x0004, 0x0001},
    {0x000b, 0x0004, 0x0006, 0x0006, 0x0006, 0x0003, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_12[8][8] = {      /* 12(0) */
    {0x0009, 0x0006, 0x0010, 0x0021, 0x0029, 0x0027, 0x0026, 0x001a},
    {0x0007, 0x0005, 0x0006, 0x0009, 0x0017, 0x0010, 0x001a, 0x000b},
    {0x0011, 0x0007, 0x000b, 0x000e, 0x0015, 0x001e, 0x000a, 0x0007},
    {0x0011, 0x000a, 0x000f, 0x000c, 0x0012, 0x001c, 0x000e, 0x0005},
    {0x0020, 0x000d, 0x0016, 0x0013, 0x0012, 0x0010, 0x0009, 0x0005},
    {0x0028, 0x0011, 0x001f, 0x001d, 0x0011, 0x000d, 0x0004, 0x0002},
    {0x001b, 0x000c, 0x000b, 0x000f, 0x000a, 0x0007, 0x0004, 0x0001},
    {0x001b, 0x000c, 0x0008, 0x000c, 0x0006, 0x0003, 0x0001, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_13[16][16] = {      /* 13(0) */
    {0x0001, 0x0005, 0x000e, 0x0015, 0x0022, 0x0033, 0x002e, 0x0047, 0x002a, 0x0034, 0x0044, 0x0034, 0x0043, 0x002c, 0x002b, 0x0013},
    {0x0003, 0x0004, 0x000c, 0x0013, 0x001f, 0x001a, 0x002c, 0x0021, 0x001f, 0x0018, 0x0020, 0x0018, 0x001f, 0x0023, 0x0016, 0x000e},
    {0x000f, 0x000d, 0x0017, 0x0024, 0x003b, 0x0031, 0x004d, 0x0041, 0x001d, 0x0028, 0x001e, 0x0028, 0x001b, 0x0021, 0x002a, 0x0010},
    {0x0016, 0x0014, 0x0025, 0x003d, 0x0038, 0x004f, 0x0049, 0x0040, 0x002b, 0x004c, 0x0038, 0x0025, 0x001a, 0x001f, 0x0019, 0x000e},
    {0x0023, 0x0010, 0x003c, 0x0039, 0x0061, 0x004b, 0x0072, 0x005b, 0x0036, 0x0049, 0x0037, 0x0029, 0x0030, 0x0035, 0x0017, 0x0018},
    {0x003a, 0x001b, 0x0032, 0x0060, 0x004c, 0x0046, 0x005d, 0x0054, 0x004d, 0x003a, 0x004f, 0x001d, 0x004a, 0x0031, 0x0029, 0x0011},
    {0x002f, 0x002d, 0x004e, 0x004a, 0x0073, 0x005e, 0x005a, 0x004f, 0x0045, 0x0053, 0x0047, 0x0032, 0x003b, 0x0026, 0x0024, 0x000f},
    {0x0048, 0x0022, 0x0038, 0x005f, 0x005c, 0x0055, 0x005b, 0x005a, 0x0056, 0x0049, 0x004d, 0x0041, 0x0033, 0x002c, 0x002b, 0x002a},
    {0x002b, 0x0014, 0x001e, 0x002c, 0x0037, 0x004e, 0x0048, 0x0057, 0x004e, 0x003d, 0x002e, 0x0036, 0x0025, 0x001e, 0x0014, 0x0010},
    {0x0035, 0x0019, 0x0029, 0x0025, 0x002c, 0x003b, 0x0036, 0x0051, 0x0042, 0x004c, 0x0039, 0x0036, 0x0025, 0x0012, 0x0027, 0x000b},
    {0x0023, 0x0021, 0x001f, 0x0039, 0x002a, 0x0052, 0x0048, 0x0050, 0x002f, 0x003a, 0x0037, 0x0015, 0x0016, 0x001a, 0x0026, 0x0016},
    {0x0035, 0x0019, 0x0017, 0x0026, 0x0046, 0x003c, 0x0033, 0x0024, 0x0037, 0x001a, 0x0022, 0x0017, 0x001b, 0x000e, 0x0009, 0x0007},
    {0x0022, 0x0020, 0x001c, 0x0027, 0x0031, 0x004b, 0x001e, 0x0034, 0x0030, 0x0028, 0x0034, 0x001c, 0x0012, 0x0011, 0x0009, 0x0005},
    {0x002d, 0x0015, 0x0022, 0x0040, 0x0038, 0x0032, 0x0031, 0x002d, 0x001f, 0x0013, 0x000c, 0x000f, 0x000a, 0x0007, 0x0006, 0x0003},
    {0x0030, 0x0017, 0x0014, 0x0027, 0x0024, 0x0023, 0x0035, 0x0015, 0x0010, 0x0017, 0x000d, 0x000a, 0x0006, 0x0001, 0x0004, 0x0002},
    {0x0010, 0x000f, 0x0011, 0x001b, 0x0019, 0x0014, 0x001d, 0x000b, 0x0011, 0x000c, 0x0010, 0x0008, 0x0001, 0x0001, 0x0000, 0x0001}
};

ALIGN_16_BYTE static const int huff_cod_tab_15[16][16] = {      /* 15(0) */
    {0x0007, 0x000c, 0x0012, 0x0035, 0x002f, 0x004c, 0x007c, 0x006c, 0x0059, 0x007b, 0x006c, 0x0077, 0x006b, 0x0051, 0x007a, 0x003f},
    {0x000d, 0x0005, 0x0010, 0x001b, 0x002e, 0x0024, 0x003d, 0x0033, 0x002a, 0x0046, 0x0034, 0x0053, 0x0041, 0x0029, 0x003b, 0x0024},
    {0x0013, 0x0011, 0x000f, 0x0018, 0x0029, 0x0022, 0x003b, 0x0030, 0x0028, 0x0040, 0x0032, 0x004e, 0x003e, 0x0050, 0x0038, 0x0021},
    {0x001d, 0x001c, 0x0019, 0x002b, 0x0027, 0x003f, 0x0037, 0x005d, 0x004c, 0x003b, 0x005d, 0x0048, 0x0036, 0x004b, 0x0032, 0x001d},
    {0x0034, 0x0016, 0x002a, 0x0028, 0x0043, 0x0039, 0x005f, 0x004f, 0x0048, 0x0039, 0x0059, 0x0045, 0x0031, 0x0042, 0x002e, 0x001b},
    {0x004d, 0x0025, 0x0023, 0x0042, 0x003a, 0x0034, 0x005b, 0x004a, 0x003e, 0x0030, 0x004f, 0x003f, 0x005a, 0x003e, 0x0028, 0x0026},
    {0x007d, 0x0020, 0x003c, 0x0038, 0x0032, 0x005c, 0x004e, 0x0041, 0x0037, 0x0057, 0x0047, 0x0033, 0x0049, 0x0033, 0x0046, 0x001e},
    {0x006d, 0x0035, 0x0031, 0x005e, 0x0058, 0x004b, 0x0042, 0x007a, 0x005b, 0x0049, 0x0038, 0x002a, 0x0040, 0x002c, 0x0015, 0x0019},
    {0x005a, 0x002b, 0x0029, 0x004d, 0x0049, 0x003f, 0x0038, 0x005c, 0x004d, 0x0042, 0x002f, 0x0043, 0x0030, 0x0035, 0x0024, 0x0014},
    {0x0047, 0x0022, 0x0043, 0x003c, 0x003a, 0x0031, 0x0058, 0x004c, 0x0043, 0x006a, 0x0047, 0x0036, 0x0026, 0x0027, 0x0017, 0x000f},
    {0x006d, 0x0035, 0x0033, 0x002f, 0x005a, 0x0052, 0x003a, 0x0039, 0x0030, 0x0048, 0x0039, 0x0029, 0x0017, 0x001b, 0x003e, 0x0009},
    {0x0056, 0x002a, 0x0028, 0x0025, 0x0046, 0x0040, 0x0034, 0x002b, 0x0046, 0x0037, 0x002a, 0x0019, 0x001d, 0x0012, 0x000b, 0x000b},
    {0x0076, 0x0044, 0x001e, 0x0037, 0x0032, 0x002e, 0x004a, 0x0041, 0x0031, 0x0027, 0x0018, 0x0010, 0x0016, 0x000d, 0x000e, 0x0007},
    {0x005b, 0x002c, 0x0027, 0x0026, 0x0022, 0x003f, 0x0034, 0x002d, 0x001f, 0x0034, 0x001c, 0x0013, 0x000e, 0x0008, 0x0009, 0x0003},
    {0x007b, 0x003c, 0x003a, 0x0035, 0x002f, 0x002b, 0x0020, 0x0016, 0x0025, 0x0018, 0x0011, 0x000c, 0x000f, 0x000a, 0x0002, 0x0001},
    {0x0047, 0x0025, 0x0022, 0x001e, 0x001c, 0x0014, 0x0011, 0x001a, 0x0015, 0x0010, 0x000a, 0x0006, 0x0008, 0x0006, 0x0002, 0x0000}
};

ALIGN_16_BYTE static const int huff_cod_tab_16[16][16] = {      /* 16(1) 17(2) 18(3) 19(4) 20(6) 21(8) 22(10) 23(13) */
    {0x0001, 0x0005, 0x000e, 0x002c, 0x004a, 0x003f, 0x006e, 0x005d, 0x00ac, 0x0095, 0x008a, 0x00f2, 0x00e1, 0x00c3, 0x0178, 0x0011},
    {0x0003, 0x0004, 0x000c, 0x0014, 0x0023, 0x003e, 0x0035, 0x002f, 0x0053, 0x004b, 0x0044, 0x0077, 0x00c9, 0x006b, 0x00cf, 0x0009},
    {0x000f, 0x000d, 0x0017, 0x0026, 0x0043, 0x003a, 0x0067, 0x005a, 0x00a1, 0x0048, 0x007f, 0x0075, 0x006e, 0x00d1, 0x00ce, 0x0010},
    {0x002d, 0x0015, 0x0027, 0x0045, 0x0040, 0x0072, 0x0063, 0x0057, 0x009e, 0x008c, 0x00fc, 0x00d4, 0x00c7, 0x0183, 0x016d, 0x001a},
    {0x004b, 0x0024, 0x0044, 0x0041, 0x0073, 0x0065, 0x00b3, 0x00a4, 0x009b, 0x0108, 0x00f6, 0x00e2, 0x018b, 0x017e, 0x016a, 0x0009},
    {0x0042, 0x001e, 0x003b, 0x0038, 0x0066, 0x00b9, 0x00ad, 0x0109, 0x008e, 0x00fd, 0x00e8, 0x0190, 0x0184, 0x017a, 0x01bd, 0x0010},
    {0x006f, 0x0036, 0x0034, 0x0064, 0x00b8, 0x00b2, 0x00a0, 0x0085, 0x0101, 0x00f4, 0x00e4, 0x00d9, 0x0181, 0x016e, 0x02cb, 0x000a},
    {0x0062, 0x0030, 0x005b, 0x0058, 0x00a5, 0x009d, 0x0094, 0x0105, 0x00f8, 0x0197, 0x018d, 0x0174, 0x017c, 0x0379, 0x0374, 0x0008},
    {0x0055, 0x0054, 0x0051, 0x009f, 0x009c, 0x008f, 0x0104, 0x00f9, 0x01ab, 0x0191, 0x0188, 0x017f, 0x02d7, 0x02c9, 0x02c4, 0x0007},
    {0x009a, 0x004c, 0x0049, 0x008d, 0x0083, 0x0100, 0x00f5, 0x01aa, 0x0196, 0x018a, 0x0180, 0x02df, 0x0167, 0x02c6, 0x0160, 0x000b},
    {0x008b, 0x0081, 0x0043, 0x007d, 0x00f7, 0x00e9, 0x00e5, 0x00db, 0x0189, 0x02e7, 0x02e1, 0x02d0, 0x0375, 0x0372, 0x01b7, 0x0004},
    {0x00f3, 0x0078, 0x0076, 0x0073, 0x00e3, 0x00df, 0x018c, 0x02ea, 0x02e6, 0x02e0, 0x02d1, 0x02c8, 0x02c2, 0x00df, 0x01b4, 0x0006},
    {0x00ca, 0x00e0, 0x00de, 0x00da, 0x00d8, 0x0185, 0x0182, 0x017d, 0x016c, 0x0378, 0x01bb, 0x02c3, 0x01b8, 0x01b5, 0x06c0, 0x0004},
    {0x02eb, 0x00d3, 0x00d2, 0x00d0, 0x0172, 0x017b, 0x02de, 0x02d3, 0x02ca, 0x06c7, 0x0373, 0x036d, 0x036c, 0x0d83, 0x0361, 0x0002},
    {0x0179, 0x0171, 0x0066, 0x00bb, 0x02d6, 0x02d2, 0x0166, 0x02c7, 0x02c5, 0x0362, 0x06c6, 0x0367, 0x0d82, 0x0366, 0x01b2, 0x0000},
    {0x000c, 0x000a, 0x0007, 0x000b, 0x000a, 0x0011, 0x000b, 0x0009, 0x000d, 0x000c, 0x000a, 0x0007, 0x0005, 0x0003, 0x0001, 0x0003}
};

ALIGN_16_BYTE static const int huff_cod_tab_24[18][16] = {      /* 24(4) 25(5) 26(6) 27(7) 28(8) 29(9) 30(11) 31(13) */
    {0x000f, 0x000d, 0x002e, 0x0050, 0x0092, 0x0106, 0x00f8, 0x01b2, 0x01aa, 0x029d, 0x028d, 0x0289, 0x026d, 0x0205, 0x0408, 0x0058},
    {0x000e, 0x000c, 0x0015, 0x0026, 0x0047, 0x0082, 0x007a, 0x00d8, 0x00d1, 0x00c6, 0x0147, 0x0159, 0x013f, 0x0129, 0x0117, 0x002a},
    {0x002f, 0x0016, 0x0029, 0x004a, 0x0044, 0x0080, 0x0078, 0x00dd, 0x00cf, 0x00c2, 0x00b6, 0x0154, 0x013b, 0x0127, 0x021d, 0x0012},
    {0x0051, 0x0027, 0x004b, 0x0046, 0x0086, 0x007d, 0x0074, 0x00dc, 0x00cc, 0x00be, 0x00b2, 0x0145, 0x0137, 0x0125, 0x010f, 0x0010},
    {0x0093, 0x0048, 0x0045, 0x0087, 0x007f, 0x0076, 0x0070, 0x00d2, 0x00c8, 0x00bc, 0x0160, 0x0143, 0x0132, 0x011d, 0x021c, 0x000e},
    {0x0107, 0x0042, 0x0081, 0x007e, 0x0077, 0x0072, 0x00d6, 0x00ca, 0x00c0, 0x00b4, 0x0155, 0x013d, 0x012d, 0x0119, 0x0106, 0x000c},
    {0x00f9, 0x007b, 0x0079, 0x0075, 0x0071, 0x00d7, 0x00ce, 0x00c3, 0x00b9, 0x015b, 0x014a, 0x0134, 0x0123, 0x0110, 0x0208, 0x000a},
    {0x01b3, 0x0073, 0x006f, 0x006d, 0x00d3, 0x00cb, 0x00c4, 0x00bb, 0x0161, 0x014c, 0x0139, 0x012a, 0x011b, 0x0213, 0x017d, 0x0011},
    {0x01ab, 0x00d4, 0x00d0, 0x00cd, 0x00c9, 0x00c1, 0x00ba, 0x00b1, 0x00a9, 0x0140, 0x012f, 0x011e, 0x010c, 0x0202, 0x0179, 0x0010},
    {0x014f, 0x00c7, 0x00c5, 0x00bf, 0x00bd, 0x00b5, 0x00ae, 0x014d, 0x0141, 0x0131, 0x0121, 0x0113, 0x0209, 0x017b, 0x0173, 0x000b},
    {0x029c, 0x00b8, 0x00b7, 0x00b3, 0x00af, 0x0158, 0x014b, 0x013a, 0x0130, 0x0122, 0x0115, 0x0212, 0x017f, 0x0175, 0x016e, 0x000a},
    {0x028c, 0x015a, 0x00ab, 0x00a8, 0x00a4, 0x013e, 0x0135, 0x012b, 0x011f, 0x0114, 0x0107, 0x0201, 0x0177, 0x0170, 0x016a, 0x0006},
    {0x0288, 0x0142, 0x013c, 0x0138, 0x0133, 0x012e, 0x0124, 0x011c, 0x010d, 0x0105, 0x0200, 0x0178, 0x0172, 0x016c, 0x0167, 0x0004},
    {0x026c, 0x012c, 0x0128, 0x0126, 0x0120, 0x011a, 0x0111, 0x010a, 0x0203, 0x017c, 0x0176, 0x0171, 0x016d, 0x0169, 0x0165, 0x0002},
    {0x0409, 0x0118, 0x0116, 0x0112, 0x010b, 0x0108, 0x0103, 0x017e, 0x017a, 0x0174, 0x016f, 0x016b, 0x0168, 0x0166, 0x0164, 0x0000},
    {0x002b, 0x0014, 0x0013, 0x0011, 0x000f, 0x000d, 0x000b, 0x0009, 0x0007, 0x0006, 0x0004, 0x0007, 0x0005, 0x0003, 0x0001, 0x0003},
    {0x0018, 0x0000, 0x00a5, 0x00b7, 0x00f5, 0x0091, 0x00b3, 0x0029, 0x006f, 0x0068, 0x0098, 0x00aa, 0x00c5, 0x0057, 0x00e7, 0x00fe},
    {0x008a, 0x0044, 0x00da, 0x0004, 0x0012, 0x00a7, 0x00e9, 0x00b9, 0x00d9, 0x006d, 0x0087, 0x00ad, 0x00d1, 0x00b7, 0x00c7, 0x0045}
};



ALIGN_16_BYTE static const int huff_len_tab_01[2][2] = {     /* 1(0) */
       {1,    3},
       {2,    3}
};

ALIGN_16_BYTE static const int huff_len_tab_02[3][3] = {     /* 2(0) */
       {1,    3,    6},
       {3,    3,    5},
       {5,    5,    6} 
};

ALIGN_16_BYTE static const int huff_len_tab_03[3][3] = {     /* 3(0) */		
       {2,    2,    6}, 
       {3,    2,    5}, 
       {5,    5,    6}
};

ALIGN_16_BYTE static const int huff_len_tab_05[4][4] = {     /* 5(0) */
       {1,    3,    6,    7}, 
       {3,    3,    6,    7}, 
       {6,    6,    7,    8}, 
       {7,    6,    7,    8} 
};

ALIGN_16_BYTE static const int huff_len_tab_06[4][4] = {     /* 6(0) */
       {3,    3,    5,    7}, 
       {3,    2,    4,    5}, 
       {4,    4,    5,    6},
       {6,    5,    6,    7} 
};



ALIGN_16_BYTE static const int huff_len_tab_07[6][6] = {     /* 7(0) */

   {1,   3,   6,   8,   8,   9},
   {3,   4,   6,   7,   7,   8},
   {6,   5,   7,   8,   8,   9},
   {7,   7,   8,   9,   9,   9},
   {7,   7,   8,   9,   9,  10},
   {8,   8,   9,  10,  10,  10}
};
    
    
ALIGN_16_BYTE static const int huff_len_tab_08[6][6] = {     /* 8(0) */
   {2,   3,   6,   8,   8,   9},
   {3,   2,   4,   8,   8,   8},
   {6,   4,   6,   8,   8,   9},
   {8,   8,   8,   9,   9,  10},
   {8,   7,   8,   9,  10,  10},
   {9,   8,   9,   9,  11,  11}
};
    

ALIGN_16_BYTE static const int huff_len_tab_09[6][6] = {     /* 9(0) */
   {3,   3,   5,   6,   8,   9},
   {3,   3,   4,   5,   6,   8},
   {4,   4,   5,   6,   7,   8},
   {6,   5,   6,   7,   7,   8},
   {7,   6,   7,   7,   8,   9},
   {8,   7,   8,   8,   9,   9}
};
    

ALIGN_16_BYTE static const int huff_len_tab_10[8][8] = {     /* 10(0) */
   {1,   3,   6,   8,   9,   9,   9,  10},
   {3,   4,   6,   7,   8,   9,   8,   8},
   {6,   6,   7,   8,   9,  10,   9,   9},
   {7,   7,   8,   9,  10,  10,   9,  10},
   {8,   8,   9,  10,  10,  10,  10,  10},
   {9,   9,  10,  10,  11,  11,  10,  11},
   {8,   8,   9,  10,  10,  10,  11,  11},
   {9,   8,   9,  10,  10,  11,  11,  11}
};
    

ALIGN_16_BYTE static const int huff_len_tab_11[8][8] = {     /* 11(0) */
   {2,   3,   5,   7,   8,   9,   8,   9},
   {3,   3,   4,   6,   8,   8,   7,   8},
   {5,   5,   6,   7,   8,   9,   8,   8},
   {7,   6,   7,   9,   8,  10,   8,   9},
   {8,   8,   8,   9,   9,  10,   9,  10},
   {8,   8,   9,  10,  10,  11,  10,  11},
   {8,   7,   7,   8,   9,  10,  10,  10},
   {8,   7,   8,   9,  10,  10,  10,  10}
};

ALIGN_16_BYTE static const int huff_len_tab_12[8][8] = {     /* 12(0) */
   {4,   3,   5,   7,   8,   9,   9,   9},
   {3,   3,   4,   5,   7,   7,   8,   8},
   {5,   4,   5,   6,   7,   8,   7,   8},
   {6,   5,   6,   6,   7,   8,   8,   8},
   {7,   6,   7,   7,   8,   8,   8,   9},
   {8,   7,   8,   8,   8,   9,   8,   9},
   {8,   7,   7,   8,   8,   9,   9,  10},
   {9,   8,   8,   9,   9,   9,   9,  10}
};

ALIGN_16_BYTE static const int huff_len_tab_13[16][16] = {     /* 13(0) */
  {1,  4,  6,  7,  8,  9,  9, 10,  9, 10, 11, 11, 12, 12, 13, 13},
  {3,  4,  6,  7,  8,  8,  9,  9,  9,  9, 10, 10, 11, 12, 12, 12},
  {6,  6,  7,  8,  9,  9, 10, 10,  9, 10, 10, 11, 11, 12, 13, 13},
  {7,  7,  8,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 13, 13},
  {8,  7,  9,  9, 10, 10, 11, 11, 10, 11, 11, 12, 12, 13, 13, 14},
  {9,  8,  9, 10, 10, 10, 11, 11, 11, 11, 12, 11, 13, 13, 14, 14},
  {9,  9, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 13, 13, 14, 14},
 {10,  9, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 16, 16},
  {9,  8,  9, 10, 10, 11, 11, 12, 12, 12, 12, 13, 13, 14, 15, 15},
 {10,  9, 10, 10, 11, 11, 11, 13, 12, 13, 13, 14, 14, 14, 16, 15},
 {10, 10, 10, 11, 11, 12, 12, 13, 12, 13, 14, 13, 14, 15, 16, 17},
 {11, 10, 10, 11, 12, 12, 12, 12, 13, 13, 13, 14, 15, 15, 15, 16},
 {11, 11, 11, 12, 12, 13, 12, 13, 14, 14, 15, 15, 15, 16, 16, 16},
 {12, 11, 12, 13, 13, 13, 14, 14, 14, 14, 14, 15, 16, 15, 16, 16},
 {13, 12, 12, 13, 13, 13, 15, 14, 14, 17, 15, 15, 15, 17, 16, 16},
 {12, 12, 13, 14, 14, 14, 15, 14, 15, 15, 16, 16, 19, 18, 19, 16}
};


ALIGN_16_BYTE static const int huff_len_tab_15[16][16] = {     /* 15(0) */
  {3,  4,  5,  7,  7,  8,  9,  9,  9, 10, 10, 11, 11, 11, 12, 13},
  {4,  3,  5,  6,  7,  7,  8,  8,  8,  9,  9, 10, 10, 10, 11, 11},
  {5,  5,  5,  6,  7,  7,  8,  8,  8,  9,  9, 10, 10, 11, 11, 11},
  {6,  6,  6,  7,  7,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11},
  {7,  6,  7,  7,  8,  8,  9,  9,  9,  9, 10, 10, 10, 11, 11, 11},
  {8,  7,  7,  8,  8,  8,  9,  9,  9,  9, 10, 10, 11, 11, 11, 12},
  {9,  7,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 11, 11, 12, 12},
  {9,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10, 11, 11, 11, 12},
  {9,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 12, 12, 12},
  {9,  8,  9,  9,  9,  9, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12},
 {10,  9,  9,  9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 13, 12},
 {10,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13},
 {11, 10,  9, 10, 10, 10, 11, 11, 11, 11, 11, 11, 12, 12, 13, 13},
 {11, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13},
 {12, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 12, 13},
 {12, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13}
};
    

ALIGN_16_BYTE static const int huff_len_tab_16[16][16] = {     /* 16(1) 17(2) 18(3) 19(4) 20(6) 21(8) 22(10) 23(13) */
  {1,  4,  6,  8,  9,  9, 10, 10, 11, 11, 11, 12, 12, 12, 13,  9},
  {3,  4,  6,  7,  8,  9,  9,  9, 10, 10, 10, 11, 12, 11, 12,  8},
  {6,  6,  7,  8,  9,  9, 10, 10, 11, 10, 11, 11, 11, 12, 12,  9},
  {8,  7,  8,  9,  9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 10},
  {9,  8,  9,  9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13,  9},
  {9,  8,  9,  9, 10, 11, 11, 12, 11, 12, 12, 13, 13, 13, 14, 10},
 {10,  9,  9, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 14, 10},
 {10,  9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 13, 15, 15, 10},
 {10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 13, 14, 14, 14, 10},
 {11, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 14, 13, 14, 13, 11},
 {11, 11, 10, 11, 12, 12, 12, 12, 13, 14, 14, 14, 15, 15, 14, 10},
 {12, 11, 11, 11, 12, 12, 13, 14, 14, 14, 14, 14, 14, 13, 14, 11},
 {12, 12, 12, 12, 12, 13, 13, 13, 13, 15, 14, 14, 14, 14, 16, 11},
 {14, 12, 12, 12, 13, 13, 14, 14, 14, 16, 15, 15, 15, 17, 15, 11},
 {13, 13, 11, 12, 14, 14, 13, 14, 14, 15, 16, 15, 17, 15, 14, 11},
  {9,  8,  8,  9,  9, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11,  8}
};
    
 

ALIGN_16_BYTE static const int huff_len_tab_24[16][16] = {     /* 24(4) 25(5) 26(6) 27(7) 28(8) 29(9) 30(11) 31(13) */
  {4,  4,  6,  7,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11, 12,  9},
  {4,  4,  5,  6,  7,  8,  8,  9,  9,  9, 10, 10, 10, 10, 10,  8},
  {6,  5,  6,  7,  7,  8,  8,  9,  9,  9,  9, 10, 10, 10, 11,  7},
  {7,  6,  7,  7,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10,  7},
  {8,  7,  7,  8,  8,  8,  8,  9,  9,  9, 10, 10, 10, 10, 11,  7},
  {9,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10,  7},
  {9,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10, 11,  7},
 {10,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10, 11, 11,  8},
 {10,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11,  8},
 {10,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 11, 11, 11,  8},
 {11,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11,  8},
 {11, 10,  9,  9,  9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11,  8},
 {11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11,  8},
 {11, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11,  8},
 {12, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,  8},
  {8,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  4}
};



/*
    Quadruples tables
*/

ALIGN_16_BYTE static const int huff_len_tab_32[] = {
     1,  4,  4,  5,  4,  6,  5,  6,  4,  5,  5,  6,  5,  6,  6, 6
};

ALIGN_16_BYTE static const int huff_len_tab_33[] = {
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
};

ALIGN_16_BYTE static const int huff_cod_tab_32[] = {
    0x0001, 0x0005, 0x0004, 0x0005, 0x0006, 0x0005, 0x0004, 0x0004, 0x0007, 0x0003, 0x0006, 0x0000, 0x0007, 0x0002, 0x0003, 0x0001
};

ALIGN_16_BYTE static const int huff_cod_tab_33[] = {
    0x000f, 0x000e, 0x000d, 0x000c, 0x000b, 0x000a, 0x0009, 0x0008, 0x0007, 0x0006, 0x0005, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000
};



/*
    encode functions
*/



static void encodeFunction0(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
  (void) v1; (void) s1; (void) v2; (void) s2; (void) lb; (void) hBitstream;
}

static void encodeFunction1(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_01[v1][v2],huff_len_tab_01[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction2(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_02[v1][v2],huff_len_tab_02[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction3(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_03[v1][v2],huff_len_tab_03[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction5(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_05[v1][v2],huff_len_tab_05[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction6(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_06[v1][v2],huff_len_tab_06[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction7(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_07[v1][v2],huff_len_tab_07[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction8(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_08[v1][v2],huff_len_tab_08[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction9(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_09[v1][v2],huff_len_tab_09[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction10(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_10[v1][v2],huff_len_tab_10[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction11(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_11[v1][v2],huff_len_tab_11[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction12(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_12[v1][v2],huff_len_tab_12[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);

}

static void encodeFunction13(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_13[v1][v2],huff_len_tab_13[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction15(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    (void) lb;

    mp3WriteBits(hBitstream,huff_cod_tab_15[v1][v2],huff_len_tab_15[v1][v2]);
    if(v1>0)
        mp3WriteBits(hBitstream,s1,1);
    if(v2>0)
        mp3WriteBits(hBitstream,s2,1);

}

static void encodeFunction16(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    int v11,v21;
    
    v11= min(v1,15);
    v21= min(v2,15);

    mp3WriteBits(hBitstream,huff_cod_tab_16[v11][v21],huff_len_tab_16[v11][v21]);
    
    if(v11 == 15)
        mp3WriteBits(hBitstream,v1-v11,lb);
    if(v11>0)
        mp3WriteBits(hBitstream,s1,1);

    if(v21 == 15)
        mp3WriteBits(hBitstream,v2-v21,lb);
    if(v21>0)
        mp3WriteBits(hBitstream,s2,1);

}

static void encodeFunction24(int v1,int s1,int v2,int s2,int lb,HANDLE_BIT_BUF hBitstream)
{
    int v11,v21;
    
    v11= min(v1,15);
    v21= min(v2,15);

    mp3WriteBits(hBitstream,huff_cod_tab_24[v11][v21],huff_len_tab_24[v11][v21]);
    
    if(v11 == 15)
        mp3WriteBits(hBitstream,v1-v11,lb);
    if(v11>0)
        mp3WriteBits(hBitstream,s1,1);

    if(v21 == 15)
        mp3WriteBits(hBitstream,v2-v21,lb);
    if(v21>0)
        mp3WriteBits(hBitstream,s2,1);
}

static void encodeFunction32(int q,int sq,int sqBits,HANDLE_BIT_BUF hBitstream)
{
    mp3WriteBits(hBitstream,huff_cod_tab_32[q],huff_len_tab_32[q]);
    mp3WriteBits(hBitstream,sq,sqBits);

}

static void encodeFunction33(int q,int sq,int sqBits,HANDLE_BIT_BUF hBitstream)
{
    mp3WriteBits(hBitstream,huff_cod_tab_33[q],huff_len_tab_33[q]);
    mp3WriteBits(hBitstream,sq,sqBits);
}

/*
    select tables
*/


ALIGN_16_BYTE static ENCODE_PAIR_FUNC huffPairSelect[MAX_PAIR_TABLE]=
{
    {encodeFunction0,0},    /* 0    */
    {encodeFunction1,0},    /*   1  */
    {encodeFunction2,0},    /*   2  */
    {encodeFunction3,0},    /*   3  */
    {NULL,0},               /*   4  */ 
    {encodeFunction5,0},    /*   5  */ 
    {encodeFunction6,0},    /*   6  */
    {encodeFunction7,0},    /*   7  */  
    {encodeFunction8,0},    /*   8  */
    {encodeFunction9,0},    /*   9  */
    {encodeFunction10,0},   /*  10  */
    {encodeFunction11,0},   /*  11  */
    {encodeFunction12,0},   /*  12  */
    {encodeFunction13,0},   /*  13  */
    {NULL,0},               /*  14  */
    {encodeFunction15,0},   /*  15  */
    {encodeFunction16,1},   /*  16  */
    {encodeFunction16,2},   /*  17  */
    {encodeFunction16,3},   /*  18  */
    {encodeFunction16,4},   /*  19  */
    {encodeFunction16,6},   /*  20  */
    {encodeFunction16,8},   /*  21  */
    {encodeFunction16,10},  /*  22  */
    {encodeFunction16,13},  /*  23  */
    {encodeFunction24,4},   /*  24  */
    {encodeFunction24,5},   /*  25  */
    {encodeFunction24,6},   /*  26  */
    {encodeFunction24,7},   /*  27  */
    {encodeFunction24,8},   /*  28  */
    {encodeFunction24,9},   /*  29  */
    {encodeFunction24,11},  /*  30  */
    {encodeFunction24,13}   /*  31  */
};

ALIGN_16_BYTE static ENCODE_QUAD_FUNC huffQuadSelect[MAX_QUAD_TABLE] ={
    encodeFunction32,
    encodeFunction33
};


/*****************************************************************************

    functionname:EncodeHuffPair  
    description:   
    returns:
    input:         
    output:        
    globals:       


*****************************************************************************/
void mp3EncodeHuffPair(int           *specValuePair,
                       int            huffTableSelect,
                       HANDLE_BIT_BUF hBitstream)
{
    int  v1,v2;
    int s1,s2;

    v1 = specValuePair[0];
    s1 = (v1 < 0);
    v1 = abs(v1);

    v2 = specValuePair[1];
    s2 = (v2 < 0);
    v2 = abs(v2);
    
    assert((huffTableSelect >= 0) && (huffTableSelect< MAX_PAIR_TABLE));
    assert(huffPairSelect[huffTableSelect].encodePairFunction != NULL);
    huffPairSelect[huffTableSelect].encodePairFunction(v1,s1,v2,s2,huffPairSelect[huffTableSelect].linbits,hBitstream);        
}

/*****************************************************************************

    functionname:EncodeHuffQuadruple  
    description:   
    returns:
    input:         
    output:        
    globals:       


*****************************************************************************/
void mp3EncodeHuffQuadruple(int           *specValueQuadruple,
                            int            huffTableSelect,
                            HANDLE_BIT_BUF hBitstream)
{

    int i,v,s,q,sq,sqBits;

    sqBits=0;
    sq=0;
    q=0;

    for(i=0;i<4;i++){
        v = specValueQuadruple[i];
        s = (v < 0);
        v = abs(v);
        assert(v <=1);
        q<<=1;
        q|=v;
        if(v){
            sq<<=1;
            sq|=s;
            sqBits++;
        }
    }
    assert((huffTableSelect >= 0) && (huffTableSelect< MAX_QUAD_TABLE));
    
    huffQuadSelect[huffTableSelect](q,sq,sqBits,hBitstream);
}





