#include "pluginloader.h"
#include "../winamp/wa_ipc.h"
#include "../nu/AutoWide.h"
#include <shlwapi.h>

extern winampMediaLibraryPlugin plugin;

C_ItemList m_plugins;

extern HWND mainMessageWindow;

int wmDeviceChange(WPARAM wParam, LPARAM lParam) {
	int ret=0;
	for(int i=0; i < m_plugins.GetSize(); i++) {
		PMPDevicePlugin * plugin = (PMPDevicePlugin *)m_plugins.Get(i);
    /*
		if(plugin->wmDeviceChange)
		{
			if(plugin->wmDeviceChange(wParam, lParam) == BROADCAST_QUERY_DENY)
				ret = BROADCAST_QUERY_DENY;
		}
    */
		if(plugin->MessageProc)
		{
			if(plugin->MessageProc(PMP_DEVICECHANGE,wParam,lParam,0) == BROADCAST_QUERY_DENY)
				ret = BROADCAST_QUERY_DENY;
		}
	}
	return ret;
}

PMPDevicePlugin * loadPlugin(wchar_t * file)
{
	HINSTANCE m=LoadLibrary(file);
	if(m)
	{
		PMPDevicePlugin *(*gp)();
		gp=(PMPDevicePlugin *(__cdecl *)(void))GetProcAddress(m,"winampGetPMPDevicePlugin");
		if(!gp)
		{
			FreeLibrary(m);
			return NULL;
		}

		PMPDevicePlugin *devplugin=gp();
		if(!devplugin || devplugin->version!=PMPHDR_VER)
		{
			FreeLibrary(m);
			return NULL;
		}

		devplugin->hDllInstance=m;
		devplugin->hwndLibraryParent=plugin.hwndLibraryParent;
		devplugin->hwndWinampParent=plugin.hwndWinampParent;
		devplugin->hwndPortablesParent=mainMessageWindow;

		if(devplugin->init())
		{
			FreeLibrary(m);
		}
		else
		{
			m_plugins.Add((void *)devplugin);
			return devplugin;
		}
	}
	return NULL;
}

BOOL loadDevPlugins()
{
	BOOL loaded = FALSE;
	wchar_t tofind[MAX_PATH];
	char * charDir = (char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETPLUGINDIRECTORY);
	AutoWide dir(charDir);
	PathCombine(tofind, dir, L"pmp_*.dll"); 

	WIN32_FIND_DATA d;
	HANDLE h =	FindFirstFile(tofind,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t file[MAX_PATH];
			PathCombine(file, dir, d.cFileName);
			loaded += (!!loadPlugin(file));
		}
		while(FindNextFile(h,&d));
		FindClose(h);
	}
	return loaded;
}

void unloadPlugin(PMPDevicePlugin *devplugin, int n=-1) {
	if(n == -1) for(int i=0; i<m_plugins.GetSize(); i++) if(m_plugins.Get(i) == (void*)devplugin) n=i;
	devplugin->quit();
	//if (devplugin->hDllInstance) FreeLibrary(devplugin->hDllInstance);
	m_plugins.Del(n);
}

void unloadDevPlugins()
{
	int i=m_plugins.GetSize();
	while (i-->0)  // reverse order to aid in not fucking up subclassing shit
	{
	    PMPDevicePlugin *devplugin=(PMPDevicePlugin *)m_plugins.Get(i);
		unloadPlugin(devplugin,i);
	}
}