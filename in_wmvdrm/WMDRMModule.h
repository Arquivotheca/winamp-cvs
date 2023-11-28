#ifndef NULLSOFT_WMDRMMODULEH
#define NULLSOFT_WMDRMMODULEH

#include "Remaining.h"
#include <wmsdk.h>
// layers
#include "AudioLayer.h"
#include "VideoLayer.h"
#include "ClockLayer.h"
#include "DRMLayer.h"
#include "WaitLayer.h"
#include "BufferLayer.h"
#include "SeekLayer.h"
#include "GainLayer.h"

#include "WMHandler.h"
#include "WMCallback.h"
#include "WMInformation.h"

class WMDRM : public WMHandler
{
public:
	WMDRM();
	~WMDRM();
	void Config(HWND hwndParent);
	void Init();
	void Quit();
	void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms);
	int InfoBox(const wchar_t *file, HWND hwndParent);
	int IsOurFile(const wchar_t *fn);
	int Play(const wchar_t *fn);
	void Pause();
	void UnPause();
	int IsPaused();
	void Stop();
	int GetLength();
	int GetOutputTime();
	void SetOutputTime(int time_in_ms);
	void SetVolume(int volume);
	void SetPan(int pan);
	void EQSet(int on, char data[10], int preamp);
/*
	bool IsDSPSecure()
	{
			int x;
			int t = 37 + ((((int) & mod) >> 8) & 0x0f);
			for (x = 0; x < t; x ++)
			{
				int v = GetTickCount();
				if (mod->dsp_dosamples(0, 0, 0, 0xdeadbeef, v) != (((((v + 66191213)*1103515245) + 13293)&0x7FFFFFFF) ^ t))
					break;
			}
			dsp_allowed = (x == t);

		
		return dsp_allowed;
	}*/
	
	void BuildBuffers();
	void OutputAudioSamples(void *data,  long samples, DWORD&);
	void OutputAudioSamples(void *data,  long samples);
	void QuantizedViz(void *data,  long sizeBytes, DWORD);
	long GetPosition();
	void EndOfFile();
	bool OpenVideo(int fourcc, int width, int height, bool flipped);
	void ReOpen();
	void NewSourceFlags();
//	const char *GetFile() { return fn.c_str();}
	bool playing;
	int startAtMilliseconds;
	void InitWM();
protected:
	//WMHandler
	void AudioDataReceived(void *_data, unsigned long sizeBytes, DWORD timestamp);
	void Opened();
	void NewMetadata();
	void Closed();
	void Started();
	void Error();
	void OpenFailed();
	void Stopped();
	void Kill();

	BufferLayer *buffer;
	ClockLayer *clock;
#ifndef NO_DRM
	DRMLayer *drm;
#endif
	AudioLayer *audio;
	VideoLayer *video;
	WaitLayer *wait;
	SeekLayer *seek;
	GainLayer *gain;
	WMCallback callback;
	IWMReader *reader;
	IWMReaderAdvanced *reader1;
	IWMReaderAdvanced2 *reader2;
	IWMReaderNetworkConfig *network;
	WMInformation *info;
	Remaining remaining;
	unsigned char *dspBuffer, *vizBuffer;
	int volume, pan;
	bool flushed, paused;

	HANDLE killswitch;

	bool opened;
	bool drmProtected;
	void Connecting();
	void Locating();
	void AssignOutput();	
	void AccessDenied();

	
};
#endif
