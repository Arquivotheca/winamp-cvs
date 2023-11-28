#pragma once
#include "../in_mp4/mpeg4audio.h"
#include "../a52dec/ac3_dec.h"

// {14131927-C1E5-4ad8-9BAA-4957C20DE1D0}
static const GUID mp4_a52_guid = 
{ 0x14131927, 0xc1e5, 0x4ad8, { 0x9b, 0xaa, 0x49, 0x57, 0xc2, 0xd, 0xe1, 0xd0 } };


class MP4A52Decoder : public MP4AudioDecoder
{
public:
	static const char *getServiceName() { return "ATSC A/52 MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_a52_guid; } 
	MP4A52Decoder();
	int Open();
	void Close();
	void Flush();
	int OutputFrameSize(size_t *frameSize);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int CanHandleCodec(const char *codecName);
	int CanHandleType(unsigned __int8 type);
	//int SetGain(float _gain) { gain=_gain; return MP4_SUCCESS; }

	AC3Dec *decoder;
	int bps;
protected:
	RECVS_DISPATCH;
};