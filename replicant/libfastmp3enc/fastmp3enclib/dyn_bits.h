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
*   $Id: dyn_bits.h,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __DYN_BITS_H
#define __DYN_BITS_H

#include "scf.h"
#include "psy_const.h"
#include "mp3alloc.h"

#define REGION0_COUNT_SHORT       9
#define REGION0_COUNT_START_STOP  8
#define REGION1_COUNT_SSS        36
#define MAX_REGION0_COUNT_LONG   16
#define MAX_REGION1_COUNT_LONG    8		
#define MAX_HC_SUBREGIONS         3 
#define MAX_SUBDIV_COMBINATIONS (16*8) 

#define HC_REG_STD             0
#define HC_REG_FULLSEARCH      1
#define MAX_HUFFMAN_TABLES    19
#define MAX_ALTERNATIVE_TABLES 3





typedef ALIGN_16_BYTE struct REGION_INFO{
  int countZero;  /* nr of zero spectral lines */
  int count1;     /* nr of count1 quadrupels */ 
  int count1Table; /* count1 huffmancode table */
  int count1Bitsum; /* nr of bits used for count1 encoding */ 
  int bigValuePairs; /* nr of bigvalues pairs */
  int nrOfSubregions;  /* nr of bigvalue subregions */
ALIGN_16_BYTE int regionCountMinus1[MAX_HC_SUBREGIONS-1]; /* nr of sfbs in a bigvalues subregion - 1 */
ALIGN_16_BYTE int tableSelect[MAX_HC_SUBREGIONS]; /* bigvalues huffman code table for each subregion */
ALIGN_16_BYTE int bitsumSubregion[MAX_HC_SUBREGIONS]; /* nr of bits for each bigvalues subregion */
ALIGN_16_BYTE int pairsSubregion[MAX_HC_SUBREGIONS];  /* nr of pairs in each subregion */
  int bigValueBitsum;   /* nr of bits used for bigvalue pairs */
  int huffmanBitsum;    /* overall huffman bits */
}REGION_INFO;








struct BITCNTR_STATE
{
  int method;
  unsigned int *quantSpectrum;
};


int  BCNew(struct BITCNTR_STATE **phBC);
void BCDelete(struct BITCNTR_STATE *hBC);
int  BCInit(struct BITCNTR_STATE *hBC, int fullHuffmanSearch);
int  dynBitCount(struct BITCNTR_STATE *hBC,
                 const int *squantSpectrum,
                 const unsigned int *maxValueInSfb,
                 const int blockType,
                 const int sfbCnt,
                 const int *sfbOffset,
                 struct REGION_INFO *regionInfo);

int
scfBitCount(/*const signed int scalefac[MAX_GROUPED_SFB],
              const int        scfCompress,*/
            const int        scfCntPerPartition[SCF_PARTITIONS],
            const int        scfBitsPerPartition[SCF_PARTITIONS]);

#endif /* __DYN_BITS_H */
