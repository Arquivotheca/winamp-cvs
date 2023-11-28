/*
  joint stereo processing
*/
#include <math.h>
#include "stereo.h"
#include "aac_rom.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function reads joint stereo data from bitstream.
*/
void CJointStereo_Read(HANDLE_BIT_BUF bs,                   /*!< pointer to bitstream */
                       CJointStereoData *pJointStereoData,  /*!< pointer to joint stereo side info */
                       int windowGroups,                    /*!< number of window groups */
                       int scaleFactorBandsTransmitted)     /*!< number of transmitted scalefactor bands */
{
  int group,band;
  
    
  pJointStereoData->MsMaskPresent = (char) GetBits(bs,2);
  /* Clear MS mask */
   /* pJointStereoData->MsUsed[] */
  
  for (band=0; band<scaleFactorBandsTransmitted; band++)
  {
    
    pJointStereoData->MsUsed[band] = 0;
  }
   
  switch (pJointStereoData->MsMaskPresent)
  {
    case 0 : /* no M/S */
      /* all flags are already cleared */
      break ;
    case 1 : /* read ms_used */
      
      for (group=0; group<windowGroups; group++)
      {
         /* pJointStereoData->MsUsed[] */
        
        for (band=0; band<scaleFactorBandsTransmitted; band++)
        {
             
          pJointStereoData->MsUsed[band] |= (GetBits(bs,1) << group);
        }
      }
      break ;
    case 2 : /* full spectrum M/S */
       /* pJointStereoData->MsUsed[] */
      
      for (band=0; band<scaleFactorBandsTransmitted; band++)
      {
        
        pJointStereoData->MsUsed[band] = 255 ;  /* set all flags to 1 */
      }
      break ;
  }
  
}
/*
  The function applies MS stereo.
*/
void CJointStereo_ApplyMS(CAacDecoderChannelInfo pAacDecoderChannelInfo[2],   /*!< aac channel info */
                          const short *pScaleFactorBandOffsets,                /*!< pointer to scalefactor band offsets */
                          char *pWindowGroupLength,                            /*!< pointer to window group length array */
                          int windowGroups,                                    /*!< number of window groups */
                          int scaleFactorBandsTransmitted)                     /*!< number of transmitted scalefactor bands */
{
  CJointStereoData *pJointStereoData = pAacDecoderChannelInfo[0].pJointStereoData;
  int window, group, groupwin, band, index;
  
    
  
  for (window=0,group=0; group<windowGroups; group++)
  {
     
    for (groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)
    {
      float *LeftSpectrum = &pAacDecoderChannelInfo[0].aSpectralCoefficient[window*FRAME_SIZE/8];
      float *RightSpectrum = &pAacDecoderChannelInfo[1].aSpectralCoefficient[window*FRAME_SIZE/8];
      
      for (band=0; band<scaleFactorBandsTransmitted; band++)
      {
          
        if (pJointStereoData->MsUsed[band] & (1 << group))
        {
          
          for (index=pScaleFactorBandOffsets[band]; index<pScaleFactorBandOffsets[band+1]; index++)
          {
            float LeftCoefficient  = LeftSpectrum[index];
            float RightCoefficient = RightSpectrum[index];
             
            LeftSpectrum[index]  = LeftCoefficient+RightCoefficient;
            RightSpectrum[index] = LeftCoefficient-RightCoefficient;
          }
        }
      }
    }
  }
  
}
/*
  The function applies intensity stereo.
*/
void CJointStereo_ApplyIS(CAacDecoderChannelInfo pAacDecoderChannelInfo[2], /*!< aac channel info */
                          const short *pScaleFactorBandOffsets,              /*!< pointer to scalefactor band offsets */
                          char *pWindowGroupLength,                          /*!< pointer to window group length array */
                          int windowGroups,                                  /*!< number of window groups */
                          int scaleFactorBandsTransmitted,                   /*!< number of transmitted scalefactor bands */
                          unsigned char CommonWindow)                        /*!< common window bit */
{
  CJointStereoData *pJointStereoData = pAacDecoderChannelInfo[0].pJointStereoData;
  int window, group, groupwin, band, index;
  for (window=0,group=0; group<windowGroups; group++)
  {
    const char *CodeBook = &pAacDecoderChannelInfo[1].aCodeBook[group*MaximumScaleFactorBandsShort];
    const short *ScaleFactor = &pAacDecoderChannelInfo[1].aScaleFactor[group*MaximumScaleFactorBandsShort];
    
    for (groupwin=0; groupwin<pWindowGroupLength[group]; groupwin++, window++)
    {
      const float *LeftSpectrum = &pAacDecoderChannelInfo[0].aSpectralCoefficient[window*FRAME_SIZE/8];
      float *RightSpectrum = &pAacDecoderChannelInfo[1].aSpectralCoefficient[window*FRAME_SIZE/8];
 
      for (band=0; band<scaleFactorBandsTransmitted; band++)
      {
          
        if ((CodeBook [band] == INTENSITY_HCB) || (CodeBook [band] == INTENSITY_HCB2))
        {
          float scale = (float) pow (0.5f, 0.25f * (ScaleFactor[band] + 100));
             
          
            
          if (CommonWindow && (pJointStereoData->MsUsed[band] & (1 << group)))
          {
             
            if (CodeBook[band] == INTENSITY_HCB) /* _NOT_ in-phase */
            {
              
              scale = -scale ;
            }
          }
          else
          {
             
            if (CodeBook[band] == INTENSITY_HCB2) /* out-of-phase */
            {
              
              scale = -scale ;
            }
          }
          
          for (index=pScaleFactorBandOffsets[band]; index<pScaleFactorBandOffsets[band+1]; index++)
          {
             
            RightSpectrum[index] = LeftSpectrum[index] * scale;
          }
        }
      }
    }
  }
  
}
