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
*   $Id: huff_cnt.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: huffman bigvalue count functions                                     *
*                                                                                              *
************************************************************************************************/

#ifndef __HUFF_CNT_H
#define __HUFF_CNT_H

void countFunc_0(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_1(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_2_3(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_5_6(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_7_8_9(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_10_11_12(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_13_15(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_16_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_17_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_18_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_19_24(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_20_25(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_20_26(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_21_27(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_21_28(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_22_29(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_22_30(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_23_30(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void countFunc_23_31(unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
#endif
