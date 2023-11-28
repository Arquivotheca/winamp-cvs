#pragma once
#include "../in_ogg/svc_oggdecoder.h"
#include <vorbis/codec.h>

// {851B80FD-421F-4787-82E7-CC6A8258352D}
static const GUID ogg_vorbis_guid = 
{ 0x851b80fd, 0x421f, 0x4787, { 0x82, 0xe7, 0xcc, 0x6a, 0x82, 0x58, 0x35, 0x2d } };


class OggDecoderFactory : public svc_oggdecoder
{
public:
	static const char *getServiceName() { return "Ogg Vorbis Decoder"; }
	static GUID getServiceGuid() { return ogg_vorbis_guid; }

	ifc_oggdecoder *CreateDecoder(const ogg_packet *packet);
protected:
	RECVS_DISPATCH;
};

class OggVorbisDecoder : public ifc_oggdecoder
{
public:
	friend class OggDecoderFactory;
	OggVorbisDecoder();
private:
	unsigned int bps;
	vorbis_info info;
	vorbis_dsp_state dsp;
	vorbis_block block;
	vorbis_comment comment;
	ogg_int64_t packet_number;

	RECVS_DISPATCH;
};