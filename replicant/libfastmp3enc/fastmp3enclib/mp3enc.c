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
*   $Id: mp3enc.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mconfig.h"
#include "mp3alloc.h"
#include "mp3enc.h"
#include "psy_const.h"
#include "psy_main.h"
#include "bitenc.h"
#include "interface.h"
#include "qc_main.h"
#include "framctrl.h"
#include "utillib.h"
#include "time_buffer.h"
#include "stprepro.h"
#include "qc_data.h"
#include "adj_thr.h"

#ifndef min 
#define min(a,b) ((a<b)?(a):(b))
#endif

#if defined IISMP3_USE_THREADS && defined _OPENMP
#include <omp.h>
#pragma message( "Compiling " __FILE__  " with OpenMP enabled!" )
#endif

enum
{
  ERROR_NSAMPLES = 1, /* wrong # of samples in input buffer */
  ERROR_BUFSIZE,      /* output buffer too small */
  ERROR_WRONGVERSION, /* wrong library version */
  ERROR_PANIC,        /* panic: we wrote more bytes than anticipated */
  ERROR_ANCILLARY     /* avoid negativ numAncDataBytes */
};

struct MP3_ENCODER
{
  /* private */

ALIGN_16_BYTE struct MP3ENC_CONFIG config;

  struct QC_STATE *qcKernel;
  struct QC_OUT   *qcOut[2]; /* for MPEG-1, we need two qcOut */

  struct PSY_INTERNAL *hpsy;

  struct PSY_OUT_LIST *qcPsyOut;
  struct PSY_OUT_LIST *firstPsyOut;
  struct PSY_OUT_LIST *lastPsyOut;

  struct FRAME_CONTROL *frameControl;

  struct BITSTREAM_ENCODER *bsEnc;
 
  CHANNEL_ELEMENT * channelElement ;
  
  /*
   * these are constant over the lifetime of one encoder instance
   */
  int hdSiBits ; /* number of bits for header and sideinfo */
  int granPerFrame ;
  int granPerMetaframe ;
  int cbBufsizeMin ;
  int nDelay ;   /* encoder + decoder delay */
  int noOfChannelConfigurations;

  /*
   * these are variable
   */
  float *pScratch ; /* needed for mono bitstreams */
  unsigned int psyGranuleCnt;
  int modeExtension ;
  int bitRateIndex;
  int paddingByte;
  int frameSize;
  int dynbitsInFrame;
  int nZeroesAppended; /* at end-of-file, count the number of zeroes
                          appended after end of client input */
  int psyZeroesAppended;
  int psyActive;

  int ancBitsPerFrame;        /* desired ancillary bits per frame */ 
  int ancRate;                /* ancillary rate */ 
  mp3_ancillary_mode ancMode; /* ancillary mode */
  int needAncBitsGran[2];     /* number of ancillary bits steal from QC */

  int oflOffset; /* begin of ofl data in byte from file begin */
  
} ;

/*---------------------------------------------------------------------------

    functionname:mp3encInit
    description: allocate global resources for the layer-3 library
    returns:     MP3ENC_OK if success

  ---------------------------------------------------------------------------*/
int  MP3ENCAPI mp3encInit
    (
    unsigned long needVersion, /* pass MP3ENC_VERSION to check compatibility */
    unsigned long *pVersion    /* contains library version on exit */
    )
{
  if (pVersion) *pVersion = MP3ENC_VERSION;
  return (MP3ENC_VERSION != needVersion) ? ERROR_WRONGVERSION : MP3ENC_OK;
}

/*---------------------------------------------------------------------------

    functionname:mp3encInitDefaultConfig
    description: initializes the MP3ENC_CONFIG to zero
    returns:     void

  ---------------------------------------------------------------------------*/
void MP3ENCAPI mp3encInitDefaultConfig(struct MP3ENC_CONFIG* pConfig)
{
  memset(pConfig, 0, sizeof(struct MP3ENC_CONFIG));
}


/*---------------------------------------------------------------------------

    functionname:mp3encOpen
    description: allocate and initialize a new encoder instance
    returns:     L3ENC_OK if success

  ---------------------------------------------------------------------------*/
int  MP3ENCAPI mp3encOpen
    (
    struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
    const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
    )
{
  int error;
  int i;
  struct MP3_ENCODER *hMp3enc = 0;

  /* sanity checks on config structure */
  error = (pConfig == 0              || phMp3Enc == 0             ||

           pConfig->nChannelsIn  < 1 || pConfig->nChannelsIn  > 2 ||
           pConfig->nChannelsOut < 1 || pConfig->nChannelsOut > 2 ||
           pConfig->nChannelsIn  < pConfig->nChannelsOut          ||

           (pConfig->fVbrMode &&
            (pConfig->vbrQuality < 0                              ||
             pConfig->vbrQuality > 100))                          ||

           (pConfig->useDualPass && pConfig->meanBitrate==320000) ||

           (!pConfig->fVbrMode &&

#ifndef MP3_SURROUND

            (pConfig->bitRate / pConfig->nChannelsOut < 8000      ||
             pConfig->bitRate < (pConfig->sampleRate >= 32000 ? 32000 : 8000) ||
             pConfig->bitRate > (pConfig->sampleRate >= 32000 ? 320000 : 160000)
            )

#else

			 ((pConfig->bitRate != 192000) &&
			 ((pConfig->sampleRate != 44100) ||
			  (pConfig->sampleRate != 48000))
			 )

#endif

           )
           || (pConfig->dualMonoFlag && pConfig->useMS )
           || (pConfig->meanBitrate && !pConfig->useDualPass)
          );

  /* check sample rate */

  if (!error)
  {
    switch (pConfig->sampleRate)
    {

#ifndef MP3_SURROUND

    case  8000: case 11025: case 12000:
    case 16000: case 22050: case 24000:
    case 32000: case 44100: case 48000:

#else

	case 44100: case 48000:

#endif

      break;
    default:
      error = 1; break;
    }
  }

  /* allocate encoder structure **********************************/
  if (!error)
  {
    hMp3enc = (struct MP3_ENCODER *) mp3Calloc(sizeof(struct MP3_ENCODER),1);
    error = (hMp3enc == 0);
  }

  if (!error)
  {
    hMp3enc->config = *pConfig ;

    /* currently, vbr is signalled as bitrate == 0 */
    if (pConfig->fVbrMode) hMp3enc->config.bitRate= 0;
    /* set number of granules to 2 for MPEG-1 */
    hMp3enc->granPerFrame = (pConfig->sampleRate >= 32000) ? 2 : 1;

    if( !hMp3enc->config.peStats )
      {
        hMp3enc->config.peStats = (struct PE_STATS *)mp3Calloc(sizeof(struct PE_STATS), 1);
        if( !hMp3enc->config.peStats )
          error = 1;
        else
	  PeStatsInit( hMp3enc->config.peStats, 1.0f, (!pConfig->meanBitrate&&pConfig->vbrQuality&&!pConfig->useDualPass) );
      }
  }

  /* initializing frame rate control **************************/

  if (!error)
  {
    error = FCNew(&(hMp3enc->frameControl));
  }

  if (!error)
  {
      error = FCInit(hMp3enc->frameControl,
                     pConfig->bitRate ,
                     pConfig->sampleRate,
                     pConfig->paddingMode);
  }
  
  /* initializing bitstream encoder ****************************/

  if (!error)
    error = BSNew(&(hMp3enc->bsEnc));

  if (!error)
  {
    struct BITSTREAMENCODER_INIT bsInit;
    
    if ( pConfig->nChannelsOut == 1 ) {
      bsInit.channelMode = SINGLE_CHANNEL;
    }
    else {
      if ( !pConfig->useMS && pConfig->fNoIntensity && pConfig->dualMonoFlag ) {
        bsInit.channelMode = DUAL_CHANNEL;
      }
      else if ( !pConfig->useMS && pConfig->fNoIntensity && !pConfig->dualMonoFlag ) {
        bsInit.channelMode = STEREO;
      }
      else {
        bsInit.channelMode = JOINT_STEREO;
      }
    }

    bsInit.bitrate     = pConfig->bitRate;
    bsInit.sampleRate  = pConfig->sampleRate;
    bsInit.protection  = pConfig->fCrc;

    bsInit.privateBit      = pConfig->privateBit;
    bsInit.copyRightBit    = pConfig->copyRightBit;
    bsInit.originalCopyBit = pConfig->originalCopyBit;
    bsInit.ancMode         = pConfig->ancillary.anc_Mode;

    error = BSInit(hMp3enc->bsEnc, &bsInit);
  }

  if (!error)
    hMp3enc->hdSiBits = GetHdSiBits(hMp3enc->bsEnc) ;


  /* initializing psychoacoustic **********************************/

  if (!error)
  {
    struct PSY_INIT psyInit;

    if (pConfig->bandWidth == 0)
    {
      float bandWidth = 0.0f ;

      /*
        if bandwidth has been left at zero, find a default
      */

      if (pConfig->bitRate != 0 || pConfig->meanBitrate!=0 ) /* constant bit rate ? */
      {
        int bitRate = (pConfig->bitRate ? pConfig->bitRate : pConfig->meanBitrate) - pConfig->ancillary.anc_Rate;
        switch (pConfig->nChannelsOut)
        {
        case 1: /* Mono */
          bandWidth = 0.182f * bitRate + 1810.0F;
          break;

        case 2: /* Stereo */
          if (bitRate < 32000)
          {
            /*
              lowest bitrates get different treatment. We prefer bandwidth
              over "no" coding artefacts.
            */
            bandWidth = 0.1125F * bitRate + 1600.0F ;
          }
          else
          {
            int bitsPerFrame;
            float fpFac = 1.75f, dpFac = 1.85f, fac = 0.0f;
            (pConfig->useDualPass)?(fac = fpFac):(fac=dpFac);
            /* calculate how many bits we can spend for spectrum + scalefactors.
               Assume a constant ratio of ~1.5 bits per line. */
            bitsPerFrame = bitRate * FRAME_LEN_LONG *
                           hMp3enc->granPerFrame / hMp3enc->config.sampleRate;
            bandWidth = (bitsPerFrame - hMp3enc->hdSiBits) /
                        (2 * hMp3enc->granPerFrame *
                         (pConfig->sampleRate >= 32000 ? 
                          fac : 1.39f ));
            bandWidth *= 0.5f*pConfig->sampleRate / 576.0f;
            if ( pConfig->useDualPass ) {
              /* limit to 20 kHz */
              if (bandWidth > 20000.0 )
                bandWidth = 20000.0;
            }             
          }
          break;
        } /* end of switch (nChannelsOut) */
      }
      else
      {
        /* variable bit rate */
        float factor = (pConfig->vbrQuality > 70) ? 0.38f : 0.32f;
#if 0
        float factor = min( pConfig->vbrQuality, 75.0f );
        if( pConfig->sampleRate > 32000 )
          factor = 0.25f * factor / 75.0f + 0.15f;
        else
          factor = 0.25f * factor / 25.0f + 0.25f;
#endif
        bandWidth = factor * pConfig->sampleRate ;
      }

      if (bandWidth > (0.5f * pConfig->sampleRate))
        bandWidth = 0.5f * pConfig->sampleRate ;

      hMp3enc->config.bandWidth = bandWidth ;
    }

    psyInit.sampleRate   = pConfig->sampleRate;

    if (pConfig->bitRate <= pConfig->ancillary.anc_Rate )
      psyInit.bitRate = 0;
    else
      psyInit.bitRate      = pConfig->bitRate - pConfig->ancillary.anc_Rate;


    psyInit.nChannels    = pConfig->nChannelsOut;
    psyInit.bandWidth    = hMp3enc->config.bandWidth;
    psyInit.noIntensity  = pConfig->fNoIntensity;
    psyInit.useDualPass  = pConfig->useDualPass;
    psyInit.pcmResolution = pConfig->pcmResolution;


    if ( pConfig->meanBitrate <= pConfig->ancillary.anc_Rate )
      psyInit.meanBitrate = 0;
    else
      psyInit.meanBitrate  = pConfig->meanBitrate - pConfig->ancillary.anc_Rate;

    psyInit.useMS       = pConfig->useMS ; 

    error = CreatePsyList( &hMp3enc->firstPsyOut,
                           &hMp3enc->lastPsyOut,
                           &hMp3enc->qcPsyOut,
                           pConfig,
                           pConfig->predictGranules ?
#ifndef IISMP3_USE_THREADS
                           pConfig->predictGranules : 1
#else
                           pConfig->predictGranules : 3
#endif
                           );

    error = ( error
              || PsyNew(&(hMp3enc->hpsy), &psyInit)
              || PsyInit(hMp3enc->hpsy,&psyInit) );

  }

  /*
    initializing quantization and coding
  */

  /* allocate the Q&C Out structure */
  if (!error)
  {
    for (i = 0; i < hMp3enc->granPerFrame; i++)
    {
      if (!error)
        error = QCOutNew(&(hMp3enc->qcOut[i]));
      if (!error)
        error = QCOutInit(hMp3enc->qcOut[i]);
    }
  }

  if(!error) 
  {
    if (pConfig->nChannelsOut == 1)
    {
       hMp3enc->pScratch = (float *) mp3Alloc(sizeof(float)*2*FRAME_LEN_LONG);
    } 
    else { 
       hMp3enc->pScratch = NULL;
    }
  }

  if (!error)
  {
    /* allocate the Q&C kernel */
    error = QCNew(&(hMp3enc->qcKernel));
  }

  if (!error)
    {
      int no = (pConfig->dualMonoFlag ? 2 : 1) ;
      error = CENew(&(hMp3enc->channelElement));
      InitCE ( hMp3enc->channelElement, no );
    }

  if (!error)
  {
    struct QC_INIT qcInit;

    /* for ancillary data */   
    int diffToByteAlign = 0;
    /* average number of bits per frame */
    int bitsPerFrame = 0;
    /* the maximum number of frames we can point back */
    int backFrames;
    int brLow = 0;
    /* the maximum backpointer at this bitrate / samplerate */
    int maxbp;
    int dynBitsMin = 0;
    /* for bitrate switching, find the lower bitrate */
    int brHigh = 0;

    bitsPerFrame = hMp3enc->config.bitRate * FRAME_LEN_LONG *
                       hMp3enc->granPerFrame / hMp3enc->config.sampleRate;

    /* ert + wmb: ensure byte alignment, otherwise bitrateindex may exceed 320kbits */
    bitsPerFrame -= bitsPerFrame % 8;
    
    brHigh = FCFindFit(hMp3enc->frameControl, bitsPerFrame, NULL,0);
    
    /* ert + wmb: FCFindFit returns -1 if the bitrateIndex is out of Range */
    if (brHigh == -1){
        error = 1;
        mp3encClose(hMp3enc);
        hMp3enc = 0;
       *phMp3Enc = hMp3enc;
        return (error);
    }
    
    brLow = (FCSize(hMp3enc->frameControl, brHigh) > bitsPerFrame) ?
                 brHigh - 1 : brHigh;
    
    /* number of dynbits in lower-br frames */
    dynBitsMin = FCSize(hMp3enc->frameControl, brLow) - hMp3enc->hdSiBits;

    qcInit.useDualPass     = pConfig->useDualPass;
    qcInit.dualPassLoop    = pConfig->dualPassLoop;
    qcInit.predictGranules = pConfig->predictGranules;
    qcInit.sampleRate      = pConfig->sampleRate;
    qcInit.granPerFrame    = hMp3enc->granPerFrame;
    qcInit.peStats         = hMp3enc->config.peStats;

    if (hMp3enc->config.fVbrMode == 0)
    {
      maxbp = hMp3enc->granPerFrame * 256 - 1;
      if (maxbp * 8 + FCSize(hMp3enc->frameControl, brLow) > 7680)
      {
        maxbp = (7680 - FCSize(hMp3enc->frameControl, brLow))/8;
        if (maxbp < 0)
          maxbp = 0;
      }

      /* the maximum number of frames we can point back */
      backFrames = (maxbp*8+dynBitsMin-1) / dynBitsMin;

      /* the most extreme situation is if all of the bitreservoir is
         flushed at once, the first frame is of a higher bitrate, and this
         one is, too (and all of these have the padding bit on). */
      hMp3enc->cbBufsizeMin = ((backFrames-1)*(FCSize(hMp3enc->frameControl, brLow)/8+1)
                               + 2*(FCSize(hMp3enc->frameControl, brHigh)/8+1));
      hMp3enc->granPerMetaframe = hMp3enc->granPerFrame * FCFramesInMeta(hMp3enc->frameControl);

      qcInit.maxBits     = (FCSize(hMp3enc->frameControl, brLow)
                            - hMp3enc->hdSiBits
                            + 8 * maxbp);

      qcInit.averageBits = ((pConfig->bitRate * FRAME_LEN_LONG) / 
                            pConfig->sampleRate * hMp3enc->granPerFrame 
                            - hMp3enc->hdSiBits); 

      if (qcInit.averageBits >  qcInit.maxBits)
        qcInit.averageBits =  qcInit.maxBits;

      qcInit.averageBitsPerGranule = qcInit.averageBits / hMp3enc->granPerFrame ;
      qcInit.desiredMeanBitrate = 0;
    }
    else
    {
      /* VBR mode */
      int maxBitrateIndex = /* hMp3enc->frameControl->maxBitrateIndex-1 */ (pConfig->sampleRate < 16000?8:14) ;
      hMp3enc->cbBufsizeMin = 2*FCSize(hMp3enc->frameControl,maxBitrateIndex)/8;
      hMp3enc->granPerMetaframe = hMp3enc->granPerFrame;
      /* in vbr mode maxBits are the maximum allowed bits per granule */
      qcInit.maxBits     = (FCSize(hMp3enc->frameControl, maxBitrateIndex ) - 
                            hMp3enc->hdSiBits) / hMp3enc->granPerFrame;
      qcInit.desiredMeanBitrate = pConfig->meanBitrate;

      hMp3enc->qcKernel->hAdjThr[0]->desiredMeanBitrate = pConfig->meanBitrate;
      hMp3enc->qcKernel->hAdjThr[0]->desiredMeanPe =
        bits2pe( ( pConfig->meanBitrate * hMp3enc->granPerFrame
                   * FRAME_LEN_LONG / (float)pConfig->sampleRate
                   - hMp3enc->hdSiBits )
                 / (float)hMp3enc->granPerFrame );

      qcInit.averageBits = 0;
      qcInit.averageBitsPerGranule = 0;
    }
    qcInit.vbrQuality = pConfig->vbrQuality;

    hMp3enc->nDelay = GetBlockSwitchingOffset(hMp3enc->hpsy) + FRAME_LEN_LONG + 2*POLY_PHASE_DELAY + 1;
    hMp3enc->nZeroesAppended = 0;
    hMp3enc->psyZeroesAppended = 0;
    hMp3enc->psyActive = 1;
    hMp3enc->psyGranuleCnt = 0;
    hMp3enc->qcKernel->granuleCnt = 0;

    qcInit.nChannels   = pConfig->nChannelsOut;
    qcInit.bitresStart = 0;

    qcInit.mpegVersion = pConfig->sampleRate >= 32000 ?
                         MPEG1 : (pConfig->sampleRate >=16000 ?
                         MPEG2 : MPEG2_5);

#ifndef OLD_THR_REDUCTION
    qcInit.meanPe = 10.0f * FRAME_LEN_LONG * 
                    hMp3enc->config.bandWidth/(pConfig->sampleRate/2.0f);
#endif


    if ( pConfig->bitRate <= pConfig->ancillary.anc_Rate ) /* VBR case: bitRate = 0 */
      qcInit.channelBitrate = 0;
    else
      qcInit.channelBitrate = (pConfig->bitRate - pConfig->ancillary.anc_Rate) / pConfig->nChannelsOut ;
   

    qcInit.fullHuffmanSearch = pConfig->fFullHuffman ;

    qcInit.dualMonoFlag = pConfig->dualMonoFlag ;

    /* calculate desired ancillary bits per frame */
    if ( pConfig->ancillary.anc_Rate > 0 ) {
      hMp3enc->ancBitsPerFrame = ( pConfig->ancillary.anc_Rate *  
                                   FRAME_LEN_LONG *
                                   hMp3enc->granPerFrame ) / hMp3enc->config.sampleRate;
    } else {
      hMp3enc->ancBitsPerFrame = ANC_BITS_DEFAULT;
    }
    diffToByteAlign = hMp3enc->ancBitsPerFrame % 8;
    hMp3enc->ancBitsPerFrame = hMp3enc->ancBitsPerFrame - diffToByteAlign ; 
    hMp3enc->ancMode = pConfig->ancillary.anc_Mode;
    hMp3enc->ancRate = pConfig->ancillary.anc_Rate;
    if ( pConfig->ancillary.anc_Mode == ANC_FHG_MODE ) {
      hMp3enc->ancBitsPerFrame += ANC_FHG_WRAPPING;    
    }   
   
    hMp3enc->noOfChannelConfigurations = (qcInit.dualMonoFlag ? 2 : 1);

    error = QCInit(hMp3enc->qcKernel, &qcInit);
  }

  /* ENABLE PROCESSOR SPECIFIC OPTIMIZATIONS */ 
#define _USE_ALTIVEC
#if defined __APPLE_CC__ && defined __ppc__ && defined _USE_ALTIVEC
  extern void initAltivec();
  initAltivec();
#endif

#if defined P4_INTRINSIC && !defined P4_CODE
  extern void initSSE();
  initSSE();
#endif

  if (error)
  {
    mp3encClose(hMp3enc);
    hMp3enc = 0;
  }

  *phMp3Enc = hMp3enc;
  return (error);
}

void MP3ENCAPI mp3encGetDualPassState
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
    )
{
  *peStats = (void *)hMp3enc->config.peStats;
}

/*---------------------------------------------------------------------------

    functionname:mp3encClose
    description: deallocate an encoder instance

  ---------------------------------------------------------------------------*/
void MP3ENCAPI mp3encClose
    (
    struct MP3_ENCODER*        hMp3enc  /* an encoder handle */
    )
{
  if (hMp3enc)
  {
    int i ;

    /** Delete peStats */
    if ( hMp3enc->config.peStats && ((hMp3enc->config.dualPassLoop == 0)||(hMp3enc->config.dualPassLoop >= 2)) ) {
      mp3Free(hMp3enc->config.peStats);
      hMp3enc->config.peStats = 0;
    }

    /* free QCOut */
    for (i = 0; i < hMp3enc->granPerFrame; i++)
      QCOutDelete(hMp3enc->qcOut[i]);

    /* free Q&C kernel */
    QCDelete(hMp3enc->qcKernel);

    /* free channelElement */
    CEDelete(hMp3enc->channelElement);

    /* free Bitstream encoder */
    BSDelete(hMp3enc->bsEnc);

    /* free psychoacoustic and psych->quant interface */
    PsyDelete(hMp3enc->hpsy);

    /* free psyOut list */
    DelPsyList( hMp3enc->firstPsyOut );

    /* free bitrate control */
    FCDelete(hMp3enc->frameControl);

    /* free helper input buffer */
    if (hMp3enc->pScratch)
      mp3Free(hMp3enc->pScratch);

    /* free encoder struct itself */
    mp3Free(hMp3enc);
  }
}

/*---------------------------------------------------------------------------

    functionname:mp3encFlush
    description: calls BSFlush

  ---------------------------------------------------------------------------*/
void MP3ENCAPI mp3encFlush(struct MP3_ENCODER*  hMp3enc, 
                           unsigned char *pOutput,
                           int *outBytes)
{
  BSFlush(hMp3enc->bsEnc, pOutput, outBytes);
}

/*---------------------------------------------------------------------------

    functionname:mp3encDone
    description: release all global resources

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encDone
    (
    void
    )
{
  /* nothing to do */
}

/* sanity check */
#if MP3ENC_BLOCKSIZE != FRAME_LEN_LONG
#error this code only works for MP3ENC_BLOCKSIZE == FRAME_LEN_LONG
#endif

/*---------------------------------------------------------------------------

    functionname:mp3encEncode
    description: encode BLOCKSIZE*nChannels samples
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/
int  MP3ENCAPI mp3encEncode
    (
    struct MP3_ENCODER * const hMp3enc,  /* an encoder handle */
    const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
    const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
    unsigned char* const       pOutput,  /* pointer to bitstream buffer; contains complete frames only */
    int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
    int* const                 cbOut,    /* number of bytes in bitstream buffer */   
    unsigned char*             ancDataBytes,    /* ancillary data */
    int*                       numAncDataBytes, /* ancillary bytes left */
    unsigned int*              writeOflOnce     /* if ofl claimed, write it once */
    )
{
  int i , cc;
  int granule;
  int ancBitsToWrite=0;
  struct PSY_OUT *psyOut;
  struct PSY_OUT *qcPsyOut;

  /*
   * sanity checks
   */

  if (nSamples > hMp3enc->config.nChannelsIn * FRAME_LEN_LONG ||
      nSamples % hMp3enc->config.nChannelsIn != 0)
  {
    return ERROR_NSAMPLES ;
  }

  if (cbSize < hMp3enc->cbBufsizeMin)
  {
    return ERROR_BUFSIZE;
  }

  if (numAncDataBytes != NULL) {
    if (*numAncDataBytes < 0) {
      return ERROR_ANCILLARY;
    }
  }

  *cbOut = 0;

  /* 
     Try to avoid a counter overflow in  hMp3enc->psyGranuleCnt as well as
     in hMp3enc->qcKernel->granuleCnt by subtracting one from the other
     ATTENTION: This does not work for dual pass mode !!!
  */
  if ( hMp3enc->qcKernel->granuleCnt >= (unsigned int) hMp3enc->granPerFrame ) {
    assert( hMp3enc->psyGranuleCnt >= hMp3enc->qcKernel->granuleCnt );
    if(hMp3enc->psyGranuleCnt>=hMp3enc->qcKernel->granuleCnt) {
      int decrement = 
	hMp3enc->qcKernel->granuleCnt - (hMp3enc->qcKernel->granuleCnt%hMp3enc->granPerFrame);

      hMp3enc->psyGranuleCnt        -= decrement;
      hMp3enc->qcKernel->granuleCnt -= decrement;
    }
  }

  if ( 0 == nSamples )
    hMp3enc->psyActive = 0;

  if (hMp3enc->bsEnc && mp3GetFrameStats (hMp3enc->bsEnc))
    mp3GetFrameStats (hMp3enc->bsEnc) [0] = 0 ;

#ifndef IISMP3_USE_THREADS
  if( hMp3enc->config.predictGranules )
#endif
    {
      FeedPsyList( &hMp3enc->firstPsyOut,
                   &hMp3enc->lastPsyOut,
                   &hMp3enc->qcPsyOut );
    }
  psyOut = hMp3enc->firstPsyOut->psyOut;
  hMp3enc->firstPsyOut->psyOutValid = 0;

#if defined IISMP3_USE_THREADS && defined _OPENMP
  if ( 2 <= omp_get_num_procs() )
    omp_set_num_threads(2);
  else
    omp_set_num_threads(1);

#pragma omp parallel sections private (granule)
  {
#endif
#if defined IISMP3_USE_THREADS && defined _OPENMP
#pragma omp section
    {
#endif
  /*******************************
   *        Psychoacoustic       *
   *******************************/
  psyOut->nSamples = nSamples;

  if( hMp3enc->psyActive )
    {
      /*
       * feed input into the decoder kernel. Channel conversion is done here.
       */
      switch(hMp3enc->config.nChannelsIn)
        {
        case 1:
          /* expand input signal to two channels (the encoder takes interleaved
             data only) */
          for (i=0; i < nSamples; i++)
            {
              hMp3enc->pScratch[i*2] = pSamples[i];
            }

          /* clear out until end-of-buffer */
          for (; i < FRAME_LEN_LONG; i++)
            {
              hMp3enc->pScratch[i*2] = 0.0f;
            }

          psyFeedInputBufferFloat(hMp3enc->hpsy, (float(*)[2])hMp3enc->pScratch, FRAME_LEN_LONG);
          break;

        case 2:
          if (hMp3enc->config.nChannelsOut == 1)
            {
              /* downmix stereo signal to mono */
              for (i=0; i<nSamples / 2; i++)
                {
                  hMp3enc->pScratch[2*i] = 0.5f*(pSamples[2*i] + pSamples[2*i+1]);
                }

              /* clear out until end-of-buffer */
              for (; i < FRAME_LEN_LONG; i++)
                {
                  hMp3enc->pScratch[i*2] = 0.0f;
                }
              
              psyFeedInputBufferFloat(hMp3enc->hpsy, (float(*)[2])hMp3enc->pScratch, FRAME_LEN_LONG);
            }
          else
            {
              /* feed samples directly */
              psyFeedInputBufferFloat(hMp3enc->hpsy, (float(*)[2])pSamples, nSamples/2);
              /* clear out until end-of-buffer */
              for (i = 0; i < 2*FRAME_LEN_LONG-nSamples; i+=2)
                {
                  static const float x[2] = {0.0f,0.0f};
                  psyFeedInputBufferFloat(hMp3enc->hpsy, (float(*)[2])x, 1);
                }
            }
        }

      /*
       * count encoder appended silence (flushing)
       */
      if (psyOut->nSamples < hMp3enc->config.nChannelsIn * FRAME_LEN_LONG)
        {
          hMp3enc->psyZeroesAppended += FRAME_LEN_LONG - psyOut->nSamples / hMp3enc->config.nChannelsIn;
        }
      
      granule = hMp3enc->psyGranuleCnt % hMp3enc->granPerFrame;
      
      AdvanceStereoPreStep1( psyOut->hStereoPrePro, hMp3enc->config.nChannelsOut,
                             psyGetInputBufferFloat(hMp3enc->hpsy),
                             MP3ENC_BLOCKSIZE );

      /*
        advance psychoacoustic
      */
      assert( hMp3enc->firstPsyOut->psyOut );
      psyMain(hMp3enc->hpsy, granule, hMp3enc->firstPsyOut, (hMp3enc->channelElement->noOfChannelConfigurations == 2 ? 1 : 0 ) );
      hMp3enc->firstPsyOut->psyOutValid = 1;

      hMp3enc->psyGranuleCnt++;

      hMp3enc->firstPsyOut->peData.redPe = hMp3enc->firstPsyOut->peData.pe;


#ifdef IISMP3_USE_THREADS
      /* Save Ancillary Data Bytes with actual psych data to multiplex them time aligned 
         with input signal into the bitstream later*/
      if ( (numAncDataBytes != NULL) && (ancDataBytes != NULL) ) {
        if(*numAncDataBytes>0) {
          int ancBytes = min(*numAncDataBytes, hMp3enc->ancBitsPerFrame/8);
          hMp3enc->firstPsyOut->numAncDataBytes = *numAncDataBytes;
          memcpy(hMp3enc->firstPsyOut->ancDataBytes, ancDataBytes, ancBytes);
          if (granule == hMp3enc->granPerFrame-1) {
            *numAncDataBytes -= hMp3enc->firstPsyOut->numAncDataBytes;
          }
        }
        else {
          hMp3enc->firstPsyOut->numAncDataBytes = 0;
        }
      }
#endif


      /* check if psychoacoustic is done */
      if( !nSamples &&
          ( hMp3enc->psyZeroesAppended >= hMp3enc->nDelay  &&
            !(hMp3enc->psyGranuleCnt % hMp3enc->granPerMetaframe) ) )
        hMp3enc->psyActive = 0;


      if( hMp3enc->config.meanBitrate && hMp3enc->config.dualPassLoop < 2 )
        {
          PeStatsAdd( hMp3enc->config.peStats,
		      hMp3enc->firstPsyOut->peData.noRedPe,
		      hMp3enc->firstPsyOut->windowSequence );

#ifndef IISMP3_USE_THREADS
          if( hMp3enc->config.dualPassLoop == 1 ) /* for dual-pass coding */
            {
              if( hMp3enc->psyActive )
                return MP3ENC_OK;
              else
                return MP3ENC_READY;  /* encoding done (first pass) */
            }
#endif

        }
    }
#if defined IISMP3_USE_THREADS && defined _OPENMP
    }  /* End Open MP section 1 - psych thread */
#pragma omp section
    {
#endif

  qcPsyOut = hMp3enc->qcPsyOut->psyOut;

  /*******************************
   *          Quantize           *
   *******************************/
  if( hMp3enc->qcPsyOut->psyOutValid )
  {
    /*
     * count encoder appended silence (flushing)
     */
    if (qcPsyOut->nSamples < hMp3enc->config.nChannelsIn * FRAME_LEN_LONG)
    {
      hMp3enc->nZeroesAppended += FRAME_LEN_LONG - qcPsyOut->nSamples / hMp3enc->config.nChannelsIn;
    }

    granule = hMp3enc->qcKernel->granuleCnt % hMp3enc->granPerFrame;

    if (granule == 0)
    {
      hMp3enc->modeExtension = 0;
      hMp3enc->dynbitsInFrame = 0;

      /* put this frame's bits (not counting static bits)
         into the bit reservoir (only if we are not running VBR) */
      if (!hMp3enc->config.fVbrMode)
      {
        FCAdvance(hMp3enc->frameControl);

        hMp3enc->bitRateIndex   = FCBitrateIdx(hMp3enc->frameControl);
        hMp3enc->paddingByte    = FCNuansdroByte(hMp3enc->frameControl);
        hMp3enc->frameSize      = FCSize(hMp3enc->frameControl, hMp3enc->bitRateIndex)
                                  + 8*hMp3enc->paddingByte;
        if ( hMp3enc->channelElement->noOfChannelConfigurations == 2 ){
          int bits = 0 ;
       
          bits = (int) (hMp3enc->frameSize* 0.5f - hMp3enc->hdSiBits* 0.5f) ;

          for (cc = 0 ; cc < hMp3enc->channelElement->noOfChannelConfigurations ; cc++ ){
            if ( cc == 1 && ( (float)hMp3enc->frameSize* 0.5  - (float)hMp3enc->hdSiBits* 0.5 -bits >= 0.5 ))
              bits++;
            updateBitres(hMp3enc->qcKernel, bits , cc);
            resetDeltaBitres(hMp3enc->qcKernel, cc);
          }
        }
        else {
          updateBitres(hMp3enc->qcKernel, hMp3enc->frameSize - hMp3enc->hdSiBits , 0);
          resetDeltaBitres(hMp3enc->qcKernel, 0);
        }
      }
    }

    if (hMp3enc->config.nChannelsOut == 2)
    {
      hMp3enc->modeExtension |= (qcPsyOut->msUsed ? 2 : 0);
      hMp3enc->modeExtension |= (qcPsyOut->isUsed ? 1 : 0);
    }

    AdvanceStereoPreStep2( qcPsyOut, qcPsyOut->hStereoPrePro );

    hMp3enc->needAncBitsGran[granule] = 
      needBitsForAnc(hMp3enc->ancBitsPerFrame,
                     hMp3enc->ancMode,
#ifndef IISMP3_USE_THREADS
                     numAncDataBytes,
#else
                     &hMp3enc->qcPsyOut->numAncDataBytes,
#endif
                     writeOflOnce,
                     hMp3enc->granPerFrame);
   
    /* advance quantizer */
    QCMain(hMp3enc->qcKernel,
           /*hMp3enc->firstPsyOut,*/
           hMp3enc->qcPsyOut,
           hMp3enc->lastPsyOut,
           hMp3enc->qcOut[granule],
           hMp3enc->channelElement,
           hMp3enc->needAncBitsGran[granule]);

    if ( hMp3enc->channelElement->noOfChannelConfigurations == 2 ) {
      for ( cc = 0; cc < hMp3enc->channelElement->noOfChannelConfigurations; cc++){
        hMp3enc->dynbitsInFrame += (hMp3enc->qcOut[granule]->dynBitsUsedPerChannel[cc] +
                                    (hMp3enc->needAncBitsGran[granule]>>1));
        updateDeltaBitres(hMp3enc->qcKernel, 
                          (hMp3enc->qcOut[granule]->dynBitsUsedPerChannel[cc] +
                           (hMp3enc->needAncBitsGran[granule]>>1)), 
                          cc);
      }
    }
    else {
      hMp3enc->dynbitsInFrame += (hMp3enc->qcOut[granule]->dynBitsUsed +
                                  hMp3enc->needAncBitsGran[granule]);
      updateDeltaBitres(hMp3enc->qcKernel, 
                        (hMp3enc->qcOut[granule]->dynBitsUsed + 
                         hMp3enc->needAncBitsGran[granule]), 
                        0);
    }

    if (granule == hMp3enc->granPerFrame-1)
    {
      /* we have just written the last granule in this frame. Call the
         bitstream encoder and return encoded data. */

      /* determine the mode extension for this frame */
      int actualFrameBits;
      int bitsPerFrameUsed[2] = {0,0} ;

      /* calculate ancillary bits to write in bitstream encoder */
      if (hMp3enc->granPerFrame == 2) {
        ancBitsToWrite = hMp3enc->needAncBitsGran[0] + hMp3enc->needAncBitsGran[1];
        ancBitsToWrite -= (ancBitsToWrite % 8); 
      } 
      else {
        ancBitsToWrite = hMp3enc->needAncBitsGran[0];
      }  
     
      /* if we are running in VBR mode, find out the number of bits
         in this frame. */
      if (hMp3enc->config.fVbrMode)
	{
        hMp3enc->bitRateIndex = FCFindFit(hMp3enc->frameControl,
                                          hMp3enc->dynbitsInFrame
                                          - BitReservoir(hMp3enc->qcKernel, 0)
                                          + hMp3enc->hdSiBits,
                                          &ancBitsToWrite, hMp3enc->config.useDualPass);
       /* ert + wmb: FCFindFit returns -1 if the bitrateIndex is out of Range */
        if (hMp3enc->bitRateIndex == -1){
#ifndef IISMP3_USE_THREADS
           return ERROR_BUFSIZE;
#else
        assert(0);
#endif
        }
        hMp3enc->paddingByte  = 0;
      }

      /* now we can call the bitstream encoder */
      actualFrameBits =
        BSWrite(hMp3enc->bsEnc,
                hMp3enc->qcOut,
                hMp3enc->bitRateIndex,
                hMp3enc->paddingByte,
                hMp3enc->modeExtension,
                pOutput, cbOut,
                hMp3enc->ancMode,                
#ifndef IISMP3_USE_THREADS
                ancDataBytes,
                numAncDataBytes,
#else
                hMp3enc->qcPsyOut->ancDataBytes,
                &hMp3enc->qcPsyOut->numAncDataBytes,
#endif
                bitsPerFrameUsed,
                &hMp3enc->oflOffset,
                writeOflOnce, 
                ancBitsToWrite
                );

#ifndef IISMP3_USE_THREADS
      if (numAncDataBytes != NULL) {
        if (*numAncDataBytes < 0) {
          return ERROR_ANCILLARY;
        }
      }
#endif

      /*
        take written bits (not counting static bits) from bitreservoir.
        If we are running VBR, put this frame's bits into it as well.
      */
      if ( hMp3enc->channelElement->noOfChannelConfigurations == 2 ) 
        for ( cc = 0; cc < hMp3enc->channelElement->noOfChannelConfigurations; cc++){
          updateBitres(hMp3enc->qcKernel, (-bitsPerFrameUsed[cc]), cc);
        }
      else
        updateBitres(hMp3enc->qcKernel, -(actualFrameBits - hMp3enc->hdSiBits), 0);

      hMp3enc->qcKernel->hAdjThr[0]->usedTotalBits += actualFrameBits;
      hMp3enc->qcKernel->hAdjThr[0]->desiredTotalBits +=
        hMp3enc->qcKernel->hAdjThr[0]->desiredFrameBits + hMp3enc->hdSiBits;
      hMp3enc->qcKernel->hAdjThr[0]->desiredFrameBits = 0;

      if (hMp3enc->config.fVbrMode)
      {
        updateBitres(hMp3enc->qcKernel,
                     FCSize(hMp3enc->frameControl, hMp3enc->bitRateIndex)
                     - hMp3enc->hdSiBits, 0);
      }
    } /* if last granule in frame */

    hMp3enc->qcKernel->granuleCnt++;
  }
#if defined IISMP3_USE_THREADS && defined _OPENMP
    }
  }
/* #pragma omp barrier */ /* all threads stopped synchron here */
  ; /*This semi colon is needed to get the code compiled with Intel C Compiler 8.1.20 */
#endif

  if( !hMp3enc->psyActive &&
      psyOut->nSamples == 0 &&
      qcPsyOut->nSamples == 0 &&
      *cbOut == 0 &&
      (hMp3enc->qcKernel->granuleCnt == hMp3enc->psyGranuleCnt) )
    {
      /*
       * all that is left to do now is to empty the bit reservoir
       */
      BSFlush(hMp3enc->bsEnc, pOutput, cbOut);
      if( *cbOut == 0 )
        {
          return MP3ENC_READY;  /* encoding done */
        }
    }

  /*
    check if we have overwritten the user's buffer. This is theoretically
    impossible; it indicates an implementation error.
  */
  if (*cbOut > cbSize)
  {
    return ERROR_PANIC;
  }

  return MP3ENC_OK ;
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetInfo
    description: get information about the encoding process
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encGetInfo
    (
    const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
    struct MP3ENC_INFO*       pInfo     /* receives information */
    )
{
  if (hMp3Enc && pInfo)
  {
    pInfo->bandWidth = hMp3Enc->config.bandWidth;
    pInfo->nDelay    = hMp3Enc->nDelay;

    pInfo->cbBufsizeMin = hMp3Enc->cbBufsizeMin;
    pInfo->nFramesInMeta = FCFramesInMeta(hMp3Enc->frameControl);
    pInfo->cbMetaframeSize = FCMetaframeSize(hMp3Enc->frameControl);

    pInfo->granPerFrame = hMp3Enc->granPerFrame;
    pInfo->ancBitsPerFrame = hMp3Enc->ancBitsPerFrame;
  }
  return MP3ENC_OK;
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetFrameStats
    description: 
    returns:

  ---------------------------------------------------------------------------*/

const int * MP3ENCAPI mp3encGetFrameStats
    (
    const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
    )
{
  if (hMp3Enc && hMp3Enc->bsEnc)
  {
    return mp3GetFrameStats (hMp3Enc->bsEnc) ;
  }

  return NULL ;
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetOflOffset
    description: 
    returns: begin of ofl data in byte from file begin 

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetOflOffset(const struct MP3_ENCODER* hMp3Enc) 
{
  return hMp3Enc->oflOffset;
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetGranCnt
    description: 
    returns: number of processed granules

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetGranCnt(const struct MP3_ENCODER* hMp3Enc) 
{
  return hMp3Enc->qcKernel->granuleCnt;
}
