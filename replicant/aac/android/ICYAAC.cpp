#include "api.h"
#include "ICYAAC.h"
#include "nsaac/ADTSHeader.h"
#include "nx/nxsleep.h"
#include <stdio.h> // TODO: cut


static const size_t synchronize_bytes = 16384; // how many bytes to read before giving up on stream synchronization

ICYAAC::ICYAAC()
{
	http=0;
	decoder=0;
}

ICYAAC::~ICYAAC()
{
	delete decoder;
}

int ICYAAC::Initialize(jnl_http_t http)
{
	this->http=http;
	return NErr_Success;
}
/* helper function for making sure we read exactly as much as we want to */
int ICYAAC::ICYRead(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested)
{
	size_t bytes_in_buffer=0;
	while (bytes_in_buffer < bytes_requested)
	{
		size_t this_read = bytes_requested-bytes_in_buffer;
		int ret = reader->Read(buffer+bytes_in_buffer, this_read, &this_read); 
		if (ret != NErr_Success)
			return ret;

		bytes_in_buffer += this_read;
	}
	return NErr_Success;
}

int ICYAAC::ICYPeek(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested)
{
	// if we get less than requested, it's probably because there's not enough data in the socket
	// so we'll keep trying until we get it right.  probably not ideal but it should suffice

	for (;;)
	{
		size_t bytes_in_buffer;
		int ret = reader->Peek(buffer, bytes_requested, &bytes_in_buffer);
		if (ret)
			return ret;
		if (bytes_in_buffer == bytes_requested)
			return NErr_Success;
		if (reader->IsClosed())
			return NErr_Closed;
	} 
}

int ICYAAC::ICYSync(ifc_icy_reader *reader)
{
	int ret;
	size_t bytes_in_buffer;
	size_t position=0;
	uint8_t buffer[synchronize_bytes];
	ret = reader->Peek(buffer, synchronize_bytes, &bytes_in_buffer); 
	if (ret)
		return ret;

	for (;position<bytes_in_buffer;position++)
	{
		// find POTENTIAL sync
		if (buffer[position] == 0xFF && bytes_in_buffer - position >= 7)
		{
			ADTSHeader header;
			if (nsaac_adts_parse(&header, &buffer[position]) == NErr_Success)
			{
				int frame_length = header.frame_length;
				if (frame_length && bytes_in_buffer - position - frame_length >= 7)
				{
					ADTSHeader header2;
					if (nsaac_adts_parse(&header2, &buffer[position+frame_length]) == NErr_Success)
					{
						// verify that parameters match
						if (nsaac_adts_match(&header, &header2) != NErr_True)
							return NErr_Changed;

						// do a dummy read to advance the stream
						return reader->Read(buffer, position, &position);
					}
				}
			}
		}
	}

	return NErr_False;	
}

int ICYAAC::ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader)
{	
	int ret;
	bool opened=false, paused=false;
	ifc_audioout *out=0;	

	ret = AACDecoder_Create(&decoder, TYPE_ADTS_AAC);
	if (ret != NErr_Success)
		return ret;

	player->SetBufferStatus(0);

resync:
	/* buffer enough to be able to synchronize */
	while(reader->BytesBuffered() < 16384)
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_KILL_MASK);
		if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	/* Synchronize to the stream */
	ret = ICYSync(reader);
	if (ret)
		return ret;

	player->OnReady();

	/* wait for playback to start */
	for (;;)
	{
		ret = http_parent->Wake(ifc_http::WAKE_PLAY|ifc_http::WAKE_STOP); 
		if (ret == ifc_http::WAKE_PLAY)
		{
			break;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	/* buffer more data */
	while(reader->BytesBuffered() < 1024*64)
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_STOP);
		if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	player->SetBufferStatus(100);

	for (;;)
	{
		/* check for kill switch */
		ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
		if (ret == ifc_http::WAKE_PAUSE)
		{
			if (opened)
				out->Pause(1);
			paused=true;
			continue;
		}
		else if (ret== ifc_http::WAKE_UNPAUSE)
		{
			if (opened)
				out->Pause(0);
			paused=false;
			continue;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			player->OnStopped();
			return NErr_Success;			
		}

		if (paused)
		{
			continue;
		}

		/* Read the ADTS header and make sure we didn't lose sync */
		ret = ICYRead(reader, buffer, 7);
		if (ret)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return ret;
		}

		ADTSHeader header;
		if (nsaac_adts_parse(&header, buffer) != NErr_Success)
		{
			goto resync;
#if 0
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return NErr_LostSynchronization;
#endif
		}

		// TODO: verify that header matches the initial one
		size_t header_size = nsaac_adts_get_header_size(&header);
		/* Read the ADTS frame */
		ret = ICYRead(reader, buffer+header_size, header.frame_length-header_size);
		if (ret)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return ret;
		}

		size_t samples_decoded;

		if (decoder->Decode(buffer, header.frame_length, output_buffer, &samples_decoded) == NErr_Success)		
		{
			if (!opened)
			{
				ifc_audioout::Parameters parameters={sizeof(parameters), };
				decoder->FillParameters(&parameters.audio);

				ret = http_parent->AudioOpen(&parameters, &out);
				if (ret != NErr_Success)
					return ret;

				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				opened=true;
			}

			size_t bytes_decoded = samples_decoded * sizeof(int16_t);
			/* write to output */
			const uint8_t *decoded_data = (const uint8_t *)output_buffer;
			size_t buffer_position=0;
			while (bytes_decoded)
			{
				size_t to_write = out->CanWrite();
				if (to_write)
				{
					if (bytes_decoded < to_write)
						to_write = bytes_decoded;

					ret = out->Output(&decoded_data[buffer_position], to_write);
					if (ret != NErr_Success)
					{
						out->Release();
						return ret;
					}
					bytes_decoded -= to_write;
					buffer_position += to_write;
				}
				else
				{
					reader->Run(); /* might as well run while we wait */
					int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
					if (ret == ifc_http::WAKE_STOP)
					{
						if (opened)
						{
							out->Stop();
							out->Release();
						}
						player->OnStopped();
						return NErr_Success;
					}

				}
			}
		}

	}

	return NErr_Success;
}