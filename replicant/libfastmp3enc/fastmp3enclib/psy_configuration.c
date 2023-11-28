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
*   $Id: psy_configuration.c,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <math.h>
#include "mconfig.h"
#include "psy_configuration.h"
#include "mathmac.h"
#include "mathlib.h"
#include "mp3alloc.h"

typedef struct{
    int sfbCnt;                /* Number of scalefactor bands */
    ALIGN_16_BYTE int sfbWidth[MAX_SFB];            /* Width of scalefactor bands  */
}SFB_PARAM;


typedef struct{
    long  sampleRate;
    const SFB_PARAM *paramLong;
    const SFB_PARAM *paramShort;
}SFB_INFO_TAB;


static const float ABS_LOW = 16887.8f; /* maximum peak sine - 96 db*/


#ifndef MP3_SURROUND
/* 
  8000 Hz
*/

ALIGN_16_BYTE static const SFB_PARAM p_8000_long = {
  22,
  {12, 12, 12, 12, 12, 12, 16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2}

};

ALIGN_16_BYTE static const SFB_PARAM p_8000_short = {
   13,
  {8,8,8,12,16,20,24,28,36,2,2,2,26}
};

/*
  11025 Hz
*/

ALIGN_16_BYTE static const SFB_PARAM p_11025_long = {
  22,
  {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54}
};

ALIGN_16_BYTE static const SFB_PARAM p_11025_short = {
  13,
  {4,4,4,6,8,10,12,14,18,24,30,40,18}
};



/*
  12000 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_12000_long = {
   22,
  {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54}
};


ALIGN_16_BYTE static const SFB_PARAM p_12000_short = {
   13,
  {4,4,4,6,8,10,12,14,18,24,30,40,18}
};


/*
    16000 Hz
*/

ALIGN_16_BYTE static const SFB_PARAM p_16000_long = {
  22,
  {6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16, 20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54 }
};


ALIGN_16_BYTE static const SFB_PARAM p_16000_short = {
  13,
  {4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 30, 40, 18}
};


/*
    22050 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_22050_long = {
  22,
  {6 ,6, 6, 6, 6, 6, 8, 10, 12, 14, 16, 20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54}
};
ALIGN_16_BYTE static const SFB_PARAM p_22050_short = {
  13,
  {4, 4, 4, 6, 6, 8, 10, 14, 18, 26, 32, 42, 18}
};


/*
    24000 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_24000_long = {
  22,
  {6,6, 6, 6, 6, 6, 8,10,12,14,16,18, 22, 26, 32, 38, 46, 54, 62, 70, 76, 36}
};
ALIGN_16_BYTE static const SFB_PARAM p_24000_short = {
  13,
  {4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 32, 44, 12}
};


/*
	32000 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_32000_long = {
  22,
  {4, 4, 4, 4, 4, 4, 6, 6, 8, 10, 12, 16, 20, 24, 30, 38, 46, 56, 68, 84, 102, 26}
};
ALIGN_16_BYTE static const  SFB_PARAM p_32000_short = {
  13,
  {4, 4, 4, 4, 6, 8, 12, 16, 20, 26, 34, 42, 12}
};

#endif

/*
	44100 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_44100_long = {
  22,
  {4, 4, 4, 4, 4, 4, 6, 6, 8, 8, 10, 12, 16, 20, 24, 28, 34, 42, 50, 54, 76, 158}
};
ALIGN_16_BYTE static const SFB_PARAM p_44100_short = {
  13,
  {4, 4, 4, 4, 6, 8, 10, 12, 14, 18, 22, 30, 56}
};


/*
    48000 Hz
*/
ALIGN_16_BYTE static const SFB_PARAM p_48000_long = {
  22,
  {4, 4, 4, 4, 4, 4, 6, 6, 6, 8, 10, 12, 16, 18, 22, 28, 34, 40, 46, 54, 54, 192},
};

ALIGN_16_BYTE static const SFB_PARAM p_48000_short = {
  13,
  {4, 4, 4, 4, 6, 6, 10, 12, 14, 16, 20, 26, 66},
};



ALIGN_16_BYTE static const SFB_INFO_TAB sfbInfoTab[] ={

#ifndef MP3_SURROUND

  {8000,&p_8000_long,&p_8000_short},
  {11025,&p_11025_long,&p_11025_short},
  {12000,&p_12000_long,&p_12000_short},
  {16000,&p_16000_long,&p_16000_short},
  {22050,&p_22050_long,&p_22050_short},
  {24000,&p_24000_long,&p_24000_short},
  {32000,&p_32000_long,&p_32000_short},

#endif

  {44100,&p_44100_long,&p_44100_short},
  {48000,&p_48000_long,&p_48000_short}
};


static int initSfbTable(long sampleRate,int blockType,int *sfbOffset,int *sfbCnt)
{
  const SFB_PARAM *sfbParam = 0;
  unsigned int  i = 0;
  int specStartOffset;
  
  /*
    select table
  */
  
  for(i = 0; i < sizeof(sfbInfoTab)/sizeof(SFB_INFO_TAB); i++){
    if(sfbInfoTab[i].sampleRate == sampleRate){
      switch(blockType){
      case LONG_WINDOW:
      case START_WINDOW:
      case STOP_WINDOW:
        sfbParam = sfbInfoTab[i].paramLong;
        break;
      case SHORT_WINDOW:
        sfbParam = sfbInfoTab[i].paramShort;
        break;
      }
      break;
    }
  }
  if(sfbParam==0)
    return(1);
  
  /*
    calc sfb offsets
  */
  
  *sfbCnt=sfbParam->sfbCnt;
  specStartOffset = 0;
  for(i = 0; (int)i < *sfbCnt; i++){
    sfbOffset[i] = specStartOffset;
    specStartOffset += sfbParam->sfbWidth[i];
  }
  sfbOffset[*sfbCnt] = specStartOffset;
  return(0);
}


static void initPbTable(int sfbCnt,int *sfbOffset,int *pbCnt,int *pbOffset)
{
  /* for the dummy psych: psych bands and scale factor bands are identical */
  *pbCnt = sfbCnt;
  copyINT(sfbOffset, pbOffset, sfbCnt+1);
}


//* InitMinPCMResolution: Absolute values are used, so scaling to NORM_PCM_ENERGY is needed*/
void InitMinPCMResolution(int numPb, int pcmResolution,
                    PSY_CONFIGURATION * psyConf) 
{
  int i = 0;
  int tmp = - (pcmResolution * 6 - 26); /* subtract -26: -70 for 16bit */

  /* tmp of -70 is equal to a white noise at -72dB */ 


  float pcmQuantNoise =  (float) pow(10.0f, tmp / 10.0f);


  for( i = 0; i < numPb; i++ ) 
  {
    psyConf->pbPCMquantThreshold[i] = pcmQuantNoise * ABS_LOW * (float)(psyConf->pbOffset[i+1] - psyConf->pbOffset[i]) 
    * NORM_PCM_ENERGY; 

  }
}


int InitPsyConfiguration(long  bitrate,
                         long  samplerate,
                         float bandwidth,
                         int   blocktype,
                         PSY_CONFIGURATION *psyConf)
{
  int pb;
  int sfb;

  if(initSfbTable(samplerate,blocktype,psyConf->sfbOffset,&(psyConf->sfbCnt)))
    return(1);

  initPbTable(psyConf->sfbCnt,psyConf->sfbOffset,&(psyConf->pbCnt),psyConf->pbOffset );

  psyConf->lowpassLine = (int)(2.0f*bandwidth/samplerate *
                               ( blocktype != SHORT_WINDOW ?
				 FRAME_LEN_LONG : FRAME_LEN_SHORT ));
  psyConf->mpegVersion =
    samplerate >= 32000 ? 0 :
    (samplerate >= 16000 ? 1 : 2);

  psyConf->clipEnergy = 1.0e3f; /* max threshold value to avoid clipping */

  if (blocktype == SHORT_WINDOW) {
     psyConf->clipEnergy /= (TRANS_FAC * TRANS_FAC);
  }
 
  /* which psych band / codec band is lowpassLine in? */

  for (pb = 0; pb < psyConf->pbCnt; pb++)
  {
    if (psyConf->pbOffset[pb] >= psyConf->lowpassLine)
      break;
  }
  psyConf->pbActive  = pb;

  for (sfb = 0; sfb < psyConf->sfbCnt; sfb++)
  {
    if (psyConf->sfbOffset[sfb] >= psyConf->lowpassLine)
      break;
  }
  psyConf->sfbActive  = sfb;

#ifdef NO_INTENSITY
  psyConf->useIntensity = 0;
#else /* #ifdef NO_INTENSITY */
  psyConf->useIntensity = ( (bitrate>0) && (bitrate<96000) );
#endif /* #ifdef NO_INTENSITY */

  /* because psycho acoustic is for DUMMY_PSYCH far too stringent; we tweak
     thresholds up by 27 dB to get something useable for intensity stereo */
  psyConf->isD2max      = (float)pow(10.0f, -27.0f/10.0f);

  /* never choose an is limit below 7. Encoder failures may result. sdb */
  psyConf->isLimitLow   = 7 ;

  psyConf->useMS        = (psyConf->useMS  && (bitrate <= 192000)) ;

  psyConf->useMsPreprocessing = 0;
  /* try to achieve 6dB more energy in the mid channel compared to side */
  psyConf->msRatioWanted      = (float)pow(10.0f, 9.0f / 10.0f);
  psyConf->msPreprocessingK   = 0.1f; /* speed of adaptation */

  psyConf->nWindows     = (blocktype != SHORT_WINDOW) ? 1 : TRANS_FAC ;

  /* calc chaos measure for all lines */
  if( blocktype != SHORT_WINDOW )
    psyConf->cmLines = psyConf->lowpassLine * (FFT_LEN/2) / FRAME_LEN_LONG;
  else
    psyConf->cmLines = psyConf->lowpassLine;
  psyConf->toneDefault  = 1.0f;

#ifndef LEAN_PSYCH
    psyConf->noiseRatio = 8.0f;
    psyConf->toneRatio  = 29.0f;
#else
  /* we need the linear representation of noiseRatio/toneRatio here */
  psyConf->noiseRatio = (float)pow(10.0f,  -8.0f/10.0f);
  psyConf->toneRatio  = (float)pow(10.0f, -29.0f/10.0f);
#endif

#ifndef LEAN_PSYCH  
    /* linear energy to threshold ratio */
    psyConf->ratio = (float)pow(10.0f, -29.0f/10.0f);
#endif

  psyConf->maxAllowedIncreaseFactor = 3.0f;
  psyConf->minRemainingThresholdFactor = 0.001f;

  return(0);
}
