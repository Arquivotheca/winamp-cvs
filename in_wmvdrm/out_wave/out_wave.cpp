#include "../output/OutWave.h"
#include "../Winamp/in2.h"
#include "res_wav/resource.h"
#include "api.h"

void InitStubWave();
void QuitStubWave();
int CanWriteStubWave();
int GetWrittenTimeStubWave();
int IsPlayingStubWave();
int OpenStubWave(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
void CloseStubWave();
int WriteStubWave(char *buf, int len);
void FlushStubWave(int t);
void SetVolumeStubWave(int _volume);
int PauseStubWave(int new_state);
int GetOutputTimeStubWave();
void SetPanStubWave(int _pan);
void AboutStubWave(HWND p);
void ConfigStubWave(HWND w);

// allows us to have the embedded versions correctly localised without as much resource duplication
HINSTANCE WASABI_API_LNG_HINST_WAV_ORIG = 0;
extern In_Module plugin;

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

Out_Module waveMod = {	OUT_VER_U,
						WAVENAME,
						1471482036,
						0, 0,
						ConfigStubWave,
						AboutStubWave,
						InitStubWave,
						QuitStubWave,
						OpenStubWave,
						CloseStubWave,
						WriteStubWave,
						CanWriteStubWave,
						IsPlayingStubWave,
						PauseStubWave,
						SetVolumeStubWave,
						SetPanStubWave,
						FlushStubWave,
						GetOutputTimeStubWave,
						GetWrittenTimeStubWave,
					};

void InitStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.Init();
}

void QuitStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.Quit();
}

int CanWriteStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.CanWrite();
}
int GetWrittenTimeStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.GetWrittenTime();
}
int IsPlayingStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.IsPlaying();
}

int OpenStubWave(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.Open(samplerate, numchannels, bitspersamp, bufferlenms, prebufferms);
}

void CloseStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.Close();
}

int WriteStubWave(char *buf, int len)
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.Write(buf, len);
}

void FlushStubWave(int t)
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.Flush(t);
}

void SetVolumeStubWave(int _volume)
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.SetVolume(_volume);
}

int PauseStubWave(int new_state)
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.Pause(new_state);
}

int GetOutputTimeStubWave()
{
	wave.winampWnd = waveMod.hMainWindow;
	return wave.GetOutputTime();
}

void SetPanStubWave(int _pan)
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.SetPan(_pan);
}

void AboutStubWave(HWND p)
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.About(p);
}

void ConfigStubWave(HWND w)
{
	wave.winampWnd = waveMod.hMainWindow;
	wave.Config(w);
};

extern "C" __declspec( dllexport ) Out_Module * GetWave(HINSTANCE hinst)
{
	WASABI_API_LNG_HINST_WAV = WASABI_API_LNG->StartLanguageSupport((WASABI_API_LNG_HINST_WAV_ORIG = hinst),OutWaveLangGUID);
	static wchar_t szWave[256];
	swprintf(szWave,256,WASABI_API_LNGSTRINGW_WAV(IDS_NULLSOFT_WAVEOUT),WIDEN(OUT_WAVE_VER));
	waveMod.description = (char*)szWave;
	return &waveMod;
}