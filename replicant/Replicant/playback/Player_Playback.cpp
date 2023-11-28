#include "Player.h"

void Player::Player_SetMetadata(ifc_metadata *_metadata)
{
	if (_metadata)
	{
		threadloop_node_t *apc = thread_loop.GetAPC();
		if (apc)
		{
			_metadata->Retain();
			apc->func = APC_SetMetadata;
			apc->param1 = this;
			apc->param2 = _metadata;
			thread_loop.Schedule(apc);
		}	
	}	
}

void Player::Player_SetLength(double new_length)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetLength;
		apc->param1 = this;
		apc->real_value = new_length;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_SetPosition(double new_position)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetPosition;
		apc->param1 = this;
		apc->real_value = new_position;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_OnLoaded(nx_uri_t filename)
{
	/* this gets called when the playback thread begins */
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnLoaded;
		apc->param1 = this;
		apc->param2 = (void *)NXURIRetain(filename); /* TODO: error check */
		thread_loop.Schedule(apc);
	}
}

void Player::Player_OnEndOfFile()
{
	/* this gets called when the song has stopped playing due to end-of-file */
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnEndOfFile;
		apc->param1 = this;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_SetBitrate(uint64_t bitrate, double timestamp)
{
}

void Player::Player_OnError(NError code)
{
	/* when this gets called, the playback plugin will be in a 'stopped' state so Stop() does not have to be explicitly called */

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnError;
		apc->param1 = this;
		apc->param2 = (void *)code;
		thread_loop.Schedule(apc);
	}	
}

/* this will get called on playback thread */
void Player::Player_SetEqualizer(ifc_equalizer *_equalizer)
{
	if (eq.on)
		_equalizer->Enable();
	_equalizer->SetPreamp(eq.preamp);
	for (unsigned int i=0;i<10;i++)
	{
		_equalizer->SetBand(i, eq.bands[i]);
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		_equalizer->Retain();
		apc->func = APC_SetEqualizer;
		apc->param1 = this;
		apc->param2 = _equalizer;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_SetBufferStatus(int percent)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetBufferStatus;
		apc->param1 = this;
		apc->param2 = (void *)percent;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_OnStopped()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnStopped;
		apc->param1 = this;
		thread_loop.Schedule(apc);
	}
}

void Player::Player_OnSeekComplete(int error_code, double new_position)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnSeekComplete;
		apc->param1 = this;
		apc->param2 = (void *)error_code;
		apc->real_value = new_position;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_SetSeekable(int seekable)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_SetSeekable;
		apc->param1 = this;
		apc->param2 = (void *)seekable;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_AsynchronousFunctionCall(void (*function)(void *, void *, double), void *param1, void *param2, double real_param)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = function;
		apc->param1 = param1;
		apc->param2 = param2;
		apc->real_value = real_param;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_OnReady()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnReady;
		apc->param1 = this;
		thread_loop.Schedule(apc);
	}	
}

void Player::Player_OnClosed()
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_OnClosed;
		apc->param1 = this;
		thread_loop.Schedule(apc);
	}	
}
