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
*   $Id: is_stereo.c,v 1.1 2007/05/29 16:02:28 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "is_stereo.h"
#include "mathmac.h"
#include "mathlib.h"
#include "psy_const.h"
#include "psy_types.h"

#include "utillib.h"

#ifdef PLOTMTV
static float atan2_arr[576];
#endif

/*****************************************************************************

    functionname: CalcIntensityPositions
    description:  determine intensity positions and intensity limit.
    returns:      intensity limit
    input:        spectrum, energies, blocktype, config data
    output:       sfb-wise position, helper values

*****************************************************************************/

int CalcIntensityPositions(const int    mpegVersion,
                           const float *mdctSpectrumLeft,
                           const float *mdctSpectrumRight,
                           const float *sfbEnergyLeft,
                           const float *sfbEnergyRight,
                           const float *sfbThresholdLeft,
                           const float *sfbThresholdRight,
                           const int   *sfbOffset,
                           const int    sfbLimitLow,
                           const int    sfbActive,
                           const int    fIsShort,
                           const float  D2max,
                           int         *sfbIsPosition,
                           const int   *sfbPreviousIsPosition,
                           float       *sfbIsDirX,
                           float       *sfbIsDirY,
                           float       *sfbCrossProduct)
{
  static const float attTab[3][2][31] =
  {
    /* MPEG 1 */
    {
/*R*/ {0.000000f, 0.258819f, 0.500000f, 0.707107f,
       0.866025f, 0.965926f, 1.000000f},
/*L*/ {1.000000f, 0.965926f, 0.866025f, 0.707107f,
       0.500000f, 0.258819f, 0.000000f},
    },
    /* MPEG 2 */
    {
      {
        7.071E-01f, 5.774E-01f, 8.165E-01f, 4.472E-01f,
        8.944E-01f, 3.333E-01f, 9.428E-01f, 2.425E-01f,
        9.701E-01f, 1.741E-01f, 9.847E-01f, 1.240E-01f,
        9.923E-01f, 8.805E-02f, 9.961E-01f, 6.238E-02f,
        9.981E-01f, 4.415E-02f, 9.990E-01f, 3.123E-02f,
        9.995E-01f, 2.209E-02f, 9.998E-01f, 1.562E-02f,
        9.999E-01f, 1.105E-02f, 9.999E-01f, 7.812E-03f,
        1.000E+00f, 5.524E-03f, 1.000E+00f,
      },
      {
        7.071E-01f, 8.165E-01f, 5.774E-01f, 8.944E-01f,
        4.472E-01f, 9.428E-01f, 3.333E-01f, 9.701E-01f,
        2.425E-01f, 9.847E-01f, 1.741E-01f, 9.923E-01f,
        1.240E-01f, 9.961E-01f, 8.805E-02f, 9.981E-01f,
        6.238E-02f, 9.990E-01f, 4.415E-02f, 9.995E-01f,
        3.123E-02f, 9.998E-01f, 2.209E-02f, 9.999E-01f,
        1.562E-02f, 9.999E-01f, 1.105E-02f, 1.000E+00f,
        7.812E-03f, 1.000E+00f, 5.524E-03f,
      }
    },
    /* MPEG 2.5 == MPEG2 */
    {
      {
        7.071E-01f, 5.774E-01f, 8.165E-01f, 4.472E-01f,
        8.944E-01f, 3.333E-01f, 9.428E-01f, 2.425E-01f,
        9.701E-01f, 1.741E-01f, 9.847E-01f, 1.240E-01f,
        9.923E-01f, 8.805E-02f, 9.961E-01f, 6.238E-02f,
        9.981E-01f, 4.415E-02f, 9.990E-01f, 3.123E-02f,
        9.995E-01f, 2.209E-02f, 9.998E-01f, 1.562E-02f,
        9.999E-01f, 1.105E-02f, 9.999E-01f, 7.812E-03f,
        1.000E+00f, 5.524E-03f, 1.000E+00f,
      },
      {
        7.071E-01f, 8.165E-01f, 5.774E-01f, 8.944E-01f,
        4.472E-01f, 9.428E-01f, 3.333E-01f, 9.701E-01f,
        2.425E-01f, 9.847E-01f, 1.741E-01f, 9.923E-01f,
        1.240E-01f, 9.961E-01f, 8.805E-02f, 9.981E-01f,
        6.238E-02f, 9.990E-01f, 4.415E-02f, 9.995E-01f,
        3.123E-02f, 9.998E-01f, 2.209E-02f, 9.999E-01f,
        1.562E-02f, 9.999E-01f, 1.105E-02f, 1.000E+00f,
        7.812E-03f, 1.000E+00f, 5.524E-03f,
      }
    }
  };

  int sfb;
  int illegalPosition = (mpegVersion == 0 ? 7 : 31);

  for (sfb = sfbLimitLow; sfb < sfbActive ; sfb++)
  {
    int sfbWidth = sfbOffset[sfb+1]-sfbOffset[sfb];
    float R = sfbEnergyRight[sfb];
    float L = sfbEnergyLeft[sfb];
    float Q = 2.0f*dotFLOAT(mdctSpectrumLeft + sfbOffset[sfb],
                            mdctSpectrumRight+ sfbOffset[sfb],
                            sfbWidth);

    if (sfb != (fIsShort ? MAX_SFB_SHORT-1 : MAX_SFB_LONG-1))
    {
      int dir, bestDir = 3 ;
      float D2min = FLT_MAX ;

      /* find intensity direction that minimizes error */
#if 1 /* search for the direction */
      for (dir = 0; dir < illegalPosition; dir++ )
#else /* use energy criterion */
      dir = (int)(atan2(L,R) * 12.0/M_PI + 0.5);
#endif
      {
        /* (Nx, Ny) is the normal vector on the intensity direction */
        float Nx =  attTab[mpegVersion][0][dir];
        float Ny = -attTab[mpegVersion][1][dir];
#if 0  /* bugfix ? */
        float C  =  Nx*Ny * Q;
#else
        /* this seems to be more correct, as it maps a "out of phase" direction 
           to its accordant "in phase" direction instead of the nearest "in phase" direction 
           i.e a outmost direction */
        float C  =  (float) (Nx*Ny * fabs(Q));
#endif
        /* D2 is the distance^2 summed over all lines in this sfb */
        float D2 = Nx*Nx * R + Ny*Ny * L + C;
        if (D2 < D2min)
        {
          D2min = D2;
          bestDir = dir;
        }
      }

	  {
		/* for low Energies set IS position to middle Position  */
		static const float lowNrg = 1.e-4f * NORM_PCM_ENERGY;  /* this constant should be reviewed, 
								but for now seems to be reasonable*/
		if( R<lowNrg && L<lowNrg ) {
		if (mpegVersion == MPEG1) 
			bestDir = 3;
		else
			bestDir = 0;
		}
	  }

      /* limit intensity direction vector fluctuation */

      if (mpegVersion == MPEG1)
      {
#if 1
        /* limit intensity position */
        /* for MPEG1 use outmost IS position only, 
           if there is almost no energy in the oppposing channel */
        static const float energyThreshold = 0.0001f;
        if( (bestDir == 0) && ( R*energyThreshold < L) )
          bestDir = 1;
        if( (bestDir == 6) && ( L*energyThreshold < R) )
          bestDir = 5;
#ifdef DEBUG
        if( (bestDir==0) || (bestDir==6) )
          fprintf(stderr,"\n!!extreme IS Position in band: %d", sfb);
#endif
#endif

        if (bestDir < sfbPreviousIsPosition[sfb]-1)
          bestDir = sfbPreviousIsPosition[sfb]-1;
        else if (bestDir > sfbPreviousIsPosition[sfb]+1)
          bestDir = sfbPreviousIsPosition[sfb]+1;
      }
      else /* MPEG2, MPEG2.5 */
      {
        /* map is position to contigous range, then back */
        int bd = ((bestDir % 2) ? -(bestDir+1) : bestDir ) / 2;
        int lbd = ((sfbPreviousIsPosition[sfb] % 2) ? -(sfbPreviousIsPosition[sfb]+1) : sfbPreviousIsPosition[sfb] ) / 2;

        if (bd < lbd-4)
          bd = lbd-4;
        else if (bd > lbd+4)
          bd = lbd+4;

        bestDir = (bd < 0) ? -bd*2 - 1 : bd * 2;
        assert(bestDir >=0 && bestDir < illegalPosition);
      }

      sfbIsPosition[sfb]   = bestDir ;
      sfbIsDirX[sfb]       = attTab[mpegVersion][1][bestDir];
      sfbIsDirY[sfb]       = attTab[mpegVersion][0][bestDir];
      sfbCrossProduct[sfb] = sfbIsDirX[sfb]*sfbIsDirY[sfb] * Q;
    }
    else
    {
      /* copy sfb20 position to sfb21 */
      sfbIsPosition[sfb]   = sfbIsPosition[sfb-1] ;
      sfbIsDirX[sfb]       = sfbIsDirX[sfb-1] ;
      sfbIsDirY[sfb]       = sfbIsDirY[sfb-1] ;
      sfbCrossProduct[sfb] = sfbIsDirX[sfb-1]*sfbIsDirY[sfb-1] * Q;
    }

    assert(sfbIsPosition[sfb] >= 0 && sfbIsPosition[sfb] < illegalPosition);
  }

#ifdef PLOTMTV
  for (sfb = sfbOffset[sfbLimitLow]; sfb < sfbOffset[sfbActive]; sfb++)
  {
    atan2_arr[sfb] = mdctSpectrumLeft[sfb] < 0 ? atan2(-mdctSpectrumLeft[sfb],-mdctSpectrumRight[sfb]) :atan2(mdctSpectrumLeft[sfb],mdctSpectrumRight[sfb]) ;
  }

  sendDebout("intensity", sfbOffset[sfbLimitLow+4]-sfbOffset[sfbLimitLow], 1,
             "orig",
             MTV_FLOAT, atan2_arr + sfbOffset[sfbLimitLow]);

  for (sfb = sfbLimitLow; sfb < sfbActive; sfb++)
  {
    int i;
    float t = atan2(sfbIsDirY[sfb],sfbIsDirX[sfb]);
    for (i = sfbOffset[sfb]; i < sfbOffset[sfb+1]; i++)
      atan2_arr[i] = t;
  }

  sendDebout("intensity", sfbOffset[sfbLimitLow+4]-sfbOffset[sfbLimitLow], 1,
             "code",
             MTV_FLOAT, atan2_arr + sfbOffset[sfbLimitLow]);

  for (sfb = sfbLimitLow; sfb < sfbActive; sfb++)
  {
    int i;
    float t = atan2(sfbEnergyLeft[sfb],sfbEnergyRight[sfb]);
    for (i = sfbOffset[sfb]; i < sfbOffset[sfb+1]; i++)
      atan2_arr[i] = t;
  }

  sendDebout("intensity", sfbOffset[sfbLimitLow+4]-sfbOffset[sfbLimitLow], 1,
             "nrg",
             MTV_FLOAT, atan2_arr + sfbOffset[sfbLimitLow]);

  for (sfb = sfbLimitLow; sfb < sfbActive; sfb++)
  {
    int i;
    float t = atan2(sqrt(sfbEnergyLeft[sfb]),
                    sqrt(sfbEnergyRight[sfb]));
    for (i = sfbOffset[sfb]; i < sfbOffset[sfb+1]; i++)
      atan2_arr[i] = t;
  }

  sendDebout("intensity", sfbOffset[sfbLimitLow+4]-sfbOffset[sfbLimitLow], 1,
             "nrgsqrt",
             MTV_FLOAT, atan2_arr + sfbOffset[sfbLimitLow]);
#endif /* ifdef PLOTMTV */

  /* decide on isLimit */
  for (sfb = sfbActive-1; sfb >= sfbLimitLow ; sfb--)
  {
    /*int sfbWidth = sfbOffset[sfb+1]-sfbOffset[sfb];*/
    float Nx =  sfbIsDirY[sfb];
    float Ny = -sfbIsDirX[sfb];
    float L  =  sfbEnergyLeft[sfb];
    float R  =  sfbEnergyRight[sfb];
    float D2 =  Nx*Nx*R + Ny*Ny*L - sfbCrossProduct[sfb];

    if ( (D2*D2max*Nx) > sfbThresholdRight[sfb] ||
        -(D2*D2max*Ny) > sfbThresholdLeft [sfb] )
    {
      /* we do not code this band as intensity */
      break;
    }
  }

  return sfb + 1;
}

void
AdaptIntensityPositions(const int mpegVersion,
                        const int fLastBlockShort,
                        const int fThisBlockShort,
                        const int lastSfbIsPosition[],
                        int       thisSfbIsPosition[])
{
  int transTable[3][MAX_SFB_SHORT][2] = 
  { 
    { /* MPEG 1 */
      {0,2},{3,5},{5,7},{7,9},{9,11},
      {11,12},{13,14},{14,15},{15,17},
      {17,18},{18,19},{19,20},{20,21}
    }, 
    { /* MPEG 2 */
      {0,1},{2,3},{4,6},{6,8},{8,10},
      {10,12},{12,13},{13,15},{15,16},
      {16,18},{18,19},{19,20},{20,21}
    }, 
    { /* MPEG 2.5 */
      {0,1},{2,3},{4,6},{6,8},{8,10},
      {10,11},{11,13},{13,14},{14,16},
      {16,17},{18,19},{19,20},{20,21}
    }
  };

  IS_POSITION isPrevPos;
  int         i;

  if (fLastBlockShort == fThisBlockShort)  return ; /* no adaptation necessary */

  if (fThisBlockShort) {
    int sfbShort;

    copyINT(lastSfbIsPosition,isPrevPos.Long,MAX_SFB_LONG);

    if (mpegVersion != MPEG1) {
      for(i=0;i<MAX_SFB_LONG;i++) {
        isPrevPos.Long[i] = ((isPrevPos.Long[i] % 2) ? - (isPrevPos.Long[i]+1) : isPrevPos.Long[i] ) / 2;
      }
    }

    /* "compress" directions into short block directions */
    for (sfbShort = 0; sfbShort < MAX_SFB_SHORT; sfbShort++) {
      int   sfbLong;
      float mean = 0.0f;
      int   nCnt = 0;

      for (sfbLong = transTable[mpegVersion][sfbShort][0]; sfbLong <= transTable[mpegVersion][sfbShort][1]; sfbLong++) {
        mean += isPrevPos.Long[sfbLong];
        nCnt++;
      }
      thisSfbIsPosition[sfbShort] = (int) (mean/(float)nCnt + 0.5f);
    }
    
    if (mpegVersion != MPEG1) {
      for(i=0;i<MAX_SFB_SHORT;i++) {
        thisSfbIsPosition[i] =  (thisSfbIsPosition[i] < 0) ? -thisSfbIsPosition[i]*2 - 1 : thisSfbIsPosition[i] * 2;
      }
    }
  }
  else {
    int sfbShort;
    int lastLong =  0;

    copyINT(lastSfbIsPosition,&isPrevPos.Short[0][0],MAX_SFB_SHORT);

    if (mpegVersion != MPEG1) {
      for(i=0;i<MAX_SFB_SHORT;i++) {
        isPrevPos.Short[0][i] = ((isPrevPos.Short[0][i] % 2) ? - (isPrevPos.Short[0][i]+1) : isPrevPos.Short[0][i] ) / 2;
      }
    }

    for (sfbShort = 0; sfbShort < MAX_SFB_SHORT; sfbShort++) {
      int sfbLong;

      if (sfbShort && lastLong == transTable[mpegVersion][sfbShort][0]) {
        thisSfbIsPosition[lastLong] += isPrevPos.Short[0][sfbShort];
        thisSfbIsPosition[lastLong] /= 2;
      }
      else {
        thisSfbIsPosition[transTable[mpegVersion][sfbShort][0]] = isPrevPos.Short[0][sfbShort];
      }

      for (sfbLong = transTable[mpegVersion][sfbShort][0]+1; sfbLong <= transTable[mpegVersion][sfbShort][1]; sfbLong++) {
        lastLong = sfbLong;
        thisSfbIsPosition[sfbLong] = isPrevPos.Short[0][sfbShort];
      }
    }
    
    if (mpegVersion != MPEG1) {
      for(i=0;i<MAX_SFB_LONG;i++) {
        thisSfbIsPosition[i] =  (thisSfbIsPosition[i] < 0) ? -thisSfbIsPosition[i]*2 - 1 : thisSfbIsPosition[i] * 2;
      }
    }
  }
}

void
IsStereoProcessing(const int    mpegVersion,
                   float       *sfbEnergyLeft,     /* modified above isLimit */
                   float       *sfbEnergyRight,    /* modified above isLimit */
                   float       *mdctSpectrumLeft,  /* modified above isLimit */
                   float       *mdctSpectrumRight, /* modified above isLimit */
                   float       *sfbThresholdLeft,  /* modified above isLimit */
                   float       *sfbThresholdRight, /* modified above isLimit */
                   const float *sfbIsDirX,
                   const float *sfbIsDirY,
                   const float *sfbIsCrossProduct,
                   const int    isLimit[TRANS_FAC],
                   const int    sfbActive,
                   const int   *sfbOffset,
                   const int    nWindows)
{
  int sfb;
  int line;
  int win;

  for (win = 0 ; win < nWindows; win++)
  {
    int offsetLine = win * FRAME_LEN_SHORT;
    int offsetSfb  = win * MAX_SFB_SHORT;

    for (sfb = isLimit[win] ; sfb < sfbActive; sfb ++)
    {
      float X = sfbIsDirX[offsetSfb + sfb];
      float Y = sfbIsDirY[offsetSfb + sfb];

      float iRatioLeft, iRatioRight, iRatio;
      float energyL = sfbEnergyLeft [offsetSfb + sfb];
      float energyR = sfbEnergyRight[offsetSfb + sfb];
      /* a little algebra shows the following to be true. */
      float factor2 = (mpegVersion == 0) ? 1.0f + 2.0f*X*Y : max(X*X,Y*Y);
      float factor  = (float)sqrt(factor2);

      float energyI = (Y*Y*energyL + X*X*energyR + sfbIsCrossProduct[offsetSfb + sfb]);

      energyI *= factor2;

      /* project L/R tuple onto intensity direction vector */
      for (line = offsetLine + sfbOffset[sfb] ; line < offsetLine + sfbOffset[sfb+1]; line++)
      {
        float dotProd = ( mdctSpectrumLeft [line] * Y
                         +mdctSpectrumRight[line] * X);

        mdctSpectrumLeft[line]  = dotProd * factor ;
        mdctSpectrumRight[line] = 0.0f;
      }

#if 0
      printf("%f %f\n",energyI,dotFLOAT(mdctSpectrumLeft + offsetLine + sfbOffset[sfb],
                                        mdctSpectrumLeft + offsetLine + sfbOffset[sfb],
                                        sfbOffset[sfb+1]-sfbOffset[sfb]));
#endif

      iRatioLeft  = energyL / sfbThresholdLeft [offsetSfb + sfb];
      iRatioRight = energyR / sfbThresholdRight[offsetSfb + sfb];
      iRatio = max(iRatioLeft,iRatioRight);

      sfbEnergyLeft[offsetSfb + sfb] = energyI;
      sfbEnergyRight[offsetSfb + sfb] = 0.0f ;

      /*
        minimum of L/R ratios * energy of intensity channel
       */

      sfbThresholdLeft[offsetSfb + sfb] = (iRatio != 0.0f) ? energyI / iRatio : 1.0f ;
    }
  }
}
