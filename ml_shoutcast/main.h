#ifndef NULLSOFT_ML_SHOUTCAST_MAIN_H
#define NULLSOFT_ML_SHOUTCAST_MAIN_H

#include "../gen_ml/ml.h"
#include "../winamp/wa_ipc.h"
#include "../nu/MediaLibraryInterface.h"

#include "../nde/nde_c.h"
extern wchar_t ini_file[MAX_PATH];
void Download();
void DownloadTV();
void CloseDatabase();
void OpenDatabase();
nde_table_t CreateRadioTable(const wchar_t *filename, const wchar_t *indexname);

INT_PTR WINAPI StationProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI TVProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern HWND radioHWND, tvHWND;
extern winampMediaLibraryPlugin plugin;

#include "../xml/ifc_xmlreadercallback.h"

class Updater : public ifc_xmlreadercallback
{
public:
	virtual void Open(const wchar_t *filename, const wchar_t *index)=0;
	bool Run();
	void Stop();
	virtual void Close()=0;
	virtual const char *GetUrl()=0;
	virtual HWND GetUpdateWindow()=0;
	
protected:
	bool running;
};

extern nde_database_t db;

enum
{
	RADIOTABLE_ID = 0,
	RADIOTABLE_NAME = 1,
	RADIOTABLE_MIMETYPE = 2,
	RADIOTABLE_BITRATE = 3,
	RADIOTABLE_NOWPLAYING = 4,
	RADIOTABLE_GENRE = 5,
	RADIOTABLE_LISTENERS = 6,
};

extern nde_table_t radio_table;
extern nde_scanner_t radio_scanner;
#include "../nu/AutoLock.h"
extern Nullsoft::Utility::LockGuard radio_guard;
#endif
