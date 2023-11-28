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
*   $Id: bit_cnt.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __BITCOUNT_H
#define __BITCOUNT_H

#include "mp3bitbuf.h"

void mp3CountFunc_0(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_1(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_2_3(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_5_6(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_7_8_9(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_10_11_12(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_13_15(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_16_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_17_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_18_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_19_24(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_20_25(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_20_26(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_21_27(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_21_28(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_22_29(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_22_30(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_23_30(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);
void mp3CountFunc_23_31(const unsigned int *quant_ptr,int no_of_pairs,int *bitsum);





void mp3EncodeHuffPair(int           *specValuePair,
                       int            huffTableSelect,
                       HANDLE_BIT_BUF hBitstream);

void mp3EncodeHuffQuadruple(int           *specValueQuadruple,
                            int            huffTableSelect,
                            HANDLE_BIT_BUF hBitstream);


#endif
