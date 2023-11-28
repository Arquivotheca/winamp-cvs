/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1997-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: huff_cnt.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: huffman bigvalue count functions                                     *
*                                                                                              *
************************************************************************************************/

#include "huff_cnt.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define HD(a,b) ((a<<16)+b)
#define HT(a,b,c) ((a<<20)+(b<<10)+c)

static const unsigned long len_1_tab[2][2] =
{
	{1,4},
	{3,5}
};

static const unsigned long len_2_3_tab[3][3] =
{
	{ HD(1,2),HD(4,3),HD(7,7)},
	{ HD(4,4),HD(5,4),HD(7,7)},
	{ HD(6,6),HD(7,7),HD(8,8)}
};

static const unsigned long len_5_6_tab[4][4] =
{
	{HD(1,3),HD(4,4),HD(7,6),HD(8,8)},
	{HD(4,4),HD(5,4),HD(8,6),HD(9,7)},
	{HD(7,5),HD(8,6),HD(9,7),HD(10,8)},
	{HD(8,7),HD(8,7),HD(9,8),HD(10,9)}
};

static const unsigned long len_7_8_9_tab[6][6]=
{
	{HT(1,2,3),HT(4,4,4),HT(7,7,6),HT(9,9,7),HT(9,9,9),HT(10,10,10)},
	{HT(4,4,4),HT(6,4,5),HT(8,6,6),HT(9,10,7),HT(9,10,8),HT(10,10,10)},
	{HT(7,7,5),HT(7,6,6),HT(9,8,7),HT(10,10,8),HT(10,10,9),HT(11,11,10)},
	{HT(8,9,7),HT(9,10,7),HT(10,10,8),HT(11,11,9),HT(11,11,9),HT(11,12,10)},
	{HT(8,9,8),HT(9,9,8),HT(10,10,9),HT(11,11,9),HT(11,12,10),HT(12,12,11)},
	{HT(9,10,9),HT(10,10,9),HT(11,11,10),HT(12,11,10),HT(12,13,11),HT(12,13,11)},
};

static const unsigned long len_10_11_12_tab[8][8]=
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

static const unsigned long len_13_15_tab[16][16]=
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

static const unsigned long len_16_24_tab[16][16] =
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



void countFunc_0(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	*bitsum = 0;
}


void countFunc_1(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	int i;
	unsigned long bit_cnt=0;
	
	for(i=0;i<no_of_pairs;i++)
		bit_cnt+=len_1_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	
	*bitsum = bit_cnt;

}

void countFunc_2_3(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	int i;
	unsigned long bit_cnt=0;
	
	for(i=0;i<no_of_pairs;i++)
		bit_cnt+=len_2_3_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	
	
	bitsum[0] = (bit_cnt>>16);
	bitsum[1] = (bit_cnt & 0xffff);

}


void countFunc_5_6(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{

	int i;
	unsigned long bit_cnt=0;
	
	for(i=0;i<no_of_pairs;i++)
		bit_cnt+=len_5_6_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	
	
	bitsum[0] = (bit_cnt>>16);
	bitsum[1] = (bit_cnt & 0xffff);

}

void countFunc_7_8_9(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
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

void countFunc_10_11_12(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
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

void countFunc_13_15(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	int i;
	unsigned long bit_cnt=0;
	
	for(i=0;i<no_of_pairs;i++)
		bit_cnt+=len_13_15_tab[quant_ptr[i*2]][quant_ptr[i*2+1]];	
	
	bitsum[0] = (bit_cnt>>16);
	bitsum[1] = (bit_cnt & 0xffff);

}

static void countFunc_Escape_Generic(unsigned int *quant_ptr,
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




void countFunc_16_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00010004L);
}


void countFunc_17_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00020004L);

}
void countFunc_18_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00030004L);
}

void countFunc_19_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00040004L);
}

void countFunc_20_25(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00060005L);
}

void countFunc_20_26(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00060006L);
}

void countFunc_21_27(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00080007L);
}

void countFunc_21_28(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x00080008L);
}

void countFunc_22_29(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000a0009L);
}

void countFunc_22_30(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000a000bL);
}

void countFunc_23_30(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000d000bL);
}

void countFunc_23_31(unsigned int *quant_ptr,int no_of_pairs,int *bitsum)
{
	countFunc_Escape_Generic(quant_ptr,no_of_pairs,bitsum,0x000d000dL);
}
