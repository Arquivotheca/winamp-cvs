/*
  independent channel concealment
*/
/*
  The concealment routine swaps the spectral data from the previous and the current frame just before 
  the final frequency to time conversion. In case a single frame is corrupted, concealmant interpolates 
  between the last good and the first good frame to create the spectral data for the missing frame.
  If multiple frames are corrupted, concealment implements first a fade out based on slightly modified
  spectral values from the last good frame. As soon as good frames are available, concealmant fades
  in the new spectral data. 
*/
#include <math.h>
#include "conceal.h"
#include "channelinfo.h"
#include "aac_rom.h"
#include "math/FloatFR.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
#define MINUS_3dB 0.70710678f
extern const float sbr_randomPhase[AAC_NF_NO_RANDOM_VAL][2];
static const float fadeFacTable[] = {
  MINUS_3dB,
  MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB,
  MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB * MINUS_3dB
};
void CConcealment_UpdateState (CConcealmentInfo *pConcealmentInfo,
                               int FrameOk);
void CConcealment_ApplyRandomSign (CConcealmentInfo *pConcealmentInfo,
                                   float *spec);
void CConcealment_CalcBandEnergy (float                  *spectrum,
                                  int                     blockType,
                                  int                     samplingRateIndex,
                                  CConcealmentExpandType  ex,
                                  float                  *sfbEnergy);
void CConcealment_InterpolateBuffer (float       *spectrum,
                                     float       *enPrev,
                                     float       *enAct,
                                     int          sfbCnt,
                                     const short *pSfbOffset);
/*
  The function initializes the concealment information.
*/
void CConcealment_Init (CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo)
{
  int i;
  CConcealmentInfo *pConcealmentInfo = &pAacDecoderStaticChannelInfo->ConcealmentInfo;
  
    
   /* pConcealmentInfo->SpectralCoefficient[i] */
  
  for (i = 0; i < FRAME_SIZE; i++) {
    
    pConcealmentInfo->SpectralCoefficient[i] = 0.0;
  }
  
  pConcealmentInfo->iRandomPhase = 0;
  
  pConcealmentInfo->WindowSequence = 0;
  pConcealmentInfo->WindowShape = 0;
  
  pConcealmentInfo->prevFrameOk[0] = pConcealmentInfo->prevFrameOk[1] = 1;
  
  pConcealmentInfo->cntConcealFrame = 0;
  pConcealmentInfo->nFadeInFrames   = 5;
  pConcealmentInfo->nValidFrames    = 5;
  pConcealmentInfo->nFadeOutFrames  = 5;
  pConcealmentInfo->ConcealState    = CConcealment_Ok;
  
}
/*
  The function swaps the data from the current and the previous frame. If an
  error has occured, frame interpolation is performed to restore the missing 
  frame. In case of multiple faulty frames, fade-in and fade-out is applied.
*/
void CConcealment_Apply (CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
                         CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                         char FrameOk)
{
  float sfbEnergyPrev[MAXSFB];
  float sfbEnergyAct[MAXSFB];
  char  tmp1;
  float tmp2;
  int   i;
  CConcealmentInfo *pConcealmentInfo     =  &pAacDecoderStaticChannelInfo->ConcealmentInfo;
  float            *pSpectralCoefficient =  pAacDecoderChannelInfo->aSpectralCoefficient;
  CIcsInfo         *pIcsInfo             = &pAacDecoderChannelInfo->IcsInfo;
  
    
  /* if the current frame is ok, save it (and swap out stored frame, i.e. the stored
     frame will be current output frame. For correct frames, this means that the
     previous frame will be played now. */
  
  if (FrameOk) {
     
    tmp1 = pIcsInfo->WindowShape;
    pIcsInfo->WindowShape = pConcealmentInfo->WindowShape;
    pConcealmentInfo->WindowShape = tmp1;
     
    tmp1 = pIcsInfo->WindowSequence;
    pIcsInfo->WindowSequence = pConcealmentInfo->WindowSequence;
    pConcealmentInfo->WindowSequence = tmp1;
     /* pSpectralCoefficient[]
                    pConcealmentInfo->SpectralCoefficient[]
                 */
    
    for (i = 0; i < FRAME_SIZE; i++) {
      
      tmp2 = pSpectralCoefficient[i];
      pSpectralCoefficient[i] = pConcealmentInfo->SpectralCoefficient[i];
      pConcealmentInfo->SpectralCoefficient[i] = tmp2;
    }
  } else {
     
    pIcsInfo->WindowShape = pConcealmentInfo->WindowShape;
    pIcsInfo->WindowSequence = pConcealmentInfo->WindowSequence;
     /* pSpectralCoefficient[]
                    pConcealmentInfo->SpectralCoefficient[]
                 */
    
    for (i = 0; i < FRAME_SIZE; i++) {
      
      pSpectralCoefficient[i] = pConcealmentInfo->SpectralCoefficient[i];
    }
  }
  
  CConcealment_UpdateState(pConcealmentInfo, FrameOk);
  /* if previous frame was not ok */
   
  if (!pConcealmentInfo->prevFrameOk[1]) {
      
    if (FrameOk && pConcealmentInfo->prevFrameOk[0]) {
        
      if (pIcsInfo->WindowSequence == EightShortSequence) { /* f_(n-2) == EightShortSequence */
        int wnd;
          
        if (pConcealmentInfo->WindowSequence == EightShortSequence) { /* f_n == EightShortSequence */
          int scaleFactorBandsTotal = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Short;
          const short *pSfbOffset   = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Short;
             
           
          pIcsInfo->WindowShape = 1;
          pIcsInfo->WindowSequence = EightShortSequence;
           /* pConcealmentInfo->SpectralCoefficient[wnd * (FRAME_SIZE / MAX_WINDOWS)]
                          pSpectralCoefficient[wnd * (FRAME_SIZE / MAX_WINDOWS)]
                       */
          
          for (wnd = 0; wnd < MAX_WINDOWS; wnd++) {
             
            CConcealment_CalcBandEnergy(&pSpectralCoefficient[wnd * (FRAME_SIZE / MAX_WINDOWS)], /* spec_(n-2) */
                                        EightShortSequence,
                                        pIcsInfo->SamplingRateIndex,
                                        CConcealment_NoExpand,
                                        sfbEnergyPrev);
            
            CConcealment_CalcBandEnergy(&pConcealmentInfo->SpectralCoefficient[wnd * (FRAME_SIZE / MAX_WINDOWS)], /* spec_n */
                                        EightShortSequence,
                                        pIcsInfo->SamplingRateIndex,
                                        CConcealment_NoExpand,
                                        sfbEnergyAct);
            
            CConcealment_InterpolateBuffer(&pSpectralCoefficient[wnd * (FRAME_SIZE / MAX_WINDOWS)], /* spec_(n-1) */
                                           sfbEnergyPrev,
                                           sfbEnergyAct,
                                           scaleFactorBandsTotal,
                                           pSfbOffset);
          }
        } else { /* f_n != EightShortSequence */
          int scaleFactorBandsTotal = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Long;
          const short *pSfbOffset   = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Long;
             
            
          CConcealment_CalcBandEnergy(&pSpectralCoefficient[FRAME_SIZE - (FRAME_SIZE / MAX_WINDOWS)], /* spec_(n-2) last window */
                                      EightShortSequence,
                                      pIcsInfo->SamplingRateIndex,
                                      CConcealment_Expand,
                                      sfbEnergyAct);
           
          CConcealment_CalcBandEnergy(pConcealmentInfo->SpectralCoefficient, /* spec_n */
                                      OnlyLongSequence,
                                      pIcsInfo->SamplingRateIndex,
                                      CConcealment_NoExpand,
                                      sfbEnergyPrev);
           
          pIcsInfo->WindowShape = 0;
          pIcsInfo->WindowSequence = LongStopSequence;
           /* pSpectralCoefficient[]
                          pConcealmentInfo->SpectralCoefficient[]
                       */
          
          for (i = 0; i < FRAME_SIZE; i++) {
            
            pSpectralCoefficient[i] = pConcealmentInfo->SpectralCoefficient[i]; /* spec_n */
          }
          
          CConcealment_InterpolateBuffer(pSpectralCoefficient, /* spec_(n-1) */
                                         sfbEnergyPrev,
                                         sfbEnergyAct,
                                         scaleFactorBandsTotal,
                                         pSfbOffset);
        }
      }
      else {
        int scaleFactorBandsTotal = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Long;
        const short *pSfbOffset   = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Long;
           
         
        CConcealment_CalcBandEnergy(pSpectralCoefficient,  /* spec_(n-2) */
                                    OnlyLongSequence,
                                    pIcsInfo->SamplingRateIndex,
                                    CConcealment_NoExpand,
                                    sfbEnergyPrev);
          
        if(pConcealmentInfo->WindowSequence == EightShortSequence) {  /* f_n == EightShortSequence */
           
          pIcsInfo->WindowShape = 1;
          pIcsInfo->WindowSequence = LongStartSequence;
          /* Expand first short spectrum */
           
          CConcealment_CalcBandEnergy(pConcealmentInfo->SpectralCoefficient,  /* spec_n */
                                      EightShortSequence,
                                      pIcsInfo->SamplingRateIndex,
                                      CConcealment_Expand,
                                      sfbEnergyAct);
        }
        else {
           
          pIcsInfo->WindowShape = 0;
          pIcsInfo->WindowSequence = OnlyLongSequence;
           
          CConcealment_CalcBandEnergy(pConcealmentInfo->SpectralCoefficient,  /* spec_n */
                                      OnlyLongSequence,
                                      pIcsInfo->SamplingRateIndex,
                                      CConcealment_NoExpand,
                                      sfbEnergyAct);
        }
        
        CConcealment_InterpolateBuffer(pSpectralCoefficient,  /* spec_(n-1) */
                                       sfbEnergyPrev,
                                       sfbEnergyAct,
                                       scaleFactorBandsTotal,
                                       pSfbOffset);
      }
    }
    /* Noise substitution of sign of the output spectral coefficients */
    
    CConcealment_ApplyRandomSign (pConcealmentInfo,
                                  pSpectralCoefficient);
  }
  /* update FrameOk */
   
  pConcealmentInfo->prevFrameOk[0]  = pConcealmentInfo->prevFrameOk[1];
  pConcealmentInfo->prevFrameOk[1]  = FrameOk;
  /* scale spectrum according to concealment state */
   
  switch (pConcealmentInfo->ConcealState) {
  case CConcealment_FadeOut:
     /* pSpectralCoefficient[] */
     
    for (i = 0; i < FRAME_SIZE; i++) {
       
      pSpectralCoefficient[i] = pSpectralCoefficient[i] * fadeFacTable[pConcealmentInfo->cntConcealFrame];
    }
    break;
  case CConcealment_FadeIn:
     /* pSpectralCoefficient[] */
     
    for (i = 0; i < FRAME_SIZE; i++) {
       
      pSpectralCoefficient[i] = pSpectralCoefficient[i] * fadeFacTable[pConcealmentInfo->nFadeInFrames - pConcealmentInfo->cntConcealFrame - 1];
    }
    break;
  case CConcealment_Mute:
     /* pSpectralCoefficient[] */
     
    for (i = 0; i < FRAME_SIZE; i++) {
      
      pSpectralCoefficient[i] = 0.0;
    }
    break;
  }
  
}
/*
  The function toggles the sign of the spectral data randomly. This is 
  useful to ensure the quality of the concealed frames.
  return:  none
*/
void CConcealment_ApplyRandomSign (CConcealmentInfo *pConcealmentInfo,
                                   float *spec)
{
  int i;
  
   /* spec[]
                  sbr_randomPhase[pConcealmentInfo->iRandomPhase][0]
               */
  
  for(i = 0; i < FRAME_SIZE; i++) {
    
    if( sbr_randomPhase[pConcealmentInfo->iRandomPhase][0] < 0.0 ) {
       
      spec[i] = -spec[i];
    }
    pConcealmentInfo->iRandomPhase = (pConcealmentInfo->iRandomPhase + 1) & (AAC_NF_NO_RANDOM_VAL - 1);
  }
  
}
/*
  The function calculates band-wise the spectral energy. This is used for 
  frame interpolation.
  The band-wise energy is not stored logarithmized anymore.
  return:  none
*/
void CConcealment_CalcBandEnergy (float                  *spectrum,
                                  int                     blockType,
                                  int                     samplingRateIndex,
                                  CConcealmentExpandType  expandType,
                                  float                  *sfbEnergy)
{
  int line, sfb;
  const short *pSfbOffset;
  int scaleFactorBandsTotal;
  float enAccu;
  
  /* In the following calculations, enAccu is initialized with LSB-value in order to avoid zero energy-level */
  
  line = 0;
  
  switch(blockType) {
  case OnlyLongSequence:
  case LongStartSequence:
  case LongStopSequence:
     
    if(expandType == CConcealment_NoExpand) {
      /* standard long calculation */
       
      scaleFactorBandsTotal = SamplingRateInfoTable[samplingRateIndex].NumberOfScaleFactorBands_Long;
      pSfbOffset = SamplingRateInfoTable[samplingRateIndex].ScaleFactorBands_Long;
       /* sfbEnergy[] */
      
      for (sfb = 0; sfb < scaleFactorBandsTotal; sfb++) {
        
        enAccu = 1;
         /* spectrum[] */
        
        for( ; line < pSfbOffset[sfb+1]; line++) {
          
          enAccu += spectrum[line] * spectrum[line];
        }
        
        sfbEnergy[sfb] = enAccu;
      }
    }
    else {
      /* compress long to short */
       
      scaleFactorBandsTotal = SamplingRateInfoTable[samplingRateIndex].NumberOfScaleFactorBands_Short;
      pSfbOffset = SamplingRateInfoTable[samplingRateIndex].ScaleFactorBands_Short;
       /* sfbEnergy[] */
      
      for (sfb = 0; sfb < scaleFactorBandsTotal; sfb++) {
        
        enAccu = 1;
         /* spectrum[] */
        
        for(; line < pSfbOffset[sfb+1] << 3; line++) {
          
          enAccu += spectrum[line] * spectrum[line];
        }
         
        sfbEnergy[sfb] = enAccu * 0.125f;
      }
    }
    break;
  case EightShortSequence:
     
    if(expandType == CConcealment_NoExpand) {   
      /* standard short calculation */
       
      scaleFactorBandsTotal = SamplingRateInfoTable[samplingRateIndex].NumberOfScaleFactorBands_Short;
      pSfbOffset = SamplingRateInfoTable[samplingRateIndex].ScaleFactorBands_Short;
       /* sfbEnergy[] */
      
      for(sfb = 0; sfb < scaleFactorBandsTotal; sfb++) {
        
        enAccu = 1;
         /* spectrum[] */
        
        for( ; line < pSfbOffset[sfb+1]; line++) {
          
          enAccu += spectrum[line] * spectrum[line];
        }
        
        sfbEnergy[sfb] = enAccu;
      }
    }
    else {
      /* expand short to long */
       
      scaleFactorBandsTotal = SamplingRateInfoTable[samplingRateIndex].NumberOfScaleFactorBands_Long;
      pSfbOffset = SamplingRateInfoTable[samplingRateIndex].ScaleFactorBands_Long;
       /* sfbEnergy[] */
      
      for (sfb = 0; sfb < scaleFactorBandsTotal; sfb++) {
        
        enAccu = 1;
         /* spectrum[] */
        
        for ( ; line < pSfbOffset[sfb+1]; line++) {
          
          enAccu += spectrum[line >> 3] * spectrum[line >> 3];
        }
        
        sfbEnergy[sfb] = enAccu;
      }
    }
    break;
  }
  
}
/*
  The function creates the interpolated spectral data according to the 
  energy of the last good frame and the current (good) frame.
*/
void CConcealment_InterpolateBuffer (float       *spectrum,
                                     float       *enPrev,
                                     float       *enAct,
                                     int          sfbCnt,
                                     const short *pSfbOffset)
{
  int    sfb, line = 0;
  float  multiplier;
  
   
   /* enAct[]
                  enPrev[]
                  spectrum[]
               */
  
  for(sfb = 0; sfb < sfbCnt; sfb++) {
    DIV(1); 
    multiplier = (float) pow(enAct[sfb] / enPrev[sfb], 0.25);
    
    for(; line < pSfbOffset[sfb+1]; line++) {
       
      spectrum [line] = spectrum[line] * multiplier;
    }
  }
  
}
/*
  The function updates the state of the concealment state-machine. The 
  states are: mute, fade-in, fade-out, and frame-ok.
*/
void CConcealment_UpdateState(CConcealmentInfo *pConcealmentInfo,
                              int FrameOk)
{
  
  
  switch(pConcealmentInfo->ConcealState){
  case CConcealment_Mute:
      
    if(pConcealmentInfo->prevFrameOk[1] ||
       (pConcealmentInfo->prevFrameOk[0] &&
        !pConcealmentInfo->prevFrameOk[1] &&
        FrameOk)) {
        
      pConcealmentInfo->cntConcealFrame++;
        
      if (pConcealmentInfo->cntConcealFrame >= pConcealmentInfo->nValidFrames){
         
        pConcealmentInfo->cntConcealFrame = 0;
        pConcealmentInfo->ConcealState    = CConcealment_FadeIn;
      }
    }
    break;
  case CConcealment_FadeIn:
      
    if(pConcealmentInfo->prevFrameOk[1] ||
       (pConcealmentInfo->prevFrameOk[0] &&
        !pConcealmentInfo->prevFrameOk[1] &&
        FrameOk)) {
        
      pConcealmentInfo->cntConcealFrame++;
        
      if(pConcealmentInfo->cntConcealFrame == pConcealmentInfo->nFadeInFrames) {
         
        pConcealmentInfo->cntConcealFrame = 0;
        pConcealmentInfo->ConcealState    = CConcealment_Ok;
      }
    }
    else{
       
      pConcealmentInfo->cntConcealFrame  = 0;
      pConcealmentInfo->ConcealState     = CConcealment_FadeOut;
    }
    break;
  case CConcealment_Ok:
      
    if(!(pConcealmentInfo->prevFrameOk[1] ||
         (pConcealmentInfo->prevFrameOk[0] &&
          !pConcealmentInfo->prevFrameOk[1] &&
          FrameOk))) {
       
      pConcealmentInfo->ConcealState = CConcealment_FadeOut;
    }
    break;
  case CConcealment_FadeOut:
      
    if(pConcealmentInfo->prevFrameOk[1] ||
       (pConcealmentInfo->prevFrameOk[0] &&
        !pConcealmentInfo->prevFrameOk[1] &&
        FrameOk)) {
       
      pConcealmentInfo->cntConcealFrame = 0;
      pConcealmentInfo->ConcealState    = CConcealment_Ok;
    }
    else{
        
      pConcealmentInfo->cntConcealFrame++;
        
      if(pConcealmentInfo->cntConcealFrame == pConcealmentInfo->nFadeOutFrames) {
         
        pConcealmentInfo->cntConcealFrame = 0;
        pConcealmentInfo->ConcealState    = CConcealment_Mute;
      }
    }
    break;
  }
  
  
}
