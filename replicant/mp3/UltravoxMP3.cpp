#include "UltravoxMP3.h"
#include "nx/nxsleep.h"
#include "gioultravox.h"

UltravoxMP3::UltravoxMP3(jnl_http_t http) : http(http)
{
	mpeg=0;
}

UltravoxMP3::~UltravoxMP3()
{
	delete mpeg;
}

int UltravoxMP3::UltravoxPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_ultravox_reader *reader)
{
	float decode_buffer[576*2*2];
	bool paused=false, opened=false;
	ifc_audioout *out=0;	
	int sample_rate=0;
	GioUltravox gio_ultravox;

	int ret;

	/* initialize stuff */
	mpeg = new CMpgaDecoder;
	if (!mpeg)
		return NErr_OutOfMemory;

	ret = gio_ultravox.Open(reader);
	if (ret != NErr_Success)
		return ret;

	mpeg->Connect(&gio_ultravox);

	/* pre-buffer */
	while(reader->BytesBuffered() < 256000) 
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_STOP);
		if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	for (;;)
	{
		/* check for kill switch */
		ret = http_parent->Wake(ifc_http::WAKE_ALL_MASK);
		if (ret == ifc_http::WAKE_KILL)
			return NErr_Success;
		else if (ret == ifc_http::WAKE_PAUSE)
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
				out->Stop();
			return NErr_Success;			
		}

		size_t decoded=0;
		SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &decoded);

		if (SSC_SUCCESS(mpeg_ret))
		{
			// if we need to, open the audio output
			if (!opened)
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

				int ret = http_parent->AudioOpen(&parameters, &out);
				if (ret != NErr_Success)
					return ret;

				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				opened=true;
			}

			if (decoded)
			{
				/* write to output */
				const uint8_t *decoded_data = (const uint8_t *)decode_buffer;
				size_t buffer_position=0;
				while (decoded)
				{
					size_t to_write = out->CanWrite();
					if (to_write)
					{
						if (decoded < to_write)
							to_write = decoded;

						out->Output(&decoded_data[buffer_position], to_write);
						decoded -= to_write;
						buffer_position += to_write;
					}
					else
					{
						NXSleep(55);
					}
				}
			}
		} 
		else
			switch(mpeg_ret)
		{
			case SSC_W_MPGA_SYNCNEEDDATA:
				NXSleep(55);
				break;
			case SSC_W_MPGA_SYNCEOF:
				if (opened)
					out->Done();
				return NErr_EndOfFile;

			case SSC_E_MPGA_WRONGLAYER:
				if (opened)
				{
					out->Done();
					return NErr_Error; // TODO: find better error code
				}
				else
					mpeg->m_Mbs.Seek(1);

				break;
		}
	}

	return NErr_Success;
}