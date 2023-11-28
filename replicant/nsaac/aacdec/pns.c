/*
  perceptual noise substitution tool
*/
#include <math.h>
#include <assert.h>
#include "aac_rom.h"
#include "channelinfo.h"
#include "block.h"
#include "pns.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */

#define PNS_BAND_FLAGS_MASK              ((1 << PNS_BAND_FLAGS_SHIFT) - 1)
#define PNS_BAND_FLAGS_SHIFT             3
#define NOISE_OFFSET 90           /* cf. ISO 14496-3 p. 175 */

static void GenerateRandomVector(float scale, float spec[], int bandOffsetStart, int bandOffsetEnd, short *randomState)
{
  int i;
  float nrg = 0.0f;
   
  for (i = bandOffsetStart; i < bandOffsetEnd; i++) {
      
    spec[i] = *randomState = (0x529L * *randomState) + 0x3a7fL; /* snd */
    nrg += spec[i] * spec[i];
  }
   
  scale /= (float) sqrt(nrg);
  
  for (i = bandOffsetStart; i < bandOffsetEnd; i++) {
     
    spec[i] *= scale;
  }
}

/* The function initializes the InterChannel data */
void CPns_InitInterChannelData(CAacDecoderChannelInfo *pAacDecoderChannelInfo) 
{
  unsigned int i;
  CPnsInterChannelData *pInterChannelData = pAacDecoderChannelInfo->pPnsInterChannelData;
  
  for (i = 0; i < PNS_BAND_FLAGS_SIZE; i++)
  {
    pInterChannelData->correlated[i] = 0;
  }
}

/* The function initializes the PNS data */
void CPns_InitPns(CAacDecoderChannelInfo *pAacDecoderChannelInfo) 
{
  unsigned int i;
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;
  
  for (i = 0; i < PNS_BAND_FLAGS_SIZE; i++)
  {
    pPnsData->pnsUsed[i] = 0;
  }
  
  pPnsData->PnsActive = 0;
  pPnsData->CurrentEnergy = 0;
}

/*
  The function returns a value indicating whether PNS is used or not 
  acordding to the noise energy
  return:  PNS used
*/
int CPns_IsPnsUsed (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
               const int group,
               const int band)
{
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
        
  
  return (pPnsData->pnsUsed[pns_band >> PNS_BAND_FLAGS_SHIFT] >> (pns_band & PNS_BAND_FLAGS_MASK)) & (unsigned char)1;
}

/* The function activates the noise correlation between the channel pair */
void CPns_SetCorrelation(CAacDecoderChannelInfo *pAacDecoderChannelInfo, const int group, const int band)
{
  CPnsInterChannelData *pInterChannelData = pAacDecoderChannelInfo->pPnsInterChannelData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
     
  pInterChannelData->correlated[pns_band >> PNS_BAND_FLAGS_SHIFT] |= (unsigned char)1 << (pns_band & PNS_BAND_FLAGS_MASK);
}

/*
  The function indicates if the noise correlation between the channel pair
  is activated
  return:  PNS used
*/
int CPns_IsCorrelated(CAacDecoderChannelInfo *pAacDecoderChannelInfo, const int group, const int band)
{
  CPnsInterChannelData *pInterChannelData = pAacDecoderChannelInfo->pPnsInterChannelData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
  
  return (pInterChannelData->correlated[pns_band >> PNS_BAND_FLAGS_SHIFT] >> (pns_band & PNS_BAND_FLAGS_MASK)) & (unsigned char)1;
}

/* The function reads the PNS information from the bitstream */
void CPns_Read (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                HANDLE_BIT_BUF bs,
                const CodeBookDescription *hcb,
                unsigned char global_gain,
                int band,
                int group /* = 0 */)
{
  int delta ;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;
  
  if (pPnsData->PnsActive) 
	{
    delta = CBlock_DecodeHuffmanWord (bs, hcb->CodeBook) - 60;
  }
	else 
	{
    int noiseStartValue = GetBits(bs,9);
    delta = noiseStartValue - 256 ;
    pPnsData->PnsActive = 1;
    pPnsData->CurrentEnergy = global_gain - NOISE_OFFSET;
  }
    
  pPnsData->CurrentEnergy += delta ;
  pAacDecoderChannelInfo->aScaleFactor[pns_band] = pPnsData->CurrentEnergy;
  pPnsData->pnsUsed[pns_band >> PNS_BAND_FLAGS_SHIFT] |= (unsigned char)1 << (pns_band & PNS_BAND_FLAGS_MASK);
}

/*
  The function applies PNS (i.e. it generates noise) on the bands
  flagged as noisy bands
*/
void CPns_Apply (CAacDecoderChannelInfo pAacDecoderChannelInfo[], int channel)
{
  int window, group, groupwin, band;
  CPnsData *pPnsData = &pAacDecoderChannelInfo[channel].PnsData;
  CIcsInfo *pIcsInfo = &pAacDecoderChannelInfo[channel].IcsInfo;
   
  if (pPnsData->PnsActive) {
    const short *BandOffsets = GetScaleFactorBandOffsets(pIcsInfo);
    int fft_exp = IsLongBlock(&pAacDecoderChannelInfo[channel].IcsInfo)? 9 : 6; /* This coefficient is related to the spectral exponent used in the
                                                                                    requantization (see functions CLongBlock_ReadSpectralData() and 
                                                                                    CShortBlock_ReadSpectralData() ).
                                                                                 */
     
    for (window = 0, group = 0; group < GetWindowGroups(pIcsInfo); group++) 
		{
      for (groupwin = 0; groupwin < GetWindowGroupLength(pIcsInfo, group); groupwin++, window++) 
			{
        float *spectrum = &pAacDecoderChannelInfo[channel].aSpectralCoefficient[FRAME_SIZE/8*window];
          
        for (band = 0 ; band < GetScaleFactorBandsTransmitted(pIcsInfo); band++) 
				{
          if (CPns_IsPnsUsed (&pAacDecoderChannelInfo[channel], group, band)) 
					{
            unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
            float scale = (float) pow(2.0, 0.25 * pAacDecoderChannelInfo[channel].aScaleFactor[pns_band] - fft_exp);
               
            if (CPns_IsCorrelated(&pAacDecoderChannelInfo[0], group, band)) {
              if (channel == 0) {
                /* store random state for right channel */
                pAacDecoderChannelInfo[0].pPnsInterChannelData->randomState[pns_band] = pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->current_seed;
                GenerateRandomVector(scale,
                                     spectrum,
                                     BandOffsets[band],
                                     BandOffsets[band + 1],
                                     &(pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->current_seed));
              } else {
                /* use same random state as was used for left channel */
                GenerateRandomVector(scale,
                                     spectrum,
                                     BandOffsets[band],
                                     BandOffsets[band + 1],
                                     &(pAacDecoderChannelInfo[0].pPnsInterChannelData->randomState[pns_band]));
              }
            }
            else {
              GenerateRandomVector(scale,
                                   spectrum,
                                   BandOffsets[band],
                                   BandOffsets[band + 1],
                                   &(pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->current_seed));
            }
          }
        }
      }
    }
    pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->current_seed += pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->pns_frame_number;
  } 
  if (channel == 0) {
    pAacDecoderChannelInfo[0].pPnsStaticInterChannelData->pns_frame_number++;
  }
  
}
