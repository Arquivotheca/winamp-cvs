/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: aux_func.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $
   Initial author:       E. Allamanche
   contents/description: Auxilliary Functions C-File 

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#include <assert.h>
#include <math.h>
#include <float.h>
/*#include "bastypes.h"*/
/* #include "errorhnd.h" */
/*#include "aux_func.h"*/
#include "mathlib.h"

static const float log2_1=1.442695041f;    /* 1/log(2) for dual logarithm */

/*****************************************************************************

    functionname: EvalPercepEntropyByLine
    description:  Evaluation of perceptual entropy for each line within a band
    returns:     
    input:        
    output:       
    globals:      

*****************************************************************************/
/* HANDLE_ERROR_INFO */
/* EvalPercepEntropyByLine(const float *signal, */
/*                         const float threshold, */
/*                         const int width, */
/*                         float *pe) */
/* { */
/*   int i, n; */
/*   DOUBLE ratio, invThreshold, tmp; */

  
/*   if (threshold <= FLT_MIN) { */
/*     return ERROR(CDI," threshold <= FLT_MIN"); */
/*   } */
/*   if (width <= 0){ */
/*     return ERROR(CDI," zero sfb width"); */
/*   } */

/*   tmp = 1.0; */
/*   n = 0; */
/*   invThreshold = (float) (width) / threshold; */
  
/*   for (i=0; i<width; i++) { */
/*     ratio = ((DOUBLE)(signal[i]*signal[i]))*invThreshold; */
/*     if (ratio > 1.0) { */
/*       if (ratio>1e9) ratio = 1e9;  */
/*       if (tmp>1e290) tmp = 1e290; */
/*       tmp *= ratio; */
/*       n++; */
/*     } */
/*   } */

/*   *pe = (float)(0.5f*log2_1*log(tmp)) + (float)n; */

/*   return (noError); */
/* } */


/*****************************************************************************

    functionname: FreqToBandWithRounding
    description:  Returns index of nearest band border
    returns:
    input:        frequency, sampling frequency, total number of bands,
                  table of band borders
    output:
    globals:

*****************************************************************************/
int FreqToBandWithRounding(float freq, float fs, int numOfBands,
                           const int *bandStartOffset)
{
  int lineNumber, band;

  assert(freq >= 0);

  lineNumber =  (int)(freq / fs * 2.0f * 
                      (float)bandStartOffset[numOfBands] + 0.5f);

  /* freq > fs/2 */
  if (lineNumber >= bandStartOffset[numOfBands])
    return numOfBands;

  /* find band the line number lies in */
  for (band=0; band<numOfBands; band++) {
    if (bandStartOffset[band+1]>lineNumber) break;
  }

  /* round to nearest band border */
  if (lineNumber - bandStartOffset[band] >
      bandStartOffset[band+1] - lineNumber )
    {
      band++;
    }

  return(band);
}

int BandToFreqWithRounding(int band, float fs, int numOfLines,
                           const int *bandOffsetTable)
{
  int line;
  int freq;
  line = bandOffsetTable[band]-1;
  freq = (int)(((float)line/(float)numOfLines )* (fs/2) + 0.5f);
  return freq;  

}
/*****************************************************************************

    functionname: FreqToBandCut
    description:  Mapping from frequency to band, cutting highest band
    returns:     
    input:        
    output:       
    globals:      

*****************************************************************************/
int FreqToBandCut (float freq, float fs, int numOfBands, 
                   const int *bandStartOffset)
{
  int lineNumber, band;
  
  assert(freq >= 0);

  lineNumber =  (int)(freq / fs * 2.0f * 
                      (float)bandStartOffset[numOfBands] + 0.5f);

  for(band=0; band<numOfBands; band++) {
    if ( lineNumber < bandStartOffset[band+1] ) break;
  }
  return(band);
}

/*****************************************************************************

    functionname: BuildIdxUp
    description:  writes an upward sorted index of float vector elements
                  this is an easy but slow algorithm, should not be used
                  for long vectors (i.e. >~30 elements)
    returns:      -
    input:        float vector of elements size
    output:       sorted index vector

*****************************************************************************/
void BuildIdxUp (float *inData, int *idx, int elements) {
  int i,j;
  float a;

  /* init idx */
  for (j=0;j<elements; j++) {
    idx[j]=j;
  }
   


  for (j=1;j<elements; j++) {
    a = inData[ idx[j] ];
    i=j-1;
    
    while( i>=0 && inData[ idx[i] ]>a ) {
      idx[i+1] = idx[i];
      i--;
    }

    idx[i+1] = j;
  }

}
