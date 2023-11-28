#pragma once
#include "nsmp3/mpgadecoder.h"
#include "mp3/giofile_crt.h"
#include "audio/ifc_audio_decoder_packet.h"

class MP3DecoderPacket : public ifc_audio_decoder_packet
{
public:
	MP3DecoderPacket();
	~MP3DecoderPacket();
	int Initialize(nx_uri_t filename);

private:
	/* TODO: we might want three of these so we can cut end gap correctly */
	float decode_buffer[1152*2];
	bool packet_available;
	size_t bytes_available;
	GioFile *giofile;
	CMpgaDecoder *mpeg;

	int DecodeNextFrame(bool first=false);
};