#include "OutDS.h"
#include <windows.h>
#include "../nu/AutoWide.h"
#include "api.h"
#include "out_ds/ds2.h"
#include "out_ds/DSConfig.h"
#include "../Winamp/out.h"
#include "out_ds/ds_ipc.h"
#include "../Winamp/wa_ipc.h"
#include "../Winamp/In2.h"
#include "../nu/AutoChar.h"
#include "WMDRMModule.h"
#include "loadini.h"
#include "util.h"

using namespace Nullsoft::Utility;
using namespace out_ds;

extern In_Module plugin;
extern Out_Module dsMod;
void WINAPI IPCTimer(HWND hwnd, UINT uMsg, UINT id,  DWORD time)
{
	KillTimer(0, id);
	AutoLock lock(ds.dsGuard LOCKNAME("IPCTimer"));
	if (plugin.hMainWindow)
	{
		ds.winampWnd = plugin.hMainWindow;
			ds.CreateIPC();
	}
	else if (ds.winampWnd)
	{
		ds.CreateIPC();
	}
	else if (dsMod.hMainWindow)
	{
		ds.winampWnd = dsMod.hMainWindow;
		ds.CreateIPC();
	}
	else
	{
		SetTimer(0, id, 100, IPCTimer);
	}
	
}

void OutDS::Init()
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::Init"));
	
	if (dllInstance == 0)
		dllInstance = GetModuleHandle(TEXT("in_wm.dll"));
	if (pluginState == StatusInit)
		return ;

	//dsDefaultConfig();
#ifdef UNICODE
	DSLoadConfig(INI_FILE);
#else
	AutoChar narrowIniFile(INI_FILE);
	DSLoadConfig(narrowIniFile);
#endif
	DS2::SetTotalTime(cfg_total_time);
	//if (pluginState == StatusInit)
		//return ;
	SetTimer(0, (UINT)this, 100, IPCTimer);
	pluginState = StatusInit;
}

void OutDS::Quit()
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::Quit"));
	if (pluginState == StatusQuit)
		return ;
	DestroyIPC();
	if (pDS2)
	{
		pDS2->Release();
		pDS2 = 0;
	}
	if (cfg_wait)
	{
		while (DS2::InstanceCount() > 0)
			Sleep(1);
	}
	cfg_total_time = DS2::GetTotalTime();
	
	DS2::Quit();
	pluginState = StatusQuit;
	DSSaveConfig();
}

int OutDS::CanWrite()
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::CanWrite"));
	int rv = 0;
	if (!paused)
	{
		if (!pDS2)
		{
			make_new_ds2();
			hack_canwrite_count = -1;
		}
		if (pDS2)
		{

			rv = pDS2->CanWrite();
			if (rv < 0)
				rv = 0;
			if (++hack_canwrite_count > 2
			    && pDS2->BufferStatusPercent() > 50)
				pDS2->ForcePlay(); //big prebuffer hack
		}
	}
	return rv;
}

int OutDS::GetWrittenTime()
{
	int rv;
	AutoLock lock (dsGuard LOCKNAME("OutDS::GetWrittenTime"));
	rv = is_playing ? (int)(pos_delta + get_written_time()) : 0;

	return rv;
}


int OutDS::IsPlaying()
{

	AutoLock lock (dsGuard LOCKNAME("OutDS::IsPlaying"));
	int rv = 0;
	if (pDS2)
	{
		int foo = cfg_fade_stop;
		pDS2->KillEndGap();
		pDS2->ForcePlay();
		int lat = pDS2->GetLatency();
		wa2_hint = HINT_EOF;
		if (foo > 0)
		{
			rv = lat > foo;
		}
		else if (lat > (int)cfg_trackhack)
		{
			rv = 1;
		}
		else
		{
			wa2_hint = HINT_EOF_GAPLESS;
			rv = 0;
		}
	}
	return rv;
}

int OutDS::Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int /*prebufferms*/)
{ //messy

	AutoLock lock (dsGuard LOCKNAME("OutDS::Open"));
	isVideo = (bufferlenms == -666);
	is_playing = 0;

	FORMATSPEC newformat(samplerate, numchannels, bitspersamp);

	DS2 * wait = 0;
	bool nofadein = pDS2 ? 1 : 0;
	bool nosetvol = nofadein;

	if (pDS2)
	{
		pDS2->SetCloseOnStop(0);
		if (pDS2->IsClosed())
		{
			pDS2->Release();pDS2 = 0;

		}
		else
		{
			if (dataspec != newformat
			    || cfg_fade_stop > 0
			    || cfg_fade_start > 0)
			{
				wait = pDS2;
				pDS2 = 0;
			}
		}
	}

	if (!pDS2)
	{
		nosetvol = 0;

		dataspec = newformat;

		DS2config cnfg;
		setup_config(&cnfg);
		pDS2 = DS2::Create(&cnfg);
		if (!pDS2)
		{

			if (wait)
				wait->Release();
			LPCTSTR moo = cnfg.GetError();
			if (moo)
			{
				lock.ManualUnlock();
				wchar_t errStr[128];
				wsprintf(errStr,WASABI_API_LNGSTRINGW_DS(IDS_ERROR),dsMod.description);
				MessageBox(winampWnd, moo, errStr, MB_ICONERROR);
				lock.ManualLock();
			}

			return -1;
		}
	}
	else
	{ //reusing old DS2

		pDS2->StartNewStream();
		pos_delta -= get_written_time();
	}

	if (!cfg_volume) volume = 255;
	pDS2->SetPan_Int(pan);
	UINT ft = 0;
	if (DS2::InstanceCount()>1)
	{
			ft=cfg_fade_start;
	}
	else
	{
			ft=cfg_fade_firststart;
	}

	if (ft && !nofadein)
	{

		pDS2->SetVolume_Int(0);
		pDS2->Fade_Int(ft, volume);
	}
	else if (!nosetvol)
	{
		pDS2->SetVolume_Int(volume);
	}

	if (wait) pDS2->WaitFor(wait, 0);

	pos_delta = 0;
	samples_written = 0;
	paused = 0;

	wa2_hint = HINT_NONE;
	hack_canwrite_count = 0;
	is_playing = 1;

	int crossfadetime = cfg_fade_stop.usedef ? cfg_def_fade : cfg_fade_stop.time;
	int buffersize = cfg_fade_stop.on ? (crossfadetime > cfg_buf_ms ? crossfadetime : cfg_buf_ms) : cfg_buf_ms;
	int rv = buffersize;
	return rv;
}

void OutDS::Close()
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::Close"));
	if (pDS2)
	{

		pDS2->KillEndGap();
		switch (wa2_hint)
		{
		case HINT_NONE:
			pDS2->FadeAndForget(cfg_fade_pause);
			pDS2 = 0;
			break;
		case HINT_EOF:
			pDS2->FadeAndForget(cfg_fade_stop);
			pDS2 = 0;
			break;
		case HINT_EOF_GAPLESS:
			if (pDS2->GetLatency() > 0)
				pDS2->SetCloseOnStop(1);
			else
			{pDS2->Release();pDS2 = 0;}
			break;
		}
	}
	is_playing = 0;
}

int OutDS::Write(char *buf, int len)
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::Write"));
	hack_canwrite_count = 0;
	wa2_hint = HINT_NONE;
	if (paused)
	{
		return 1;
	}
	if (!pDS2)
	{

		make_new_ds2();
		if (!pDS2 || !buf || !len)
		{
			return 0;
		}
	}
	samples_written += len / dataspec.Size();
	int rv = 0;
	if (buf && len > 0)
	{
		rv = !pDS2->ForceWriteData(buf, len, !isVideo); //flood warning
	}
	return rv;
}

void OutDS::Flush(int t)
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::Flush"));
	if (pDS2)
	{
		UINT t = cfg_fade_seek;
		pDS2->FadeAndForget(t);
		pDS2 = 0;
		fadetimehack = t;
	}
	samples_written = 0;
	pos_delta = t;
}


void OutDS::SetVolume(int _volume) // volume is 0-255
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::SetVolume"));
	if (_volume != -666 && cfg_volume)
	{
		volume = _volume;
		if (pDS2)
		{
			if (cfg_fadevol)
				pDS2->FadeX_Int(150, _volume);
			else
				pDS2->SetVolume_Int(_volume);
		}
	}
}


int OutDS::Pause(int new_state)
{

	AutoLock lock (dsGuard LOCKNAME("OutDS::Pause"));
	int rv = paused;
	paused = !!new_state;
	if (new_state)
	{
		if (pDS2)
		{
			UINT ft = cfg_fade_pause;
			if (!ft)
			{
				pDS2->Pause(1);
			}
			else if (cfg_oldpause)
			{
				pDS2->FadeAndForget(ft);
				pDS2 = 0;

				fadetimehack = ft;
			}
			else
			{
				pDS2->FadePause(ft);
			}
		}
	}
	else
	{
		if (pDS2) pDS2->Pause(0);
	}
	return rv;
}

int OutDS::GetOutputTime()
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::GetOutputTime"));
	int rv = (int)(pos_delta + GetOutputTime64());

	return rv;
}

void OutDS::SetPan(int _pan) // pan is -128 to 128
{
	AutoLock lock (dsGuard LOCKNAME("OutDS::SetPan"));
	if (cfg_volume)
	{
		pan = _pan;
		if (pDS2)
			pDS2->SetPan_Int(pan);
	}
}

void OutDS::About(HWND hwndParent)
{
	wchar_t message[1024], text[1024];
	WASABI_API_LNGSTRINGW_BUF_DS(IDS_NULLSOFT_DS_OUTPUT_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW_DS(IDS_ABOUT_TEXT),
			  dsMod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

void OutDS::Config(HWND w)
{
	if (WASABI_API_LNG->LDialogBoxParamW(WASABI_API_LNG_HINST_DS, WASABI_API_LNG_HINST_DS_ORIG,
										IDD_DS_CONFIG, w, CfgProc, 0))
	{
		int maxfade = cfg_fade_firststart;
		int f1 = cfg_fade_start;
		if (f1 > maxfade) maxfade = f1;
		f1 = cfg_fade_seek;
		if (f1 > maxfade) maxfade = f1;
		f1 = cfg_fade_pause;
		if (f1 > maxfade) maxfade = f1;
		if (maxfade > cfg_buf_ms)
		{
			//			cfg(cfg_buf_ms=maxfade;
			TCHAR foo[256];
			wsprintf(foo,WASABI_API_LNGSTRINGW_DS(IDS_SOME_FADE_TIMES_ARE_BIGGER_THAN_BUFFER),maxfade);
			if (MessageBox(w,foo,WASABI_API_LNGSTRINGW_DS(IDS_WARNING),MB_ICONEXCLAMATION|MB_YESNO)==IDYES)
			{
				AutoLock lock (dsGuard LOCKNAME("OutDS::Config"));
				cfg_buf_ms = maxfade;
			}
		}
	}
}

/*---------------*/

LRESULT CALLBACK ipcProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DS_IPC:
		switch (lParam)
		{
		case DS_IPC_CB_CFGREFRESH:
			// trap me !
			return 0;
		case DS_IPC_CB_ONSHUTDOWN:
			// trap me !
			return 0;
		case DS_IPC_SET_CROSSFADE:
			{
				AutoLock lock (ds.dsGuard LOCKNAME("DS_IPC_SET_CROSSFADE"));
				cfg_fade_stop.on = wParam;
				// update the config wnd if it is showing the fades page
				if (fades_config_wnd) 
				{
					HWND list = GetDlgItem(fades_config_wnd, IDC_LIST);
					int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
					FadeCfgCopy * c = (FadeCfgCopy*)SendMessage(list, LB_GETITEMDATA, 2, 0);
					c->on = wParam;
					c->usedef = cfg_fade_stop.usedef;
					c->time = cfg_fade_stop.time;
					TCHAR txt[256];
					format_fade(txt, c, 2);
					SendMessage(list, LB_DELETESTRING, 2, 0);
					SendMessage(list, LB_INSERTSTRING, 2, (long)txt);
					SendMessage(list, LB_SETITEMDATA, 2, (long)c);
					if (cursel == 2)
					{
						CheckDlgButton(fades_config_wnd, IDC_FADE_ENABLED, c->on);
						CheckDlgButton(fades_config_wnd, IDC_USE_CUSTOM_FADE, !c->usedef);
						SetDlgItemInt(fades_config_wnd, IDC_CUSTOM_FADE, c->time, 0);
					}
					SendMessage(list, LB_SETCURSEL, cursel, 0);
				}

				return 0;
			}
		case DS_IPC_SET_CROSSFADE_TIME:
			{
				AutoLock lock (ds.dsGuard LOCKNAME("DS_IPC_SET_CROSSFADE_TIME"));
				cfg_fade_stop.usedef = 0;
				cfg_fade_stop.time = wParam;
				return 0;
			}
		case DS_IPC_GET_CROSSFADE:
			return cfg_fade_stop.on;
		case DS_IPC_GET_CROSSFADE_TIME:
			if (cfg_fade_stop.usedef) return cfg_def_fade;
			return cfg_fade_stop.time;
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}




void OutDS::CreateIPC()
{
	WNDCLASS wc;
	if (!GetClassInfo(dllInstance, DS_IPC_CLASSNAME, &wc))
	{
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = ipcProc;
		wc.hInstance = dllInstance;
		wc.lpszClassName = DS_IPC_CLASSNAME;
		wc.style = 0;
		int _r = RegisterClass(&wc);
	}
	
	ipcWnd = CreateWindow(DS_IPC_CLASSNAME, TEXT(""), WS_CHILD, 0, 0, 1, 1, winampWnd, NULL, dllInstance, NULL);
	PostMessage(winampWnd, WM_WA_IPC, 0, IPC_CB_OUTPUTCHANGED);
	
}

void OutDS::DestroyIPC()
{
	if (ipcWnd)
	{
		if (IsWindow(winampWnd))
			DestroyWindow(ipcWnd); ipcWnd = NULL;
	}
}

void OutDS::make_new_ds2()
{

	DS2config cnfg;
	setup_config(&cnfg);
	pDS2 = DS2::Create(&cnfg);

	if (pDS2)
	{
		pDS2->SetPan_Int(pan);
		pDS2->SetVolume_Int(0);
		pDS2->Fade_Int(fadetimehack, volume);
		fadetimehack = 0;
	}
}

void OutDS::setup_config(DS2config * cnfg)
{
	cnfg->SetPCM(dataspec.freq, dataspec.bps, dataspec.nch);

	cnfg->SetCreatePrimary(!!cfg_createprimary);
	cnfg->SetWindow(winampWnd);
	cnfg->SetDeviceGUID(cfg_dev2);
	int crossfadetime = cfg_fade_stop.usedef ? cfg_def_fade : cfg_fade_stop.time;
	int buffersize = cfg_fade_stop.on ? (crossfadetime > cfg_buf_ms) ? crossfadetime : cfg_buf_ms: cfg_buf_ms;
	cnfg->SetBuffer(buffersize, cfg_prebuf2);
	if (cfg_killsil) cnfg->SetSilence((float)cfg_sil_db*(float)0.1);
	cnfg->SetVolMode(cfg_volmode, cfg_logvol_min, !!cfg_logfades);
	cnfg->SetMixing(cfg_hw_mix? 0 : 2);
	if (cfg_override)
	{
		cnfg->SetPrimaryOverride(1);
		cnfg->SetPrimaryOverrideFormat(cfg_override_freq, cfg_override_bps, cfg_override_nch);
	}
	cnfg->SetCpuManagement(!!cfg_autocpu);
	cnfg->SetRefresh(cfg_refresh);
	//cnfg->SetCoop(0);
}

__int64 OutDS::get_written_time()
{
	return dataspec.freq ? samples_written*1000 / (__int64)dataspec.freq : 0;
}

__int64 OutDS::GetOutputTime64()
{
	if (!is_playing) return 0;
	__int64 rv = get_written_time();
	if (pDS2) rv -= pDS2->GetLatency();
	if (rv < 0) rv = 0;
	return rv;
}

bool OutDS::wa2_GetRealtimeStat(DS2_REALTIME_STAT * stat) //for config
{
	bool rv = 0;
	AutoLock lock (dsGuard LOCKNAME("wa2_GetRealtimeStat"));
	if (pDS2 && !pDS2->IsClosed())
	{
		rv = 1;
		pDS2->GetRealtimeStat(stat);
	}

	return rv;
}

OutDS ds;
