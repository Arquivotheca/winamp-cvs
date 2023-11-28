#pragma once
#include "icy/ifc_icy_playback.h"
#include "IAACDecoder.h"

class ICYAAC : public ifc_icy_playback
{
public:
	ICYAAC();
	~ICYAAC();
	int Initialize(jnl_http_t http);
	int WASABICALL ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader);

private:
	jnl_http_t http;

	IAACDecoder *decoder;
	
	/* i'm going to put these here just to keep them off the stack */
	uint8_t buffer[8192];
	int16_t output_buffer[4096];

	/* helper function for making sure we read exactly as much as we want to */
	int ICYRead(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested);

	/* helper function that keeps looping until the peek is satisfied */
	int ICYPeek(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested);

	/* helper function to synchronization to the stream */
	int ICYSync(ifc_icy_reader *reader);
};