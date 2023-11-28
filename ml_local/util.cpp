#include "main.h"
#include "../nu/ns_wc.h"
#include "ml_local.h"
#include <string.h>
#include <shlobj.h>
#include "config.h"
#include "resource.h"

extern winampMediaLibraryPlugin lMedia;

extern "C" {

void process_substantives(wchar_t* dest)
{
	if(!substantives)
	{
		wchar_t *b = dest;
		while (!IsCharSpaceW(*b) && *b) b++;
		while (IsCharSpaceW(*b) && *b) b++;
		while (!IsCharSpaceW(*b) && *b) b++;
		CharLowerW(b);
	}
}

char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s=str+strlen(str)-1;
	if (strlen(str) < 1) return defval;
	if (strlen(toscan) < 1) return defval;
	while (1)
	{
		char *t=toscan;
		while (*t)
			if (*t++ == *s) return s;
		t=CharPrev(str,s);
		if (t==s) return defval;
		s=t;
	}
}

char *extension(char *fn) 
{
  char *s = scanstr_back(fn,".\\",fn-1);
  if (s < fn) return "";
  if (*s == '\\') return "";
  return (s+1);
}

wchar_t *GetLastCharacterW(wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

wchar_t *scanstr_backW(wchar_t *str, wchar_t *toscan, wchar_t *defval)
{
	wchar_t *s = GetLastCharacterW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	while (1)
	{
		wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

wchar_t *extensionW(wchar_t *fn)
{
	// TODO: deal with making sure that URLs don't return .com, etc.
	// e.g. http://www.winamp.com  should return nothing
	wchar_t *end = scanstr_backW((wchar_t*)fn, L"./\\", 0);
	if (!end)
		return (wchar_t *)(fn+lstrlenW(fn));

	if (*end == L'.')
		return CharNextW(end);

	return (wchar_t*)(fn+lstrlenW(fn));
}
	
HRESULT ResolveShortCut(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath)
{
	HRESULT hres;
	IShellLink* psl;
	char szGotPath[MAX_PATH];
	WIN32_FIND_DATA wfd;

	*pszPath = 0;   // assume failure

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
							IID_IShellLink, (void **) &psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf); // OLE 2!  Yay! --YO
		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH];
			MultiByteToWideCharSZ(CP_ACP, 0, pszShortcutFile, -1, wsz, MAX_PATH);

			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(hwnd, SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					strcpy(szGotPath, pszShortcutFile);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH );
					strcpy(pszPath,szGotPath);
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}

void ConvertRatingMenuStar(HMENU menu, UINT menu_id)
{
MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
wchar_t rateBuf[32], *rateStr = rateBuf;
	mi.dwTypeData = rateBuf;
	mi.cch = 32;
	if(GetMenuItemInfoW(menu, menu_id, FALSE, &mi))
	{
		while(rateStr && *rateStr)
		{
			if(*rateStr == L'*') *rateStr = L'\u2605';
			rateStr=CharNextW(rateStr);
		}
		SetMenuItemInfoW(menu, menu_id, FALSE, &mi);
	}
}

};