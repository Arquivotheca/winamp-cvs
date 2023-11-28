#pragma once
#include "nu/ThreadLoop.h"
#include "nu/PtrDeque.h"
#include "nx/nxonce.h"
#include "nx/nxstring.h"
#include "AudioSession.h"
#include "../player/svc_output.h"
#include "nx/nxthread.h"
#include "../Equalizer.h"
#include "../ifc_audiotrackpro_settings.h"
#include "nswasabi/ServiceName.h"

#ifdef REPLICANT_AUDIOTRACK9
#include <media/AudioTrack9.h>
#else
#include <media/AudioTrack.h>
#endif


#define AUDIO_PREBUFFER_SECONDS				0.5f		// Default 0.5 half a second
#define AUDIOTRACK_BUFFER_SECONDS			0.5f		// Default 0.25f... Quarter second
#define AUDIOTRACK_BUFFER_FRAME_SIZE		1024		// Default 1024
#define AUDIOTRACK_BUFFER_TIMEOUT			128			// Default 128
#define AUDIOTRACK_BUFFER_THREAD_LOOP_STEP	10	// Default 10

// {3159C24C-7695-4685-AE6F-7AADC7A86C8E}
static const GUID android_audiotrack_guid = 
{ 0x3159c24c, 0x7695, 0x4685, { 0xae, 0x6f, 0x7a, 0xad, 0xc7, 0xa8, 0x6c, 0x8e } };


class AudioThread : public svc_output
{
private:
	enum AudioState
	{
		IDLE,
		PREBUFFER,
		PLAYING,
		TRANSITION,
		STOPPING,
	};

public:
	WASABI_SERVICE_NAME("Android Output Service");
	static GUID GetServiceGUID() { return android_audiotrack_guid; }

	AudioThread();
	~AudioThread();
	int OutputService_AudioOpen(const ifc_audioout::Parameters *format, ifc_player *player, ifc_playback_parameters *secondary_parameters, ifc_audioout **out_output);
	
	/* called by audio session */
	int Flush(size_t position);
	int Pause(int state);
	int Stop();
	int Done(AudioSession *session);
private:
	
	void Internal_Stop();
	void Internal_Flush(size_t position);
	static void APCAddSession(void *_this, void *_session, double unused);
	static void APCFlush(void *_this, void *_position, double unused);
	static void APCPause(void *_this, void *_state, double unused);
	static void APCStop(void *_this, void *unused1, double unused2);
	static void APCDone(void *_this, void *_session, double unused);

	void Run();
	void RunIdle();
	void RunPrebuffer();
	void RunPlaying();

	int AccumulateBuffer(float *output_buffer, size_t output_samples);
	int FillBuffer(int16_t *&output_buffer, size_t &output_samples);
	int SwitchSessions(); /* an internal method with state-changing side effects. not my favorite but it simplifies some logic */
	int IsNextSessionCompatible() const;
	int IsLastSessionCompatible() const;
private:
	volatile int killswitch;
	typedef nu::PtrDeque<AudioSession> SessionList;
	SessionList audio_sessions;
	AudioSession *active_session;
	ifc_audiotrackpro_settings *settings;
	ThreadLoop thread_loop;
	android::AudioTrack *audio_track;
	bool started;
	bool crossfading;
	bool crossfade_tryagain;
	bool paused;
	float float_buffer[AUDIOTRACK_BUFFER_FRAME_SIZE * 2]; /* going to take advantage of the fact that max channels for android is 2 */
		
	/* thread stuff */
	nx_thread_t audio_thread;
	static nx_thread_return_t NXTHREADCALL AudioThreadFunction(nx_thread_parameter_t parameter);

	AudioState audio_state;
	Equalizer *equalizer;
	ifc_player *player;	
};
