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
*   $Id: qc_main.c,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                                        *
*   author:   M. Werner                                                                        *
*   contents/description:  Quantizing & coding                                                 *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include <stdio.h>
#include "mp3alloc.h"
#include <assert.h>
#include <math.h>
#include "sf_estim.h"
#include "qc_main.h"
#include "quantize.h"
#include "adj_thr.h"

#include "sf_cmprs.h"

#include "interface.h"

#include "bit_cnt.h"
#include "dyn_bits.h"
#include "mathmac.h"
#include "mathlib.h"
#include "utillib.h"

/* forward declarations */

static int calcMaxValueInSfb(const int sfbCnt,
                             const int sfbOffset[MAX_GROUPED_SFB],
                             const signed int quantSpectrum[FRAME_LEN_LONG],
                             int maxValue[MAX_GROUPED_SFB]);


#define MP3_MAX_GRANULE_LENGTH  7680
#define MP3_MAX_PART23_LENGTH   4092 /* originally 4095 but for compatibility reasons 
                                        with non compliant decoders, reduced to the 
                                        largest multiple of 4 less than 4095          */

void InitChannelConf( struct QC_INIT init, struct QC_STATE hQC ){
  
  if ( hQC.dualMonoFlag )
    {
      hQC.channelConf[0]->nChannels = 1;
      hQC.channelConf[1]->nChannels = 1;

      hQC.channelConf[0]->channelBitrate = (int)(init.channelBitrate);
      hQC.channelConf[1]->channelBitrate = (int)(init.channelBitrate);
    
      hQC.channelConf[0]->averageBits = (int)(hQC.averageBits * 0.5);
      hQC.channelConf[1]->averageBits = hQC.averageBits -
        hQC.channelConf[0]->averageBits;

      hQC.channelConf[0]->averageBitsPerGranule = (int)(hQC.averageBitsPerGranule * 0.5);
      hQC.channelConf[1]->averageBitsPerGranule = hQC.averageBitsPerGranule -
        hQC.channelConf[0]->averageBitsPerGranule;
      
      hQC.channelConf[0]->maxBits = (int)(init.maxBits * 0.5);
      hQC.channelConf[1]->maxBits = init.maxBits - hQC.channelConf[0]->maxBits ;
      hQC.channelConf[0]->bitRes =  (int)(hQC.bitRes * 0.5);
      hQC.channelConf[1]->bitRes =  hQC.bitRes - hQC.channelConf[0]->bitRes;
    
      hQC.channelConf[0]->deltaBitres = hQC.channelConf[1]->deltaBitres = hQC.deltaBitres ;    

      hQC.channelConf[0]->maxBitFac = hQC.channelConf[1]->maxBitFac = 
        (float) min( hQC.channelConf[0]->maxBits,
                     MP3_MAX_PART23_LENGTH)   / 
        (float) (hQC.channelConf[0]->averageBitsPerGranule );
    }
  else
    {
      hQC.channelConf[0]->nChannels = hQC.channelConf[1]->nChannels = init.nChannels;
      hQC.channelConf[0]->channelBitrate = hQC.channelConf[1]->channelBitrate = (int)(init.channelBitrate);
  
      hQC.channelConf[0]->averageBits = hQC.channelConf[1]->averageBits = (int)(hQC.averageBits);
      hQC.channelConf[0]->averageBitsPerGranule = 
        hQC.channelConf[1]->averageBitsPerGranule = (int)(hQC.averageBitsPerGranule);
    
      hQC.channelConf[0]->maxBits = hQC.channelConf[1]->maxBits =  hQC.maxBits;
      hQC.channelConf[0]->bitRes = hQC.channelConf[1]->bitRes = hQC.bitRes;
         
      hQC.channelConf[0]->deltaBitres = hQC.channelConf[1]->deltaBitres = hQC.deltaBitres ;

      hQC.channelConf[0]->maxBitFac = hQC.channelConf[1]->maxBitFac =
        (float) min(  min( (hQC.channelConf[0]->nChannels * MP3_MAX_PART23_LENGTH ),
                           MP3_MAX_GRANULE_LENGTH ),
                      hQC.channelConf[0]->maxBits) / 
        (float) (hQC.channelConf[0]->averageBitsPerGranule );
    }
}

/*****************************************************************************

    functionname: QCOutNew
    description:  allocate a Q&C Output structure
    returns:      error code

*****************************************************************************/
int QCOutNew(struct QC_OUT **phQC)
{
  struct QC_OUT *hQC = (struct QC_OUT *) mp3Alloc(sizeof(struct QC_OUT));

  if (hQC)
  {
    /*
      for performance reasons, the sse quantize routine might write some additional 
      (usually 2, but not more than 4) values to quantSpec, therefore enlarge the 
      allocated memory by four ints, to avoid unwanted side effects 
    */
    hQC->quantSpec[0] = (int *) mp3Alloc( (FRAME_LEN_LONG+4) * sizeof(int)); 
    hQC->quantSpec[1] = (int *) mp3Alloc( (FRAME_LEN_LONG+4) * sizeof(int)); 
    hQC->maxValueInSfb[0] = (int *) mp3Alloc(MAX_GROUPED_SFB* sizeof(int));
    hQC->maxValueInSfb[1] = (int *) mp3Alloc(MAX_GROUPED_SFB * sizeof(int));
    hQC->scf[0] = (int *) mp3Alloc(MAX_GROUPED_SFB * sizeof(int));
    hQC->scf[1] = (int *) mp3Alloc(MAX_GROUPED_SFB * sizeof(int));

    if (hQC->quantSpec[0]     == 0 ||
        hQC->quantSpec[1]     == 0 ||
        hQC->maxValueInSfb[0] == 0 ||
        hQC->maxValueInSfb[1] == 0 ||
        hQC->scf[0]           == 0 ||
        hQC->scf[1]           == 0)
    {
      QCOutDelete(hQC); hQC = 0;
    }
  }
  *phQC = hQC;
  return (hQC == 0);
}

/*****************************************************************************

    functionname: QCOutInit
    description:  Initialize a Q&C Out structure
    returns:      error code
    input:        an allocated Q&C Out structure

*****************************************************************************/
int QCOutInit(struct QC_OUT *hQC)
{
  /* nothing to do */
  (void) hQC ;
  return 0;
}

/*****************************************************************************

    functionname: QCOutDelete
    description:  Release all memory allocated by QCOutNew

*****************************************************************************/
void QCOutDelete(struct QC_OUT *hQC)
{
  if (hQC)
  {
    if (hQC->quantSpec[0])     mp3Free(hQC->quantSpec[0]);
    if (hQC->quantSpec[1])     mp3Free(hQC->quantSpec[1]);
    if (hQC->scf[0])           mp3Free(hQC->scf[0]);
    if (hQC->scf[1])           mp3Free(hQC->scf[1]);
    if (hQC->maxValueInSfb[0]) mp3Free(hQC->maxValueInSfb[0]);
    if (hQC->maxValueInSfb[1]) mp3Free(hQC->maxValueInSfb[1]);

    mp3Free(hQC);
  }
}

/*****************************************************************************

    functionname: QCNew
    description:  Allocate all memory for a Q&C handle
    returns:      error code
    input:        a pointer to a handle

*****************************************************************************/
int QCNew(struct QC_STATE **phQC)
{
  int i;
  struct QC_STATE *hQC = (struct QC_STATE *) mp3Alloc(sizeof(struct QC_STATE));

#ifndef OLD_THR_REDUCTION
  if (AdjThrNew(&(hQC->hAdjThr[0])))
  {
     QCDelete(hQC); hQC = 0;
  }
  if (AdjThrNew(&(hQC->hAdjThr[1])))
  {
     QCDelete(hQC); hQC = 0;
  }
  
#endif
  if (BCNew(&(hQC->hBitCounter)))
  {
    QCDelete(hQC); hQC = 0;
  }

  for ( i = 0; i < 2 ; i++)
    if ( CCNew(&(hQC->channelConf[i])) )
      {
        QCDelete(hQC); hQC = 0;
      }

  if ( CENew(&(hQC->chanElement)))
    {
      QCDelete(hQC); hQC = 0;
    }

  *phQC = hQC;
  return (hQC == 0);
}

/*****************************************************************************

    functionname: QCDelete
    description:  Release all memory allocated by QCNew()

*****************************************************************************/
void QCDelete(struct QC_STATE *hQC)
{
  if (hQC)
  {
#ifndef OLD_THR_REDUCTION
    AdjThrDelete(hQC->hAdjThr[0]);
    AdjThrDelete(hQC->hAdjThr[1]);    
#endif

    BCDelete(hQC->hBitCounter);
    CCDelete(hQC->channelConf);
    CEDelete(hQC->chanElement);
    mp3Free(hQC);
  }
}

extern int loop;

static const struct 
{
  int   quality;
  float vbrFactor;
  int   channelBitrate;
} tableTanh[] = 
{
  { 0,  300.0f, 20000},
  { 5,  600.0f, 24000},
  {10,  800.0f, 32000},
  {20, 1000.0f, 40000},
  {30, 2000.0f, 48000},
  {40, 3500.0f, 56000},
  {60, 5000.0f, 64000},
  {80, 7000.0f, 80000},
  {100,9000.0f, 96000}
};
static const struct {
  int   quality;
  float vbrFactor;
  int   channelBitrate;
} tableLog[6] = 
{
  { 0, 550.0f,  40000},
  {20, 625.0f,  48000},
  {40, 850.0f,  56000},
  {60, 1200.0f, 64000},
  {80, 2100.0f, 80000},
  {100,3500.0f, 96000}
} ;


/*****************************************************************************

    functionname: QCInit
    description:  Initialize an allocated Q&C handle
    returns:      error code
    input:        a handle, a filled config struct
    output:       an initialized QC handle

*****************************************************************************/
int QCInit(struct QC_STATE *hQC, struct QC_INIT *init)
{
  int channelBitrate;
  float t=0;
  unsigned int i;
#ifndef NO_NEW_AVOID_HOLE_STRATEGY
  float bitsPerSample = (float)init->averageBitsPerGranule / (init->nChannels*FRAME_LEN_LONG);
#endif
  hQC->nChannels        = init->nChannels;
  hQC->maxBits          = init->maxBits;
  hQC->dualPassLoop     = init->dualPassLoop;
  hQC->predictGranules  = init->predictGranules;
  hQC->bitRes           = init->bitresStart;
  hQC->averageBits      = init->averageBits;
  hQC->mpegVersion      = init->mpegVersion;
  hQC->peStats          = init->peStats;

  BCInit(hQC->hBitCounter, init->fullHuffmanSearch);
  hQC->averageBitsPerGranule = init->averageBitsPerGranule;
  hQC->deltaBitres = 0;

  channelBitrate = init->channelBitrate;

  t = min( t, 1.0f );
  t = max( t, 0.0f );

  if( !init->desiredMeanBitrate && !channelBitrate )
    {
      /* use quality to interpolate between table entries */
      for (i = 0; i+1 < sizeof(tableLog) / sizeof(tableLog[0]) - 1; i++)
        if(tableLog[i+1].quality >= init->vbrQuality)
          break;

      t = (float)( init->vbrQuality - tableLog[i].quality ) /
        (float)( tableLog[i+1].quality - tableLog[i].quality );

      channelBitrate = (int)(tableLog[i].channelBitrate * (1.0f-t) +
                             tableLog[i+1].channelBitrate * t );
      hQC->vbrFactor  = tableLog[i].vbrFactor * (1.0f-t) + tableLog[i+1].vbrFactor * t;
      hQC->vbrFactor *= 44100.0f / (float)min( init->sampleRate, 44100 );

    }
  else
    {
      if( !channelBitrate )
        channelBitrate = init->desiredMeanBitrate / init->nChannels;

      for (i = 0; i+1 < sizeof(tableTanh) / sizeof(tableTanh[0]) - 1; i++)
        if( tableTanh[i+1].channelBitrate >= channelBitrate )
          break;

      t = (float)( channelBitrate - tableTanh[i].channelBitrate ) /
        (float)( tableTanh[i+1].channelBitrate - tableTanh[i].channelBitrate );
      hQC->vbrFactor  = tableTanh[i].vbrFactor * (1.0f-t) + tableTanh[i+1].vbrFactor * t;
      hQC->vbrFactor *= 44100.0f / (float)min( init->sampleRate, 44100 );
    }


  /* hQC->peStats->Q = hQC->vbrFactor; */

#ifndef OLD_THR_REDUCTION
  hQC->hAdjThr[0]->granPerFrame = init->granPerFrame;

#ifdef NO_NEW_AVOID_HOLE_STRATEGY
  AdjThrInit(hQC->hAdjThr[0], init->meanPe, channelBitrate);
  AdjThrInit(hQC->hAdjThr[1], init->meanPe, channelBitrate);
#else
  AdjThrInit(hQC->hAdjThr[0], init->meanPe, channelBitrate, bitsPerSample);
  AdjThrInit(hQC->hAdjThr[1], init->meanPe, channelBitrate, bitsPerSample);
#endif  

#endif

  hQC->dualMonoFlag = init->dualMonoFlag ;

  hQC->useDualPass = init->useDualPass;

  InitChannelConf( *init, *hQC );

  return 0;
}

/*****************************************************************************

    functionname: QCMain
    description:  quantizes spectrum; finds scalefactors
    returns:      error code
    input:        Psych output
    output:       everything we need to write a bitstream

*****************************************************************************/
int QCMain(struct QC_STATE *hQC,
           /*struct PSY_OUT_LIST *firstPsyOut,*/
           struct PSY_OUT_LIST *qcPsyOut,
           struct PSY_OUT_LIST *lastPsyOut,
           struct QC_OUT *qcOut,    /* out */
           CHANNEL_ELEMENT * chanElement,
           int needAncBitsGran
           ) /* returns error code */
{

  int sfb;
  int channel;
  int maxScf[MAX_CHANNELS][MAX_GROUPED_SFB];
  struct PSY_OUT *psyOut = qcPsyOut->psyOut;
  ALIGN_16_BYTE int maxPart2_3Length[MAX_CHANNELS];
  ALIGN_16_BYTE int part2_3LengthAll = 0;
  ALIGN_16_BYTE int maxPart2_3LengthAll = 0;
  ALIGN_16_BYTE float chBitDistribution[MAX_CHANNELS];
  int fullPsych = (hQC->useDualPass)?1:0;
  int bitsFirst[MAX_CHANNELS] = {0};
  int iter = 0;
#if 0
  fprintf(stderr,"%d   ", psyOut->psyOutChannel[0].windowSequence);
  fprintf(stderr,"bitresFillLevel: % 5d (%.2f)\n", hQC->channelConf[chanElement->channel[0]]->bitRes, (float)hQC->channelConf[chanElement->channel[0]]->bitRes/hQC->maxBits);
  if (hQC->bitRes < 0) {
     fprintf(stderr, "too little bits in bitres\n");
     exit(20);
  }
  if (hQC->bitRes > hQC->maxBits) {
     fprintf(stderr, "too many bits in bitres\n");
     exit(20);
  }
#endif
 
#ifdef PLOTMTV 
  {
    int ch;
    for(ch=0;ch<hQC->nChannels;ch++) {
    setDeboutVars(-1,-1,ch,-1);
    sendDebout("estScalInp", psyOut->psyOutChannel[ch].sfbActive,1,"thresB",MTV_FLOAT,psyOut->psyOutChannel[ch].sfbThreshold);
  }
  setDeboutVars(-1,-1,0,-1); }
#endif
  if (hQC->averageBits > 0)
  {
#ifdef OLD_THR_REDUCTION
    AdjustThresholds(psyOut, chBitDistribution, hQC->nChannels, 
                     hQC->averageBitsPerGranule, 
                     hQC->bitRes - hQC->averageBits + hQC->deltaBitres, 
                     hQC->maxBits - hQC->averageBits);
#else
    int cc, startCh, endCh;
    for ( cc = 0 ; cc < chanElement->noOfChannelConfigurations; cc++ ){

      startCh = (chanElement->noOfChannelConfigurations == 2 ) ? cc : 0 ;
      endCh = (chanElement->noOfChannelConfigurations == 2 ) ? (cc+1) : 2 ;

      setDeboutVars(-1,-1,-1,cc);
#ifndef NO_DUAL_MONO_FIXES
      if(chanElement->noOfChannelConfigurations==2) {
        int ch;
        qcPsyOut->peData.pe = 0;
        for(ch=startCh;ch<endCh;ch++) {
          qcPsyOut->peData.pe += qcPsyOut->peData.peChannelData[ch].pe;
        }
      }
#endif
      AdjustThresholds(hQC->hAdjThr[chanElement->channel[cc]], qcPsyOut, chBitDistribution, 
                       hQC->channelConf[chanElement->channel[cc]]->nChannels, 
                       (hQC->channelConf[chanElement->channel[cc]]->averageBitsPerGranule -
                        (needAncBitsGran>>hQC->dualMonoFlag)), 
                       hQC->channelConf[chanElement->channel[cc]]->bitRes - 
                       hQC->channelConf[chanElement->channel[cc]]->averageBits + 
                       hQC->channelConf[chanElement->channel[cc]]->deltaBitres,
                       hQC->channelConf[chanElement->channel[cc]]->maxBits - 
                       hQC->channelConf[chanElement->channel[cc]]->averageBits,
                       hQC->channelConf[chanElement->channel[cc]]->maxBitFac,
                       startCh, endCh );
      
      setDeboutVars(-1,-1,-1,0);
    }
#endif
  }
#ifndef OLD_THR_REDUCTION
  else
  {
    int cc, startCh, endCh;
    for ( cc = 0 ; cc < chanElement->noOfChannelConfigurations; cc++ )
      {
        startCh = (chanElement->noOfChannelConfigurations == 2 ) ? cc : 0 ;
        endCh = (chanElement->noOfChannelConfigurations == 2 ) ? (cc+1) : 2 ;

        if( !hQC->useDualPass  )
          AdjustThresholdsVBR( hQC->hAdjThr[0], /*firstPsyOut,*/ qcPsyOut,
                               lastPsyOut, chBitDistribution,
                               hQC->channelConf[0]->nChannels,
                               hQC->channelConf[0]->maxBits - (needAncBitsGran>>hQC->dualMonoFlag),
                               hQC->vbrFactor, startCh, endCh,
                               hQC->granuleCnt,
			       hQC->predictGranules, hQC->peStats );
        else
          AdjustThrDualPass( hQC->hAdjThr[0], qcPsyOut, chBitDistribution,
                             hQC->channelConf[0]->nChannels,
                             hQC->channelConf[0]->maxBits - (needAncBitsGran>>hQC->dualMonoFlag),
                             /*hQC->vbrFactor, */startCh, endCh,
                             hQC->granuleCnt, hQC->peStats );
      }
  }
#endif

  /*
    estimate scalefactors that fulfill the psychoacoustic requirements
  */
  for (channel = 0; channel < hQC->nChannels; channel++) {
    (void)setDeboutVars(-1,-1,channel,-1);
    EstimateScaleFactorsChannel(&(psyOut->psyOutChannel[channel]),
                                qcOut->scf[channel],
                                psyOut->psyOutChannel[channel].sfbFormFactor,
                                psyOut->psyOutChannel[channel].sfbMaxSpec);
  }

  /*
    adapt scale factors to layer 3 requirements.
  */
  for (channel = 0; channel < hQC->nChannels; channel++)
  {
    struct PSY_OUT_CHANNEL *pPsyChan = &(psyOut->psyOutChannel[channel]);
    int windowSequence = pPsyChan->windowSequence ;

    /* estimateScaleFactorsChannel knows nothing about layer3 peculiarities.
       We deal with that here, fiddling with the scfs until they fit into the
       layer-3 scheme of things. */

    /* use a default scf compression scheme that always works */
    copyINT(scfCntPerPartitionDefault[windowSequence != SHORT_WINDOW ? 0 : 1],
            qcOut->scfCntPerPartition[channel], 4);
    copyINT(scfBitsPerPartitionDefault[windowSequence != SHORT_WINDOW ? 0 : 1],
            qcOut->scfBitsPerPartition[channel], 4);

    AdaptScfToMp3Channel(/*&(psyOut->psyOutChannel[channel]),*/
                         pPsyChan->sfbActive,
                         windowSequence,
                         /*hQC->mpegVersion,*/
                         fullPsych,
                         qcOut->scf[channel],
                         &(qcOut->globalGain[channel]),
                         qcOut->subblockGain[channel],
                         (unsigned int *) &(qcOut->scfScale[channel]),
                         /*(unsigned int *) &(qcOut->scfCompress[channel]),*/
                         qcOut->scfCntPerPartition[channel],
                         qcOut->scfBitsPerPartition[channel],
                         hQC->mpegVersion == MPEG1 && !psyOut->isUsed,
                         &(qcOut->preEmphasisFlag[channel]),
                         maxScf[channel],
                         psyOut->psyOutChannel[channel].sfbMaxSpec);
  }

  /*
    we now got scalefactors. Go ahead and quantize.
   */
  for (channel = 0; channel < hQC->nChannels; channel++) {
    int maxPart2_3LengthBitres;
    maxPart2_3Length[channel] = 4093; /* This value belongs to the fillbit section in BSWrite. Maximum is not 4095 but 4092. */
     
    /* reserve some space for ancillary Data bits -- not tested for dual mono yet */
    maxPart2_3Length[channel] -= needAncBitsGran>>(hQC->nChannels-1); /* ert + wmb: might be: hQC->nChannels-1, was: needAncBitsGran>>hQC->nChannels ?!?*/

    /* additional condition to prevent empty bitreservoir */
    if (hQC->averageBits > 0) {
       
      chBitDistribution[channel] = min ( 1.0f , chBitDistribution[channel] ); 

      maxPart2_3LengthBitres = (int)floor(chBitDistribution[channel] * 
                                          ((hQC->channelConf[chanElement->channel[channel]]->averageBitsPerGranule + 
                                            hQC->channelConf[chanElement->channel[channel]]->bitRes - 
                                            hQC->channelConf[chanElement->channel[channel]]->averageBits +
                                            hQC->channelConf[chanElement->channel[channel]]->deltaBitres
                                            - (needAncBitsGran>>hQC->dualMonoFlag))));
       maxPart2_3Length[channel] = min(maxPart2_3Length[channel],
				       maxPart2_3LengthBitres);
       maxPart2_3LengthAll += maxPart2_3Length[channel];
     }
     else {
        /* condition for vbr mode */
       maxPart2_3LengthBitres = (int)floor(chBitDistribution[channel] *
					   (hQC->channelConf[chanElement->channel[channel]]->maxBits +
					    hQC->channelConf[chanElement->channel[channel]]->bitRes -
                        (needAncBitsGran>>hQC->dualMonoFlag)));
       /* ert + wmb: maxPart2_3LengthBitres always refers to the upper max limit (e.g. 320kbits for MPEG-1), therefore we subtract the ancbits there */
       maxPart2_3Length[channel] = min(maxPart2_3Length[channel],
				       maxPart2_3LengthBitres);
       maxPart2_3LengthAll += maxPart2_3Length[channel];
     }
  }

  qcOut->dynBitsUsed = 0;
  {

  int constraintsFulfilled = 1;
  int reIterate[2] = {0,0};
  iter = 0;
  do
  {

    constraintsFulfilled = 1;
    for (channel = 0; channel < hQC->nChannels; channel++)
    {

        qcOut->dynBitsUsedPerChannel[channel] = 0;
        /* now loop until bitstream constraints (p23Length < 4096,
           maxquant < 8192) are fulfilled */
        
      mp3QuantizeSpectrum(psyOut->psyOutChannel[channel].sfbActive,
                          psyOut->psyOutChannel[channel].sfbOffsets,
                          psyOut->psyOutChannel[channel].mdctSpectrum,
                          psyOut->psyOutChannel[channel].sfbThreshold,
                          psyOut->psyOutChannel[channel].sfbMaxSpec,
                          qcOut->globalGain[channel],
                          psyOut->psyOutChannel[channel].windowSequence,
                          qcOut->subblockGain[channel],
                          qcOut->preEmphasisFlag[channel],
                          qcOut->scfScale[channel],
                          maxScf[channel],
                          fullPsych,
                          qcOut->scf[channel],
                          qcOut->quantSpec[channel]);

      /*
        check how large the quantized values are.
      */
      if (calcMaxValueInSfb(psyOut->psyOutChannel[channel].sfbActive,
                            psyOut->psyOutChannel[channel].sfbOffsets,
                            qcOut->quantSpec[channel],
                            qcOut->maxValueInSfb[channel]) > MAX_QUANT)
        {
          constraintsFulfilled = 0;
          reIterate[channel] = 1;
        }

      if (!reIterate[channel])
        {
          /*
              Intensity processing.

              We need to make sure the decoder will not decode the
              upper zero part of the right-channel spectrum as intensity.

              So, if MS is active, we set the scalefactors there to the mid
              position, or to illegal position otherwise.
            */
#ifndef NO_INTENSITY
          if (channel == 1 && psyOut->isUsed)
            {
              static const int isIllegalPosition = 7;

              int nWindows = psyOut->psyOutChannel[channel].nWindows ;
              int win ;

              /*
                copy intensity coefficients into scalefactor array
              */

              for (win = 0 ; win < nWindows; win++)
                {
                  int isLimit = psyOut->psyOutChannel[channel].isLimit[win];

                  copyINTflex(psyOut->sfbIsPositions + isLimit * nWindows + win,
                              nWindows,
                              qcOut->scf[channel] + isLimit * nWindows + win,
                              nWindows,
                              psyOut->psyOutChannel[channel].sfbActive / nWindows - isLimit);

                  if (hQC->mpegVersion == MPEG1)
                    {
                      /*
                        MPEG 1 illegal intensity position handling:

                        if intensity is active,
                        starting from isLimit downward, set scalefactors of right
                        channel (=intensity coefficients) to illegalPos if the right
                        band is completely zero.
                        (If the left band is completely zero, set the scalefactor
                        to 0 to save bits by allowing a better scfCompress to be
                        found). Stop at first band that still contains energy.
                      */

                      for (sfb = (isLimit-1)*nWindows+win;
                           sfb >= 0 && qcOut->maxValueInSfb[channel][sfb] == 0;
                           sfb-= nWindows)
                        {
                          qcOut->scf[1][sfb] = qcOut->maxValueInSfb[0][sfb] ? isIllegalPosition : 0 ;
                        }
                    }
                }
            }
#endif   /*#ifndef NO_INTENSITY */

          /*
              clear out scalefactors above sfbActive
            */
          setINT(SCF_DONT_CARE, qcOut->scf[channel]+psyOut->psyOutChannel[channel].sfbActive,
                 psyOut->psyOutChannel[channel].sfbCnt-psyOut->psyOutChannel[channel].sfbActive);

          /*
              now find a better-than-default scf compression scheme
            */
          qcOut->scfCompress[channel] =
            (hQC->mpegVersion == MPEG1) ? 
            findScfCompressMPEG1(psyOut->psyOutChannel[channel].windowSequence,
                                 qcOut->scf[channel],
                                 qcOut->scfCntPerPartition[channel],
                                 qcOut->scfBitsPerPartition[channel]) :
            findScfCompressMPEG2((channel == 1 && psyOut->isUsed) ? 1 : 0,
                                 psyOut->psyOutChannel[channel].windowSequence,
                                 qcOut->scf[channel],
                                 qcOut->preEmphasisFlag[channel],
                                 qcOut->scfCntPerPartition[channel],
                                 qcOut->scfBitsPerPartition[channel],
                                 psyOut->psyOutChannel[channel].isLimit,
                                 psyOut->psyOutChannel[channel].sfbActive);


#ifndef NO_INTENSITY
          if (channel == 1 && psyOut->isUsed && hQC->mpegVersion != MPEG1)
            {
              int nWindows = psyOut->psyOutChannel[channel].nWindows ;
              int win ;

              for (win = 0 ; win < nWindows; win++)
                {
                  int isLimit = psyOut->psyOutChannel[channel].isLimit[win];
                  int part, offset;
                  /*
                    MPEG 2 illegal intensity position handling:

                    if intensity is active,
                    starting from isLimit downward, set scalefactors of right
                    channel (=intensity coefficients) to the maximum value allowed
                    by the compression scheme if the band in the right channel
                    is completely zero.
                    Stop at first band that still contains energy.
                  */

                  /* find the last band that contains energy */
                  for (sfb = (isLimit-1)*nWindows+win;
                       sfb >= 0 && qcOut->maxValueInSfb[channel][sfb] == 0;
                       sfb-= nWindows) ;

                  part = 0;
                  offset = 0;

                  for (sfb = sfb+nWindows; sfb < isLimit*nWindows; sfb+=nWindows)
                    {
                      int isIllegalPosition;

                      /* find the scf partition this band lies in */
                      while (offset+qcOut->scfCntPerPartition[channel][part] <= sfb)
                        {
                          offset += qcOut->scfCntPerPartition[channel][part];
                          part++;
                        }

                      isIllegalPosition = (1<<qcOut->scfBitsPerPartition[channel][part])-1;
                      qcOut->scf[1][sfb] = isIllegalPosition;
                    }
                }
            }
#endif   /*#ifndef NO_INTENSITY */

          /*
              count the bits and check for maximal p23Length
            */
          qcOut->blockType[channel] = psyOut->psyOutChannel[channel].windowSequence;

          qcOut->part2_3Length[channel] =
            dynBitCount(hQC->hBitCounter,
                        qcOut->quantSpec[channel],
                        (const unsigned int *) qcOut->maxValueInSfb[channel],
                        psyOut->psyOutChannel[channel].windowSequence,
                        psyOut->psyOutChannel[channel].sfbActive,
                        psyOut->psyOutChannel[channel].sfbOffsets,
                        &(qcOut->regionInfo[channel]))
            + scfBitCount(/*qcOut->scf[channel],
                            qcOut->scfCompress[channel],*/
                          qcOut->scfCntPerPartition[channel],
                          qcOut->scfBitsPerPartition[channel]) ;


          if(iter==0) {
            bitsFirst[channel] = qcOut->part2_3Length[channel];
          }

        } /* if constraintsFulfilled */
    if ( qcOut->part2_3Length[channel] > MP3_MAX_PART23_LENGTH ) {
          reIterate[channel] = 1;
          constraintsFulfilled = 0;
#ifndef NDEBUG
          fprintf(stderr, "\nWARNING: part2_3Length longer than 4092, extra quantization necessary\n");
#endif
       }
       part2_3LengthAll += qcOut->part2_3Length[channel];
    }
    
    if ( part2_3LengthAll >= maxPart2_3LengthAll ) {
      part2_3LengthAll = 0;
      constraintsFulfilled = 0;
#ifndef NDEBUG
      fprintf(stderr, "\nWARNING: part2_3Length too big, extra quantization necessary\n");
#endif
    }

    /* Usually one channel has too less bits, we increment its global gain */
    if ( !constraintsFulfilled ) {
      for (channel = 0; channel < hQC->nChannels; channel++) {
        if ( qcOut->part2_3Length[channel] >= maxPart2_3Length[channel] || reIterate[channel] || (iter>2) ) {
          /* Globalgain belongs to 16 bit/sample representation: Remove 60 when switching
             to float [-1:1] (9 lines beneath as well) */
          if(qcOut->globalGain[channel] == (256-(210 - PCM_CORRECTION)-1)) {
            /* zero for further encoding */
            setFLOAT(0.0f, psyOut->psyOutChannel[channel].mdctSpectrum, FRAME_LEN_LONG);
            setINT(0, qcOut->quantSpec[channel], FRAME_LEN_LONG);
            setINT(SCF_DONT_CARE, qcOut->scf[channel], MAX_SFB);
          }
          else { 
            qcOut->globalGain[channel]++;
            assert(qcOut->globalGain[channel] < 256-(210 - PCM_CORRECTION)) ;
          }
        }
	reIterate[channel] = 0;
      }
      part2_3LengthAll = 0;
    }
  
    iter++;
  } while(!constraintsFulfilled);

  for ( channel = 0; channel < hQC->nChannels; channel++)
    qcOut->dynBitsUsedPerChannel[channel] += qcOut->part2_3Length[channel];
  }
#ifndef OLD_THR_REDUCTION
  /* save dynBitsUsed for correction of bits2pe relation */
  {
    int cc;
    for (channel = 0; channel < hQC->nChannels; channel++)
      qcOut->dynBitsUsed+=qcOut->dynBitsUsedPerChannel[channel];
#ifdef NO_DUAL_MONO_FIXES
    for ( cc = 0 ; cc < chanElement->noOfChannelConfigurations; cc++ ) {
      AdjThrUpdate(hQC->hAdjThr[chanElement->channel[cc]], 
                   (qcOut->dynBitsUsed + (needAncBitsGran>>hQC->dualMonoFlag)));
    }
#else
    for ( cc = 0 ; cc < chanElement->noOfChannelConfigurations; cc++ ) {
      int bitsPerCEFirst = 0;
      for (channel = 0; channel < hQC->nChannels; channel++)
        bitsPerCEFirst+=bitsFirst[channel];

      AdjThrUpdate(hQC->hAdjThr[chanElement->channel[cc]], 
                   (bitsPerCEFirst + (needAncBitsGran>>hQC->dualMonoFlag)));
    }
#endif
  }
#endif

#ifdef PLOTMTV
  /*
    for debugging purposes calculate and plot the distortion
   */
  {
    int cc;

    setDeboutVars(-1,-1,0,-1);
    channel = 0;

    for (cc=0; cc<chanElement->noOfChannelConfigurations; cc++) {
      float bits2peShort  = 1.18f + 2.f;
      float bits2peLong   = 1.18f;
      int   usedBitsFirst = 0;
      int   dynBits       = 0;
      float pe = 0.f;
      int ch;

      setDeboutVars(-1,-1,-1,cc);
      for(ch=0; ch<(hQC->channelConf[cc]->nChannels); ch++) {
        usedBitsFirst += bitsFirst[channel];
        dynBits       += qcOut->dynBitsUsedPerChannel[channel];
        pe            += qcPsyOut->peData.peChannelData[channel].pe;
        channel++;
      }
      channel--;
      sendDeboutHist( "bitres", "bitsFirst",    MTV_INT, &usedBitsFirst );
      sendDeboutHist( "bitres", "dynBitsUsed",  MTV_INT, &dynBits );

      if(usedBitsFirst) {
        if(qcPsyOut->psyOut->psyOutChannel[channel].windowSequence==SHORT_WINDOW)
          bits2peShort = 2.f + pe/ (float)usedBitsFirst;
        else
          bits2peLong  =       pe/ (float)usedBitsFirst;
      }
      sendDeboutHist( "bits2pe", "bits2peShort",  MTV_FLOAT, &bits2peShort );
      sendDeboutHist( "bits2pe", "bits2peLong",   MTV_FLOAT, &bits2peLong  );

    }
  }

  for (channel = 0; channel < hQC->nChannels; channel++)
    {
    float dist[MAX_GROUPED_SFB];
    float iquaSpec[FRAME_LEN_LONG];

    mp3InvQuantizeSpectrum(psyOut->psyOutChannel[channel].sfbActive,
                        psyOut->psyOutChannel[channel].sfbOffsets,
                        qcOut->quantSpec[channel],
                        qcOut->globalGain[channel],
                        psyOut->psyOutChannel[channel].windowSequence,
                        qcOut->subblockGain[channel],
                        qcOut->preEmphasisFlag[channel],
                        qcOut->scfScale[channel],
                        qcOut->scf[channel],
                        iquaSpec);
    for (sfb = 0; sfb < psyOut->psyOutChannel[channel].sfbActive; sfb++)
    {
      int sfbOffset = psyOut->psyOutChannel[channel].sfbOffsets[sfb];
      int sfbWidth  = psyOut->psyOutChannel[channel].sfbOffsets[sfb+1] - sfbOffset;

      dist[sfb] = dist2FLOAT(iquaSpec + sfbOffset,
                             psyOut->psyOutChannel[channel].mdctSpectrum + sfbOffset,
                             sfbWidth);
    }
    setDeboutVars(-1,-1,channel,-1);
    sendDebout("estScalInp",psyOut->psyOutChannel[channel].sfbActive,1,"dist",MTV_FLOAT,dist);
  }
#endif

  /*
    now set all DONT_CARE scalefactors to zero
   */

  for (channel = 0; channel < hQC->nChannels; channel++)
  {
    for (sfb = 0; sfb < psyOut->psyOutChannel[channel].sfbCnt; sfb++)
    {
      if (qcOut->scf[channel][sfb] == SCF_DONT_CARE)
      {
        qcOut->scf[channel][sfb] = 0 ;
      }
    }
  }

  return 0; /* OK */
}

/*****************************************************************************

    functionname: needBitsForAnc
    description:  calculate bits for ancillary data and ofl
    return:       ancillary bits per granule to steal from QC

*****************************************************************************/
int needBitsForAnc(int ancBitsPerFrame,
                   int ancMode,
                   int* numAncDataBytes, 
                   unsigned int* writeOflOnce,
                   int granules )
{
  int needAncBits = 0;
  int needAncBitsGran = 0;

  if (numAncDataBytes != NULL) {
    if(*numAncDataBytes != 0) {

      needAncBits = ancBitsPerFrame;
    
      if (ancMode == 0) {
        /* RAW */
        if( (needAncBits/8) > *numAncDataBytes) {
          needAncBits = *numAncDataBytes * 8;
        } 
      }
      else if (ancMode == 1) {
        /* FhG */
        if( ((needAncBits - ANC_FHG_WRAPPING)/8) > *numAncDataBytes) {
          needAncBits = (*numAncDataBytes * 8) + ANC_FHG_WRAPPING;
        } 
      }
    }
  }

  if (*writeOflOnce > 0) {
    needAncBits += (OFL_V1_LEN * 8); 
  }

  needAncBitsGran = needAncBits / granules;

  assert((needAncBitsGran % 2) == 0);
  return needAncBitsGran;
}

/*****************************************************************************

    functionname: calcMaxValueInSfb
    description:  calculate the maximum quantized value per sfb and total
    returns:      the maximum quantized value.
    input:        the quantized mdct values
    output:       max quant value per sfb

*****************************************************************************/
static int calcMaxValueInSfb(const int sfbCnt,
                             const int sfbOffset[MAX_GROUPED_SFB],
                             const signed int quantSpectrum[FRAME_LEN_LONG],
                             int maxValue[MAX_GROUPED_SFB])
{
  int sfb;
  int maxValueAll = 0;

  for (sfb = 0; sfb < sfbCnt; sfb++)
  {
    int line;
    int maxThisSfb = 0;

    for (line = sfbOffset[sfb]; line < sfbOffset[sfb+1]; line++)
    {
      if (abs(quantSpectrum[line]) > maxThisSfb)
      {
        maxThisSfb = abs(quantSpectrum[line]);
      }
    }

    maxValue[sfb] = maxThisSfb;
    if (maxThisSfb > maxValueAll)
      maxValueAll = maxThisSfb;
  }
  return maxValueAll;
}


void updateBitres(struct QC_STATE *hQC,
                  int usedBits, int cc)
{
  hQC->channelConf[cc]->bitRes += usedBits;
}

int BitReservoir(struct QC_STATE *hQC, int cc)
{
  return hQC->channelConf[cc]->bitRes;
}

void resetDeltaBitres(struct QC_STATE *hQC, int cc)
{
   hQC->channelConf[cc]->deltaBitres = 0;
}

void updateDeltaBitres(struct QC_STATE *hQC, int dynBitsUsed, int cc)
{
   hQC->channelConf[cc]->deltaBitres -= 
     (dynBitsUsed - hQC->channelConf[cc]->averageBitsPerGranule);
}

