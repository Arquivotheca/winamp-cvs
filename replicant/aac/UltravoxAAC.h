#pragma once
#include "ultravox/ifc_ultravox_playback.h"
#include "android/PVMP4.h"

class UltravoxAAC : public ifc_ultravox_playback
{
public:
	UltravoxAAC(jnl_http_t http);
	~UltravoxAAC();
	int WASABICALL UltravoxPlayback_Run(ifc_http *http_parent, svc_output *output, ifc_player *player, ifc_ultravox_reader *reader, ifc_playback_parameters *secondary_parameters);

private:
	jnl_http_t http;
	pvmp4_decoder_value_t decoder;
	
	int16_t output_buffer[4096];

	/* helper function that keeps looping until the peek is satisfied */
	int UltravoxPeek(ifc_ultravox_reader *reader, uint8_t *buffer, size_t bytes_requested);

	/* helper function to synchronization to the stream */
	int UltravoxSync(ifc_ultravox_reader *reader);
};