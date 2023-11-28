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
*   $Id: dyn_bits.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: Noiseless coder module                                               *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <limits.h>
#include <assert.h>
#include "dyn_bits.h"
#include "bit_cnt.h"
#include "psy_const.h"
#include "mathmac.h"
#include "mathlib.h"
#include "sf_cmprs.h"

typedef struct{
	unsigned int maxValue;
	int possibleTables;
ALIGN_16_BYTE int HuffmanTable[MAX_ALTERNATIVE_TABLES];
	void (*countFunction)(const unsigned int *quantPtr,int no_of_pairs,int *bitsum);
}HUFFTABLE_SELECT;


typedef struct{
	int nrOfCombinations;
	int nrOfSubregions;
ALIGN_16_BYTE int sfbSubDivCombinations[MAX_SUBDIV_COMBINATIONS][MAX_HC_SUBREGIONS];
}SUB_DIVIDE_INFO;


/*  
   count1 codeword length tables    
 */

ALIGN_16_BYTE static const int huff_len_cnt1_tab_00[] =
{
  1, 5, 5, 7, 5, 8, 7, 9, 5, 7, 7, 9, 7, 9, 9, 10
};

ALIGN_16_BYTE static const int huff_len_cnt1_tab_01[] =
{
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


/*
   huffman table information
 */

ALIGN_16_BYTE static HUFFTABLE_SELECT huffTableSelect[MAX_HUFFMAN_TABLES] =
{
  {0, 1, {0, -1, -1}, mp3CountFunc_0},
  {1, 1, {1, -1, -1}, mp3CountFunc_1},
  {2, 2, {2, 3, -1}, mp3CountFunc_2_3},
  {3, 2, {5, 6, -1}, mp3CountFunc_5_6},
  {5, 3, {7, 8, 9}, mp3CountFunc_7_8_9},
  {7, 3, {10, 11, 12}, mp3CountFunc_10_11_12},
  {15, 2, {13, 15, -1}, mp3CountFunc_13_15},
  {16, 2, {16, 24, -1}, mp3CountFunc_16_24},
  {18, 2, {17, 24, -1}, mp3CountFunc_17_24},
  {22, 2, {18, 24, -1}, mp3CountFunc_18_24},
  {30, 2, {19, 24, -1}, mp3CountFunc_19_24},
  {46, 2, {20, 25, -1}, mp3CountFunc_20_25},
  {78, 2, {20, 26, -1}, mp3CountFunc_20_26},
  {142, 2, {21, 27, -1}, mp3CountFunc_21_27},
  {270, 2, {21, 28, -1}, mp3CountFunc_21_28},
  {526, 2, {22, 29, -1}, mp3CountFunc_22_29},
  {1038, 2, {22, 30, -1}, mp3CountFunc_22_30},
  {2062, 2, {23, 30, -1}, mp3CountFunc_23_30},
  {8206, 2, {23, 31, -1}, mp3CountFunc_23_31}
};


/*****************************************************************************

    functionname:countZeroLines 
    description: Counts number of zero spectral lines
                                 at upper part of the spectrum wich are a multiple of 2
    returns:     number of zero lines    
    input:         
    output:        
    globals:       

*****************************************************************************/
static int countZeroRegion(const unsigned int * quantSpectrum, int noOfActiveLines)
{
  int i;
  int noOfZeros = FRAME_LEN_LONG - noOfActiveLines;

  assert(noOfActiveLines % 2 == 0);

  i = noOfActiveLines;
  while (i > 0) {
    i -= 2;
    if ((quantSpectrum[i] != 0) || (quantSpectrum[i + 1] != 0))
      break;
    noOfZeros += 2;
  }
  return (noOfZeros);
}

/*****************************************************************************

    functionname:countQuadruplesRegion 
    description: Counts number of count1 quadruples
                                 
    returns:     number of quadruples    
    input:         
    output:        
    globals:       

*****************************************************************************/
static int countQuadruplesRegion(const unsigned int * quantSpectrum, int countZero, int * count1Table, int * bitsumCount1)
{
  int i, codeWord;
  int bitsumTable0, bitsumTable1, noOfQuadruples;

  bitsumTable0 = 0;
  bitsumTable1 = 0;
  noOfQuadruples = 0;

  i = FRAME_LEN_LONG - countZero;


  while (i >= 4) {
    i -= 4;
    if ((quantSpectrum[i] > 1) ||
        (quantSpectrum[i + 1] > 1) ||
        (quantSpectrum[i + 2] > 1) ||
        (quantSpectrum[i + 3] > 1))
      break;

    noOfQuadruples++;

    codeWord = (quantSpectrum[i] << 3) +
        (quantSpectrum[i + 1] << 2) +
        (quantSpectrum[i + 2] << 1) +
        (quantSpectrum[i + 3]);

    bitsumTable0 += huff_len_cnt1_tab_00[codeWord];
    bitsumTable1 += huff_len_cnt1_tab_01[codeWord];
  }
  if (bitsumTable0 > bitsumTable1) {
    *count1Table = 1;
    *bitsumCount1 = bitsumTable1;
  } else {
    *count1Table = 0;
    *bitsumCount1 = bitsumTable0;
  }

  return (noOfQuadruples);
}



/*****************************************************************************

    functionname:subdivideBigvalRegion 
    description: Subdivides Bigval Region dependent on blocktype and method
                                 
    returns:     Subdivde Combination structure  
    input:         
    output:        
    globals:       

*****************************************************************************/
static void subdivideBigvalRegion(int bigValuePairs,
                                  int blockType,
                                  const int *sfbOffset,
                                  int method,
                                  SUB_DIVIDE_INFO * subDivideInfo)
{


  int BigValues, i, k, reg1_2, reg2;
  int nrOfBigvalueSfb, nrOfSfbRemaining, temp;

  if (blockType != LONG_WINDOW) {
    subDivideInfo->nrOfCombinations = 1;
    subDivideInfo->nrOfSubregions = 2;
    if (blockType == SHORT_WINDOW) {
      subDivideInfo->sfbSubDivCombinations[0][0] = REGION0_COUNT_SHORT;
      subDivideInfo->sfbSubDivCombinations[0][1] = REGION1_COUNT_SSS;
      subDivideInfo->sfbSubDivCombinations[0][2] = 0;
    } else {
      /* START,STOP,MIXED_START,MIXED_STOP,MIXED_SHORT */
      subDivideInfo->sfbSubDivCombinations[0][0] = REGION0_COUNT_START_STOP;
      subDivideInfo->sfbSubDivCombinations[0][1] = REGION1_COUNT_SSS;
      subDivideInfo->sfbSubDivCombinations[0][2] = 0;
    }
  } else {

    subDivideInfo->nrOfSubregions = 3;
    BigValues = bigValuePairs * 2;

    /*
       Count nr of sfb covering bigvalues 
     */
    nrOfBigvalueSfb = 0;
    while (BigValues > 0){
      BigValues -= sfbOffset[nrOfBigvalueSfb+1]-sfbOffset[nrOfBigvalueSfb];
    nrOfBigvalueSfb++;
    }

    /*method = HC_REG_FULLSEARCH;*/

    switch (method) {
    case HC_REG_STD:

      subDivideInfo->nrOfCombinations = 1;

      nrOfSfbRemaining = nrOfBigvalueSfb;

      /*
         first subregion
       */

      if (nrOfSfbRemaining <= 1)
        temp = 1;
      else
        temp = (int) (nrOfSfbRemaining * 0.333333333333f + 0.5f);

      temp = min(temp, MAX_REGION0_COUNT_LONG);

      subDivideInfo->sfbSubDivCombinations[0][0] = temp;

      /* 
         second & third subregion 
       */
      nrOfSfbRemaining -= temp;

      if (nrOfSfbRemaining > 0) {
        if (nrOfSfbRemaining <= 1)
          temp = 1;
        else
          temp = ((nrOfSfbRemaining + 1) >> 1);
        temp = min(temp, MAX_REGION1_COUNT_LONG);
        subDivideInfo->sfbSubDivCombinations[0][1] = temp;
        subDivideInfo->sfbSubDivCombinations[0][2] = nrOfSfbRemaining - temp;
      } else {
        subDivideInfo->sfbSubDivCombinations[0][1] = 1;
        subDivideInfo->sfbSubDivCombinations[0][2] = 0;
      }

      break;

    case HC_REG_FULLSEARCH:

      subDivideInfo->nrOfCombinations = 0;

      for (i = 1; i <= 16; i++) {
        if ((reg1_2 = nrOfBigvalueSfb - i) > 0) {
          for (k = 1; k <= 8; k++) {
            if ((reg2 = reg1_2 - k) > 0) {
              subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][0] = i;
              subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][1] = k;
              subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][2] = reg2;
              subDivideInfo->nrOfCombinations++;
            } else
              break;
          }
        } else
          break;
      }

      if (subDivideInfo->nrOfCombinations == 0) {
        subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][0] = 1;
        subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][1] = 1;
        subDivideInfo->sfbSubDivCombinations[subDivideInfo->nrOfCombinations][2] = 0;
        subDivideInfo->nrOfCombinations++;
      }
      break;
    }
  }

}

/*****************************************************************************

    functionname:countBigvalRegion 
    description: counts number of big value bits
                                 
    returns:     huffman coding sheme    
    input:         
    output:        
    globals:       

*****************************************************************************/
static void countBigvalRegion(int  blockType,
                              const int  *sfbOffset,
                              int method,
                              const unsigned int *quantSpectrum,
                              const unsigned int *maxValueInSfb,
                              REGION_INFO * regionInfo)
{

  int comb, reg, bigValuesLeft, sfbNdx, sfbCnt, tabNdx, possibleTableNdx;
  unsigned int localMax;

  unsigned int regLocalMax[MAX_HC_SUBREGIONS];
  int regBigValues[MAX_HC_SUBREGIONS];
  int regTableSelect[MAX_HC_SUBREGIONS];
  int regBitsum[MAX_HC_SUBREGIONS];
  int regSpecNdx;
  int totBitsum;
  int hufTableBitsum[MAX_ALTERNATIVE_TABLES];


  SUB_DIVIDE_INFO subDivideInfo;

  /*
     subdivide bigvalue region depending on block type & subdivide method
   */

  subdivideBigvalRegion(regionInfo->bigValuePairs,
                        blockType,
                        sfbOffset,
                        method,
                        &subDivideInfo);

  regionInfo->bigValueBitsum = INT_MAX;
  regionInfo->nrOfSubregions = subDivideInfo.nrOfSubregions;



  /*
     clear all unused tab/bitsum fields, not really neccesary,
     but keeps structure in clean state in case of START/STOP/SHORT blocks
   */

  for (reg = subDivideInfo.nrOfSubregions; reg < MAX_HC_SUBREGIONS; reg++) {
    regionInfo->tableSelect[reg] = 0;
    regionInfo->bitsumSubregion[reg] = 0;
    regionInfo->pairsSubregion[reg] = 0;
  }



  /*
     Step through all combinations
   */

  for (comb = 0; comb < subDivideInfo.nrOfCombinations; comb++) {

    bigValuesLeft = regionInfo->bigValuePairs * 2;
    sfbNdx = 0;

    /*
       Calculate number of bigvalues and local maximum of each region
     */

    for (reg = 0; reg < regionInfo->nrOfSubregions; reg++) {
      regBigValues[reg] = 0;
      regLocalMax[reg] = 0;
      sfbCnt = 0;

      do {
        if (bigValuesLeft > 0) {
          regBigValues[reg] += min(sfbOffset[sfbNdx+1]-sfbOffset[sfbNdx], bigValuesLeft);
          bigValuesLeft -= min(sfbOffset[sfbNdx+1]-sfbOffset[sfbNdx], bigValuesLeft);
          localMax = maxValueInSfb[sfbNdx];

          if (localMax > regLocalMax[reg])
            regLocalMax[reg] = localMax;
        }
        sfbNdx++;
        sfbCnt++;
      } while (sfbCnt < subDivideInfo.sfbSubDivCombinations[comb][reg]);

    }

    /*
       Select tables with minimum bit count for this combination
     */

    totBitsum = 0;
    regSpecNdx = 0;
    for (reg = 0; reg < regionInfo->nrOfSubregions; reg++) {

      regBitsum[reg] = INT_MAX;

      /*
         test all possible tables in this region 
         Note: optimization possible, not all possible tables
         have to be tested (e.g.tables 16-31)
         Note1: obsilate, break inserted
       */
      for (tabNdx = 0; tabNdx < MAX_HUFFMAN_TABLES; tabNdx++) {
        if (regLocalMax[reg] <= huffTableSelect[tabNdx].maxValue) {
          huffTableSelect[tabNdx].countFunction(quantSpectrum + regSpecNdx, regBigValues[reg] / 2, hufTableBitsum);
          for (possibleTableNdx = 0; possibleTableNdx < huffTableSelect[tabNdx].possibleTables; possibleTableNdx++) {
            if (hufTableBitsum[possibleTableNdx] < regBitsum[reg]) {
              regBitsum[reg] = hufTableBitsum[possibleTableNdx];
              regTableSelect[reg] = huffTableSelect[tabNdx].HuffmanTable[possibleTableNdx];
            }
          }
          break;
        }
      }
      totBitsum += regBitsum[reg];
      regSpecNdx += regBigValues[reg];
    }

    /*
       save sheme if current bitsum in this combination < best bitsum
     */

    if (totBitsum < regionInfo->bigValueBitsum) {
      regionInfo->bigValueBitsum = totBitsum;
      regionInfo->regionCountMinus1[0] = subDivideInfo.sfbSubDivCombinations[comb][0] - 1;
      regionInfo->regionCountMinus1[1] = subDivideInfo.sfbSubDivCombinations[comb][1] - 1;

      for (reg = 0; reg < regionInfo->nrOfSubregions; reg++) {
        regionInfo->tableSelect[reg] = regTableSelect[reg];
        regionInfo->bitsumSubregion[reg] = regBitsum[reg];
        regionInfo->pairsSubregion[reg] = regBigValues[reg] / 2;

      }
    }
  }


}

int
scfBitCount(/*const signed int scalefac[MAX_GROUPED_SFB],
              const int        scfCompress,*/
            const int        scfCntPerPartition[SCF_PARTITIONS],
            const int        scfBitsPerPartition[SCF_PARTITIONS])
{

  int i,scfBits;

  scfBits=0;
  for(i = 0; i < SCF_PARTITIONS && scfCntPerPartition[i]!=0; i++)
  {
    scfBits += scfBitsPerPartition[i]*scfCntPerPartition[i];
  }
  
  return(scfBits);
}



int
dynBitCount(struct BITCNTR_STATE *hBC,
            const int            *squantSpectrum,
            const unsigned int   *maxValueInSfb,
            const int             blockType,
            const int             sfbCnt,
            const int            *sfbOffset,
            struct REGION_INFO   *regionInfo)
{
 
  
  unsigned int *quantSpectrum = hBC->quantSpectrum;

  absINT(squantSpectrum, quantSpectrum, FRAME_LEN_LONG);

  /*
     Calculate count zero region
  */

  regionInfo->countZero = countZeroRegion(quantSpectrum, sfbOffset[sfbCnt]);

  /*
     Calculate count1 quadruples
   */

  regionInfo->count1 = countQuadruplesRegion(quantSpectrum,
                                              regionInfo->countZero,
                                              &(regionInfo->count1Table),
                                              &(regionInfo->count1Bitsum));

  /*
     Calculate bigvalue region
   */

  regionInfo->bigValuePairs = (FRAME_LEN_LONG - regionInfo->countZero - regionInfo->count1 * 4) / 2;

  /*
     Calculate big value bits
   */

  countBigvalRegion(blockType,
                    sfbOffset,
                    hBC->method,
                    quantSpectrum,
                    maxValueInSfb,
                    regionInfo);

  regionInfo->huffmanBitsum = regionInfo->count1Bitsum+regionInfo->bigValueBitsum;

  return (regionInfo->huffmanBitsum);
 
}


int BCNew(struct BITCNTR_STATE **phBC)
{
  struct BITCNTR_STATE *hBC = (struct BITCNTR_STATE *) mp3Alloc(sizeof(struct BITCNTR_STATE));
  if (hBC)
  {
    hBC->quantSpectrum   = (unsigned int *) mp3Alloc(sizeof(unsigned int)*FRAME_LEN_LONG);
    hBC->method = HC_REG_STD;
  
    if (hBC->quantSpectrum  == 0) 
    {
      BCDelete(hBC); hBC = 0;
    }
  }
  *phBC = hBC;
  return (hBC == 0);
}

void BCDelete(struct BITCNTR_STATE *hBC)
{
  if (hBC)
  {
    if (hBC->quantSpectrum) mp3Free(hBC->quantSpectrum);
    mp3Free(hBC);
  }
}

int
BCInit(struct BITCNTR_STATE *hBC, int fullHuffmanSearch)
{
  hBC->method = fullHuffmanSearch ? HC_REG_FULLSEARCH : HC_REG_STD;
  
  return (0);
}
