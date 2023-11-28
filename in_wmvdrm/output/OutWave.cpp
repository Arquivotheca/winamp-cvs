#include "OutWave.h"
#include "out_wave/WaveConfig.h"
#include "res_wav/resource.h"
#include "../Winamp/wa_ipc.h"
#include "../Winamp/In2.h"
#include "../nu/Autowide.h"
#include "api.h"
#include <assert.h>
#include "loadini.h"
#include "util.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

extern Out_Module waveMod;
OutWave wave;
using namespace Nullsoft::Utility;
using namespace out_wave;


void OutWave::Init()
{
	AutoLock lock (waveGuard);

	if (dllInstance == 0)
		dllInstance = GetModuleHandle(L"in_wm.dll");

	if (pluginState == StatusInit)
		return ;
	
	assert(INI_FILE[0]);

	SetDefaults();
	LoadConfig(INI_FILE);

	if (cfg_dev > waveOutGetNumDevs())
		cfg_dev = 0;

	pluginState = StatusInit;
}

void OutWave::Quit()
{
	AutoLock lock (waveGuard);
	if (pluginState == StatusQuit)
		return ;

	if (pWO)
	{
		delete pWO;
		pWO = 0;
	}

	SaveConfig();

	pluginState = StatusQuit;
}

int OutWave::Pause(int new_state)
{
	AutoLock lock (waveGuard);
	int rv;

	if (pWO)
	{
		rv = pWO->IsPaused();
		pWO->Pause(new_state);
	}
	else
		rv = 0;

	return rv;
}

void OutWave::About(HWND hwndParent)
{
	wchar_t message[1024], text[1024];
	WASABI_API_LNGSTRINGW_BUF_WAV(IDS_NULLSOFT_WAVEOUT_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW_WAV(IDS_ABOUT_TEXT),
			  waveMod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

int OutWave::get_written_time() //this ignores high 32bits of total_written
{
	return MulDiv(static_cast<int>(total_written), 1000, fmt_sr);
}

int OutWave::GetWrittenTime()
{
	AutoLock lock (waveGuard);
	int r;
	r = pWO ? pos_delta + get_written_time() : 0;
	return r;
}

void OutWave::Close()
{
	AutoLock lock (waveGuard);

	if (!pWO)
		return ;

	if (gapless_stop) //end-of-song stop, dont close yet, use gapless hacks
	{
		pWO->SetCloseOnStop(1);	//will self-destruct when out of PCM data to play, has no more than 200ms in buffer
	}
	else	//regular stop (user action)
	{
		delete pWO;
		pWO = 0;
	}
}

int OutWave::CanWrite()
{
	AutoLock lock (waveGuard);

	int r;

	if (!pWO)
		return 0;

	r = pWO->CanWrite();

	if (++canwrite_hack > 2)
		pWO->ForcePlay(); //avoid constant-small-canwrite-while-still-prebuffering snafu

	return r;
}

void OutWave::Flush(int pos)
{
	AutoLock lock (waveGuard);

	if (pWO)
		pWO->Flush();

	Reset();
	pos_delta = pos;
}

void OutWave::Reset()
{
	canwrite_hack = 0;
	pos_delta = 0;
	total_written = 0;
	gapless_stop = 0;
}

void OutWave::_init()
{
	if (pluginState == StatusInit)
		return ;

	assert(INI_FILE[0]);	
	
	SetDefaults();
	LoadConfig(INI_FILE);
	if (cfg_dev > waveOutGetNumDevs())
		cfg_dev = 0;
	pluginState = StatusInit;
}

int OutWave::Open(int sr, int nch, int bps, int bufferlenms, int prebufferms)
{
	AutoLock lock (waveGuard);
	_init();

	if (pWO)	//"gapless" track change (or someone forgot to close output)
	{
		pWO->SetCloseOnStop(0);	//turn off self-destruct on out-of-PCM-data
		if (!pWO->IsClosed())	//has it run out of PCM data or not ? if yes, we can only delete and create new one
		{
			if (sr != fmt_sr || nch != fmt_nch || bps != fmt_bps) //new pcm format
			{ //wait-then-close, dont cut previous track
				while (pWO->GetLatency() > 0) Sleep(1);
			}
			else
			{ //successful gapless track change. yay.
				Reset();

				int r = pWO->GetMaxLatency();

				return r;
			}
		}

		delete pWO;
	}

	WaveOutConfig cfg;
	cfg.SetBuffer(cfg_buf_ms, cfg_prebuf);
	cfg.SetDevice(cfg_dev);
	cfg.SetVolumeSetup(cfg_volume, cfg_altvol, cfg_resetvol);

	fmt_sr = sr;
	fmt_nch = nch;
	fmt_bps = bps;

	cfg.SetPCM(sr, nch, bps);

	pWO = WaveOut::Create(&cfg);
	if (!pWO)
	{
		const wchar_t *error = cfg.GetError();
		if (error)
		{
			lock.ManualUnlock();
			char err[128];
			wsprintfA(err,WASABI_API_LNGSTRING_WAV(IDS_ERROR),waveMod.description);
			MessageBox(winampWnd, error, AutoWide(err), MB_ICONERROR);
			lock.ManualLock();
		}
	}
	else
		Reset();

	if (pWO)
		return pWO->GetMaxLatency();
	else
		return -1;
}

int OutWave::Write(char *data, int size)
{
	AutoLock lock (waveGuard);
	gapless_stop = 0;
	canwrite_hack = 0;

	if (!pWO)
		return 0;

	pWO->WriteData(data, size);
	total_written += size / ((fmt_bps / 8) * fmt_nch);
	return 0;
}

int OutWave::IsPlaying()
{ //this is called only when decoding is done unless some input plugin dev is really nuts about making useless calls
	AutoLock lock (waveGuard);

	if (!pWO)
		return 0;

	pWO->ForcePlay(); //evil short files: make sure that output has started
	if (pWO->GetLatency() > cfg_trackhack) //cfg_trackhack used to be 200ms constant
	{	//just for the case some input plugin dev is actually nuts about making useless calls or user presses stop/prev/next when decoding is finished, we don't activate gapless_stop here
		gapless_stop = 0;
		return 1;
	}
	else
	{	//ok so looks like we're really near the end-of-track, time to do gapless track switch mumbo-jumbo
		gapless_stop = 1;
		return 0; //hack: make the input plugin think that we're done with current track
	}

}

void OutWave::SetVolume(int v)
{
	AutoLock lock (waveGuard);

	if (v != -666)
		volume = v;
	if (pWO)
		pWO->SetVolume(volume);
}


void OutWave::SetPan(int p)
{
	AutoLock lock (waveGuard);
	pan = p;
	if (pWO)
		pWO->SetPan(pan);
}

int OutWave::GetOutputTime()
{
	AutoLock lock (waveGuard);

	return pWO ? (pos_delta + get_written_time()) - pWO->GetLatency() : 0;
}

void OutWave::Config(HWND w)
{
	_init();
	WASABI_API_LNG->LDialogBoxParamW(WASABI_API_LNG_HINST_WAV, WASABI_API_LNG_HINST_WAV_ORIG,
									 IDD_WAVE_CONFIG, w, WaveCfgProc, 0);
}

bool OutWave::get_waveout_state(wchar_t * z, size_t cchLength)
{
	if (pWO) return pWO->PrintState(z, cchLength);
	else return 0;
}