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
*   $Id: sf_cmprs.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"

#include <assert.h>
#include <limits.h>

#include "sf_cmprs.h"
#include "mathlib.h"
#include "mp3alloc.h"

/* -------------------------------------------------------- */
/* external constants */

ALIGN_16_BYTE const int scfCntPerPartitionDefault [2][SCF_PARTITIONS] =
{
  {6,5,5,5},{9,9,9,9}
};
ALIGN_16_BYTE const int scfBitsPerPartitionDefault[2][SCF_PARTITIONS] =
{
  {4,4,3,3},{4,4,3,3}
};

/* -------------------------------------------------------- */
/* internal constants */

enum {
  N_SCHEMES_MPEG1 = 16,
  N_SCHEMES_MPEG2 = 6
};

static const int scfBitsPerMpeg1Partition[N_SCHEMES_MPEG1][2] =
{
  {0,0},{0,1},{0,2},{0,3},{3,0},{1,1},{1,2},{1,3},
  {2,1},{2,2},{2,3},{3,1},{3,2},{3,3},{4,2},{4,3}
};

static const int scfCntPerMpeg2Partition[N_SCHEMES_MPEG2][3/*blocktype*/][SCF_PARTITIONS] =
{
  /* long, short, mixed */

  /* non-intensity */
  {{6,5,5,5},  {9,9,9,9},   {6,9,9,9}},
  {{6,5,7,3},  {9,9,12,6},  {6,9,12,6}},
  {{11,10,0,0},{18,18,0,0}, {15,18,0,0}},
  /* intensity */
  {{7,7,7,0},  {12,12,12,0},{6,15,12,0}},
  {{6,6,6,3},  {12,9,9,6},  {6,12,9,6}},
  {{8,8,5,0},  {15,12,9,0}, {6,18,9,0}}
};

static const int preEmphasisWithScheme[N_SCHEMES_MPEG2] =
{0,0,1,0,0,0};

static const int scfBitsPerMpeg2Partition[N_SCHEMES_MPEG2][SCF_PARTITIONS] =
{
  /* non-intensity */
  {4,4,3,3}, {4,4,3,0}, {3,2,0,0},
  /* intensity */
  {4,5,5,0}, {3,3,3,0}, {3,2,0,0}
};

/* log2[i] */
static const int ldTab[33] =
  {0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};

static int encodeSlen(int scheme, const int scfBits[SCF_PARTITIONS])
{
  static const int intensity_scale = 0;
  switch (scheme)
  {
  case 0:
    return ((scfBits[0]*5 + scfBits[1])*4 + scfBits[2])*4 + scfBits[3] + 0 ;
  case 1:
    return (scfBits[0]*5 + scfBits[1])*4 + scfBits[2] + 400 ;
  case 2:
    return scfBits[0]*3 + scfBits[1] + 500 ;

  /* intensity compress */
  case 3:
    return ((scfBits[0]*6 + scfBits[1])*6 + scfBits[2])*2 + intensity_scale ;
  case 4:
    return (((scfBits[0]*4 + scfBits[1])*4 + scfBits[2])+180)*2 + intensity_scale ;
  case 5:
    return ((scfBits[0] * 3 + scfBits[1])+244)*2 + intensity_scale ;
  }
  assert(0); /* never get here */
  return 0;
}

static int findscfMax(const int *scf, int sfbCnt)
{
  int i;
  int scfMax = INT_MIN;

  for (i = 0; i < sfbCnt; i++)
  {
    if (scf[i] != SCF_DONT_CARE && scf[i] > scfMax)
      scfMax = scf[i];
  }

  return scfMax == INT_MIN ? 0 : scfMax;
}

unsigned int
findScfCompressMPEG1(const int blockType,
                     const int scf[MAX_GROUPED_SFB],
                     int       scfCntPerPartition[SCF_PARTITIONS],
                     int       scfBitsPerPartition[SCF_PARTITIONS])
{
  int i,scheme;
  int offset;

  int bitsPerPartition[SCF_PARTITIONS]={0,0,0,0};

  int minScfBits;
  int scfCompress;

  copyINT(scfCntPerPartitionDefault[blockType != SHORT_WINDOW ? 0 : 1],
          scfCntPerPartition,
          SCF_PARTITIONS);

  /*
    First, find the maximum scalefactor in each partition.
  */
  offset = 0;
  for ( i = 0; i < SCF_PARTITIONS; i++ )
  {
    bitsPerPartition[i] = ldTab[findscfMax(scf+offset,scfCntPerPartition[i])];
    offset  += scfCntPerPartition[i];
  }

  scfCompress = -1;
  minScfBits = INT_MAX;
  for (scheme = 0; scheme < N_SCHEMES_MPEG1; scheme++)
  {
    int j ;

    /*
      Check if the scalefactors fit into the number of bits provided by this scheme
    */

    for ( j = 0; j < SCF_PARTITIONS; j++ )
    {
      if (bitsPerPartition[j] > scfBitsPerMpeg1Partition[scheme][j/2]) break;
    }

    if (j == SCF_PARTITIONS )
    /*
      scalefactors fit into this compression scheme. Now check if this scheme
      uses up less bits than the best scheme found so far.
     */
    {
      int scfBits = 0;
      /* count the number of bits the scfs take with this scheme */
      for ( j = 0; j < SCF_PARTITIONS; j++ )
      {
        scfBits += scfBitsPerPartition[j]*scfCntPerPartition[j];
      }

      /* is this scheme better? */
      if ( scfBits < minScfBits )
      { /* yes! */
        minScfBits = scfBits ;
        scfCompress = scheme;
      }
    }
  }
  assert(scfCompress != -1); /* assert that a valid scfCompress was found */

  /*
    set scfBitsPerPartition to the best compression scheme found.
   */

  scfBitsPerPartition[0] = scfBitsPerPartition[1]
    = scfBitsPerMpeg1Partition[scfCompress][0];
  scfBitsPerPartition[2] = scfBitsPerPartition[3]
    = scfBitsPerMpeg1Partition[scfCompress][1];
  return scfCompress;
}

unsigned int
findScfCompressMPEG2(const int intensityOn,
                     const int blockType,
                     const int scf[MAX_GROUPED_SFB],
                     const int preEmphasis,
                     int       scfCntPerPartition[SCF_PARTITIONS],
                     int       scfBitsPerPartition[SCF_PARTITIONS],
                     const int isLimit[TRANS_FAC],
                     int       sfbActive)
{
  int scheme;
  int bestScheme  = -1;
  int scfCompress = -1;
  int minScfBits  = INT_MAX;
  int tmpScf[MAX_GROUPED_SFB] ={0};

  /* add intensityOn to tmpScf considering isLimit */ 
  copyINT(scf, tmpScf, MAX_GROUPED_SFB);
  if(intensityOn) {
    if(blockType != SHORT_WINDOW) { 
      /* long */
      int k;
      for(k=isLimit[0]; k < sfbActive; k++) { 
        if(tmpScf[k] != SCF_DONT_CARE) {
          tmpScf[k] = tmpScf[k] + intensityOn;
        }
      } 
    }
    else { 
      /* short */
      int i,j;
      for(i=0; i<TRANS_FAC; i++) {
        /* interleaved */
        for(j=isLimit[i]*TRANS_FAC+i; j < MAX_GROUPED_SFB; j=j+TRANS_FAC) { 
          if(tmpScf[j] != SCF_DONT_CARE) {  
            tmpScf[j] = tmpScf[j] + intensityOn;           
          }
        }
      }
    }
  }  

  for (scheme = intensityOn*3; scheme < (intensityOn+1)*3; scheme++)
  {
    int i,j;
    int offset;
    int bitsPerPartition[SCF_PARTITIONS]={0,0,0,0};

    /* preemphasis on/off is coded in the scheme used. */
    if (preEmphasisWithScheme[scheme] != preEmphasis)
      continue ;

    /* mixed block types not yet supported. sdb */
    copyINT(scfCntPerMpeg2Partition[scheme][blockType != SHORT_WINDOW ? 0 : 1],
            scfCntPerPartition,
            SCF_PARTITIONS);

    /*
      First, find the maximum scalefactor in each partition.
    */
    offset = 0;
    for ( i = 0; i < SCF_PARTITIONS && scfCntPerPartition[i]!=0; i++ )
    {
      /*
        in partitions where intensity positions are transmitted, we need one
        value more than the maximum value, else there are cases where the
        decoder will decode this value as an illegal intensity position.
        In the first partition, where the max nr of bits is 4,
        there will never be intensity, because isLimitLow
        is set to 7 in psy_configuration.c. Nevertheless the
        number of bits in this partition must be greater than 0 */

      /* do not use last entry in ldTab */
      assert( findscfMax(tmpScf+offset, scfCntPerPartition[i]) < (int)( (sizeof(ldTab)/sizeof(int) )-1) );

      bitsPerPartition[i] =
        ldTab[ findscfMax(tmpScf+offset, scfCntPerPartition[i]) ]; 
     
      if (intensityOn && bitsPerPartition[i] == 0) {
        bitsPerPartition[i] = 1; 
      }
 
      offset  += scfCntPerPartition[i];
    }

    /*
      Check if the scalefactors fit into the number of bits provided by this scheme
    */

    for ( j = 0; j < SCF_PARTITIONS && scfCntPerPartition[j]!=0; j++ )
    {
      if (bitsPerPartition[j] > scfBitsPerMpeg2Partition[scheme][j]) break;
    }

    if (j == SCF_PARTITIONS || scfCntPerPartition[j]==0 )
    /*
      scalefactors fit into this compression scheme. Now check if this scheme
      uses up less bits than the best scheme found so far.
     */
    {
      int scfBits = 0;
      /* count the number of bits the scfs take with this scheme */
      for ( j = 0; j < SCF_PARTITIONS && scfCntPerPartition[j]!=0; j++ )
      {
        scfBits += bitsPerPartition[j] * scfCntPerPartition[j];
      }

      /* is this scheme better? */
      if ( scfBits < minScfBits )
      { /* yes! */
        minScfBits  = scfBits ;
        bestScheme  = scheme;
        scfCompress = encodeSlen(scheme, bitsPerPartition);
        assert(scfCompress < 512);

        /*
          set scfBitsPerPartition
        */
        copyINT(bitsPerPartition,
                scfBitsPerPartition,
                SCF_PARTITIONS);
      }
    }
  }
  assert(bestScheme != -1); /* assert that a valid scfCompress was found */

  /* mixed block types not yet supported sdb */
  copyINT(scfCntPerMpeg2Partition[bestScheme][blockType != SHORT_WINDOW ? 0 : 1],
          scfCntPerPartition,
          SCF_PARTITIONS);

  return scfCompress;
}
