#pragma once
#include "../in_avi/svc_avidecoder.h"
#include "../in_avi/ifc_aviaudiodecoder.h"
#include "../a52dec/ac3_dec.h"
#include "../nu/SpillBuffer.h"

// {6607B3AB-B152-4bbc-8260-1759CED0B3E3}
static const GUID avi_a52_guid = 
{ 0x6607b3ab, 0xb152, 0x4bbc, { 0x82, 0x60, 0x17, 0x59, 0xce, 0xd0, 0xb3, 0xe3 } };

class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "ATSC A/52 Decoder"; }
	static GUID getServiceGuid() { return avi_a52_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIA52Decoder : public ifc_aviaudiodecoder
{
public:
	AVIA52Decoder(AC3Dec *ctx, unsigned int bps, unsigned max_channels, bool floating_point);
	
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
/* data */
	AC3Dec *decoder;
	unsigned int bps, max_channels;
	bool floating_point;
	size_t preDelay;
	SpillBuffer spill_buffer;
};