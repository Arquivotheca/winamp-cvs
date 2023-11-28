#include "api.h"
#include "AudioThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "pcmutils.h"
#include "player/ifc_player.h"
#include "application/features.h"
#include "nx/nxsleep.h"
#include <new>

/* TODO if we wanted support for multiple simultaneously players, we'd probably need to make a separate AudioThread per player,
and have the service keep track of them */

/* TODO: deal with the following
* paused - if needed to fade-out on pausing, we could do getPosition
* fade-on-seek
*/

const static double config_prebuffer_seconds = AUDIO_PREBUFFER_SECONDS;			// Default: Half of a second

static void APCRelease(void *_session, void *unused1, double unused2)
{
	AudioSession *session = (AudioSession *)_session;

	if (session)
		session->Release();
}

AudioThread::AudioThread()
{
	audio_thread=0;
	audio_track=0;
	started=false;
	crossfading=false;
	paused=false;
	crossfade_tryagain=false;
	audio_state = IDLE;
	active_session = 0;
	equalizer = 0;
	player=0;
	killswitch=0;
	settings=0;
}

AudioThread::~AudioThread()
{
	if (settings)
		settings->Release();
}

nx_thread_return_t AudioThread::AudioThreadFunction(nx_thread_parameter_t parameter)
{
	AudioThread *a = (AudioThread *)parameter;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_AUDIO_OUTPUT);
	a->Run();
	a->Release();
	return 0;
}

int AudioThread::OutputService_AudioOpen(const ifc_audioout::Parameters *format, ifc_player *player, ifc_playback_parameters *secondary_parameters, ifc_audioout **out_output)
{
	this->player = player;
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
				player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
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
			player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
			active_session = 0;
			for (;;)
			{
				AudioSession *next_session = audio_sessions.front();
				if (audio_sessions.size() > 1)
				{
					audio_sessions.pop_front();
					player->AsynchronousFunctionCall(APCRelease, next_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
				}
				else
				{
					next_session->Flush(position);
					break;
				}
			}

			/* stop (and flush) the sound card */
			if (audio_track)
			{
				audio_track->stop();
				audio_track->flush();
				delete audio_track;
				audio_track=0;
				audio_state = IDLE;
			}
		}
	}

	if (active_session)
	{
		active_session->Flush(position);
	}

	if (audio_track)
	{
		audio_track->stop();
		audio_track->flush();
		audio_state = PREBUFFER;
	}
}

void AudioThread::APCPause(void *_this, void *_state, double unused)
{
	AudioThread *a = (AudioThread *)_this;
	int state = (int)_state;
	if (a->audio_track)
	{
		if (a->audio_state == PLAYING)
		{
			if (state)
				a->audio_track->pause();
			else
			{
				a->audio_track->start();
				a->audio_track->setPositionUpdatePeriod(0);
				a->audio_track->setMarkerPosition(0); 
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
	/* stop (and flush) the sound card */
	if (audio_track)
	{
		audio_track->stop();
		audio_track->flush();
		delete audio_track;
		audio_track=0;
		audio_state = IDLE;
	}

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
	/* wait for creation */
	while (!audio_track)
	{
		/* check if we have a session */
		if (!audio_sessions.empty())
		{
			active_session = audio_sessions.front();
			audio_sessions.pop_front();

			const ifc_audioout::Parameters *parameters = active_session->GetParameters();
			android::status_t status;
			void *blah = malloc(sizeof(android::AudioTrack) + 4096);
			if (!blah)
			{
				active_session->SetError(NErr_OutOfMemory);
				active_session->Release();
				active_session=0;
			}
			else
			{
				audio_track = new (blah) android::AudioTrack();
				if (!audio_track)
				{
					active_session->SetError(NErr_OutOfMemory);
					active_session->Release();
					active_session=0;
				}
				else
				{
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] opening soundcard.  sample rate = %u, channels = %u", (unsigned int)parameters->audio.sample_rate, parameters->audio.number_of_channels);
					unsigned int buffer_frames = (unsigned int)(parameters->audio.sample_rate * AUDIOTRACK_BUFFER_SECONDS); /* default: quarter second changed to half a second for Android 4.0 of buffer */
					/* create AudioTrack object (soundcard output for android) */
					if (WASABI2_API_ANDROID->GetSDKVersion() <= 5)
					{
						status = audio_track->set(android::AudioSystem::MUSIC, parameters->audio.sample_rate, android::AudioSystem::PCM_16_BIT, parameters->audio.number_of_channels, buffer_frames /* latency */, 0 /* flags */, 0, 0);
					}
					else
					{
						int chans = (parameters->audio.number_of_channels == 2) ? 12 : 4;
						status = audio_track->set(android::AudioSystem::MUSIC, parameters->audio.sample_rate, android::AudioSystem::PCM_16_BIT, chans, buffer_frames /* latency */, 0 /* flags */, 0, 0);
					}

					if (status != android::NO_ERROR)
					{	
						active_session->SetError(NErr_UnsupportedFormat);
						active_session->Release();
						active_session=0;
						delete audio_track;
						audio_track=0;
						return;
					}

					/* Create equalizer */
					if (equalizer)
						equalizer->Release();

					equalizer=0;

					equalizer = new (std::nothrow) ReferenceCounted<Equalizer>;
					if (!equalizer)
					{
						active_session->SetError(NErr_OutOfMemory);
						active_session->Release();
						active_session=0;
						delete audio_track;
						audio_track=0;
						return ;
					}

					int ret = equalizer->Initialize(parameters->audio.number_of_channels, parameters->audio.sample_rate);
					if (ret != NErr_Success)
					{
						equalizer->Release();
						equalizer=0;
						active_session->SetError(ret);
						active_session->Release();
						active_session=0;
						delete audio_track;
						audio_track=0;
						return ;
					}
					player->SetEqualizer(equalizer);


					active_session->OnStart(false, 0); /* tell it that we are starting but NOT transitioning from another track */
					started = false;
					crossfading = false;
					crossfade_tryagain = false;
					audio_state = PREBUFFER;
					return;
				}
			}
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

			if (!paused)
			{
				audio_track->start();
				audio_track->setPositionUpdatePeriod(0);
				audio_track->setMarkerPosition(0); 
			}
			else
			{
			}

			started=true;
			audio_state = PLAYING;
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
			player->AsynchronousFunctionCall(APCRelease, active_session, 0, 0); /* Release() the session on another thread so we don't block during this critical period */
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
			__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[audiotrack-pro] Underrun in AccumulateBuffer");
			return NErr_EndOfFile; 
		case NErr_TryAgain:
			/* we'll just loop and continue */
			output_buffer += bytes_written/sizeof(float);
			bytes_to_read -= bytes_written;
			continue;

		case NErr_EndOfFile:
			/* oh shit.  our crossfade-in track is ALREADY over.  we'll need to call switch sessions from sessions[0] to sessions[1] (but leave active_session alone!).  fun times */
			/* TODO: for now we're just going to let it end like this */
			__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[audiotrack-pro] EndOfFile in AccumulateBuffer");
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

void AudioThread::RunPlaying()
{
	android::AudioTrack::Buffer buffer;
	while (audio_state == PLAYING && !killswitch)
	{
		/* request a buffer */
		buffer.frameCount=AUDIOTRACK_BUFFER_FRAME_SIZE;
		android::status_t audio_ret = audio_track->obtainBuffer(&buffer, AUDIOTRACK_BUFFER_TIMEOUT);
		if (audio_ret == android::NO_ERROR || audio_ret == android::AudioTrack::STOPPED)
		{
			bool close=false;
			size_t samples_to_read = buffer.size / sizeof(int16_t);
			int16_t *output_buffer = buffer.i16;
			int times_underrun=0;
			for (;;)
			{
				int ret = FillBuffer(output_buffer, samples_to_read); /* function will update these two parameters in-place */
				if (ret == NErr_Success) /* full buffer read */
				{
					if (crossfade_tryagain && !crossfading && active_session->Done())
					{
						__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] handling crossfade try again ");
						if (settings && settings->GetCrossfadeStatus() == NErr_True && IsNextSessionCompatible() == NErr_True)
						{
							AudioSession *next_session = audio_sessions.front();
							if (next_session->SecondsBuffered() > config_prebuffer_seconds)
							{
								__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] transition");
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
					if (times_underrun++ < 3)
					{
						NXSleep(50);
						continue;
					}
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] underrun, filling up %u samples with zeroes", samples_to_read);
					/* fill any remaining bytes up with silence */
					while (samples_to_read--)
						*output_buffer++ = 0;
					audio_track->releaseBuffer(&buffer);
					audio_track->stop();
					audio_state = PREBUFFER;
					return;
				}
				else if (ret == NErr_EndOfFile) /* session was done */
				{
					if (SwitchSessions() != NErr_Success)
					{
						/* fill any remaining bytes up with silence */
						while (samples_to_read--)
							*output_buffer++ = 0;
						audio_track->releaseBuffer(&buffer);
						audio_track->stop();
						active_session->Release();
						active_session=0;
						delete audio_track;
						audio_track=0;
						audio_state = IDLE;
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
								__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] Aborted crossfade due to not enough buffering");
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
				else
				{
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] some other error=%d", ret);
					break;
				}
			}
			audio_track->releaseBuffer(&buffer);
			thread_loop.Try();
		}
		else
		{
			if (paused)
				thread_loop.Step();
			else
				thread_loop.Step(AUDIOTRACK_BUFFER_THREAD_LOOP_STEP);
		}		
	}
}

void AudioThread::Run()
{
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
		case PLAYING:
			RunPlaying();
			break;
		}
	}

}
