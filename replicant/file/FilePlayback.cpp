#include "api.h"
#include "FilePlayback.h"
#include "svc_fileplayback.h"
#include "nswasabi/ReferenceCounted.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

FilePlaybackService::FilePlaybackService()
{

}

int FilePlaybackService::PlaybackService_CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object)
{
	if (NXPathIsURL(filename) == NErr_True)
		return NErr_False;

	if (pass == 0)
		return NErr_TryAgain;

	FilePlayback *file_playback = new (std::nothrow) ReferenceCounted<FilePlayback>();
	if (!file_playback)
		return NErr_OutOfMemory;

	int ret = file_playback->Initialize(filename, player);
	if (ret != NErr_Success)
	{
		delete file_playback;
		return ret;
	}

	*out_playback_object = file_playback;
	
	return NErr_Success;
}

/* -------------------------------------------------------- */

FilePlayback::FilePlayback()
{
	file=0;
	out=0;
	implementation=0;
	paused=false;
	last_position=0;
	output_pointers=0;
	exact_length=NErr_False;
	exact_bitrate=NErr_False;
	memset(&parameters, 0, sizeof(parameters));
	implementation_metadata=0;
	metadata=0;
}

FilePlayback::~FilePlayback()
{
	if (file)
		NXFileRelease(file);

	if (implementation)
		implementation->Release();

	if (implementation_metadata)
		implementation_metadata->Release();

	if (metadata)
		metadata->Release();
	metadata=0;
}

ns_error_t FilePlayback::Initialize(nx_uri_t filename, ifc_player *player)
{
	int ret = PlaybackBase::Initialize(filename, player);
	if (ret != NErr_Success)
		return ret;

	ifc_playback::Retain(); /* the thread needs to hold a reference to this object so that it doesn't disappear out from under us */
	NXThreadCreate(&playback_thread, FilePlayerThreadFunction, this);
	return NErr_Success;
}

nx_thread_return_t FilePlayback::FilePlayerThreadFunction(nx_thread_parameter_t param)
{
	FilePlayback *playback = (FilePlayback *)param;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_PLAYBACK);
	nx_thread_return_t ret = playback->DecodeLoop();
	playback->ifc_playback::Release(); /* give up the reference that was acquired before spawning the thread */
	return ret;
}

static ns_error_t GetFilePlayback(ifc_fileplayback **out_fileplayback, nx_uri_t filename, nx_file_t file, ifc_fileplayback_parent *parent, ifc_metadata *parent_metadata)
{
	GUID fileplayback_guid = svc_fileplayback::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(fileplayback_guid, n++))
	{
		svc_fileplayback *l = (svc_fileplayback*)sf->GetInterface();
		if (l)
		{
			ifc_fileplayback *playback=0;
			int ret = l->CreatePlayback(&playback, filename, file, parent_metadata, parent);
			l->Release();

			if (ret == NErr_Success && playback)
			{
				*out_fileplayback = playback;
				return NErr_Success;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

nx_thread_return_t FilePlayback::DecodeLoop()
{
	player->OnLoaded(filename);

	if (REPLICANT_API_FILELOCK)
		REPLICANT_API_FILELOCK->WaitForReadInterruptable(filename, this);

	ns_error_t ret = NXFileOpenFile(&file, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		goto cleanup;
	}

	metadata = new (std::nothrow) ReferenceCounted<FileMetadataRead>;
	if (!metadata)
	{
		player->OnError(NErr_OutOfMemory);
		goto cleanup;
	}

	nx_file_stat_s file_stat;
	if (NXFileStat(file, &file_stat) == NErr_Success)	
		metadata->SetFileInformation(filename, &file_stat);	
	else
		metadata->SetFileInformation(filename, 0);

	ret = metadata->FindMetadata(file);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		goto cleanup;
	}

	ret = GetFilePlayback(&implementation, filename, file, this, metadata);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		goto cleanup;
	}

	if (implementation->GetMetadata(&implementation_metadata) == NErr_Success)
		player->SetMetadata(implementation_metadata);
	else
		player->SetMetadata(0);

	if (implementation->Seekable() == NErr_True)
		player->SetSeekable(1);
	else
		player->SetSeekable(0);

	double length;
	ret = implementation->GetLength(&length, &exact_length);
	if (ret == NErr_Success)
		player->SetLength(length);

	double bitrate;
	ret = implementation->GetBitrate(&bitrate, &exact_bitrate);
	if (ret == NErr_Success)
		player->SetBitrate(bitrate, 0);

	player->OnReady();

	/* wait for Play (or Stop to abort) */
	for (;;)
	{
		ret = Wake(WAKE_PLAY|WAKE_STOP|WAKE_INTERRUPT);
		if (ret == WAKE_PLAY)
		{
			break;
		}
		else if (ret == WAKE_STOP)
		{
			player->OnStopped();
			goto cleanup;
		}
		else if (ret == WAKE_INTERRUPT)
		{
			ns_error_t ret = Internal_Interrupt();
			if (ret != NErr_Success)
			{
				implementation->Close();
				player->OnError(ret);
				goto cleanup;
			}			
		}
	}

	/* at this point, we know that PLAY is on */
	for (;;)
	{
		int ret = Check(WAKE_STOP|WAKE_PAUSE|WAKE_INTERRUPT);
		if (ret == WAKE_PAUSE)
		{
			if (out)
				out->Pause(1);
			paused=true;
			continue; /* continue in case there's another wake reason */
		}
		else if (ret== WAKE_UNPAUSE)
		{
			if (out)
				out->Pause(0);
			paused=false;
			continue; /* continue in case there's another wake reason */
		}
		else if (ret == WAKE_STOP)
		{
			if (out)
			{
				out->Stop();
				out->Release();
				out=0;
			}
			player->OnStopped();
			goto cleanup;
		}
		else if (ret == WAKE_INTERRUPT)
		{
			ns_error_t ret = Internal_Interrupt();
			if (ret != NErr_Success)
			{
				implementation->Close();
				player->OnError(ret);
				goto cleanup;
			}
			continue;
		}

		Agave_Seek *seek = PlaybackBase::GetSeek();
		if (seek)
		{
			ns_error_t seek_error;
			double new_position;
			ns_error_t ret = implementation->Seek(seek, &seek_error, &new_position);
			if (ret != NErr_Success)
			{
				player->OnError(ret);
				goto cleanup;
			}
			if (out)
			out->Flush(new_position);
			player->OnSeekComplete(seek_error, new_position);
			PlaybackBase::FreeSeek(seek);
		}

		ret = implementation->DecodeStep();
		if (ret == NErr_EndOfFile)
		{
			if (out)
				out->Done();

			PlaybackBase::OnStopPlaying();
			player->OnEndOfFile();

			ret = WaitForClose();
			if (out)
				out->Release();
			out=0;

			if (ret != NErr_True)
				goto cleanup;
		}
		else if (ret == NErr_Stopped)
		{
			// stop was called
			goto cleanup;
		}
		else if (ret == NErr_Aborted)
		{
			// seek
		}
		else if (ret == NErr_Interrupted)
		{
			// interrupted
			ns_error_t ret = Internal_Interrupt();
			if (ret != NErr_Success)
				return (nx_thread_return_t) ret;				
		}
		else if (ret != NErr_Success)
		{
			if (out)
			{
				out->Done();
				out->Release();
				out=0;
			}
			if (ret != NErr_False)
				player->OnError(NErr_Error); // TODO: find better error code
			goto cleanup;
		}
		else
		{
			if (exact_length != NErr_True)
			{
				double length;
				ret = implementation->GetLength(&length, &exact_length);
				if (ret == NErr_Success)
					player->SetLength(length);
			}

			if (exact_bitrate != NErr_True)
			{
				double bitrate;
				ret = implementation->GetBitrate(&bitrate, &exact_length);
				if (ret == NErr_Success)
					player->SetBitrate(bitrate, last_position);
			}
		}
	}

cleanup:
	if (implementation)
		implementation->Close();
	if (file)
		NXFileRelease(file);
	file=0;

	if (REPLICANT_API_FILELOCK)
		REPLICANT_API_FILELOCK->UnlockFile(filename);
	return 0;
}

ns_error_t FilePlayback::WaitForClose()
{
	if (!out)
	{
		player->OnClosed();
		return NErr_False;
	}
	else for (;;)
	{
		int ret = Wait(10, WAKE_PLAY|WAKE_KILL|WAKE_STOP);
		if (ret == WAKE_KILL)
		{
			player->OnClosed();
			return NErr_False;
		}
		else if (ret == WAKE_PLAY)
		{
			return NErr_True;
		}
		else if (ret == WAKE_STOP)
		{
			player->OnStopped();
			return NErr_False;
		}
		else
		{
			if (out->Playing() == NErr_True)
				player->SetPosition(last_position - out->Latency());
			else
			{
				player->SetPosition(last_position);
				player->OnClosed();
				return NErr_False;
			}			
		}
	}
}

ns_error_t FilePlayback::FilePlaybackParent_OpenOutput(const ifc_audioout::Parameters *_parameters)
{
	// if out is already set, it means that there was a change in parameters, so we'll start a new stream
	if (out)
	{
		// check to see that the parameters actually changed
		if (!memcmp(&parameters, _parameters, sizeof(parameters)))
			return NErr_Success;

		out->Done();
		out=0;
	}

	parameters = *_parameters;

	/* read replaygain info */
	ifc_replaygain_settings *replaygain_settings;
	if (secondary_parameters && secondary_parameters->QueryInterface(&replaygain_settings) == NErr_Success)
	{
		if (replaygain_settings->GetGain(implementation_metadata, &parameters.gain, 0) == NErr_Success)
		{
#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FilePlayback] using replaygain adjustment of %f", parameters.gain);
#endif
			parameters.extended_fields_flags |= ifc_audioout::EXTENDED_FLAG_REPLAYGAIN;
		}
		replaygain_settings->Release();
	}

	free(output_pointers);
	output_pointers=0;
	if (parameters.audio.format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED)
	{
		output_pointers = (const uint8_t **)malloc(parameters.audio.number_of_channels * sizeof(const uint8_t *));
		if (!output_pointers)
			return NErr_OutOfMemory;
	}

	ns_error_t ret = output_service->AudioOpen(&parameters, player, secondary_parameters, &out);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	if (paused)
		out->Pause(1);
	else
		out->Pause(0);


	return NErr_Success;
}

int FilePlayback::FilePlaybackParent_OutputNonInterleaved(const void *decode_buffer, size_t decoded, size_t *frames_consumed, double start_position)
{
	int ret;
	size_t frames_written=0;
	const uint8_t **buffer = (const uint8_t **)decode_buffer;
	*frames_consumed=0;
	for (size_t c=0;c<parameters.audio.number_of_channels;c++)
	{
		output_pointers[c] = buffer[c];
	}

	while (decoded)
	{
		size_t to_write = out->CanWrite();
		if (to_write)
		{
			if (decoded < to_write)
				to_write = decoded;

			ret = out->Output(output_pointers, to_write);
			if (ret != NErr_Success)
			{
				out->Release();
				out=0;
				return ret;
			}

			decoded -= to_write;
			size_t frames_to_write = to_write/(parameters.audio.number_of_channels * parameters.audio.bytes_per_sample);
			*frames_consumed += frames_to_write;
			for (size_t c=0;c<parameters.audio.number_of_channels;c++)
			{
				output_pointers[c] += to_write/parameters.audio.number_of_channels;
			}
			frames_written += frames_to_write;
			last_position = start_position + (double)frames_written/parameters.audio.sample_rate;
			player->SetPosition(last_position - out->Latency());
		}
		else
		{
			ns_error_t ret = OutputWait();
			if (ret != NErr_Success)
				return ret;
		}
	}
	return NErr_Success;
}

int FilePlayback::FilePlaybackParent_Output(const void *decode_buffer, size_t decoded, size_t *frames_consumed, double start_position)
{
	int ret;
	size_t frames_written=0;
	const uint8_t *decode8 = (const uint8_t *)decode_buffer;
	size_t buffer_position=0;
	*frames_consumed=0;

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
				out=0;
				return ret;
			}

			decoded -= to_write;
			buffer_position += to_write;
			size_t frames_to_write = to_write/(parameters.audio.number_of_channels * parameters.audio.bytes_per_sample);
			*frames_consumed += frames_to_write;
			frames_written += frames_to_write;
			last_position = start_position + (double)frames_written/parameters.audio.sample_rate;
			player->SetPosition(last_position - out->Latency());
		}
		else
		{
			ns_error_t ret = OutputWait();
			if (ret != NErr_Success)
				return ret;	
		}
	}
	return NErr_Success;
}

ns_error_t FilePlayback::FilePlaybackParent_OnMetadata(ifc_metadata *new_metadata)
{
	player->SetMetadata(new_metadata);
	return NErr_Success;
}

ns_error_t FilePlayback::OutputWait()
{
	if (paused)
	{
		/* if we're paused, we need to sit and wait until we're eiter unpaused or stopped */
		for (;;)
		{
			int ret = Wake(WAKE_STOP|WAKE_PAUSE|WAKE_INTERRUPT);
			if (ret == WAKE_STOP)
			{
				out->Stop();
				out->Release();
				out=0;
				player->OnStopped();
				return NErr_Stopped;
			}
			else if (ret == WAKE_UNPAUSE)
			{
				out->Pause(0);
				paused=false;
				break;
			}
			else if (ret == WAKE_PAUSE)
			{
				out->Pause(1);
				paused=true;
			}
			else if (PlaybackBase::PendingSeek())
			{
				return NErr_Aborted;
			}
			else if (ret == WAKE_INTERRUPT)
			{
				return NErr_Interrupted;
			}
		}
	}
	else
	{
		int ret = Wait(10, WAKE_STOP);
		if (ret == WAKE_STOP)
		{
			out->Stop();
			out->Release();
			out=0;
			player->OnStopped();
			return NErr_Stopped;
		}
	}
	return NErr_Success;
}

ns_error_t FilePlayback::Internal_Interrupt()
{
	Agave_Seek resume_information;
	
	if (implementation_metadata)
		implementation_metadata->Release();
	implementation_metadata=0;

	implementation->Interrupt(&resume_information);

	if (file)
		NXFileRelease(file);
	file=0;

	if (metadata)
		metadata->Release();
	metadata=0;
	
	ns_error_t ret = REPLICANT_API_FILELOCK->UnlockFile(filename);
	if (ret != NErr_Success)
	{
		implementation->Close();
		return ret;
	}
	PlaybackBase::OnInterrupted();
	REPLICANT_API_FILELOCK->WaitForReadInterruptable(filename, this);
	
	ret = NXFileOpenFile(&file, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	metadata = new (std::nothrow) ReferenceCounted<FileMetadataRead>;
	if (!metadata)
		return NErr_OutOfMemory;
	
	nx_file_stat_s file_stat;
	if (NXFileStat(file, &file_stat) == NErr_Success)	
		metadata->SetFileInformation(filename, &file_stat);	
	else
		metadata->SetFileInformation(filename, 0);

	ret = implementation->Resume(&resume_information, file, metadata);
	if (ret != NErr_Success)
		return ret;

	if (implementation->GetMetadata(&implementation_metadata) == NErr_Success)
		player->SetMetadata(implementation_metadata);
	else
		player->SetMetadata(0);

	return NErr_Success;
}