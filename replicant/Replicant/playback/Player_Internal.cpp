#include "Player.h"
#include <assert.h>

#ifdef __ANDROID__
#include <android/log.h> // TODO: replace with generic logging API

#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
void __android_log_print(int, const char *, const char *, ...)
{
}
#endif

void Player::Internal_Play()
{	
	switch(player_state)
	{

	case State_Loading: // too early, bro.  but we can deal with it anyway
	case State_Loaded:
		assert(playback);
		playback->Play(output, playback_parameters);
		LogStateTransition(player_state, State_Playing);
		player_state = State_Playing;
		break;
	case State_None:
	case State_Playing: // already playing, we can ignore
	case State_Stopping:	// too late, bro.
		__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[Player] Tried to start playback when in State:Playing", GetStateName(player_state));
		break;
	case State_EndOfFile: 	// re-start playback
		assert(playback);
		playback->Play(output, 0 /* TODO */);
		LogStateTransition(player_state, State_Playing);
		player_state = State_Playing;
		break;
	}	
}


void Player::Internal_SeekSeconds(double seconds)
{
	if (playback)
	{
		playback->SeekSeconds(seconds);
	}
}

void Player::Internal_SeekPercent(double percent)
{
	if (playback)
	{
		playback->SeekSeconds(percent*length);
	}
}

void Player::Internal_Pause()
{
	if (playback)
	{
		int ret;
		if (!paused)
			ret=playback->Pause();
		else
			ret=playback->Unpause();
		if (ret == 0)
			paused = !paused;
	}
}


void Player::Internal_Stop()
{
	switch(player_state)
	{

	case State_Loading:
	case State_Loaded:
	case State_Playing:
		assert(playback);
		playback->Stop();
		LogStateTransition(player_state, State_Stopping);

		player_state = State_Stopping;
		break;
	case State_None:
	case State_Stopping:
		__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[Player] Tried to stop playback when in State:Stopping", GetStateName(player_state));
		break;
	case State_EndOfFile: /* playback plugin is sitting waiting for the next command, let's kill it */
		assert(playback);
		playback->Close();

		LogStateTransition(player_state, State_None);
		player_state = State_None;
		break;
	}	
}

void Player::Internal_Reset()
{
	switch(player_state)
	{
	case State_Loading:
	case State_Loaded:
	case State_Playing:
		assert(playback);
		playback->Stop();
		LogStateTransition(player_state, State_Stopping);

		player_state = State_Stopping;
		break;
	}	
}

void Player::Internal_RegisterForEvents(cb_playerevents *event_handler)
{
	player_events.push_back(event_handler);
	event_handler->OnInitialized();
}

void Player::Internal_UnregisterForEvents(cb_playerevents *event_handler)
{
	player_events.erase(event_handler);
	event_handler->Release(); /* release the reference that we had because it was in our list */	
	event_handler->Release(); /* release the reference that was retained before the APC */
}

bool Player::Internal_Load(nx_uri_t new_filename)
{
	if (playback) /* if we still have a playback object, then we're not done */
	{
		//		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Player] playback object exists");
		if (player_state == State_Loading
			|| player_state == State_Loaded
			|| player_state == State_Playing
			|| player_state == State_EndOfFile)
		{
			/* if we're in one of these states, we'll need to force a stop
			note: State_EndOfFile will force a close instead */
			Internal_Stop();
		}

		/* first give up our timeslice (to hopefully let the playback thread respond to stop) */
		NXSleepYield();

		/* then we need to re-queue the event */
		/* TODO: it might better to loop and Step the thread_loop */
		Load(new_filename);
		return false;
	}
	else
	{
		nx_uri_t old_filename = filename;
		filename = NXURIRetain(new_filename);
		if (old_filename)
			NXURIRelease(old_filename);

		playback = FindPlayback(filename);
		if (playback)
		{

			LogStateTransition(player_state, State_Loading);
			player_state = State_Loading;
			return true;
		}
		else
		{
			APC_OnError(this, (void *)NErr_NoMatchingImplementation, 0);
			return false;
		}
	}
}

void Player::Internal_OnStop()
{
	assert(player_state == State_Stopping);

	LogStateTransition(player_state, State_None);
	player_state = State_None;

	if (filename)
		NXURIRelease(filename);
	filename=0;

	paused=false;
	length=0;

	if (playback)
		playback->Release();

	playback=0;

	if (metadata)
		metadata->Release();
	metadata=0;

	for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
	{
		(*itr)->OnStopped();
	}
}

void Player::Internal_OnEndOfFile()
{
	/* if we're in the middle of a stop, but got an EOF, we want to let the OnStop clean up instead of us 
	because otherwise OnStop might show up AFTER the next track has been loaded */
	if (player_state != State_Stopping)
	{	
		LogStateTransition(player_state, State_EndOfFile);

		player_state = State_EndOfFile;

		for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
		{
			(*itr)->OnEndOfFile();
		}	
	}
}

void Player::Internal_OnLoaded(nx_uri_t filename)
{
	switch(player_state)
	{
	case State_Loading: // if we're NOT in this state, it means someone pre-empted us
		LogStateTransition(player_state, State_Loaded);
		player_state = State_Loaded;
		/* Send callbacks */
		for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
		{
			(*itr)->OnLoaded(filename);
		}
		break;
	default:
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Player] OnLoaded received when in State:%s, ignoring.", GetStateName(player_state));
		break;		
	}
}

void Player::Internal_OnError(NError code)
{
	LogStateTransition(player_state, State_None);

	player_state = State_None;

	if (filename)
		NXURIRelease(filename);
	filename=0;

	paused=false;
	length=0;

	if (playback)
		playback->Release();

	playback=0;

	if (metadata)
		metadata->Release();
	metadata=0;


	for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
	{
		(*itr)->OnError(code);
	}

}

void Player::Internal_SetEqualizer(ifc_equalizer *_equalizer)
{
	if (equalizer)
		equalizer->Release();
	equalizer = _equalizer;

	/* set the EQ values again incase EQ band changes snuck in the meantime */
	if (equalizer)
	{
		if (eq.on)
			equalizer->Enable();
		equalizer->SetPreamp(eq.preamp);
		for (unsigned int i=0;i<10;i++)
		{
			equalizer->SetBand(i, eq.bands[i]);
		}
	}

	for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
	{
		(*itr)->OnEqualizerChanged(equalizer);
	}
}

void Player::Internal_SetEQState(int _off_on)
{
	eq.on = _off_on;
	if (equalizer)
	{
		if (eq.on)
			equalizer->Enable();
		else
			equalizer->Disable();
	}	
}

void Player::Internal_SetEQPreamp(double _dB)
{
	eq.preamp=_dB;
	if (equalizer)
		equalizer->SetPreamp(_dB);
}

void Player::Internal_SetEQBand(unsigned int _band, double _dB)
{
	eq.bands[_band]=_dB;
	if (equalizer)
		equalizer->SetBand(_band, _dB);
}

void Player::Internal_OnClosed()
{
	if (filename)
		NXURIRelease(filename);
	filename=0;

	paused=false;
	length=0;

	if (playback)
		playback->Release();
	playback=0;

	if (metadata)
		metadata->Release();
	metadata=0;

	LogStateTransition(player_state, State_None);

	player_state = State_None;

	for (EventList::iterator itr=player_events.begin(); itr != player_events.end(); itr++)
	{
		(*itr)->OnClosed();
	}
}
