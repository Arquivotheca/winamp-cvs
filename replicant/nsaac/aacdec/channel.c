/*
  channel info
*/
#include "channel.h"
#include "bitbuffer.h"
#include "conceal.h"
#include "stereo.h"
#include "pns.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function reads the bitstream elements of the individual channel stream.
  Depending on window sequence the subroutine for short or long blocks are called.
*/
int ReadICS(HANDLE_BIT_BUF bs,                             /*!< pointer to bitstream */
            CAacDecoderChannelInfo *pAacDecoderChannelInfo /*!< pointer to aac decoder channel info */
            )
{
  int ErrorStatus = AAC_DEC_OK;
  /* reads one individual_channel_stream */
  pAacDecoderChannelInfo->RawDataInfo.GlobalGain = (unsigned char) GetBits(bs,8);
   
  if (!IsValid(&pAacDecoderChannelInfo->IcsInfo))
  {
      
    if ((ErrorStatus = IcsRead(bs,&pAacDecoderChannelInfo->IcsInfo)))
    {
      
      return (ErrorStatus);
    }
  }
  
  if (IsLongBlock(&pAacDecoderChannelInfo->IcsInfo))
  {
       
    if ((ErrorStatus = CLongBlock_Read(bs,pAacDecoderChannelInfo,pAacDecoderChannelInfo->RawDataInfo.GlobalGain)))
    {
      
      return (ErrorStatus);
    }
  }
  else
  {
    
    CShortBlock_Init(pAacDecoderChannelInfo);
       
    if ((ErrorStatus = CShortBlock_Read(bs,pAacDecoderChannelInfo,pAacDecoderChannelInfo->RawDataInfo.GlobalGain)))
    {
      
      return (ErrorStatus);
    }
  }
  
  return (ErrorStatus);
}
/*
  The function reads the bitstream elements of the single channel element.
  return:  element instance tag
*/
int CSingleChannelElement_Read(HANDLE_BIT_BUF bs,                                 /*!< pointer to bitstream */
                               CAacDecoderChannelInfo pAacDecoderChannelInfo[2], /*!< pointer to aac decoder channel info */
                               CStreamInfo *pStreamInfo                           /*!< pointer to stream info */
                               )
{
  int ErrorStatus = AAC_DEC_OK;
  pAacDecoderChannelInfo[L].RawDataInfo.ElementInstanceTag = (char) GetBits(bs,4);
  
  IcsReset(&pAacDecoderChannelInfo[L].IcsInfo,pStreamInfo);
  
  if (( ErrorStatus = ReadICS(bs, &pAacDecoderChannelInfo[L])))
    {
      return (ErrorStatus);
    }
  
  return (ErrorStatus);
}
/*
  The function reads the bitstream elements of the channel pair element.
  return:  element instance tag
*/
int CChannelPairElement_Read(HANDLE_BIT_BUF bs,                                 /*!< pointer to bitstream */
                             CAacDecoderChannelInfo pAacDecoderChannelInfo[2], /*!< pointer to aac decoder channel info */
                             CStreamInfo *pStreamInfo                           /*!< pointer to stream info */
                             )
{
  int ErrorStatus = AAC_DEC_OK;
  pAacDecoderChannelInfo[L].RawDataInfo.ElementInstanceTag = (char) GetBits(bs,4);
  IcsReset(&pAacDecoderChannelInfo[L].IcsInfo,pStreamInfo);
  IcsReset(&pAacDecoderChannelInfo[R].IcsInfo,pStreamInfo);
  pAacDecoderChannelInfo[L].RawDataInfo.CommonWindow = (char) GetBits(bs,1);
  if (pAacDecoderChannelInfo[L].RawDataInfo.CommonWindow)
  {
    if ((ErrorStatus = IcsRead(bs,&pAacDecoderChannelInfo[L].IcsInfo)))
    {
      
      return (ErrorStatus);
    }
    pAacDecoderChannelInfo[R].IcsInfo = pAacDecoderChannelInfo[L].IcsInfo;
    CJointStereo_Read(bs,
                      pAacDecoderChannelInfo[L].pJointStereoData,
                      GetWindowGroups(&pAacDecoderChannelInfo[L].IcsInfo),
                      GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L].IcsInfo));
  }
    
  if ((ErrorStatus = ReadICS(bs,&pAacDecoderChannelInfo[L])))
  {
    
    return (ErrorStatus);
  }
    
  if ((ErrorStatus = ReadICS(bs,&pAacDecoderChannelInfo[R])))
  {
    
    return (ErrorStatus);
  }
  
  return (ErrorStatus);
}
/*
  The function decodes a single channel element.
  return:  none
*/
void CSingleChannelElement_Decode(CAacDecoderChannelInfo pAacDecoderChannelInfo[2]  /*!< pointer to aac decoder channel info */
                                  )
{  
  ApplyTools (pAacDecoderChannelInfo,L);
}
/*
  The function decodes a channel pair element.
  return:  none
*/
void CChannelPairElement_Decode(CAacDecoderChannelInfo pAacDecoderChannelInfo[2]  /*!< pointer to aac decoder channel info */
                                )
{
  int channel;
  
  /* apply ms */
   
  if (pAacDecoderChannelInfo[L].RawDataInfo.CommonWindow) {
      
    if (pAacDecoderChannelInfo[L].PnsData.PnsActive || pAacDecoderChannelInfo[R].PnsData.PnsActive)
    {
      
      MapMidSideMaskToPnsCorrelation(pAacDecoderChannelInfo);
    }
          
    CJointStereo_ApplyMS(pAacDecoderChannelInfo,
                         GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L].IcsInfo),
                         GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L].IcsInfo),
                         GetWindowGroups(&pAacDecoderChannelInfo[L].IcsInfo),
                         GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L].IcsInfo));
  }
  /* apply intensity stereo */
         
  CJointStereo_ApplyIS(pAacDecoderChannelInfo,
                       GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L].IcsInfo),
                       GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L].IcsInfo),
                       GetWindowGroups(&pAacDecoderChannelInfo[L].IcsInfo),
                       GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L].IcsInfo),
                       pAacDecoderChannelInfo[L].RawDataInfo.CommonWindow ? 1 : 0);
  
  for (channel=0; channel < Channels; channel++)
  {
    
    ApplyTools (pAacDecoderChannelInfo, channel);
  }
  
}
void MapMidSideMaskToPnsCorrelation (CAacDecoderChannelInfo pAacDecoderChannelInfo[Channels])
{
  int group;
  char band;
  
   
  for (group = 0 ; group < pAacDecoderChannelInfo[L].IcsInfo.WindowGroups; group++) {
    
    for (band = 0 ; band < pAacDecoderChannelInfo[L].IcsInfo.MaxSfBands; band++) {
        
      if (pAacDecoderChannelInfo[L].pJointStereoData->MsUsed[band] & (1 << group)) { /* channels are correlated */
        
        CPns_SetCorrelation(&pAacDecoderChannelInfo[L], group, band);
           
        if (CPns_IsPnsUsed(&pAacDecoderChannelInfo[L], group, band) &&
            CPns_IsPnsUsed(&pAacDecoderChannelInfo[R], group, band))
        {
           
          pAacDecoderChannelInfo[L].pJointStereoData->MsUsed[band] ^= (1 << group);
        }
      }
    }
  }
  
}
