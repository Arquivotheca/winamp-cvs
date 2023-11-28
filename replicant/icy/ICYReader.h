/*
 *  ICYReader.h
 *  shoutcast_player
 *
 *  Created by Ben Allison on 2/1/08.
 *  Copyright 2008 Nullsoft, Inc. All rights reserved.
 *
 */
 
#include "ifc_icy_reader.h"
#include "jnetlib/jnetlib.h"
#include "foundation/types.h"
#include "player/ifc_player.h"
#include "ICYMetadata.h"

class ICYReader : public ifc_icy_reader
{
public:
	enum ICYStage
	{
		STAGE_NEW=0,
		STAGE_CONNECTING=1,
		STAGE_CONNECTED=2,
		STAGE_READING=3,
		STAGE_CLOSED=4,
		STAGE_FAIL=100,
	};
public:
	ICYReader(jnl_http_t http, ifc_player *player);
	~ICYReader();
	
	ICYStage GetState() const;
	const char *GetMimeType();
	// TODO: container for stream metadata
	// TODO: default metadata (read from the HTTP headers)
private:
	/* ifc_icy_reader implementation */
	size_t WASABICALL ICYReader_BytesBuffered();
	int WASABICALL ICYReader_Read(void *buffer, size_t length, size_t *readLength);
	int WASABICALL ICYReader_Peek(void *buffer, size_t length, size_t *readLength);
	int WASABICALL ICYReader_IsClosed();
	int WASABICALL ICYReader_Run();
	
private:
	int Setup();
	void Run();
	
private:
	jnl_http_t http;
	uint64_t metadataPosition;
	uint64_t metadata_interval;
	ICYStage stage;
	uint32_t bitrate;
	ifc_player *player;
	ICYMetadata *stream_metadata;
};
