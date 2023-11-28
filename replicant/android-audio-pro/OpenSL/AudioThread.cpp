#include "../api.h"
#include "AudioThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "../pcmutils.h"
#include "player/ifc_player.h"
#include "application/features.h"
#include "nx/nxsleep.h"
#include <android/log.h>
#include <assert.h>

/* TODO if we wanted support for multiple simultaneously players, we'd probably need to make a separate AudioThread per player,
and have the service keep track of them */

/* TODO: deal with the following
* paused - if needed to fade-out on pausing, we could do getPosition
* fade-on-seek
*/

const static double config_prebuffer_seconds = AUDIO_PREBUFFER_SECONDS;			// Default: Half of a second

#define LOG_SLRESULT_CASE(x) case x: return #x
static const char *GetSLResultName(SLresult result)
{
	switch(result)
	{
		LOG_SLRESULT_CASE(SL_RESULT_SUCCESS);
		LOG_SLRESULT_CASE(SL_RESULT_PRECONDITIONS_VIOLATED);
		LOG_SLRESULT_CASE(SL_RESULT_PARAMETER_INVALID);
		LOG_SLRESULT_CASE(SL_RESULT_MEMORY_FAILURE);
		LOG_SLRESULT_CASE(SL_RESULT_RESOURCE_ERROR);
		LOG_SLRESULT_CASE(SL_RESULT_RESOURCE_LOST);
		LOG_SLRESULT_CASE(SL_RESULT_IO_ERROR);
		LOG_SLRESULT_CASE(SL_RESULT_BUFFER_INSUFFICIENT);
		LOG_SLRESULT_CASE(SL_RESULT_CONTENT_CORRUPTED);
		LOG_SLRESULT_CASE(SL_RESULT_CONTENT_UNSUPPORTED);
		LOG_SLRESULT_CASE(SL_RESULT_CONTENT_NOT_FOUND);
		LOG_SLRESULT_CASE(SL_RESULT_PERMISSION_DENIED);
		LOG_SLRESULT_CASE(SL_RESULT_FEATURE_UNSUPPORTED);
		LOG_SLRESULT_CASE(SL_RESULT_INTERNAL_ERROR);
		LOG_SLRESULT_CASE(SL_RESULT_UNKNOWN_ERROR);
		LOG_SLRESULT_CASE(SL_RESULT_OPERATION_ABORTED);
		LOG_SLRESULT_CASE(SL_RESULT_CONTROL_LOST);
	}
	return 0;
}

static void LogSL(int kind, SLresult result, const char *method, const char *more_info="")
{
	if (!more_info)
		more_info="";
	
	const char *result_name = GetSLResultName(result);
	if (result_name)
		__android_log_print(kind, "libreplicant", "[OpenSL] result=%s method=%s %s", result_name, method, more_info);
	else
		__android_log_print(kind, "libreplicant", "[OpenSL] result=0x%08x method=%s %s", result, method, more_info);
}

static void LogSLOnError(SLresult result, const char *command, const char *more_info="")
{
	if (result != SL_RESULT_SUCCESS)
		LogSL(ANDROID_LOG_ERROR, result, command, more_info);
}

static void APCRelease(void *_session, void *unused1, double unused2)
{
	AudioSession *session = (AudioSession *)_session;

	if (session)
		session->Release();
}

AudioThread::AudioThread()
{
	// OpenSL objects
	sl.object=0;
	sl.engine=0;

	player.object=0;
	player.play=0;
	player.buffer_queue=0;
	player.volume=0;
	player.config=0;

	output_mix.object=0;

	audio_stop=0;
	audio_thread=0;
	crossfading=false;
	paused=false;
	crossfade_tryagain=false;
	audio_state = IDLE;
	active_session = 0;
	equalizer = 0;
	replicant_player=0;
	killswitch=0;
	settings=0;
}

AudioThread::~AudioThread()
{
	if (settings)
		settings->Release();
}

void AudioThread::DestroyPlayer()
{
	if (player.object)
	{
		(*player.object)->Destroy(player.object);

		player.play=0;
		player.buffer_queue=0;
		player.object=0;
		player.config=0;
		player.volume=0;
	}
}

void AudioThread::TransitionToIdle()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] Transition To Idle");
	if (player.object)
	{
		SLresult result;
		nx_atomic_inc_acquire(&audio_stop);
		result = (*player.play)->SetPlayState(player.play, SL_PLAYSTATE_STOPPED);
		LogSLOnError(result, "SetPlayState(SL_PLAYSTATE_STOPPED)", "TransitionToIdle()");
		result = (*player.buffer_queue)->Clear(player.buffer_queue);
		LogSLOnError(result, "Clear", "TransitionToIdle()");
		nx_atomic_dec_release(&audio_stop);
		buffer_manager.OnClear();
		DestroyPlayer();

		audio_state = IDLE;
	}
}

void AudioThread::TransitionToPrebuffer()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] Transition To Prebuffer");
	SLresult result;  
	nx_atomic_inc_acquire(&audio_stop);
	result = (*player.play)->SetPlayState(player.play, SL_PLAYSTATE_STOPPED);
	LogSLOnError(result, "SetPlayState(SL_PLAYSTATE_STOPPED)", "TransitionToPrebuffer()");
	result = (*player.buffer_queue)->Clear(player.buffer_queue);
	LogSLOnError(result, "Clear", "TransitionToPrebuffer()");
	nx_atomic_dec_release(&audio_stop);
	audio_state = PREBUFFER;
}

nx_thread_return_t AudioThread::AudioThreadFunction(nx_thread_parameter_t parameter)
{
	AudioThread *a = (AudioThread *)parameter;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_AUDIO_OUTPUT);
	a->Run();
	a->Release();
	return 0;
}

int AudioThread::OutputService_AudioOpen(const ifc_audioout::Parameters *format, ifc_player *replicant_player, ifc_playback_parameters *secondary_parameters, ifc_audioout **out_output)
{
	this->replicant_player = replicant_player;
	/* TODO: this isn't very thread friendly */
	if (secondary_parameters)
	{
		if (settings)
			settings->Release();

		if (secondary_parameters->QueryInterface(ifc_audiotrackpro_settings::GetInterfaceGUID(), (void **)&settings) != NErr_Success)
			settings=0; /* just in case */
	}


	/* NXOnce this, once the API can take a parameter */
	if (!audio_thread)
	{
		Retain();
		int ret = NXThreadCreate(&audio_thread, AudioThreadFunction, (nx_thread_parameter_t)this);
		if (ret != NErr_Success)
		{
			Release();
			return ret;
		}
	}

	/* create a new AudioSession object */
	AudioSession *new_session = new (std::nothrow) ReferenceCounted<AudioSession>;
	if (!new_session)
		return NErr_OutOfMemory;

	int ret = new_session->Initialize(this, format, secondary_parameters);
	if (ret != NErr_Success)
	{
		new_session->Release();
		return ret;
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCAddSession;
		apc->param1 = this;
		apc->param2 = new_session;
		new_session->Retain();
		thread_loop.Schedule(apc);
	}
	else 
	{
		new_session->Release();
		return NErr_OutOfMemory;
	}


	*out_output = new_session;
	return NErr_Success;
}

int AudioThread::Flush(size_t position)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCFlush;
		apc->param1 = this;
		apc->param2 = (void *)position;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

int AudioThread::Pause(int state)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCPause;
		apc->param1 = this;
		apc->param2 = (void *)state;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

int AudioThread::Stop()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCStop;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

int AudioThread::Done(AudioSession *session)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCDone;
		apc->param1 = this;
		apc->param2 = session;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

void AudioThread::APCDone(void *_this, void *_session, double unused)
{
	AudioThread *a = (AudioThread *)_this;
	AudioSession *session = (AudioSession *)_session;

	session->OnDone();
}

void AudioThread::APCAddSession(void *_this, void *_session, double unused)
{
	AudioThread *a = (AudioThread *)_this;
	AudioSession *session = (AudioSession *)_session;
	/* session was already retained during OutputService_AudioOpen */
	a->audio_sessions.push_back(session);
}

void AudioThread::APCFlush(void *_this, void *_position, double unused)
{
	/* TODO: how are we going to deal with fade-on-seek.
	I propose setting a state flag and the flush position, but handle the pre-flush fade-out first.  
	basically, delay the flush until the fade-out is done */
	AudioThread *a = (AudioThread *)_this;
	size_t position = (size_t)_position;
	a->Internal_Flush(position);
}

void AudioThread::Internal_Flush(size_t position)
{
	if (!audio_sessions.empty())
	{
		if (IsLastSessionCompatible() == NErr_True)
		{
			while (!audio_sessions.empty())
			{
				replicant_player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
				active_session = audio_sessions.front();
				audio_sessions.pop_front();
			}
			active_session->OnStart(false, 0); /* tell it that we are transitioning, if it wasn't already notified during crossfade */
			crossfading = false;
			crossfade_tryagain = false;
		}
		else
		{
			/* session isn't compatible, we need to kill the audio track */
			replicant_player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
			active_session = 0;
			for (;;)
			{
				AudioSession *next_session = audio_sessions.front();
				if (audio_sessions.size() > 1)
				{
					audio_sessions.pop_front();
					replicant_player->AsynchronousFunctionCall(APCRelease, next_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
				}
				else
				{
					next_session->Flush(position);
					break;
				}
			}

			TransitionToIdle();
		}
	}

	if (active_session)
	{
		active_session->Flush(position);
	}

	if (player.object)
	{
		TransitionToPrebuffer();
		buffer_manager.OnClear();
	}
}

void AudioThread::APCPause(void *_this, void *_state, double unused)
{
	AudioThread *a = (AudioThread *)_this;
	int state = (int)_state;
	if (a->player.play)
	{
		if (a->audio_state == PLAYING)
		{
			SLresult result;
			if (state)
			{
				result = (*a->player.play)->SetPlayState(a->player.play, SL_PLAYSTATE_PAUSED);
				LogSLOnError(result, "SetPlayState(SL_PLAYSTATE_PAUSED)", "APCPause()");
			}
			else
			{
				result = (*a->player.play)->SetPlayState(a->player.play, SL_PLAYSTATE_PLAYING);
				LogSLOnError(result, "SetPlayState(SL_PLAYSTATE_PLAYING)", "APCPause()");
			}
			
		}
	}	
	a->paused = !!state;
}

void AudioThread::APCStop(void *_this, void *unused1, double unused2)
{
	AudioThread *a = (AudioThread *)_this;
	a->Internal_Stop();	
}

void AudioThread::Internal_Stop()
{
	TransitionToIdle();

	if (active_session)
		active_session->Release();
	active_session=0;

	/* remove all sessions */
	while (!audio_sessions.empty())
	{
		AudioSession *s = audio_sessions.front();
		s->Release();
		audio_sessions.pop_front();
	}
}

void AudioThread::RunIdle()
{
	int ret;
	/* wait for creation */
	while (!player.object)
	{
		/* check if we have a session */
		if (!audio_sessions.empty())
		{
			if (equalizer)
				equalizer->Release();
			equalizer=0;

			active_session = audio_sessions.front();
			audio_sessions.pop_front();

			const ifc_audioout::Parameters *parameters = active_session->GetParameters();

			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] opening soundcard.  sample rate = %u, channels = %u", (unsigned int)parameters->audio.sample_rate, parameters->audio.number_of_channels);

			SLDataFormat_PCM output_format;
			output_format.formatType = SL_DATAFORMAT_PCM;
			output_format.numChannels = parameters->audio.number_of_channels;
			output_format.samplesPerSec = parameters->audio.sample_rate * 1000;
			output_format.bitsPerSample = 16;
			output_format.containerSize = 16;
			if (parameters->audio.number_of_channels == 1)
				output_format.channelMask = SL_SPEAKER_FRONT_CENTER;
			else if (parameters->audio.number_of_channels == 2)
				output_format.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
			output_format.endianness = SL_BYTEORDER_LITTLEENDIAN;	

			SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, AUDIO_NUMBER_OF_BUFFERS};
			SLDataSource audioSrc = {&loc_bufq, &output_format};

			SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, output_mix.object};
			SLDataSink audioSnk = {&loc_outmix, NULL};

			const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_ANDROIDCONFIGURATION};
			const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};
			
			SLint32 streamType = SL_ANDROID_STREAM_MEDIA; // TODO: read from ifc_audiotrackpro_settings *settings

			SLresult result;
			result = (*sl.engine)->CreateAudioPlayer(sl.engine, &player.object, &audioSrc, &audioSnk, 3, ids, req);
			LogSLOnError(result, "CreateAudioPlayer", "sl.engine");
			if (result != SL_RESULT_SUCCESS)
			{
				ret = NErr_FailedCreate;
				goto on_error;
			}
			result = (*player.object)->GetInterface(player.object, SL_IID_ANDROIDCONFIGURATION, &player.config);
			LogSLOnError(result, "GetInterface(SL_IID_ANDROIDCONFIGURATION)", "player.object");
			if (result == SL_RESULT_SUCCESS)
			{
				result = (*player.config)->SetConfiguration(player.config, SL_ANDROID_KEY_STREAM_TYPE, &streamType, sizeof(SLint32));
				LogSLOnError(result, "Realize", "player.config");
			}
			else
			{
				player.config=0; // Just in case
			}
			result = (*player.object)->Realize(player.object, SL_BOOLEAN_FALSE);
			LogSLOnError(result, "Realize", "player.object");
			result = (*player.object)->GetInterface(player.object, SL_IID_PLAY, &player.play);
			LogSLOnError(result, "GetInterface(SL_IID_PLAY)", "player.object");
			result = (*player.object)->GetInterface(player.object, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &player.buffer_queue);
			LogSLOnError(result, "GetInterface(SL_IID_BUFFERQUEUE)", "player.object");
			result = (*player.buffer_queue)->RegisterCallback(player.buffer_queue, BufferQueueCallback, this);
			LogSLOnError(result, "RegisterCallback", "player.buffer_queue");
			
			result = (*player.object)->GetInterface(player.object, SL_IID_VOLUME, &player.volume);
			LogSLOnError(result, "GetInterface(SL_IID_VOLUME)", "player.object");

			ret = buffer_manager.Initialize(AUDIO_BUFFER_FRAME_SIZE*sizeof(int16_t)*parameters->audio.number_of_channels, AUDIO_NUMBER_OF_BUFFERS+AUDIO_EXTRA_BUFFERS);
			if (ret != NErr_Success)
				goto on_error;

			/* Create equalizer */
			equalizer = new (std::nothrow) ReferenceCounted<Equalizer>;
			if (!equalizer)
			{
				ret = NErr_OutOfMemory;
				goto on_error;
			}

			ret = equalizer->Initialize(parameters->audio.number_of_channels, parameters->audio.sample_rate);
			if (ret != NErr_Success)
				goto on_error;

			replicant_player->SetEqualizer(equalizer);

			active_session->SetLatency(AUDIO_BUFFER_FRAME_SIZE * (AUDIO_NUMBER_OF_BUFFERS+AUDIO_EXTRA_BUFFERS) * 1000 / parameters->audio.sample_rate);
			active_session->OnStart(false, 0); /* tell it that we are starting but NOT transitioning from another track */
			crossfading = false;
			crossfade_tryagain = false;
			audio_state = PREBUFFER;
			return;
on_error:
			if (equalizer)
				equalizer->Release();
			equalizer=0;
			active_session->SetError(ret);
			active_session->Release();
			active_session=0;
			DestroyPlayer();
		}
		else
		{
			thread_loop.Step(); /* wait for an incoming message */
		}

		if (killswitch) 
			return; // TODO: probably do more than just return
	}
}

void AudioThread::RunPrebuffer()
{
	while (!killswitch)
	{
		/* see if it's time to start, yet */
		if (!active_session)
		{
			audio_state = IDLE;
			return;
		}
		else if (active_session->Done() || active_session->SecondsBuffered() > config_prebuffer_seconds)
		{
			audio_state = PLAYING_PREBUFFER;
			return;
		}
		else
		{
			thread_loop.Step(10);
		}
	}
}

int AudioThread::IsNextSessionCompatible() const
{
	if (WASABI2_API_APP->GetPermission(Features::gapless) != NErr_True)
		return NErr_False;

	AudioSession *next_session = audio_sessions.front();
	if (!next_session) /* nothing to play next? */
	{
		return NErr_False;
	}
	else
	{
		const ifc_audioout::Parameters *current_parameters=active_session->GetParameters();
		const ifc_audioout::Parameters *next_parameters=next_session->GetParameters();
		if (current_parameters->audio.number_of_channels == next_parameters->audio.number_of_channels
			&& (unsigned int)current_parameters->audio.sample_rate == (unsigned int)next_parameters->audio.sample_rate)
		{
			return NErr_True;
		}
		else
		{
			return NErr_False;
		}
	}
}

int AudioThread::IsLastSessionCompatible() const
{
	AudioSession *last_session = audio_sessions.back();
	if (!last_session || !active_session)
	{
		return NErr_False;
	}
	else
	{
		const ifc_audioout::Parameters *current_parameters=active_session->GetParameters();
		const ifc_audioout::Parameters *next_parameters=last_session->GetParameters();
		if (current_parameters->audio.number_of_channels == next_parameters->audio.number_of_channels
			&& (unsigned int)current_parameters->audio.sample_rate == (unsigned int)next_parameters->audio.sample_rate)
		{
			return NErr_True;
		}
		else
		{
			return NErr_False;
		}
	}
}

int AudioThread::SwitchSessions()
{
	if (IsNextSessionCompatible() == NErr_True)
	{
		AudioSession *next_session = audio_sessions.front();
		/* make sure we have enough pre-buffered */
		if (next_session->SecondsBuffered() > config_prebuffer_seconds)
		{
			// just continue where we left off
			replicant_player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
			active_session = next_session;
			audio_sessions.pop_front();
			active_session->OnStart(false, 0); /* tell it that we are transitioning, if it wasn't already notified during crossfade */
			crossfading = false;
			crossfade_tryagain = false;
			return NErr_Success;
		}
	}

	return NErr_False;	
}

/* accumulate audio data from next session */
int AudioThread::AccumulateBuffer(float *output_buffer, size_t output_samples)
{
	size_t bytes_to_read = output_samples * sizeof(float);
	AudioSession *next_session = audio_sessions.front();

	while (bytes_to_read)
	{
		size_t bytes_written;
		int ret = next_session->AccumulateFloat(output_buffer, bytes_to_read, &bytes_written);
		switch(ret)
		{
		case NErr_Underrun:
			/* if we hit an underrun, just treat it like EOF and let the track transition that way */
			return NErr_EndOfFile; 
		case NErr_TryAgain:
			/* we'll just loop and continue */
			output_buffer += bytes_written/sizeof(float);
			bytes_to_read -= bytes_written;
			continue;

		case NErr_EndOfFile:
			/* oh shit.  our crossfade-in track is ALREADY over.  we'll need to call switch sessions from sessions[0] to sessions[1] (but leave active_session alone!).  fun times */
			/* TODO: for now we're just going to let it end like this */
			return NErr_Success;
		case NErr_Success:
		default: /* some other error that we can't handle */
			return ret; 
		}
	}
	return NErr_Success;
}

int AudioThread::FillBuffer(int16_t *&output_buffer, size_t &output_samples)
{
	size_t bytes_written=0;
	size_t bytes_to_read = output_samples * sizeof(float);

	/* read floating point data into our internal buffer */
	int ret = active_session->ReadFloat(float_buffer, bytes_to_read, &bytes_written);
	size_t samples_written = bytes_written / sizeof(float);

	if (crossfading)
	{
		int ret2 = AccumulateBuffer(float_buffer, samples_written);
		if (ret2 != NErr_Success)
			return ret2;
	}

	/* Apply EQ if necessary */
	if (equalizer && equalizer->IsEnabled())
	{			
		if (equalizer->GetGain() != 1.0)
			Float32_To_Float32(float_buffer, float_buffer, samples_written, equalizer->GetGain());
		equalizer->ProcessSamples(float_buffer, float_buffer, samples_written);
		/* convert to 16bit PCM for android */
		Float32_To_Int16_Clip(output_buffer, float_buffer, samples_written);
	}
	else
	{
		Float32_To_Int16_Clip(output_buffer, float_buffer, samples_written);
	}

	output_samples -= samples_written;
	output_buffer += samples_written;

	return ret;
}

void AudioThread::TransitionToPlaying()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] Transition To Playing");
	// TODO: if we got here because of an underrun, things will get strange

	// enqueue ONE buffer.  we don't want to enqueue more because we'll have a race condition
	BufferManager::BufferHeader *buffer = BufferManager::GetBuffer(buffer_manager.filled);
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] TransitionToPlaying() buffer = %x", buffer);
	// we'll put this in the in_use queue _before_ Enqueue, because we have a race condition with the callback (which happens on another thread)
	BufferManager::PutBuffer(buffer_manager.in_use, buffer);
	SLresult result = (*player.buffer_queue)->Enqueue(player.buffer_queue, buffer->GetData(), buffer->bytes_used);
	LogSLOnError(result, "Enqueue", "TransitionToPlaying()");

	if (!paused)
	{
		result = (*player.play)->SetPlayState(player.play, SL_PLAYSTATE_PLAYING);
		LogSLOnError(result, "SetPlayState(SL_PLAYSTATE_PLAYING)", "TransitionToPlaying()");
	}
	audio_state = PLAYING;
}

void AudioThread::RunPlaying()
{
	while ((audio_state == PLAYING || audio_state == PLAYING_PREBUFFER) && !killswitch)
	{
		/* request a buffer */
		BufferManager::BufferHeader *buffer = BufferManager::GetBuffer(buffer_manager.available);
		if (buffer)
		{
			bool close=false;
			size_t samples_to_read = buffer->bytes / sizeof(int16_t);
			int16_t *output_buffer = (int16_t *)buffer->GetData();
			int times_underrun=0;
			for (;;)
			{
				int ret = FillBuffer(output_buffer, samples_to_read); /* function will update these two parameters in-place */
				if (ret == NErr_Success) /* full buffer read */
				{
					if (crossfade_tryagain && !crossfading && active_session->Done())
					{
						if (settings && settings->GetCrossfadeStatus() == NErr_True && IsNextSessionCompatible() == NErr_True)
						{
							AudioSession *next_session = audio_sessions.front();
							if (next_session->SecondsBuffered() > config_prebuffer_seconds)
							{
								size_t crossfade_samples;
								active_session->OnTransition(true, &crossfade_samples);
								next_session->OnStart(true, crossfade_samples);
								crossfading=true;
								crossfade_tryagain=false;
							}
						}
					}
					break;
				}
				else if (ret == NErr_Underrun)
				{
					assert(audio_state == PLAYING || audio_state == PLAYING_PREBUFFER); // our input prebuffer is larger than our intermediate buffer, so this should never occur
					if (times_underrun++ < 100)
					{
						NXSleep(1);
						continue;
					}
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] underrun, need %u more samples", samples_to_read);

					TransitionToPrebuffer();
					buffer_manager.OnRestart();
					buffer->bytes_used = buffer->bytes - samples_to_read * sizeof(int16_t);
					BufferManager::PutBuffer(buffer_manager.filled, buffer);
					audio_state = PREBUFFER;
					return;
				}
				else if (ret == NErr_EndOfFile) /* session was done */
				{
					if (SwitchSessions() != NErr_Success)
					{
						buffer->bytes_used = buffer->bytes - samples_to_read * sizeof(int16_t);
						BufferManager::PutBuffer(buffer_manager.filled, buffer);
						if (audio_state == PLAYING_PREBUFFER) /* if we never even got done with prebuffering, then we need to start */
						{
							TransitionToPlaying();
						}

						active_session->Release();
						active_session=0;
						audio_state = FINISHING;
						return;
					}
				}
				else if (ret == NErr_TryAgain)
				{
					/* we'll get a TryAgain error when output is either transitioning from fade-in to playing or from playing to fade-out 
					so we'll check if it's the latter and turn on crossfade if we need to */
					if (!crossfading && active_session->Done())
					{
						if (settings && settings->GetCrossfadeStatus() == NErr_True && IsNextSessionCompatible() == NErr_True)
						{
							AudioSession *next_session = audio_sessions.front();
							if (next_session->SecondsBuffered() > config_prebuffer_seconds)
							{
								size_t crossfade_samples;
								active_session->OnTransition(true, &crossfade_samples);
								next_session->OnStart(true, crossfade_samples);
								crossfading=true;
							}
							else
							{
								/* next track isn't buffered up enough, so we missed the crossfade opportunity
								let the current track know so it can continue without fading out */
								active_session->OnTransition(false, 0);
								crossfade_tryagain = true;
							}
						}
						else
						{
							/* crossfading is turned off or the next track can't be crossfaded (e.g. different sample rate)
							let the current track know so it can continue without fading out */
							active_session->OnTransition(false, 0);
							if (settings && settings->GetCrossfadeStatus() == NErr_True)
							{
								crossfade_tryagain = true;
							}
						}
					}
				}
			}
			buffer->bytes_used = buffer->bytes;
			BufferManager::PutBuffer(buffer_manager.filled, buffer);
			thread_loop.Try();
		}
		else if (audio_state == PLAYING_PREBUFFER)
		{
			TransitionToPlaying();
			thread_loop.Try();
		}
		else
		{
			if (paused)
				thread_loop.Step();
			else
				thread_loop.uStep(50);
		}		
	}
}

void AudioThread::RunFinishing()
{
	while (audio_state == FINISHING && !killswitch)
	{
		thread_loop.Step();
	}
}

int AudioThread::Finish()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APCFinish;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

void AudioThread::APCFinish(void *_this, void *, double)
{
	AudioThread *a = (AudioThread *)_this;
	switch(a->audio_state)
	{
	case FINISHING:
		a->TransitionToIdle();
		break;
	case PLAYING:
	case PLAYING_PREBUFFER:
		a->TransitionToPrebuffer();
		a->buffer_manager.OnRestart();
		break;
	}
		
}

void AudioThread::Run()
{
	SLresult result;
	result = slCreateEngine(&sl.object, 0, NULL, 0, NULL, NULL);
	LogSLOnError(result, "slCreateEngine");
	result = (*sl.object)->Realize(sl.object, SL_BOOLEAN_FALSE);
	LogSLOnError(result, "Realize", "sl.object");
	result = (*sl.object)->GetInterface(sl.object, SL_IID_ENGINE, &sl.engine);
	LogSLOnError(result, "GetInterface(SL_IID_ENGINE)", "sl.object");
	result = (*sl.engine)->CreateOutputMix(sl.engine, &output_mix.object, 0, 0, 0);
	LogSLOnError(result, "CreateOutputMix", "sl.engine");
	result = (*output_mix.object)->Realize(output_mix.object, SL_BOOLEAN_FALSE);
	LogSLOnError(result, "Realize", "output_mix.object");

	while (!killswitch)
	{
		switch(audio_state)
		{
		case IDLE:
			RunIdle();
			break;
		case PREBUFFER:
			RunPrebuffer();
			break;
		case PLAYING_PREBUFFER:
		case PLAYING:
			RunPlaying();
			break;
		case FINISHING:
			RunFinishing();
			break;
		}
	}
}

void AudioThread::BufferQueueCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	AudioThread *audio_thread = (AudioThread *)context;
	audio_thread->BufferQueueCallback(bq);
}

void AudioThread::BufferQueueCallback(SLAndroidSimpleBufferQueueItf bq)
{
	/* first, let's put the buffer into the available pool */
	BufferManager::BufferHeader *last_used_buffer=BufferManager::GetBuffer(buffer_manager.in_use);
	assert(last_used_buffer);
	BufferManager::PutBuffer(buffer_manager.available, last_used_buffer);

	if (!audio_stop)
	{
		while (BufferManager::BuffersInPool(buffer_manager.in_use) < AUDIO_NUMBER_OF_BUFFERS)
		{
			BufferManager::BufferHeader *filled_buffer = BufferManager::PeekBuffer(buffer_manager.filled);
			if (filled_buffer)
			{
				SLresult result = (*bq)->Enqueue(bq, filled_buffer->GetData(), filled_buffer->bytes_used);
				LogSLOnError(result, "Enqueue", "Buffer Queue Callback");
				if (result == SL_RESULT_SUCCESS)
				{
					BufferManager::Advance(buffer_manager.filled);
					BufferManager::PutBuffer(buffer_manager.in_use, filled_buffer);
					continue;
				}
			}

			if (BufferManager::Empty(buffer_manager.in_use))
			{
				__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[OpenSL] Buffer Queue Callback: Underrun");
				Finish();
			}
			break;			
		}
	}
}