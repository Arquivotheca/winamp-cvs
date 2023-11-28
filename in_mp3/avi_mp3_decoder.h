#pragma once
#include "../in_avi/svc_avidecoder.h"
#include "../in_avi/ifc_aviaudiodecoder.h"
#include "../mp3/mp3dec/mpgadecoder.h"

// {11D89BF5-014E-463a-B284-385FF0662FCC}
static const GUID avi_mp3_guid = 
{ 0x11d89bf5, 0x14e, 0x463a, { 0xb2, 0x84, 0x38, 0x5f, 0xf0, 0x66, 0x2f, 0xcc } };

class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "MP3 AVI Decoder"; }
	static GUID getServiceGuid() { return avi_mp3_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIMP3Decoder : public ifc_aviaudiodecoder
{
public:
	AVIMP3Decoder(CMpgaDecoder *mp3, unsigned int bps, unsigned max_channels, bool floating_point);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_mkvaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();

private:
	CMpgaDecoder *mp3;
	unsigned int bits;
	int pregap;
	unsigned int max_channels;
	bool floating_point;
};