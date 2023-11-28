/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include <io.h>
#include <sys/stat.h>
#include "api.h"
#include "WinampPlaylist.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>

void saveplsfn(const wchar_t *_fn)
{
	wchar_t fnbuf[FILENAME_SIZE], ftbuf[FILETITLE_SIZE];
	int x;
	int pos = PlayList_getPosition();
	PlayList_setposition(0);
	x = 0;
	FILE *plsFile = _wfopen(_fn, L"wt");
	if(plsFile)
	{
		fprintf(plsFile, "[playlist]\r\n");
		for (;;)
		{
			int plspos = PlayList_getPosition();
			if (!PlayList_gethidden(plspos) && !PlayList_hasanycurtain(plspos))
			{
				PlayList_getitem2W(plspos, fnbuf, ftbuf);
				PlayList_makerelative(_fn, fnbuf, 0);
				fprintf(plsFile, "File%d=%s\r\n", ++x, (char *)AutoChar(fnbuf));
				/*
				const char *sBrowser = PlayList_getbrowser(plspos);
				if (sBrowser)
					fprintf(plsFile, "Browser%d=%s\r\n", x, sBrowser);
					*/

				if (PlayList_getcached(plspos))
				{
					fprintf(plsFile, "Title%d=%s\r\n", x, (char *)AutoChar(ftbuf));
					fprintf(plsFile, "Length%d=%d\r\n", x, PlayList_getcurrentlength());
				}
				int repeat =  PlayList_getrepeatcount(plspos);
				if (repeat > 0)
					fprintf(plsFile, "Repeat%d=%d\r\n", x, repeat);
			}
			if (PlayList_advance(1) < 0) break;
		}
		fprintf(plsFile, "NumberOfEntries=%d\r\n", x);
		fprintf(plsFile, "Version=2\r\n");

		PlayList_setposition(pos);
		fclose(plsFile);
	}
	return ;
}

int loadpls(HWND hwnd, int whattodo)
{
	if (!playlistManager)
	{
		LPMessageBox(hwnd, IDS_PLAYLIST_SUPPORT_NOT_INSTALLED,IDS_PLAYLIST_LOAD_ERROR, MB_OK);
		return 0;
	}
	wchar_t filter[2048];
	playlistManager->GetFilterList(filter, 2048);
	wchar_t temp[FILENAME_SIZE];

	wchar_t oldCurPath[MAX_PATH], newCurPath[MAX_PATH], titleStr[128];

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	OPENFILENAMEW l = {sizeof(OPENFILENAMEW),0};
	static int q;
	int retv = 0;
	if (q) return 0;
	q = 1;
	l.hwndOwner = DIALOG_PARENT(hMainWindow); //hwnd;
	l.hInstance = hMainInstance;
	l.lpstrFilter = filter;
	l.lpstrFile = temp;
	temp[0] = 0;
	l.nMaxFile = FILENAME_SIZE;
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();

	if (whattodo == 0) l.lpstrTitle = getStringW(IDS_OPEN_PLAYLIST,titleStr,128);
	else if (whattodo == 1) l.lpstrTitle = getStringW(IDS_ADD_PLAYLIST,titleStr,128);
	else l.lpstrTitle = getStringW(IDS_OPEN_PLAYLIST,titleStr,128);
	l.lpstrDefExt = L"pls";

	l.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;

	UninitDragDrops();
	if (GetOpenFileNameW(&l))
	{
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);

		q = 0;
		if (LoadPlaylist(temp, whattodo, 0) != 0)
			retv = 0;
		//else // TODO: this wasn't there in the old code, so we'll leave it out for now
		retv = 1;
	}
	SetCurrentDirectoryW(oldCurPath);
	q = 0;
	InitDragDrops();
	return retv;
}

static void checkplswritable(wchar_t *filename)
{
	if (!_waccess(filename, 0))
	{
		if (_waccess(filename, 2))
		{
			_wchmod(filename, _S_IREAD | _S_IWRITE);
		}
	}
}

int savepls(HWND hwnd)
{
	wchar_t oldCurPath[MAX_PATH], newCurPath[MAX_PATH], filterStr[MAX_PATH] = {0}, *fStr = filterStr;

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	static wchar_t temp[MAX_PATH];
	OPENFILENAMEW l = {sizeof(OPENFILENAMEW),0};
	static int q;
	if (q) return 0;
	q = 1;
	//  temp[0]=0;
	l.hwndOwner = hwnd;
	l.hInstance = hMainInstance;
	//l.lpstrFilter = L"M3U Playlist\0*.m3u\0PLS Playlist\0*.pls\0M3U8 (Unicode) Playlist\0*.m3u8\0";
	l.lpstrFilter = getStringW(IDS_PLAYLIST_FILTER_STRING,filterStr,MAX_PATH);
	// store the filter string with | separators so that parts can be translated as needed
	// (\0 or more \0000 will work but it's not reliable to work with that - the pipe is a
	// a work around from mfc apps wanting to do what we want for language support)
	while(fStr && *fStr)
	{
	wchar_t* rfsStr = 0;
		if(*fStr == L'|')
		{
			rfsStr = fStr;// = 0;
		}
		fStr = CharNextW(fStr);
		if(rfsStr)
		{
			*rfsStr = 0;
		}
	}
	l.lpstrFile = temp;
	l.nMaxFile = MAX_PATH - 1;
	l.lpstrTitle = getStringW(IDS_SAVE_PLAYLIST,NULL,0);
	l.lpstrDefExt = L"m3u";

	l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	if (GetSaveFileNameW(&l))
	{
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		SetCurrentDirectoryW(oldCurPath);

		checkplswritable(temp);
		q = 0;
		if (!_wcsicmp(extensionW(temp), L"pls"))
		{
			DeleteFileW(temp);
			saveplsfn(temp);
			return 1;
		}
		else // if it's not PLS, then it's m3u or m3u8, both of which are handled by the same function
		{
			savem3ufn(temp, 1, 0);
			return 1;
		}
	}
	
	SetCurrentDirectoryW(oldCurPath);
	q = 0;
	return 0;
}
