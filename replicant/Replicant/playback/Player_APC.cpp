#include "Player.h"
/*
part of the Player object.
this implementation file is separate just to prevent Player.cpp from getting too unwieldy 
*/

template<class disp_t>
static void SwapAndRelease(disp_t **destination, disp_t *source)
{
	disp_t *old = *destination;
	*destination = (disp_t *)source;
	if (old)
		old->Release();
}

void Player::APC_SetMetadata(void *_player, void *_new_metadata, double unused)
{
	Player *player = (Player *)_player;
	ifc_metadata *new_metadata = (ifc_metadata *)_new_metadata;

	SwapAndRelease(&player->metadata, new_metadata);
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnMetadataChanged(new_metadata);
	}
}

void Player::APC_SetLength(void *_player, void *unused, double new_length)
{
	Player *player = (Player *)_player;
	player->length = new_length;
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnLengthChanged(new_length);
	}
}

void Player::APC_SetPosition(void *_player, void *unused, double new_position)
{
	Player *player = (Player *)_player;
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnPositionChanged(new_position);
	}
}

void Player::APC_SetEqualizer(void *_player, void *_new_eq, double unused)
{
	Player *player = (Player *)_player;
	ifc_equalizer *new_eq = (ifc_equalizer *)_new_eq;
	player->Internal_SetEqualizer(new_eq);
}

void Player::APC_Play(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_Play();
}

void Player::APC_Stop(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_Stop();
}

void Player::APC_Reset(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_Reset();
}

void Player::APC_Load(void *_player, void *filename, double unused)
{
	Player *player = (Player *)_player;
	nx_uri_t nx_filename = (nx_uri_t)filename;
	player->Internal_Load(nx_filename);
	NXURIRelease(nx_filename);
}

void Player::APC_OnLoaded(void *_player, void *filename, double unused)
{
	Player *player = (Player *)_player;
	nx_uri_t nx_filename = (nx_uri_t)filename;
	player->Internal_OnLoaded(nx_filename);
	NXURIRelease(nx_filename);
}

void Player::APC_OnError(void *_player, void *error_code, double unused)
{
	Player *player = (Player *)_player;
	NError code = (NError)(intptr_t)error_code;
	
	player->Internal_OnError(code);
}

void Player::APC_OnEndOfFile(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;

	player->Internal_OnEndOfFile();
}

void Player::APC_RegisterForEvents(void *_player, void *callback, double unused)
{
	Player *player = (Player *)_player;
	player->Internal_RegisterForEvents((cb_playerevents *)callback);
}

void Player::APC_UnregisterForEvents(void *_player, void *callback, double unused)
{
	Player *player = (Player *)_player;
	player->Internal_UnregisterForEvents((cb_playerevents *)callback);
}

void Player::APC_OnSeekComplete(void *_player, void *_error_code, double new_position)
{
	Player *player = (Player *)_player;
	int error_code = (int)(intptr_t)_error_code;
	
	/* Send callbacks */
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnSeekComplete(error_code, new_position);
	}
}

void Player::APC_SetSeekable(void *_player, void *_is_seekable, double unused)
{
		Player *player = (Player *)_player;
		int is_seekable = (int)(intptr_t)_is_seekable;
	
	/* Send callbacks */
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnSeekable(is_seekable);
	}
}

void Player::APC_OnStopped(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_OnStop();

}

void Player::APC_SeekSeconds(void *_player, void *unused, double seconds)
{
	Player *player = (Player *)_player;
	player->Internal_SeekSeconds(seconds);
}

void Player::APC_SeekPercent(void *_player, void *unused, double percent)
{
	Player *player = (Player *)_player;
	player->Internal_SeekPercent(percent);
}

void Player::APC_Pause(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_Pause();
}

void Player::APC_SetBufferStatus(void *_player, void *_percent, double unused)
{
	Player *player = (Player *)_player;
	int buffer_percentage = (int)(intptr_t)_percent;
		/* Send callbacks */
	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnBuffering(buffer_percentage);
	}
}

void Player::APC_SetEQState(void *_player, void *_off_on, double unused)
{
	Player *player = (Player *)_player;
	player->Internal_SetEQState((int)(intptr_t)_off_on);
}

void Player::APC_SetEQPreamp(void *_player, void *unused, double _dB)
{
	Player *player = (Player *)_player;
	player->Internal_SetEQPreamp(_dB);
}

void Player::APC_SetEQBand(void *_player, void *_band, double _dB)
{
	Player *player = (Player *)_player;
	player->Internal_SetEQBand((unsigned int)(uintptr_t)_band, _dB);
}


void Player::APC_OnReady(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;

	for (EventList::iterator itr=player->player_events.begin(); itr != player->player_events.end(); itr++)
	{
		(*itr)->OnReady();
	}	
}

void Player::APC_OnClosed(void *_player, void *unused, double unused2)
{
	Player *player = (Player *)_player;
	player->Internal_OnClosed();
}
