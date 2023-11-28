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
*   $Id: block_switch_p4.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $                             *
*   author:   W. Fiesel                                                                        *
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

#ifdef P4_INTRINSIC
#include "xmmintrin.h"
#endif


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
  /* minWindowEnergy */
  47110000.0f   /* TO DO minimal window Energie filtered */
};

static const float hiPassCoeff[2] = {-0.5095f, 0.7548f};

/****************** Routines ****************************/
#ifdef P4_INTRINSIC
static void fastIIREnergy(const float *timeSignal,
						  BLOCK_SWITCHING_HANDLE blockSwitchingControl)
{

	ALIGN_16_BYTE float iirOut[BLOCK_SWITCH_WINDOW_LEN*BLOCK_SWITCH_WINDOWS]; 
	float state0, state1, coeff0, coeff1;
	float x,y;
	int i,w;

	/* read last filter state */
	state0 = blockSwitchingControl->iirStates[0];
	state1 = blockSwitchingControl->iirStates[1];
	coeff0 = hiPassCoeff[0];
	coeff1 = hiPassCoeff[1];

	for (i = 0; i < BLOCK_SWITCH_WINDOW_LEN*BLOCK_SWITCH_WINDOWS; i++)
	{	
		x = timeSignal[2*i];
		y = coeff1*(x - state0) - coeff0 * state1;
		iirOut[i] = y;

		state0 = x;
		state1 = y;		
	}
	/*  save last filter state */
	blockSwitchingControl->iirStates[0] = state0;
	blockSwitchingControl->iirStates[1] = state1;


	/* get energy for each short block */
	for (w = 0; w < BLOCK_SWITCH_WINDOWS; w++)
	{  
		int offset = w * BLOCK_SWITCH_WINDOW_LEN;
		blockSwitchingControl->winNrgF[1][w] = 
		dotFLOAT(&iirOut[offset], &iirOut[offset], BLOCK_SWITCH_WINDOW_LEN); 
	}
}
#endif


#ifdef P4_INTRINSIC
static int 
mp3BlockSwitching_Opt(BLOCK_SWITCHING_HANDLE blockSwitchingControl,
		      const float *timeSignal)
{
  int i;
  int w;

  unsigned int nrgCritWin2, nrgCritWin1, nrgCritWin0;
  ALIGN_16_BYTE unsigned int partialCrit[NO_OF_ATTACK_CRITERIA];

  /* save window energy from last frame */
  for (w = 0; w < BLOCK_SWITCH_WINDOWS; w++)
  {
    blockSwitchingControl->winNrgF[0][w] = blockSwitchingControl->winNrgF[1][w];
  }

  fastIIREnergy(timeSignal,
		blockSwitchingControl);

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
  if (partialCrit[0] && partialCrit[1] && partialCrit[2])
    nrgCritWin2 = TRUE;

  /* Combine Energie Criterion for mid Window (1) */
  if (partialCrit[3] && partialCrit[4] && partialCrit[5])
    nrgCritWin1 = TRUE;

  /* Combine Energie Criterion for older Window (0) */
  if (partialCrit[6] && partialCrit[7] && partialCrit[8])
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
  }
  else
  {
    blockSwitchingControl->nextwindowSequence = LONG_WINDOW;
  }

  if (blockSwitchingControl->nextwindowSequence == SHORT_WINDOW)
  {
    if (blockSwitchingControl->windowSequence == LONG_WINDOW)
      blockSwitchingControl->windowSequence = START_WINDOW;
    if (blockSwitchingControl->windowSequence == STOP_WINDOW)
      blockSwitchingControl->windowSequence = SHORT_WINDOW;
  }

  if (blockSwitchingControl->nextwindowSequence == LONG_WINDOW)
  {
    if (blockSwitchingControl->windowSequence == SHORT_WINDOW)
      blockSwitchingControl->nextwindowSequence = STOP_WINDOW;
  }
  return (TRUE);
}


#ifdef P4_CODE
int (*mp3BlockSwitching) (BLOCK_SWITCHING_HANDLE blockSwitchingControl,
			  const float *timeSignal) = mp3BlockSwitching_Opt;
#endif

void initBlockSwitchSSE(void )
{
    mp3BlockSwitching = mp3BlockSwitching_Opt;
}

#endif
