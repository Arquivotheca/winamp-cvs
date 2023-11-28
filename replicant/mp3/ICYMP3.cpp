#include "ICYMP3.h"
#include "nx/nxsleep.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

ICYMP3::ICYMP3()
{
	http=0;
	mpeg=0;
}

ICYMP3::~ICYMP3()
{
	delete mpeg;
}

int ICYMP3::Initialize(jnl_http_t http)
{
	this->http = http;
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

int ICYMP3::Buffer(size_t bytes_to_buffer, ifc_http *http_parent, ifc_icy_reader *reader, ifc_player *player)
{
	size_t bytes_buffered;
	int last_percent=0;

	if (reader->IsClosed() != NErr_Success)
		return 0;

	player->SetBufferStatus(0);

	while (bytes_buffered=reader->BytesBuffered(), bytes_buffered < bytes_to_buffer)
	{
		int percentage = bytes_buffered / bytes_to_buffer;
		SendBufferCallback(bytes_buffered, bytes_to_buffer, last_percent, player);
		if (reader->IsClosed() != NErr_Success)
			break;

		int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
		if (ret)
			return ret;
	}

	player->SetBufferStatus(100);
	return 0;
}

int ICYMP3::ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader)
{
	bool paused=false;
	float decode_buffer[576*2*2];
	bool opened=false;
	double start_position=0;
	uint64_t samples_written=0;
	ifc_audioout *out=0;
	double samples_per_second=0;
	int sample_rate=0;
	GioICY gio_icy;

	int ret;

	/* initialize stuff */
	mpeg = new CMpgaDecoder;
	if (!mpeg)
		return NErr_OutOfMemory;

	ret = gio_icy.Open(reader, http_parent);
	if (ret != NErr_Success)
		return ret;
	mpeg->Connect(&gio_icy);
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

	/* TODO: pre-buffer enough to get a frame or two and have a more reliable bitrate, so we know how much exactly to buffer */
	/* buffer the socket */
	switch(Buffer(131072, http_parent, reader, player))
	{
	case ifc_http::WAKE_STOP:
		player->OnStopped();
		return NErr_Success;
	}


	for (;;)
	{
		/* rebuffer if we get too low */
		if (reader->BytesBuffered() < 2048)
		{
			switch(Buffer(131072, http_parent, reader, player))
			{
			case ifc_http::WAKE_STOP:
				if (opened)
				{
					out->Stop();
					out->Release();
				}
				player->OnStopped();
				return NErr_Success;
			}
		}

		reader->Run(); /* might as well run while we wait */
		/* check for kill switch */
		ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
		if (ret == ifc_http::WAKE_PAUSE)
		{
			if (out)
				out->Pause(1);
			paused=true;
			continue;
		}
		else if (ret== ifc_http::WAKE_UNPAUSE)
		{
			if (out)
				out->Pause(0);
			paused=false;
			continue;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			if (out)
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
		size_t decoded=0;
		SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &decoded);
		if (SSC_SUCCESS(mpeg_ret))
		{
			if (!out)
			{
				const CMp3StreamInfo *info = mpeg->GetStreamInfo();
				ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
				parameters.audio.sample_rate = info->GetSFreq();
				parameters.audio.format_type = nsaudio::format_type_float;
				parameters.audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
				parameters.audio.bytes_per_sample = 4;
				parameters.audio.bits_per_sample = 32;
				parameters.audio.number_of_channels = info->GetChannels();

				sample_rate = info->GetSFreq();
				samples_per_second = sample_rate * info->GetChannels();

				int ret = http_parent->AudioOpen(&parameters, &out);
				if (ret != NErr_Success)
				{
					return ret;
				}

				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				opened=true;
			}

			if (decoded)
			{
				/* write to output */
				samples_written += decoded / sizeof(float);
				const uint8_t *decoded_data = (const uint8_t *)decode_buffer;
				size_t buffer_position=0;
				while (decoded)
				{
					size_t to_write = out->CanWrite();
					if (to_write)
					{
						if (decoded < to_write)
							to_write = decoded;

						ret = out->Output(&decoded_data[buffer_position], to_write);
						if (ret != NErr_Success)
						{
							out->Release();
							return ret;							
						}
						decoded -= to_write;
						buffer_position += to_write;
					}
					else
					{
						reader->Run(); /* might as well run while we wait */
						int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
						if (ret == ifc_http::WAKE_STOP)
						{
							if (out)
							{
								out->Stop();
								out->Release();
							}
							player->OnStopped();
							return NErr_Success;
						}
					}
				}
				player->SetPosition(start_position + (double)samples_written/samples_per_second - out->Latency());
			}
		} 

		else if (reader->IsClosed() == NErr_True)
		{
			if (out)
			{
				out->Done();
				out->Release();
			}
			return NErr_ConnectionFailed;
		}
		else switch(mpeg_ret)
		{
		case SSC_W_MPGA_SYNCEOF:
			if (out)
			{
				out->Done();
				out->Release();
			}
			return NErr_EndOfFile;
		case SSC_E_MPGA_WRONGLAYER:
			mpeg->m_Mbs.Seek(1);
			// fall through
		case SSC_W_MPGA_SYNCSEARCHED:
		case SSC_W_MPGA_SYNCLOST:
		case SSC_W_MPGA_SYNCNEEDDATA:
			reader->Run(); /* might as well run while we wait */
			NXSleep(10);
			break;

		default:
			if (out)
			{
				out->Done();
				out->Release();
				return NErr_Error; // TODO: find better error code
			}
			break;
		}
	}

	return NErr_Success;
}
