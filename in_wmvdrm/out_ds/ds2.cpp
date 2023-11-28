#include "api.h"
#include "ds2.h"
#include <dsound.h>
#include <math.h>
#include <ks.h>
#include <ksmedia.h>
#include "wa2.h"
#include "../winamp/wa_ipc.h"
using namespace Nullsoft::Utility;
DS2::tDirectSoundCreate DS2::pDirectSoundCreate = 0;
DS2::tDirectSoundEnumerate DS2::pDirectSoundEnumerate = 0;

static UINT refresh_timer = 10;

static const GUID NULL_GUID;

HRESULT DS2::myDirectSoundCreate(const GUID * g, IDirectSound ** out)
{
	HRESULT r;
	try
	{
		r = DS2::pDirectSoundCreate( (!g || *g == NULL_GUID) ? (const GUID*)0 : g, out, 0);
	}
	catch (...)
	{
		*out = 0;
		r = DSERR_GENERIC;
	}
	return r;
}

#define ftest(X) (!!(flags & FLAG_##X))
#define fset(X) flags|=FLAG_##X
#define funset(X) flags&=~FLAG_##X
#define fsetc(X,Y) {if (Y) fset(X); else funset(X);}

static HINSTANCE hdsound;

static bool g_delayed_deinit = 1;

static __int64 g_total_time;

static HANDLE g_hEvent;
static bool g_quitting, g_quitting_waiting;


static DWORD last_rel_time;
static DWORD coop_mode;
IDirectSound * DS2::pDS = 0;
static IDirectSoundBuffer * pPrimary;
static UINT prim_bps, prim_nch, prim_sr;
static GUID cur_dev;
static DS2* ds2s;
static HANDLE g_hThread;
static bool create_primary = 0;

static int calc_silence(float _db, int bps) //_db is in -0.1db
{
	return (int) (pow(10.0, (double)_db / ( -20.0)) * pow(2.0, (double)bps));
}

void DS2::test_silence(char * buf, int len, int * first, int* last)
{
	int bps = fmt_bps >> 3;
	if (bps > 4 || bps < 1)
	{
		if (first) *first = 0;
		if (last) *last = (len - fmt_nch * bps);
		return ;
	}

	int ptr = 0;

	while (ptr < len)
	{
		int p = ptr;
		UINT n;
		for (n = 0;n < fmt_nch;n++)
		{
			int s;
			void * _p = buf + p;
			switch (bps)
			{
			case 1:
				s = (UINT) * (BYTE*)_p - 0x80;
				break;
			case 2:
				s = *(short*)_p;
				break;
			case 3:
				{
					long poo = 0;
					memcpy(&poo, _p, 3);
					if (poo&0x800000) poo |= 0xFF000000;
					s = poo;
				}
				break;
			case 4:
				s = *(long*)_p;
				break;

			}
			if (s < 0) s = -s;
			if (s > silence_delta)
			{
				if (first && *first < 0) *first = ptr;
				if (last) *last = ptr;
			}
			p += bps;
		}
		ptr = p;
	}
}

DS2::DS2(DS2config * cfg) : BlockList(cfg->bps == 8 ? 0x80 : 0)
		, VolCtrl(cfg->volmode, cfg->logvol_min, cfg->logfades)
{
#ifdef _DEBUG
	srand(GetTickCount());
	serial = rand();
	sync_n = 0;
#endif
	next = ds2s;
	ds2s = this;
	wait = 0;
	flags = 0;

	LockCount = 0;
	Underruns = 0;

	fsetc(USE_CPU_MNGMNT, cfg->use_cpu_management);
	refresh_timer = cfg->refresh;
	if (refresh_timer < 1) refresh_timer = 1;
	else if (refresh_timer > 100) refresh_timer = 100;

	pDSB = 0;
	myDS = 0;
}

DS2::~DS2()
{
	AutoLock lock (ds2Guard LOCKNAME("~DS2"));

	ds_kill();
}

DS2* DS2::Create(DS2config * cfg)
{
	Init();
	if (!hdsound) return 0;
	AutoLock lock (ds2Guard LOCKNAME("DS2::Create"));
	DS2 * r = new DS2(cfg);
	if (!r->Open(cfg))
	{
		delete r;
		r = 0;
	}
	else SetEvent(g_hEvent); //wake update thread up

	return r;
}

void DS2::ds_kill()
{
	if (wait)
	{
		delete wait;
		wait = 0;
	}
	if (pDSB)
	{
		if (ftest(PLAYING) && !ftest(PAUSED)) pDSB->Stop();
		pDSB->Release();
		pDSB = 0;
		last_rel_time = GetTickCount();
	}
	if (myDS)
	{
		myDS->Release();
		myDS = 0;
	}
	do_reset_vars();
	//UGLY moved from destructor
	DS2 * foo = ds2s;
	DS2 ** foo2 = &ds2s;
	while (foo)
	{
		if (foo == this) {*foo2 = next;break;}
		foo2 = &foo->next;foo = *foo2;
	}
}

int DS2::WriteData(void * _data, UINT size, bool doSilence, bool *killswitch)
{ //note: calling code may or may not care about CanWrite() (but if they do, we wont sleep)
	if (ftest(PAUSED)) return 0;
	char * data = (char*)_data;
	size = _align_var(size); //avoid evil shit
	AutoLock lock (ds2Guard LOCKNAME("DS2::WriteData"));
	if (!doSilence)
		silence_delta=0;
	if (silence_delta >= 0) //no need to sync this
	{
		if (ftest(STARTSIL))
		{
			int first = -1;
			test_silence(data, size, &first, 0);
			if (first >= 0)
			{
				size -= first;
				data += first;
				funset(STARTSIL);
			}
			else
			{
				return 1;
			}
		}

		int last = -1;
		test_silence(data, size, 0, &last);
		if (last != -1)
		{
			last_nonsil = last + data_written + BlockList.DataSize();
		}
	}
	BlockList.AddBlock(data, size);
	if (data_buffered < clear_size)
		SetEvent(g_hEvent);
	else
	{
		while (!*killswitch && CanWrite() < 0)
		{
			lock.ManualUnlock();
			Sleep(1);
			lock.ManualLock(MANUALLOCKNAME("DS2::WriteData (manual lock)"));
		}
	}

	return 1;
}
/*
int DS2::WriteDataNow(void * data, UINT size)
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::WriteDataNow"));
	int cw = CanWrite();
	int rv = 0;
	if (cw > 0)
	{
		if (size > (UINT)cw) size = (UINT)cw;
		if (ForceWriteData(data, size)) rv = size;
	}

	return rv;
}
*/
int DS2::ForceWriteData(void * data, UINT size, bool doSilence)
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::ForceWriteData"));

	bool killswitch = 1;
	int r = WriteData(data, size, doSilence, &killswitch);

	return r;
}

DWORD WINAPI DS2::ThreadFunc(void* zzz)
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::ThreadFunc"));
	while (1)
	{
		DS2* foo = ds2s;
		while (foo)
		{
			foo->flags &= ~FLAG_UPDATED;
			foo = foo->next;
		}
		foo = ds2s;
		while (foo)
		{
			if (!(foo->flags&FLAG_UPDATED) && foo->Update())
			{ //one *or more* of instances got deleted
				foo = ds2s;
			}
			else
			{
				foo->flags |= FLAG_UPDATED;
				foo = foo->next;
			}
		}
		DWORD t = ds2s ? refresh_timer : (pDS ? 1000 : -1);
		lock.ManualUnlock();
		WaitForSingleObject(g_hEvent, t);
		//use g_hEvent to wake thread up when something's going on
		lock.ManualLock(MANUALLOCKNAME("DS2::ThreadFunc (manual lock)"));
		if (g_quitting) break;

		if (!ds2s && pDS)
		{
			if (pPrimary) {pPrimary->Release();pPrimary = 0;}
			if (!g_delayed_deinit || GetTickCount() - last_rel_time > 3000)
			{
				pDS->Release();
				pDS = 0;
			}
		} 
	}
	while (ds2s) delete ds2s;
if (pPrimary) {pPrimary->Release();pPrimary = 0;}
	if (pDS) {pDS->Release();pDS = 0;}

	return 0;
}

//static void __cdecl __quit() {DS2::Quit(0);}

bool DS2::InitDLL()
{
	if (!hdsound)
	{
		hdsound = LoadLibrary(TEXT("dsound.dll"));
		if (!hdsound) return 0; //ouch
		pDirectSoundCreate = (tDirectSoundCreate)GetProcAddress((HMODULE)hdsound, "DirectSoundCreate");
	if (!pDirectSoundCreate) {FreeLibrary(hdsound);hdsound = 0; return 0;}
		pDirectSoundEnumerate = (tDirectSoundEnumerate)GetProcAddress((HMODULE)hdsound, "DirectSoundEnumerateW");
		if (!pDirectSoundEnumerate) {pDirectSoundCreate = 0;FreeLibrary(hdsound);hdsound = 0; return 0;}

	}
	return 1;
}

void DS2::Init()
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::Init"));
	InitDLL();
	if (g_hThread || !hdsound) return ;

	pDS = 0;
	ds2s = 0;

	g_quitting = 0;
	g_quitting_waiting = 0;
	g_hEvent = CreateEvent(0, 0, 0, 0);

	DWORD id;
	g_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, 0, 0, &id);
	SetThreadPriority(g_hThread, THREAD_PRIORITY_TIME_CRITICAL);
}

void DS2::Quit(bool wait)
{
	if (!g_hThread) return ;
	g_quitting_waiting = 1;
	if (wait) while (ds2s) Sleep(3);
	g_quitting = 1;
	SetEvent(g_hEvent);
	WaitForSingleObject(g_hThread, INFINITE);
	CloseHandle(g_hThread);
	g_hThread = 0;
	CloseHandle(g_hEvent);
	g_hEvent = 0;

	if (hdsound)
	{
		FreeLibrary(hdsound);
		pDirectSoundCreate = 0;

		pDirectSoundEnumerate = 0;

		hdsound = 0;
	}
}

void DS2::ds_stop()
{

	if (ftest(PLAYING))
	{
		if (pDSB)
		{
			pDSB->Stop();
			pDSB->SetCurrentPosition(0);
		}
	}
	do_reset_vars();
}

void DS2::update_pos() //AKA update P.O.S.
{
	//called from Update(), no need for shit condition tests
	DWORD play_pos, play_pos_w;

	try
	{
		pDSB->GetCurrentPosition(&play_pos, &play_pos_w);
	}
	catch (...)
	{
		return ;
	}

	UINT write_pos = (UINT)(data_written % buf_size);

	data_buffered = write_pos > play_pos ? write_pos - play_pos : write_pos + buf_size - play_pos;

	VolCtrl.SetTime(GetCurPos());
	VolCtrl.Apply(pDSB);
}

bool DS2::Update() //inside sync already
{
	if (g_quitting_waiting && (!ftest(PLAYING) || ftest(PAUSED) || !pDSB))
	{
		delete this;
		return 1;
	}
	if (!pDSB || ftest(PAUSED))
	{
		return 0;
	}

	{
		UINT min_refresh = bytes2ms(clear_size) >> 1;
		if (refresh_timer > min_refresh) refresh_timer = min_refresh;
	}

	if (ftest(PLAYING)) update_pos();

	if (!ftest(PLAYING) && data_written + BlockList.DataSize() >= (int)prebuf && !wait)
	{
		fset(NEED_PLAY_NOW);
	}

	DoLock();

	if (wait)
	{

		if (wait->GetLatency() <= waitfade)
		{
			wait->FadeAndForget(waitfade);
			wait = 0;
			if (!ftest(PLAYING)) fset(NEED_PLAY_NOW);
		}
	}

	if (ftest(NEED_PLAY_NOW) && data_buffered > 0 /* && !(ftest(PLAYING)*/)
	{
		if (!ftest(PAUSED) )
		{
			HRESULT res = pDSB->Play(0, 0, DSBPLAY_LOOPING);

			if (FAILED(res))
			{
				if (res == DSERR_BUFFERLOST) pDSB->Restore();
				return 0;
			}
			pos_delta = GetOutputTime();pos_delta2 = data_written;
		}
		PostMessage(dsMod.hMainWindow, WM_WA_IPC, 0, IPC_OUTPUT_STARTED);
		fset(PLAYING);
	}
	funset(NEED_PLAY_NOW);
	if (ftest(PLAYING))
	{
		{
			DWORD foo = 0;
			pDSB->GetStatus(&foo);
			if (foo&DSBSTATUS_BUFFERLOST)
				pDSB->Restore();
			if (foo != (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING))
				pDSB->Play(0, 0, DSBPLAY_LOOPING);
		}


		if (!VolCtrl.Fading())
		{
			if (ftest(FADEPAUSING)) {Pause(1);funset(FADEPAUSING);}
			if (ftest(DIE_ON_STOP) || g_quitting_waiting)
			{
				delete this;
				return 1;
			}
		}

		if (data_buffered <= silence_buffered)
		{
			ds_stop();
			if (ftest(DIE_ON_STOP) || g_quitting_waiting)
			{
				delete this;
				return 1;
			}
			else if (ftest(CLOSE_ON_STOP))
			{
				ds_kill();
			}

			else if (ftest(FADEPAUSING)) {Pause(1);funset(FADEPAUSING);}
			else Underruns++;
		}

	}
	return 0;
}

int DS2::Open(DS2config * cfg)
{
	//	SYNCFUNC;	//inside sync already
	HRESULT hr;

	g_delayed_deinit = cfg->delayed_shutdown;
	if (cfg->sil_db > 0) {silence_delta = calc_silence(cfg->sil_db, cfg->bps);fset(STARTSIL);}
	else silence_delta = -1;

	create_primary = cfg->create_primary;

	UINT _p_bps = 0, _p_nch = 0, _p_sr = 0;

	if (cfg->prim_override)
	{
		_p_bps = cfg->_p_bps;
		_p_nch = cfg->_p_nch;
		_p_sr = cfg->_p_sr;
	}

	if (cfg->guid != cur_dev && pDS)
	{
		pDS->Release();
		pDS = 0;
	}

	if (!pDS)
	{
		hr = 0;
		cur_dev = cfg->guid;
		hr = myDirectSoundCreate(&cur_dev, &pDS);

		if (!pDS)
		{
			cfg->SetErrorCodeMsg(WASABI_API_LNGSTRINGW_DS(DsDevEnumGuid(cur_dev) ? IDS_BAD_DS_DRIVER : IDS_DEVICE_NOT_FOUND_SELECT_ANOTHER),hr);
			return 0;
		}
		coop_mode = 0;
	}
	fmt_sr = cfg->sr;
	fmt_nch = cfg->nch;
	fmt_bps = cfg->bps;
	if ((signed)fmt_sr <= 0 || (signed)fmt_bps <= 0 || (signed)fmt_nch <= 0) return 0;
	fmt_mul = fmt_sr * (fmt_bps >> 3) * fmt_nch;

	if (!_p_bps) _p_bps = fmt_bps;
	if (!_p_nch) _p_nch = fmt_nch;
	if (!_p_sr) _p_sr = fmt_sr;


	WAVEFORMATEX wfx =
	    {
	        WAVE_FORMAT_PCM,
	        fmt_nch,
	        fmt_sr,
	        fmt_mul,
	        fmt_nch * (fmt_bps >> 3),
	        fmt_bps,
	        0
	    };

	{
		static DWORD coop_tab[3] = {DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_EXCLUSIVE};

		DWORD new_coop = coop_tab[cfg->coop];
		if (pPrimary && !create_primary)
		{
			pPrimary->Release();
			pPrimary = 0;
		}
		if (coop_mode != new_coop)
		{
			if (FAILED(hr = pDS->SetCooperativeLevel(cfg->wnd, coop_mode = new_coop)))
			{
				pDS->Release();
				pDS = 0;
				cfg->SetErrorCodeMsg(WASABI_API_LNGSTRINGW_DS(IDS_ERROR_SETTING_DS_COOPERATIVE_LEVEL), hr);
				return 0;
			}
		}
		if (create_primary && !pPrimary)
		{

			DSBUFFERDESC desc = {	sizeof(DSBUFFERDESC),
			                      DSBCAPS_PRIMARYBUFFER,
			                      0,
			                      0,
			                      0
			                    };

			pDS->CreateSoundBuffer(&desc, &pPrimary, 0);
			prim_nch = prim_bps = prim_sr = 0;
		}
		if (pPrimary && (_p_bps != prim_bps || _p_nch != prim_nch || _p_sr != prim_sr))
		{
			WAVEFORMATEX wfx1 = {	WAVE_FORMAT_PCM,
			                      _p_nch,
			                      _p_sr,
			                      _p_sr * (_p_bps >> 3) * _p_nch,
			                      _p_nch * (_p_bps >> 3),
			                      _p_bps,
			                      0
			                    };
			pPrimary->SetFormat(&wfx1);
			prim_bps = _p_bps;
			prim_nch = _p_nch;
			prim_sr = _p_sr;
		}
	}

	UINT new_buf_ms = cfg->ms;

	if (new_buf_ms < 100) new_buf_ms = 100; // <= DO NOT TOUCH
	else if (new_buf_ms > 100000) new_buf_ms = 100000;

	buf_size = _align_var(ms2bytes(new_buf_ms));

	prebuf = ms2bytes(cfg->preb);

	if (prebuf > buf_size) prebuf = buf_size;
	else if (prebuf < 0) prebuf = 0;

	DSBUFFERDESC desc =
	    {
	        sizeof(DSBUFFERDESC),
	        DSBCAPS_GETCURRENTPOSITION2 |
	        DSBCAPS_STICKYFOCUS |
	        DSBCAPS_GLOBALFOCUS |
	        DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME
#ifdef DS2_HAVE_PITCH
	        | (cfg->have_pitch ? DSBCAPS_CTRLFREQUENCY : 0)
#endif

	        ,
	        buf_size,
	        0,
	        &wfx
	    };
	switch (cfg->mixing)
	{
	case DS2config::MIXING_FORCE_HARDWARE:
		desc.dwFlags |= DSBCAPS_LOCHARDWARE;
		break;
	case DS2config::MIXING_FORCE_SOFTWARE:
		desc.dwFlags |= DSBCAPS_LOCSOFTWARE;
		break;
	}

	do
	{
		hr = pDS->CreateSoundBuffer(&desc, &pDSB, 0);
		if (SUCCEEDED(hr)) break;
		WAVEFORMATEXTENSIBLE wfxe;
		wfxe.Format = wfx;
		wfxe.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfxe.Format.cbSize = 22;
		wfxe.Samples.wReserved = 0;
		wfxe.dwChannelMask = cfg->chan_mask;
		if (!wfxe.dwChannelMask)
		{
			if (fmt_nch == 4)
			{
				wfxe.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
			}
			else if (fmt_nch == 6)
			{
				wfxe.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY;
			}
		}
		wfxe.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		desc.lpwfxFormat = &wfxe.Format;
		if (SUCCEEDED(pDS->CreateSoundBuffer(&desc, &pDSB, 0))) {hr = 0;break;}
	}
	while (0);

	if (FAILED(hr) || !pDSB)
	{
		cfg->SetErrorCodeMsg(WASABI_API_LNGSTRINGW_DS(IDS_ERROR_CREATING_DS_BUFFER), hr);
		return 0;
	}

	pDS->AddRef();
	myDS = pDS;

	{
		DSBCAPS caps;
		memset(&caps, 0, sizeof(caps));
		caps.dwSize = sizeof(caps);
		pDSB->GetCaps(&caps);
		if (caps.dwFlags & DSBCAPS_LOCSOFTWARE) fset(SWMIXED);
	}

	clear_size = ms2bytes(200);
	if (clear_size > buf_size >> 2) clear_size = buf_size >> 2;
	clear_size = _align_var(clear_size);
	if (prebuf < clear_size + (clear_size >> 1)) prebuf = clear_size + (clear_size >> 1);

	VolCtrl.Apply(pDSB);

	reset_vars();

	//pDSB->SetVolume(DSBVOLUME_MAX);
	return 1;
}

void DS2::reset_vars()
{
	pos_delta = 0;pos_delta2 = 0;
	flags &= ~(FLAG_NEED_PLAY_NOW | FLAG_PLAYING);
	data_buffered = 0;
	data_written = 0;
	silence_buffered = 0;
	last_nonsil = -1;
	BlockList.Reset();
	VolCtrl.Reset();
}

void DS2::do_reset_vars()
{
	g_total_time += GetOutputTime();
	reset_vars();
}

bool DS2::DoLock()
{
	void *p1 = 0, *p2 = 0;
	DWORD s1 = 0, s2 = 0;

	UINT LockSize = BlockList.DataSize();
	if (LockSize > 0 && silence_buffered)
	{
		data_written -= silence_buffered;
		__int64 min = GetSafeWrite();
		if (data_written < min) data_written = min;
		silence_buffered = 0;
	}

	UINT MaxData = buf_size;

	int FooScale = ftest(SWMIXED) ? 6 : 4;

	MaxData = _align_var(MaxData - (MaxData >> FooScale));

	UINT MaxLock = MaxData > data_buffered ? MaxData - data_buffered : 0;

	if (!MaxLock) return 0;

	if (
	    ((ftest(PLAYING)) || ftest(NEED_PLAY_NOW))
	    && data_buffered + LockSize < clear_size)
		//underrun warning, put extra silence
		LockSize = clear_size - data_buffered;

	if (LockSize > MaxLock) LockSize = MaxLock;

	if (LockSize == 0) return 0; //final check for useless locks

	if (data_buffered > clear_size && LockSize < buf_size >> FooScale) return 0;

	while (1)
	{
		HRESULT hr = pDSB->Lock((UINT)(data_written % (__int64)buf_size), LockSize, &p1, &s1, &p2, &s2, 0);

		if (SUCCEEDED(hr))
		{
			LockCount++;
			UINT written;
			written = BlockList.DumpBlocks(p1, s1);
			if (p2 && s2) written += BlockList.DumpBlocks(p2, s2);
			//note: we fill with silence when not enough data
			UINT total = s1 + s2;
			data_written = data_written + total;
			data_buffered += total;
			if (written > 0) silence_buffered = total - written;
			else silence_buffered += total;
			pDSB->Unlock(p1, s1, p2, s2);
			break;
		}
		else if (hr == DSERR_BUFFERLOST)
		{
			if (FAILED(pDSB->Restore())) break;
		}
		else break;
	}

	return 1;
}

int DS2::CanWrite() //result can be negative !
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::CanWrite"));
	if (ftest(PAUSED)) return 0;

	int rv;

	int m = buf_size - (int)(data_buffered + BlockList.DataSize());

	if (ftest(USE_CPU_MNGMNT) && ftest(PLAYING) ) // && data_written<buf_size && GetCurPos()<buf_size)
	{
		__int64 t = ((GetCurPos() - pos_delta) << 2) - (data_written - pos_delta2 + BlockList.DataSize());
		rv = t > m ? m : (int)t;
	}
	else
	{
		rv = m;
	}

	if (wait) rv -= ms2bytes(wait->GetLatency());

	return _align_var(rv);
}

LockGuard DS2::ds2Guard("ds2Guard");
void DS2::Pause(int new_state)
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::Pause"));

	if (new_state && !ftest(PAUSED))
	{ //pause

		if (ftest(PLAYING) && pDSB)
		{
			pDSB->Stop();

		}
		fset(PAUSED);
	}
	else if (!new_state)
	{
		if (ftest(PAUSED))
		{ //unpause

			if (ftest(PLAYING)) fset(NEED_PLAY_NOW);

			if (ftest(FADEPAUSE))
			{
				VolCtrl.SetTime(GetCurPos());
				VolCtrl.SetFadeVol(ms2bytes(fadepause_time), fadepause_orgvol);
			}
		}

		else if (ftest(FADEPAUSING)) //abort fadeout
		{
			VolCtrl.SetTime(GetCurPos());
			VolCtrl.SetFadeVol(VolCtrl.RelFade(ms2bytes(fadepause_time), fadepause_orgvol), fadepause_orgvol);
		}
		funset(FADEPAUSE);
		funset(FADEPAUSING);

		funset(PAUSED);
	}

	if (wait)
	{
		wait->Pause(new_state);
	}
}

void DS2::SetVolume(double v)
{
	AutoLock lock (ds2Guard LOCKNAME("DS2::SetVolume"));
	if (!ftest(DIE_ON_STOP) && pDSB)
	{
		VolCtrl.SetVolume(v);
		VolCtrl.Apply(pDSB);
	}
	if (wait) wait->SetVolume(v);
}

void DS2::SetPan(double p)
{
	AutoLock lock (ds2Guard LOCKNAME("SetPan"));
	if (!ftest(DIE_ON_STOP) && pDSB)
	{
		VolCtrl.SetPan(p);
		VolCtrl.Apply(pDSB);
	}
	if (wait) wait->SetPan(p);
}

UINT DS2::GetLatency()
{
	AutoLock lock (ds2Guard LOCKNAME("GetLatency"));
	UINT bDataSize = BlockList.DataSize();
	int bytes;
	if (bDataSize) bytes = data_buffered + BlockList.DataSize();
	else bytes = data_buffered - silence_buffered;
	if (bytes < 0) bytes = 0;
	UINT rv = bytes2ms((UINT)bytes);
	if (wait) rv += wait->GetLatency();
	return rv;
}

void DS2::Fade(UINT time, double destvol)
{
	AutoLock lock (ds2Guard LOCKNAME("Fade"));
	VolCtrl.SetFadeVol(ms2bytes(time), destvol);
}

void DS2::FadeAndForget(UINT time)
{
	AutoLock lock (ds2Guard LOCKNAME("FadeAndForget"));
	if (!pDSB || time == 0 || ftest(PAUSED) || (!data_written && !BlockList.DataSize()))
	{
		delete this;
	}
	else
	{
		fset(DIE_ON_STOP);
		if (!ftest(PLAYING)) fset(NEED_PLAY_NOW);

		__int64 fadetime = ms2bytes(time);
		__int64 max = data_written + BlockList.DataSize() - GetCurPos();
		if (max < 0) max = 0;
		if (fadetime > max) fadetime = max;

		VolCtrl.SetFadeVol(fadetime, 0);
	}
}

void DS2::FadeX(UINT time, double dest)
{
	AutoLock lock (ds2Guard LOCKNAME("FadeX"));
	if (ftest(PAUSED) && ftest(FADEPAUSE))
	{
		fadepause_orgvol = dest;
	}
	VolCtrl.SetFadeVol(VolCtrl.RelFade(ms2bytes(time), dest), dest);
}

void DS2::FadePause(UINT time)
{
	AutoLock lock (ds2Guard LOCKNAME("FadePause"));
	if (!time)
	{
		Pause(1);
	}
	else
	{
		if (wait)
		{
			wait->FadeAndForget(time);
			wait = 0;
		}
		if (!ftest(PLAYING))
		{
			fset(PAUSED);
		}
		else
		{
			fadepause_time = time;
			fset(FADEPAUSE);
			fset(FADEPAUSING);
			fadepause_orgvol = VolCtrl.GetDestVol();
			VolCtrl.SetFadeVol(ms2bytes(time), 0);
		}
	}
}

UINT DS2::InstanceCount()
{
	AutoLock lock (ds2Guard LOCKNAME("InstanceCount"));
	UINT rv = 0;
	DS2 * p = ds2s;
	while (p) {rv++;p = p->next;}
	return rv;
}

__int64 DS2::GetSafeWrite()
{
	return GetCurPos() + clear_size + ms2bytes(refresh_timer);
}

void DS2::KillEndGap()
{
	AutoLock lock (ds2Guard LOCKNAME("KillEndGap"));
	if (silence_delta >= 0 && last_nonsil >= 0)
	{
		__int64 cp = GetSafeWrite();
		if (cp < data_written)
		{
			__int64 dest = last_nonsil < cp ? cp : last_nonsil;
			if (dest > data_written)
			{ //need to take data from blocklist
				UINT s = BlockList.DataSize();
				char *temp0r = (char*)malloc(s);
				BlockList.DumpBlocks(temp0r, s);
				BlockList.Reset();
				BlockList.AddBlock(temp0r, (UINT)(dest - data_written));
				free(temp0r);
			}
			else
			{
				BlockList.Reset();
				data_written = dest;
			}
		}
		last_nonsil = -1;
		fset(STARTSIL);
	}
}

void DS2::Flush()
{
	AutoLock lock (ds2Guard LOCKNAME("Flush"));
	ds_stop();
}

void DS2::ForcePlay()
{
	AutoLock lock (ds2Guard LOCKNAME("ForcePlay"));
	if (!ftest(PAUSED) && !ftest(PLAYING) && !wait && data_buffered + BlockList.DataSize() > 0)
	{
		fset(NEED_PLAY_NOW);
	}
}

void DS2::WaitFor(DS2 * prev, UINT fade)
{
	AutoLock lock (ds2Guard LOCKNAME("WaitFor"));
	if (wait) delete wait;
	wait = prev;
	waitfade = fade;
	wait->flags |= FLAG_WAITED;
	wait->ForcePlay();
}

void DS2::StartNewStream()
{
	AutoLock lock (ds2Guard LOCKNAME("StartNewSteam"));
	if (last_nonsil > data_written + BlockList.DataSize()) last_nonsil = data_written + BlockList.DataSize();
	pos_delta = GetCurPos();pos_delta2 = data_written;
}

void DS2::SetCloseOnStop(bool b)
{
	AutoLock lock (ds2Guard LOCKNAME("SetCloseOnStop"));
	fsetc(CLOSE_ON_STOP, b);
	if (b && !ftest(PLAYING)) ds_kill();
}

bool DS2::IsClosed()
{
	AutoLock lock (ds2Guard LOCKNAME("IsClosed"));
	bool rv = pDSB ? 0 : 1;
	return rv;
}

void DS2::GetRealtimeStat(DS2_REALTIME_STAT * stat)
{
	AutoLock lock (ds2Guard LOCKNAME("GetRealtimeStat"));
	__int64 curpos = GetCurPos();
	stat->sr = fmt_sr;
	stat->bps = fmt_bps;
	stat->nch = fmt_nch;
	stat->buf_size_bytes = buf_size;
	stat->buf_size_ms = bytes2ms(buf_size);
	stat->pos_play = (UINT)(curpos % buf_size);
	stat->pos_write = (UINT)(data_written % buf_size);
	stat->latency = data_buffered + BlockList.DataSize();
	if (stat->latency < 0) stat->latency = 0;
	stat->latency_ms = bytes2ms(stat->latency);
	stat->lock_count = LockCount;
	stat->underruns = Underruns;
	stat->bytes_async = BlockList.DataSize();
	stat->bytes_written = data_written + BlockList.DataSize();
	stat->bytes_played = curpos;
	stat->have_primary_buffer = pPrimary ? true : false;
	stat->current_device = cur_dev;
	stat->vol_left = VolCtrl.Stat_GetVolLeft();
	stat->vol_right = VolCtrl.Stat_GetVolRight();
	if (pDSB)
	{
		DSBCAPS caps;
		memset(&caps, 0, sizeof(caps));
		caps.dwSize = sizeof(caps);
		pDSB->GetCaps(&caps);
		stat->dscaps_flags = caps.dwFlags;
	}
	else stat->dscaps_flags = 0;
	if (pPrimary)
	{
		DSBCAPS caps;
		memset(&caps, 0, sizeof(caps));
		caps.dwSize = sizeof(caps);
		pPrimary->GetCaps(&caps);
		stat->dscaps_flags_primary = caps.dwFlags;
	}
	else stat->dscaps_flags_primary = 0;
	stat->paused = !!ftest(PAUSED);
}

bool DS2::GetRealtimeStatStatic(DS2_REALTIME_STAT * stat)
{
	bool rv = 0;
	AutoLock lock (ds2Guard LOCKNAME("GetRealtimeStatStatic"));
	if (ds2s) {ds2s->GetRealtimeStat(stat);rv = 1;}
	return rv;
}

void DS2::SetTotalTime(__int64 z)
{
	AutoLock lock (ds2Guard LOCKNAME("SetTotalTime"));
	g_total_time = z;
}

__int64 DS2::GetTotalTime()
{
	AutoLock lock (ds2Guard LOCKNAME("GetTotalTime"));
	__int64 r = g_total_time;
	DS2 * p = ds2s;
	while (p)
	{
		r += p->GetOutputTime();
		p = p->next;
	}

	return r;
}

__int64 DS2::GetOutputTime()
{
	if (!fmt_bps || !fmt_nch || !fmt_sr) return 0;
	AutoLock lock (ds2Guard LOCKNAME("GetOutputTime"));
	__int64 r = (GetCurPos()) / ((fmt_bps >> 3) * fmt_nch) * 1000 / fmt_sr;

	return r;
}

#ifdef DS2_HAVE_PITCH
void DS2::SetPitch(double p)
{
	AutoLock lock (ds2Guard LOCKNAME("SetPitch"));
	DWORD f = (DWORD)(p * (double)fmt_sr);
	if (f < DSBFREQUENCY_MIN) f = DSBFREQUENCY_MIN;
	else if (f > DSBFREQUENCY_MAX) f = DSBFREQUENCY_MAX;
	if (pDSB) pDSB->SetFrequency(f);

}
#endif

GUID DS2::GetCurDev() { return cur_dev;}