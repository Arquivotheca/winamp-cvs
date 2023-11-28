#ifndef NULLSOFT_OUTWAVEH
#define NULLSOFT_OUTWAVEH

#include "../../nu/AutoLock.h"
#include "AudioOut.h"
#include "../out_wave/waveout.h"


#define OUT_WAVE_VER "2.15 (d)"
#define WAVENAME "Nullsoft WaveOut Output "OUT_WAVE_VER

class OutWave : public AudioOut
{
public:
		OutWave() 
			: pluginState(StatusNone),
			pWO(0),
			volume(255),
			pan(0),
			waveGuard(GUARDNAME("Wave Guard"))
	{}
	virtual void Init();
	virtual void Quit();
	virtual int CanWrite();
	virtual int GetWrittenTime();
	virtual int IsPlaying();
	virtual int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
	virtual void Close();
	virtual int Write(char *buf, int len);
	virtual void Flush(int t);
	virtual void SetVolume(int _volume);
	virtual int Pause(int new_state);
	virtual int GetOutputTime();
	virtual void SetPan(int _pan);
	virtual void About(HWND p);
	virtual void Config(HWND w);

	int get_written_time();
	void Reset();
	void _init();
	bool get_waveout_state(wchar_t * z, size_t cchLength);

	InitState pluginState;
	Nullsoft::Utility::LockGuard waveGuard;
	WaveOut *pWO;
	__int64 total_written;
	int pos_delta, fmt_sr,fmt_bps,fmt_nch;
	bool gapless_stop;
	UINT canwrite_hack;
	int volume, pan;
};

extern OutWave wave;

#endif