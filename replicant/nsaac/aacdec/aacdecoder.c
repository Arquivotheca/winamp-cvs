/*
  decoder main
*/
#include <assert.h>
#include "aac_rom.h"
#include "aac_ram.h"
#include "aacdecoder.h"
#include "bitbuffer.h"
#include "sbrdec/aacPLUScheck.h"
#include "dse.h"
#include "pce.h"
#include "foundation/error.h"

struct AAC_DECODER_INSTANCE {
  unsigned char frameOK;   /*!< Will be unset if the CRC, a consistency check etc. fails */
  unsigned long bitCount;
  long byteAlignBits;
  HANDLE_BIT_BUF pBs;
  float *pTimeData;
  SBRBITSTREAM *pStreamSbr;
  CStreamInfo StreamInfo;
  CAacDecoderChannelInfo AacDecoderChannelInfo[Channels];
  CAacDecoderStaticChannelInfo AacDecoderStaticChannelInfo[Channels];
	CAacDecoderDynamicCommonData AacDecoderDynamicCommonDataInit;
	CPnsStaticInterChannelData PnsStaticInterChannelData;
} AacDecoderInstance;

#include "conceal.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
void CPns_InitPns(CAacDecoderChannelInfo *pAacDecoderChannelInfo);
void CPns_InitInterChannelData(CAacDecoderChannelInfo *pAacDecoderChannelInfo);
/*
  The function initializes the pointers to AacDecoderChannelInfo for each channel,
  set the start values for window shape and window sequence of overlap&add to zero,
  set the overlap buffer to zero and initializes the pointers to the window coefficients.
*/
AACDECODER CAacDecoderOpen(HANDLE_BIT_BUF pBs,       /*!< pointer to bitbuffer structure */
                           SBRBITSTREAM *pStreamSbr, /*!< pointer to sbr bitstream structure */
                           float *pTimeData)         /*!< pointer to time data */
{
  int i,ch;
  
  /* initialize bit counter for syncroutine */
  
  AacDecoderInstance.bitCount = 0;
  /* initialize pointer to bit buffer structure */
  
  AacDecoderInstance.pBs = pBs;
  /* initialize pointer to time data */
  
  AacDecoderInstance.pTimeData = pTimeData;
  /* initialize pointer to sbr bitstream structure */
  
  AacDecoderInstance.pStreamSbr = pStreamSbr;
  /* initialize stream info */
   
  CStreamInfoOpen(&AacDecoderInstance.StreamInfo);
  
  
  for (ch=0; ch<Channels; ch++)
  {
    /* initialize CAacDecoderStaticChannelInfo for each channel */
    

    /* initialize overlap & add for each channel */
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].OverlapAddData.WindowShape = 0;
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].OverlapAddData.WindowSequence = 0;
         
    for (i=0; i<FRAME_SIZE/2; i++)
    {
      AacDecoderInstance.AacDecoderStaticChannelInfo[ch].OverlapAddData.pOverlapBuffer[i] = 0.0;
    }

    /* initialize window shapes for each channel */
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].pLongWindow[0] = OnlyLongWindowSine;
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].pShortWindow[0] = OnlyShortWindowSine;
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].pLongWindow[1] = OnlyLongWindowKBD;
    AacDecoderInstance.AacDecoderStaticChannelInfo[ch].pShortWindow[1] = OnlyShortWindowKBD;
    
    CConcealment_Init(&AacDecoderInstance.AacDecoderStaticChannelInfo[ch]);
  }
  /* these are static, but we access them via pointers inside the dynamic data */
  AacDecoderInstance.PnsStaticInterChannelData.current_seed = 0;
  AacDecoderInstance.PnsStaticInterChannelData.pns_frame_number = 0;
   /* counting post-operation */
  
  return &AacDecoderInstance;
}

int CAacDecoderInit(AACDECODER self, int samplingRate, int bitrate)
{
  int i;
  int numEntries = sizeof(SamplingRateInfoTable)/sizeof(SamplingRateInfo);
  if (!self)
  {
    return NErr_NullPointer;
  }
  self->StreamInfo.SamplingRate  = samplingRate;
  
  for (i=0; i<numEntries; i++)
  {
    if (samplingRate == SamplingRateInfoTable[i].SamplingFrequency)
      break;
  }
  if (i == numEntries)
  {
    return NErr_UnsupportedFormat;
  }
  self->StreamInfo.SamplingRateIndex = i;
  self->StreamInfo.BitRate = bitrate;
  return NErr_Success;
}
/*!
  The function decodes one aac frame. The decoding of coupling channel
  elements are not supported. The transport layer might signal, that the
  data of the current frame is invalid, e.g. as a result of a packet
  loss in streaming mode.
*/
int CAacDecoder_DecodeFrame(AACDECODER self,            /*!< pointer to aacdecoder structure */
                            int *frameSize,             /*!< pointer to frame size */
                            int *sampleRate,            /*!< pointer to sample rate */
                            int *numChannels,           /*!< pointer to number of channels */
                            char *channelMode,          /*!< mode: 0=mono, 1=stereo */
                            char frameOK                /*!< indicates if current frame data is valid */
                            )
{
  unsigned char aacChannels=0;
  long tmp;
  unsigned char ch;
  int type = 0;
  int ErrorStatus = frameOK;
  static int BlockNumber = 0;
  HANDLE_BIT_BUF bs = self->pBs;
  int previous_element;
  
  for (ch=0; ch<Channels; ch++)
  {
    self->AacDecoderChannelInfo[ch].pJointStereoData = &self->AacDecoderDynamicCommonDataInit.JointStereoData;
    self->AacDecoderChannelInfo[ch].pPnsInterChannelData = &self->AacDecoderDynamicCommonDataInit.PnsInterChannelData;
    self->AacDecoderChannelInfo[ch].pPnsStaticInterChannelData = &self->PnsStaticInterChannelData;
		CPns_InitPns(&self->AacDecoderChannelInfo[ch]);
  }
  self->frameOK = 1;
  
  CPns_InitInterChannelData(&self->AacDecoderChannelInfo[0]);
  aacChannels = 0;
  ByteAlign(bs,&self->byteAlignBits);
  previous_element = ID_END;
  while(type != ID_END && self->frameOK )
  {
    type = GetBits(bs,3);
    if (bs->cntBits < 0)
    {
      self->frameOK = 0;
    }
    switch(type)
    {
      case ID_SCE:
      
        /* Consistency check */
        if (aacChannels >= Channels)
				{
          self->frameOK = 0;
          break;
        }
          
        self->pStreamSbr->sbrElement[self->pStreamSbr->NrElements].ElementID = SBR_ID_SCE;
        if (self->frameOK) 
				{
          ErrorStatus = CSingleChannelElement_Read(bs,self->AacDecoderChannelInfo, &self->StreamInfo);
          if (ErrorStatus) 
					{
            self->frameOK = 0;
          }
        }
        if (self->frameOK)
        {
          CSingleChannelElement_Decode(self->AacDecoderChannelInfo);
          aacChannels += 1;
        }
        break;
      
      case ID_CPE:
       
        /* Consistency check */
        if (aacChannels >= Channels)
				{
          self->frameOK = 0;
          break;
        }
          
        self->pStreamSbr->sbrElement[self->pStreamSbr->NrElements].ElementID = SBR_ID_CPE;
        if (self->frameOK) 
				{
          ErrorStatus = CChannelPairElement_Read(bs,self->AacDecoderChannelInfo,&self->StreamInfo);
          
          if (ErrorStatus) 
					{             
            self->frameOK = 0;
          }
        }
         
        if (self->frameOK)
				{           
          CChannelPairElement_Decode(self->AacDecoderChannelInfo);          
          aacChannels += 2;
        }
        break;
      
      case ID_CCE:
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_CCE;
        self->frameOK = 0;
        break;

      case ID_LFE:         
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_LFE;
        self->frameOK = 0;
        break;
      
      case ID_DSE:
          
        DSE_Read(bs, &self->byteAlignBits);
        break;
      case ID_PCE:
				PCE_Read(bs, &self->byteAlignBits);
        //ErrorStatus = AAC_DEC_UNIMPLEMENTED_PCE;
        //self->frameOK = 0;
        break;
      
      case ID_FIL:
        tmp = bs->cntBits;
        FFRaacplus_checkForPayload(bs,self->pStreamSbr, previous_element);
        break;
      
      case ID_END:
        break;
    }
    
    previous_element = type;
  }
  /* Update number of channels (if valid) */
   
  if (self->frameOK) {
     
    self->StreamInfo.Channels = aacChannels;
  }
  self->frameOK = self->frameOK && frameOK;
  /* Inverse transform  */
  
  for (ch=0; ch<self->StreamInfo.Channels; ch++)
	{
    /* Conceal erred spectral data  */    
    CConcealment_Apply(&self->AacDecoderStaticChannelInfo[ch],
                       &self->AacDecoderChannelInfo[ch],
                       self->frameOK);
  
    if (IsLongBlock(&self->AacDecoderChannelInfo[ch].IcsInfo))
    {
      CLongBlock_FrequencyToTime(&self->AacDecoderStaticChannelInfo[ch],
                                 &self->AacDecoderChannelInfo[ch],
                                 &self->pTimeData[ch*MaximumBinsLong],
                                 1);
    }
    else
    {
      CShortBlock_FrequencyToTime(&self->AacDecoderStaticChannelInfo[ch],
                                  &self->AacDecoderChannelInfo[ch],
                                  &self->pTimeData[ch*MaximumBinsLong],
                                  1);
    }
  }
  ByteAlign(bs,&self->byteAlignBits);
  *frameSize = self->StreamInfo.SamplesPerFrame;
  *sampleRate = self->StreamInfo.SamplingRate;
  *numChannels = self->StreamInfo.Channels;
  if (*numChannels == 1)
  {
    *channelMode = 0;
  }
  else 
	{     
    if (*numChannels == 2)
      *channelMode = 1;
    else
      *channelMode = 2;
  }
  BlockNumber++;
  return ErrorStatus;
}
