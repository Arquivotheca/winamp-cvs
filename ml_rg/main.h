#ifndef NULLSOFT_ML_RG_MAIN_H
#define NULLSOFT_ML_RG_MAIN_H

#include <windows.h>
#include "../gen_ml/ml.h"
#include <windowsx.h>
#include "../winamp/wa_ipc.h"
#include "../gen_ml/ml.h"
#include "resource.h"
#include "../nu/String.h"
#include "../nu/Vector.h"
#include "../nu/PtrMap.h"
#include "../nu/Map.h"
#include "../nu/PtrList.h"
#include "libebur128/ebur128.h"

extern winampMediaLibraryPlugin plugin;
extern char *iniFile;

LRESULT SetFileInfo(const wchar_t *filename, const wchar_t *metadata, const wchar_t *data);
int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len);
void WriteFileInfo();
void TagUpdated(const wchar_t *filename);

class RGWorkFile
{
public:
	RGWorkFile(const wchar_t *_filename=0) 
	{
		if (_filename)
			lstrcpynW(filename, _filename, MAX_PATH);
		else
			filename[0]=0;
		track_gain[0]=0;
		track_peak[0]=0;
		
		state=0;
	}

	~RGWorkFile()
	{
		if (state)
			ebur128_destroy(&state);
	}

	wchar_t filename[MAX_PATH];
	wchar_t track_gain[64];
	wchar_t track_peak[64];
	
	ebur128_state *state;
};

class RGWorkAlbum
{
public:
	RGWorkAlbum()
	{
		album_gain[0]=0;
		album_peak[0]=0;
	}
	~RGWorkAlbum()
	{
		files.deleteAll();
	}
	RGWorkAlbum &operator = (const RGWorkAlbum &copy)
	{
		lstrcpynW(album_gain, copy.album_gain, 64);
		lstrcpynW(album_peak, copy.album_peak, 64);
		for (RGWorkFiles::iterator itr=copy.files.begin();itr!=copy.files.end();itr++)
		{
			files.push_back(new RGWorkFile(**itr));
		}
		return *this;
	}
	typedef nu::PtrList<RGWorkFile> RGWorkFiles;
	RGWorkFiles files;
	wchar_t album_gain[64];
	wchar_t album_peak[64];
};

class ProgressCallback;

class WorkQueue
{
public:
	WorkQueue() : totalFiles(0){}
	~WorkQueue() 
	{
		//albums.deleteAll(); 
	}
	void Add(const wchar_t *filename);
	void Calculate(ProgressCallback *callback, int *killSwitch);
	typedef nu::Map<UnicodeString, RGWorkAlbum> AlbumMap;
	AlbumMap albums;
	RGWorkAlbum unclassified;
	size_t totalFiles;
};

class ProgressCallback
{
public:
	ProgressCallback(HWND _c) : callback(_c) {}
	void InformSize(size_t bytes)
	{
		PostMessage(callback, WM_USER + 3, 0, bytes);
	}
	void Progress(size_t bytes)
	{
		PostMessage(callback, WM_USER + 4, 0, bytes);
	}
	void FileFinished()
	{
		PostMessage(callback, WM_USER, 0, 0);
	}
	void AlbumFinished(RGWorkAlbum *album)
	{
		PostMessage(callback, WM_USER + 1, 0, (LPARAM)album);
	}
	HWND callback;
};

void CopyAlbumData(RGWorkAlbum &album, const wchar_t *album_gain, const wchar_t *album_peak);
void WriteAlbum(RGWorkAlbum &album);
void WriteTracks(RGWorkAlbum &album);
void DoResults(RGWorkAlbum &album);
void DoResults(WorkQueue &queue);
void DoProgress(WorkQueue &workQueue);

void DestroyRG(void *context);
void *CreateRG();
void CalculateAlbumRG(void *context, wchar_t album_gain[64], wchar_t album_peak[64], float &albumPeak);
void StartRG(void *context);
void CalculateRG(void *context, const wchar_t *filename, wchar_t track_gain[64], wchar_t track_peak[64], ProgressCallback *callback, int *killSwitch, float &albumPeak);

HWND GetDialogBoxParent();
BOOL windowOffScreen(HWND hwnd, POINT pt);

extern int config_ask, config_ask_each_album, config_ignore_gained_album;
INT_PTR WINAPI RGConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif