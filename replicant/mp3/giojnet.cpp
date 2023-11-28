#include "api.h"
#include "giojnet.h"
#include "nx/nx.h"
#include "foundation/error.h"
#include "metadata/MetadataKeys.h"
#include <new>

GioJNetLib::GioJNetLib() 
{
	state = State_ID3v2;

	http_uri=0;
}

GioJNetLib::~GioJNetLib()
{
	NXStringRelease(http_uri);
}

bool GioJNetLib::HasContentLength() const
{
	return mpeg_length != 0;
}

void GioJNetLib::Run()
{
	if (state == State_Fail || state == State_Closed)
		return;

	int run_status = jnl_http_run(http);
	if (run_status == 1)
	{
		state=State_Closed;
	}
	else if (run_status == -1)
	{
		state = State_Fail;
	}
}

int GioJNetLib::Open(jnl_http_t http, ifc_http *_http_parent)
{
	this->http = http;
	http_parent = _http_parent;
	if (!http_parent || !http)
		return NErr_NullPointer;

	const char *http_url = jnl_http_get_url(http);
	if (http_url)
	{
		NXStringCreateWithUTF8(&http_uri, http_url);
	}

	mpeg_length = jnl_http_content_length(http);

	int ret;
	/* see if there's an ID3v2 tag */
	while (state == State_ID3v2)
	{
		ret = Internal_Buffer(10);
		if (ret != NErr_Success)
		{
			state = State_Fail;
			return ret;
		}

		uint8_t id3v2_header_data[10];
		if (jnl_http_peek_bytes(http, id3v2_header_data, 10) == 10 && NSID3v2_Header_Valid(id3v2_header_data) == NErr_Success)
		{
			jnl_http_get_bytes(http, 0, 10); /* skip past the already-peeked bytes */

			nsid3v2_header_t id3v2_header;
			uint32_t id3v2_tag_size;
			if (NSID3v2_Header_Create(&id3v2_header, id3v2_header_data, 10) != NErr_Success)
				break;

			if (NSID3v2_Header_TagSize(id3v2_header, &id3v2_tag_size) != NErr_Success)
			{
				NSID3v2_Header_Destroy(id3v2_header);
				break;
			}

			void *id3v2_data=0;
			if (id3v2.tag)
			{
				// already read a tag
				mpeg_position+=id3v2_tag_size+10;
				mpeg_length-=id3v2_tag_size+10;
				Internal_Skip(id3v2_tag_size); /* skip past the tag */
			}
			else
			{
				id3v2_data = malloc(id3v2_tag_size);
				if (!id3v2_data)
				{
					// TODO: debatable what we should do here. 
					NSID3v2_Header_Destroy(id3v2_header);
					return NErr_OutOfMemory;
				}

				ret = Internal_Read(id3v2_data, id3v2_tag_size);
				if (ret != NErr_Success)
				{
					free(id3v2_data);
					state = State_Fail;
					return ret;
				}

				if (NSID3v2_Tag_Create(&id3v2.tag, id3v2_header, id3v2_data, id3v2_tag_size) == NErr_Success)
				{
					id3v2.length=id3v2_tag_size+10;
					id3v2.position=mpeg_position;
					mpeg_position+=id3v2_tag_size+10;
					mpeg_length-=id3v2_tag_size+10;
					if (NSID3v2_Header_HasFooter(id3v2_header) == NErr_True)
					{
						id3v2.length+=10;
						mpeg_position+=10;
						mpeg_length-=10;
					}
					id3v2_metadata.Initialize(id3v2.tag);
				}
				else
				{
					free(id3v2_data);
					break;
				}
				free(id3v2_data);
			}
			NSID3v2_Header_Destroy(id3v2_header);
		}
		else
		{
			state = State_Header;
		}
	}

	/* check if there's a LAME header */
	if (state == State_Header)
	{
		/* If we're lucky, we start right on an MPEG frame */
		uint8_t frame_buffer[1448];
		size_t frame_buffer_length;
		if (Internal_PeekFirstFrame(frame_buffer, 1448, &frame_buffer_length) == NErr_Success)
		{
			/* read LAME/Xing header */
			lame.tag = new (std::nothrow) LAMEInfo;
			if (!lame.tag)
				return NErr_OutOfMemory;

			if (lame.tag->Read(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
			{
				size_t vbr_header_length=first_frame_header.FrameSize();
				lame.position = mpeg_position;
				lame.length = vbr_header_length;
				// TODO: validate values and update flags accordingly
				mpeg_position += vbr_header_length; 
				mpeg_length -= vbr_header_length;

				mpeg_duration = lame.tag->GetLengthSeconds();
				mpeg_samples = lame.tag->GetSamples();
				mpeg_frames = lame.tag->GetFrames();

				/* take the bytes out of the stream */
				jnl_http_get_bytes(http, 0, first_frame_header.FrameSize());
			}
			else
			{
				delete lame.tag;
				lame.tag=0;
				/* read VBRI header */
				vbri.tag = new (std::nothrow) CVbriHeader;
				if (!vbri.tag)
					return NErr_OutOfMemory;
				if (vbri.tag->readVbriHeader(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
				{
					size_t vbr_header_length=first_frame_header.FrameSize();
					vbri.position = mpeg_position;
					vbri.length = vbr_header_length;
					mpeg_position += vbr_header_length;
					mpeg_length -= vbr_header_length;

					mpeg_duration = vbri.tag->GetLengthSeconds();
					mpeg_samples = vbri.tag->GetSamples();
					mpeg_frames = vbri.tag->GetFrames();
					if (mpeg_frames)
						mpeg_frames--;

					/* take the bytes out of the stream */
					jnl_http_get_bytes(http, 0, first_frame_header.FrameSize());
				}
				else
				{
					delete vbri.tag;
					vbri.tag=0;
				}
			}

			/* read ofl data */
			if (Internal_PeekFirstFrame(frame_buffer, 1448, &frame_buffer_length) == NErr_Success)
			{
				ofl.tag = new (std::nothrow) OFL;
				if (!ofl.tag)
					return NErr_OutOfMemory;
				if (ofl.tag->Read(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
				{
					ofl.position = mpeg_position;
					ofl.length = first_frame_header.FrameSize();
					mpeg_duration = ofl.tag->GetLengthSeconds();
					mpeg_samples = ofl.tag->GetSamples();
					if (!mpeg_frames)
						mpeg_frames = ofl.tag->GetFrames();
				}
				else
				{
					delete ofl.tag;
					ofl.tag=0;
				}
			}
		}
		state = State_Data;
	}

	return NErr_Success;
}

static void SendBufferCallback(size_t bytes_available, size_t bytes_needed, int &last_percent, ifc_player *player)
{
	if (player)
	{
		if (bytes_available && bytes_available < bytes_needed)
		{
			int percent = 100*bytes_available/bytes_needed;
			if (percent > last_percent && percent != 100)
			{
				player->SetBufferStatus(percent);
				percent=last_percent;
			}
		}
	}
}

int GioJNetLib::Internal_Skip(size_t bytes)
{
	while (bytes)
	{
		Run();
		size_t bytes_read = jnl_http_get_bytes(http, 0, bytes);
		bytes -= bytes_read;
		if (bytes_read == 0 && IsEof())
			return NErr_EndOfFile;
		int ret = http_parent->Sleep(10, ifc_http::WAKE_STOP);
		if (ret)
		{
			state = State_Fail;
			return NErr_Interrupted;
		}
	}
	return NErr_Success;
}

int GioJNetLib::Internal_Buffer(size_t bytes, ifc_player *player)
{
	if (state == State_Closed)
		return NErr_Closed;

	if (state == State_Fail)
		return NErr_ConnectionFailed;

	int last_percent=0;

	if (player)
		player->SetBufferStatus(0);

	unsigned int times_looped=0;
	size_t bytes_available = jnl_http_bytes_available(http);

	SendBufferCallback(bytes_available, bytes, last_percent, player);
	while (bytes_available < bytes)
	{
		size_t bytes_available_pre = jnl_http_bytes_available(http);
		int run_status = jnl_http_run(http);
		if (run_status == 1)
		{
			state=State_Closed;
			if (player)
				player->SetBufferStatus(100);
			return NErr_Closed;
		}
		else if (run_status == -1 || jnl_http_get_status(http) == -1)
		{
			state=State_Fail;
			return NErr_Closed; // fail
		}
		bytes_available = jnl_http_bytes_available(http);

		if (++times_looped == 16 /* sleep every 16 times through the loop (to throttle CPU) */
			|| bytes_available == bytes_available_pre) /* or sleep if we didn't get back any additional data */
		{
			SendBufferCallback(bytes_available, bytes, last_percent, player);

			int ret = http_parent->Sleep(10, ifc_http::WAKE_STOP);
			if (ret)
			{
				state = State_Fail;
				return NErr_Interrupted;
			}
			times_looped=0;
		}
	}

	if (player)
		player->SetBufferStatus(100);

	return NErr_Success;
}

int GioJNetLib::Internal_Read(void *buffer, size_t bytes)
{
	uint8_t *data = (uint8_t *)buffer;

	if (state == State_Closed)
		return NErr_Closed;

	if (state == State_Fail)
		return NErr_ConnectionFailed;

	int last_percent=0;


	size_t bytes_available = jnl_http_bytes_available(http);

	while (bytes)
	{
		int run_status = jnl_http_run(http);
		if (run_status == 1)
		{
			state=State_Closed;
			return NErr_Closed;
		}
		else if (run_status == -1 || jnl_http_get_status(http) == -1)
		{
			state=State_Fail;
			return NErr_Closed; // fail
		}

		size_t bytes_read = jnl_http_get_bytes(http, data, bytes);
		data += bytes_read;
		bytes -= bytes_read;
		if (bytes_read == 0)
		{
			int ret = http_parent->Sleep(10, ifc_http::WAKE_STOP);
			if (ret)
			{
				state = State_Fail;
				return NErr_Interrupted;
			}
		}
	}	

	return NErr_Success;
}

int GioJNetLib::Internal_PeekFirstFrame(uint8_t *frame_buffer, size_t buffer_len, size_t *buffer_written)
{
	char mp3_data[1448];
	int ret;
	ret = Internal_Buffer(4);
	if (ret != NErr_Success)
		return ret;

	if (jnl_http_peek_bytes(http, frame_buffer, 4) == 4) // read header
	{
		first_frame_header.ReadBuffer(frame_buffer);
		if (first_frame_header.IsSync())
		{
			size_t frame_length = first_frame_header.FrameSize();

			ret = Internal_Buffer(frame_length);
			if (ret != NErr_Success)
				return ret;

			if (jnl_http_peek_bytes(http, mp3_data, frame_length) == frame_length)
			{
				memcpy(frame_buffer, mp3_data+first_frame_header.HeaderSize(), frame_length-first_frame_header.HeaderSize());
				*buffer_written = frame_length-first_frame_header.HeaderSize();
				return NErr_Success;
			}
		}
	}
	return NErr_Error;
}

SSC GioJNetLib::Read(void *pBuffer, int cbToRead, int *pcbRead)
{
	int ret;

	ret = Internal_Buffer(cbToRead);
	size_t bytes_read = jnl_http_get_bytes(http, pBuffer, cbToRead);
	*pcbRead = bytes_read;
	if (ret && bytes_read == 0)
		return SSC_W_MPGA_SYNCEOF;
	return SSC_OK;
}

bool GioJNetLib::IsEof() const
{
	if (state == State_Fail)
	{
		return true;
	}
	else if (state == State_Closed && jnl_http_bytes_available(http) == 0)
	{
		return true;
	}
	else
		return false;
}


/* ifc_metadata */
static int ReturnMetadataValue(int index, nx_string_t input_value, nx_string_t *output_value)
{
	if (index == 0)
	{
		if (input_value)
			*output_value = NXStringRetain(input_value);
		else
			*output_value = 0;
		return NErr_Success;
	}
	else
		return NErr_EndOfFile;
}

int GioJNetLib::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (field == MetadataKeys::URI)
	{
		return ReturnMetadataValue(index, http_uri, value);
	}
	else
	{
		return GioReplicant::Metadata_GetField(field, index, value);
	}
}

