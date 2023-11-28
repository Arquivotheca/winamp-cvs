#include "impl_aacplusdecoder.h"
#include "aacplusdec.h"
AacPlusDecoder::AacPlusDecoder() : handle(0)
{

}

AACPLUSDEC_ERROR AacPlusDecoder::EasyOpen(AACPLUSDEC_OUTPUTFORMAT outputFormat, int nMaxAudioChannels)
{
	return aacPlusDecEasyOpen(&handle, outputFormat, nMaxAudioChannels);
}

void AacPlusDecoder::Close()
{
	aacPlusDecClose(&handle);
	handle=0;
}

AACPLUSDEC_ERROR AacPlusDecoder::Restart()
{
	return aacPlusDecRestart(handle);
}

AACPLUSDEC_EXPERTSETTINGS *AacPlusDecoder::GetDecoderSettingsHandle()
{
	return aacPlusDecGetDecoderSettingsHandle(handle);
}

AACPLUSDEC_ERROR AacPlusDecoder::SetDecoderSettings()
{
	return aacPlusDecSetDecoderSettings(handle);
}

AACPLUSDEC_STREAMPROPERTIES *AacPlusDecoder::GetStreamPropertiesHandle()
{
	return aacPlusDecGetStreamPropertiesHandle(handle);
}

AACPLUSDEC_ERROR AacPlusDecoder::StreamFeed(unsigned char *pucBitstrmBufIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hBitstrmBufInfoInOut)
{
	return aacPlusStreamFeed(handle, pucBitstrmBufIn, hBitstrmBufInfoInOut);
}

AACPLUSDEC_ERROR AacPlusDecoder::StreamDecode(void  *pPcmAudioBufOut, AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut, unsigned char *pucDataStreamBufOut, AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut)
{
return aacPlusStreamDecode(handle, pPcmAudioBufOut, hPcmAudioBufInfoInOut, pucDataStreamBufOut, hDataStreamBufInfoInOut);
}


AACPLUSDEC_ERROR AacPlusDecoder::ReadConfigStream(unsigned char *pucConfigStreamBufferIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hConfigStreamBufferInfoInOut, AACPLUSDEC_CONFIGTYPE nConfigTypeIn, int bConfigStreamInBand, AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn)
{
	return aacPlusDecReadConfigStream(handle, pucConfigStreamBufferIn, hConfigStreamBufferInfoInOut,  nConfigTypeIn, bConfigStreamInBand, bitstreamFormatIn);
}


AACPLUSDEC_ERROR AacPlusDecoder::FrameDecode(
                    void          *pPcmAudioBufOut,                     
                    AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,  
                    unsigned char *pucFrameBufferIn,                    
                    AACPLUSDEC_BITSTREAMBUFFERINFO *hFrameBufferInfoInOut,
                    int            bFrameCorrupt,                         
                    unsigned char *pucDataStreamBufOut,                   
                    AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut )
{
return aacPlusFrameDecode( handle,
                    pPcmAudioBufOut,          
                    hPcmAudioBufInfoInOut,    
                    pucFrameBufferIn,         
                    hFrameBufferInfoInOut,    
                    bFrameCorrupt,                                pucDataStreamBufOut,      
                    hDataStreamBufInfoInOut  );
}



#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS AacPlusDecoder
START_DISPATCH;
CB(AACPLUSDECODER_EASY_OPEN, EasyOpen)
VCB(AACPLUSDECODER_CLOSE, Close)
CB(AACPLUSDECODER_RESTART, Restart)
CB(AACPLUSDECODER_READ_CONFIG_STREAM, ReadConfigStream)
CB(AACPLUSDECODER_GET_DECODER_SETTINGS_HANDLE, GetDecoderSettingsHandle)
CB(AACPLUSDECODER_SET_DECODER_SETTINGS, SetDecoderSettings) 
CB(AACPLUSDECODER_GET_STREAM_PROPERTIES_HANDLE, GetStreamPropertiesHandle)
CB(AACPLUSDECODER_STREAM_FEED, StreamFeed)
CB(AACPLUSDECODER_STREAM_DECODE, StreamDecode)
CB(AACPLUSDECODER_FRAME_DECODE, FrameDecode)
END_DISPATCH;

		