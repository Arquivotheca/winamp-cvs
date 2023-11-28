/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description: Utility functions for handling language support
** Author:
** Created:
**/

#include <locale.h>
#include "main.h"
#include "language.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../zlib/unzip.h"
#include "PtrList.h"
#include "../nu/AutoCharFn.h"

typedef struct {
	char *module;
	int guid;
	char *guidstr;	// generally is 0 or 39 (38+null)
	HINSTANCE hDllInstance;
} winampLangStruct;

static nu::PtrList<winampLangStruct> lnglist;
int already_extracted = 0, prev_wlz_ex_state = 0, geno = 1;

// data storage of the read in values requires just the hash and the id to be stored
// and has to be logged against the different id types so just need a list of structs
// based against each type (dialog, string resource, etc)

// for the moment we deal with the following resource types
// RT_DIALOG, RT_MENU, RT_STRING & custom resources
// so for initial implementation we only need to store upto 4 hash+id lists

#if 0
typedef struct {
	int id;
	char id_str[32];
	char hash[17];
	char str[64];
} hashstruct;

nu::PtrList<hashstruct> dialogList;
nu::PtrList<hashstruct> menuList;
nu::PtrList<hashstruct> stringList;	// will be the largest of the lot
nu::PtrList<hashstruct> customList; // should be very few of this
#endif

// have section header (text or int id)
// then the hash and then the id that's related the hash eg the dialog resource id

void ReadHashFileDetails(char* data, DWORD datalen)
{
#if 0
	char* p = data, *s = p, *t = 0, *u;
	while(s && *s)
	{
		// is it the start of a block that we've just gotten to...
		if(*s == '@' || *s == '#')
		{
			int id = -1;
			char id_str[32] = {0};
			u = s = CharNext(s);
			if(!*s){break;}

			// advance to the end of the line to get the block identifier
			// would need to use the @ or # to process the type used
			// ie if a type 5 then only use on dialog loading calls
			while(u && *u && *u != '\n'){u = CharNext(u);}
			if(*u == '\n'){u = CharNext(u);*CharPrev(p,u) = 0;}
			if(!*u){break;}

			// identifier of the block is found here :)
			if(*s)
			{
				id = atoi(s);
				if(!id)
				{
					lstrcpyn(id_str, s, sizeof(id_str));
				}
			}
			*CharPrev(p,u) = '\n';

			while(s && *s && (*s != '@' && *s != '#'))
			{
				int end = 0;

				while(s && *s && *s != '\n'){s = CharNext(s);}
				if(*s == '\n'){s = CharNext(s);}
				// if nothing else then need to abort (since we don't want to do bad things)
				// and have to take into account where in the buffer we are otherwise we can
				// end up going into the next part of the dll/exe resource data due to how
				// it is all stored/handled in them (ie butted up against each other)
				if(!*s || s >= p+datalen){break;}

				t = s;
				// do a check after we've advanced to the start of a new line
				// so that we can see if we've hit a new resource type block
				if(*s == '@' || *s == '#')
				{
					s = CharPrev(p,s);
					break;
				}

				// scan through to the start of the second part of the <hash:id> block
				while(t && *t && *t != ':')
				{
					t = CharNext(t);
				}

				if(*t == ':')
				{
					t = CharNext(t);
					*CharPrev(p,t) = 0;
				}

				// scan through to the end of the line so that we then have the id
				u = t;
				while(u && *u && *u != '\n')
				{
					u = CharNext(u);
				}

				if(*u == '\n')
				{
					u = CharNext(u);
					*CharPrev(p,u) = 0;
				}

				// hash and identifier of the entry is found here :)
				// -> need to check how it works with IDD_CRASHDLG$()
				if(*s)
				{
					hashstruct* tempList = reinterpret_cast<hashstruct*>(malloc(sizeof(hashstruct)));
					ZeroMemory(tempList,sizeof(hashstruct));

					/*if(*t == 1) wsprintf(a,"%s %d (%s)\n", s, *t, t+1);
					else wsprintf(a,"%s %s (%d)\n", s, t+1, *t);*/
					if(*t == 1)	// int_id
						lstrcpyn(tempList->str, t+1, sizeof(tempList->str));
					else		// string_id
						lstrcpyn(tempList->str, t+1, *t/*sizeof(tempList->str)*/);

					lstrcpyn(tempList->hash, s, sizeof(tempList->hash));
					if(id) tempList->id = id;

					switch(id)
					{
						case RT_MENU:
						{
							menuList.push_back(tempList);
						}
							break;
						case RT_DIALOG:
						{
							dialogList.push_back(tempList);
						}
							break;
						case RT_STRING:
						{
							stringList.push_back(tempList);
						}
							break;
						default:
							// only do if there's no id from atoi (indicates a custom resource id)
							if(!id)
							{
								lstrcpyn(tempList->id_str, id_str, sizeof(tempList->id_str));
								customList.push_back(tempList);
							}
							break;
					}

					{
						char zz[100];
						StringCchPrintf(zz,100,"ID: '%s' %d\t%s %s\n",
										tempList->id_str, tempList->id,
										tempList->hash, tempList->str);
						OutputDebugString(zz);
					}
				}
				*CharPrev(p,u) = '\n';
				s = CharPrev(p,u);
			}
		}
		s = CharNext(s);
	}
#endif
}

int GetImageHashData(HINSTANCE imageInstance)
{
	DWORD datalen = 0;
	void* data = langManager->LoadResourceFromFile(imageInstance,imageInstance,"HASH","HASH",&datalen);
	ReadHashFileDetails((char*)data,datalen);
	UnlockResource(data);
	FreeResource(data);
	return 0;
}

#ifdef _DEBUG
static void CheckLangThread()
{
	if (mainThreadId != GetCurrentThreadId())
	{
//		DebugBreak();
		/**
		** If you hit this breakpoint, it's because you tried to use WASABI_API_LANG->GetString on another thread,
		** without supplying your own buffer.
		** You really don't want to be doing that.
		** Hit alt+7, check what function is calling GetString/GetStringW and go fix it.
		** Now.
		**/
	}
}

#else
#define CheckLangThread()
#endif

char *getString(UINT uID, char *str, size_t maxlen)
{
	return langManager->GetString(language_pack_instance,hMainInstance, uID, str, maxlen);
}

int LPMessageBox(HWND parent, UINT idMessage, UINT idTitle, UINT type)
{
	wchar_t message[32768];
	wchar_t title[256];

	// TODO consider exposing something in the winamp.lng file so we can follow this where possible as it should allow for a localised messagebox as long as the OS supports the language
	// return MessageBoxExW(parent,getStringW(idMessage,message,32768),getStringW(idTitle,title,256),type,MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH));
	return MessageBoxW(parent,getStringW(idMessage,message,32768),getStringW(idTitle,title,256),type);
}

char* Language::GetString(HINSTANCE hinst, HINSTANCE owner, UINT uID, char *str, size_t maxlen)
{
	__declspec(thread) static char *buf;
	if (!str)
	{
		CheckLangThread();
		if (!buf)
			buf = (char *)malloc(LANG_STATIC_BUFFER_SIZE*sizeof(buf[0]));
		str = buf; 
		maxlen = LANG_STATIC_BUFFER_SIZE; 
	}
	if (!LoadString(hinst, uID, str, maxlen))
	{
		if (hinst == owner || !LoadString(owner, uID, str, maxlen))
		{
			lstrcpyn(str, "Error loading string", maxlen);
		}
	}
	return str;
}

wchar_t *getStringW(UINT uID, wchar_t *str, size_t maxlen)
{
	return langManager->GetStringW(language_pack_instance,hMainInstance, uID, str, maxlen);
}

wchar_t* Language::GetStringW(HINSTANCE hinst, HINSTANCE owner, UINT uID, wchar_t *str, size_t maxlen)
{
	__declspec(thread) static wchar_t *buf;
	if (!str) 
	{
		CheckLangThread();
		if (!buf) 	
			buf = (wchar_t *)malloc(LANG_STATIC_BUFFER_SIZE*sizeof(buf[0]));
		str = buf; 
		maxlen = LANG_STATIC_BUFFER_SIZE; 
	}
	if (!LoadStringW(hinst, uID, str, maxlen))
	{
		if (hinst == owner || !LoadStringW(owner, uID, str, maxlen))
		{
			lstrcpynW(str, L"Error loading string", maxlen);
		}
	}
	return str;
}

char* Language::GetStringFromGUID(const GUID guid, HINSTANCE owner, UINT uID, char *str, size_t maxlen)
{
	__declspec(thread) static char *buf;
	if (!str) 
	{
		CheckLangThread();
		if (!buf) 	
			buf = (char *)malloc(LANG_STATIC_BUFFER_SIZE*sizeof(buf[0]));
		str = buf; 
		maxlen = LANG_STATIC_BUFFER_SIZE; 
	}
	HINSTANCE tl = FindDllHandleByGUID(guid);
	if(!tl) tl = owner;
	if (!LoadString(tl, uID, str, maxlen))
	{
		if (!LoadString(owner, uID, str, maxlen))
		{
			lstrcpyn(str, "Error loading string", maxlen);
		}
	}
	return str;
}

wchar_t* Language::GetStringFromGUIDW(const GUID guid, HINSTANCE owner, UINT uID, wchar_t *str, size_t maxlen)
{
	__declspec(thread) static wchar_t *buf;
	if (!str) 
	{
		CheckLangThread();
		if (!buf) 	
			buf = (wchar_t *)malloc(LANG_STATIC_BUFFER_SIZE*sizeof(buf[0]));
		str = buf; 
		maxlen = LANG_STATIC_BUFFER_SIZE; 
	}
	HINSTANCE tl = FindDllHandleByGUID(guid);
	if(!tl) tl = owner;
	if (!LoadStringW(tl, uID, str, maxlen))
	{
		if (!LoadStringW(owner, uID, str, maxlen))
		{
			lstrcpynW(str, L"Error loading string", maxlen);
		}
	}
	return str;
}

const wchar_t *Language::GetLanguageFolder()
{
	return LANGTEMPDIR;
}

void* Language::LoadResourceFromFileW(HINSTANCE hinst, HINSTANCE owner, LPCWSTR lpType, LPCWSTR lpName, DWORD* size)
{
	HINSTANCE hmod = hinst;
	HRSRC rsrc = FindResourceW(hmod, lpName, lpType);
	if(!rsrc)
	{
		hmod = owner;
		rsrc = FindResourceW(hmod, lpName, lpType);
	}
	if(rsrc)
	{
		HGLOBAL resourceHandle = LoadResource(hmod, rsrc);
		if(size){*size = SizeofResource(hmod, rsrc);}
		return LockResource(resourceHandle);
	}
	return 0;
}

void* Language::LoadResourceFromFile(HINSTANCE hinst, HINSTANCE owner, LPCTSTR lpType, LPCTSTR lpName, DWORD* size)
{
	HINSTANCE hmod = hinst;
	HRSRC rsrc = FindResource(hmod, lpName, lpType);
	if(!rsrc)
	{
		hmod = owner;
		rsrc = FindResource(hmod, lpName, lpType);
	}
	if(rsrc)
	{
		HGLOBAL resourceHandle = LoadResource(hmod, rsrc);
		if(size){*size = SizeofResource(hmod, rsrc);}
		return LockResource(resourceHandle);
	}
	return 0;
}

const wchar_t *Language::GetLanguageIdentifier(int mode)
{
	static wchar_t id_str[9] = {0};
	id_str[0] = 0;
	// 5.58 fix - was returning en-US on all calls to this via load_extra_lng(..)
	// make sure to try to use a loaded winamp.lng as load_extra_lng(..) relies on
	// this for the path to use but calls it before getStringW(..) will work fully
	GetStringFromGUIDW(WinampLangGUID, hMainInstance, LANG_PACK_LANG_ID, id_str, 9);

	if(!_wcsicmp(id_str, L"Error l"))
	{
		id_str[0] = 0;
	}

	if(mode && id_str[0])
	{
		wchar_t* iStr = id_str;
		while(iStr && *iStr && *iStr != L'-'){ iStr = CharNextW(iStr); }
		if(*iStr == '-'){ iStr = CharNextW(iStr);*CharPrevW(id_str, iStr) = 0; }

		if(mode == LANG_LANG_CODE)
		{
			return id_str;
		}
		else if(mode == LANG_COUNTRY_CODE)
		{
			return iStr;
		}
	}
	return (id_str[0]?id_str:0);
}

HWND Language::CreateLDialogParam(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param)
{
	HWND hwnd = (HWND)CreateDialogParam(localised, MAKEINTRESOURCE(id), parent, proc, param);
	if (!hwnd && localised != original)
		hwnd = (HWND)CreateDialogParam(original, MAKEINTRESOURCE(id), parent, proc, param);
	return hwnd;
}

HWND Language::CreateLDialogParamW(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param)
{
	HWND hwnd = (HWND)CreateDialogParamW(localised, MAKEINTRESOURCEW(id), parent, proc, param);
	if (!hwnd && localised != original)
		hwnd = (HWND)CreateDialogParamW(original, MAKEINTRESOURCEW(id), parent, proc, param);
	return hwnd;
}

INT_PTR Language::LDialogBoxParam(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param)
{
	INT_PTR ret = DialogBoxParam(localised, MAKEINTRESOURCE(id), parent, proc, param);
	if ((ret == -1 && GetLastError() != ERROR_SUCCESS) && localised != original)
		ret = DialogBoxParam(original, MAKEINTRESOURCE(id), parent, proc, param);
	return ret;
}

INT_PTR Language::LDialogBoxParamW(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param)
{
	INT_PTR ret = DialogBoxParamW(localised, MAKEINTRESOURCEW(id), parent, proc, param);
	if ((ret == -1 && GetLastError() != ERROR_SUCCESS) && localised != original)
		ret = DialogBoxParamW(original, MAKEINTRESOURCEW(id), parent, proc, param);
	return ret;
}

HWND LPCreateDialogParam(int id, HWND parent, DLGPROC proc, LPARAM param)
{
	return langManager->CreateLDialogParam(language_pack_instance, hMainInstance, id, parent, proc, param);
}

HWND LPCreateDialogParamW(int id, HWND parent, DLGPROC proc, LPARAM param)
{
	return langManager->CreateLDialogParamW(language_pack_instance, hMainInstance, id, parent, proc, param);
}

INT_PTR LPDialogBoxParam(int id, HWND parent, DLGPROC proc, LPARAM param)
{
	return langManager->LDialogBoxParam(language_pack_instance, hMainInstance, id, parent, proc, param);
}

INT_PTR LPDialogBoxParamW(int id, HWND parent, DLGPROC proc, LPARAM param)
{
	return langManager->LDialogBoxParamW(language_pack_instance, hMainInstance, id, parent, proc, param);
}

HMENU Language::LoadLMenu(HINSTANCE localised, HINSTANCE original, UINT id)
{
	HMENU menu = LoadMenu(localised, MAKEINTRESOURCE(id));
	if (!menu && localised != original)
		menu = LoadMenu(original, MAKEINTRESOURCE(id));
	return menu;
}

HMENU Language::LoadLMenuW(HINSTANCE localised, HINSTANCE original, UINT id)
{
	HMENU menu = LoadMenuW(localised, MAKEINTRESOURCEW(id));
	if (!menu && localised != original)
		menu = LoadMenuW(original, MAKEINTRESOURCEW(id));
	return menu;
}

HACCEL Language::LoadAcceleratorsA(HINSTANCE hinst, HINSTANCE owner, LPCSTR lpTableName)
{
	HACCEL hAccel = ::LoadAcceleratorsA(hinst, lpTableName);
	if (!hAccel && hinst != owner)
		hAccel = ::LoadAcceleratorsA(hinst, lpTableName);
	return hAccel;
}

HACCEL Language::LoadAcceleratorsW(HINSTANCE hinst, HINSTANCE owner, LPCWSTR lpTableName)
{
	HACCEL hAccel = ::LoadAcceleratorsW(hinst, lpTableName);
	if (!hAccel && hinst != owner)
		hAccel = ::LoadAcceleratorsW(hinst, lpTableName);
	return hAccel;
}

// Implemented in 5.58+
// when we're loading a language pack we really need to specify if we're
// going to require correct use of the user's locale setting so that the
// output of certain text ie '%+6.1f' uses the correct decimal separator
// ref: http://msdn.microsoft.com/en-us/library/aa246453%28VS.60%29.aspx
BOOL Language::UseUserNumericLocale(void)
{
	wchar_t tmp[4], lang[4], ctry[4];
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, lang, 4);
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, ctry, 4);

	// do this check to ensure that the we only change the locale
	// if the language pack and the user locale identifiers match
	if(!_wcsicmp(lang, GetLanguageIdentifier(LANG_LANG_CODE)) &&
	   !_wcsicmp(ctry, GetLanguageIdentifier(LANG_COUNTRY_CODE)) &&
	   GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, tmp, 4))
	{
		_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
		// now we set the functions to use the user's numeric locale
		return !!_wsetlocale(LC_NUMERIC,tmp);
	}
	return FALSE;
}

_locale_t Language::Get_C_NumericLocale(void)
{
	__declspec(thread) static _locale_t C_locale;
	if(!C_locale) C_locale = _create_locale(LC_NUMERIC, "C");
	return C_locale;
}

// Implemented in 5.7+
wchar_t* Language::FormattedSizeString(wchar_t *pszDest, int cchDest, __int64 size)
{
	if (!pszDest) return 0;
	size_t remaining = cchDest;
	DWORD part = 0;
	pszDest[0] = 0x00;

	if (size < 1024)
	{
		StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS,
						   L"%u %s", (DWORD)(size >> 10) + ((((DWORD)(size))&1023) ? 1: 0),
						   getStringW(IDS_BYTES, NULL, 0));
	}
	else if (size < 1048576)
	{
		part = ((((DWORD)(size))&1023)*100) >> 10;
		if (part > 0)
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u.%02u %s",
							   (DWORD)(size >> 10), part, getStringW(geno ? IDS_KB : IDS_KIB, NULL, 0));
		}
		else
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u %s",
							   (DWORD)(size >> 10), getStringW(geno ? IDS_KB : IDS_KIB, NULL, 0));
		}
	}
	else if (size < 1073741824)
	{
		part = ((((DWORD)(size >> 10))&1023)*100) >> 10;
		if (part > 0)
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u.%02u %s",
							   (DWORD)(size >> 20), part, getStringW(geno ? IDS_MB : IDS_MIB, NULL, 0));
		}
		else
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u %s",
							   (DWORD)(size >> 20), getStringW(geno ? IDS_MB : IDS_MIB, NULL, 0));
		}
	}
	else if (size < 1099511627776)
	{
		part = ((((DWORD)(size >> 20))&1023)*100) >> 10;
		if (part > 0)
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u.%02u %s",
							   (DWORD)(size >> 30), part, getStringW(geno ? IDS_GB : IDS_GIB, NULL, 0));
		}
		else
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u %s",
							   (DWORD)(size >> 30), getStringW(geno ? IDS_GB : IDS_GIB, NULL, 0));
		}
	}
	else
	{
		part = ((((DWORD)(size >> 30))&1023)*100) >> 10;
		if (part > 0)
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u.%02u %s",
							   (DWORD)(size >> 40), part, getStringW(geno ? IDS_TB : IDS_TIB, NULL, 0));
		}
		else
		{
			StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS, L"%u %s",
							   (DWORD)(size >> 40), getStringW(geno ? IDS_TB : IDS_TIB, NULL, 0));
		}
	}


	return pszDest;
}

HMENU LPLoadMenu(UINT id)
{
	return langManager->LoadLMenu(language_pack_instance, hMainInstance, id);
}


void Lang_CleanupZip(void)
{
	if (!LANGTEMPDIR[0]) return ;
	_cleanupDirW(LANGTEMPDIR);
	char str[78];
	StringCchPrintf(str,78,"lang_clean_up%ws",szAppName);
	_w_s(str, 0);
}


// attempt to cleanup the last extracted temp folder for a wlz incase Winamp crashed on exit
void Lang_CleanupAfterCrash(void)
{
	wchar_t buf[1024] = {0};
	char str[78];
	StringCchPrintf(str,78,"lang_clean_up%ws",szAppName);
	_r_sW(str, buf, sizeof(buf));
	if (buf[0])
	{
		_cleanupDirW(buf);
		_w_s(str, 0);
	}
}

static void load_extra_lng()
{
	if (langManager)
	{
		const wchar_t *lang_identifier = langManager->GetLanguageIdentifier(LANG_IDENT_STR);
		if (lang_identifier)
		{
			wchar_t extra_lang_path[MAX_PATH];
			wchar_t lng_file[MAX_PATH];
			PathCombineW(extra_lang_path, LANGDIR, lang_identifier);
			PathCombineW(lng_file, extra_lang_path, L"*.lng");
			WIN32_FIND_DATAW find_data;
			HANDLE h = FindFirstFileW(lng_file, &find_data);

			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					PathCombineW(lng_file, extra_lang_path, find_data.cFileName);
					winampLangStruct* templng = reinterpret_cast<winampLangStruct*>(malloc(sizeof(winampLangStruct)));
					ZeroMemory(templng,sizeof(winampLangStruct));
					templng->hDllInstance = LoadLibraryW(lng_file);
					if(templng->hDllInstance)
					{
						char s[39] = {0};
						if(LoadString(templng->hDllInstance, LANG_DLL_GUID_STRING_ID, s, 39))
						{
							templng->guidstr = _strdup(s);
							GetImageHashData(templng->hDllInstance);
						}
						// only keep if it's a valid lng dll ie doesn't have load issues
						lnglist.push_back(templng);
					}
				}
				while (FindNextFileW(h, &find_data));
				FindClose(h);
			}
		}
	}
}


// return 1 if we're working from a wlz otherwise return 0
int extract_wlz_to_dir(wchar_t* readme_only_wlz_extraction)
{
	int is_wlz = 0;
	if (config_langpack[0] || readme_only_wlz_extraction && readme_only_wlz_extraction[0])
	{
		wchar_t* langpack = (readme_only_wlz_extraction?readme_only_wlz_extraction:config_langpack),
				 tempdirbuf[MAX_PATH] = {0}, *TEMPDIR = LANGTEMPDIR;

		if (_wcsicmp(extensionW(langpack), L"wlz"))
		{
			if (PathIsFileSpecW(langpack) || PathIsRelativeW(langpack))
				PathCombineW(lang_directory, LANGDIR, langpack);
			else 
				StringCchCopyW(lang_directory, MAX_PATH, langpack);
		}
		else
		{
			wchar_t dirmask[MAX_PATH*4] = {0};
			char str[78] = {0};
			unzFile f = {0};

			// make sure that we use a different folder from the current wlz temp folder otherwise we have issues
			if(readme_only_wlz_extraction){
				wchar_t buf[MAX_PATH] = {0};
				GetTempPathW(MAX_PATH, buf);
				GetTempFileNameW(buf, L"WLZ", GetTickCount(), tempdirbuf);
				TEMPDIR = tempdirbuf;
			}

			CreateDirectoryW(TEMPDIR, NULL);
			StringCchPrintf(str,78,"lang_clean_up%ws",szAppName);
			if(!readme_only_wlz_extraction){
				StringCchCopyW(lang_directory, MAX_PATH, TEMPDIR);
				_w_sW(str, TEMPDIR);
			}

			if (PathIsFileSpecW(langpack)|| PathIsRelativeW(langpack))
				PathCombineW(dirmask, LANGDIR, langpack);
			else
				StringCchCopyW(dirmask, MAX_PATH*4, langpack);

			// now we're going to extract, if doing a temp extraction then set the path into the passed buffer
			if(readme_only_wlz_extraction){
				StringCchCopyW(readme_only_wlz_extraction, MAX_PATH, TEMPDIR);
			}

			f = unzOpen(AutoCharFn(dirmask));
			if (f)
			{
				if (unzGoToFirstFile(f) == UNZ_OK)
				{
					OVERLAPPED asyncIO = {0};
					int isNT = (GetVersion() < 0x80000000);
					if (isNT)
					{
						asyncIO.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
						asyncIO.OffsetHigh = 0;
					}
					do
					{
						char filename[MAX_PATH] = {0}, *fn = 0, *p = 0;
						if (isNT)
							SetEvent(asyncIO.hEvent);
						unzGetCurrentFileInfo(f, NULL, filename, sizeof(filename), NULL, 0, NULL, 0);

						//Only extract the file-types that could be in a skin
						//If we don't filter here it's a security hole

						// expand out folders if we've got a freeform based folder
						if(!_strnicmp(filename,"freeform\\",9) || !_strnicmp(filename,"freeform/",9))
							fn = filename;
						// otherwise just extract to the root of the temp directory
						else
							fn = scanstr_back(filename, "\\/", filename - 1) + 1;

						p = extension(fn);
						// TODO: really should enum image loaders so we only extract supported image files
						if (!_stricmp(p, "lng") || !_stricmp(p, "ini") || !_stricmp(p, "txt") ||
							!_stricmp(p, "png") || !_stricmp(p, "bmp") || !_stricmp(p, "gif") ||
							!_stricmp(p, "jpg") || !_stricmp(p, "xml") ||
							// not too keen on dll in there but that's how the GN dlls are named
							!_stricmp(p, "dll"))
						{
							int success = 0;
							if (unzOpenCurrentFile(f) == UNZ_OK)
							{
								HANDLE fp = INVALID_HANDLE_VALUE;

								PathCombineW(dirmask, TEMPDIR, AutoWide(fn));
								CreateDirectoryForFileW(dirmask, TEMPDIR);

								fp = CreateFileW(dirmask, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | (isNT ? FILE_FLAG_OVERLAPPED : 0), NULL);
								if (fp != INVALID_HANDLE_VALUE)
								{
									DWORD written = 0;
									int l = 0, pos = 0, bufNum=0;

									#define LANG_ZIP_BUFFER_SIZE 2048
									char buf[LANG_ZIP_BUFFER_SIZE*2] = {0};
									success = 1;
									do
									{
										bufNum = !bufNum;
										l = unzReadCurrentFile(f, buf+LANG_ZIP_BUFFER_SIZE*bufNum, LANG_ZIP_BUFFER_SIZE);
										if (!l)
											unzCloseCurrentFile(f);
										if (isNT)
										{
											WaitForSingleObject(asyncIO.hEvent, INFINITE);
											if (l > 0)
											{
												asyncIO.Offset = pos;
												if (WriteFile(fp, buf+LANG_ZIP_BUFFER_SIZE*bufNum, l, NULL, &asyncIO) == FALSE
													&& GetLastError() != ERROR_IO_PENDING)
												{
													success=0;
												}
												pos += l;
											}
										}
										else
										{
											if (l > 0)
											{
												if (WriteFile(fp, buf+LANG_ZIP_BUFFER_SIZE*bufNum, l, &written, NULL) == FALSE)
													success = 0;
											}
										}
									} while (l > 0 && success);

									CloseHandle(fp);

									// cache information about the extracted lng files
									if(!_stricmp(p, "lng") && !readme_only_wlz_extraction)
									{
										is_wlz = 1;
										winampLangStruct* templng = reinterpret_cast<winampLangStruct*>(malloc(sizeof(winampLangStruct)));
										ZeroMemory(templng,sizeof(winampLangStruct));
										templng->module = _strdup(filename);

										templng->hDllInstance = LoadLibraryW(dirmask);

										if(templng->hDllInstance)
										{
											char s[39] = {0};
											if(LoadString(templng->hDllInstance, LANG_DLL_GUID_STRING_ID, s, 39))
											{
												templng->guidstr = _strdup(s);
												GetImageHashData(templng->hDllInstance);
											}
											// only keep if it's a valid lng dll ie doesn't have load issues
											lnglist.push_back(templng);
										}
									}
								}
							}
						}
					}
					while (unzGoToNextFile(f) == UNZ_OK);
					if (isNT && asyncIO.hEvent)
					{
						CloseHandle(asyncIO.hEvent);
					}
				} 
				unzClose(f);
			}
		}
	}
	else
	{
		lang_directory[0] = 0;
	}
	return is_wlz;
}


HINSTANCE Language::FindDllHandleByGUID(const GUID guid)
{
	char gs[40];
	getGUIDstr(guid,gs);
	for(size_t i = 0; i < lnglist.size(); i++)
	{
		if(lnglist[i]->guidstr && *lnglist[i]->guidstr && !_strnicmp(gs, lnglist[i]->guidstr, 38))
			return lnglist[i]->hDllInstance;
	}
	return NULL;
}


HINSTANCE Language::FindDllHandleByString(const char* str)
{
	if(str && *str)
	{
		for(size_t i = 0; i < lnglist.size(); i++)
		{
			if(lnglist[i]->module && *lnglist[i]->module && !_strnicmp(lnglist[i]->module, str, lstrlen(str)))
				return lnglist[i]->hDllInstance;
		}
	}
	return NULL;
}


HINSTANCE Language::FindDllHandleByStringW(const wchar_t* _str)
{
	AutoChar str__(_str);
	const char *str = str__;
	if(str && *str)
	{
		for(size_t i = 0; i < lnglist.size(); i++)
		{
			if(lnglist[i]->module && *lnglist[i]->module && !_strnicmp(lnglist[i]->module, str, lstrlen(str)))
				return lnglist[i]->hDllInstance;
		}
	}
	return NULL;
}


HINSTANCE Lang_InitLangSupport(HINSTANCE hinst, const GUID guid)
{
	geno = _r_i("geno", 1);
	return langManager->StartLanguageSupport(hinst, guid);
}


void Lang_FollowUserDecimalLocale(void)
{
	langManager->UseUserNumericLocale();
}


// use this to load based on the module specified so that we make sure
// we've got the correct hinstance based on lng file or default handle
HINSTANCE Language::StartLanguageSupport(HINSTANCE hinstance, const GUID guid)
{
	if (!g_safeMode)
	{
		HWND agent = FindWindow("WinampAgentMain", NULL);
		wchar_t winampaLngPath[MAX_PATH], winampaWlzPath[MAX_PATH];
		int is_wlz;

		// if we find Winamp Agent running then we need to tell it
		// to unload it's winampa.lng for what we're about to do..
		if (IsWindow(agent) && !already_extracted)
		{
			SendMessage(agent, WM_USER + 16, 1, 0);
		}

		// always remove winampa.lng just incase we crashed and it leaves things out of synch
		if(!already_extracted){
			StringCchPrintfW(winampaLngPath, MAX_PATH, L"%s\\winampa.lng", CONFIGDIR);
			DeleteFileW(winampaLngPath);
		}

		config_load_langpack_var();
		if(!already_extracted)
		{
			already_extracted = 1;
			prev_wlz_ex_state = is_wlz = extract_wlz_to_dir(0);
			load_extra_lng();
		}
		else
		{
			is_wlz = prev_wlz_ex_state;
			agent = 0;
		}

		// make sure that we don't try and load the exe/dll being localised as the lng dll
		wchar_t modulename[MAX_PATH], *p = 0;
		GetModuleFileNameW(hinstance, modulename, MAX_PATH);
		p = scanstr_backW(modulename, L"\\/", NULL);
		if(p) p = CharNextW(p);

		// if is_wlz != 0 then we can attempt to use the wlz extracted files otherwise
		// (for the time being) we drop back to the older lng pack system
		// either way we still need to make sure that what we're using is valid
		if (config_langpack[0] && is_wlz)
		{
			HMODULE h = langManager->FindDllHandleByGUID(guid);
			if(!h)	// possible fallback usage if things failed to work on guid look up
			{		// though wouldn't be reliable if people change the lng file names
				wchar_t tmpfile[MAX_PATH], *t = 0;
				lstrcpynW(tmpfile,p,MAX_PATH);
				t = scanstr_backW(tmpfile, L".", NULL);
				lstrcpynW(t,L".lng",MAX_PATH);
				h = langManager->FindDllHandleByStringW(tmpfile);
			}

			if (h)
			{
				// if the wlz was able to be loaded (as we believe at this point)
				// then we see if Winamp Agent is running and tell it to refresh
				// it's version of winampa.lng once we've copied into %inidir%
				if (IsWindow(agent))
				{
					// copy from the wlz folder to the settings folder
					StringCchPrintfW(winampaWlzPath, MAX_PATH, L"%s\\winampa.lng", lang_directory);
					CopyFileW(winampaWlzPath,winampaLngPath,FALSE);
					SendMessage(agent, WM_USER + 16, 0, 0);
				}

				// if we get here then we've managed to load the language pack
				// (still could be invalid but that's generally from failed dll files)
				return h;
			}
		}
	}

	// make sure we return the passed hinstance incase of failure to load/invalid lng file/etc
	return hinstance;
}


void Lang_EndLangSupport(void)
{
	// need to fully clean up things here including unloading of the langpack
	if(language_pack_instance != hMainInstance)
	{
		FreeLibrary(language_pack_instance);
		language_pack_instance = hMainInstance;
	}

	for(size_t i = 0; i < lnglist.size(); i++)
	{
		if(lnglist[i]->module)
			free(lnglist[i]->module);
		if(lnglist[i]->guidstr)
			free(lnglist[i]->guidstr);
		if(lnglist[i]->hDllInstance)
			FreeLibrary(lnglist[i]->hDllInstance);
	}

	lnglist.clear();
	prev_wlz_ex_state = already_extracted = 0;
}


HINSTANCE Lang_FakeWinampLangHInst(HINSTANCE adjustedHInst){
	HINSTANCE previousHInst = language_pack_instance;
	language_pack_instance = adjustedHInst;
	return previousHInst;
}


void Lang_LocaliseAgentOnTheFly(BOOL refresh){
	// if we need to refresh then attempt to use the winampa.lng from the
	// current language pack if one is present and has been extracted so
	// we test to see if we've extracted a language pack already
	if(already_extracted){
	HWND agent = FindWindow("WinampAgentMain", NULL);
	wchar_t winampaLngPath[MAX_PATH];

		// if we find Winamp Agent running then we need to tell it
		// to unload it's winampa.lng for what we're about to do...
		// although this is likely to be a new load, doing this will
		// help to ensure that things are unloaded incase of issues
		if(IsWindow(agent)){
			SendMessage(agent, WM_USER + 16, 1, 0);
		}

		// always remove winampa.lng just incase we crashed and it leaves things out of synch
		StringCchPrintfW(winampaLngPath, MAX_PATH, L"%s\\winampa.lng", CONFIGDIR);
		DeleteFileW(winampaLngPath);

		if(refresh){
		wchar_t winampaWlzPath[MAX_PATH];
			StringCchPrintfW(winampaWlzPath, MAX_PATH, L"%s\\winampa.lng", lang_directory);
			CopyFileW(winampaWlzPath,winampaLngPath,FALSE);
			SendMessage(agent, WM_USER + 16, 0, 0);
		}
	}
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Language
START_DISPATCH;
CB(API_LANGUAGE_GETSTRING, GetString)
CB(API_LANGUAGE_GETSTRINGW, GetStringW)
CB(API_LANGUAGE_GETSTRINGFROMGUID, GetStringFromGUID)
CB(API_LANGUAGE_GETSTRINGFROMGUIDW, GetStringFromGUIDW)
CB(API_LANGUAGE_GETHINSTANCEBYGUID, FindDllHandleByGUID)
CB(API_LANGUAGE_GETHINSTANCEBYNAME, FindDllHandleByString)
CB(API_LANGUAGE_GETHINSTANCEBYNAMEW, FindDllHandleByStringW)
CB(API_LANGUAGE_STARTUP, StartLanguageSupport)
CB(API_LANGUAGE_GETLANGUAGEFOLDER, GetLanguageFolder)
CB(API_LANGUAGE_CREATELDIALOGPARAM, CreateLDialogParam)
CB(API_LANGUAGE_LDIALOGBOXPARAM, LDialogBoxParam)
CB(API_LANGUAGE_LOADLMENU, LoadLMenu)
CB(API_LANGUAGE_CREATELDIALOGPARAMW, CreateLDialogParamW)
CB(API_LANGUAGE_LDIALOGBOXPARAMW, LDialogBoxParamW)
CB(API_LANGUAGE_LOADLMENUW, LoadLMenuW)
CB(API_LANGUAGE_GETLANGUAGEIDENTIFIER, GetLanguageIdentifier)
CB(API_LANGUAGE_LOADRESOURCEFROMFILE, LoadResourceFromFile)
CB(API_LANGUAGE_LOADRESOURCEFROMFILEW, LoadResourceFromFileW)
CB(API_LANGUAGE_LOADACCELERATORSA, LoadAcceleratorsA)
CB(API_LANGUAGE_LOADACCELERATORSW, LoadAcceleratorsW)
CB(API_LANGUAGE_USEUSERNUMERICLOCALE, UseUserNumericLocale)
CB(API_LANGUAGE_GET_C_NUMERICLOCALE, Get_C_NumericLocale)
CB(API_LANGUAGE_FORMATTEDSIZESTRING, FormattedSizeString)
END_DISPATCH