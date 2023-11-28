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
*   $Id: stprepro.c,v 1.1 2007/05/29 16:02:32 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "mconfig.h"
#include "stprepro.h"
#include "psy_const.h"
#include "mp3alloc.h"
#include "mathlib.h"
#include "utillib.h"

int CreateStereoPreProcessing(HANDLE_STEREO_PREPRO* hStPrePro, 
                              int nChannels, 
                              int bitRate,
                              int sampleRate,
                              int stPreProFlag)
{
   int   error = 0;
   float att = 0.0f;
   int   index = 0;

   if (!stPreProFlag || (nChannels<2)) {
      *hStPrePro = NULL;
      return 0;
   }

   (*hStPrePro)=(HANDLE_STEREO_PREPRO)mp3Calloc(1,sizeof(struct STEREO_PREPRO));
   if ((*hStPrePro)==NULL)
      error = 1;

   (*hStPrePro)->bitsPerFrameRaw=(float)bitRate*576/sampleRate/nChannels;

   (*hStPrePro)->ConstAtt   = 0.0f;

   (*hStPrePro)->stereoAttMax   = 15.0f; /* 12.0f; */ /*  8.0;  */
   (*hStPrePro)->stereoAttResol = 0.5f;

   (*hStPrePro)->stereoAttTable[0] = (float *)mp3Calloc(
          (int)((*hStPrePro)->stereoAttMax/(*hStPrePro)->stereoAttResol)+1, 
          sizeof(float));
   if (((*hStPrePro)->stereoAttTable[0])==NULL)
      error = 1;

   (*hStPrePro)->stereoAttTable[1] = (float *)mp3Calloc(
          (int)((*hStPrePro)->stereoAttMax/(*hStPrePro)->stereoAttResol)+1, 
          sizeof(float));
   if (((*hStPrePro)->stereoAttTable[1])==NULL)
      error = 1;


   (*hStPrePro)->stereoAttTable[0][0] = 1;
   (*hStPrePro)->stereoAttTable[1][0] = 0;

   (*hStPrePro)->stereoAttenuationFlag = 1;
   (*hStPrePro)->stereoAttenuationInc = sampleRate / 128000.0f;/*  0.125;   */

   (*hStPrePro)->stereoAttenuationDec = sampleRate / 1.6e6f; /*   0.04; */

   /* energy ratio thresholds (dB) */

   (*hStPrePro)->SMMin = 5.0f;

   (*hStPrePro)->SMMax = 25.0f;

   /* pe thresholds */
   (*hStPrePro)->PeCrit =  120.0f/* 550.0f */ ; 
   (*hStPrePro)->PeMin =  30.0f /* 250.0f */  ;
   (*hStPrePro)->PeImpactMax = 100.0f;  
   
   (*hStPrePro)->ImpactFactor =  48000.0f*500000.0f/((float)bitRate*(float)bitRate) ;
   
   /* start value of the attenuation index */
   (*hStPrePro)->stereoAttenuation = 0.1f * (*hStPrePro)->stereoAttMax;/*  0.5* (*hStPrePro)->stereoAttMax; */

   att += (*hStPrePro)->stereoAttResol ;

   if (!error) {
      index++;
      while (att<=(*hStPrePro)->stereoAttMax){
         (*hStPrePro)->stereoAttTable[0][index] = 
            0.5f * (1.0f + (float)pow(10.0, 0.05*(-att)));
         (*hStPrePro)->stereoAttTable[1][index]= 
            0.5f * (1.0f - (float)pow(10.0, 0.05*(-att)));
         att += (*hStPrePro)->stereoAttResol;
         index++;
      }
   }

   return (error);
}


void DeleteStereoPreProcessing(HANDLE_STEREO_PREPRO hStPrePro)
{
   if ((hStPrePro)!=NULL) {
      mp3Free((hStPrePro)->stereoAttTable[1]);
      mp3Free((hStPrePro)->stereoAttTable[0]);
      mp3Free((hStPrePro));
      hStPrePro=NULL;
   }
}
/* stereo preprocessing for reducing stereo width :

   Detection:
 -------------
 
   Side channel is to be attenuated with influence of the following parameters:

   - total PE, smoothed over past frames ( hStPrePro->smoothedPeSumSum ): 
      the bigger, the more attenuation

   - bits per frame ( hStPrePro->bitsPerFrameRaw ):
      the bigger, the less attenuation

   - energy ratio side channel/mid channel, 
     smoothed over past frames ( hStPrePro->avgStoM )
      if side channel _very_ strong, attenuate less

   - bitrate:
      the smaller, the more attenuation

   1.) Energy influence ( EnImpact ):

     EnImpact
       ^
       |
     1 |  -------------
       |               \
       |                \
       |                 \          
     0 |                  ---------> SToM 
                      |   |        
                  SMMin   SMMax

   2.) PE influence ( PeImpact ):

      PEFac = hStPrePro->smoothedPeSumSum / hStPrePro->bitsPerFrameRaw * fix_factor

                PeImpact
                  ^
                  |
                  |
      PeImpactMax |                                ___________________
                  |                              /
                  |                              /
                  |                             /
                1 |                            /
                  |                          /
                  |                        /
                  |                   __--
                0 |_______________--- ___________________\
                                  |          |           /  PeFac
                                 PeMin     PeCrit

       
   3.) ImpactFactor ( hStPrePro->ImpactFactor ):

      ImpactFactor = fix_factor / (bitrate^2)

   4.) desired attenuation ( AttAimed ):
    
      AttAimed = ImpactFactor * EnImpact * PeImpact 

   5.) Attenuation = AttAimed, limited by AttenuationInc or AttenuationDec


   Processing:   
 --------------

   Matrix for input signal preprocessing    :
   L' = xL + yR
   R' = yL + xR
   
   M = 0.5  (L' + R') =  0.5 ( ( xL + yR ) +  (yL + xR) ) = 0.5 ( ( x+y)L + (x+y)R) = 0.5 (x+y) (L+R); 
   since M may not be modified by the preprocessing  => x+ y = 1;
   
   S = 0.5  (L' - R') =  0.5 ( ( xL + yR ) -  (yL + xR) ) = 0.5 ( ( x-y) L + (y-x) R) = 0.5 ( ( x-y) L - (x-y) R)= 0.5 (x-y) (L-R)  ; 
   => S is attenuated by x-y or 10*log10(x-y) dB = att ; att < 0 !
   
   att (in dB) = 20*log10(x-y) 
   0.05 att=log10(x-y)
   
   exp( 0.05 att) = x-y
   1             = x+y 
   -------------------
   1+exp(0.05 att) )=2 x
   
   0.5 (1 + exp(0.05 att) = x   (I)
   
   exp(0.05att) -1 = -2y 
   -0.5(exp(0.05att) -1) = y
   
   0.5 ( 1- exp(0.05att)) = y    (II)
   
   
*/
void AdvanceStereoPreStep1(HANDLE_STEREO_PREPRO hStPrePro, 
                           const int nChannels,
                           float *timeData,
                           int granuleLen)
{
   /* inplace operation on inData ! */
   if (hStPrePro != NULL) {

     float SMRatio, StoM;
     float EnImpact, PeImpact, PEFac;
     float Att, AttAimed;
     float DELTA=0.1f;
     
     SMRatio=(hStPrePro->avrgFreqEnergyS+(float)1.0f)/(hStPrePro->avrgFreqEnergyM+(float)1.0f);
     StoM = (float)(10.0f * log10((double)SMRatio));
     
     hStPrePro->avgStoM =
       DELTA * StoM +
       (1-DELTA) * hStPrePro->avgStoM;
     
     /* energy influence: Don't attenuate if S/M too large*/
     
     EnImpact=1;
     if (hStPrePro->avgStoM > hStPrePro->SMMin)
       {
         if (hStPrePro->avgStoM > hStPrePro->SMMax)
           EnImpact = 0;
         else
           EnImpact = (hStPrePro->SMMax - hStPrePro->avgStoM)/(hStPrePro->SMMax - hStPrePro->SMMin);
       };
     
     sendDeboutHist("StPro3","EnImpact",MTV_FLOAT,&EnImpact);
     
     /* PE influence: Do attenuation for large PEs */
     
     PeImpact=0;
     
     PEFac = hStPrePro->smoothedPeSumSum / hStPrePro->bitsPerFrameRaw * 1500.0f;
     
     if ( PEFac > hStPrePro->PeMin )
       {
         PeImpact= ((PEFac - hStPrePro->PeMin) /
                    (hStPrePro->PeCrit - hStPrePro->PeMin));
         PeImpact= PeImpact * PeImpact;
       };
     
     if (PeImpact > hStPrePro->PeImpactMax)
       PeImpact = hStPrePro->PeImpactMax;
     
     sendDeboutHist("StPro","PEFac",MTV_FLOAT,&PEFac); 
     
     sendDeboutHist("StPro4","PeImpact",MTV_FLOAT,&PeImpact); 
      
     AttAimed = EnImpact * PeImpact * hStPrePro->ImpactFactor;
     
     if (AttAimed> hStPrePro->stereoAttMax)
       AttAimed = hStPrePro->stereoAttMax;
     
     Att = AttAimed;
     if (Att > hStPrePro->stereoAttenuation + hStPrePro->stereoAttenuationInc)
       Att= hStPrePro->stereoAttenuation + hStPrePro->stereoAttenuationInc;
     if (Att < hStPrePro->stereoAttenuation - hStPrePro->stereoAttenuationDec)
       Att= hStPrePro->stereoAttenuation - hStPrePro->stereoAttenuationDec;
     
     if (hStPrePro->ConstAtt==0) hStPrePro->stereoAttenuation = Att;
     else hStPrePro->stereoAttenuation = hStPrePro->ConstAtt;
     
     sendDeboutHist("StPro5","Attenuation",MTV_FLOAT,&hStPrePro->stereoAttenuation);
  
     /* perform attenuation of Side Channel 
      */
  
     if (hStPrePro->stereoAttenuationFlag)  {

       if (nChannels==2){
         float helpLVec[FRAME_LEN_LONG];
         float helpRVec[FRAME_LEN_LONG];
         float helpSumVec[FRAME_LEN_LONG];
         
         float lFac,rFac ;
               
         sendDebout( "StPreTimeSig", granuleLen,1,"leftInput",MTV_FLOAT,&timeData[0]);
         sendDebout( "StPreTimeSig", granuleLen,1,"rightInput",MTV_FLOAT,&timeData[1]);
         
         lFac = hStPrePro->stereoAttTable[0][(int)(hStPrePro->stereoAttenuation+0.5f)];
         rFac = hStPrePro->stereoAttTable[1][(int)(hStPrePro->stereoAttenuation+0.5f)];
         smulFLOATflex(lFac, &timeData[0], 2, helpLVec, 1, granuleLen);
         smulFLOATflex(rFac, &timeData[1], 2, helpRVec, 1, granuleLen);
         
         addFLOAT( helpLVec,  helpRVec ,helpSumVec, granuleLen) ;
         
         smulFLOATflex(rFac, &timeData[0], 2, helpLVec, 1, granuleLen);
         smulFLOATflex(lFac, &timeData[1], 2, helpRVec, 1, granuleLen);
         
         addFLOATflex(helpLVec, 1, helpRVec , 1, &timeData[1], 2, granuleLen); 
         copyFLOATflex(helpSumVec, 1, &timeData[0], 2, granuleLen);
         
         sendDebout( "StPreTimeSig", granuleLen,1,"leftOutput",MTV_FLOAT,&timeData[0]);
         sendDebout( "StPreTimeSig", granuleLen,1,"rightOutput",MTV_FLOAT,&timeData[1]);
         
       }
     }
   }
}

void AdvanceStereoPreStep2(struct PSY_OUT *hPsyOut,
                           HANDLE_STEREO_PREPRO hStPrePro)
{
   int x ;
   float DELTA = 0.1f ;

   if (hStPrePro!=NULL){ 
      hStPrePro->avrgFreqEnergyM = 0.0f; /*avrgEn=0.0f;*/
      /* average M energy over freq */
      for (x=0;x<hPsyOut->psyOutChannel[0].sfbCnt;x++) 
         hStPrePro->avrgFreqEnergyM += hPsyOut->psyOutChannel[0].sfbEnergyMS[x];

      hStPrePro->avrgFreqEnergyS=0.0f;
      /* average S energy over freq */
      for (x=0;x<hPsyOut->psyOutChannel[1].sfbCnt;x++) 
         hStPrePro->avrgFreqEnergyS += hPsyOut->psyOutChannel[1].sfbEnergyMS[x];
      
      hStPrePro->smoothedPeSumSum =
        DELTA*0.5f*( hPsyOut->psyOutChannel[0].pe + hPsyOut->psyOutChannel[1].pe );/* + */
        /* (1-DELTA)*hStPrePro->smoothedPeSumSum; */

      sendDeboutHist("StPro","SmoothedPe",MTV_FLOAT,&hStPrePro->smoothedPeSumSum);
   }

}
