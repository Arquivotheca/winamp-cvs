#include "FileTypeRegistrar.h"
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#define _STD_USING
//#include "../nu/cast64.h"
#include <strsafe.h>
#ifdef ELEVATOR_SCOPE
void Lock();
void UnLock();
#endif

FileTypeRegistrar::FileTypeRegistrar()
{
	refCount = 0;
}

HRESULT FileTypeRegistrar::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
	if (!ppvObj)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = static_cast<IUnknown *>(this);
	else if (IsEqualIID(riid, __uuidof(IFileTypeRegistrar)))
		*ppvObj = static_cast<IFileTypeRegistrar *>(this);
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG FileTypeRegistrar::AddRef(void)
{
#ifdef ELEVATOR_SCOPE
	Lock();
#endif
	return ++refCount;
}

ULONG FileTypeRegistrar::Release(void)
{
	ULONG retCount=--refCount;
#ifdef ELEVATOR_SCOPE
	UnLock();
#endif
	return retCount;
}

static LONG SetValue(HKEY hkey, const wchar_t *data)
{
	return RegSetValueExW(hkey, NULL,0,REG_SZ,(const BYTE *)data, (DWORD)sizeof(wchar_t)*(wcslen(data)+1));
}

static LONG SetValue(HKEY hkey, const wchar_t *value, const wchar_t *data)
{
	return RegSetValueExW(hkey, value,0,REG_SZ,(const BYTE *)data, (DWORD)sizeof(wchar_t)*(wcslen(data)+1));
}

HRESULT FileTypeRegistrar::RegisterMIMEType(const wchar_t *mimeType, const wchar_t *programName, const wchar_t *extension, int netscapeOnly)
{
	HKEY mp3Key;

	if (!netscapeOnly)
	{
		wchar_t s[MAX_PATH];
		if (RegCreateKeyW(HKEY_CLASSES_ROOT, extension, &mp3Key) == ERROR_SUCCESS)
		{
			SetValue(mp3Key, L"Content Type", mimeType);
			RegCloseKey(mp3Key);
		}
		StringCchPrintfW(s, MAX_PATH, L"MIME\\Database\\Content Type\\%s", mimeType);
		if (RegCreateKeyW(HKEY_CLASSES_ROOT, s, &mp3Key) == ERROR_SUCCESS)
		{
			RegDeleteValueW(mp3Key, L"CLSID");
			SetValue(mp3Key, L"Extension", extension);
			RegCloseKey(mp3Key);
		}
	}
	if (RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\Netscape\\Netscape Navigator\\Viewers", &mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, mimeType, programName);
		for (int x = 0; x < 999; x ++)
		{
			wchar_t st[100];
			DWORD vt;
			DWORD s = 128;
			wchar_t b[128];
			StringCchPrintfW(st, 100, L"TYPE%d", x);
			if (RegQueryValueExW(mp3Key, st, 0, &vt, (LPBYTE)b, &s) == ERROR_SUCCESS)
			{
				if (!wcscmp(b, mimeType)) break;
			}
			else
			{
				SetValue(mp3Key, st, mimeType);
				break;
			}
		}
		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterCDPlayer(const wchar_t *programName)
{
	wchar_t b[MAX_PATH];
	wchar_t buf2[MAX_PATH];
	HKEY mp3Key;
	StringCchPrintfW(buf2, MAX_PATH, L"\"%s\" %%1", programName);
	if (RegOpenKeyW(HKEY_CLASSES_ROOT, L"AudioCD\\shell\\play\\command", &mp3Key) == ERROR_SUCCESS)
	{
		DWORD s = sizeof(b)/sizeof(*b);
		if (RegQueryValueEx(mp3Key, NULL, 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
		{
			if (_wcsicmp(b, buf2))
			{
				wchar_t buf3[MAX_PATH];
				DWORD st = sizeof(buf3)/sizeof(*buf3);
				if (RegQueryValueExW(mp3Key, L"Winamp_Back", 0, NULL, (LPBYTE)buf3, &st) != ERROR_SUCCESS ||
					_wcsicmp(buf3, b))
				{
					SetValue(mp3Key, L"Winamp_Back", b);
				}
				SetValue(mp3Key, buf2);
			}
		}
		else SetValue(mp3Key, buf2);

		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::UnregisterCDPlayer(const wchar_t *programName)
{
	wchar_t b[MAX_PATH];
	wchar_t buf2[MAX_PATH];
	HKEY mp3Key;
	StringCchPrintfW(buf2, MAX_PATH, L"\"%s\" %%1", programName);
	if (RegOpenKeyW(HKEY_CLASSES_ROOT, L"AudioCD\\shell\\play\\command", &mp3Key) == ERROR_SUCCESS)
	{
		DWORD s = sizeof(b)/sizeof(*b);
		if (RegQueryValueEx(mp3Key, NULL, 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
		{
			if (!wcscmp(b, buf2))
			{
				s = sizeof(b);
				if (RegQueryValueExW(mp3Key, L"Winamp_Back", 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
				{
					if (!_wcsicmp(b, buf2)) b[0] = 0;
					if (SetValue(mp3Key, b) == ERROR_SUCCESS)
						RegDeleteValueW(mp3Key, L"Winamp_Back");
				}
				else
				{
					buf2[0] = 0;
					SetValue(mp3Key, buf2);
				}
			}
		}

		RegCloseKey(mp3Key);
	}
	return S_OK;
}


static LONG myRegDeleteKeyEx(HKEY thiskey, LPCWSTR lpSubKey)
{
	HKEY key;
	int retval = RegOpenKeyW(thiskey, lpSubKey, &key);
	if (retval == ERROR_SUCCESS)
	{
		wchar_t buffer[1024];
		while (RegEnumKeyW(key, 0, buffer, 1024) == ERROR_SUCCESS)
			if ((retval = myRegDeleteKeyEx(key, buffer)) != ERROR_SUCCESS) break;
		RegCloseKey(key);
		retval = RegDeleteKeyW(thiskey, lpSubKey);
	}
	return retval;
}


HRESULT FileTypeRegistrar::RegisterType(const wchar_t *extension, const wchar_t *which_str, const wchar_t *prog_name)
{
	HKEY mp3Key;
	LONG regResult;
	regResult = RegCreateKeyW(HKEY_CLASSES_ROOT, extension, &mp3Key);

	wchar_t b[128];
	if (regResult == ERROR_SUCCESS)
	{
		DWORD s = sizeof(b)/sizeof(*b);
		if (RegQueryValueEx(mp3Key, NULL, 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
		{
			if (wcsncmp(b, which_str, wcslen(b)))
			{
				SetValue(mp3Key, L"Winamp_Back", b);
				SetValue(mp3Key, which_str);
			}
		}
		else SetValue(mp3Key, which_str);


		if (mp3Key) RegCloseKey(mp3Key);
	}
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion >= 6) // Vista
	{
		TCHAR szKey[256];
		StringCbPrintfW(szKey, sizeof(szKey), L"Software\\Clients\\Media\\%s\\Capabilities\\FileAssociations", prog_name);
		if (ERROR_SUCCESS == RegCreateKeyW(HKEY_LOCAL_MACHINE, szKey, &mp3Key))
		{
			SetValue(mp3Key, extension, which_str);
			// TODO: cache IApplicationAssociationRegistration
			IApplicationAssociationRegistration* pAAR;

			HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
				NULL,
				CLSCTX_INPROC,
				__uuidof(IApplicationAssociationRegistration),
				(void**)&pAAR);
			if (SUCCEEDED(hr))
			{
				hr = pAAR->SetAppAsDefault(prog_name, extension, AT_FILEEXTENSION);
				pAAR->Release();
			}

			RegCloseKey(mp3Key);
		}
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::UnregisterType(const wchar_t *extension, const wchar_t *which_str, const wchar_t *prog_name, int is_playlist)
{
	HKEY mp3Key;

	LONG regResult;
	regResult = RegOpenKeyW(HKEY_CLASSES_ROOT, extension, &mp3Key);

	wchar_t b[128];
	if (regResult == ERROR_SUCCESS)
	{
		DWORD s = sizeof(b)/sizeof(*b);
		if (RegQueryValueEx(mp3Key, NULL, 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
		{
			size_t len = min(wcslen(b), wcslen(which_str));
			if (!wcsncmp(b, which_str, len))
			{
				s = sizeof(b)/sizeof(*b);
				if (RegQueryValueExW(mp3Key, L"Winamp_Back", 0, NULL, (LPBYTE)b, &s) == ERROR_SUCCESS)
				{
					if (SetValue(mp3Key, b) == ERROR_SUCCESS)
						RegDeleteValueW(mp3Key, L"Winamp_Back");
				}
				else
				{
					RegDeleteValue(mp3Key, NULL);
					RegCloseKey(mp3Key);
					mp3Key = NULL;
					myRegDeleteKeyEx(HKEY_CLASSES_ROOT, extension);
				}
			}
		}

		if (mp3Key) RegCloseKey(mp3Key);

		if(!is_playlist)
		{
			myRegDeleteKeyEx(HKEY_CLASSES_ROOT, which_str);
		}
	}
	// TODO: benski> it'd be nice to have a function to remove any capabilities for which we no longer have a plugin for ...
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion >= 6) // Vista
	{
		TCHAR szKey[256];
		StringCbPrintfW(szKey, sizeof(szKey), L"Software\\Clients\\Media\\%s\\Capabilities\\FileAssociations", prog_name);
		if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &mp3Key))
		{
			RegDeleteValue(mp3Key, extension);
			RegCloseKey(mp3Key);
		}
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::AddDirectoryContext(const wchar_t *commandLine, const wchar_t *which_str, const wchar_t *description)
{
	HKEY mp3Key;
	wchar_t regStr[256];
	StringCbPrintfW(regStr, sizeof(regStr), L"SOFTWARE\\Classes\\Directory\\shell\\%s", which_str);
	if (RegCreateKeyW(HKEY_LOCAL_MACHINE, regStr,&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, description);
		RegCloseKey(mp3Key);
	}

	StringCbPrintfW(regStr, sizeof(regStr), L"SOFTWARE\\Classes\\Directory\\shell\\%s\\command", which_str);
	if (RegCreateKeyW(HKEY_LOCAL_MACHINE,regStr,&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, commandLine);
		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::RemoveDirectoryContext(const wchar_t *which_str)
{
	wchar_t regStr[256];
	StringCbPrintfW(regStr, sizeof(regStr), L"SOFTWARE\\Classes\\Directory\\shell\\%s", which_str);
	myRegDeleteKeyEx(HKEY_LOCAL_MACHINE, regStr);
	return S_OK;
}

HRESULT FileTypeRegistrar::AddAgent(const wchar_t *agentFilename)
{
	HKEY key;
	STARTUPINFOW si = {sizeof(si), };
	PROCESS_INFORMATION pi = {0, };

	if (RegCreateKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
	{
		SetValue(key, L"WinampAgent", agentFilename);
		RegCloseKey(key);
	}
	CreateProcessW(NULL, (LPWSTR)agentFilename, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	return S_OK;
}

HRESULT FileTypeRegistrar::RemoveAgent()
{
	HKEY key;
	HWND hwnd;

	// caller will have done this also, but we'll do it again at elevated level just in case that's where winamp agent is running
	hwnd = FindWindow(L"WinampAgentMain", NULL);
	if (hwnd)
	{
		SendMessage(hwnd, WM_CLOSE, 0, 0);
	}

	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
	{
		RegDeleteValue(key, L"WinampAgent");
		RegCloseKey(key);
	}
	return S_OK;
}

static LONG RegMedia_CreateKey(LPCWSTR lpSubKey, HKEY *phkResult, LPCWSTR prog_name)
{
	HRESULT hr;
	WCHAR szKey[256];

	hr = (lpSubKey && *lpSubKey) ?
		StringCbPrintfW(szKey, sizeof(szKey), L"Software\\Clients\\Media\\%s\\%s", prog_name, lpSubKey) :
	StringCbPrintfW(szKey, sizeof(szKey), L"Software\\Clients\\Media\\%s", prog_name);
	if (S_OK != hr) return ERROR_NOT_ENOUGH_MEMORY;
	return RegCreateKeyW(HKEY_LOCAL_MACHINE, szKey, phkResult);
}

HRESULT FileTypeRegistrar::RegisterMediaPlayer(DWORD accessEnabled, const wchar_t *programName, const wchar_t *prog_name, int iconNumber)
{
	HKEY mp3Key;
	wchar_t str[MAX_PATH+128];
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));

	if (RegMedia_CreateKey(NULL, &mp3Key, prog_name) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, prog_name);
		RegCloseKey(mp3Key);
	}

	if (RegMedia_CreateKey(L"DefaultIcon",&mp3Key, prog_name) == ERROR_SUCCESS)
	{
		StringCchPrintfW(str,MAX_PATH+128,L"%s,%d",programName,iconNumber);
		SetValue(mp3Key, str);
		RegCloseKey(mp3Key);
	}
	if (RegMedia_CreateKey(L"InstallInfo",&mp3Key, prog_name) == ERROR_SUCCESS)
	{
		RegSetValueExW(mp3Key, L"IconsVisible", 0, REG_DWORD, (BYTE *)&accessEnabled, sizeof(accessEnabled));
		StringCchPrintfW(str,MAX_PATH+128, L"\"%s\" /REG=AVCDL",programName,iconNumber);
		SetValue(mp3Key,L"ReinstallCommand", str);
		SetValue(mp3Key,L"ShowIconsCommand", str);
		StringCchPrintfW(str,MAX_PATH+128,L"\"%s\" /UNREG",programName,iconNumber);
		SetValue(mp3Key,L"HideIconsCommand", str);
		RegCloseKey(mp3Key);
	}
	if (RegMedia_CreateKey(L"shell",&mp3Key, prog_name) == ERROR_SUCCESS)
		RegCloseKey(mp3Key);

	if (RegMedia_CreateKey(L"shell\\open",&mp3Key, prog_name) == ERROR_SUCCESS)
		RegCloseKey(mp3Key);

	if (RegMedia_CreateKey(L"shell\\open\\command",&mp3Key, prog_name) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, programName);
		RegCloseKey(mp3Key);
	}



	if (version.dwMajorVersion >= 6) // Vista
	{

		if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities",&mp3Key, prog_name))
		{
			SetValue(mp3Key,L"ApplicationDescription", L"Winamp. The Ultimate Media Player.");
			SetValue(mp3Key,L"ApplicationName", prog_name);
			RegCloseKey(mp3Key);
		}
		if (ERROR_SUCCESS == RegCreateKeyW(HKEY_LOCAL_MACHINE, L"Software\\RegisteredApplications", &mp3Key))
		{
			TCHAR szValue[256];
			StringCbPrintfW(szValue, sizeof(szValue), L"Software\\Clients\\Media\\%s\\Capabilities", prog_name);
			SetValue(mp3Key, prog_name, szValue);
			RegCloseKey(mp3Key);
		}

		if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities\\FileAssociations",&mp3Key, prog_name))
		{
			RegCloseKey(mp3Key);
		}

		if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities\\MimeAssociations",&mp3Key, prog_name))
		{
			RegCloseKey(mp3Key);
		}
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterMediaPlayerProtocol(LPCWSTR protocol, LPCWSTR prog_name)
{
	HKEY mp3Key;
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion >= 6) // Vista
	{
		if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities\\URLAssociations",&mp3Key, prog_name))
		{
			wchar_t szBuffer[64];

			StringCbCopyW(szBuffer, sizeof(szBuffer), protocol);
			CharUpperW(szBuffer);
			SetValue(mp3Key, protocol, szBuffer);

			RegCloseKey(mp3Key);
		}
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::UnregisterMediaPlayerProtocol(LPCWSTR protocol, LPCWSTR prog_name)
{
	HKEY mp3Key;
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion >= 6) // Vista
	{
		if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities\\URLAssociations",&mp3Key, prog_name))
		{
			RegDeleteValue(mp3Key, protocol);

			RegCloseKey(mp3Key);
		}
	}
	return S_OK;
}

static LONG RegCreateKey4(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpSubKey2, PHKEY phkResult)
{
	wchar_t temp[1024];
	StringCchPrintfW(temp, 1024, L"%s%s", lpSubKey, lpSubKey2);
	return RegCreateKeyW(hKey,temp,phkResult);
}

static LONG RegCreateKey5(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpSubKey2, LPCWSTR lpSubKey3, PHKEY phkResult)
{
	wchar_t temp[1024];
	StringCchPrintfW(temp, 1024, L"%s%s%s", lpSubKey, lpSubKey2, lpSubKey3);
	return RegCreateKeyW(hKey,temp,phkResult);
}

static LONG RegCreateKey6(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpSubKey2, LPCWSTR lpSubKey3, LPCWSTR lpSubKey4, PHKEY phkResult)
{
	wchar_t temp[1024];
	StringCchPrintfW(temp, 1024, L"%s%s%s%s", lpSubKey, lpSubKey2, lpSubKey3,lpSubKey4);
	return RegCreateKeyW(hKey,temp,phkResult);
}

HRESULT FileTypeRegistrar::SetupShell(const wchar_t *commandLine, const wchar_t *winamp_file, const wchar_t *description, const wchar_t *commandName, const wchar_t *dragAndDropGUID)
{
	HKEY mp3Key;
	if (RegCreateKey5(HKEY_CLASSES_ROOT,winamp_file, L"\\shell\\", commandName,&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, description);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey6(HKEY_CLASSES_ROOT,winamp_file, L"\\shell\\", commandName, L"\\command",&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, commandLine);
		RegCloseKey(mp3Key);
	}

	/* Drag and Drop stuff */
	if (dragAndDropGUID && *dragAndDropGUID)
	{
		if (RegCreateKey6(HKEY_CLASSES_ROOT,winamp_file, L"\\shell\\", commandName, L"\\DropTarget",&mp3Key) == ERROR_SUCCESS)
		{
			SetValue(mp3Key, L"Clsid", dragAndDropGUID);
			RegCloseKey(mp3Key);
		}
	}

	return S_OK;
}

HRESULT FileTypeRegistrar::RemoveShell(const wchar_t *winamp_file, const wchar_t *commandName)
{
	wchar_t temp[1024];
	StringCchPrintfW(temp, 1024, L"%s\\shell\\%s", winamp_file, commandName);
	myRegDeleteKeyEx(HKEY_CLASSES_ROOT, temp);
	return S_OK;
}

HRESULT FileTypeRegistrar::SetupFileType(const wchar_t *programName, const wchar_t *winamp_file, const wchar_t *name, int iconNumber, const wchar_t *defaultShellCommand, const wchar_t *iconPath)
{
	HKEY mp3Key;
	wchar_t str[MAX_PATH+32];

	if (RegCreateKeyW(HKEY_CLASSES_ROOT,winamp_file,&mp3Key) == ERROR_SUCCESS)
	{
		StringCchCopyW(str,MAX_PATH+32,name);
		SetValue(mp3Key, str);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_CLASSES_ROOT,winamp_file, L"\\DefaultIcon",&mp3Key) == ERROR_SUCCESS)
	{
		StringCchPrintfW(str,MAX_PATH+32,(iconPath[0]?L"%s":L"%s,%d"),(iconPath[0]?iconPath:programName),iconNumber);
		SetValue(mp3Key, str);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_CLASSES_ROOT,winamp_file, L"\\shell",&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, defaultShellCommand);
		RegCloseKey(mp3Key);
	}

	return S_OK;
}

HRESULT FileTypeRegistrar::SetupDefaultFileType(const wchar_t *winamp_file, const wchar_t *defaultShellCommand){

	HKEY mp3Key;
	if (RegCreateKey4(HKEY_CLASSES_ROOT,winamp_file, L"\\shell",&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, defaultShellCommand);
		RegCloseKey(mp3Key);
	}

	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterTypeShell(const wchar_t *programName, const wchar_t *which_file, const wchar_t *description, int iconNumber, const wchar_t *commandName)
{
	HKEY mp3Key;
	if (RegCreateKeyW(HKEY_CLASSES_ROOT,which_file,&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, description);
		char str[4];
		str[0]=0;
		str[1]=0;
		str[2]=1;
		str[3]=0;
		RegSetValueExW(mp3Key, L"EditFlags",0,REG_BINARY,(BYTE *)str,4);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_CLASSES_ROOT, which_file, L"\\DefaultIcon",&mp3Key) == ERROR_SUCCESS)
	{
		wchar_t str[MAX_PATH+32];
		StringCchPrintfW(str,MAX_PATH+32,L"%s,%d",programName,iconNumber);
		SetValue(mp3Key, str);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_CLASSES_ROOT,which_file, L"\\shell",&mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, commandName);
		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterGUID(const wchar_t *programName, const wchar_t *guidString)
{
	HKEY mp3Key;
	if (RegCreateKey4(HKEY_CLASSES_ROOT,L"CLSID\\", guidString, &mp3Key) == ERROR_SUCCESS)
	{
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey5(HKEY_CLASSES_ROOT,L"CLSID\\", guidString, L"\\LocalServer32",&mp3Key) == ERROR_SUCCESS)
	{
		wchar_t str[MAX_PATH+32];
		StringCchPrintfW(str,MAX_PATH+32, L"\"%s\"",programName);
		SetValue(mp3Key, str);
		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterDVDPlayer(const wchar_t *programName, int iconNumber, 
																						 const wchar_t *which_file /*Winamp.DVD*/, const wchar_t *commandName /* Winamp */,
																						 const wchar_t *provider /* Nullsoft Winamp */, const wchar_t *description /* Play in Winamp */)
{
	// TODO: stop hardcoding stuff as soon as we start using this
	wchar_t winampPath[MAX_PATH+128]; 
	wchar_t winampIcon[MAX_PATH];
	HKEY mp3Key;

	// create icon path and exe-with-param path
	StringCbPrintfW(winampIcon,sizeof(winampIcon), L"\"%s\", %d", programName, iconNumber); 
	StringCbPrintfW(winampPath,sizeof(winampPath), L"\"%s\" %1", programName);

	// uncomment if we ever want to overwrite the default icon... not set because we don't have a good dvd icon in winamp.exe
	/*
	if (RegOpenKey(HKEY_CLASSES_ROOT,"DVD\\DefaultIcon",&mp3Key) == ERROR_SUCCESS)
	{
	RegSetValueExW(mp3Key,NULL,0,REG_SZ,winampIcon,iconSize);
	}
	*/
	if (RegCreateKeyW(HKEY_CLASSES_ROOT,L"DVD\\shell",&mp3Key) == ERROR_SUCCESS)
	{
		// set winamp to be the default player
		SetValue(mp3Key, commandName);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_CLASSES_ROOT,L"DVD\\shell\\", commandName, &mp3Key) == ERROR_SUCCESS)
	{
		// text to appear in right-click shell menu in explorer
		SetValue(mp3Key, description);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey5(HKEY_CLASSES_ROOT,L"DVD\\shell\\", commandName, L"\\command",&mp3Key) == ERROR_SUCCESS)
	{
		// set the executable path
		SetValue(mp3Key, winampPath);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKeyW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\PlayDVDMovieOnArrival",&mp3Key) == ERROR_SUCCESS)
	{
		// register winamp for dvd autoplay
		RegSetValueExW(mp3Key,which_file,0,REG_SZ,0,0);
		RegCloseKey(mp3Key);
	}

	if (RegCreateKey4(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers\\", which_file, &mp3Key) == ERROR_SUCCESS)
	{
		// autoplay details
		SetValue(mp3Key,L"Action", description);
		SetValue(mp3Key,L"DefaultIcon", winampIcon);
		SetValue(mp3Key,L"InvokeProgID", L"DVD");
		SetValue(mp3Key,L"InvokeVerb", commandName);
		SetValue(mp3Key,L"Provider", provider);
		RegCloseKey(mp3Key);
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::InstallItem(LPCWSTR sourceFile, LPCWSTR destinationFolder, LPCWSTR destinationFilename)
{
	SHFILEOPSTRUCT op = {0};

	op.hwnd = 0;
	op.wFunc = FO_COPY;

	wchar_t srcFile[MAX_PATH+1];
	wchar_t *end;
	StringCchCopyExW(srcFile, MAX_PATH, sourceFile, &end, 0, 0);
	if (end)
		end[1]=0; // double null terminate
	op.pFrom = srcFile;

	wchar_t destFile[MAX_PATH+1];
	PathCombineW(destFile, destinationFolder, destinationFilename);
	destFile[wcslen(destFile)+1]=0; // double null terminate
	op.pTo = destFile;

	op.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS|FOF_NOERRORUI|FOF_SILENT;
	op.lpszProgressTitle = L"";
	int val = SHFileOperation(&op);

	// send back the error message so we can show on the UI
	if (val != 0 && val != 0x71/*DE_SAMEFILE*/)
	{
		StringCchPrintfW((LPWSTR)destinationFilename,MAX_PATH,L"0x%x",val);
	}

	return ((val == 0 || val == 0x71/*DE_SAMEFILE*/) ? S_OK : E_FAIL);
}

HRESULT FileTypeRegistrar::DeleteItem(LPCWSTR file)
{
	SHFILEOPSTRUCT op;

	op.hwnd = 0;
	op.wFunc = FO_DELETE;

	wchar_t srcFile[MAX_PATH+1];
	wchar_t *end;
	StringCchCopyExW(srcFile, MAX_PATH, file, &end, 0, 0);
	if (end)
		end[1]=0; // double null terminate
	op.pFrom = srcFile;

	op.pTo = 0;

	op.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS|FOF_NORECURSION|FOF_NOERRORUI|FOF_SILENT|FOF_ALLOWUNDO;
	op.lpszProgressTitle = L"";
	int val = SHFileOperation(&op);

	return val == 0?S_OK:E_FAIL;
}

HRESULT FileTypeRegistrar::RenameItem(LPCWSTR oldFile, LPCWSTR newFile, BOOL force)
{
	SHFILEOPSTRUCT op;

	op.hwnd = 0;
	op.wFunc = force?FO_MOVE:FO_RENAME;

	wchar_t srcFile[MAX_PATH+1];
	wchar_t *end;
	StringCchCopyExW(srcFile, MAX_PATH, oldFile, &end, 0, 0);
	if (end)
		end[1]=0; // double null terminate
	op.pFrom = srcFile;

	wchar_t destFile[MAX_PATH+1];
	end=0;
	StringCchCopyExW(destFile, MAX_PATH, newFile, &end, 0, 0);
	if (end)
		end[1]=0; // double null terminate
	op.pTo = destFile;

	op.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS|FOF_NOERRORUI|FOF_SILENT;
	op.lpszProgressTitle = L"";
	int val = SHFileOperation(&op);

	return val == 0?S_OK:E_FAIL;

	/*
	if (!MoveFile(oldFile, newFile))
	{
	// if move failed

	if (!force) // if they don't want to force the move
	return E_FAIL;

	if (CopyFile(oldFile, newFile, FALSE) && DeleteFile(oldFile)) // copy then delete
	return E_FAIL;
	}
	return S_OK;
	*/
}

HRESULT FileTypeRegistrar::CleanupDirectory(LPCWSTR directory)
{
	wchar_t dirmask[MAX_PATH];
	HANDLE h;
	WIN32_FIND_DATAW d;
	PathCombineW(dirmask, directory, L"*.*");
	h = FindFirstFileW(dirmask, &d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t v[MAX_PATH];
			PathCombineW(v, directory, d.cFileName);
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(d.cFileName,L".") && wcscmp(d.cFileName,L".."))
					CleanupDirectory(v);
			}
			else
			{
				if(!DeleteFileW(v))
				{
					// this handles some rogue cases where files in the wlz's aren't unloadable
					MoveFileExW(v, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					MoveFileExW(directory, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
		while (FindNextFileW(h, &d));
		FindClose(h);
	}
	RemoveDirectoryW(directory);
	return S_OK;
}

HRESULT FileTypeRegistrar::MoveDirectoryContents(LPCWSTR oldDirectory, LPCWSTR newDirectory)
{
	wchar_t fromDirectory[MAX_PATH+1] = {0}; // need to zero it all so it will be double-null terminated
	SHFILEOPSTRUCT sf;

	ZeroMemory(&sf,sizeof(SHFILEOPSTRUCT));
	StringCchPrintfW(fromDirectory,MAX_PATH,L"%s\\*",oldDirectory);

	sf.wFunc = FO_MOVE;
	sf.pFrom = fromDirectory;
	sf.pTo = newDirectory;
	sf.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_NORECURSION;

	if(SHFileOperation(&sf)){
		return E_FAIL;
	}
	return S_OK;
}

HRESULT FileTypeRegistrar::WriteProKey(LPCWSTR name, LPCWSTR key)
{
	HKEY hkey;

	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp", 0, 0, 0, KEY_READ | KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		wchar_t buf[33];
		int x;
		if (*name)
			SetValue(hkey, L"regname", name);
		else
			RegDeleteValueW(hkey, L"regname");

		if (*key)
		{
		StringCchCopyW(buf + 1, 32, key);
		buf[0] = '~';
		for (x = 1;buf[x]; x ++)
		{
			wchar_t c = towupper(buf[x]); 
			//char c = buf[x]; // benski> goes with above commented line that i'd like to uncomment 
			if (c >= 'A' && c <= 'Z')
			{
				int w = c - 'A';
				w += x;
				w %= 26;
				buf[x] = 'A' + w;
			}
			else if (c >= '0' && c <= '9')
			{
				int w = c - '0';
				w += x * 27;
				w %= 10;
				buf[x] = 'a' + w;
			}
			else if (c == '-') buf[x] = '/';
		}
		SetValue(hkey, L"regkey", buf);
		}
		else
		{
			RegDeleteValueW(hkey, L"regkey");
		}
		RegCloseKey(hkey);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT FileTypeRegistrar::WriteClientUIDKey(LPCWSTR path, LPCWSTR uid_str)
{
	HKEY hkey;

	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp", 0, 0, 0, KEY_READ | KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		SetValue(hkey, path, uid_str);
		RegCloseKey(hkey);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT FileTypeRegistrar::RegisterProtocol(LPCWSTR protocol, LPCWSTR command, LPCWSTR icon)
{
	HKEY mp3Key;
	wchar_t regStr[256];
	if (RegCreateKeyW(HKEY_CLASSES_ROOT, protocol, &mp3Key) == ERROR_SUCCESS)
	{
		SetValue(mp3Key, L"URL:Winamp Command Handler");
		SetValue(mp3Key, L"URL Protocol", L"");
		RegCloseKey(mp3Key);

		StringCbPrintfW(regStr, sizeof(regStr), L"%s\\DefaultIcon", protocol);
		if (RegCreateKeyW(HKEY_CLASSES_ROOT,regStr,&mp3Key) == ERROR_SUCCESS)
		{
			SetValue(mp3Key, icon);
			RegCloseKey(mp3Key);
		}

		StringCbPrintfW(regStr, sizeof(regStr), L"%s\\shell\\open\\command", protocol);
		if (RegCreateKeyW(HKEY_CLASSES_ROOT,regStr,&mp3Key) == ERROR_SUCCESS)
		{
			SetValue(mp3Key, command);
			RegCloseKey(mp3Key);
		}
	}

	return S_OK;
}

HRESULT FileTypeRegistrar::RegisterCapability(const wchar_t *programName, const wchar_t *winamp_file, const wchar_t *extension)
{
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion >= 6) // Vista
	{

		HKEY hkey_file_associations;
			if (ERROR_SUCCESS == RegMedia_CreateKey(L"Capabilities\\FileAssociations",&hkey_file_associations, programName))
			{
					RegDeleteValueW(hkey_file_associations, extension); // to make sure that they all in case that we need
					SetValue(hkey_file_associations, extension, winamp_file);
					RegCloseKey(hkey_file_associations);
				}
	}
	return S_OK;
}
