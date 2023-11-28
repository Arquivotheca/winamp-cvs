#pragma once
#include "player/ifc_playback.h"
#include "player/ifc_player.h"
#include "player/cb_playerevents.h"
#include "metadata/ifc_metadata.h"
#include "nu/PtrDeque.h"
#include "nu/AutoLock.h"
#include "nu/ThreadLoop.h"
#include "nx/nxthread.h"
#include "nx/nxsleep.h"

/*
implementation is separated into four files:
Player.cpp - API that is used by the host application
Player_APC.cpp - implements the APC() family of functions (stub functions that get run via the ThreadLoop), keeps this 'glue' code separate for ease of maintenance
Player_Internal.cpp - implements the Internal_() family of functions that are called via the APC functions
Player_Playback.cpp - implements ifc_player
*/



class Player : public ifc_player
{
public:
	Player();
	~Player();

	/* API used by the application */
	int Init();
	int Load(nx_uri_t uri);
	
	int Start();
	int Pause();
	int Stop();
	int Reset(); /* same as Stop(), except that it won't stop if in EndOfFile state */
	int Seek(double percent);
	int SeekSeconds(double seconds);
	int SeekPercent(double percent);
	
	void RegisterForEvents(cb_playerevents *event_handler);
	void UnregisterForEvents(cb_playerevents *event_handler);
	
	void SetEQState(int off_on); /* off=0 on=1 */
	void SetEQPreamp(double dB); 
	void SetEQBand(unsigned int band, double dB); 
	int GetEQState(void); /* off=0 on=1 */
	double GetEQPreamp(void); 
	double GetEQBand(unsigned int band); 
	void SetPlaybackParameters(ifc_playback_parameters *new_parameters);


	enum PlayerState
	{
		State_None, /* ready for a Load call to happen, basically */
		State_Loading, /* waiting for playback thread to spawn */
		State_Loaded, /* Loaded */
		State_Playing, /* playback thread is active (but potentially paused) */
		State_Stopping, /* playback thread has been asked to shut down */
		State_EndOfFile, /* playback thread hit the end of the file and is waiting for us to kill it */
	};
private:
	static nx_thread_return_t NXTHREADCALL PlayerThreadFunction(nx_thread_parameter_t param);
	static void LogStateTransition(PlayerState from, PlayerState to);
	static const char *GetStateName(Player::PlayerState state);

	
	/* ifc_player implementations */
	void WASABICALL Player_SetMetadata(ifc_metadata *metadata);
	void WASABICALL Player_SetLength(double length);
	void WASABICALL Player_SetPosition(double new_position);
	void WASABICALL Player_OnLoaded(nx_uri_t filename);
	void WASABICALL Player_OnEndOfFile();
	void WASABICALL Player_OnStopped();
	void WASABICALL Player_SetBitrate(uint64_t bitrate, double timestamp);
	void WASABICALL Player_OnError(NError code);
	void WASABICALL Player_SetEqualizer(ifc_equalizer *equalizer);
	void WASABICALL Player_SetBufferStatus(int percent);
	void WASABICALL Player_OnSeekComplete(int error_code, double new_position);
	void WASABICALL Player_SetSeekable(int seekable);
	void WASABICALL Player_AsynchronousFunctionCall(void (*function)(void *, void *, double), void *param1, void *param2, double real_param);
	void WASABICALL Player_OnReady();
	void WASABICALL Player_OnClosed();


	/* stuff that got APC'd over to the player thread */	
	static void APC_SetMetadata(void *player, void *metadata, double unused);
	static void APC_SetLength(void *player, void *unused, double length);
	static void APC_SetPosition(void *player, void *unused, double new_position);
	static void APC_SetEqualizer(void *_player, void *new_eq, double unused);
	static void APC_Play(void *_player, void *unused, double unused2);
	static void APC_Stop(void *_player, void *unused, double unused2);
	static void APC_Reset(void *_player, void *unused, double unused2);
	static void APC_Load(void *_player, void *filename, double unused);
	static void APC_OnLoaded(void *_player, void *filename, double unused);
	static void APC_OnError(void *_player, void *error_code, double unused);
	static void APC_OnEndOfFile(void *_player, void *unused, double unused2);
	static void APC_RegisterForEvents(void *_player, void *callback, double unused);
	static void APC_UnregisterForEvents(void *_player, void *callback, double unused);
	static void APC_OnSeekComplete(void *_player, void *error_code, double new_position);
	static void APC_SetSeekable(void *_player, void *is_seekable, double unused);
	static void APC_OnStopped(void *_player, void *unused, double unused2);
	static void APC_SeekSeconds(void *_player, void *unused, double seconds);
	static void APC_SeekPercent(void *_player, void *unused, double percent);
	static void APC_Pause(void *_player, void *unused, double unused2);
	static void APC_SetBufferStatus(void *_player, void *_percent, double unused);
	static void APC_SetEQState(void *_player, void *_off_on, double unused);
	static void APC_SetEQPreamp(void *_player, void *unused, double _dB);
	static void APC_SetEQBand(void *_player, void *_band, double _dB);
	static void APC_OnReady(void *_player, void *unused, double unused2);
	static void APC_OnClosed(void *_player, void *unused, double unused2);
	

	/* private implementations */
	void Internal_Play();
	void Internal_Stop();
	void Internal_Reset();
	bool Internal_Load(nx_uri_t new_filename); /* returns true if Load completed properly, false if Load is still pending (will re-queue the event if necessary) */
	void Internal_RegisterForEvents(cb_playerevents *event_handler);
	void Internal_UnregisterForEvents(cb_playerevents *event_handler);
	void Internal_OnLoaded(nx_uri_t filename);
	void Internal_OnError(NError code);
	void Internal_OnStop();
	void Internal_OnEndOfFile();
	void Internal_SeekSeconds(double seconds);
	void Internal_SeekPercent(double percent);
	void Internal_Pause();
	void Internal_SetEqualizer(ifc_equalizer *_equalizer);
	void Internal_SetEQState(int _off_on);
	void Internal_SetEQPreamp(double _dB);
	void Internal_SetEQBand(unsigned int _band, double _dB);
	void Internal_OnClosed();

	PlayerState player_state;

	ifc_playback *playback; // active playback object
	ifc_metadata *metadata; // active metadata object
	ifc_equalizer *equalizer; // active equalizer object
	double length;
	bool paused;
	ifc_playback *FindPlayback(nx_uri_t filename);
	typedef nu::PtrDeque<cb_playerevents> EventList;
	EventList player_events;

	ThreadLoop thread_loop;
	nx_thread_t player_thread;
	svc_output *output;
	nx_uri_t filename;
	ifc_playback_parameters *playback_parameters;

	struct EQParameters
	{
		int on;
		float preamp;
		float bands[10];
	} eq;
};
