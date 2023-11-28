#ifndef NULLSOFT_IMPL_AACPLUSDECODERH
#define NULLSOFT_IMPL_AACPLUSDECODERH
#include "api_aacplusdecoder.h"
#include "aacplusdectypes.h"
class AacPlusDecoder : public api_aacplusdecoder
{
public:
	AacPlusDecoder();
	AACPLUSDEC_ERROR EasyOpen(AACPLUSDEC_OUTPUTFORMAT outputFormat, int nMaxAudioChannels);
	void Close();
	AACPLUSDEC_ERROR Restart();
	AACPLUSDEC_ERROR ReadConfigStream(unsigned char *pucConfigStreamBufferIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hConfigStreamBufferInfoInOut, AACPLUSDEC_CONFIGTYPE nConfigTypeIn, int bConfigStreamInBand, AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn);
	AACPLUSDEC_EXPERTSETTINGS *GetDecoderSettingsHandle();
	AACPLUSDEC_ERROR SetDecoderSettings();
	AACPLUSDEC_STREAMPROPERTIES *GetStreamPropertiesHandle();
	AACPLUSDEC_ERROR StreamFeed(unsigned char *pucBitstrmBufIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hBitstrmBufInfoInOut);
	AACPLUSDEC_ERROR StreamDecode(void  *pPcmAudioBufOut, AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut, unsigned char *pucDataStreamBufOut, AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut);
	
AACPLUSDEC_ERROR FrameDecode(
                    void          *pPcmAudioBufOut,                     
                    AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,  
                    unsigned char *pucFrameBufferIn,                    
                    AACPLUSDEC_BITSTREAMBUFFERINFO *hFrameBufferInfoInOut,
                    int            bFrameCorrupt,                         
                    unsigned char *pucDataStreamBufOut,                   
                    AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut );
protected:
	RECVS_DISPATCH;
	HANDLE_AACPLUSDEC_DECODER handle;
};
#endif