#include "Player.h"
#include "player/svc_playback.h"
#include "player/ifc_playback.h"
#include "player/svc_output.h"
#include "api.h"

#ifdef __ANDROID__
#include <android/log.h> // TODO: replace with generic logging API

#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
static void __android_log_print(int, const char *, const char *, ...)
{
}
#endif

const char *Player::GetStateName(Player::PlayerState state)
{
	
	switch(state)
	{
	case 			State_None: return "None"; 
	case 	State_Loading: return "Loading"; 
	case 			State_Loaded: return "Loaded";
	case 	State_Playing: return"Playing"; 
	case 			State_Stopping: return"Stopping"; 
	case 			State_EndOfFile: return "EndOfFile"; 
	}
	return "Unknown State";

}

void Player::LogStateTransition(Player::PlayerState from, Player::PlayerState to)
{
	const char *f=GetStateName(from), *t=GetStateName(to);
	
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Player] state transition from %s to %s", f, t);
}

Player::Player()
{
	eq.on=0;
	eq.preamp=0;
	for (int i=0;i<10;i++)
	{
		eq.bands[i]=0;
	}

	filename=0;
	playback=0;
	metadata=0;
	paused=false;
	equalizer=0;
	player_thread=0;
	output=0;
	playback_parameters=0;
	player_state = State_None;
}

Player::~Player()
{
	thread_loop.Kill();
	NXThreadJoin(player_thread, 0);
	if (filename)
		NXURIRelease(filename);
	if (playback_parameters)
		playback_parameters->Release();
}

void Player::SetPlaybackParameters(ifc_playback_parameters *new_parameters)
{
	ifc_playback_parameters *old = playback_parameters;
	playback_parameters = new_parameters;
	if (playback_parameters)
		playback_parameters->Retain();
	if (old)
		old->Release();
}

int Player::Load(nx_uri_t uri)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Load;
		apc->param1 = this;
		apc->param2 = (void *)NXURIRetain(uri);
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
		return NErr_OutOfMemory;
}

static svc_output *FindOutput()
{
	GUID output_guid = svc_output::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(output_guid);
	for (size_t i=0; i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(output_guid,i);
		if (sf)
		{	
			svc_output * l = (svc_output*)sf->GetInterface();
			if (l)
			{
				return l;
			}
		}
	}
	return NULL;
}

int Player::Init()
{
	if (!player_thread)
		NXThreadCreate(&player_thread, PlayerThreadFunction, this);

	return NErr_Success;
}

nx_thread_return_t Player::PlayerThreadFunction(nx_thread_parameter_t param)
{
	Player *player = (Player *)param;
	player->output = FindOutput();

	/* Send callbacks */
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnInitialized();
	}

	player->thread_loop.Run();
	return 0;
}

static ifc_playback *FindPlaybackTryAgain(size_t i, size_t n, nx_uri_t filename, ifc_player *player, svc_playback *fallback)
{
	ifc_playback *playback;

	GUID playback_guid = svc_playback::GetServiceType();
	for (;i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(playback_guid, i);
		if (sf)
		{	
			svc_playback * l = (svc_playback*)sf->GetInterface();
			if (l)
			{
				ns_error_t ret = l->CreatePlayback(0, filename, player, &playback);
				if (ret == NErr_Success)
				{
					l->Release();
					return playback;
				}
				else if (ret == NErr_TryAgain)
				{
					playback = FindPlaybackTryAgain(i+1, n, filename, player, l);
					l->Release();
					return playback;
				}
				l->Release();
			}
		}
	}

	if (fallback && fallback->CreatePlayback(1, filename, player, &playback) == NErr_Success)
	{
		return playback;
	}

	return 0;
}

ifc_playback *Player::FindPlayback(nx_uri_t filename)
{
	GUID playback_guid = svc_playback::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(playback_guid);
	return FindPlaybackTryAgain(0, n, filename, this, 0);
}

int Player::Start()
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Play;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}	
	else
		return NErr_OutOfMemory;	
}



int Player::Pause()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Pause;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int Player::Seek(double percent)
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SeekPercent;
		apc->param1 = this;
		apc->real_value = percent;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int Player::SeekSeconds(double seconds)
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SeekSeconds;
		apc->param1 = this;
		apc->real_value = seconds;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;


}

int Player::SeekPercent(double percent)
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SeekPercent;
		apc->param1 = this;
		apc->real_value = percent;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;


}

int Player::Stop()
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Stop;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}	
	else
		return NErr_OutOfMemory;		
}

int Player::Reset()
{
	// get onto the player thread
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Reset;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}	
	else
		return NErr_OutOfMemory;		
}

void Player::RegisterForEvents(cb_playerevents *event_handler)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		event_handler->Retain();
		apc->func = APC_RegisterForEvents;
		apc->param1 = this;
		apc->param2 = event_handler;
		thread_loop.Schedule(apc);
	}
}

void Player::UnregisterForEvents(cb_playerevents *event_handler)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		event_handler->Retain(); /* in theory we already hold it. but if they unregister something that was never registered, we could have issues if we don't do this */
		apc->func = APC_UnregisterForEvents;
		apc->param1 = this;
		apc->param2 = event_handler;
		thread_loop.Schedule(apc);
	}	
}

void Player::SetEQState(int off_on)
{
	eq.on=off_on;
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetEQState;
		apc->param1 = this;
		apc->param2 = (void *)off_on;
		thread_loop.Schedule(apc);
	}	
}

void Player::SetEQPreamp(double dB)
{
	eq.preamp=dB;
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetEQPreamp;
		apc->param1 = this;
		apc->real_value = dB;
		thread_loop.Schedule(apc);
	}	
}

void Player::SetEQBand(unsigned int band, double dB)
{
	eq.bands[band]=dB;
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetEQBand;
		apc->param1 = this;
		apc->param2 = (void *)band;
		apc->real_value = dB;
		thread_loop.Schedule(apc);
	}	
}

int Player::GetEQState(void)
{
	return eq.on;
}

double Player::GetEQPreamp(void)
{
	return eq.preamp;
}

double Player::GetEQBand(unsigned int band)
{
	if (band < 10)
		return eq.bands[band];
	else
		return 0.0;		// Should we catch this error better ?
}

