#include "./main.h"
#include <string.h>
#include <shlobj.h>
#include "./config.h"
#include "./resource.h"

#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "api.h"
#include <strsafe.h>
#include <api/syscb/callbacks/browsercb.h>



extern "C" {
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

};

void myOpenURLWithFallback(HWND hwnd, wchar_t *loc, wchar_t *fallbackLoc)
{
	bool override=false;
	if (loc)
	{
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::BROWSER, BrowserCallback::ONOPENURL, reinterpret_cast<intptr_t>(loc), reinterpret_cast<intptr_t>(&override));
	}
	if (!override && fallbackLoc)
		ShellExecuteW(hwnd, L"open", fallbackLoc, NULL, NULL, SW_SHOWNORMAL);
}

void FixAmps(char *str, size_t len)
{
	size_t realSize=0;
	size_t extra=0;
	char *itr = str;
	while (*itr)
	{
		if (*itr == '&')
			extra++;
		itr++;
		realSize++;
	}

	extra = min(len-(realSize+1), extra); 
	
	while (extra)
	{
		str[extra+realSize]=str[realSize];
		if (str[realSize] == '&')
		{		
			extra--;
			str[extra+realSize]='&';
		}		
		realSize--;
	}
}

LPWSTR FixAmpsW(LPWSTR pszText, INT cchMaxText)
{
	INT realSize, extra;
	LPWSTR itr;
			
	for (itr = pszText, extra = 0; NULL != *itr; itr++) if (L'&' == *itr) extra++;
	
	realSize = (INT)(itr - pszText);

	for (extra = min(cchMaxText - (realSize + 1), extra); extra > 0; realSize--)
	{
		pszText[extra+realSize] = pszText[realSize];
		if (L'&' == pszText[realSize])
		{		
			extra--;
			pszText[extra+realSize] = L'&';
		}		
	}
	return pszText;
}

bool IsVista()
{
	static INT fVista = -1; 
	
	if (-1 == fVista) 
	{
		OSVERSIONINFO osver;
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fVista = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 	(osver.dwMajorVersion >= 6 )) ? 1 : 0;
	}		

	return (1 == fVista);
}


