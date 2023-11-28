#pragma once
#include "../in_mkv/svc_mkvdecoder.h"

// {7405C1CE-6CD6-4975-B22B-8DBABAA64C44}
static const GUID AACMKVGUID = 
{ 0x7405c1ce, 0x6cd6, 0x4975, { 0xb2, 0x2b, 0x8d, 0xba, 0xba, 0xa6, 0x4c, 0x44 } };


class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "AAC MKV Decoder"; }
	static GUID getServiceGuid() { return AACMKVGUID; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};