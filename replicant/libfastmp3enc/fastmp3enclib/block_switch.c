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
*   $Id: block_switch.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

/****************** Includes *****************************/
#include "mconfig.h"
#include "mp3alloc.h"

#include <float.h>

#include "psy_const.h"
#include "block_switch.h"
#include "mathlib.h"
#include "time_buffer.h"
#include "utillib.h"


/****************** Defines ******************************/
#define BLOCK_SWITCH_WINDOWS TRANS_FAC               /* number of windows for energy calculation */
#define BLOCK_SWITCH_WINDOW_LEN FRAME_LEN_SHORT      /* minimal granularity of energy calculation */
#define NO_OF_ATTACK_CRITERIA 9
#define N_IIR_STATES 2           /* Length of HighPass-FIR-Filter for Attack-Detection */

/****************** Structures ***************************/
struct BLOCK_SWITCHING_CONTROL {
  int   windowSequence;
  int   nextwindowSequence;
  int	  attack;
  int   lastattack;
  ALIGN_16_BYTE float winNrgF[2][BLOCK_SWITCH_WINDOWS];        /* filtered time signal energy in windows (last and current) */
  ALIGN_16_BYTE float iirStates[N_IIR_STATES];      /* filter delay-line */
};

/**************** internal function prototypes ***********/

__inline static float
IIRnextSample(const float x, float states[]);

/****************** Globals *****************************/

struct BLOCK_SWITCHING_PARAM
{
  ALIGN_16_BYTE float attackFac[2];                           /* fractional 24  nrg attack over 1 window, over 2 windows */
  float minWindowNrg;                           /* fractional 48  min window nrg filtered */
};

static const struct BLOCK_SWITCHING_PARAM blockSwitchParam =
{
  { /* attackFac */
    0.25f, /* filtered energy relation over 2 windows */
    0.25f, /* filtered energy relation over adjacent windows */
  },
  /** minWindowEnergy */
  
  47110000.0f/2.0f * NORM_PCM_ENERGY   /* TO DO minimal window Energie filtered */


};

static const float hiPassCoeff[2] = {-0.5095f, 0.7548f};


int mp3BlockSwitchingNew(BLOCK_SWITCHING_HANDLE *blockSwitchingControl)
{
  *blockSwitchingControl = (BLOCK_SWITCHING_HANDLE)mp3Alloc(sizeof(struct BLOCK_SWITCHING_CONTROL));

  return (*blockSwitchingControl != 0) ? 0 : 1;
}

int 
mp3BlockSwitchingInit(BLOCK_SWITCHING_HANDLE blockSwitchingControl)
{
  int i, s;

  /* Initialize HighPass-FIR-Filter */
  for (i = 0; i < N_IIR_STATES; i++)
    blockSwitchingControl->iirStates[i] = 0.0;  /* clear filter delay-line */

  /* Clear Window Energies */
  for (s = 0; s < BLOCK_SWITCH_WINDOWS; s++)
  {
    blockSwitchingControl->winNrgF[0][s] = 0.0f;
    blockSwitchingControl->winNrgF[1][s] = 0.0f;
  }

  /* Initialize startvalue for blocktype */
  blockSwitchingControl->windowSequence     = LONG_WINDOW;
  blockSwitchingControl->nextwindowSequence = LONG_WINDOW;

  blockSwitchingControl->attack     = 0;
  blockSwitchingControl->lastattack = 0;
  return (0);
}

void mp3BlockSwitchingDelete(BLOCK_SWITCHING_HANDLE h)
{
  if (h) mp3Free(h);
}

#ifndef P4_CODE
static int 
mp3BlockSwitching_NoOpt(BLOCK_SWITCHING_HANDLE blockSwitchingControl,
			const float *timeSignal)
{
  int i;
  int w;

  unsigned int nrgCritWin2, nrgCritWin1, nrgCritWin0;
  ALIGN_16_BYTE unsigned int partialCrit[NO_OF_ATTACK_CRITERIA];

  sendDebout( "blockSwitch", FRAME_LEN_LONG, 2, "time",
	      MTV_FLOAT, timeSignal );

  /* save window energy from last frame */
  for (w = 0; w < BLOCK_SWITCH_WINDOWS; w++)
  {
    blockSwitchingControl->winNrgF[0][w] = blockSwitchingControl->winNrgF[1][w];
  }

  /* calculate energy per subwindow in filtered time-signal */
  for (w = 0; w < BLOCK_SWITCH_WINDOWS; w++)
  {  
    int offset = w * BLOCK_SWITCH_WINDOW_LEN;
    
	float sum = 0;

    for (i = offset; i < offset + BLOCK_SWITCH_WINDOW_LEN; i++)
    {
      float y = IIRnextSample(timeSignal[2*i],
                              blockSwitchingControl->iirStates);
      sum += y*y;
    }
    blockSwitchingControl->winNrgF[1][w] = sum;
  }


  /* Now calculate the attack-criteria */

  /* reset all partial criteria */
  for (i = 0; i < NO_OF_ATTACK_CRITERIA; i++)
    partialCrit[i] = FALSE;

  /* reset all combined criteria */
  nrgCritWin0 = FALSE;
  nrgCritWin1 = FALSE;
  nrgCritWin2 = FALSE;

  /* reset attack */
  blockSwitchingControl->attack = FALSE;

  
  /*
   printf( "blockSwitching:  %4.2E  %4.2E  %4.2E %4.2E\n",
           blockSwitchingControl->winNrgF[1][0],
           blockSwitchingControl->winNrgF[1][1],
           blockSwitchingControl->winNrgF[1][2],
           blockSwitchParam.minWindowNrg );
  */

  /* Energie Criterion for newest Window (2) */
  /* EF2(T) > F1 * EF1(T), or 1/F1*EF2(T) > EF1(T) */
  if (blockSwitchParam.attackFac[0] * blockSwitchingControl->winNrgF[1][2] > blockSwitchingControl->winNrgF[1][1])
    partialCrit[0] = TRUE;

  /* EF2(T) > F2 * EF0(T), or 1/F2*EF2(T) > EF0(T) */
  if (blockSwitchParam.attackFac[1] * blockSwitchingControl->winNrgF[1][2] > blockSwitchingControl->winNrgF[1][0])
    partialCrit[1] = TRUE;

  /* EF2(T) > minNrgFiltered */
  if (blockSwitchingControl->winNrgF[1][2] > blockSwitchParam.minWindowNrg)
    partialCrit[2] = TRUE;

  /* Energie Criterion for mid Window (1) */
  /* EF1(T) > F1 * EF0(T), or 1/F1*EF1(T) > EF0(T) */
  if (blockSwitchParam.attackFac[0] * blockSwitchingControl->winNrgF[1][1] > blockSwitchingControl->winNrgF[1][0])
    partialCrit[3] = TRUE;

  /* EF1(T) > F2 * EF2(T-1), or 1/F2*EF1(T) > EF2(T-1) */
  if (blockSwitchParam.attackFac[1] * blockSwitchingControl->winNrgF[1][1] > blockSwitchingControl->winNrgF[0][2])
    partialCrit[4] = TRUE;

  /* EF1(T) > minNrgFiltered */
  if (blockSwitchingControl->winNrgF[1][1] > blockSwitchParam.minWindowNrg)
    partialCrit[5] = TRUE;

  /* Energie Criterion for older Window (0) */
  /* EF0(T) > F1 * EF2(T-1), or 1/F1*EF0(T) > EF2(T-1) */
  if (blockSwitchParam.attackFac[0] * blockSwitchingControl->winNrgF[1][0] > blockSwitchingControl->winNrgF[0][2])
    partialCrit[6] = TRUE;

  /* EF0(T) > F2 * EF1(T-1), or 1/F2*EF0(T) > EF1(T-1) */
  if (blockSwitchParam.attackFac[1] * blockSwitchingControl->winNrgF[1][0] > blockSwitchingControl->winNrgF[0][1])
    partialCrit[7] = TRUE;

  /* EF0(T) > minNrgFiltered */
  if (blockSwitchingControl->winNrgF[1][0] > blockSwitchParam.minWindowNrg)
    partialCrit[8] = TRUE;

  /* Combine Energie Criterion for newest Window (2) */
  if ((partialCrit[0] && partialCrit[1]) && partialCrit[2])
    nrgCritWin2 = TRUE;

  /* Combine Energie Criterion for mid Window (1) */
  if ((partialCrit[3] && partialCrit[4]) && partialCrit[5])
    nrgCritWin1 = TRUE;

  /* Combine Energie Criterion for older Window (0) */
  if ((partialCrit[6] && partialCrit[7]) && partialCrit[8])
    nrgCritWin0 = TRUE;

  /* Calculate attack */
  if (nrgCritWin2 || nrgCritWin1 || nrgCritWin0)
    blockSwitchingControl->attack = TRUE;

  /*
     Check if attack spreads over frame border
   */
  if ((!blockSwitchingControl->attack) && (blockSwitchingControl->lastattack)) 
  {
    /* EF0(T) > minNrgFiltered */
    /* EF0(T) > F1 * EF1(T), or 1/F1*EF0(T) > EF1(T) */
    if ((blockSwitchingControl->winNrgF[1][0] > blockSwitchParam.minWindowNrg) &&
  (blockSwitchParam.attackFac[0] * blockSwitchingControl->winNrgF[1][0] > blockSwitchingControl->winNrgF[1][1]))
      blockSwitchingControl->attack = TRUE;

    /* EF2(T-1) > minNrgFiltered */
    /* EF2(T-1) > F1 * EF0(T), or 1/F1*EF2(T-1) > EF0(T) */
    if ((blockSwitchingControl->winNrgF[0][2] > blockSwitchParam.minWindowNrg) &&
  (blockSwitchParam.attackFac[0] * blockSwitchingControl->winNrgF[0][2] > blockSwitchingControl->winNrgF[1][0]))
      blockSwitchingControl->attack = TRUE;

    blockSwitchingControl->lastattack = FALSE;
  } 
  else 
  {
    blockSwitchingControl->lastattack = blockSwitchingControl->attack;
  }

  blockSwitchingControl->windowSequence = blockSwitchingControl->nextwindowSequence;

  if (blockSwitchingControl->attack)
  {
    blockSwitchingControl->nextwindowSequence = SHORT_WINDOW;

    if (blockSwitchingControl->windowSequence == LONG_WINDOW)
      blockSwitchingControl->windowSequence = START_WINDOW;
    if (blockSwitchingControl->windowSequence == STOP_WINDOW)
      blockSwitchingControl->windowSequence = SHORT_WINDOW;
  }
  else
  {
    if (blockSwitchingControl->windowSequence == SHORT_WINDOW)
      blockSwitchingControl->nextwindowSequence = STOP_WINDOW;
    else
      {
	blockSwitchingControl->nextwindowSequence = LONG_WINDOW;
      }
  }

  return (TRUE);
}

int (*mp3BlockSwitching)(BLOCK_SWITCHING_HANDLE blockSwitchingControl,
			 const float *timeSignal) = mp3BlockSwitching_NoOpt;
#endif


__inline static float
IIRnextSample(const float x, float states[])
{
  /*
    IIR filter
    numerator in coeff[2] and coeff[3],
    denominator in coeff[0] and coeff[1]
   */

  float y = hiPassCoeff[1]*(x - states[0]) - hiPassCoeff[0] * states[1];

  states[0] = x;
  states[1] = y;

  return y;
}              

ALIGN_16_BYTE static const int synchronizedBlockTypeTable[4][4] =
{
/*                  LONG_WINDOW    START_WINDOW  SHORT_WINDOW  STOP_WINDOW */
/* LONG_WINDOW  */  {LONG_WINDOW,  START_WINDOW, SHORT_WINDOW, STOP_WINDOW},
/* START_WINDOW */  {START_WINDOW, START_WINDOW, SHORT_WINDOW, SHORT_WINDOW},
/* SHORT_WINDOW */  {SHORT_WINDOW, SHORT_WINDOW, SHORT_WINDOW, SHORT_WINDOW},
/* STOP_WINDOW  */  {STOP_WINDOW,  SHORT_WINDOW, SHORT_WINDOW, STOP_WINDOW}
};


/* synchronize window sequences of left and right channel. Output is
   through windowSequence */

int 
mp3SyncBlockSwitching(BLOCK_SWITCHING_HANDLE blockSwitchingControlLeft,
                      BLOCK_SWITCHING_HANDLE blockSwitchingControlRight,
                      const int nChannels,
                      const int commonWindow,
                      int      *windowSequenceLeft,
                      int      *windowSequenceRight)
{
  int patchType = LONG_WINDOW;

  if (nChannels == 1)   /* Mono */
  {
    *windowSequenceLeft = blockSwitchingControlLeft->windowSequence;
  }
  else if (!commonWindow) /* Dual Channel */
  {
    *windowSequenceLeft  = blockSwitchingControlLeft->windowSequence;
    *windowSequenceRight = blockSwitchingControlRight->windowSequence;
  }
  else /* Stereo common Window */
  {

    /* could be better with a channel loop (need a handle to psy_data) */
    /* get suggested Block Types and synchronize */
    patchType = synchronizedBlockTypeTable[patchType][blockSwitchingControlLeft->windowSequence];
    patchType = synchronizedBlockTypeTable[patchType][blockSwitchingControlRight->windowSequence];

    /* Set synchronized Blocktype */
    *windowSequenceLeft  = *windowSequenceRight =
      blockSwitchingControlLeft->windowSequence  =
      blockSwitchingControlRight->windowSequence = patchType;
  }       /*endif Mono or Stereo */
  return (TRUE);
}
