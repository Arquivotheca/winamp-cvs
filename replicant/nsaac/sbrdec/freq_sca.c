/*
  Frequency scale calculation
*/
#include <math.h>
#include <string.h>
#include <assert.h>
#include "sbr_rom.h"
#include "sbr_const.h"
#include "env_extr.h"
#include "freq_sca.h"
#include "math/FloatFR.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
#define MAX_OCTAVE         29
#define MAX_SECOND_REGION  50
static int  numberOfBands(float bpo, int start, int stop, int warpFlag);
static void CalcBands(unsigned char * diff, unsigned char start, unsigned char stop, unsigned char num_bands);
static int  modifyBands(unsigned char max_band, unsigned char * diff, unsigned char length);
static void cumSum(unsigned char start_value, unsigned char* diff, unsigned char length, unsigned char *start_adress);
/*!
  \brief     Retrieve QMF-band where the SBR range starts
  \return  Number of start band
*/
static unsigned char
getStartBand(unsigned short fs,
             unsigned char  startFreq)
{
  int band;
  
    switch(fs) {
    case 48000:
       
      band = sbr_start_freq_48[startFreq];
      break;
    case 44100:
       
      band = sbr_start_freq_44[startFreq];
      break;
    case 32000:
       
      band = sbr_start_freq_32[startFreq];
      break;
    case 24000:
       
      band = sbr_start_freq_24[startFreq];
      break;
    case 22050:
       
      band = sbr_start_freq_22[startFreq];
      break;
    case 16000:
       
      band = sbr_start_freq_16[startFreq];
      break;
    default:
      
      band = -1;
    }
  
  return band;
}
/*!
  \brief     Generates master frequency tables
  \return  errorCode, 0 if successful
*/
int
sbrdecUpdateFreqScale(unsigned char * v_k_master,
                      unsigned char *numMaster,
                      SBR_HEADER_DATA * hHeaderData)
{
  int      err=0;
  unsigned short fs = hHeaderData->outSampleRate;
  float    bpo;
  int      dk=0;
  unsigned char     k0, k2, k1=0, i;
  unsigned char     num_bands0;
  unsigned char     num_bands1;
  unsigned char     diff_tot[MAX_OCTAVE + MAX_SECOND_REGION];
  unsigned char     *diff0 = diff_tot;
  unsigned char     *diff1 = diff_tot+MAX_OCTAVE;
  int     k2_achived;
  int     k2_diff;
  int     incr=0;
  
     
   
  k0 = getStartBand(fs, hHeaderData->startFreq);
    
  if(hHeaderData->stopFreq<14) {
    switch(fs) {
    case 48000:
      
      k1 = 21;
      break;
    case 44100:
      
      k1 = 23;
      break;
    case 32000:
    case 24000:
      
      k1 = 32;
      break;
    case 22050:
      
      k1 = 35;
      break;
    case 16000:
      
      k1 = 48;
      break;
    default:
      
      return -1;
    }
    
    CalcBands( diff0, k1, 64, 13 );
    
    shellsort( diff0, 13 );
    
    cumSum(k1, diff0, 13, diff1);
     
    k2 = diff1[hHeaderData->stopFreq];
  }
  else {
      
    if(hHeaderData->stopFreq==14) {
    
    k2=2*k0;
    }
  else
  {
    
    k2=3*k0;
  }
  }
   
  if (k2 > NO_SYNTHESIS_CHANNELS)
  {
    
    k2 = NO_SYNTHESIS_CHANNELS;
  }
    
  if ( ((k2 - k0) > MAX_FREQ_COEFFS) || (k2 <= k0) ) {
    
    return -1;
  }
    
  if ( (fs == 44100) && ( (k2 - k0) > MAX_FREQ_COEFFS_FS44100 ) ) {
    
    return -1;
  }
    
  if ( (fs >= 48000) && ( (k2 - k0) > MAX_FREQ_COEFFS_FS48000 ) ) {
    
    return -1;
  }
    
  if(hHeaderData->freqScale>0) {
      
    if(hHeaderData->freqScale==1) {
      
      bpo = 12.0f;
    }
    else {
        
      if(hHeaderData->freqScale==2) {
      
      bpo = 10.0f;
    }
    else {
      
      bpo =  8.0f;
    }
    }
      
    if( 1000 * k2 > 2245 * k0 ) {
      
      k1 = 2*k0;
      
      num_bands0 = numberOfBands(bpo, k0, k1, 0);
       
      num_bands1 = numberOfBands(bpo, k1, k2, hHeaderData->alterScale );
       
      if ( num_bands0 < 1) {
        
        return -1;
      }
       
      if ( num_bands1 < 1 ) {
        
        return -1;
      }
      
      CalcBands(diff0, k0, k1, num_bands0);
      
      shellsort( diff0, num_bands0);
      
      if (diff0[0] == 0) {
        
        return -1;
      }
      
      cumSum(k0, diff0, num_bands0, v_k_master);
      
      CalcBands(diff1, k1, k2, num_bands1);
      
      shellsort( diff1, num_bands1);
        
      if(diff0[num_bands0-1] > diff1[0]) {
        
        err = modifyBands(diff0[num_bands0-1],diff1, num_bands1);
        
        if (err)
        {
          
          return -1;
        }
      }
        
      cumSum(k1, diff1, num_bands1, &v_k_master[num_bands0]);
       
      *numMaster = num_bands0 + num_bands1;
    }
    else {
      
      k1=k2;
      
      num_bands0 = numberOfBands(bpo, k0, k1, 0);
       
      if ( num_bands0 < 1) {
        
        return -1;
      }
      
      CalcBands(diff0, k0, k1, num_bands0);
      
      shellsort(diff0, num_bands0);
      
      if (diff0[0] == 0) {
        
        return -1;
      }
      
      cumSum(k0, diff0, num_bands0, v_k_master);
      
      *numMaster = num_bands0;
    }
  }
  else {
      
     if (hHeaderData->alterScale==0) {
        
        dk = 1;
         
        num_bands0 = (k2 - k0) & 254;
      } else {
        
        dk = 2;
          
        num_bands0 = ( ((k2 - k0) >> 1) + 1 ) & 254;
      }
       
      if (num_bands0 < 1) {
        
        return -1;
      }
       
      k2_achived = k0 + num_bands0*dk;
      
      k2_diff = k2 - k2_achived;
       /* diff_tot[i] */
      
      for(i=0;i<num_bands0;i++)
      {
        
        diff_tot[i] = dk;
      }
      
      if (k2_diff < 0) {
          
          incr = 1;
          i = 0;
      }
      
      if (k2_diff > 0) {
          
          incr = -1;
          
          i = num_bands0-1;
      }
       /* diff_tot[i] */
      
      while (k2_diff != 0) {
         
        diff_tot[i] = diff_tot[i] - incr;
        
        i = i + incr;
        
        k2_diff = k2_diff + incr;
      }
      
      cumSum(k0, diff_tot, num_bands0, v_k_master);/* cumsum */
    
    *numMaster = num_bands0;
  }
   
  if (*numMaster < 1) {
    
    return -1;
  }
  
  return 0;
}
/*!
  \brief     Calculate number of SBR bands between start and stop band
  \return    number of bands
*/
static int
numberOfBands(float  bpo,
              int    start,
              int    stop,
              int    warpFlag)
{
  float num_bands_div2;
  int   num_bands;
  
   
  num_bands_div2 = 0.5f * FloatFR_getNumOctaves(start,stop) * bpo;
  
  if (warpFlag) {
    
    num_bands_div2 *= (25200.0f/32768.0f);
  }
  
  num_bands_div2 += 0.5f;
  
  num_bands = 2 * (int)num_bands_div2;
  
  return(num_bands);
}
/*!
  \brief     Calculate width of SBR bands
*/
static void
CalcBands(unsigned char * diff,
          unsigned char start,
          unsigned char stop,
          unsigned char num_bands)
{
  int i;
  int previous;
  int current;
  float exact = (float)start;
  float bandfactor = (float) pow( (float)stop * sbr_invIntTable[start], sbr_invIntTable[num_bands] );
  
      
  
  previous=start;
   /* diff[] */
  
  for(i=1; i<=num_bands; i++)  {
    
    exact *= bandfactor;
    
    current = (int)(exact + 0.5f);
     
    diff[i-1]=current-previous;
    
    previous=current;
  }
  
}
/*!
  \brief     Calculate cumulated sum vector from delta vector
*/
static void
cumSum(unsigned char start_value, unsigned char* diff, unsigned char length, unsigned char *start_adress)
{
  int i;
  
  
  start_adress[0]=start_value;
   /* start_adress[]
                  diff[]
               */
  
  for(i=1; i<=length; i++)
  {
     
    start_adress[i] = start_adress[i-1] + diff[i-1];
  }
  
}
/*!
  \brief     Adapt width of frequency bands in the second region
*/
static int
modifyBands(unsigned char max_band_previous, unsigned char * diff, unsigned char length)
{
  int change = max_band_previous - diff[0];
  
   
    DIV(1); 
  if ( change > (diff[length-1]-diff[0])/2 )
  {
    
    change = (diff[length-1]-diff[0])/2;
  }
   
  diff[0] += change;
  diff[length-1] -= change;
  
  shellsort(diff, length);
  
  return 0;
}
/*!
  \brief   Update high resolution frequency band table
*/
void
sbrdecUpdateHiRes(unsigned char * h_hires,
                  unsigned char * num_hires,
                  unsigned char * v_k_master,
                  unsigned char num_bands,
                  unsigned char xover_band)
{
  unsigned char i;
  
   
  *num_hires = num_bands-xover_band;
   /* h_hires[]
                  v_k_master[]
               */
  
  for(i=xover_band; i<=num_bands; i++) {
    
    h_hires[i-xover_band] = v_k_master[i];
  }
  
}
/*!
  \brief  Build low resolution table out of high resolution table
*/
void
sbrdecUpdateLoRes(unsigned char * h_lores,
                  unsigned char * num_lores,
                  unsigned char * h_hires,
                  unsigned char num_hires)
{
  unsigned char i;
  
   
  if( (num_hires & 1) == 0) {
     
    *num_lores = num_hires >> 1;
     /* h_lores[]
                    h_hires[]
                 */
    
    for(i=0; i<=*num_lores; i++)
    {
      
      h_lores[i] = h_hires[i*2];
    }
  }
  else {
      
    *num_lores = (num_hires+1) >> 1;
    
    h_lores[0] = h_hires[0];
     /* h_lores[]
                    h_hires[]
                 */
    
    for(i=1; i<=*num_lores; i++) {
      
      h_lores[i] = h_hires[i*2-1];
    }
  }
  
}
/*!
  \brief   Derive a low-resolution frequency-table from the master frequency table
*/
void
sbrdecDownSampleLoRes(unsigned char *v_result,
                      unsigned char num_result,
                      unsigned char *freqBandTableRef,
                      unsigned char num_Ref)
{
  int step;
  int i,j;
  int org_length,result_length;
  int v_index[MAX_FREQ_COEFFS/2];
  
  /* init */
  
  org_length = num_Ref;
  result_length = num_result;
  
  v_index[0] = 0;
  i=0;
   /* v_index[] */
  
  while(org_length > 0) {
    
    i++;
    DIV(1);
    step = org_length / result_length;
    
    org_length = org_length - step;
    
    result_length--;
     
    v_index[i] = v_index[i-1] + step;
  }
   /* v_result[]
                  v_index[]
               */
  
  for(j=0;j<=i;j++) {
     
    v_result[j]=freqBandTableRef[v_index[j]];
  }
  
}
/*!
  \brief   Sorting routine
*/
void shellsort(unsigned char *in, unsigned char n)
{
  int i, j, v, w;
  int inc = 1;
  
   
  
  do
  {
     
    inc = 3 * inc + 1;
  }
  while (inc <= n);
  
  do {
    DIV(1);
    inc = inc / 3;
     /* in[i] */
    
    for (i = inc; i < n; i++) {
      
      v = in[i];
      j = i;
       /* in[j-inc]
                      in[j]
                   */
      
      while ((w=in[j-inc]) > v) {
        
        in[j] = w;
        
        j -= inc;
         
        if (j < inc)
          break;
      }
      
      in[j] = v;
    }
  } while (inc > 1);
  
}
/*!
  \brief   Reset frequency band tables
  \return  errorCode, 0 if successful
*/
int
resetFreqBandTables(HANDLE_SBR_HEADER_DATA hHeaderData)
{
  int err;
  int k2,kx, lsb, usb;
  int     intTemp;
  unsigned char    nBandsLo, nBandsHi;
  HANDLE_FREQ_BAND_DATA hFreq = &hHeaderData->FreqBandData;
  
    
    
  err = sbrdecUpdateFreqScale(hFreq->v_k_master,
                              &hFreq->numMaster,
                              hHeaderData);
     
  if ( err || (hHeaderData->xover_band > hFreq->numMaster) ) {
    
    return -1;
  }
    
  sbrdecUpdateHiRes(hFreq->freqBandTable[HI], &nBandsHi, hFreq->v_k_master, hFreq->numMaster, hHeaderData->xover_band );
    
  sbrdecUpdateLoRes(hFreq->freqBandTable[LO], &nBandsLo, hFreq->freqBandTable[HI], nBandsHi);
   
  hFreq->nSfb[LO] = nBandsLo;
  hFreq->nSfb[HI] = nBandsHi;
    
  if ( (nBandsLo <= 0) || (nBandsLo > MAX_FREQ_COEFFS / 2) ) {
    
    return -1;
  }
   
  lsb = hFreq->freqBandTable[LOW_RES][0];
  usb = hFreq->freqBandTable[LOW_RES][nBandsLo];
   
  if ( (lsb > NO_ANALYSIS_CHANNELS) || (lsb >= usb) ) {
    
    return -1;
  }
   
  k2 = hFreq->freqBandTable[HI][nBandsHi];
  kx = hFreq->freqBandTable[HI][0];
   
  if (hHeaderData->noise_bands == 0)
  {
     
    hFreq->nNfb = 1;
  }
  else
  {
    float temp;
    
    temp = FloatFR_getNumOctaves(kx,k2);
     
    temp = temp * (float)hHeaderData->noise_bands;
    
    intTemp = (int)(temp + 0.5f);
    assert( intTemp ==  (int)((hHeaderData->noise_bands * log( (float)k2/kx) / (float)(log(2.0)))+0.5) );
    
    if( intTemp==0)
    {
      
      intTemp=1;
    }
     
    hFreq->nNfb = intTemp;
  }
   
  hFreq->nInvfBands = hFreq->nNfb;
    
  if( hFreq->nNfb > MAX_NOISE_COEFFS ) {
    
    return -1;
  }
   
  sbrdecDownSampleLoRes(hFreq->freqBandTableNoise,
                        hFreq->nNfb,
                        hFreq->freqBandTable[LO],
                        nBandsLo);
   
  hFreq->lowSubband  = lsb;
  hFreq->highSubband = usb;
  
  return 0;
}
