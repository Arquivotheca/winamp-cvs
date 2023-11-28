#include "ICYDemuxer.h"
#include "ICYReader.h"
ICYDemuxer::ICYDemuxer()
{
	http=0;
	playback=0;
}

ICYDemuxer::~ICYDemuxer()
{
	if (playback)
		playback->Release();
	
	jnl_http_release(http);
}

void ICYDemuxer::Initialize(jnl_http_t http, ifc_icy_playback *playback)
{
	this->http = http;
	jnl_http_retain(http);
	this->playback = playback;
	if (playback)
		playback->Retain();
}

int WASABICALL ICYDemuxer::HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)
{
	ICYReader reader(http, player);
	player->SetSeekable(0);
	int ret = playback->Run(http_parent, player, &reader);
	switch(ret)
	{
	case NErr_Success:
		return NErr_Success;
	case NErr_EndOfFile:
		player->OnEndOfFile();
		/* TODO: continue to call SetPosition, if needed.  but ICY streams don't really "end" */
		return NErr_EndOfFile;
	default:
		player->OnError(ret);
		return ret;
	}	
}
