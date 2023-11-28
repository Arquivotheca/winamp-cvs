#include "Main.h"
#include "../nu/AutoChar.h"

Out_Module *out_modules[32];
Out_Module *out_mod;

void out_init(void)
{
	HANDLE h;
	WIN32_FIND_DATAW d;
	int x=0;
	wchar_t dirstr[MAX_PATH];
	PathCombineW(dirstr, PLUGINDIR, L"OUT_*.DLL");
	h = FindFirstFileW(dirstr,&d);
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
				Out_Module * (*pr)();
				Out_Module *mod;
				pr = (Out_Module *(*)()) GetProcAddress(hLib,"winampGetOutModule");
				if (pr)
				{
					mod = pr();
					if (mod && (mod->version == OUT_VER || mod->version == OUT_VER_U))
					{
						AutoChar narrowFn(d.cFileName);
						size_t fileNameSize = lstrlen(narrowFn);

						if (g_safeMode)
						{
							if (!(mod->id == 1471482036 || mod->id == 426119909 || mod->id == 203968848))
							{
								FreeModule(hLib);
								continue;
							}
						}

						mod->hDllInstance = hLib;
						mod->hMainWindow = hMainWindow;
						mod->id=(intptr_t)GlobalAlloc(GPTR,fileNameSize + 1);
						StringCchCopy((char *)mod->id, fileNameSize+1, narrowFn);
						mod->Init();
						out_modules[x++] = mod;
					}
				}
			}
		} while (FindNextFileW(h,&d) && x < 31);
		FindClose(h);
	}
}

void out_setwnd(void) {
	int x;
	for (x = 0; out_modules[x]; x ++)
	{
		out_modules[x]->hMainWindow = hMainWindow;
	}
}

void out_changed(HINSTANCE hLib, int enabled) {

	typedef void (__cdecl *OutModeChange)(int);
	OutModeChange modeChange = (OutModeChange)GetProcAddress(hLib, "winampGetOutModeChange");
	if (modeChange)
	{
		modeChange(enabled);
	}
}

void out_deinit(void)
{
	int x;
	for (x = 0; out_modules[x]; x ++)
	{
		//HINSTANCE hLib = out_modules[x]->hDllInstance;
		out_modules[x]->Quit();
		GlobalFree((HGLOBAL) out_modules[x]->id);
		out_modules[x]->id=0;
		//FreeLibrary(hLib); // benski> we're just going to let it leak because it might be subclassing
		out_modules[x]=0;
	}
}