#pragma once
#include "../in_mkv/svc_mkvdecoder.h"
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include "../a52dec/ac3_dec.h"

// {3518EC98-61BD-414c-A943-15065D237CAF}
static const GUID mkv_a52_guid = 
{ 0x3518ec98, 0x61bd, 0x414c, { 0xa9, 0x43, 0x15, 0x6, 0x5d, 0x23, 0x7c, 0xaf } };

class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "ATSC A/52 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_a52_guid; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVA52Decoder : public ifc_mkvaudiodecoder
{
public:
	MKVA52Decoder(AC3Dec *ctx, unsigned int bps, unsigned max_channels, bool floating_point);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_mkvaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();

private:
	
	/* internal implementation */

	/* data */
	AC3Dec *decoder;
	unsigned int bps, max_channels;
	bool floating_point;
	size_t preDelay;
};