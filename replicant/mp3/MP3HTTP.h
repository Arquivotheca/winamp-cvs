#pragma once
#include "http/ifc_http_demuxer.h"
#include "nsmp3/mpgadecoder.h"
#include "giojnet.h"
class MP3HTTP : public ifc_http_demuxer
{
public:
	MP3HTTP();
	~MP3HTTP();

	int Initialize(jnl_http_t http);
private:
	/* ifc_http_demuxer implementation */
	int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters);

	/* member data */
	GioJNetLib *gio_jnetlib;
	jnl_http_t http;
	CMpgaDecoder *mpeg;
};
