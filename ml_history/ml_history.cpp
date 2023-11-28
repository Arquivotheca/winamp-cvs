#pragma warning(disable:4530)
#include "main.h"
#include "config.h"
#include "gaystring.h" 
#include "../gen_hotkeys/wa_hotkeys.h"
#include "../gen_ml/MediaLibraryCOM.h"
#include "../gen_ml/ml_ipc_0313.h"
#include <malloc.h>

extern "C"
{
	HWND g_hwnd;
};

int ml_history_tree;
static HWND m_curview_hwnd = NULL;
static WNDPROC wa_oldWndProc=0;
static wchar_t *history_fn;
static int timer=-1;
static wchar_t *last_history_fn;
static int last_timer=-1;
static int last_play_pos=0;
static int history_fn_mode;

nde_database_t g_db;
nde_table_t g_table;
int g_table_dirty;
C_Config *g_config;
CRITICAL_SECTION g_db_cs;

HWND onTreeViewSelectChange(HWND hwnd)
{
	openDb();
	if (m_curview_hwnd)	DestroyWindow(m_curview_hwnd);

	if (!g_table || nde_error)
	{
		m_curview_hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_DB_ERROR, hwnd, view_errorinfoDialogProc, 0);
	}
	else
	{
		m_curview_hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_RECENTITEMS, hwnd, view_historyDialogProc, 0);
	}
	return m_curview_hwnd; 
}

static void history_cleanupifnecessary();
static LRESULT APIENTRY wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int history_init()
{
	if (!g_config->ReadInt("showrecentitems", 1)) return 1;

	NAVINSERTSTRUCT nis = {0};
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText = WASABI_API_LNGSTRINGW(IDS_HISTORY);
	nis.item.pszInvariant = L"History";
	nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
	nis.item.iSelectedImage = nis.item.iImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_RECENT);

	// map to item id (will probably have to change but is a quick port to support invariant item naming)
	NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
	nvItem.hItem = MLNavCtrl_InsertItem(lMedia.hwndLibraryParent, &nis);
	MLNavItem_GetInfo(lMedia.hwndLibraryParent, &nvItem);
	ml_history_tree = nvItem.id;
	
	if (!wa_oldWndProc) // don't double dip (we call history_init() dynamically if the user fiddles with prefs
	{
		wa_oldWndProc = (WNDPROC)SetWindowLongPtrW(lMedia.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)wa_newWndProc);
	}
	return 1;
}

void history_quit()
{
	if (last_history_fn) 
	{
		if ((last_timer <= 0) || (last_play_pos > last_timer))
		{
			if (last_play_pos) history_onFile(last_history_fn, last_play_pos);
		}
	}
	free(last_history_fn);
	last_history_fn = 0;
	free(history_fn);
	history_fn = 0;
	closeDb();
	mediaLibrary.RemoveTreeItem(ml_history_tree);
	ml_history_tree=0;
}

static void RetypeFilename(nde_table_t table)
{
	// TODO: UI
	int totalRecords = NDE_Table_GetRecordsCount(g_table);
	if (totalRecords == 0) // bail out early so we don't flash a dialog
		return;
	nde_scanner_t pruneScanner = NDE_Table_CreateScanner(table);
	int recordNum = 0;
	if (pruneScanner)
	{
		NDE_Scanner_First(pruneScanner);
		while (!NDE_Scanner_EOF(pruneScanner))
		{
			nde_field_t f = NDE_Scanner_GetFieldByID(pruneScanner, HISTORYVIEW_COL_FILENAME);
			if (f && NDE_Field_GetType(f) == FIELD_STRING)
			{
				wchar_t *s = NDE_StringField_GetString(f);
				ndestring_retain(s);

				NDE_Scanner_DeleteField(pruneScanner, f);

				nde_field_t new_f = NDE_Scanner_NewFieldByID(pruneScanner, HISTORYVIEW_COL_FILENAME);
				NDE_StringField_SetNDEString(new_f, s);

				ndestring_release(s);
				NDE_Scanner_Post(pruneScanner);
			}
			else if (f)
				break;

			NDE_Scanner_Next(pruneScanner);
		}

		NDE_Table_DestroyScanner(table, pruneScanner);
		NDE_Table_Sync(table);
	}
}

static void CreateFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_LASTPLAYED, L"lastplay", FIELD_DATETIME);
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_PLAYCOUNT, L"playcount", FIELD_INTEGER);
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_TITLE, L"title", FIELD_STRING);
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_LENGTH, L"length", FIELD_INTEGER);
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_FILENAME, L"filename", FIELD_FILENAME);
	NDE_Table_NewColumnW(g_table, HISTORYVIEW_COL_OFFSET, L"offset", FIELD_INTEGER);
	NDE_Table_PostColumns(g_table);
	NDE_Table_AddIndexByIDW(g_table, 0, L"filename");}

int openDb()
{
	if (g_table) return 1; // need to close first

	EnterCriticalSection(&g_db_cs);

	// benski> i know this looks redundant, but we might have sat and blocked at the above Critical Section for a while
	if (g_table) 
	{
		LeaveCriticalSection(&g_db_cs);
		return 1; 
	}

	if (!g_db)
	{
		__try
		{
			g_db = NDE_CreateDatabase(lMedia.hDllInstance);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			g_db = NULL;
			LeaveCriticalSection(&g_db_cs);
			return 0;
		}
	}
	
	wchar_t tableName[MAX_PATH], indexName[MAX_PATH];
	PathCombineW(tableName, g_tableDir, L"recent.dat");
	PathCombineW(indexName, g_tableDir, L"recent.idx");

	if (!g_db) 
	{
		LeaveCriticalSection(&g_db_cs);
		return 0;
	}
	g_table = NDE_Database_OpenTable(g_db, tableName, indexName, NDE_OPEN_ALWAYS, NDE_CACHE);
	if (g_table)
	{
		CreateFields(g_table);
		RetypeFilename(g_table);
	}
	LeaveCriticalSection(&g_db_cs);
	return (g_table != NULL);
}

void closeDb(bool clear_dirty)
{
	if (g_table_dirty && g_table)
	{
		history_bgQuery_Stop();
		NDE_Table_Sync(g_table);
		if (clear_dirty) g_table_dirty=0;
	}
	if (g_db)
	{
		__try
		{
			if (g_table)
				NDE_Database_CloseTable(g_db, g_table);

			NDE_DestroyDatabase(g_db);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	g_db = NULL;
	g_table = NULL;
}

INT_PTR pluginHandleIpcMessage(int msg, INT_PTR param)
{
	return (INT_PTR) SendMessage(lMedia.hwndLibraryParent, WM_ML_IPC, param, msg);
}

static LRESULT APIENTRY wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN) &&
	    wParam == 'H' &&
	    !(GetAsyncKeyState(VK_MENU)&0x8000) &&
	    !(GetAsyncKeyState(VK_SHIFT)&0x8000) &&
	    (GetAsyncKeyState(VK_CONTROL)&0x8000)
		&& ml_history_tree)
	{
		mediaLibrary.ShowMediaLibrary();
		mediaLibrary.SelectTreeItem(ml_history_tree);
	}
	else if (history_fn && uMsg == WM_TIMER && wParam == 8082)
	{
		if (!history_fn_mode || SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_GETOUTPUTTIME) > 350)
		{
			KillTimer(hwndDlg, 8082);
			if (SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
			{
				history_onFile(history_fn, -1);
			}
		}
		return CallWindowProcW(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
	}
	else if (last_history_fn && uMsg == WM_TIMER && wParam == 8083)
	{
		KillTimer(hwndDlg, 8083);
		history_onFile(last_history_fn, last_play_pos);
		
		return CallWindowProcW(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
	}
	else if (uMsg == WM_WA_IPC)
	{
		if (lParam == IPC_PLAYING_FILEW)
		{
			if (wParam)
			{
				const wchar_t *f = (const wchar_t *)wParam;
				KillTimer(hwndDlg, 8082);
				free(history_fn);
				history_fn = 0;
				history_fn_mode = 0;
				if (wcsstr(f, L"://") && _wcsnicmp(f, L"cda://", 6) && _wcsnicmp(f, L"file://", 7))
				{
					history_fn_mode = 1;
				}
				history_fn = _wcsdup(f);
				
				int timer1 = -1, timer2 = -1, timer3 = -1;

				// wait for x seconds
				if(g_config->ReadInt("recent_wait_secs",0))
				{
					timer1 = g_config->ReadInt("recent_wait_secs_lim",5)*1000;
				}

				// wait for x percent of the song (approx to a second)
				if(g_config->ReadInt("recent_wait_percent",0))
				{
					basicFileInfoStructW bfiW = {0};
					bfiW.filename = history_fn;
					SendMessage(hwndDlg, WM_WA_IPC, (WPARAM)&bfiW, IPC_GET_BASIC_FILE_INFOW);
					if(bfiW.length > 0)
					{
						bfiW.length=bfiW.length*1000;
						timer2 = (bfiW.length*g_config->ReadInt("recent_wait_percent_lim",50))/100;
					}
				}

				// wait for the end of the item (within the last second of the track hopefully)
				if(g_config->ReadInt("recent_wait_end",0))
				{
					basicFileInfoStructW bfiW = {0};
					bfiW.filename = history_fn;
					SendMessage(hwndDlg, WM_WA_IPC, (WPARAM)&bfiW, IPC_GET_BASIC_FILE_INFOW);
					if(bfiW.length > 0)
					{
						timer3=(bfiW.length-1)*1000;
					}
				}

				// decide on which playback option will be the prefered duration (smallest wins)
				if(timer1 != -1 && timer2 != -1)
				{
					if(timer1 > timer2)
					{
						timer = timer2;
					}
					if(timer2 > timer1)
					{
						timer = timer1;
					}
				}
				else if(timer1 == -1 && timer2 != -1)
				{
					timer = timer2;
				}
				else if(timer2 == -1 && timer1 != -1)
				{
					timer = timer1;
				}

				// only track on end of file as very last method
				if((timer <= 0) && (timer3 > 0)){ timer = timer3; }
				
				// if no match or something went wrong then try to ensure the default timer value is used
				SetTimer(hwndDlg, 8082, ((timer > 0)? timer : 350), NULL);
			}
			return CallWindowProc(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
		}
		else if(lParam == IPC_STOPPLAYING && g_config->ReadInt("resumeplayback",0))
		{
			KillTimer(hwndDlg, 8082);
			free(last_history_fn);
			last_history_fn = 0;
			if (history_fn) last_history_fn = _wcsdup(history_fn);
			last_timer = timer;

			stopPlayingInfoStruct *stopPlayingInfo = (stopPlayingInfoStruct *)wParam;
			last_play_pos = stopPlayingInfo->last_time;

			if (!stopPlayingInfo->g_fullstop)
			{
				if ((last_timer <= 0) || (last_play_pos > last_timer)) 
				{
					if (last_play_pos) SetTimer(hwndDlg, 8083, 150, NULL);
				}
			}
			else // clean up play offset 
			{
				history_onFile(last_history_fn, 0);
			}
		}
		else if(lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_TITLE)
		{
			if(g_config->ReadInt("resumeplayback",0))
			{
				int is_playing = SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_ISPLAYING);
				int play_pos = SendMessage(hwndDlg,WM_WA_IPC,0,IPC_GETOUTPUTTIME);
				if(is_playing == 1 && !(play_pos/1000 > 0)) //playing, look up last play offset and send seek message
				{
					wchar_t genre[256]={0,};
					extendedFileInfoStructW efis={
						history_fn,
						L"genre",
						genre,
						sizeof(genre)/sizeof(genre[0]),
					};
					SendMessage(hwndDlg,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW); 

					wchar_t ispodcast[8]={0,};
					extendedFileInfoStructW efis1={
						history_fn,
						L"ispodcast",
						ispodcast,
						sizeof(ispodcast)/sizeof(ispodcast),
					};
					SendMessage(hwndDlg,WM_WA_IPC,(WPARAM)&efis1,IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);
					
					if ((ispodcast[0] && _wtoi(ispodcast) > 0) || (genre[0] && !_wcsicmp(genre, L"podcast")) )
					{
						int offset = retrieve_offset(history_fn);
						if (offset && offset/1000 > 0) SendMessage(hwndDlg,WM_WA_IPC,offset,IPC_JUMPTOTIME);
					}
				}
			}
		}
		else if(lParam == IPC_WRITECONFIG && wParam)
		{
			closeDb();
			openDb();
		}
	} // wm_wa_ipc
	return CallWindowProcW(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}