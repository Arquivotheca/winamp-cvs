#ifndef NULLSOFT_OUTDSH
#define NULLSOFT_OUTDSH

#include "AudioOut.h"
#include <windows.h>
#include "../../nu/AutoLock.h"

#define DSNAME "NullSoft DirectSound output " DS2_ENGINE_VER

class FORMATSPEC
{
public:
	UINT freq, nch, bps;
	FORMATSPEC(UINT f, UINT n, UINT b) {freq = f;nch = n;bps = b;}
	FORMATSPEC() {freq = 0;nch = 0;bps = 0;}
	bool operator==(FORMATSPEC & foo) { return foo.freq == freq && foo.nch == nch && foo.bps == bps;}
	bool operator!=(FORMATSPEC & foo) { return !(*this == foo);}
	FORMATSPEC & operator=(FORMATSPEC &foo) {freq = foo.freq;bps = foo.bps;nch = foo.nch; return *this;}
	UINT Size() { return nch*(bps >> 3);}
};


enum WA2_HINT
{
    HINT_NONE = 0,
    HINT_EOF,
    HINT_EOF_GAPLESS
};

class OutDS : public AudioOut
{
public:
	OutDS() : pluginState(StatusNone),
			volume(255),
			is_playing(false),
			ipcWnd(0),
			dsGuard("dsGuard"),
			isVideo(false)
	{}
	void Init();
	void Quit();
	int CanWrite();
	int GetWrittenTime();
	int IsPlaying();
	int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
	void Close();
	int Write(char *buf, int len);
	void Flush(int t);
	void SetVolume(int _volume);
	int Pause(int new_state);
	int GetOutputTime();
	void SetPan(int _pan);
	void About(HWND p);
	void Config(HWND w);

	/*----*/
	void CreateIPC();
	void DestroyIPC();
	void make_new_ds2();
	void setup_config(class DS2config * cfg);
	__int64 get_written_time();
	__int64 GetOutputTime64();
	bool wa2_GetRealtimeStat(struct DS2_REALTIME_STAT * stat);

	InitState pluginState;
	Nullsoft::Utility::LockGuard dsGuard;
	class DS2* pDS2;
	bool paused, is_playing;
	int hack_canwrite_count;
	int volume, pan;
	UINT fadetimehack;
	FORMATSPEC dataspec;
	__int64 pos_delta, samples_written;
	WA2_HINT wa2_hint;
	HWND ipcWnd;

	bool isVideo;
};

extern OutDS ds;
#endif
