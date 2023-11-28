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
*   $Id: qc_data.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _QC_DATA_H
#define _QC_DATA_H

#include "dyn_bits.h"
#include "channelelement.h"
#include "mp3alloc.h"

#define OFL_V1_LEN       10 /* ofl data in byte */
#define ANC_BITS_DEFAULT 64 /* default ancillary bits per frame */  
#define ANC_FHG_WRAPPING 24 /* 24 bits wrapping for FhG mode */

/* Quantizing & coding stage */
 struct QC_INIT
{
  int nChannels;   /* number of channels */
  int mpegVersion; /* MPEG1, 2 or 2.5 ? */
  int maxBits;     /* maximum number of bits in reservoir */
  int dualPassLoop;     /* 0 == noDualPass,  1: First DualPassLoop, 2: Second DualPassLoop  */
  int desiredMeanBitrate;
  int predictGranules;
  int averageBits; /* average number of bits we should use */
  int averageBitsPerGranule; /* average number of bits we should use */
  int bitresStart; /* fill level of reservoir at start time (0, usually) */
  int granPerFrame;
  struct PE_STATS *peStats;

#ifndef OLD_THR_REDUCTION
  float meanPe;
#endif
  int vbrQuality;
  int channelBitrate;
  int fullHuffmanSearch;

  int useDualPass;
  /* int useLookAhead; */
  
  int dualMonoFlag;
  int sampleRate;
  
  CHANNEL_ELEMENT * chanElement ;
  CHANNEL_CONFIGURATION * channelConf[2] ;

};

 struct QC_OUT
{
  /* shared between both channels */

	int          dynBitsUsed;  /* for verification purposes */
	int          dynBitsUsedPerChannel[2];  /* for verification purposes */

  /* per-channel information */
	signed int  *quantSpec[2];     /* [FRAME_LEN_LONG]  quantized spectrum */
	int         *maxValueInSfb[2]; /* [MAX_GROUPED_SFB] max quantized value per sfb */
	int         *scf[2];           /* [MAX_GROUPED_SFB] scalefactors */
ALIGN_16_BYTE  int          globalGain[2];
ALIGN_16_BYTE  int          subblockGain[2][TRANS_FAC];
ALIGN_16_BYTE  int          scfScale[2];
ALIGN_16_BYTE  int          scfCompress[2];
ALIGN_16_BYTE  int          scfCntPerPartition[2][4];      /* number of sfb per partition          */
ALIGN_16_BYTE  int          scfBitsPerPartition[2][4];     /* bits used for each partition         */
ALIGN_16_BYTE  int          part2_3Length[2];              /* huffman+scf bits                     */
ALIGN_16_BYTE  int          preEmphasisFlag[2];


ALIGN_16_BYTE  int          blockType[2];   /* doesn't really belong here. Convenient, though. */
ALIGN_16_BYTE  int          mixedBlockFlag[2]; /* not used. */

ALIGN_16_BYTE  struct REGION_INFO regionInfo[2];
};

struct QC_STATE
{
  int mpegVersion;
  int averageBits;
  int maxBits;
  float maxPe;       /* maximum allowed pe for vbr mode */
  float minPe;       /* minimum required pe for vbr mode */
  int nChannels;
  int bitRes;
  /* more to be defined... */
#ifndef OLD_THR_REDUCTION
  struct ADJ_THR_STATE *hAdjThr[2];
#endif
  struct BITCNTR_STATE *hBitCounter;
  int averageBitsPerGranule;
  int deltaBitres;
  float vbrFactor; 

  int dualMonoFlag;

  int useDualPass;
  /* int useLookAhead; */
  
  CHANNEL_ELEMENT * chanElement ;
  CHANNEL_CONFIGURATION * channelConf[2] ;

  int dualPassLoop;    /* 0 == noDualPass,  1: First DualPassLoop, 2: Second DualPassLoop  */
  int predictGranules;
  unsigned int granuleCnt;      /* total number of granules quantized */
  struct PE_STATS *peStats;
};

#endif /* _QC_DATA_H */
