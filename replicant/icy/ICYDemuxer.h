#pragma once
#include "http/ifc_http_demuxer.h"
#include "icy/ifc_icy_playback.h"

class ICYDemuxer : public ifc_http_demuxer
{
public:
	ICYDemuxer();
	~ICYDemuxer();
	void Initialize(jnl_http_t http, ifc_icy_playback *playback);
private:
	jnl_http_t http;
	ifc_icy_playback *playback;

	int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters);
};
