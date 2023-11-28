#include "Main.h"
#include "api_wa5component.h"
#include "PtrList.h"
#include "api.h"
#include "LazyServiceFactory.h"

nu::PtrList<ifc_wa5component> systemComponents;
nu::PtrList<LazyServiceFactory> lazyFactories;

enum
{
	W5S_LOAD = 0,
	W5S_LAZYLOAD = 1,
};

static uint32_t magic_word = 0xdeadbeefUL;
/* layout (binary)
0xdeadbeef - 32 bits
service guid - 128 bits
service fourcc - 32 bits
length of service name - 16bits
service name - see previous
length of test string - 16 bits
test string - see previous
repeat as necessary
*/
static int w5s_load_binary_manifest(const wchar_t *filename, const wchar_t *w5s_filename)
{
	HANDLE manifest = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (manifest != INVALID_HANDLE_VALUE)
	{
		for(;;)
		{
			uint32_t manifest_magic_word;
			GUID service_guid;
			FOURCC service_fourcc;

			DWORD bytesRead=0;
			ReadFile(manifest, &manifest_magic_word, sizeof(manifest_magic_word), &bytesRead, NULL);
			if (bytesRead == 0) // EOF
			{
				CloseHandle(manifest);
				return W5S_LAZYLOAD;
			}

			if (bytesRead != sizeof(manifest_magic_word) || memcmp(&manifest_magic_word, &magic_word, sizeof(magic_word)))
				break;

			bytesRead=0;
			ReadFile(manifest, &service_guid, sizeof(service_guid), &bytesRead, NULL);
			if (bytesRead != sizeof(service_guid))
				break;

			bytesRead=0;
			ReadFile(manifest, &service_fourcc, sizeof(service_fourcc), &bytesRead, NULL);
			if (bytesRead != sizeof(service_fourcc))
				break;

			uint16_t service_name_length;
			bytesRead=0;
			ReadFile(manifest, &service_name_length, sizeof(service_name_length), &bytesRead, NULL);
			if (bytesRead != sizeof(service_name_length))
				break;

			char *service_name = 0;
			if (service_name_length)
			{
				service_name = (char *)malloc(service_name_length + 1);
				if (service_name)
				{
					service_name[service_name_length]=0;
					bytesRead=0;
					ReadFile(manifest, service_name, service_name_length, &bytesRead, NULL);
					if (bytesRead != service_name_length)
					{
						free(service_name);
						break;
					}
				}
			}

			uint16_t service_test_string_length;
			bytesRead=0;
			ReadFile(manifest, &service_test_string_length, sizeof(service_test_string_length), &bytesRead, NULL);
			if (bytesRead != sizeof(service_test_string_length))
				break;

			char *service_test_string = 0;
			if (service_test_string_length)
			{
				service_test_string = (char *)malloc(service_test_string_length + 1);
				if (service_name)
				{
					service_test_string[service_test_string_length]=0;
					bytesRead=0;
					ReadFile(manifest, service_test_string, service_test_string_length, &bytesRead, NULL);
					if (bytesRead != service_test_string_length)
					{
						free(service_name);
						free(service_test_string);
						break;
					}
				}
			}

			// if we got here, we're OK :)
			LazyServiceFactory *factory = new LazyServiceFactory(service_fourcc, service_guid, service_name, service_test_string, w5s_filename);
			lazyFactories.push_back(factory);
			WASABI_API_SVC->service_register(factory);
		}

		// file seems to be malformed, go ahead and load w5s.
		// any lazy factories we already loaded will self-destruct when the real services load
		CloseHandle(manifest);
		return W5S_LOAD; 
	}

	return W5S_LOAD;
}

void w5s_load(const wchar_t *filename)
{
	HMODULE hLib = LoadLibraryW(filename);
	if (hLib)
	{
		typedef ifc_wa5component *(*W5SGetter)();
		W5SGetter pr;
		ifc_wa5component *mod;
		pr = (W5SGetter)GetProcAddress(hLib,"GetWinamp5SystemComponent");
		if (pr)
		{
			mod = pr();
			if (mod)
			{
				if (g_safeMode)
				{
					int retval = 0;
					mod->_dispatch(15, &retval);
					if (!retval)
					{
						FreeLibrary(hLib);
						return;
					}
				}

				systemComponents.push_back(mod);
				mod->hModule = hLib;
				mod->RegisterServices(WASABI_API_SVC);
			}
		}
	}
}

void w5s_init()
{
	HANDLE h;
	WIN32_FIND_DATAW d;
	wchar_t dirstr[MAX_PATH];

	PathCombineW(dirstr, SYSPLUGINDIR, L"*.W5S");
	h = FindFirstFileW(dirstr,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			// due to how this plug-in works, is better to do a filename check to not load in
			// safe mode as it otherwise causes the FreeLibrary(..) call to crash Winamp :o(
			if (g_safeMode && !wcsnicmp(d.cFileName, L"UnicodeTaskbarFix.w5s", 21)) continue;

			wchar_t manifeststr[MAX_PATH], namestr[MAX_PATH];
			PathCombineW(manifeststr, SYSPLUGINDIR, d.cFileName);
			PathRemoveExtensionW(manifeststr);
			PathAddExtensionW(manifeststr, L".wbm");
			PathCombineW(namestr, SYSPLUGINDIR, d.cFileName);
			if (w5s_load_binary_manifest(manifeststr, namestr) == W5S_LOAD)
			{
				w5s_load(namestr);
			}
		}
		while (FindNextFileW(h, &d));
		FindClose(h);
	}

	Wasabi_FindSystemServices();
}

void w5s_deinit()
{
	Wasabi_ForgetSystemServices();
	for (size_t i=0;i!=systemComponents.size();i++)
	{
		systemComponents[i]->DeregisterServices(WASABI_API_SVC);
		systemComponents[i]=0;
	}
	systemComponents.clear();
	lazyFactories.deleteAll();
}