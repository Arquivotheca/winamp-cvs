#include "Main.h"
#include "resource.h"
#include <math.h>
#include "api.h"
//#define PROFILE_PLUGINS_LOAD_TIME
#include "timing.h"
#include "PtrList.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "WinampAttributes.h"
#include "eq10dsp.h"
#include "../nsutil/pcm.h"

int filter_srate,  filter_enabled, filter_top, filter_top2;
static In_Module *dsp_init_mod = 0;
static int dsp_in_init = 0;
nu::PtrList<In_Module> in_modules;
In_Module *in_mod=0;
float preamp_val=1.0f;

eq10_t *eq=0;

extern "C" volatile int sa_override;

static void setinfo(int bitrate, int srate, int stereo, int synched);
static void vissa_init(int maxlatency_in_ms, int srate);
static void vissa_deinit();
static int sa_getmode();
static int eq_dosamples_4front(short *samples, int numsamples, int bps, int nch, int srate);
static int eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate);
int benskiQ_eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate);
static int eq_isactive();

static int myisourfile(const char *filename) // mjf bullshit
{
	return 0;
}

void in_init(void)
{
	HANDLE h;
	WIN32_FIND_DATAW d;
	wchar_t dirstr[MAX_PATH];

	CreateMainWindow(); // input plugins require the main window, at least for IPC calls
	PathCombineW(dirstr, PLUGINDIR, L"IN_*.DLL");
	h = FindFirstFileW(dirstr, &d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t namestr[MAX_PATH];
			HINSTANCE hLib;
			PathCombineW(namestr, PLUGINDIR, d.cFileName);
			hLib = LoadLibraryW(namestr);
			if (hLib)
			{
				In_Module * (*pr)();
				In_Module *mod;
				pr = (In_Module * (*)()) GetProcAddress(hLib, "winampGetInModule2");
				if (pr)
				{
					mod = pr();
					if (mod && ((mod->version & ~IN_UNICODE) == IN_VER || (mod->version & ~IN_UNICODE) == IN_VER_U))
					{
						if (g_safeMode)
						{
							char desc[128];
							lstrcpyn(desc, mod->description, sizeof(desc));
							if (desc[0] && !memcmp(desc, "nullsoft(", 9))
							{
								char* p = strrchr(desc, ')');
								if (p)
								{
									*p = 0;
									if(_wcsicmp(d.cFileName, AutoWide(desc+9)))
									{
										FreeModule(hLib);
										continue;
									}
								}
							}
							else
							{
								FreeModule(hLib);
								continue;
							}
						}

						mod->hDllInstance = hLib;
						mod->dsp_dosamples = eq_dosamples;
						mod->dsp_isactive = eq_isactive;
						mod->SAGetMode = sa_getmode;
						mod->SAAdd = (int (__cdecl *)(void *, int, int))sa_add;
						mod->SAVSAInit = vissa_init;
						mod->SAVSADeInit = vissa_deinit;
						mod->VSASetInfo = vis_setinfo;
						mod->VSAAdd = vsa_add;
						mod->VSAGetMode = vsa_getmode;
						mod->SAAddPCMData = sa_addpcmdata;
						mod->VSAAddPCMData = vsa_addpcmdata;
						mod->hMainWindow = hMainWindow;
						mod->SetInfo = NULL;
						mod->UsesOutputPlug &= ~(2 | 4 | 8 | 16);
						if (!_wcsicmp(d.cFileName, L"in_mjf.dll"))
						{
							mod->IsOurFile = myisourfile;
						}
						in_modules.push_back(mod);
						mod->Init();
						if (mod->SetInfo)
						{
							char *p = (char *)mod->SetInfo;
							if (*p)
							{
								StringCchCat(metric_plugin_list, 512, ":");
								StringCchCat(metric_plugin_list, 512, p);
							}
						}
						mod->SetInfo = setinfo;
						if (!g_has_video_plugin)
						{
							
							int (*gefiW)(const wchar_t *fn, const char *data, wchar_t *dest, int destlen);
							gefiW = (int (__cdecl *)(const wchar_t *, const char *, wchar_t *, int))GetProcAddress(hLib, "winampGetExtendedFileInfoW");
							if (gefiW)
							{
								wchar_t dest[16];
								dest[0] = 0;
								gefiW(L"", "type", dest, 16);
								if (_wtoi(dest) == 1) g_has_video_plugin = 1;
							}

							int (*gefi)(const char *fn, const char *data, char *dest, int destlen);
							gefi = (int (__cdecl *)(const char *, const char *, char *, int))GetProcAddress(hLib, "winampGetExtendedFileInfo");
							if (gefi)
							{
								char dest[16];
								dest[0] = 0;
								gefi("", "type", dest, 16);
								if (atoi(dest) == 1) g_has_video_plugin = 1;
							}
						}
					}
					else
					{
						FreeModule(hLib);
						continue;
					}
				}
			}
		}
		while (FindNextFileW(h, &d));
		FindClose(h);
	}
}

In_Module *g_in_infobox = 0;

void in_deinit(void)
{
	size_t x = in_modules.size();
	while (x--)
	{
		In_Module *&mod = in_modules[x];
		// make sure there's something valid due to the dynamic unload changes in 5.5+
		if (mod != g_in_infobox && mod)
		{
			//HINSTANCE hLib = mod->hDllInstance;
			mod->Quit();
			//FreeLibrary(hLib); // benski> we're just going to let it leak because it might be subclassing
			mod = 0;
		}
	}
	in_modules.clear();
}

In_Module *in_setmod_noplay(const wchar_t *filename, int *start_offs)
{
	if (!_wcsnicmp(L"audit://", filename, 8))
		return 0;

	size_t x;
	char ext[128];
	extension_ex(AutoChar(filename), ext, sizeof(ext));
	for (x = start_offs ? *start_offs : 0; x < in_modules.size(); x ++)
	{
		if (InW_IsOurFile(in_modules[x], filename))
		{
			if (start_offs) *start_offs = x;
			if (!in_modules[x]->hMainWindow) in_modules[x]->hMainWindow = hMainWindow;
			return in_modules[x];
		}
	}
	for (x = start_offs ? *start_offs : 0; x < in_modules.size(); x ++)
	{
		if(in_modules[x])
		{
			char *p = in_modules[x]->FileExtensions;
			while (p && *p)
			{
				char *b = p;
				char *c;
				do
				{
					char d[20];
					lstrcpyn(d, b, 15);
					if ((c = strstr(b, ";")))
					{
						if ((c-b)<15)
							d[c - b] = 0;
					}
					//else
						//d[lstrlen(b)] = 0;

					if (!lstrcmpi(ext, d))
					{
						if (start_offs) *start_offs = x;
						if (!in_modules[x]->hMainWindow) in_modules[x]->hMainWindow = hMainWindow;
						return in_modules[x];
					}
					b = c + 1;
				}
				while (c);
				p += lstrlen(p) + 1;
				if (!*p) break;
				p += lstrlen(p) + 1;
			}
		}
	}
	if (start_offs)
	{
		*start_offs = -1;
		return 0;
	}

	{
		static int r;
		const wchar_t *p;

		if (PathFindExtensionW(filename)[0]
		    && (p = wcsstr(filename, L"://")) && (p = wcsstr(p, L"?")))
		{
			wchar_t * d = _wcsdup(filename);
			In_Module *v;
			d[p - filename] = 0;
			v = in_setmod_noplay(d, 0);
			free(d);
			return v;
		}

		if (!config_defext[0] || config_defext[0] == ' ') StringCchCopy(config_defext, 32, "mp3");
		if (!r)
		{
			wchar_t a[128] = L"hi.";
			In_Module *v;
			MultiByteToWideCharSZ(CP_ACP, 0, config_defext, -1, a+3, 120);
			r = 1;
			v = in_setmod_noplay(a, 0);
			r = 0;
			return v;
		}
		else return 0;
	}
}

In_Module *in_setmod(const wchar_t *filename)
{
	In_Module *i = in_setmod_noplay(filename, 0);
	if (!i) return 0;
	if (i->UsesOutputPlug&IN_MODULE_FLAG_USES_OUTPUT_PLUGIN)
	{
		int t;
		for (t = 0; out_modules[t] && _stricmp(config_outname, (char *)out_modules[t]->id); t++);
		if (!out_modules[t])
		{
			LPMessageBox(hMainWindow, IDS_NOOUTPUT, IDS_ERROR, MB_OK);
			return 0;
		}

		// TODO only call out_changed(..) if we are different from before
		//		though currently changing the output prefs and playing a
		//		new track will generate a change (which is expected but
		//		it might be assumed to be wrong so may need to document)
		int changed = 0;
		if (!i->outMod || i->outMod && i->outMod->hDllInstance != out_modules[t]->hDllInstance)
		{
			changed = 1;
			if (i->outMod) out_changed(i->outMod->hDllInstance, OUT_UNSET | OUT_PLAYBACK);
		}
		i->outMod = out_modules[t];
		i->outMod->hMainWindow = hMainWindow;
		if (changed) out_changed(i->outMod->hDllInstance, OUT_SET | OUT_PLAYBACK);
	}
	else
	{
		if (i->outMod) out_changed(i->outMod->hDllInstance, OUT_UNSET | OUT_PLAYBACK);
		i->outMod = NULL;
	}
	return i;
}

void in_flush(int ms)
{
	if (in_mod && in_mod->outMod)
	{
		in_mod->outMod->Flush(ms);
	}
}

int in_getouttime(void)
{
	if (in_mod) return in_mod->GetOutputTime();
	return 0;
}

int in_getlength(void)
{
	if (in_mod)
	{
		int t = in_mod->GetLength() / 1000;
		if (t < 1
		    && t != -1) t = 1;
		return t;
	}
	return -1;
}

void in_pause(int p)
{
	if (in_mod)
	{
		if (p) in_mod->Pause();
		else in_mod->UnPause();
	}
}

void in_setvol(int v)
{
	if (in_mod)
	{
		in_mod->SetVolume(v);
	}
	if (config_eq_ws && config_eq_open) draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));

	PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_VOLUME, IPC_CB_MISC);
}

void in_setpan(int p)
{
	if (in_mod)
	{
		in_mod->SetPan(p);
	}
	if (config_eq_ws && config_eq_open) draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));
	PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_VOLUME, IPC_CB_MISC);
}

int in_seek(int time_in_ms)
{
	if (in_mod)
	{
		in_mod->SetOutputTime(time_in_ms);
	}
	return 0;
}

extern "C" int g_need_trusted_dsp = 0;

int in_open(const wchar_t *fn)
{
	int r;
	in_mod = in_setmod(fn);
	if (!in_mod) return 1;
	sa_setthread(config_sa);
	in_setvol(config_volume);
	in_setpan(config_pan);

	dsp_in_init = 0;
	dsp_init_mod = in_mod;
	g_need_trusted_dsp = 0;
	filter_srate = 0;

	r = InW_Play(in_mod, fn);
	return r;
}

void in_close(void)
{
	if (in_mod)
		in_mod->Stop();

	if (NULL != eq)
	{
		free(eq);
		eq = NULL;
	}

	timingPrint();
}

extern int ModernInfoBox(In_Module * in, const wchar_t * filename, HWND parent);
typedef int (__cdecl *UseUnifiedFileInfoDlg)(const wchar_t * fn);

int in_infobox(HWND hwnd, const wchar_t *fn)
{
	int a;
	In_Module *mod;

	const wchar_t *p = wcsstr(fn, L"aol.com/uvox");
	if ( p ) return 0;

	if (g_in_infobox) return 0;

	mod = in_setmod_noplay(fn, 0);

	if (!mod) return 0;

	g_in_infobox = mod;

	UseUnifiedFileInfoDlg uufid =	(UseUnifiedFileInfoDlg)GetProcAddress(mod->hDllInstance, "winampUseUnifiedFileInfoDlg");

	// focus often gets reverted back to the main window after this dialog
	// so we will remember what window used to have focus
	HWND oldFocus = GetFocus(); 

	if(uufid && uufid(fn))
		a = ModernInfoBox(mod, fn, hwnd);
	else
		a = InW_InfoBox(mod, fn, hwnd);

	SetFocus(oldFocus);

	if (!a)
	{
		SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)fn, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
	}

	g_in_infobox = 0;

	return a;
}
static void AddFilterString(char *p, size_t size, const char *a)
{
	while (a && *a)
	{
		char *t;
		//int adv = 1;

		StringCchCatEx(p, size, ";*.", &t, &size, 0);
		t = p + lstrlen(p);
		while (*a && *a != ';') *t++ = *a++;
		*t = 0;
		a++;

		if (a[ -1]) continue;
		if (!*a) break;
		a += lstrlen(a) + 1;
	}
}

char *in_getfltstr()
{
	int in_mod = -1;
	int in_wave = -1;
	size_t size = 256 * 1024;
	char *buf = (char*)GlobalAlloc(GPTR, size); // this is gay, should have a growing buffer or somethin.
	char *p = buf;
	size_t x;
	getString(IDS_ALLTYPES, p, size);
	p = p + lstrlen(p) + 1;
	wchar_t playlistString[1024];
	playlistManager->GetFilterList(playlistString, 1024);
	WideCharToMultiByteSZ(CP_ACP, 0, playlistString, -1, p, size, 0, 0);
	for (x = 0; x < in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		if (a && *a)
		{
			/* we want to skip in_mod and in_wave because they have TOO MANY extensions and we are limited to MAX_PATH (260)
			   we'll tack them at the end just in case we have enough room for 'em */
			if (in_mod < 0 && !strncmp(a, "mod;", 4))
				in_mod = x;
			else if (in_wave < 0 && strstr(a, "aiff"))  // detection for in_wave.  not the best but should work
				in_wave = x;
			else AddFilterString(p, size, a);
		}
	}

	/* add in_wave and in_mod last */
	if (in_wave >= 0)
		AddFilterString(p, size, in_modules[in_wave]->FileExtensions);

	if (in_mod >= 0) // fuck you in_mod :)
		AddFilterString(p, size, in_modules[in_mod]->FileExtensions);

	p = p + lstrlen(p) + 1;
	getString(IDS_PLAYLISTSTRING, p, size - (p - buf));
	p += lstrlen(p) + 1;
	playlistManager->GetFilterList(playlistString, 1024);
	WideCharToMultiByteSZ(CP_ACP, 0, playlistString, -1, p, size, 0, 0);
	p += lstrlen(p) + 1;
	for (x = 0; x < in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions, *b;
		while (a && *a)
		{
			b = a;
			a += lstrlen(a) + 1;
			if (!*a) break;
			StringCchCopyEx(p, size, a, &p, &size, 0);
			p++;
			{
				do
				{
					char *c;
					StringCchCopy(p, size, "*.");
					StringCchCat(p, size, b);
					if ((b = strstr(b, ";"))) b++;
					if ((c = strstr(p, ";"))) c[1] = 0;

					p += lstrlen(p);
				}
				while (b);
				p++;
				a += lstrlen(a) + 1;
			}
		}
	}
	StringCchCopyEx(p, size, getString(IDS_OFD_ALL_FILES,NULL,0), &p, &size, 0);
	p++; size--;
	lstrcpyn(p, "*.*", size);
	p += 3;
	*p = 0;

	{
		char *newbuf;
		size = p + 5 - buf;
		newbuf = (char*)GlobalAlloc(GPTR, size);
		memcpy(newbuf, buf, size);
		GlobalFree(buf);
		return newbuf;
	}
}

static void AddFilterStringW(wchar_t *p, size_t &size, const char *a, BOOL skip)
{
	while (*a)
	{
		wchar_t *t = 0;
		StringCchCatExW(p, size, ((*p)?L";*.":L"*."), &t, &size, 0);
		size_t extsize = 0;
		while (a[extsize] && a[extsize] != ';')
			extsize++;

		*(t += MultiByteToWideCharSZ(CP_ACP, 0, a, extsize, t, size) - 1);
		a+=extsize+1;

		if (a[ -1]) continue;
		if (!*a) break;
		a += lstrlen(a) + 1;

		// limit the length of the filter to fit into MAX_PATH otherwise
		// if things go past this then mainly for the all supported type
		// it can sometimes act like *.* due to where the filter is cut
		if(!skip && lstrlenW(p)>=MAX_PATH)
		{
			// if we end with a . then need to fake things to act like a new 
			// filter since windows will interpret the . as a *.* which is bad
			if(*(p+MAX_PATH) == L'.')
			{
				*(p+MAX_PATH-1) = 0;
				*(p+MAX_PATH) = 0;
			}
		}
	}
}

static wchar_t *inc(wchar_t *p, size_t &size, int extra=0)
{
	int len = lstrlenW(p) + extra;
	size-=len;
	p+=len;
	return p;
}

wchar_t *in_getfltstrW(BOOL skip)
{
	int in_mod = -1;
	int in_wave = -1;
	size_t size = 256 * 1024;
	wchar_t *buf = (wchar_t*)GlobalAlloc(GPTR, size * sizeof(wchar_t)); // this is gay, should have a growing buffer or somethin.
	wchar_t *p = buf, *ps;
	*p=0;
	size_t x = 0;

	{
		int cnt = lstrlenW(getStringW(IDS_ALLTYPES, buf, size))+1;
		p += cnt; size-=cnt;
		if (playlistManager)
		{
			playlistManager->GetExtensionList(p, size);
			size -= lstrlenW(p);
		}
	}
	ps = p;

	for (x = 0; x < in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		if (a && *a)
		{
			/* we want to skip in_mod and in_wave because they have TOO MANY extensions and we are limited to MAX_PATH (260)
			   we'll tack them at the end just in case we have enough room for 'em */
			if (in_mod < 0 && !strncmp(a, "mod;", 4))
				in_mod = x;
			else if (in_wave < 0 && strstr(a, "aiff"))  // detection for in_wave.  not the best but should work
				in_wave = x;
			else AddFilterStringW(p, size, a, skip);
		}
	}

	/* add in_wave and in_mod last */
	if (in_wave >= 0)
		AddFilterStringW(p, size, in_modules[in_wave]->FileExtensions, skip);

	if (in_mod >= 0) // fuck you in_mod :)
		AddFilterStringW(p, size, in_modules[in_mod]->FileExtensions, skip);

	if (*p)
		p += lstrlenW(p)+1; // don't decrement size here cause it was done already

	// uppercase the extensions so is consistent as can be
	CharUpperBuffW(ps, 256 * 1024);

	if (playlistManager)
	{
		wchar_t ext[512];
		playlistManager->GetExtensionList(ext, 512);
		StringCchPrintfW(p, size, getStringW(IDS_PLAYLISTSTRING_NEW,NULL,0),ext);
		int cnt = lstrlenW(p)+1;
		p += cnt; size-=cnt;
		StringCchCatW(p, size, ext);
		p = inc(p, size, 1);
	}

	for (x = 0; x < in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions, *b;
		while (a && *a)
		{
			b = a;
			a += lstrlen(a) + 1;
			if (!*a) break;
			// adjust down by 1 so that we have the actual string length exluding null termination
			int cnt=MultiByteToWideCharSZ(CP_ACP, 0, a, -1, p, size)-1; 
			p += cnt; size-=cnt;
			{
				do
				{
					wchar_t *c = 0;
					StringCchCopyW(p, size, L"*.");
					StringCchCatW(p, size, AutoWide(b));
					if ((b = strstr(b, ";"))) b++;
					if ((c = wcsstr(p, L";"))) c[1] = 0;

					p = inc(p, size);
				}
				while (b);
				p++;
				size--;
				a += lstrlen(a) + 1;
			}
		}
	}
	StringCchCopyExW(p, size, getStringW(IDS_OFD_ALL_FILES,NULL,0), &p, &size, 0);
	p++; size--;
	lstrcpynW(p, L"*.*", size);
	p += 3;
	*p = 0;

	{
		wchar_t *newbuf = 0;
		size = p + 5 - buf;
		newbuf = (wchar_t*)GlobalAlloc(GPTR, size * sizeof(wchar_t));
		memcpy(newbuf, buf, size*sizeof(wchar_t));
		GlobalFree(buf);
		return newbuf;
	}
}

char *in_getextlist()
{
	size_t x;
	char *mem = NULL, *p;
	int size = 1024;

	for (x = 0; x != in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		while (a && *a)
		{
			size += lstrlen(a) + 1;
			a += lstrlen(a) + 1;
			if (!*a) break;
			a += lstrlen(a) + 1;
		}
	}

	p = mem = (char*)GlobalAlloc(GPTR, size);

	for (x = 0; x != in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		while (a && *a)
		{
			char *b = a, *c;
			do
			{
				c = strstr(b, ";");
				if (c)
				{
					memcpy(p, b, c - b);
					p[c - b] = 0;
				}
				else
				{
					StringCchCopy(p, size, b);
				}
				p += lstrlen(p) + 1;
				b = c + 1;
			}
			while (c);

			a += lstrlen(a) + 1;
			if (!*a) break;
			a += lstrlen(a) + 1;
		}
	}
	*p = 0;
	return mem;
}

wchar_t *in_getextlistW()
{
	size_t x;
	wchar_t *mem = NULL, *p;
	int size = 1024;

	for (x = 0; x != in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		while (a && *a)
		{
			size += MultiByteToWideChar(CP_ACP, 0, a, -1, 0, 0);
			a += lstrlen(a) + 1;
			if (!*a) break;
			a += lstrlen(a) + 1;
		}
	}

	p = mem = (wchar_t*)GlobalAlloc(GPTR, size*sizeof(wchar_t));

	for (x = 0; x != in_modules.size(); x ++)
	{
		char *a = in_modules[x]->FileExtensions;
		while (a && *a)
		{
			char *b = a, *c;
			do
			{
				c = strstr(b, ";");
				if (c)
				{
					int count = MultiByteToWideChar(CP_ACP, 0, b, c-b, 0, 0); 
					MultiByteToWideChar(CP_ACP, 0, b, c-b, p, count); 
					p[count]=0;
					size-=(count+1);
				}
				else
				{
					int count = MultiByteToWideChar(CP_ACP, 0, b, -1, 0, 0); 
					MultiByteToWideChar(CP_ACP, 0, b, -1, p, count); 
					size-=count;
				}
				p += lstrlenW(p) + 1;
				b = c + 1;
			}
			while (c);

			a += lstrlen(a) + 1;
			if (!*a) break;
			a += lstrlen(a) + 1;
		}
	}
	*p = 0;
	return mem;
}

static void vissa_init(int maxlatency_in_ms, int srate)
{
	g_srate_exact=srate;
	int nf = MulDiv(maxlatency_in_ms*4, srate, 450000);
	//int nf = maxlatency_in_ms/5;
	sa_init(nf);
	vsa_init(nf);
	vu_init(nf, srate);
}

static void vissa_deinit()
{
	sa_deinit();
	vsa_deinit();
	vu_deinit();
}

static void setinfo(int bitrate, int srate, int stereo, int synched)
{
	int last_brate = g_brate, last_srate = g_srate, last_nch = g_nch;

	if (stereo != -1)
	{
		g_nch = stereo;
		// dynamic channels
		if (g_nch > 0)
		{

			if (NULL != eq) {free(eq); eq = NULL;}
			eq = (eq10_t*) malloc(sizeof(eq10_t) * g_nch);
			if (NULL == eq)
			{
				// bad. Need to handle this.
				g_nch = 0;
			}
			memset(eq, 0, sizeof(eq10_t) * g_nch);

			eq_set(config_use_eq, (char *)eq_tab, config_preamp);
		}
	}

	if (bitrate != -1 && srate != -1)
	{
		g_need_titleupd = 1;
	}

	if (bitrate != -1)
		g_brate = bitrate;
	if (srate != -1)
	{
		g_srate = srate;
		switch(srate)
		{
		case 11:
			g_srate_exact=11025; break;
		case 22:
			g_srate_exact=22050; break;
		case 44:
			g_srate_exact=44100; break;
		case 88:
			g_srate_exact=88200; break;
		default:
			g_srate_exact=srate*1000; break;
		}
	}

	if (bitrate != -1 || srate != -1 || stereo != -1)
	{
		static unsigned int last_t;
		unsigned int now = GetTickCount();

		//detect wrap with the first one
		if (now < last_t || now > last_t + 500 || last_brate != g_brate || last_srate != g_srate || last_nch != g_nch)
		{
			last_t = now;
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_INFO, IPC_CB_MISC);
		}
	}

	if (synched != -1 || stereo != -1)
	{
		g_need_infoupd = synched | (stereo != -1 ? 8 : 4) | (g_need_infoupd & 8);
	}
}

static int sa_getmode()
{
	if (sa_override) return 3;

	if (sa_curmode == 4 && !config_mw_open && config_pe_open)
		return 1;
	return sa_curmode;
}

static int eq_isactive()
{
	int r = dsp_isactive();
	if (r) return 1;
	if (in_mod && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() && (config_replaygain_non_rg_gain.GetFloat() != 0)) return 1;
	if (in_mod && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() && (config_replaygain_preamp.GetFloat() != 0)) return 1;
	if (filter_enabled) return 2;
	return 0;
}

static float eq_lookup1[64] = {
                                  4.000000f, 3.610166f, 3.320019f, 3.088821f, 2.896617f,
                                  2.732131f, 2.588368f, 2.460685f, 2.345845f, 2.241498f,
                                  2.145887f, 2.057660f, 1.975760f, 1.899338f, 1.827707f,
                                  1.760303f, 1.696653f, 1.636363f, 1.579094f, 1.524558f,
                                  1.472507f, 1.422724f, 1.375019f, 1.329225f, 1.285197f,
                                  1.242801f, 1.201923f, 1.162456f, 1.124306f, 1.087389f,
                                  1.051628f, 1.000000f, 0.983296f, 0.950604f, 0.918821f,
                                  0.887898f, 0.857789f, 0.828454f, 0.799853f, 0.771950f,
                                  0.744712f, 0.718108f, 0.692110f, 0.666689f, 0.641822f,
                                  0.617485f, 0.593655f, 0.570311f, 0.547435f, 0.525008f,
                                  0.503013f, 0.481433f, 0.460253f, 0.439458f, 0.419035f,
                                  0.398970f, 0.379252f, 0.359868f, 0.340807f, 0.322060f,
                                  0.303614f, 0.285462f, 0.267593f, 0.250000f
                              };

///////////////// EQ CODE /////////////////////////

static int __inline float_to_int(double a)
{
	a += ((((65536.0 * 65536.0 * 16) + (65536.0 * 0.5)) * 65536.0));
	return ((*(int *)&a) - 0x80000000);
}

#if 0

typedef struct
{
	float	a0, a1;
	float	k;
	float	scale;
	float	d1_l, d2_l;
	float	d1_r, d2_r;
}
filter_t;

static filter_t filters[10];

static void create_filter(filter_t *f, float sample_rate, float cutoff, float dampening)
{
	float	a2, c;

	c = (float)( 1.f / tan( 3.14159265359 * cutoff / sample_rate ) );
	a2 = 1.f + c * (c + dampening);

	f->a1 = 2.f * (1.f - c * c) / a2;
	f->a0 = (1.f + c * (c - dampening)) / a2;

	f->k = c * dampening / a2;

	f->d1_l = f->d2_l = f->d1_r = f->d2_r = 0.0f;
}

static void initialize_filters(int srate)
{
	int x;
	int freqs[10] = { 70, 180, 320, 600, 1000, 3000, 6000, 12000, 14000, 16000 };
	for (filter_top = 0; filter_top < 10 && freqs[filter_top]*2 <= srate; filter_top ++);
	filter_srate = srate;
	for (x = 0; x < filter_top; x ++)
	{
		create_filter(&filters[x], (float)srate, (float)freqs[x], 0.68f);
	}
}

static int eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate)
{
	char *csamples = (char *)samples;
	short *oldsamples = samples;
	if (filter_srate != srate) initialize_filters(srate);

	if (filter_enabled && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ))
	{
		int r = numsamples;
		if (bps == 16 && nch == 2) while (r--)
			{
				float os1 = (float)samples[0], os2 = (float)samples[1], a = 0.0f, b = 0.0f;
				int x;
				filter_t *f = filters;
				timingEnter(0);
				for (x = 0; x < filter_top; x ++)
				{
					float d0_l, d0_r;

					d0_l = f->k * os1 - (f->a1 * f->d1_l + f->a0 * f->d2_l);
					d0_r = f->k * os2 - (f->a1 * f->d1_r + f->a0 * f->d2_r);
					a += f->scale * (d0_l - f->d2_l);
					b += f->scale * (d0_r - f->d2_r);
					f->d2_l = f->d1_l;
					f->d2_r = f->d1_r;
					f->d1_l = d0_l;
					f->d1_r = d0_r;
					f++;
				}
				{
					int i, j;
					i = float_to_int(a);
					j = float_to_int(b);
					samples[0] = max(min(i, 32767), -32768);
					samples[1] = max(min(j, 32767), -32768);
					samples += 2;
				}
				timingLeave(0);
			}
		else if (bps == 16 && nch == 1) while (r--)
			{
				float os1 = (float)samples[0], a = 0.0f;
				int x;
				filter_t *f = filters;
				for (x = 0; x < filter_top; x ++)
				{
					float d0_l;

					d0_l = f->k * os1 - (f->a1 * f->d1_l + f->a0 * f->d2_l);
					a += f->scale * (d0_l - f->d2_l);
					f->d2_l = f->d1_l;
					f->d1_l = d0_l;
					f++;
				}
				{
					int i;
					i = float_to_int(a);
					samples[0] = max(min(i, 32767), -32768);
				}
				samples++;
			}
	}
	return dsp_dosamples(oldsamples, numsamples, bps, nch, srate);
}

void eq_set(int on, char data[10], int preamp)
{
	int x;
	if (in_mod && in_mod->EQSet && (in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ))
		in_mod->EQSet(on, data, preamp);
	for (x = 0; x < 10 && data[x] == 31; x ++);
	if (!on || (preamp == 31 && x == 10))
	{
		filter_enabled = 0;
	}
	else filter_enabled = 1;
	for (x = 0; x < 10; x ++) filters[x].scale = eq_lookup1[data[x]] * eq_lookup1[preamp];
}
#endif

#if 0

typedef struct
{
	double	a0, a1;
	double	k;
	double	scale;
	double	d1_l, d2_l;
	double	d1_r, d2_r;
}
filter_t;

static filter_t filters[10];

static void create_filter(filter_t *f, float sample_rate, float cutoff, float dampening)
{
	double a2, c;

	c = ( 1.0 / tan( 3.14159265359 * cutoff / sample_rate ) );
	a2 = 1.0 + c * (c + dampening);

	f->a1 = 2.0 * (1.0 - c * c) / a2;
	f->a0 = (1.0 + c * (c - dampening)) / a2;
	f->k = 1.0 / a2;
	f->d1_l = f->d2_l = f->d1_r = f->d2_r = 0.0;
}

static void initialize_filters(int srate)
{
	int x;
	int freqs[10] = { 100, 220, 400, 800, 1800, 3000, 6000, 10000, 11000, 14000 };
	for (filter_top = 0; filter_top < 10 && freqs[filter_top]*2 <= srate; filter_top ++);
	filter_srate = srate;
	for (x = 0; x < filter_top; x ++)
	{
		create_filter(&filters[x], (float)srate, (float)freqs[x], 2.0f);
	}
}

static int eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate)
{
	char *csamples = (char *)samples;
	short *oldsamples = samples;
	if (!samples && !numsamples && !bps && nch == 0xdeadbeef)
	{
		int t = 37 + (((dsp_init_mod) >> 8) & 0x0f);
		g_need_trusted_dsp = 1;
		return (((((srate + 66191213)*1103515245) + 13293)&0x7FFFFFFF) ^ t);
	}
	if (filter_srate != srate) initialize_filters(srate);

	if (filter_enabled && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ))
	{
		int r = numsamples;
		if (bps == 16 && nch == 2) while (r--)
			{
				double os_l = (double)samples[0] + 0.5, os_r = (double)samples[1] + 0.5,
							  a = 0.0, b = 0.0, l_accum = 0.0, r_accum = 0.0;
				int x = filter_top;
				filter_t *f = filters;
				while (x--)
				{
					double d0_l, d0_r, y_l, y_r;
					d0_l = f->k * os_l - f->a1 * f->d1_l - f->a0 * f->d2_l;
					d0_r = f->k * os_r - f->a1 * f->d1_r - f->a0 * f->d2_r;
					y_l = d0_l + f->d1_l + f->d1_l + f->d2_l;
					y_r = d0_r + f->d1_r + f->d1_r + f->d2_r;
					f->d2_l = f->d1_l;
					f->d2_r = f->d1_r;
					f->d1_l = d0_l;
					f->d1_r = d0_r;
					a += (y_l - l_accum) * f->scale;
					b += (y_r - r_accum) * f->scale;
					l_accum = y_l;
					r_accum = y_r;
					f++;
				}
				{
					int i, j;
					i = float_to_int(a);
					j = float_to_int(b);
					samples[0] = max(min(i, 32767), -32768);
					samples[1] = max(min(j, 32767), -32768);
					samples += 2;
				}
			}
		else if (bps == 16 && nch == 1) while (r--)
			{
				double os1 = (double)samples[0] + 0.5,
							 a = 0.0, l_accum = 0.0;
				int x = filter_top;
				filter_t *f = filters;
				while (x--)
				{
					double d0_l, y_l;
					d0_l = f->k * os1 - f->a1 * f->d1_l - f->a0 * f->d2_l;
					y_l = d0_l + f->d1_l + f->d1_l + f->d2_l;
					f->d2_l = f->d1_l;
					f->d1_l = d0_l;
					a += (y_l - l_accum) * f->scale;
					l_accum = y_l;
					f++;
				}
				{
					int i;
					i = float_to_int(a);
					samples[0] = max(min(i, 32767), -32768);
					samples++;
				}
			}
	}
	return dsp_dosamples(oldsamples, numsamples, bps, nch, srate);
}

void eq_set(int on, char data[10], int preamp)
{
	int x;
	if (in_mod && in_mod->EQSet && (in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ))
		in_mod->EQSet(on, data, preamp);
	for (x = 0; x < 10 && data[x] == 31; x ++);
	if (!on || (preamp == 31 && x == 10))
	{
		filter_enabled = 0;
	}
	else filter_enabled = 1;
	for (x = 0; x < 10; x ++) filters[x].scale = eq_lookup1[data[x]] * eq_lookup1[preamp];
}
#endif

float *splbuf;
int splbuf_alloc;
float eqt[10];

static void FillFloat(float *floatBuf, void *samples, size_t bps, size_t numSamples, size_t numChannels, float preamp)
{
	nsutil_pcm_IntToFloat_Interleaved_Gain(floatBuf, samples, bps, numSamples*numChannels, preamp);
}

static void FillSamples(void *samples, float *floatBuf, size_t bps, size_t numSamples, size_t numChannels)
{
	nsutil_pcm_FloatToInt_Interleaved(samples, floatBuf, bps, numSamples*numChannels);
}

static float NonReplayGainAdjust()
{
	if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool())
	   return pow(10.0f, (float)config_replaygain_non_rg_gain/20.0f);
	else
		return 1.0f; 
}

static float ReplayGainPreamp()
{
	if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool())
	   return pow(10.0f, (float)config_replaygain_preamp/20.0f);
	else
		return 1.0f; 
}

static int eq_dosamples_4front(short *samples, int numsamples, int bps, int nch, int srate)
{
	g_srate_exact=srate;
	//char *csamples = (char *)samples;
	short *oldsamples = samples;
	if (!samples && !numsamples && !bps && nch == 0xdeadbeef)
	{
		int t = 37 + ((((int)dsp_init_mod) >> 8) & 0x0f);
		g_need_trusted_dsp = 1;
		return (((((srate + 66191213)*1103515245) + 13293)&0x7FFFFFFF) ^ t);
	}

	if (filter_enabled && in_mod && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ))
	{
		if (!eq || filter_srate != srate)
		{
			int x;
			if (!eq)
			{
				eq = (eq10_t*) malloc(sizeof(eq10_t) * nch);
			}
			memset(eq, 0, sizeof(eq10_t) * nch);
			eq10_setup(eq, nch, (float)srate); // initialize
			for (x = 0; x < 10; x ++)
				eq10_setgain(eq, nch, x, eqt[x]);
			filter_srate = srate;
		}
		if (splbuf_alloc < numsamples*nch) 
		{
			splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * (splbuf_alloc = numsamples * nch));
		}
		if (splbuf)
		{
			int x, y = nch * numsamples;
			FillFloat(splbuf, samples, bps, numsamples, nch, preamp_val*NonReplayGainAdjust()*ReplayGainPreamp());
			for (x = 0; x < nch; x ++)
			{
				eq10_processf(eq + x, splbuf, splbuf + y, numsamples, x, nch);
			}
			FillSamples(samples, splbuf+y, bps, numsamples, nch);

		}
	}
	else if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() && (config_replaygain_non_rg_gain.GetFloat() != 0))
	{
		if (splbuf_alloc < numsamples*nch) 
		{
			splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * (splbuf_alloc = numsamples * nch));
		}
		if (splbuf)
		{
			FillFloat(splbuf, samples, bps, numsamples, nch, NonReplayGainAdjust()*ReplayGainPreamp());
			FillSamples(samples, splbuf, bps, numsamples, nch);
		}
	}
	else if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() && (config_replaygain_preamp.GetFloat() != 0))
	{
		if (splbuf_alloc < numsamples*nch) 
		{
			splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * (splbuf_alloc = numsamples * nch));
		}
		if (splbuf)
		{
			FillFloat(splbuf, samples, bps, numsamples, nch, ReplayGainPreamp());
			FillSamples(samples, splbuf, bps, numsamples, nch);
		}
	}
	else
		filter_srate = 0;
	return dsp_dosamples(oldsamples, numsamples, bps, nch, srate);
}

static __inline double VALTODB(int v)
{
	v -= 31;
	if (v < -31) v = -31;
	if (v > 32) v = 32;

	if (v > 0) return -12.0*(v / 32.0);
	else if (v < 0)
	{
		return -12.0*(v / 31.0);
	}
	return 0.0f;
}

static void eq_set_4front(char data[10])
{
	if (!eq)
		return ;

	for (int x = 0; x < 10; x ++)
	{
		eqt[x] = (float)VALTODB(data[x]);
		if (filter_srate) eq10_setgain(eq, g_nch, x, eqt[x]);
	}
}

static bool eq_do_first=true;
static int startup_config_eq_type=EQ_TYPE_4FRONT;
void benskiQ_eq_set(char data[10]);
void eq_set(int on, char data[10], int preamp)
{
	if (eq_do_first)
	{
		startup_config_eq_type=config_eq_type;
		eq_do_first=false;
	}

	int x;
	if (in_mod && in_mod->EQSet) in_mod->EQSet(on, data, preamp);

	for (x = 9; x >= 0 && data[x] == 31; x++);
	if (x >= 0)
		filter_top2=x;

	for (x = 0; x < 10 && data[x] == 31; x ++);
	if (!on || (preamp == 31 && x == 10))
	{
		filter_enabled = 0;
	}
	else filter_enabled = 1;
	preamp_val = (float)eq_lookup1[preamp];

	if (startup_config_eq_type == EQ_TYPE_4FRONT)
		eq_set_4front(data);
	if (startup_config_eq_type == EQ_TYPE_CONSTANT_Q)
		benskiQ_eq_set(data);
}

static int eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate)
{
	if (eq_do_first)
	{
		startup_config_eq_type=config_eq_type;
		eq_do_first=false;
	}
	if (startup_config_eq_type == EQ_TYPE_4FRONT)
		return eq_dosamples_4front(samples, numsamples, bps, nch, srate);
	else
		return benskiQ_eq_dosamples(samples, numsamples, bps, nch, srate);
}