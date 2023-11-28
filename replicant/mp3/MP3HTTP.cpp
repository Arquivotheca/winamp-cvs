#include "api.h"
#include "main.h"
#include "MP3HTTP.h"
#include "nx/nx.h"
#include "jnetlib/jnetlib.h"
#include "foundation/error.h"
#include "replaygain/ifc_replaygain_settings.h"
#include "service/ifc_servicefactory.h"
#include <assert.h>
#include "nswasabi/ReferenceCounted.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <new>
MP3HTTP::MP3HTTP() 
{
	mpeg=0;	
	gio_jnetlib=0;
	http=0;
}

MP3HTTP::~MP3HTTP()
{
	delete mpeg;
	if (gio_jnetlib)
		gio_jnetlib->Release();
}

int MP3HTTP::Initialize(jnl_http_t _http)
{
	http=_http;
	return NErr_Success;
}

int MP3HTTP::HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)
{
	uint64_t samples_written=0;
	double start_position = 0;
	double samples_per_second=0;
	int ret;
	unsigned int sample_rate;
	ifc_audioout *out=0;
	bool paused=false;
	gio_jnetlib = new (std::nothrow) ReferenceCounted<GioJNetLib>;
	if (!gio_jnetlib)
	{
		player->OnError(NErr_OutOfMemory);
		return NErr_OutOfMemory;
	}

	mpeg = new CMpgaDecoder;
	if (!mpeg)
	{
		player->OnError(NErr_OutOfMemory);
		return NErr_OutOfMemory;
	}

	ret = gio_jnetlib->Open(http, http_parent);
	if (ret == NErr_Interrupted)
	{
		player->OnStopped();
		return NErr_Success;
	}
	else if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	if (http_parent->Seekable() == NErr_True && gio_jnetlib->HasContentLength())
		player->SetSeekable(1);
	else 
		player->SetSeekable(0);

	mpeg->Connect(gio_jnetlib);

	float decode_buffer[576*2*2];
	double total_bitrate=0;
	double last_length=0;
	double current_bitrate = 128000; // TODO
	unsigned long total_frames=0;


	last_length = gio_jnetlib->GetLengthSeconds(current_bitrate);
	if (last_length)
		player->SetLength(last_length);

	bool opened=false;

	player->SetMetadata(gio_jnetlib);
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
prebuffer:
	/* pre-buffer */
	ret = gio_jnetlib->Internal_Buffer(256000, player);
	if (ret == NErr_Interrupted)
	{
		if (out)
		{
			out->Stop();
			out->Release();
		}
		player->OnStopped();
		return NErr_Success;
	}
	else if (ret && ret != NErr_Closed) /* maybe the whole track is less than our pre-buffer size */
	{
		player->OnError(ret);
		return ret;
	}

	for (;;)
	{
		/* check for kill switch */
		int ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
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


		Agave_Seek *seek = http_parent->GetSeek();
		if (seek)
		{
			switch(seek->position_type)
			{
			case AGAVE_PLAYPOSITION_SECONDS:
				{
					uint64_t byte_position;

					if (gio_jnetlib->GetSeekPosition(seek->position.seconds, current_bitrate, &byte_position) == NErr_Success)
					{
						if (out)
							out->Flush(seek->position.seconds);
						gio_jnetlib->Reset();
						ret = http_parent->Seek(byte_position);
						if (ret)
						{
							if (out)
							{
								out->Done();
								out->Release();
							}
							player->OnError(ret);
							return ret;
						}

						samples_written = 0;
						start_position = seek->position.seconds;

						player->OnSeekComplete(NErr_Success, seek->position.seconds);
						mpeg->Reset();
						http_parent->FreeSeek(seek);
						goto prebuffer;
					}
				}
				break;
			}

			http_parent->FreeSeek(seek);
		}

		if (paused) /* a seek will break us out of Wake, so we need to account for that */
			continue;

		size_t decoded=0;
		SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &decoded);

		if (SSC_SUCCESS(mpeg_ret))
		{
			const CMp3StreamInfo *info = mpeg->GetStreamInfo();

			total_bitrate += info->GetBitrate();
			total_frames++;
			current_bitrate = total_bitrate/(double)total_frames;
			// if we need to, open the audio output
			if (!out)
			{
				ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
				parameters.audio.sample_rate = info->GetSFreq();
				parameters.audio.format_type = nsaudio::format_type_float;
				parameters.audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
				parameters.audio.bytes_per_sample = 4;
				parameters.audio.bits_per_sample = 32;
				parameters.audio.number_of_channels = info->GetChannels();

				/* read gapless info */
				size_t pregap, postgap;
				if (GetGaps(gio_jnetlib, &pregap, &postgap) == NErr_Success)
				{
#ifdef __ANDROID__
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MP3HTTP] pre-gap = %u post-gap = %u", pregap, postgap);
#endif
					parameters.frames_trim_start = pregap;
					parameters.frames_trim_end = postgap;
				}

				/* read replaygain info */
				ifc_replaygain_settings *replaygain_settings;
				if (secondary_parameters && secondary_parameters->QueryInterface(&replaygain_settings) == NErr_Success)
				{
					if (replaygain_settings->GetGain(gio_jnetlib, &parameters.gain, 0) == NErr_Success)
					{
#ifdef __ANDROID__
						__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MP3HTTP] using replaygain adjustment of %f", parameters.gain);
#endif
						parameters.extended_fields_flags |= ifc_audioout::EXTENDED_FLAG_REPLAYGAIN;
					}
					replaygain_settings->Release();
				}

				sample_rate = info->GetSFreq();
				samples_per_second = sample_rate * info->GetChannels();

				int ret = http_parent->AudioOpen(&parameters, &out);
				if (ret != NErr_Success)
				{
					player->OnError(ret);
					return ret;
				}
				
				opened=true;
				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				player->SetPosition(start_position);				
			}

			if (decoded)
			{
				double this_length = gio_jnetlib->GetLengthSeconds(current_bitrate);
				if (this_length != last_length)
					player->SetLength(this_length);
				last_length=this_length;

				samples_written += decoded / sizeof(float);
				const uint8_t *decode8 = (const uint8_t *)decode_buffer;
				size_t buffer_position=0;
				while (decoded)
				{
					size_t to_write = out->CanWrite();
					if (to_write)
					{
						if (decoded < to_write)
							to_write = decoded;

						ret = out->Output(&decode8[buffer_position], to_write);
						if (ret != NErr_Success)
						{
							out->Release();
							player->OnError(ret);
							return ret;							
						}

						decoded -= to_write;
						buffer_position += to_write;
					}
					else
					{
						gio_jnetlib->Run(); /* might as well run while we wait */
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
		else switch(mpeg_ret)
		{
		case SSC_W_MPGA_SYNCNEEDDATA:
			gio_jnetlib->Run(); /* might as well run while we wait */
			NXSleep(10);
			break;
		case SSC_W_MPGA_SYNCEOF:
			if (out)
			{
				out->Done();
				out->Release();
			}

			player->OnEndOfFile();
			return NErr_EndOfFile;

		case SSC_E_MPGA_WRONGLAYER:
			if (out)
			{
				out->Done();
				out->Release();
				player->OnError(NErr_Error); // TODO: find better error code
				return NErr_Error; 
			}
			else
				mpeg->m_Mbs.Seek(1);

			break;
		}
	}

	return NErr_Success;
}

