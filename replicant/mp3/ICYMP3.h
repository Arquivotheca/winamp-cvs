#pragma once
#include "icy/ifc_icy_playback.h"
#include "nsmp3/mpgadecoder.h"
#include "gioicy.h"

class ICYMP3 : public ifc_icy_playback
{
public:
	ICYMP3();
	~ICYMP3();
	int Initialize(jnl_http_t http);
	int WASABICALL ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader);

private:
	jnl_http_t http;
	CMpgaDecoder *mpeg;
	bool paused;

	int Buffer(size_t bytes_to_buffer, ifc_http *http_parent, ifc_icy_reader *reader, ifc_player *player);
};