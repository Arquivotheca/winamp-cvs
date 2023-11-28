#include <windows.h>
#include "api.h"
#include "ds2.h"
#include "output/OutDS.h"

void InitStubDS();
void QuitStubDS();
int CanWriteStubDS();
int GetWrittenTimeStubDS();
int IsPlayingStubDS();
int OpenStubDS(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
void CloseStubDS();
int WriteStubDS(char *buf, int len);
void FlushStubDS(int t);
void SetVolumeStubDS(int _volume);
int PauseStubDS(int new_state);
int GetOutputTimeStubDS();
void SetPanStubDS(int _pan);
void AboutStubDS(HWND p);
void ConfigStubDS(HWND w);

// allows us to have the embedded versions correctly localised without as much resource duplication
HINSTANCE WASABI_API_LNG_HINST_DS_ORIG = 0;

Out_Module dsMod = {	OUT_VER_U,
						DSNAME,
						203968848,
						0, 0,
						ConfigStubDS,
						AboutStubDS,
						InitStubDS,
						QuitStubDS,
						OpenStubDS,
						CloseStubDS,
						WriteStubDS,
						CanWriteStubDS,
						IsPlayingStubDS,
						PauseStubDS,
						SetVolumeStubDS,
						SetPanStubDS,
						FlushStubDS,
						GetOutputTimeStubDS,
						GetWrittenTimeStubDS,
					};

void InitStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.Init();
}

void QuitStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.Quit();
}

int CanWriteStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.CanWrite();
}
int GetWrittenTimeStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.GetWrittenTime();
}
int IsPlayingStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.IsPlaying();
}

int OpenStubDS(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.Open(samplerate, numchannels, bitspersamp, bufferlenms, prebufferms);
}
void CloseStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.Close();
}

int WriteStubDS(char *buf, int len)
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.Write(buf, len);
}

void FlushStubDS(int t)
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.Flush(t);
}

void SetVolumeStubDS(int _volume)
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.SetVolume(_volume);
}

int PauseStubDS(int new_state)
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.Pause(new_state);
}

int GetOutputTimeStubDS()
{
	ds.winampWnd = dsMod.hMainWindow;
	return ds.GetOutputTime();
}

void SetPanStubDS(int _pan)
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.SetPan(_pan);
}

void AboutStubDS(HWND p)
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.About(p);
}

void ConfigStubDS(HWND w)
{
	ds.winampWnd = dsMod.hMainWindow;
	ds.Config(w);
};

extern "C" __declspec( dllexport ) Out_Module * GetDS(HINSTANCE hinst)
{
	WASABI_API_LNG_HINST_DS = WASABI_API_LNG->StartLanguageSupport((WASABI_API_LNG_HINST_DS_ORIG = hinst),OutDSLangGUID);
	static wchar_t szDS[256];
	swprintf(szDS,256,WASABI_API_LNGSTRINGW_DS(IDS_NULLSOFT_DS_OUTPUT),WIDEN(DS2_ENGINE_VER));
	dsMod.description = (char*)szDS;
	return &dsMod;
}