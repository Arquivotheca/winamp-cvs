#pragma once
#include "../in_mkv/svc_mkvdecoder.h"
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include <dca.h>

// {DDDBBA06-FC0A-43c9-A15E-75374F2FC768}
static const GUID mkv_dca_guid = 
{ 0xdddbba06, 0xfc0a, 0x43c9, { 0xa1, 0x5e, 0x75, 0x37, 0x4f, 0x2f, 0xc7, 0x68 } };


class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "DCA MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_dca_guid; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVDCADecoder : public ifc_mkvaudiodecoder
{
public:
	MKVDCADecoder(dca_state_t *ctx, unsigned int bps, unsigned max_channels, bool floating_point);
	
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
	dca_state_t *decoder;
	unsigned int bps, max_channels;
	bool floating_point;
	size_t preDelay;

	int flags, sample_rate, bit_rate, frame_length;
	int channels;
	bool syncd;
};