#define STRICT 1
#include <windows.h>
#include "waveout.h"

#include <strsafe.h>
#include "api.h"
#include "res_wav/resource.h"
#define IDS_UNKNOWN_ERROR_WAV 59
#include "../winamp/in2.h"
#include <mmreg.h>

extern In_Module plugin;
Nullsoft::Utility::LockGuard waveOutGuard GUARDNAME("waveOutGuard");
#define GET_TIME timeGetTime()

using namespace Nullsoft::Utility;
WaveOut::~WaveOut()
{
	if (hThread)
	{
		{
			AutoLock lock(waveOutGuard);
			die=1;
		}
		
		SetEvent(hEvent);
		WaitForSingleObject(hThread,INFINITE);
	}
	if (hEvent) CloseHandle(hEvent);

	killwaveout();
	if (buffer) LocalFree(buffer);
	hdr_free_list(hdrs);
	hdr_free_list(hdrs_free);
}
 
DWORD WINAPI WaveOut::ThreadProc(WaveOut * p)
{
	p->thread();
	return 0;
}

void WaveOut::thread()
{
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	while(hWo)
	{
		WaitForSingleObject(hEvent,INFINITE);

		AutoLock lock(waveOutGuard);

		if (die)
		{
			break;
		}
		

		for(HEADER * h=hdrs;h;)
		{
			if (h->hdr.dwFlags&WHDR_DONE)
			{
				n_playing--;
				buf_size_used-=h->hdr.dwBufferLength;
				last_time=GET_TIME;
				waveOutUnprepareHeader(hWo,&h->hdr,sizeof(WAVEHDR));
				HEADER* f=h;
				h=h->next;
				hdr_free(f);
			}
			else h=h->next;
		}


/*		if (needflush)
		{
			flush();
			if (!paused) waveOutRestart(hWo);
			needflush=0;
		}*/

		if (!paused && newpause)
		{
			paused=1;
			if (hWo) waveOutPause(hWo);
			p_time=GET_TIME-last_time;
		}

		if (paused && !newpause)
		{
			paused=0;
			if (hWo) waveOutRestart(hWo);
			last_time = GET_TIME-p_time;
		}


		UINT limit;
		if (needplay) limit=0;
		else if (!n_playing)
		{
			limit=prebuf;
			if (limit<avgblock) limit=avgblock;
		}
		else if (buf_size_used<(buf_size>>1) || n_playing<3) limit=minblock;//skipping warning, blow whatever we have
		else limit=avgblock;//just a block

		while(data_written>limit)
		{
			UINT d=(data_written > maxblock) ? maxblock : data_written;
			d-=d%fmt_align;
			if (!d) break;
			data_written-=d;
			buf_size_used+=d;
			
			HEADER * h=hdr_alloc();
			h->hdr.dwBytesRecorded=h->hdr.dwBufferLength=d;
			h->hdr.lpData=buffer+write_ptr;
			write_ptr+=d;
			if (write_ptr>buf_size)
			{
				write_ptr-=buf_size;
				memcpy(buffer+buf_size,buffer,write_ptr);
			}
			
			n_playing++;
			if (use_altvol) do_altvol(h->hdr.lpData,d);
			waveOutPrepareHeader(hWo,&h->hdr,sizeof(WAVEHDR));
			waveOutWrite(hWo,&h->hdr,sizeof(WAVEHDR));//important: make all waveOutWrite calls from *our* thread to keep win2k/xp happy
			if (n_playing==1) last_time=GET_TIME;
#if 0
			{
				char t[128];
				wsprintf(t,"block size: %u, limit used %u\n", d,limit);
				OutputDebugString(t);
			}
#endif
		}
		needplay=0;
		
		if (!data_written && !n_playing && closeonstop) killwaveout();

	}
	killwaveout();
}

int WaveOut::WriteData(const void * _data,UINT size)
{
	UINT written=0;
	{
	AutoLock lock(waveOutGuard);
	if (paused)	//$!#@!
	{
		return 0;
	}

	const char * data=(const char*)_data;

	{
		UINT cw=CanWrite();
		if (size>cw)
		{
			size=cw;
		}
	}

	
	while(size>0)
	{
		UINT ptr=(data_written + write_ptr)%buf_size;
		UINT delta=size;
		if (ptr+delta>buf_size) delta=buf_size-ptr;
		memcpy(buffer+ptr,data,delta);
		data+=delta;
		size-=delta;
		written+=delta;
		data_written+=delta;
	}
	
} // sync out first to prevent a ping-pong condition
	if (written) SetEvent(hEvent);//new shit, time to update
	return (int)written;
}

void WaveOut::flush()//in sync
{
	waveOutReset(hWo);

	while(hdrs)
	{
		if (hdrs->hdr.dwFlags & WHDR_PREPARED)
		{
			waveOutUnprepareHeader(hWo,&hdrs->hdr,sizeof(WAVEHDR));
		}
		hdr_free(hdrs);
	}
	reset_shit();
}

void WaveOut::Flush()
{
/*	//waveOutGuard.Lock();
	needflush=1;
	SetEvent(hEvent);
	//waveOutGuard.Unlock();
	while(needflush) Sleep(1);*/
	AutoLock lock(waveOutGuard);
	flush();
	if (!paused) waveOutRestart(hWo);
}


void WaveOut::ForcePlay()
{
		AutoLock lock(waveOutGuard);

	if (!paused) {needplay=1;SetEvent(hEvent);}

//	while(needplay) Sleep(1);
}

WaveOut::WaveOut()
{
#ifndef TINY_DLL	//TINY_DLL has its own new operator with zeroinit
	memset(&hWo,0,sizeof(*this)-((char*)&hWo-(char*)this));
#endif
	myvol=-666;
	mypan=-666;
}

int WaveOut::open(WaveOutConfig * cfg)
{
	fmt_sr=cfg->sr;
	fmt_bps=cfg->bps;
	fmt_nch=cfg->nch;
	fmt_align=(fmt_bps>>3)*fmt_nch;
	fmt_mul=fmt_align*fmt_sr;

	use_volume=cfg->use_volume;
	use_altvol=cfg->use_altvol;
	use_resetvol=cfg->resetvol;
	if (!use_volume) use_altvol=use_resetvol=0;
	else if (use_altvol) {use_resetvol=0;use_volume=0;}

	WAVEFORMATEX wfx=
	{
		WAVE_FORMAT_PCM,
		fmt_nch,
		fmt_sr,
		fmt_mul,
		fmt_align,
		fmt_bps,
		0
	};

	if (!hEvent) hEvent=CreateEvent(0,0,0,0);
	MMRESULT mr=waveOutOpen(&hWo,cfg->dev-1,&wfx,(DWORD)hEvent,0,CALLBACK_EVENT);
	if (mr)
	{
		if (mr==32)
		{
			WAVEFORMATEXTENSIBLE wfxe;
			wfxe.Format=wfx;
			wfxe.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
			wfxe.Format.cbSize=22;
			wfxe.Samples.wReserved=0;
			wfxe.dwChannelMask=0;//fmt_chan;
			wfxe.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
			mr=waveOutOpen(&hWo,cfg->dev-1,(WAVEFORMATEX*)&wfxe,(DWORD)hEvent,0,CALLBACK_EVENT);
		}
		if (mr)
		{
			wchar_t full_error[1024];
			wchar_t e2[1024];
			wchar_t poo[MAXERRORLENGTH];
			if (waveOutGetErrorText(mr,poo,MAXERRORLENGTH)!=MMSYSERR_NOERROR)
			{
				WASABI_API_LNGSTRINGW_BUF_WAV(IDS_UNKNOWN_MMSYSTEM_ERROR,poo,MAXERRORLENGTH);
			}
			switch(mr)
			{
			case 32:
				StringCchPrintf(e2, 1024, WASABI_API_LNGSTRINGW_WAV(IDS_UNSUPPORTED_PCM_FORMAT),fmt_sr,fmt_bps,fmt_nch);
				//fixme: some broken drivers blow mmsystem032 for no reason, with "standard" 44khz/16bps/stereo, need better error message when pcm format isnt weird
				break;
			case 4:
				StringCchCopy(e2, 1024, WASABI_API_LNGSTRINGW_WAV(IDS_ANOTHER_PROGRAM_IS_USING_THE_SOUNDCARD));
				break;
			case 2:
				StringCchCopy(e2, 1024, WASABI_API_LNGSTRINGW_WAV(IDS_NO_SOUND_DEVICES_FOUND));
				break;
			case 20:
				StringCchCopy(e2, 1024, WASABI_API_LNGSTRINGW_WAV(IDS_INTERNAL_DRIVER_ERROR));
				break;
			case 7:
				StringCchCopy(e2, 1024, WASABI_API_LNGSTRINGW_WAV(IDS_REINSTALL_SOUNDCARD_DRIVERS));
				break;
				//case 8: fixme
			default:
				StringCchCopy(e2, 1024, WASABI_API_LNGSTRINGW(IDS_UNKNOWN_ERROR_WAV));
			}
			
			StringCchPrintf(full_error,1024, WASABI_API_LNGSTRINGW_WAV(IDS_ERROR_CODE_WINDOWS_ERROR_MESSAGE),e2,mr,poo);
			cfg->SetError(full_error);
			return 0;
		}
	}

	
	buf_size=MulDiv(cfg->buf_ms,fmt_mul,1000);
	
	maxblock = 0x10000;
	minblock = 0x100;
	avgblock = buf_size>>4;
	if (maxblock>buf_size>>2) maxblock=buf_size>>2;
	if (avgblock>maxblock) avgblock=maxblock;
	if (maxblock<minblock) maxblock=minblock;
	if (avgblock<minblock) avgblock=minblock;
	

	buffer = (char*)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT,buf_size+maxblock);//extra space at the end of the buffer

	prebuf = MulDiv(cfg->prebuf,fmt_mul,1000);
	if (prebuf>buf_size) prebuf=buf_size;

	n_playing=0;

	waveOutRestart(hWo);
	reset_shit();
	

	if (use_resetvol) waveOutGetVolume(hWo,&orgvol);

	if (myvol!=-666 || mypan!=-666) update_vol();


	{
		DWORD dw;
		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadProc,this,0,&dw);
	}

	return 1;
}

int WaveOut::GetLatency(void)
{
	AutoLock lock(waveOutGuard);
	int r=0;
	if (hWo)
	{
		r=MulDiv(buf_size_used+data_written,1000,(fmt_bps>>3)*fmt_nch*fmt_sr);
		if (paused) r-=p_time;
		else if (n_playing) r-=GET_TIME-last_time;
		if (r<0) r=0;
	}
	return r;
}


void WaveOut::SetVolume(int v)
{
	AutoLock lock(waveOutGuard);
	myvol=v;
	update_vol();

}

void WaveOut::SetPan(int p)
{
	AutoLock lock(waveOutGuard);
	mypan=p;
	update_vol();
}

void WaveOut::update_vol()
{
	if (hWo && use_volume)
	{
		if (myvol==-666) myvol=255;
		if (mypan==-666) mypan=0;
		DWORD left,right;
		left=right=myvol|(myvol<<8);
		if (mypan<0) right=(right*(128+mypan))>>7;
		else if (mypan>0) left=(left*(128-mypan))>>7;		
		waveOutSetVolume(hWo,left|(right<<16));
	}
}

void WaveOut::reset_shit()
{
	n_playing=0;
	data_written=0;
    buf_size_used=0;
	last_time=0;
	//last_time=GET_TIME;
}

void WaveOut::Pause(int s)
{
	{
		AutoLock lock(waveOutGuard);
	newpause=s?1:0;//needs to be done in our thread to keep stupid win2k/xp happy
	}
		SetEvent(hEvent);
	while(paused!=newpause) Sleep(1);
}

void WaveOut::killwaveout()
{
	if (hWo)
	{
		flush();
		if (use_resetvol) waveOutSetVolume(hWo,orgvol);
		waveOutClose(hWo);
		hWo=0;
	}
}

int WaveOut::CanWrite()
{
	AutoLock lock(waveOutGuard);
	int rv=paused ? 0 : buf_size-buf_size_used-data_written;
	return rv;
}

WaveOut * WaveOut::Create(WaveOutConfig * cfg)
{
	WaveOut * w=new WaveOut;
	if (w->open(cfg)<=0)
	{
		delete w;
		w=0;
	}
	return w;
}


WaveOut::HEADER * WaveOut::hdr_alloc()
{
	HEADER * r;
	if (hdrs_free)
	{
		r=hdrs_free;
		hdrs_free=hdrs_free->next;
	}
	else
	{
		r=new HEADER;
	}
	r->next=hdrs;
	hdrs=r;
	memset(&r->hdr,0,sizeof(WAVEHDR));
	return r;
}

void WaveOut::hdr_free(HEADER * h)
{
	HEADER ** p=&hdrs;
	while(*p)
	{
		if (*p==h)
		{
			*p = (*p)->next;
			break;
		}		
		else p=&(*p)->next;
	}

	h->next=hdrs_free;
	hdrs_free=h;
}

void WaveOut::hdr_free_list(HEADER * h)
{
	while(h)
	{
		HEADER * t=h->next;
		delete h;
		h=t;
	}
}

bool WaveOut::PrintState(wchar_t * z, size_t cchLength)
{
	bool rv;
	AutoLock lock(waveOutGuard);

	if (!hWo) rv=0;
	else
	{
		rv=1;
		StringCchPrintf(z,cchLength,WASABI_API_LNGSTRINGW_WAV(IDS_DATA_FORMAT),fmt_sr,fmt_bps,fmt_nch);
		while(*z) z++;
		StringCchPrintf(z,cchLength,WASABI_API_LNGSTRINGW_WAV(IDS_BUFFER_STATUS),buf_size,n_playing);
		while(*z) z++;
		StringCchPrintf(z,cchLength,WASABI_API_LNGSTRINGW_WAV(IDS_LATENCY),GetLatency());
	//	while(*z) z++;
	//	wsprintf(z,"Data written: %u KB",MulDiv((int)total_written,(fmt_bps>>3)*fmt_nch,1024));
	}

	return rv;
}

void WaveOutConfig::SetError(const TCHAR * x)
{
	size_t cchLength = lstrlen(x)+1;
	error=(TCHAR *)LocalAlloc(LMEM_FIXED,sizeof(TCHAR)*cchLength);
	StringCchCopy(error,cchLength,x);
}

void WaveOut::do_altvol_i(char * ptr,UINT max,UINT start,UINT d,int vol)
{
	UINT p=start*(fmt_bps>>3);
	while(p<max)
	{
		void * z=ptr+p;
		switch(fmt_bps)
		{
		case 8:
			*(BYTE*)z=0x80^(BYTE)MulDiv(0x80^*(BYTE*)z,vol,255);
			break;
		case 16:
			*(short*)z=(short)MulDiv(*(short*)z,vol,255);
			break;
		case 24:
			{
				long l=0;
				memcpy(&l,z,3);
				if (l&0x800000) l|=0xFF000000;
				l=MulDiv(l,vol,255);
				memcpy(z,&l,3);
			}
			break;
		case 32:
			*(long*)z=MulDiv(*(long*)z,vol,255);
			break;
		}
		p+=d*(fmt_bps>>3);
	}
}

void WaveOut::do_altvol(char * ptr,UINT s)
{
	int mixvol=(myvol==-666) ? 255 : myvol;
	int mixpan=(mypan==-666) ? 0 : mypan;
	if (mixvol==255 && (fmt_nch!=2 || mixpan==0)) return;
	if (fmt_nch==2)
	{
		int rv=mixvol,lv=mixvol;
		if (mixpan<0)
		{//-128..0
			rv=MulDiv(rv,mixpan+128,128);
		}
		else if (mixpan>0)
		{
			lv=MulDiv(rv,128-mixpan,128);
		}		
		do_altvol_i(ptr,s,0,2,lv);
		do_altvol_i(ptr,s,1,2,rv);
	}
	else
	{
		do_altvol_i(ptr,s,0,1,mixvol);
	}
}

bool WaveOut::IsClosed()
{
	AutoLock lock(waveOutGuard);
	
	bool rv=hWo ? 0 : 1;
		return rv;
}