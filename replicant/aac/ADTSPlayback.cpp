#include "ADTSPlayback.h"
#include "nx/nxfile.h"
#include "nx/nxsleep.h"

ADTSPlayback::ADTSPlayback(nx_string_t filename, ifc_player *player) : PlaybackBase(filename, player), mpeg(0)
{
	stopped=0;
	paused=0;
	adts_file=0;

	NXThreadCreate(&playback_thread, ADTSPlayerThreadFunction, this);
}

ADTSPlayback::~ADTSPlayback()
{
	if (adts_file)
		fclose(adts_file);
}

nx_thread_return_t ADTSPlayback::ADTSPlayerThreadFunction(nx_thread_parameter_t param)
{
	ADTSPlayback *playback = (ADTSPlayback *)param;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_PLAYBACK);
	return playback->DecodeLoop();
}

nx_thread_return_t ADTSPlayback::DecodeLoop()
{
	float interleave_buffer[4096];
	uint8_t file_buffer[8192]; /* TODO: add some sort of "GetBuffer" to decoder object */
	
	int ret = Init();
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return 0;
	}

	bool opened=false;


	for (;;)
	{
		/* check for kill switch */
		int ret = Wake();
		if (ret == WAKE_KILL)
			break;
		else if (ret == WAKE_PAUSE)
		{
			if (opened)
			out->Pause(1);
			paused=true;
			continue;
		}
		else if (ret== WAKE_UNPAUSE)
		{
			if (opened)
			out->Pause(0);
			paused=false;
			continue;
		}
		else if (ret == WAKE_STOP)
		{
			if (opened)
			out->Stop();
			player->EndOfFile();
			continue;
		}
		else if (ret != WAKE_PLAY)
			continue;


#if 0 // TODO:
		Agave_Seek *seek = queued_seek.GetItem();
		if (seek)
		{
			switch(seek->position_type)
			{
			case AGAVE_PLAYPOSITION_SECONDS:
				{
					giofile->SeekSeconds(seek->position.seconds, /*TODO!!*/128000);
					out->Flush(seek->position.seconds);
					mpeg->Reset();
				}
				break;
			}
			free(seek);
		}
#endif


		if (paused)
		{
			continue;
		}

		size_t decoded=0;
		SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &decoded);
		switch(mpeg_ret)
		{
		case SSC_OK:
			{
				const CMp3StreamInfo *info = mpeg->GetStreamInfo();

				total_bitrate += info->GetBitrate();
				total_frames++;

				// if we need to, open the audio output
				if (!opened)
				{
					ifc_audioout::Parameters parameters={sizeof(parameters), };
					parameters.sample_rate = info->GetEffectiveSFreq();
					parameters.format_type = nsaudio_type_float;
					parameters.format_flags = FORMAT_FLAG_INTERLEAVED|FORMAT_FLAG_NATIVE_ENDIAN|FORMAT_FLAG_SIGNED;
					parameters.bytes_per_sample = sizeof(float);
					parameters.bits_per_sample = 32;
					parameters.number_of_channels = info->GetEffectiveChannels();

					sample_rate = info->GetEffectiveSFreq();

					int ret = output_service->AudioOpen(&parameters, player, &out);
					if (ret != NErr_Success)
					{
						player->OnError(ret);
						break;
					}

					opened=true;
					if (paused)
						out->Pause(1);
				}

				if (decoded)
				{
					double this_length = giofile->GetLengthSeconds(total_bitrate/(double)total_frames);
					if (this_length != last_length)
						player->SetLength(this_length);
					last_length=this_length;

					size_t buffer_position=0;
					while (decoded)
					{
						size_t to_write = out->CanWrite();
						if (to_write)
						{
							if (decoded < to_write)
								to_write = decoded;

							ret = out->Output(&decode_buffer[buffer_position], to_write);
							if (ret != NErr_Success)
							{
								player->OnError(ret);
								out->Done();
								out->Release();
								break;
							}
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
			break;
		case SSC_W_MPGA_SYNCNEEDDATA:
			break;
		case SSC_W_MPGA_SYNCEOF:
			// TODO:
			if (opened)
			{
				out->Done();
				player->EndOfFile();

				// turn off the play flag (also adjust old wake flags so we don't trigger a WAKE_STOP)
				NXConditionLock(&wake);
				wake_flags &= ~WAKE_PLAY;
				last_wake_flags &= ~WAKE_PLAY;
				NXConditionUnlock(&wake);
			}
			else
				;
			break;

			break;
		case SSC_E_MPGA_WRONGLAYER:
			if (opened)
			{
				out->Done();
				player->OnError(NErr_Error); // TODO: find better error code

				// turn off the play flag (also adjust old wake flags so we don't trigger a WAKE_STOP)
				NXConditionLock(&wake);
				wake_flags &= ~WAKE_PLAY;
				NXConditionUnlock(&wake);

			}
			else
				mpeg->m_Mbs.Seek(1);

			break;
		}
	}
	giofile->Close();
	return 0;
}

int ADTSPlayback::Init()
{
	adts_file = NXFile_fopen(filename, nx_file_FILE_read_binary);
	if (!adts_file)
		return NErr_FileNotFound;

	int ret = nsaac_decoder_create(&decoder, 2);
	if (ret != NErr_Success)
		return ret;
	
	return NErr_Success;
}
