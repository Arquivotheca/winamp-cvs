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
*   $Id: interface.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mathlib.h"
#include "mp3alloc.h"
#include "psy_const.h"
#include "interface.h"
#include "mp3enc.h"
#include "psy_main.h"

void BuildInterface(const MDCT_SPECTRUM    *mdctSpectrum,
                    const SFB_THRESHOLD    *sfbThreshold,
                    const SFB_ENERGY       *sfbEnergy,
                    const SFB_ENERGY       *sfbEnergyMS,
                    const int               windowSequence,
                    const int               sfbCnt,
                    const int               sfbActive,
                    const int              *sfbOffset,
                    struct PSY_OUT_CHANNEL *psyOutCh) /* output */
{ 
  /*
    copy values to psyOut
  */
  psyOutCh->windowSequence = windowSequence;
  psyOutCh->nWindows       = (windowSequence != SHORT_WINDOW) ? 1 : TRANS_FAC ;

  if (windowSequence != SHORT_WINDOW)
  {
    psyOutCh->sfbCnt         = sfbCnt;
    psyOutCh->sfbActive      = sfbActive;
    copyFLOAT(mdctSpectrum->Long, psyOutCh->mdctSpectrum, FRAME_LEN_LONG);
    copyINT  (sfbOffset,          psyOutCh->sfbOffsets,   sfbCnt+1);
    copyFLOAT(sfbEnergy->Long,    psyOutCh->sfbEnergy,    sfbCnt);
    copyFLOAT(sfbEnergyMS->Long,  psyOutCh->sfbEnergyMS,  sfbCnt);
    copyFLOAT(sfbThreshold->Long, psyOutCh->sfbThreshold, sfbCnt);
  }
  else
  {
    int i;
    int sfb;
    int wnd;
    int offsetAccu = 0;

    psyOutCh->sfbCnt         = sfbCnt    * TRANS_FAC;
    psyOutCh->sfbActive      = sfbActive * TRANS_FAC;

    for (sfb=0; sfb < sfbCnt; sfb++) 
    {
      int sfbWidth = sfbOffset[sfb+1] - sfbOffset[sfb];

      for (wnd = 0; wnd < TRANS_FAC; wnd++)
      {
        psyOutCh->sfbOffsets[TRANS_FAC*sfb+wnd] = offsetAccu;
        offsetAccu += sfbWidth;
      }
    }
    psyOutCh->sfbOffsets[TRANS_FAC*sfbCnt] = offsetAccu; /* set sentinel */

    /* re-group spectrum */
    i = 0;
    for (sfb = 0; sfb < sfbCnt; sfb++)
    {
      for (wnd = 0; wnd < TRANS_FAC; wnd++) 
      {
        int line ;
        for (line = sfbOffset[sfb]; line < sfbOffset[sfb+1]; line++)
        {
          psyOutCh->mdctSpectrum[i++] = mdctSpectrum->Short[wnd][line];
        }
      }
    }

    /* re-group energy and thresholds */
    i = 0;
    for (sfb = 0; sfb < sfbCnt; sfb++)
    {
      for (wnd = 0; wnd < TRANS_FAC; wnd++) 
      {
        psyOutCh->sfbEnergy[i]    = sfbEnergy->Short[wnd][sfb];
        psyOutCh->sfbEnergyMS[i]  = sfbEnergyMS->Short[wnd][sfb];
        psyOutCh->sfbThreshold[i] = sfbThreshold->Short[wnd][sfb];
        i++;
      }
    }
  }
}



/*****************************************************************************

    functionname: CreatePsyList
    description:  creates a list of psyOut-structures for storage
                  of projection-data

*****************************************************************************/
int CreatePsyList( struct PSY_OUT_LIST **firstPsyOut,
                   struct PSY_OUT_LIST **lastPsyOut,
                   struct PSY_OUT_LIST **qcPsyOut,
                   const struct MP3ENC_CONFIG* pConfig,
                   int predictGranules )
{
  int i;
  int error = 0;
  struct PSY_OUT_LIST *temp = 0;
  for( i=0; i<predictGranules; ++i )
    {
      if ( 0 == i )  /* first item */
        {
          temp = (struct PSY_OUT_LIST *)mp3Calloc( 1, sizeof(struct PSY_OUT_LIST) );
          error = PsyOutNew( &temp->psyOut );
          if ( error )
            break;
          error = CreateStereoPreProcessing( &(temp->psyOut->hStereoPrePro),
                                             pConfig->nChannelsOut,
                                             pConfig->bitRate - pConfig->ancillary.anc_Rate,
                                             pConfig->sampleRate,
                                             pConfig->stPreProFlag);
          if ( error )
            break;
          temp->prev = 0;
          *firstPsyOut = temp;
        }
      else
        {
          temp->next = (struct PSY_OUT_LIST *)mp3Calloc( 1, sizeof(struct PSY_OUT_LIST) );
          temp->next->prev = temp;
          temp = temp->next;
          if( i <= predictGranules/2 )
            {
              error = PsyOutNew( &temp->psyOut );
              if ( error )
                break;
              CreateStereoPreProcessing( &(temp->psyOut->hStereoPrePro),
                                         pConfig->nChannelsOut,
                                         pConfig->bitRate,
                                         pConfig->sampleRate,
                                         pConfig->stPreProFlag);
              if ( error )
                break;
            }
          else
            temp->psyOut = 0;
        }

      if( i == predictGranules/2 )
        *qcPsyOut = temp;

      temp->psyOutValid = 0;
#ifdef IISMP3_USE_THREADS
      temp->numAncDataBytes = 0;
#endif
    }
  temp->next = 0;
  *lastPsyOut = temp;
  return error;
}


/*****************************************************************************

    functionname: DelPsyList
    description:  frees memory allocated by the list of psyOut-structures

*****************************************************************************/
void DelPsyList( struct PSY_OUT_LIST *firstPsyOut )
{
  struct PSY_OUT_LIST *temp, *temp2;

  for( temp=firstPsyOut ; temp ; temp=temp2 )
    {
      temp2 = temp->next;
      if( temp->psyOut )
        {
          /* free stereo pre processsing */
          DeleteStereoPreProcessing( temp->psyOut->hStereoPrePro );

          /* free psyOut item */
          PsyOutDelete( temp->psyOut );
        }
      mp3Free( temp );
    }
}

/*****************************************************************************

    functionname: FeedPsyList
    description:  move last item to the beginning of the list
                  and modify pointers

*****************************************************************************/
void FeedPsyList( struct PSY_OUT_LIST **firstPsyOut,
                  struct PSY_OUT_LIST **lastPsyOut,
                  struct PSY_OUT_LIST **qcPsyOut )
{
  (*lastPsyOut)->psyOut = (*qcPsyOut)->psyOut;
  (*qcPsyOut)->psyOut   = 0;
  (*qcPsyOut)->psyOutValid = 0;

  *qcPsyOut             = (*qcPsyOut)->prev;
  (*lastPsyOut)->next   = *firstPsyOut;
  (*firstPsyOut)->prev  = *lastPsyOut;
  *firstPsyOut          = *lastPsyOut;
  *lastPsyOut           = (*firstPsyOut)->prev;
  (*lastPsyOut)->next   = 0;
  (*firstPsyOut)->prev  = 0;
}
