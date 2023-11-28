#pragma once
#include "nu/ThreadLoop.h"
#include "nu/PtrDeque.h"
#include "nx/nxonce.h"
#include "nx/nxstring.h"
#include "../AudioSession.h"
#include "player/svc_output.h"
#include "nx/nxthread.h"
#include "../Equalizer.h"
#include "../ifc_audiotrackpro_settings.h"
#include "BufferManager.h"
#include "nswasabi/ServiceName.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>


#define AUDIO_PREBUFFER_SECONDS	0.5f		// Default 0.5 half a second
#define AUDIO_BUFFER_FRAME_SIZE	8192
#define AUDIO_THREAD_LOOP_STEP	10	// Default 10
#define AUDIO_NUMBER_OF_BUFFERS 3 // number of buffers that the BufferQueue is told about
#define AUDIO_EXTRA_BUFFERS     2 // some extras for available/filled buffers
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
		PLAYING_PREBUFFER,
		PLAYING,
		FINISHING,
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

	/* called by the Callback function */
	int Finish();
private:
	
	void Internal_Stop();
	void Internal_Flush(size_t position);
	static void APCAddSession(void *_this, void *_session, double unused);
	static void APCFlush(void *_this, void *_position, double unused);
	static void APCPause(void *_this, void *_state, double unused);
	static void APCStop(void *_this, void *unused1, double unused2);
	static void APCDone(void *_this, void *_session, double unused);
	static void APCFinish(void *_this, void *unused1, double unused2);

	void Run();
	void RunIdle();
	void RunPrebuffer();
	void RunPlaying();
	void RunFinishing();

	int AccumulateBuffer(float *output_buffer, size_t output_samples);
	int FillBuffer(int16_t *&output_buffer, size_t &output_samples);
	int SwitchSessions(); /* an internal method with state-changing side effects. not my favorite but it simplifies some logic */
	int IsNextSessionCompatible() const;
	int IsLastSessionCompatible() const;
	void TransitionToIdle();
	void TransitionToPlaying();
	void TransitionToPrebuffer(); // transition from PLAYING back to PREBUFFER
	void DestroyPlayer();
private:
	volatile size_t audio_stop;
	LockFreeRingBuffer intermediate_buffer;
	volatile int killswitch;
	typedef nu::PtrDeque<AudioSession> SessionList;
	SessionList audio_sessions;
	AudioSession *active_session;
	ifc_audiotrackpro_settings *settings;
	ThreadLoop thread_loop;
	
	struct SLObjects
	{
		SLObjectItf object;
		SLEngineItf engine;
	} sl;

	struct PlayerObjects
	{
		SLObjectItf object;
		SLPlayItf play;
		SLAndroidSimpleBufferQueueItf buffer_queue;
		SLVolumeItf volume;
		SLAndroidConfigurationItf config;
	} player;

	struct OutputMixObjects
	{
		SLObjectItf object;
	} output_mix;
	
	bool crossfading;
	bool crossfade_tryagain;
	bool paused;
	float float_buffer[AUDIO_BUFFER_FRAME_SIZE * 2]; /* going to take advantage of the fact that max channels for android is 2 */
		
	/* thread stuff */
	nx_thread_t audio_thread;
	static nx_thread_return_t NXTHREADCALL AudioThreadFunction(nx_thread_parameter_t parameter);

	AudioState audio_state;
	Equalizer *equalizer;
	ifc_player *replicant_player;
	BufferManager buffer_manager;
	static void BufferQueueCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
	void BufferQueueCallback(SLAndroidSimpleBufferQueueItf bq);

};
