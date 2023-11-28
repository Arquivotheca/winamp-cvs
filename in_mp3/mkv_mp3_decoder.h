#pragma once
#include "../in_mkv/svc_mkvdecoder.h"
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include "../mp3/mp3dec/mpgadecoder.h"

// {A08A5A0C-DEF7-4604-9F35-D5A5E9B1F4DF}
static const GUID mkv_mp3_guid = 
{ 0xa08a5a0c, 0xdef7, 0x4604, { 0x9f, 0x35, 0xd5, 0xa5, 0xe9, 0xb1, 0xf4, 0xdf } };


class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "MP3 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_mp3_guid; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class MKVMP3Decoder : public ifc_mkvaudiodecoder
{
public:
	MKVMP3Decoder(CMpgaDecoder *mp3, unsigned int bps, unsigned max_channels, bool floating_point);
	
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
	CMpgaDecoder *mp3;
	unsigned int bits;
	int pregap;
	unsigned int max_channels;
	bool floating_point;
};