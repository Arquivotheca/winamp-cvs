/*
 *  ICYReader.cpp
 *  Reader class to demux metadata from shoutcast streams
 *  implements ifc_shoutcast_reader
 *
 *  Created by Ben Allison on Feb 1, 2008
 *  Copyright 2008 Nullsoft, Inc. All rights reserved.
 *
 */
#include "ICYReader.h"
#include <stdlib.h>
#include "nswasabi/ReferenceCounted.h"

ICYReader::ICYReader(jnl_http_t http, ifc_player *player) : http(http), player(player)
{
	stage = STAGE_CONNECTED;
	metadataPosition=0;
	metadata_interval=0;
	bitrate=0;
	if (player)
		player->Retain();
	stream_metadata = new ReferenceCounted<ICYMetadata>;
	stream_metadata->Initialize(http);
	player->SetMetadata(stream_metadata);
}

ICYReader::~ICYReader()
{
	if (player)
		player->Release();

	if (stream_metadata)
		stream_metadata->Release();
}

int ICYReader::Setup()
{
	/* let's get some useful information out of the headers */

	// check metadata interval
	const char *metadata_interval_str = jnl_http_getheader(http, "icy-metaint");
#ifdef _WIN32
	if (metadata_interval_str)
		metadata_interval = _strtoui64(metadata_interval_str, 0, 10);
#else
	if (metadata_interval_str)
		metadata_interval = strtoull(metadata_interval_str, 0, 10);
#endif

	unsigned int bitrate;
	const char *bitrate_str = jnl_http_getheader(http, "icy-br");
	if (bitrate_str)
		bitrate = strtoul(bitrate_str, 0, 10);

	if (bitrate == 0)
		bitrate = 128;
	else if (bitrate < 32) // a reasonable minimum
		bitrate = 32;
	else if (bitrate > 1024) // set a reasonable upper-bound
		bitrate = 1024;

	stage=STAGE_READING;
	return 0;
}

size_t ICYReader::ICYReader_BytesBuffered()
{
	Run();
	return jnl_http_bytes_available(http);
}

int ICYReader::ICYReader_Read(void *data, size_t length, size_t *readLength)
{
	/* TODO: loop until we get the whole read */
	*readLength=0;
	do
	{
		Run();
	} while (stage < STAGE_READING);
	if (stage == STAGE_READING)
	{
		if (metadata_interval && metadata_interval == metadataPosition)
		{
			// SHOUTcast metadata
			uint8_t metadata_size_16;
			if (jnl_http_peek_bytes(http, (char *)&metadata_size_16, 1) == 1)
			{
				uint16_t metadataSize = metadata_size_16 * 16;
				if (jnl_http_bytes_available(http) >= (size_t)(1U + metadataSize))
				{
					char metadata[4081];
					jnl_http_get_bytes(http, (char *)&metadata_size_16, 1); // dummy read to advance the stream
					if (metadataSize)
					{
						metadataSize=jnl_http_get_bytes(http, metadata, metadataSize);
						metadata[metadataSize]=0;						
						ICYMetadata *new_metadata = new ReferenceCounted<ICYMetadata>;
						if (!new_metadata)
							return NErr_OutOfMemory;
						
						new_metadata->Initialize(stream_metadata, metadata);
						stream_metadata->Release();
						stream_metadata = new_metadata;
						player->SetMetadata(stream_metadata);		
					}
					metadataPosition = 0; // reset interval
				}
				else
					return NErr_Success; // try again after we have some more data available
			}
		}

		// normal data read
		size_t avail = jnl_http_bytes_available(http);
		if (avail > length)
			avail = length;

		// only read until our next metadata event
		if (metadata_interval && avail > (metadata_interval - metadataPosition))
			avail = (size_t)(metadata_interval - metadataPosition);

		size_t downloadSize = jnl_http_get_bytes(http, (char *)data, avail);
		if (downloadSize)
		{
			*readLength = downloadSize;
			metadataPosition += downloadSize;
		}
		return NErr_Success;
	}
	return NErr_Error;
}

int WASABICALL ICYReader::ICYReader_Peek(void *buffer, size_t length, size_t *readLength)
{
	uint8_t *buffer8 = (uint8_t *)buffer;
	uint64_t local_metadataPosition = metadataPosition;

	*readLength=0;
	do
	{
		Run();
	} while (stage < STAGE_READING);
	if (stage == STAGE_READING)
	{
		length = jnl_http_peek_bytes(http, buffer, length);

		/* erase any metadata chunks from the buffer (we'll read them later during a Read() call */
		if (metadata_interval)
		{
			size_t buffer_position=0;
			for (;;)
			{
				size_t metadata_position = metadata_interval - local_metadataPosition;
				if (buffer_position + metadata_position < length)
				{
					uint16_t metadata_size = buffer8[metadata_position] * 16 + 1;
					if (buffer_position + metadata_size + metadata_position > length)
					{ 
						// if metadata is only at the end of the block, just truncate the buffer and return
						*readLength = buffer_position+metadata_position;
						return NErr_Success;
					}

					buffer8 += metadata_position;
					buffer_position += metadata_position;
					memmove(buffer8, buffer8 + metadata_size, length - buffer_position - metadata_size);					
					length-=metadata_size;
					local_metadataPosition = 0;
				}
				else
				{
					*readLength = length;
					return NErr_Success;
				}
			}
		}
		*readLength = length;
		return NErr_Success;
	}
	return NErr_Error;
}

int ICYReader::ICYReader_IsClosed()
{
	if (stage==STAGE_FAIL || stage == STAGE_CLOSED)
		return NErr_Closed;

	return NErr_Success;
}

const char *ICYReader::GetMimeType()
{
	return jnl_http_getheader(http, "Content-Type");
}

ICYReader::ICYStage ICYReader::GetState() const
{
	return stage;
}

void ICYReader::Run()
{
	int run_status;

	switch(stage)
	{
	case STAGE_CONNECTED:
		run_status = jnl_http_run(http);
		if (run_status == 1)
		{
			stage=STAGE_CLOSED;
			return ;
		}
		else if (run_status == -1)
		{
			stage = STAGE_FAIL;
			return;
		}


		Setup();
		break;
	case STAGE_READING:
		//if (jnl_http_bytes_available(http) < (512*1024)) // TODO: is this the best way to do it?
	{
		run_status = jnl_http_run(http);
		if (run_status == 1)
		{
			stage=STAGE_CLOSED;
			return ;
		}
		else if (run_status == -1)
		{
			stage = STAGE_FAIL;
			return;
		}

	}
	break;
	}
}

int ICYReader::ICYReader_Run()
{
	Run();
	switch(stage)
	{
	default:
		return NErr_Success;
	case STAGE_CLOSED:
		return NErr_Closed;
	case STAGE_FAIL:
		return NErr_Error;
	}
}
