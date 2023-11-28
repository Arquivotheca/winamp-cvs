/*
  Low Power Profile Transposer
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "lpp_tran.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "math/FloatFR.h"
#include <stdio.h>
#include "math/counters.h" /* the 3GPP instrumenting tool */
typedef struct {
  float  r11r;
  float  r01r;
  float  r01i;
  float  r02r;
  float  r02i;
  float  r12r;
  float  r12i;
  float  r22r;
  float  det;
} ACORR_COEFS;
/*
 *
 * \brief Calculate second order autocorrelation using 2 accumulators
 *
 */
static void
autoCorrelation2ndLP(ACORR_COEFS *ac,
                     float *realBuf,
                     int len
                     )
{
  int   j;
  float accu1, accu2;
  
  
  accu1 = 0.0;
   /* pointer for realBuf[]  */
   
  for ( j = 0; j < len - 1; j++ ) {
    
    accu1 += realBuf[j-1] * realBuf[j-1];
  }
  
  accu2 = realBuf[-2] * realBuf[-2];
  
  accu2 += accu1;
  
  accu1 += realBuf[j-1] * realBuf[j-1];
  
  ac->r11r = accu1;
  ac->r22r = accu2;
  
  accu1 = 0.0;
   /* pointer for realBuf[] */
  
  for ( j = 0; j < len - 1; j++ ) {
    
    accu1 += realBuf[j] * realBuf[j-1];
  }
  
  accu2 = realBuf[-1] * realBuf[-2];
  
  accu2 += accu1;
  
  accu1 += realBuf[j] * realBuf[j-1];
  
  ac->r01r = accu1;
  ac->r12r = accu2;
  
  accu1=0.0;
   /* pointer for realBuf[] */
  
  for ( j = 0; j < len; j++ ) {
    
    accu1 += realBuf[j] * realBuf[j-2];
  }
  
  ac->r02r = accu1;
    
  ac->det = ac->r11r * ac->r22r - ac->r12r * ac->r12r;
  
  ac->r01i = ac->r02i = ac->r12i = 0.0f;
    /* move all register variables to the structure ac->... */
  
}
#ifndef LP_SBR_ONLY
static void
autoCorrelation2nd(ACORR_COEFS *ac,
                   float *realBuf,
                   float *imagBuf,
                   int len
                   )
{
  int   j;
  float accu1,accu2;
  
  
  accu1 = 0.0;
   /* pointer for realBuf[],
                              imagBuf[]  */
   
  for ( j = 0; j < len - 1; j++ ) {
    MAC(2);
    accu1 += realBuf[j-1] * realBuf[j-1]
           + imagBuf[j-1] * imagBuf[j-1];
  }
   
  accu2 = realBuf[-2] * realBuf[-2]
        + imagBuf[-2] * imagBuf[-2];
  
  accu2 += accu1;
  MAC(2);
  accu1 += realBuf[j-1] * realBuf[j-1]
         + imagBuf[j-1] * imagBuf[j-1];
  
  ac->r11r = accu1;
  ac->r22r = accu2;
  
  accu1 = 0.0;
   /* pointer for realBuf[],
                              imagBuf[]  */
  
  for ( j = 0; j < len - 1; j++ ) {
    MAC(2);
    accu1 += realBuf[j] * realBuf[j-1]
           + imagBuf[j] * imagBuf[j-1];
  }
  MAC(2);
  accu2 = realBuf[-1] * realBuf[-2]
        + imagBuf[-1] * imagBuf[-2];
  
  accu2 += accu1;
  MAC(2);
  accu1 += realBuf[j] * realBuf[j-1]
         + imagBuf[j] * imagBuf[j-1];
  
  ac->r01r = accu1;
  ac->r12r = accu2;
  
  accu1 = 0.0;
   /* pointer for realBuf[],
                              imagBuf[]  */
  
  for ( j = 0; j < len - 1; j++ ) {
     
    accu1 += imagBuf[j] * realBuf[j-1]
           - realBuf[j] * imagBuf[j-1];
  }
   
  accu2 = imagBuf[-1] * realBuf[-2]
        - realBuf[-1] * imagBuf[-2];
  
  accu2 += accu1;
   
  accu1 += imagBuf[j] * realBuf[j-1]
         - realBuf[j] * imagBuf[j-1];
  
  ac->r01i = accu1;
  ac->r12i = accu2;
  
  accu1=accu2=0.0;
   /* pointer for realBuf[],
                              imagBuf[]  */
  
  for ( j = 0; j < len; j++ ){
    MAC(2);
    accu1 += realBuf[j] * realBuf[j-2] + imagBuf[j] * imagBuf[j-2];
     MAC(2);
    accu2 += imagBuf[j] * realBuf[j-2] - realBuf[j] * imagBuf[j-2];
  }
  
  ac->r02r = accu1;
  ac->r02i = accu2;
   MAC(2); 
  ac->det = ac->r11r * ac->r22r - (ac->r12r * ac->r12r + ac->r12i * ac->r12i);
    /* move all register variables to the structure ac->... */
  
}
#endif
/*
 *
 * \brief Get bandwidth expansion factor from filtering level
 *
 */
static float
mapInvfMode (INVF_MODE mode,
             INVF_MODE prevMode)
{
      /* worst case */
  
  switch (mode) {
  case INVF_LOW_LEVEL:
    if(prevMode == INVF_OFF)
      return 0.6f;
    else
      return 0.75f;
    break;
  case INVF_MID_LEVEL:
    return 0.90f;
    break;
  case INVF_HIGH_LEVEL:
    return 0.98f;
    break;
  default:
    if(prevMode == INVF_LOW_LEVEL)
      return 0.6f;
    else
      return 0.0f;
    break;
  }
}
/*
 *
 * \brief Perform inverse filtering level emphasis
 *
 */
static void
inverseFilteringLevelEmphasis(HANDLE_SBR_LPP_TRANS hLppTrans,
                              unsigned char nInvfBands,
                              INVF_MODE *sbr_invf_mode,
                              INVF_MODE *sbr_invf_mode_prev,
                              float * bwVector
                              )
{
  int i;
  float accu;
  float  w1, w2;
  
   /* pointer for hLppTrans->bwVectorOld[] */
  
  for(i = 0; i < nInvfBands; i++) {
      
    bwVector[i] = mapInvfMode (sbr_invf_mode[i],
                               sbr_invf_mode_prev[i]);
     
    if(bwVector[i] < hLppTrans->bwVectorOld[i]) {
      
      w1 = 0.75f;
      w2 = 0.25f;
    }
    else {
      
      w1 = 0.90625f;
      w2 = 0.09375f;
    }
     
    accu = w1*bwVector[i] + w2*hLppTrans->bwVectorOld[i];
     
    if (accu < 0.015625f) {
      
      accu=0;
    }
      
    accu = min(accu,0.99609375f);
    
    bwVector[i] = accu;
  }
  
}
/*
 * \brief Perform transposition by patching of subband samples.
 */
void lppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                    float **qmfBufferReal,
#ifndef LP_SBR_ONLY
                    float **qmfBufferImag,
#endif
                    float *degreeAlias,
                    int timeStep,
                    int firstSlotOffs,
                    int lastSlotOffs,
                    unsigned char nInvfBands,
                    INVF_MODE *sbr_invf_mode,
                    INVF_MODE *sbr_invf_mode_prev,
                    int   bUseLP
                    )
{
  int    bwIndex[MAX_NUM_PATCHES];
  float  bwVector[MAX_NUM_PATCHES];
  int    i,j;
  int    loBand, hiBand;
  PATCH_PARAM *patchParam;
  int    patch;
  float  alphar[LPC_ORDER], a0r, a1r;
  float  alphai[LPC_ORDER], a0i, a1i;
  float  bw;
  int    autoCorrLength;
  float k1, k1_below, k1_below2;
  ACORR_COEFS ac;
  int    startSample;
  int    stopSample;
  int    stopSampleClear;
  int    lb, hb;
  int targetStopBand;
  
   
  patchParam = hLppTrans->Settings.patchParam;
  
  bw = 0.0f;
  
  k1_below=0, k1_below2=0;
  
  startSample = firstSlotOffs * timeStep;
    
  stopSample  = hLppTrans->Settings.nCols + lastSlotOffs * timeStep;
  
  inverseFilteringLevelEmphasis(hLppTrans, nInvfBands, sbr_invf_mode, sbr_invf_mode_prev, bwVector);
  
  stopSampleClear = stopSample;
   /* pointers for qmfBufferReal[],
                               qmfBufferImag[]  */
   
  for ( patch = 0; patch < hLppTrans->Settings.noOfPatches; patch++ ) {
    
    for (i = startSample; i < stopSampleClear; i++) {
        
      for(j=patchParam[patch].guardStartBand; j<patchParam[patch].guardStartBand+GUARDBANDS; j++){
        
        qmfBufferReal[i][j] = 0.0;
#ifndef LP_SBR_ONLY
        
        if (!bUseLP) {
          
          qmfBufferImag[i][j] = 0.0;
        }
#endif
      }
    }
  }
   
  targetStopBand = patchParam[hLppTrans->Settings.noOfPatches-1].targetStartBand +
    patchParam[hLppTrans->Settings.noOfPatches-1].numBandsInPatch;
   /* pointers for qmfBufferReal[],
                               qmfBufferImag[]  */
  
  for (i = startSample; i < stopSampleClear; i++) {
    
    for (j=targetStopBand; j<NO_SYNTHESIS_CHANNELS; j++) {
      
      qmfBufferReal[i][j] = 0.0;
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
        
        qmfBufferImag[i][j] = 0.0;
      }
#endif
    }
  }
   
  autoCorrLength = hLppTrans->Settings.nCols + 6;
   /* pointer for bwIndex[patch] */
   
  for ( patch=0; patch<hLppTrans->Settings.noOfPatches; patch++ ) {
    
    bwIndex[patch] = 0;
  }
  
  if (bUseLP) {
       
    lb = max(1, hLppTrans->Settings.lbStartPatching - 2);
    
    hb = patchParam[0].targetStartBand;
  }
#ifndef LP_SBR_ONLY
  else {
     
    lb = hLppTrans->Settings.lbStartPatching;
    hb = hLppTrans->Settings.lbStopPatching;
  }
#endif
   /* pointers for qmfBufferReal[],
                               qmfBufferImag[]  */
   
  for ( loBand = lb; loBand < hb; loBand++ ) 
	{
    float  lowBandReal[MAX_ENV_COLS+LPC_ORDER];
    float  lowBandImag[MAX_ENV_COLS+LPC_ORDER];

    int lowBandPtr =0;
    int resetLPCCoeffs=0;

    
    for(i=0;i<LPC_ORDER;i++){
      
      lowBandReal[lowBandPtr] = hLppTrans->lpcFilterStatesReal[i][loBand];
#ifndef LP_SBR_ONLY
      if (!bUseLP) {
        
        lowBandImag[lowBandPtr] = hLppTrans->lpcFilterStatesImag[i][loBand];
      }
#endif
      lowBandPtr++;
    }
    
    for(i=0;i< 6;i++){
      
      lowBandReal[lowBandPtr] = (float) qmfBufferReal[i][loBand];
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
        
        lowBandImag[lowBandPtr] = (float) qmfBufferImag[i][loBand];
      }
#endif
      lowBandPtr++;
    }
      
    for(i=6;i<hLppTrans->Settings.nCols+6;i++){
        
        lowBandReal[lowBandPtr] = (float) qmfBufferReal[i][loBand];
#ifndef LP_SBR_ONLY
        
        if (!bUseLP) {
          
          lowBandImag[lowBandPtr] = (float) qmfBufferImag[i][loBand];
        }
#endif
        lowBandPtr++;
    }
    
    if (bUseLP) {
        
      autoCorrelation2ndLP(&ac,
                           lowBandReal+LPC_ORDER,
                           autoCorrLength);
    }
#ifndef LP_SBR_ONLY
    else {
        
      autoCorrelation2nd(&ac,
                         lowBandReal+LPC_ORDER,
                         lowBandImag+LPC_ORDER,
                         autoCorrLength);
    }
#endif
    
    alphar[1] = 0;
    alphai[1] = 0;
     
    if (ac.det != 0.0f) {
      float fac;
      DIV(1);
      fac = 1.0f / ac.det;
        
      alphar[1] = ( ac.r01r * ac.r12r - ac.r01i * ac.r12i - ac.r02r * ac.r11r ) * fac;
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
           
        alphai[1] = ( ac.r01i * ac.r12r + ac.r01r * ac.r12i - ac.r02i * ac.r11r ) * fac;
      }
#endif
    }
    
    alphar[0] = 0;
    alphai[0] = 0;
     
    if ( ac.r11r != 0.0f ) {
      float fac;
      DIV(1);
      fac = 1.0f / ac.r11r;
         
      alphar[0] = - ( ac.r01r + alphar[1] * ac.r12r + alphai[1] * ac.r12i ) * fac;
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
          
        alphai[0] = - ( ac.r01i + alphai[1] * ac.r12r - alphar[1] * ac.r12i ) * fac;
      }
#endif
    }
    
       
    if(alphar[0]*alphar[0] + alphai[0]*alphai[0] >= 16.0f) {
      
      resetLPCCoeffs=1;
    }
       
    if(alphar[1]*alphar[1] + alphai[1]*alphai[1] >= 16.0f) {
      
      resetLPCCoeffs=1;
    }
    
    if(resetLPCCoeffs){
      
      alphar[0] = alphar[1] = 0;
      alphai[0] = alphai[1] = 0;
    }
    
    if (bUseLP) {
       
      if(ac.r11r==0.0f) {
        
        k1 = 0.0f;
      }
      else {
         DIV(1); 
        k1 = -(ac.r01r/ac.r11r);
          
        k1 = min(k1, 1.0f);
          
        k1 = max(k1,-1.0f);
      }
       
      if(loBand > 1){
        float deg;
         
        deg = 1.0f - (k1_below * k1_below);
        
        degreeAlias[loBand] = 0;
         /* pointer for degreeAlias[] */
         
        if (((loBand & 1) == 0) && (k1 < 0)){
          
          if (k1_below < 0) {
            
            degreeAlias[loBand] = 1.0f;
            
            if ( k1_below2 > 0 ) {
              
              degreeAlias[loBand-1] = deg;
            }
          }
          else {
            
            if ( k1_below2 > 0 ) {
              
              degreeAlias[loBand] = deg;
            }
          }
        }
         
        if (((loBand & 1) == 1) && (k1 > 0)){
          
          if (k1_below > 0) {
            
            degreeAlias[loBand] = 1.0f;
            
            if ( k1_below2 < 0 ) {
              
              degreeAlias[loBand-1] = deg;
            }
          }
          else {
            
            if ( k1_below2 < 0 ) {
              
              degreeAlias[loBand] = deg;
            }
          }
        }
      }
      
      k1_below2 = k1_below;
      k1_below = k1;
    }
    
    patch = 0;
     /* pointer for patchParam[patch] */
     
    while ( patch < hLppTrans->Settings.noOfPatches ) {
      
      hiBand = loBand + patchParam[patch].targetBandOffs;
        
      if ( loBand < patchParam[patch].sourceStartBand || loBand >= patchParam[patch].sourceStopBand ) {
        
        patch++;
        continue;
      }
      assert( hiBand < NO_SYNTHESIS_CHANNELS );
      
      while (hiBand >= hLppTrans->Settings.bwBorders[bwIndex[patch]]) {
         /* while() condition */
         
        bwIndex[patch]++;
      }
      
      bw = bwVector[bwIndex[patch]];
       
      a0r = bw * alphar[0];
      a0i = bw * alphai[0];
      bw =  bw*bw;
      a1r = bw * alphar[1];
      a1i = bw * alphai[1];
       /* pointers for lowBandReal[],
                                   lowBandImag[],
                                   qmfBufferReal[],
                                   qmfBufferImag[]  */
      
      for(i = startSample; i < stopSample; i++ ) {
        
        qmfBufferReal[i][hiBand] = lowBandReal[LPC_ORDER+i];
        
        if (bUseLP) {
          
          if ( bw > 0 ) {
            MAC(2); 
            qmfBufferReal[i][hiBand] = qmfBufferReal[i][hiBand] +
              a0r * lowBandReal[LPC_ORDER+i-1] +
              a1r * lowBandReal[LPC_ORDER+i-2];
          }
        }
#ifndef LP_SBR_ONLY
        else {
          
          qmfBufferImag[i][hiBand] = lowBandImag[LPC_ORDER+i];
          
          if ( bw > 0 ) {
            float accu;
             
            accu = a0r * lowBandReal[LPC_ORDER+i-1] -  a0i * lowBandImag[LPC_ORDER+i-1]+
              a1r * lowBandReal[LPC_ORDER+i-2] -  a1i * lowBandImag[LPC_ORDER+i-2];
             
            qmfBufferReal[i][hiBand] = qmfBufferReal[i][hiBand] + accu;
            MAC(4);
            accu = a0i * lowBandReal[LPC_ORDER+i-1] +  a0r * lowBandImag[LPC_ORDER+i-1]+
              a1i * lowBandReal[LPC_ORDER+i-2] +  a1r * lowBandImag[LPC_ORDER+i-2];
             
            qmfBufferImag[i][hiBand] = qmfBufferImag[i][hiBand] + accu;
          }
        }
#endif
      }
      patch++;
    }  /* Patch */
  }  /* loBand (band) */
  
  for(i=0;i<LPC_ORDER;i++){
     
    for (loBand=0; loBand<patchParam[0].targetStartBand; loBand++) {
      
      hLppTrans->lpcFilterStatesReal[i][loBand] = qmfBufferReal[hLppTrans->Settings.nCols-LPC_ORDER+i][loBand];
#ifndef LP_SBR_ONLY
      
      if (!bUseLP) {
        
        hLppTrans->lpcFilterStatesImag[i][loBand] = qmfBufferImag[hLppTrans->Settings.nCols-LPC_ORDER+i][loBand];
      }
#endif
    }
  }
  
  if (bUseLP) {
     /* pointers for degreeAlias[loBand],
                                 degreeAlias[hiBand]  */
     
    for ( loBand = hLppTrans->Settings.lbStartPatching; loBand <  hLppTrans->Settings.lbStopPatching; loBand++ ) {
      
      patch = 0;
       
      while ( patch < hLppTrans->Settings.noOfPatches ) {
         
        hiBand = loBand + patchParam[patch].targetBandOffs;
          
        if ( loBand < patchParam[patch].sourceStartBand
             || loBand >= patchParam[patch].sourceStopBand
             || hiBand >= NO_SYNTHESIS_CHANNELS
             ) {
          
          patch++;
          continue;
        }
          
        if(hiBand != patchParam[patch].targetStartBand) {
          
          degreeAlias[hiBand] = degreeAlias[loBand];
        }
        else {
          
          degreeAlias[hiBand] = 0;
        }
        patch++;
      }
    }/* end  for loop */
  }
   /* pointers for bwVectorOld[],
                               bwVector[]     */
  
  for (i = 0; i < nInvfBands; i++ ) {
    
    hLppTrans->bwVectorOld[i] = bwVector[i];
  }
  
}
/*
 *
 * \brief Initialize one low power transposer instance
 *
 *
 */
int
createLppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                     unsigned char highBandStartSb,
                     unsigned char *v_k_master,
                     unsigned char numMaster,
                     unsigned char usb,
                     unsigned char nCols,
                     unsigned char* noiseBandTable,
                     unsigned char noNoiseBands,
                     unsigned short fs,
                     unsigned char  chan
                     )
{
  HANDLE_SBR_LPP_TRANS hs;
  
  
  hs = hLppTrans;
  
  
  hs->Settings.nCols = nCols;
  
  if (chan==0) {
    
    hs->Settings.nCols = nCols;
    
    
    return resetLppTransposer (hs,
                               0,
                               highBandStartSb,
                               v_k_master,
                               numMaster,
                               noiseBandTable,
                               noNoiseBands,
                               usb,
                               fs);
  }
  
  return 0;
}
static int findClosestEntry(unsigned char goalSb, unsigned char *v_k_master, unsigned char numMaster, unsigned char direction)
{
  int index;
  
   
  if( goalSb <= v_k_master[0] ) {
    
    return v_k_master[0];
  }
   
  if( goalSb >= v_k_master[numMaster] ) {
    
    return v_k_master[numMaster];
  }
  
  if(direction) {
    
    index = 0;
     /* pointer for v_k_master[index] */
     
    while( v_k_master[index] < goalSb ) {
      
      index++;
    }
  } else {
    
    index = numMaster;
     /* pointer for v_k_master[index] */
     
    while( v_k_master[index] > goalSb ) {
      
      index--;
    }
  }
  
  return v_k_master[index];
}
/*
 *
 * \brief Reset memory for one lpp transposer instance
 *
 *
 */
int
resetLppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                    unsigned char xposctrl,
                    unsigned char highBandStartSb,
                    unsigned char *v_k_master,
                    unsigned char numMaster,
                    unsigned char* noiseBandTable,
                    unsigned char noNoiseBands,
                    unsigned char usb,
                    unsigned short fs
                    )
{
  int    i, patch;
  int    targetStopBand;
  TRANSPOSER_SETTINGS *pSettings;
  PATCH_PARAM  *patchParam;
  int sourceStartBand;
  int patchDistance;
  int numBandsInPatch;
  int lsb;
  int xoverOffset;
  int goalSb;
  
  
  pSettings = &hLppTrans->Settings;
  patchParam = pSettings->patchParam;
  
  lsb = v_k_master[0];
  
  xoverOffset = highBandStartSb - lsb;
   
  if ( lsb - SHIFT_START_SB < 4 ) {
    
    return (1);
  }
   
  if (xposctrl == 1) {
    
    lsb += xoverOffset;
    
    xoverOffset = 0;
  }
    /* worst case */
  switch(fs){
  case 16000:
  case 22050:
  case 24000:
  case 32000:
    goalSb=64;
    break;
  case 44100:
    goalSb=46;
    break;
  case 48000:
    goalSb=43;
    break;
  default:
    
    return(0);
  }
  
  goalSb = findClosestEntry(goalSb, v_k_master, numMaster, 1);
   
  if ( abs(goalSb - usb) < 4 ) {
    
    goalSb = usb;
  }
  
  sourceStartBand = SHIFT_START_SB + xoverOffset;
  
  targetStopBand = lsb + xoverOffset;
  
  patch = 0;
   /* pointer for patchParam[patch] */
  
  while(targetStopBand < usb) {
     
    if (patch > MAX_NUM_PATCHES) {
      
      return -1;
    }
    
    patchParam[patch].guardStartBand = targetStopBand;
    
    targetStopBand += GUARDBANDS;
    
    patchParam[patch].targetStartBand = targetStopBand;
    
    numBandsInPatch = goalSb - targetStopBand;
     
    if ( numBandsInPatch >= lsb - sourceStartBand ) {
      
      patchDistance   = targetStopBand - sourceStartBand;
      
      patchDistance   = patchDistance & ~1;
      
      numBandsInPatch = lsb - (targetStopBand - patchDistance);
       
      numBandsInPatch = findClosestEntry(targetStopBand + numBandsInPatch, v_k_master, numMaster, 0) -
                        targetStopBand;
    }
    
    patchDistance   = numBandsInPatch + targetStopBand - lsb;
     
    patchDistance   = (patchDistance + 1) & ~1;
    
    if (numBandsInPatch > 0) {
       
      patchParam[patch].sourceStartBand = targetStopBand - patchDistance;
      
      patchParam[patch].targetBandOffs  = patchDistance;
      patchParam[patch].numBandsInPatch = numBandsInPatch;
       
      patchParam[patch].sourceStopBand  = patchParam[patch].sourceStartBand + numBandsInPatch;
      
      targetStopBand += patchParam[patch].numBandsInPatch;
      patch++;
    }
    
    sourceStartBand = SHIFT_START_SB;
      
    if( abs(targetStopBand - goalSb) < 3) {
      
      goalSb = usb;
    }
  }
  
  patch--;
    
  if ( (patch>0) && (patchParam[patch].numBandsInPatch < 3) ) {
    
    patch--;
    
    targetStopBand = patchParam[patch].targetStartBand + patchParam[patch].numBandsInPatch;
  }
   
  if (patch >= MAX_NUM_PATCHES) {
    
    return -1;
  }
   
  pSettings->noOfPatches = patch + 1;
  
  pSettings->lbStartPatching = targetStopBand;
  
  pSettings->lbStopPatching  = 0;
   /* pointer for patchParam[patch] */
  
  for ( patch = 0; patch < pSettings->noOfPatches; patch++ ) {
       
    pSettings->lbStartPatching = min( pSettings->lbStartPatching, patchParam[patch].sourceStartBand );
       
    pSettings->lbStopPatching  = max( pSettings->lbStopPatching, patchParam[patch].sourceStopBand );
  }
   /* pointer for bwBorders[i],
                              noiseBandTable[i+1]  */
  
  for(i = 0 ; i < noNoiseBands; i++){
    
    hLppTrans->Settings.bwBorders[i] = noiseBandTable[i+1];
  }
  
  return 0;
}
