#include "api.h"
#include "UltravoxDemuxer.h"
#include "nx/nx.h"
#include "jnetlib/jnetlib.h"
#include "foundation/error.h"
#include "svc_ultravox_playback.h"
#include "service/ifc_servicefactory.h"
#include <assert.h>

UltravoxDemuxer::UltravoxDemuxer(jnl_http_t http) : http(http)
{
	state = State_Sync;
	classtype = 0;
	playback = 0;
	memset(&ultravox_header, 0, sizeof(ultravox_header));

}

UltravoxDemuxer::~UltravoxDemuxer()
{
	if (http)
		jnl_http_release(http);

}

static NError FindPlayback(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback)
{
	GUID http_demuxer_guid = svc_ultravox_playback::GetServiceType();
	ifc_serviceFactory *sf;

	bool again;
	int pass=0;
	do
	{
		size_t n = 0;
		again=false;
		while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
		{
			svc_ultravox_playback *l = (svc_ultravox_playback *)sf->GetInterface();
			if (l)
			{
				NError err = l->CreatePlayback(http, classtype, playback);
				if (err == NErr_Success)
					return NErr_Success;

				if (err == NErr_TryAgain)
					again=true;
			}
		}
	} while (again);
	return NErr_NotImplemented;
}

int WASABICALL UltravoxDemuxer::HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)
{
	/* headers that might be useful for us:

	ultravox-max-msg
	Ultravox-SID
	Ultravox-Avg-Bitrate
	Ultravox-Max-Bitrate
	Ultravox-Bitrate
	Ultravox-Title
	Ultravox-URL
	*/

	/* first, we need to read enough data out of the stream to determine content-type */
	Internal_SyncToData();

	/* now we need to find a corresponding decoder */
	FindPlayback(http, classtype, &playback);

	/* let the decoder take over.  he'll call into us to get data which allows us to parse the stream */
	int ret = playback->Run(http_parent, player, this);
	if (ret != NErr_Success)
		player->OnError(ret);
	return ret;
}

int UltravoxDemuxer::Internal_Read(uint8_t *buffer, size_t bytes_requested)
{
	for (;;)
	{
		size_t bytes_in_buffer = jnl_http_get_bytes(http, buffer, bytes_requested);
		bytes_requested -= bytes_in_buffer;
		buffer += bytes_in_buffer;

		if (bytes_requested == 0)
			return NErr_Success;
		Internal_Run();
		NXSleep(10); // TODO: check for stop, disconnection, etc.
	} 
}

int UltravoxDemuxer::Internal_Skip(size_t bytes_to_skip)
{
	for (;;)
	{
		size_t bytes_in_buffer = jnl_http_get_bytes(http, 0, bytes_to_skip);
		bytes_to_skip -= bytes_in_buffer;

		if (bytes_to_skip == 0)
			return NErr_Success;
		Internal_Run();
		NXSleep(10); // TODO: check for stop, disconnection, etc.
	}
}

int UltravoxDemuxer::Internal_SyncToData()
{
	int ret;
	for (;;)
	{
		uint8_t uvox_header_buffer[6];
		ret=Internal_Read(uvox_header_buffer, 6);
		if (ret != NErr_Success)
			return ret;

		ret = uvox_header_parse(&ultravox_header, uvox_header_buffer, 6);
		if (ret != NErr_Success)
			return ret;

		if (uvox_is_data(&ultravox_header) == NErr_True)
		{
			classtype = ultravox_header.uvox_classtype;
			state = State_ReadingData;
			return NErr_Success;
		}
		else if (ultravox_header.uvox_classtype == 0x3902) /* SHOUTcast 2.0 metadata */
		{
			ret = Internal_On3902Metadata();
			if (ret != NErr_Success)
				return ret;
		}
		else // TODO: deal with other classes/types (metadata, etc)
		{
			ret = Internal_Skip(ultravox_header.uvox_length+1);
			if (ret != NErr_Success)
				return ret;
		}
		/* TODO: deal with stream closing */
	}
}

int UltravoxDemuxer::Internal_Run()
{
	if (jnl_http_run(http) == -1)
		state = State_Failure;
	return NErr_Success;
}

int UltravoxDemuxer::Internal_On3902Metadata()
{
	int ret = Internal_Buffer(ultravox_header.uvox_length+1);
	if (ret != NErr_Success)
		return ret;

	ultravox_3902.OnMetadata(&ultravox_header, http);
	Internal_Skip(ultravox_header.uvox_length+1);
	return NErr_Success;
}

/* spins until specified number of bytes are available in the socket */
int UltravoxDemuxer::Internal_Buffer(size_t bytes)
{
	while (jnl_http_bytes_available(http) < bytes)
	{
		int ret = Internal_Run();
		if (ret != NErr_Success)
			return ret;
	}
	return NErr_Success;
}

/* ifc_ultravox_reader implementation */
int UltravoxDemuxer::UltravoxReader_Read(void *buffer, size_t buffer_length, size_t *bytes_read)
{
	/* TODO: check for errors, disconnected stream, stop pressed, killswitch, etc */
	uint8_t *buffer8 = (uint8_t *)buffer;
	size_t this_bytes_read = 0;
	while (buffer_length)
	{
		Internal_Run();

		if (state == State_Sync)
			Internal_SyncToData();

		size_t this_read = buffer_length;
		if (this_read > ultravox_header.uvox_length)
			this_read = ultravox_header.uvox_length;

		Internal_Read((uint8_t *)buffer8, this_read);
		ultravox_header.uvox_length -= this_read;
		buffer_length -= this_read;
		this_bytes_read += this_read;
		buffer8 += this_read;
		if (ultravox_header.uvox_length == 0) // end of header? read the trailing byte
		{
			state = State_Sync;
			Internal_Skip(1);
		}
	}
	*bytes_read = this_bytes_read;
	return NErr_Success;
}

int UltravoxDemuxer::UltravoxReader_Peek(void *buffer, size_t buffer_length, size_t *bytes_read)
{
	/* ok this is going to be interesting.  we're going to use jnl_http_peek_bytes() and run the parser with
	"private state" to satisfy the peek request */

	/* TODO: check for errors, disconnected stream, stop pressed, killswitch, etc */
	int ret;
	Internal_Run();
	if (state == State_Sync)
		Internal_SyncToData();

	if (buffer_length <= ultravox_header.uvox_length) // if we have enough bytes to fulfill the request, great!
	{
		ret = Internal_Buffer(ultravox_header.uvox_length);
		if (ret != NErr_Success)
			return ret;

		*bytes_read = jnl_http_peek_bytes(http, buffer, buffer_length);
		return NErr_Success;
	}

	/* if we got here, we need to parse the stream.  */ 
	size_t peek_buffer_size = buffer_length;
	uint8_t *peek_buffer=0;

	for (;;)
	{
		/* double the allocation from last time (or twice the requested buffer length for the first time) */
		size_t new_peek_buffer_size = peek_buffer_size * 2;
		if (new_peek_buffer_size < peek_buffer_size) // check for overflow
			return NErr_OutOfMemory; 
		uint8_t *new_peek_buffer = (uint8_t *)realloc(peek_buffer, new_peek_buffer_size);
		peek_buffer_size = new_peek_buffer_size;
		if (!new_peek_buffer)
		{
			free(peek_buffer);
			return NErr_OutOfMemory;
		}
		peek_buffer = new_peek_buffer;

		uint8_t *output = (uint8_t *)buffer;
		size_t bytes_left = buffer_length;
		size_t position = 0;

		ret = Internal_Buffer(peek_buffer_size);
		if (ret != NErr_Success)
		{
			free(peek_buffer);
			return ret;
		}

		/* read into the peek buffer */
		size_t bytes_in_peek = jnl_http_peek_bytes(http, peek_buffer, peek_buffer_size);

		/* read from the packet currently sitting in the state machine */
		if (state == State_ReadingData)
		{
			memcpy(output, &peek_buffer[position], ultravox_header.uvox_length);
			output += ultravox_header.uvox_length;
			bytes_left -= ultravox_header.uvox_length;
			position += ultravox_header.uvox_length+1;
		}

		while (peek_buffer_size > 6 + position)
		{
			/* read the next packet */
			UltravoxHeader next_header;
			int ret = uvox_header_parse(&next_header, &peek_buffer[position], 6);
			if (ret != NErr_Success)
			{
				free(peek_buffer);
				return ret;
			}

			position+=6;

			if (uvox_is_data(&next_header) == NErr_True)
			{
				size_t to_read = bytes_left;
				if (to_read > next_header.uvox_length)
					to_read = next_header.uvox_length;

				memcpy(output, &peek_buffer[position], to_read);
				output += to_read;
				bytes_left -= to_read;
				position += to_read+1;
				if (bytes_left == 0)
				{
					assert((output - (uint8_t *)buffer) == buffer_length);
					*bytes_read = buffer_length;
					free(peek_buffer);
					return NErr_Success;
				}
			}
			else
			{
				position += ultravox_header.uvox_length+1;
			}
		}
	}
}

size_t UltravoxDemuxer::UltravoxReader_BytesBuffered()
{
	Internal_Run();
	return jnl_http_bytes_available(http);
}

int UltravoxDemuxer::UltravoxReader_IsClosed()
{
	if (state == State_Failure)
		return NErr_Closed;
	return NErr_Success;
}

int UltravoxDemuxer::UltravoxReader_ReadPacket(void *buffer, size_t buffer_length, size_t *bytes_read)
{
	/* TODO: check for errors, disconnected stream, stop pressed, killswitch, etc */

	/* sync to the next data packet if necessary */
	if (state == State_Sync)
		Internal_SyncToData();

	if (buffer_length > ultravox_header.uvox_length)
		return NErr_Insufficient;

	buffer_length = ultravox_header.uvox_length;

	Internal_Read((uint8_t *)buffer, buffer_length);
	*bytes_read = buffer_length;


	state = State_Sync;
	Internal_Skip(1); //  read the trailing byte

	return NErr_Success;
}