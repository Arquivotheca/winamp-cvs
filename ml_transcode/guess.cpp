#include <string.h>
#include <bfc/platform/types.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

wchar_t *guessTitles(const wchar_t *filename, 
                 int *tracknum,
                 wchar_t **artist, 
                 wchar_t **album,
                 wchar_t **title) // should free the result of this function after using artist/album/title
{
	*tracknum=0;
	*artist=0;
	*album=0;
	*title=0;

	if (!filename[0]) return 0;

	wchar_t *_artist=NULL;
	wchar_t *_album=NULL;

	const wchar_t *f=filename;
	while (*f) f++;
	while (f>filename && *f != '/' && *f != '\\') f--;

	if (f == filename) return 0;

	ptrdiff_t dirlen = f-filename;

	wchar_t *fullfn=_wcsdup(filename);
	fullfn[dirlen]=0;

	wchar_t *n=fullfn+dirlen;
	while (n >= fullfn && *n != '/' && *n != '\\') n--;
	n++;

	// try to guess artist and album from the dirname

	if (!wcsstr(n,L"-")) // assume dir name is album name, artist name unknown
	{
		*album=n;
		_album=n;
	}
	else 
	{
		*artist=_artist=n;
		_album=wcsstr(n,L"-")+1;
		wchar_t *t=_album-2;
		while (t >= n && *t == ' ') t--;
		t[1]=0;

		while (*_album == ' ') _album++;
		*album=_album;
	}


	// get filename shizit
	wcscpy(fullfn+dirlen+1,filename+dirlen+1);

	n=fullfn+dirlen+1;
	while (*n) n++;
	while (n > fullfn+dirlen+1 && *n != '.') n--;
	if (*n == '.') *n=0;
	n=fullfn+dirlen+1;

	while (*n == ' ') n++;

	// detect XX. filename
	if (wcsstr(n,L".") && _wtoi(n) && _wtoi(n) < 130)
	{
		wchar_t *tmp=n;
		while (*tmp >= '0' && *tmp <= '9') tmp++;
		while (*tmp == ' ') tmp++;
		if (*tmp == '.') 
		{ 
		*tracknum=_wtoi(n); 
		n=tmp+1; 
		while (*n == '.') n++;
		while (*n == ' ') n++;
		}
	}

	// detect XX- filename
	if (!*tracknum && wcsstr(n, L"-") && _wtoi(n) && _wtoi(n) < 130)
	{
		wchar_t *tmp=n;
		while (*tmp >= '0' && *tmp <= '9') tmp++;
		while (*tmp == ' ') tmp++;
		if (*tmp == '-') 
		{ 
		*tracknum=_wtoi(n); 
		n=tmp+1; 
		while (*n == '-') n++;
		while (*n == ' ') n++;
		}
	}
	 
	while (wcsstr(n,L"-"))
	{
		wchar_t *a=n;
		n=wcsstr(n,L"-");
		{
		wchar_t *t=n-1;
		while (t >= a && *t == ' ') t--;
		t[1]=0;
		}
		*n=0;
		n++;
		while (*n == '-') n++;
		while (*n == ' ') n++;

		// a is the next token.

		if (!*tracknum && !_wcsnicmp(a, L"Track ",6) && _wtoi(a+6)) *tracknum=_wtoi(a+6);
		else if (*artist== _artist)
		{
		*artist=a;
		}
		if (*artist != _artist && *tracknum) break;
	}
	*title=n;
	  
	return fullfn;
}