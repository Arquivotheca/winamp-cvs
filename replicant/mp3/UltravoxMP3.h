#pragma once
#include "ultravox/ifc_ultravox_playback.h"
#include "nsmp3/mpgadecoder.h"

class UltravoxMP3 : public ifc_ultravox_playback
{
public:
	UltravoxMP3(jnl_http_t http);
	~UltravoxMP3();

private:
		/* ifc_ultravox_playback implementation */
	int WASABICALL UltravoxPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_ultravox_reader *reader);

	jnl_http_t http;	
	CMpgaDecoder *mpeg;
};