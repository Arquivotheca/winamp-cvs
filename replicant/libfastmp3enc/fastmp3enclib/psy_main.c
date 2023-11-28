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
*   $Id: psy_main.c,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <math.h>
#include "mconfig.h"

#include "mp3alloc.h"
#include "mathmac.h"
#include "mathlib.h"

#include "psy_const.h"
#include "psy_main.h"
#include "time_buffer.h"
#include "block_switch.h"
#include "transform.h"
#include "spreading.h"
#include "pre_echo_control.h"
#include "band_nrg.h"
#include "psy_configuration.h"
#include "psy_data.h"
#include "ms_stereo.h"
#ifndef NO_INTENSITY
#include "is_stereo.h"
#endif
#include "polyana.h"
#include "interface.h"
#include "utillib.h"
#include "sf_estim.h"
#include "line_pe.h"
#include "adj_thr.h"


#define ADAPT_IS_POS

struct PSY_INTERNAL {
  /* these are constant over the lifetime of the encoder */
  int nChannels;
  int fullPsych;
#if defined _OPENMP
  int isUsed;
#endif
  int BLOCK_SWITCHING_OFFSET;
  int POLY_ANA_OFFSET;
ALIGN_16_BYTE PSY_CONFIGURATION  psyConf[2]; /* long and short block config */

  /* these are variable */
ALIGN_16_BYTE struct PSY_DATA    psyData[MAX_CHANNELS];
ALIGN_16_BYTE INPUT_BUFFER       psyInputBuffer;

#ifdef ADAPT_IS_POS
ALIGN_16_BYTE int                fWasShortWindow[2];
#endif

  /* scratch space */
ALIGN_16_BYTE float              pScratchLineEnergy[FRAME_LEN_LONG];
};

#ifdef DEBUG_INTENSITY
ALIGN_16_BYTE static int islimitBin[MAX_SFB];
ALIGN_16_BYTE static int isposbin[MAX_SFB][7];
#endif

/*
   forward definitions
*/
static int advancePsychDummyLong(int ch, /* channel */ 
                                 struct PSY_DATA         *psyData,
                                 const PSY_CONFIGURATION *psyConf);
static int advancePsychDummyShort(int ch, /* channel */ 
                                 struct PSY_DATA         *psyData,
                                 const PSY_CONFIGURATION *psyConf);


/*****************************************************************************

    functionname: lowPass
    description:  apply lowpass filtering to mdct signal
                  (currently not used; included for reference)

*****************************************************************************/
#if 0
static void lowPass(const int   nLines,      /* number of lines total */
                    const int   lowpassLine, /* start of trans. band [lines] */
                    const float atten,       /* att. factor per line */
                    float      *mdctSpec)    /* the spectrum, to be modified */
{
  int i;
  float att = 1.0f;

  /* loop until -106 dB attenuation at most */
  for (i = lowpassLine; i < nLines && att > 5E-6; i++)
  {
    mdctSpec[i] *= att;
    att *= atten;
  }
  for (; i < nLines; i++)
  {
    mdctSpec[i] = 0.0f;
  }
}
#endif

/*****************************************************************************

    functionname: PsyOutNew
    description:  allocate memory for psych output
    returns:      an error code

*****************************************************************************/
int PsyOutNew(struct PSY_OUT **phPsyOut)
{
  if (phPsyOut == 0) return 1;

  *phPsyOut = (struct PSY_OUT *) mp3Calloc(1,sizeof(struct PSY_OUT));
  (*phPsyOut)->nSamples = 0;

  return (*phPsyOut == 0);
}

/*****************************************************************************

    functionname: PsyOutDelete
    description:  free memory allocated by PsyOutNew

*****************************************************************************/
void PsyOutDelete(struct PSY_OUT *hPsyOut)
{
  if (hPsyOut)
    mp3Free(hPsyOut);
}

/*****************************************************************************

    functionname: PsyNew
    description:  allocates memory for psychoacoustic
    returns:      an error code
    input:        pointer to a psych handle, filled config structure

*****************************************************************************/
int PsyNew(struct PSY_INTERNAL  **phpsy,
           const struct PSY_INIT *pConfig)
{
  struct PSY_INTERNAL *hpsy = 0;
  int error = 0;

  if (!phpsy || !pConfig) return 1;

  hpsy = (struct PSY_INTERNAL *)mp3Calloc(1,sizeof(struct PSY_INTERNAL));
  error = (hpsy == 0);

  if (!error)
    {
      int ch;

      /*
        allocate memory for spreading function
      */
      if (!error)
        {
          hpsy->psyConf[1].pSpreadingFunction =
            hpsy->psyConf[0].pSpreadingFunction = InitSpreadingFunction();
          error = (hpsy->psyConf[0].pSpreadingFunction == 0);
        }


      /* for each channel, */
      for (ch = 0; ch < pConfig->nChannels; ch++)
        {
          struct PSY_DATA *psyDataCh = hpsy->psyData + ch ;

          /* allocate block switching objects and poly phase objects */
          if( error                                                    ||
              mp3BlockSwitchingNew(&(psyDataCh->hBlockSwitching)) != 0 ||
              PolyPhaseNew(&(psyDataCh->hPolyPhase))              != 0)
            error = 1;

        }
    }

  /* if an error occured, clean up */
  if (error)
    {
      PsyDelete(hpsy);
      hpsy = 0;
    }

  /* return the object handle to the caller */
  *phpsy = hpsy;
  return error;
}

/*****************************************************************************

    functionname: PsyInit
    description:  Initialize a psych handle. No memory allocation inside.
    returns:      an error code
    input:        a psych handle, a filled config structure

*****************************************************************************/
int PsyInit(struct PSY_INTERNAL   *hpsy,
            const struct PSY_INIT *pConfig)
{
  int error;

  error = (hpsy == 0 || pConfig == 0);

  if (!error)
  {
    float *pScratchSpreadNorm ;
    float bandwidth = pConfig->bandWidth;
    int   blockType;
    int   ch;

    hpsy->nChannels = pConfig->nChannels;

    if ( pConfig->useDualPass ){
      /* if we use the full psych there is a FFT an so another delay */
      hpsy->POLY_ANA_OFFSET = ( FFT_OFFSET + FFT_LEN/2 + POLY_PHASE_DELAY );
      hpsy->BLOCK_SWITCHING_OFFSET = ( hpsy->POLY_ANA_OFFSET+2*FRAME_LEN_SHORT-POLY_PHASE_DELAY );
    }
    else {
      hpsy->POLY_ANA_OFFSET = (0);
      hpsy->BLOCK_SWITCHING_OFFSET = (2*FRAME_LEN_SHORT-POLY_PHASE_DELAY);
    }

    /*
      Create input buffer (clear it & set offsets)
    */
    error = InputBufferNew(&(hpsy->psyInputBuffer), hpsy->BLOCK_SWITCHING_OFFSET);
    pScratchSpreadNorm = hpsy->psyInputBuffer.timeSignal;

    /*
      fill psyConf for long and short blocks
    */
    for (blockType = 0; !error && blockType < 2; blockType++)
    {
      PSY_CONFIGURATION *psyConf = hpsy->psyConf + blockType;

      psyConf->useMS = pConfig->useMS;

      psyConf->fullPsych = 0;

      InitPsyConfiguration(pConfig->bitRate,
                           pConfig->sampleRate,
                           bandwidth,
                           blockType ? SHORT_WINDOW : LONG_WINDOW,
                           psyConf);

      if( pConfig->noIntensity )
        psyConf->useIntensity = 0;

      InitBarcValues(blockType ? FRAME_LEN_SHORT : FRAME_LEN_LONG,
                     pConfig->sampleRate,
                     psyConf->pbCnt,
                     psyConf->pbOffset,
                     psyConf->pbBarcVal,
                     &(psyConf->barcValScaling),
                     psyConf->pbSpreadNorm,
                     pScratchSpreadNorm,
                     psyConf->pSpreadingFunction,
                     psyConf->pbMaskLowFactor,
                     psyConf->pbMaskHighFactor);

      InitMinPCMResolution(psyConf->pbCnt, pConfig->pcmResolution, psyConf);
    }
    /*
      Initialize input buffer (clear it & set offsets)
    */
    error = InitInputBuffer(&(hpsy->psyInputBuffer));

    /*
      per-channel initialization
    */
    for (ch = 0; !error && ch < pConfig->nChannels; ch++)
    {
      struct PSY_DATA *psyDataCh = hpsy->psyData + ch ;

      psyDataCh->dampingFactor = 1.0f;
      psyDataCh->windowSequence = LONG_WINDOW;
      if (mp3BlockSwitchingInit(psyDataCh->hBlockSwitching) != 0 ||
          PolyPhaseInit(psyDataCh->hPolyPhase)              != 0 ||
          InitTransform(&(psyDataCh->transformBuffer))      != 0 ||
          mp3InitPreEchoControl(psyDataCh->pbThresholdnm1))
      {
        error = 1;
      }

      if( !error )
      {
        if( ch == 1 )
          {
            int midPosition = hpsy->psyConf[0].mpegVersion ? 0 : 3;
#ifndef NO_INTENSITY
            /*setINT(midPosition, psyDataCh->sfbIsPositions.Long,MAX_GROUPED_SFB);*/
            setINT(midPosition, psyDataCh->sfbIsPositions.Long,MAX_SFB_LONG);
            setINT(midPosition, &psyDataCh->sfbIsPositions.Short[0][0],TRANS_FAC*MAX_SFB_SHORT);
#endif
#ifdef ADAPT_IS_POS
            hpsy->fWasShortWindow[0]=-1;
            hpsy->fWasShortWindow[1]=-1;
#endif
          }

      }
    } /* end of per-channel initialization */
  }

  return error;
}

/*****************************************************************************

    functionname: PsyDelete
    description:  Free all memory allocated by PsyNew()

*****************************************************************************/
void PsyDelete(struct PSY_INTERNAL *hpsy)
{
  if (hpsy)
  {
    int ch;
    for (ch = 0; ch < hpsy->nChannels; ch++)
    {
      mp3BlockSwitchingDelete(hpsy->psyData[ch].hBlockSwitching);
      PolyPhaseDelete(hpsy->psyData[ch].hPolyPhase);
    }
    InputBufferDelete (&hpsy->psyInputBuffer);
    FreeSpreadingFunction(hpsy->psyConf[0].pSpreadingFunction);
    mp3Free(hpsy);
  }
}

/*****************************************************************************

    functionname: psyFeedInputBufferFloat
    description:  feed nSamples interleaved floats into the modulo buffer
    returns:      error code
    input:        channel-interleaved floats

*****************************************************************************/
int psyFeedInputBufferFloat(struct PSY_INTERNAL *hPsy, float input[][2], int nSamples)
{
  FeedInputBuffer(&(hPsy->psyInputBuffer),(float*)input,nSamples);
  return 0;
}

/*****************************************************************************

    functionname: psyGetInputBufferFloat
    description:  gets pointer to samples in psyInputBuffer
    returns:      pointer to samples
    input:       

*****************************************************************************/
float *psyGetInputBufferFloat(struct PSY_INTERNAL *hPsy)
{
  return AccessInputBuffer(&hPsy->psyInputBuffer,hPsy->POLY_ANA_OFFSET,0); 
}

/*****************************************************************************

    functionname: psyMain
    description:  advance psychoacoustic. This function assumes that enough
                  input data is in the modulo buffer
    returns:      error code
    input:        
    output:       a PSY_OUT output structure for use in the Q&C kernel

*****************************************************************************/
int psyMain(struct PSY_INTERNAL *hpsy,
            const int            granule,
            struct PSY_OUT_LIST *firstPsyOut,
            const int            dualMono )
{
  int ch;                /* counts through channels */

  ALIGN_16_BYTE int fIsShortWindow[2]; /* do we have a short window ? */
  struct PSY_OUT *hpsyOut = firstPsyOut->psyOut;

  for(ch = 0; ch < hpsy->nChannels; ch++)
  {
    /*
      block switching
    */
    mp3BlockSwitching (hpsy->psyData[ch].hBlockSwitching,
                       AccessInputBuffer(&hpsy->psyInputBuffer,
                                         hpsy->BLOCK_SWITCHING_OFFSET, ch));

  } 
  /*
    poly analyse
  */

  if (hpsy->nChannels == 1)
  {
    float *pTimeSignal = AccessInputBuffer(&hpsy->psyInputBuffer,hpsy->POLY_ANA_OFFSET,0);
    (void)sendDebout("polyPhase",  576, 2, "in", MTV_FLOAT, pTimeSignal);

    ShiftTransformBuffer(&hpsy->psyData[0].transformBuffer);

    PolyAnalyse(hpsy->psyData[0].hPolyPhase,
                pTimeSignal,hpsy->psyData[0].transformBuffer.paMdctBuffer);

    sendDebout( "polyOut", 2*576, 1, "0", MTV_FLOAT,
                hpsy->psyData[0].transformBuffer.paMdctBuffer );
  }
  else
  {
    /* stereo */

    float *pTimeSignal;

    ShiftTransformBuffer(&hpsy->psyData[0].transformBuffer);
    ShiftTransformBuffer(&hpsy->psyData[1].transformBuffer);

    pTimeSignal = AccessInputBuffer(&hpsy->psyInputBuffer,hpsy->POLY_ANA_OFFSET,0);


    sendDebout("polyTime", FRAME_LEN_LONG,2,"time",MTV_FLOAT,pTimeSignal);
    /* using 2 times single polyphase */
    /* 1. channel */
    PolyAnalyse(hpsy->psyData[0].hPolyPhase,
                pTimeSignal,
                hpsy->psyData[0].transformBuffer.paMdctBuffer);    

    sendDebout( "polyOut", 2*576, 1, "0", MTV_FLOAT,
                hpsy->psyData[0].transformBuffer.paMdctBuffer );

    /* 2. channel */
    PolyAnalyse(hpsy->psyData[1].hPolyPhase,
                pTimeSignal+1,
                hpsy->psyData[1].transformBuffer.paMdctBuffer);        

    if (hpsy->psyConf[0].useMsPreprocessing)
    {
      MSGainPreprocessing( hpsy->psyData[0].transformBuffer.paMdctBuffer,
                           hpsy->psyData[1].transformBuffer.paMdctBuffer,
                           hpsy->psyData[1].dampingFactor,
                           FRAME_LEN_LONG);
    }
  }

  /*
    synchronize left and right block type
  */
#ifdef NO_DUAL_MONO_FIXES
  if ( !dualMono )
    mp3SyncBlockSwitching(hpsy->psyData[0].hBlockSwitching,
                          hpsy->psyData[1].hBlockSwitching, 
                          hpsy->nChannels, TRUE,
                          &hpsy->psyData[0].windowSequence,
                          &hpsy->psyData[1].windowSequence);
#else
  mp3SyncBlockSwitching(hpsy->psyData[0].hBlockSwitching,
                        hpsy->psyData[1].hBlockSwitching, 
                        hpsy->nChannels, ((dualMono==0)?TRUE:FALSE),
                        &hpsy->psyData[0].windowSequence,
                        &hpsy->psyData[1].windowSequence);
#endif

  for(ch = 0; ch < hpsy->nChannels; ch++)  {
    setDeboutVars(-1,-1,ch,-1);

    fIsShortWindow[ch] = (hpsy->psyData[ch].windowSequence == SHORT_WINDOW);

#if defined ADAPT_IS_POS && !defined NO_INTENSITY
    /* adapt is-positions */
    if((ch == 1) && hpsy->psyConf[fIsShortWindow[ch]].useIntensity && (hpsy->fWasShortWindow[ch] != -1))
    {
      if (fIsShortWindow[ch] != hpsy->fWasShortWindow[ch])
      {
       
        int *pIsPosIn;
        int *pIsPosOut;
        
        if(fIsShortWindow[ch]) {
          pIsPosIn  = hpsy->psyData[ch].sfbIsPositions.Long;
          pIsPosOut = hpsy->psyData[ch].sfbIsPositions.Short[0];
        }
        else {
          pIsPosIn  = &hpsy->psyData[ch].sfbIsPositions.Short[TRANS_FAC-1][0];
          pIsPosOut =  hpsy->psyData[ch].sfbIsPositions.Long;
        }
        
        AdaptIntensityPositions(hpsy->psyConf[fIsShortWindow[ch]].mpegVersion, hpsy->fWasShortWindow[ch],fIsShortWindow[ch], pIsPosIn, pIsPosOut);

        if(fIsShortWindow[ch]) {
          int i;

          for(i=1;i<TRANS_FAC;i++) {
			  copyINT(hpsy->psyData[ch].sfbIsPositions.Short[0],
						hpsy->psyData[ch].sfbIsPositions.Short[i], 
						MAX_SFB_SHORT);

			  /*memcpy(hpsy->psyData[ch].sfbIsPositions.Short[i],hpsy->psyData[ch].sfbIsPositions.Short[0],sizeof(int)*MAX_SFB_SHORT); */
          }
        }
      }
    }
    hpsy->fWasShortWindow[ch] = fIsShortWindow[ch];
#endif

    if(!fIsShortWindow[ch])  {
      advancePsychDummyLong(ch, hpsy->psyData, &hpsy->psyConf[0]);
    }
    else {
      advancePsychDummyShort(ch, hpsy->psyData, &hpsy->psyConf[1]);
    }
  }

  InvalidateInputBuffer(&hpsy->psyInputBuffer,FRAME_LEN_LONG);

  /*
    stereo Processing is only done if (surprise!) we have two channels and
    the block type is the same in both channels.
  */

  if ((hpsy->nChannels == 2)                                           &&
      (hpsy->psyData[0].windowSequence == hpsy->psyData[1].windowSequence))
  {
    PSY_CONFIGURATION *psyConf = &hpsy->psyConf[fIsShortWindow[0]];
    int w;

    /*
      MS Stereo
    */

    if ( psyConf->useMS )
      hpsyOut->msUsed = psyConf->useMS &&
        MsStereoProcessing(psyConf->nWindows,
                           hpsy->psyData[0].sfbEnergy.Long, 
                           hpsy->psyData[1].sfbEnergy.Long,
                           hpsy->psyData[0].sfbEnergyMS.Long, 
                           hpsy->psyData[1].sfbEnergyMS.Long,
                           hpsy->psyData[0].mdctSpectrum.Long, 
                           hpsy->psyData[1].mdctSpectrum.Long,
                           hpsy->psyData[0].sfbThreshold.Long, 
                           hpsy->psyData[1].sfbThreshold.Long,
                           &hpsy->psyData[0].oldMsFlag,
                           granule,
                           hpsy->psyData[1].isLimit,
                           psyConf->sfbOffset);

    /*
      IS stereo
    */

    /* set isLimit in psy output */
    for (w = 0; w < psyConf->nWindows; w++)
    {
      /* in the left channel, isLimit == sfbActive always */
      hpsyOut->psyOutChannel[0].isLimit[w] = psyConf->sfbActive;
      /* in the right channel, isLimit is variable */
      hpsyOut->psyOutChannel[1].isLimit[w] = hpsy->psyData[1].isLimit[w] ;
    }

    /* find out if intensity is switched on */
    if (granule == 0)
    {
      hpsyOut->isUsed = 0;
      if (psyConf->useIntensity)
      {
        int win;
        for (win = 0 ; win < psyConf->nWindows ; win++)
        {
          hpsyOut->isUsed |= (hpsyOut->psyOutChannel[1].isLimit[win] < psyConf->sfbActive);
        }
#if defined _OPENMP
        hpsy->isUsed = hpsyOut->isUsed;
#endif
      }
    }
#if defined _OPENMP
    else {    /* granule == 1: OMP uses different psyOuts for each garnule */
      if (psyConf->useIntensity)
        hpsyOut->isUsed = hpsy->isUsed;
    }
#endif


#ifndef NO_INTENSITY
    if (hpsyOut->isUsed)
    {
      /* IS processing adjusts left channel, zeroes out right. */
      IsStereoProcessing(psyConf->mpegVersion,
                         hpsy->psyData[0].sfbEnergy.Long,
                         hpsy->psyData[1].sfbEnergy.Long,
                         hpsy->psyData[0].mdctSpectrum.Long,
                         hpsy->psyData[1].mdctSpectrum.Long,
                         hpsy->psyData[0].sfbThreshold.Long,
                         hpsy->psyData[1].sfbThreshold.Long,
                         hpsy->psyData[1].sfbIsDirX.Long,
                         hpsy->psyData[1].sfbIsDirY.Long,
                         hpsy->psyData[1].sfbIsCrossProduct.Long,
                         hpsyOut->psyOutChannel[1].isLimit,
                         psyConf->sfbActive,
                         psyConf->sfbOffset,
                         psyConf->nWindows);

      /* copy intensity positions into output array, interleaved */
      if (hpsy->psyData[1].windowSequence == SHORT_WINDOW) 
      {
        for (w = 0; w < psyConf->nWindows; w++) 
        {
          copyINTflex(hpsy->psyData[1].sfbIsPositions.Short[w],
                      1,
                      hpsyOut->sfbIsPositions+w,
                      psyConf->nWindows,
                      psyConf->sfbActive);
        }
      }
      else 
      {
        copyINT(hpsy->psyData[1].sfbIsPositions.Long,
                hpsyOut->sfbIsPositions,
                psyConf->sfbActive);
      }
    }
#ifndef ADAPT_IS_POS
    if((psyConf[1].useIntensity)
       && (hpsy->psyData[1].windowSequence == START_WINDOW)) 
    {
      /* reset direction information  */
      int midPosition = psyConf->mpegVersion ? 0 : 3;

      setINT(midPosition, hpsy->psyData[1].sfbIsPositions.Short[0],TRANS_FAC*MAX_SFB_SHORT);
    }
#endif
#endif /* #ifdef NO_INTENSITY */
  }

  /*
    build output
  */
  for(ch = 0; ch < hpsy->nChannels; ch++)
  {
    BuildInterface(&hpsy->psyData[ch].mdctSpectrum,
                   &hpsy->psyData[ch].sfbThreshold,
                   &hpsy->psyData[ch].sfbEnergy,
                   &hpsy->psyData[ch].sfbEnergyMS,
                    hpsy->psyData[ch].windowSequence,
                    hpsy->psyConf[fIsShortWindow[ch]].sfbCnt,
                    hpsy->psyConf[fIsShortWindow[ch]].sfbActive,
                    hpsy->psyConf[fIsShortWindow[ch]].sfbOffset,
                   &(hpsyOut->psyOutChannel[ch]));
  }

  /*
    calculate pe
   */
  firstPsyOut->windowSequence = LONG_WINDOW;
  for (ch = 0; ch < hpsy->nChannels; ch++)
    {
      struct PSY_OUT_CHANNEL *psyOutChan = &hpsyOut->psyOutChannel[ch];

      CalcFormFactorChannel( psyOutChan->sfbFormFactor,
                             psyOutChan->sfbMaxSpec,
                             psyOutChan );

      prepareSfbPe(&firstPsyOut->peData.peChannelData[ch],
                   psyOutChan->sfbEnergy, 
                   psyOutChan->sfbThreshold, 
                   psyOutChan->sfbFormFactor,
                   psyOutChan->sfbOffsets, 
                   psyOutChan->sfbActive,
                   psyOutChan->windowSequence);

      if( hpsy->psyData[ch].windowSequence == SHORT_WINDOW )
	firstPsyOut->windowSequence = SHORT_WINDOW;
    }

  calcPe(&firstPsyOut->peData, hpsyOut, /*hpsy->nChannels,*/ 0, hpsy->nChannels);
  firstPsyOut->peData.noRedPe = firstPsyOut->peData.pe;

  return(0); /* no error */
}


static int advancePsychDummyLong(int ch, /* channel */
                                 struct PSY_DATA         *psyData,
                                 const PSY_CONFIGURATION *psyConf)
{
  int i;

  /*
    Transform
  */
  MdctTransform_LONG(&psyData[ch].transformBuffer,
                     psyData[ch].mdctSpectrum.Long,
                     psyData[ch].windowSequence);

  /*
    Alias reduction
  */
  AliasReduction(psyData[ch].mdctSpectrum.Long);

  sendDebout("mdctSpectrum", FRAME_LEN_LONG, 1, "mdctSpec", 
             MTV_FLOAT, psyData[ch].mdctSpectrum.Long);
  /* low pass */
  setFLOAT(0.0f, 
           psyData[ch].mdctSpectrum.Long+psyConf->lowpassLine,
           FRAME_LEN_LONG-psyConf->lowpassLine);

  /*
    Calc sfb-bandwise mdct-energies for left and right channel
  */
  CalcBandEnergy(psyData[ch].mdctSpectrum.Long,
                 psyConf->sfbOffset,
                 psyConf->sfbActive,
                 psyData[ch].sfbEnergy.Long);

  sendDebout("sfbDataLong",  psyConf[0].sfbCnt, 1, "sfbEn",
             MTV_FLOAT, psyData[ch].sfbEnergy.Long);

  sendDebout("psyBandDataLong",  psyConf[0].sfbCnt, 1, "sfbEn",
             MTV_FLOAT, psyData[ch].sfbEnergy.Long);

  sendDebout("psyBandDataLong",  psyConf[0].sfbCnt, 1, "pbMinEnergy",
             MTV_FLOAT, psyConf->pbMinEnergy);

  /*
    zero energy above lowpass.
   */
  setFLOAT(0.0f, psyData[ch].sfbEnergy.Long + psyConf->sfbActive,
           psyConf->sfbCnt - psyConf->sfbActive);

  /*
    In the dummy psych, pbs are sfb-wide, i.e. they are identical. To keep
    the original code without too many changes, we copy the energies
    to the pb-array.

    copy to pb-Energies
   */
  copyFLOAT(psyData[ch].sfbEnergy.Long, psyData[ch].pbEnergy.Long,
            psyConf->sfbActive);

  (void)sendDebout("psyBandDataLong",  psyConf->pbActive, 1, "pbEnergy", 
                   MTV_FLOAT, psyData[ch].pbThreshold.Long);

  /*
    Calc bandwise energies for mid and side channel
    Do it only if 2 channels exist and only once (in the right channel)
  */
  if (ch == 1 && psyConf->useMS)
  {
    CalcBandEnergyMS(psyData[0].mdctSpectrum.Long,
                     psyData[1].mdctSpectrum.Long,
                     psyConf->sfbOffset,
                     psyConf->sfbActive,
                     psyData[0].sfbEnergyMS.Long,
                     psyData[1].sfbEnergyMS.Long);
  }


  /*
     multiply pbEnergy by ratio
  */
  for (i=0; i<psyConf->pbActive; i++) {
     psyData[ch].pbThreshold.Long[i] = psyData[ch].pbEnergy.Long[i] * 
                                       psyConf->ratio;
     /* limit threshold to avoid clipping */
     psyData[ch].pbThreshold.Long[i] = min(psyData[ch].pbThreshold.Long[i],
                                           psyConf->clipEnergy);
  }

  (void)sendDebout("psyBandDataLong",  psyConf->pbActive, 1, "pbWeightEn", 
                   MTV_FLOAT, psyData[ch].pbThreshold.Long);
  /*
    spreading
  */  
  SpreadingMax(psyConf->pbActive,
               psyConf->pbMaskLowFactor, psyConf->pbMaskHighFactor,
               psyData[ch].pbThreshold.Long);

  /* 
    psych bands min quant. threshold
  */

  maxFLOAT(psyData[ch].pbThreshold.Long, psyConf->pbPCMquantThreshold, 
           psyData[ch].pbThreshold.Long, psyConf->pbActive);

  (void)sendDebout("psyBandDataLong",  psyConf->pbActive, 1, "pbThreshold", 
                   MTV_FLOAT, psyData[ch].pbThreshold.Long);
  /*
    preecho control
  */
  if(psyData[ch].windowSequence == STOP_WINDOW)
  {
    /* 
       prevent preecho from comparing stop 
       thresholds with short thresholds 
    */
    setFLOAT(1.0e20f, psyData[ch].pbThresholdnm1,psyConf->pbActive);
  }

  mp3PreEchoControl(psyData[ch].pbThresholdnm1,
                    psyConf->pbActive,
                    psyConf->maxAllowedIncreaseFactor,
                    psyConf->minRemainingThresholdFactor,
                    psyData[ch].pbThreshold.Long);

  if(psyData[ch].windowSequence == START_WINDOW)
  {
    /*
      prevent preecho in next frame to compare start 
      thresholds with short thresholds
    */
    setFLOAT(1.0e20f, psyData[ch].pbThresholdnm1, psyConf->pbActive);
  }

  (void)sendDebout("psyBandDataLong",  psyConf->pbActive, 1, "thrPreEcho", 
                   MTV_FLOAT, psyData[ch].pbThreshold.Long);

  /*
    adapt to sfb is a simple copy
  */
  copyFLOAT(psyData[ch].pbThreshold.Long, psyData[ch].sfbThreshold.Long,
            psyConf->sfbActive);
  setFLOAT(1.0e20f, psyData[ch].sfbThreshold.Long + psyConf->sfbActive,
           psyConf->sfbCnt - psyConf->sfbActive);

  (void)sendDebout("mdctSpec", FRAME_LEN_LONG, 1,"mdct",MTV_FLOAT,psyData[ch].mdctSpectrum.Long);
  (void)sendDebout("psyThres",  psyConf->sfbCnt,1,"treshLong",MTV_FLOAT,psyData[ch].sfbThreshold.Long);

  /*
    intensity stereo decision(s): Position, limit
  */
  if(ch == 1 && psyConf->useIntensity) 
  {
#ifndef ADAPT_IS_POS
    if(psyData[1].windowSequence == STOP_WINDOW) 
    {
      /* reset direction information  */
      int midPosition = psyConf->mpegVersion ? 0 : 3;

      setINT(midPosition, psyData[1].sfbIsPositions.Long,MAX_SFB_LONG);
    }
#endif
#ifndef NO_INTENSITY
    psyData[1].isLimit[0] = CalcIntensityPositions(psyConf->mpegVersion,
                                                   psyData[0].mdctSpectrum.Long,
                                                   psyData[1].mdctSpectrum.Long,
                                                   psyData[0].sfbEnergy.Long,
                                                   psyData[1].sfbEnergy.Long,
                                                   psyData[0].sfbThreshold.Long,
                                                   psyData[1].sfbThreshold.Long,
                                                   psyConf->sfbOffset,
                                                   psyConf->isLimitLow,
                                                   psyConf->sfbActive,
                                                   0, /* no short block! */
                                                   psyConf->isD2max,
                                                   psyData[1].sfbIsPositions.Long,
                                                   psyData[1].sfbIsPositions.Long,
                                                   psyData[1].sfbIsDirX.Long,
                                                   psyData[1].sfbIsDirY.Long,
                                                   psyData[1].sfbIsCrossProduct.Long);
#else /* #ifdef NO_INTENSITY */
    psyData[1].isLimit[0] = psyConf->sfbActive;
#endif /* #ifdef NO_INTENSITY */
  }
  else 
  {
    psyData[1].isLimit[0] = psyConf->sfbActive;
  }

#ifdef DEBUG_INTENSITY
  islimitBin[ psyData[1].isLimit[0] ]++;
  {
    int i;
    for (i=0; i<22; i++)
      isposbin[i][ psyData[1].sfbIsPositions.Long[i] ]++;
  }
#endif

  /*
    adapt MS preprocessing crosstalk constant
  */
  if (ch == 1 && psyConf->useMsPreprocessing)
  {
    AdaptMSGainPreprocessing(psyData[1].isLimit[0],
                             psyConf,
                             psyData[0].sfbEnergyMS.Long,
                             psyData[1].sfbEnergyMS.Long,
                             &(psyData[1].dampingFactor));
  }
  return 0;
}


static int advancePsychDummyShort(int ch, /* channel */
                                  struct PSY_DATA         *psyData,
                                  const PSY_CONFIGURATION *psyConf)
{
  int w;
  int i;
    
  for(w = 0; w < TRANS_FAC; w++)
  {
 
    /*
      Transform
    */
    
    MdctTransform_SHORT(&psyData[ch].transformBuffer,
                        psyData[ch].mdctSpectrum.Short[w],
                        w);
    
    sendDebout("mdctSpectrum", FRAME_LEN_SHORT, 1, "mdctSpec", 
                         MTV_FLOAT, psyData[ch].mdctSpectrum.Short[w]);
    /* low pass */
    setFLOAT(0.0f, 
             psyData[ch].mdctSpectrum.Short[w]+psyConf->lowpassLine,
             FRAME_LEN_SHORT-psyConf->lowpassLine);
    
    /*
      Calc sfb-bandwise mdct-energies for left and right channel
    */
    CalcBandEnergy(psyData[ch].mdctSpectrum.Short[w],
                   psyConf->sfbOffset,
                   psyConf->sfbActive,
                   psyData[ch].sfbEnergy.Short[w]);

    sendDebout("sfbDataLong",  psyConf->sfbCnt, 1, "sfbEn", 
                         MTV_FLOAT, psyData[ch].sfbEnergy.Short[w]);

    /*
      zero energy above lowpass.
     */
    setFLOAT(0.0f, psyData[ch].sfbEnergy.Short[w] + psyConf->sfbActive,
             psyConf->sfbCnt - psyConf->sfbActive);

    /*
      In the dummy psych, pbs are sfb-wide, i.e. they are identical. To keep
      the original code without too many changes, we copy the energies
      to the pb-array.

      copy to pb-Energies
     */
    copyFLOAT(psyData[ch].sfbEnergy.Short[w], psyData[ch].pbEnergy.Short[w],
              psyConf->sfbActive);


    /*
      Calc bandwise energies for mid and side channel
      Do it only if 2 channels exist and only once
    */
    if (ch == 1 && psyConf->useMS)
      CalcBandEnergyMS(psyData[0].mdctSpectrum.Short[w],
                       psyData[1].mdctSpectrum.Short[w],
                       psyConf->sfbOffset,
                       psyConf->sfbActive,
                       psyData[0].sfbEnergyMS.Short[w],
                       psyData[1].sfbEnergyMS.Short[w]);

    /*
       multiply pbEnergy by ratio
    */
    for (i=0; i<psyConf->pbActive; i++) {
       psyData[ch].pbThreshold.Short[w][i] = psyData[ch].pbEnergy.Short[w][i] *
                                             psyConf->ratio;
       /* limit threshold to avoid clipping */
       psyData[ch].pbThreshold.Short[w][i] = 
             min(psyData[ch].pbThreshold.Short[w][i], psyConf->clipEnergy);
    }

    /*
       spreading
    */
    SpreadingMax(psyConf->pbActive,
                 psyConf->pbMaskLowFactor, psyConf->pbMaskHighFactor,
                 psyData[ch].pbThreshold.Short[w]);


    /* 
       psych bands min quant. threshold 
    */
    maxFLOAT(psyData[ch].pbThreshold.Short[w], psyConf->pbPCMquantThreshold, 
             psyData[ch].pbThreshold.Short[w], psyConf->pbActive);


    /*
      preecho control
    */
    mp3PreEchoControl(psyData[ch].pbThresholdnm1,
                      psyConf->pbActive,
                      psyConf->maxAllowedIncreaseFactor,
                      psyConf->minRemainingThresholdFactor,
                      psyData[ch].pbThreshold.Short[w]);

    /* 
      adapt to sfb is a simple copy
    */
    copyFLOAT(psyData[ch].pbThreshold.Short[w], 
              psyData[ch].sfbThreshold.Short[w],
              psyConf->sfbActive);
    setFLOAT(1.0e20f, psyData[ch].sfbThreshold.Short[w] + psyConf->sfbActive,
             psyConf->sfbCnt-psyConf->sfbActive);

    /*
      intensity stereo decision(s): Position, limit
    */
    {
      int *pPrevIsPos;

#ifndef NO_INTENSITY

#ifdef ADAPT_IS_POS
      /* in case of short windows use IS positions of last window as starting point for fluctuation considerations */

      if(w==0) {
        pPrevIsPos=psyData[1].sfbIsPositions.Short[TRANS_FAC-1];
      }
      else {
        pPrevIsPos=psyData[1].sfbIsPositions.Short[w-1];
      }

#else
      pPrevIsPos=psyData[1].sfbIsPositions.Short[w];
#endif

      psyData[1].isLimit[w] = (ch == 1 && psyConf->useIntensity) ?
        CalcIntensityPositions(psyConf->mpegVersion,
                               psyData[0].mdctSpectrum.Short[w],
                               psyData[1].mdctSpectrum.Short[w],
                               psyData[0].sfbEnergy.Short[w],
                               psyData[1].sfbEnergy.Short[w],
                               psyData[0].sfbThreshold.Short[w],
                               psyData[1].sfbThreshold.Short[w],
                               psyConf->sfbOffset,
                               psyConf->isLimitLow,
                               psyConf->sfbActive,
                               1, /* short block! */
                               psyConf->isD2max,
                               psyData[1].sfbIsPositions.Short[w],
                               pPrevIsPos,
                               psyData[1].sfbIsDirX.Short[w],
                               psyData[1].sfbIsDirY.Short[w],
                               psyData[1].sfbIsCrossProduct.Short[w]) :
        psyConf->sfbActive;
#else /* #ifdef NO_INTENSITY */
    psyData[1].isLimit[w] =       psyConf->sfbActive;
#endif /* #ifdef NO_INTENSITY */
    }
  }
  return 0;
}


int GetBlockSwitchingOffset( /*struct PSY_INTERNAL *hpsy */) {
  return (2*FRAME_LEN_SHORT-POLY_PHASE_DELAY);
  /* return hpsy->BLOCK_SWITCHING_OFFSET; */
}
