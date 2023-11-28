#include "main.h"
#include "gen.h"
#include "PtrList.h"
#include "../nu/AutoWide.h"
nu::PtrList<winampGeneralPurposePlugin> gen_plugins;
int got_ml = 0;

typedef struct _PLUGINORDER
{
	LPCWSTR	name;
	bool	found;
} PLUGINORDER;
static PLUGINORDER preload[] =
{
	{ L"gen_crasher.dll", false },
	{ L"gen_hotkeys.dll", false },
	{ L"gen_ml.dll", false },
};

void LoadPlugin(const wchar_t *filename)
{
	wchar_t temp[MAX_PATH];
	PathCombineW(temp, PLUGINDIR, filename);
	HINSTANCE hLib = LoadLibraryW(temp);
	if (hLib)
	{
		winampGeneralPurposePluginGetter pr;
		pr = (winampGeneralPurposePluginGetter) GetProcAddress(hLib,"winampGetGeneralPurposePlugin");
		if (pr)
		{
			winampGeneralPurposePlugin *plugin = pr();
			if (plugin && (plugin->version == GPPHDR_VER || plugin->version == GPPHDR_VER_U))
			{
				if (g_safeMode)
				{
					char desc[128];
					lstrcpyn(desc, plugin->description, sizeof(desc));
					if (desc[0] && !memcmp(desc, "nullsoft(", 9))
					{
						char* p = strrchr(desc, ')');
						if (p)
						{
							*p = 0;
							if(_wcsicmp(filename, AutoWide(desc+9)))
							{
								FreeModule(hLib);
								return;
							}
						}
					}
					else
					{
						FreeModule(hLib);
						return;
					}
				}
				plugin->hwndParent=hMainWindow;
				plugin->hDllInstance=hLib;
				if (plugin->init() == GEN_INIT_SUCCESS)
				{
					if(!_wcsicmp(filename, L"gen_ml.dll")) got_ml = 1;
					gen_plugins.push_back(plugin);
				}
				else 
					FreeModule(hLib);
			} 
			else FreeModule(hLib);
		} 
		else FreeModule(hLib);
	}
}

void load_genplugins(void)
{
	int i = 0, count = sizeof(preload)/sizeof(PLUGINORDER);
	for (; i < count; i++) LoadPlugin(preload[i].name);

	wchar_t dirstr[MAX_PATH];
	HANDLE h = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAW d = {0};
	PathCombineW(dirstr, PLUGINDIR, L"GEN_*.DLL");

	h = FindFirstFileW(dirstr,&d);
	if (h != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			for (i = 0 ; i < count && (preload[i].found || lstrcmpiW(preload[i].name, d.cFileName)); i++);
			if (i == count) LoadPlugin(d.cFileName);
			else preload[i].found = true;
		}
		while (FindNextFileW(h,&d));
		FindClose(h);
	}
}

void unload_genplugins(void)
{
	size_t x=gen_plugins.size();
	while (x--)
	{
		if(gen_plugins[x])
		{
			if (!(config_no_visseh&4))
			{
				try 
				{
					if (gen_plugins[x]->quit)
						gen_plugins[x]->quit();
				}
				catch(...)
				{
				}
			}
			else
			{
				if (gen_plugins[x]->quit)
					gen_plugins[x]->quit();
			}
			gen_plugins[x]=0;
		}
	}
	try 
	{
		gen_plugins.clear();
	}
	catch(...)
	{
	}
}