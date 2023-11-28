#pragma once
#include "http/ifc_http_demuxer.h"
#include "UltravoxHeader.h"
#include "ifc_ultravox_reader.h"
#include "ifc_ultravox_playback.h"
#include "SHOUTcast2Metadata.h"

class UltravoxDemuxer : public ifc_http_demuxer, public ifc_ultravox_reader
{
public:
	UltravoxDemuxer(jnl_http_t http);
	~UltravoxDemuxer();

private:
	enum UltravoxState
	{
		State_Sync = 0, /* need to read a header */
		State_ReadingData = 1, /* read the header already, ultravox_header is valid and the next bytes in socket are data */
		State_Failure = 2, /* jnetlib returned something we didn't like */
	};

	/* internal helper functions */
	int Internal_Read(uint8_t *buffer, size_t bytes_requested);
	int Internal_Skip(size_t bytes_to_skip);
	int Internal_SyncToData(); /* synchronizes stream to next data packet */
	int Internal_Run();
	int Internal_On3902Metadata();
	int Internal_Buffer(size_t bytes); /* spins until specified number of bytes are available in the socket */

	/* ifc_ultravox_reader implementation */
	int WASABICALL UltravoxReader_Read(void *buffer, size_t buffer_length, size_t *bytes_read);
	int WASABICALL UltravoxReader_Peek(void *buffer, size_t buffer_length, size_t *bytes_read);
	size_t WASABICALL UltravoxReader_BytesBuffered();
	int WASABICALL UltravoxReader_IsClosed();
	int WASABICALL UltravoxReader_ReadPacket(void *buffer, size_t buffer_length, size_t *bytes_read);

	/* ifc_http_demuxer implementation */
	int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters);

	/* member data */
	ifc_ultravox_playback *playback;
	jnl_http_t http;
	UltravoxHeader ultravox_header; /* note! uvox_length will be updated (decremented) as data is read */
	unsigned int classtype;
	UltravoxState state;
	SHOUTcast2Metadata ultravox_3902;	
};
