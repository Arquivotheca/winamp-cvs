// needed for handling the nde failed load error catching
#pragma warning(disable:4530)
#include "main.h"
#include "ml_local.h"
#include <windowsx.h>
#include <time.h>
#include <rpc.h>
#include <assert.h>
#include "resource.h"
#include "config.h"
#include "gaystring.h"

#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc.h"

#include "../nde/nde.h"
#include "../nde/nde_c.h"

#include "../gen_hotkeys/wa_hotkeys.h"
#include "../gen_ml/MediaLibraryCOM.h"
#include "api.h"

#include "../nu/AutoWide.h"

#include "./scanfolderbrowser.h"
#include "../nu/AutoChar.h"

#include "api_mldb.h"


int IPC_GET_ML_HMENU = -1, IPC_GET_CLOUD_HINST = -1, IPC_GET_CLOUD_ACTIVE = -1;

HMENU wa_playlists_cmdmenu = NULL;
HMENU wa_play_menu = 0;


wchar_t *fieldTagFunc(wchar_t * tag, void * p); //return 0 if not found

#define WA_MENUITEM_ID 23123


WNDPROC wa_oldWndProc;
BOOL myMenu = FALSE;

HCURSOR hDragNDropCursor;

C_Config *g_config;

embedWindowState myWindowState;

char g_burner_list[32] = "";

int asked_for_playcount = 0;
int g_bgrescan_int = 120, g_bgrescan_do = 0, g_bgrescan_force = 0, g_autochannel_do = 0;
int g_guessifany = 0;
int g_querydelay = 100;
int g_viewnotplay = 0;

char g_path[MAX_PATH] = {0};
char g_tableDir[MAX_PATH] = {0};
char g_viewsDir[MAX_PATH] = {0};

//DB db;

nde_database_t g_db=0;
nde_table_t g_table=0;
int g_table_dirty;

CRITICAL_SECTION g_db_cs;

HMENU g_context_menus;
HWND m_curview_hwnd = NULL;

wchar_t *m_query = L"";
int m_query_mode;

C_Config *g_view_metaconf = NULL;

static int m_query_moving;
static HTREEITEM m_query_moving_dragplace;
static HTREEITEM m_query_moving_item, m_query_moving_lastdest;
static int m_query_moving_dragplaceisbelow;

int m_query_tree;

QueryList m_query_list;

HWND remote_dialog_parent = NULL; // FG> for popping dialogboxes from IPC without having the dialog show up on the lower right corner of the screen

//xp theme disabling shit
static HMODULE m_uxdll;
HRESULT(__stdcall *SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
HRESULT(__stdcall *IsAppThemed)(void);

void db_setFieldStringW(nde_scanner_t s, unsigned char id, const wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_StringField_SetString(f, data);
}

void db_setFieldInt(nde_scanner_t s, unsigned char id, int data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_IntegerField_SetValue(f, data);
}

int db_getFieldInt(nde_scanner_t s, unsigned char id, int defaultVal)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_IntegerField_GetValue(f);
	else
		return defaultVal;
}

void db_setFieldInt64(nde_scanner_t s, unsigned char id, __int64 data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_Int64Field_SetValue(f, data);
}

__int64 db_getFieldInt64(nde_scanner_t s, unsigned char id, __int64 defaultVal)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_Int64Field_GetValue(f);
	else
		return defaultVal;
}

void db_removeField(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		NDE_Scanner_DeleteField(s, f);
	}
}

int pluginHandleIpcMessage(int msg, int param)
{
	return SendMessage(lMedia.hwndLibraryParent, WM_ML_IPC, param, msg);
}

void TAG_FMT_EXT(const wchar_t *filename, void *f, void *ff, void *p, wchar_t *out, int out_len, int extended)
{
	waFormatTitleExtended fmt;
	fmt.filename = filename;
	fmt.useExtendedInfo = extended;
	fmt.out = out;
	fmt.out_len = out_len;
	fmt.p = p;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = f;
	*(void **)&fmt.TAGFREEFUNC = ff;
	*out = 0;

	int oldCallingGetFileInfo = m_calling_getfileinfo;
	m_calling_getfileinfo = 1;
	SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
	m_calling_getfileinfo = oldCallingGetFileInfo;
}

void main_playItemRecordList(itemRecordListW *obj, int enqueue, int startplaying)
{
	assert(enqueue != -1); // benski> i'm pretty sure this isn't used anymore
	if (obj->Size && !enqueue) SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);

	int x;
	for (x = 0; x < obj->Size; x ++)
	{
		if (obj->Items[x].filename && *obj->Items[x].filename)
		{
			wchar_t title[2048];
			TAG_FMT_EXT(obj->Items[x].filename, itemrecordWTagFunc, ndeTagFuncFree, (void*)&obj->Items[x], title, 2047, 0);

			enqueueFileWithMetaStructW s;
			s.filename = obj->Items[x].filename;
			s.title = title;
			s.length = obj->Items[x].length;
#ifndef _DEBUG
			ndestring_retain(obj->Items[x].filename);
			SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW_NDE);
#else
			SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
#endif
		}
	}

	if (obj->Size && !enqueue && startplaying) SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
}

void main_playQuery(C_Config *viewconf, const wchar_t *query, int enqueue, int startplaying)
{
	// if enqueue is -1, we do it to the playlist
	if (!g_table) return ;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	NDE_Scanner_Query(s, query);
	itemRecordListW obj = {0, };
	// no need to have this provide compatible kb
	// based filesizes as we never use the values
	saveQueryToListW(viewconf, s, &obj, 0, 0, 0);
	NDE_Table_DestroyScanner(g_table, s);
	LeaveCriticalSection(&g_db_cs);

	main_playItemRecordList(&obj, enqueue, startplaying);

	freeRecordList(&obj);
}

time_t g_bgscan_last_rescan;
int g_bgscan_scanning;

HWND updateCurrentView(HWND hwndDlg)
{
	if (m_curview_hwnd)	DestroyWindow(m_curview_hwnd);
	m_curview_hwnd = NULL;
	delete g_view_metaconf;
	g_view_metaconf = 0;
	int id = -1;
	DLGPROC proc = 0;

	if (!g_table)
	{
		// try to show something better than a blank view
		id = IDD_VIEW_DB_ERROR;
		proc = view_errorinfoDialogProc;
	}
	else
	{
		switch (m_query_mode)
		{
		case 0:
			id = IDD_VIEW_MEDIA; proc = view_mediaDialogProc;
			break;
		default:
			id = IDD_VIEW_AUDIO; proc = view_audioDialogProc;
			break;
		}
	}

	if (id == -1) proc = NULL;

	if (id != -1)
	{
		char configDir[MAX_PATH];
		PathCombineA(configDir, g_viewsDir, m_query_metafile);
		g_view_metaconf = new C_Config(configDir);

		LPARAM lParam = 0;
		INT_PTR parms[2];

		if (g_config->ReadInt("useminiinfo", 1) && (proc == view_audioDialogProc || proc == view_mediaDialogProc))
		{
			parms[0] = (INT_PTR)proc;
			parms[1] = (INT_PTR)id;
			lParam = (LPARAM) & parms;

			id = IDD_VIEW_MINIINFO;
			proc = view_miniinfoDialogProc;
		}
		if (proc) m_curview_hwnd = WASABI_API_CREATEDIALOGPARAMW(id, hwndDlg, proc, lParam);
	}

	return m_curview_hwnd;
}

void makemetafn(char *filename, char **out)
{
	int x = 0;
	for (;;)
	{
		GetTempFileName(g_viewsDir, "meta", GetTickCount() + x*4050, filename);
		if (strlen(filename) > 4)	strcpy(filename + strlen(filename) - 4, ".vmd");
		HANDLE h = CreateFile(filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			*out = _strdup(filename + strlen(g_viewsDir) + 1 /* for \\*/);
			return ;
		}
		if (++x > 4096)
		{
			*out = _strdup("meta-error.vmd");
			filename[0] = 0;
			return ;
		}
	}
}

int ImageGuessFilter(int mode, const wchar_t* val, int index)
{
	if (index != -1) return index;

	if (!lstrcmpiW(val, L"lastupd > [3 days ago]")) return TREE_IMAGE_LOCAL_RECENTLYMODIFIED;
	if (!lstrcmpiW(val, L"dateadded > [3 days ago]")) return TREE_IMAGE_LOCAL_RECENTLYADDED;
	if (!lstrcmpiW(val, L"playcount > 0")) return TREE_IMAGE_LOCAL_MOSTPLAYED;
	if (!lstrcmpiW(val, L"rating >= 3")) return TREE_IMAGE_LOCAL_TOPRATED;
	if (!lstrcmpiW(val, L"playcount = 0 | playcount isempty")) return TREE_IMAGE_LOCAL_NEVERPLAYED;
	if (!lstrcmpiW(val, L"lastplay > [2 weeks ago]")) return TREE_IMAGE_LOCAL_RECENTLYPLAYED;
	if (!lstrcmpiW(val, L"type = 0")) return TREE_IMAGE_LOCAL_AUDIO;
	if (!lstrcmpiW(val, L"type = 1")) return TREE_IMAGE_LOCAL_VIDEO;
	return index;
}

int addQueryItem(const wchar_t *name, const wchar_t *val, int mode, int select, const char *metafn, int imageIndex, int num)
{
	MLTREEITEMW newItem;
	imageIndex = ImageGuessFilter(mode, val, imageIndex);

	newItem.size = sizeof(newItem);
	newItem.parentId = m_query_tree;
	newItem.title = const_cast<wchar_t *>(name);
	newItem.hasChildren = 0;
	newItem.id = 0;
	newItem.imageIndex = imageIndex;

	if(num <= 0) mediaLibrary.AddTreeItem(newItem);
	else
	{
		for(QueryList::iterator iter = m_query_list.begin(); iter != m_query_list.end(); iter++)
			if(iter->second && iter->second->index == num - 1) { newItem.id = iter->first; break; }
			mediaLibrary.InsertTreeItem(newItem);
	}

	queryItem *qi = (queryItem *)malloc(sizeof(queryItem));
	qi->name = _wcsdup(name);
	qi->query = _wcsdup(val);
	qi->mode = mode;
	qi->imgIndex = imageIndex;
	qi->index = m_query_list.size();

	if (!metafn || !metafn[0])
	{
		char filename[1024 + 256];
		makemetafn(filename, &qi->metafn);
	}
	else qi->metafn = _strdup(metafn);

	m_query_list.insert(QueryList::MapPair(newItem.id, qi));
	return newItem.id;
}

void replaceQueryItem(int n, const wchar_t *name, const wchar_t *val, int mode, int imageIndex)
{
	queryItem *qi;

	if (mode == 32)	return ;

	qi = m_query_list[n];
	free(qi->name);
	qi->name = _wcsdup(name);
	if (val)
	{
		free(qi->query);
		qi->query = _wcsdup(val);
	}
	qi->mode = mode;
	qi->imgIndex = imageIndex;

	MLTREEITEMW item;
	item.hasChildren = 0;
	item.id = n;
	item.title = const_cast<wchar_t *>(name);
	item.parentId = m_query_tree;
	item.imageIndex = imageIndex;

	mediaLibrary.SetTreeItem(item);
	mediaLibrary.SelectTreeItem(n - 1);
	mediaLibrary.SelectTreeItem(n);
}

wchar_t* def_names[] = {L"Audio", L"Video",
						L"Most Played",
						L"Recently Added",
						L"Recently Played",
						L"Never Played",
						L"Top Rated",
						L"Recently Modified",};
int def_str_ids[] = {IDS_AUDIO, IDS_VIDEO, IDS_MOST_PLAYED, IDS_RECENTLY_ADDED,
					 IDS_RECENTLY_PLAYED, IDS_NEVER_PLAYED, IDS_TOP_RATED,
					 IDS_RECENTLY_MODIFIED};

void loadQueryTree()
{
	int meta_add_dirty = 0;
	int nb = g_config->ReadInt("query_num", 0);
	int fix = g_config->ReadInt("query_fix", 0);
	int mig = g_config->ReadInt("query_mig", 0);

	g_config->WriteInt("query_fix", 1);
	g_config->WriteInt("query_mig", 1);

	// helps to migrate existing vmd files to plugins\ml\views
	char metafnold[MAX_PATH] = {0}, metafnnew[MAX_PATH] = {0};
	if (!mig)
	{
		PathCombineA(metafnold, g_tableDir, "default.vmd");
		PathCombineA(metafnnew, g_viewsDir, "default.vmd");
		if (!PathFileExistsA(metafnnew) && PathFileExistsA(metafnold))
		{
			MoveFile(metafnold, metafnnew);
		}
	}

	for (int i = 0; i < nb; i++)
	{
		char qn[128], qv[128], qm[128], qmet[128], qbmp[128];
		char name[1024], val[1024];
		wsprintf(qn, "query%i_name", i + 1);
		UINT codePage = CP_ACP;

		if (!g_config->ReadString(qn, NULL, name, 1024) || !*name)
		{
			wsprintf(qn, "query%i_name_utf8", i + 1);
			g_config->ReadString(qn, NULL, name, 1024);
			codePage = CP_UTF8;
			if (!name)
				continue;
		}

		wchar_t unicodeNameLoc[256] = {0};
		AutoWide unicodeName(name, codePage);

		wsprintf(qv, "query%i_val", i + 1);
		codePage = CP_ACP;
		if (!g_config->ReadString(qv, NULL, val, 1024) || !*val)
		{
			wsprintf(qv, "query%i_val_utf8", i + 1);
			g_config->ReadString(qv, NULL, val, 1024);
			codePage = CP_UTF8;
		}

		// this will convert 'lastupd > [3 days ago]' to 'dateadded > [3 days ago]'
		// on older client installs so we're making use of the new dateadded column
		if (!fix && val[0])
		{
			if (!_stricmp("lastupd > [3 days ago]", val))
			{
				 lstrcpyn(val, "dateadded > [3 days ago]", sizeof(val));
				 g_config->WriteString(qv, val);
			}
		}

		AutoWide unicodeVal(val, codePage);

		wsprintf(qm, "query%i_mode", i + 1);
		wsprintf(qmet, "query%i_meta", i + 1);
		wsprintf(qbmp, "query%i_image", i + 1);

		int mode = g_config->ReadInt(qm, -1);
		if (mode == 32 || mode == -1) continue; // old playlist or empty
		char metafn[MAX_PATH] = {0};
		g_config->ReadString(qmet, "", metafn, MAX_PATH);

		// helps to migrate existing vmd files to plugins\ml\views
		if (!mig)
		{
			metafnold[0] = metafnnew[0] = 0;
			PathCombineA(metafnold, g_tableDir, metafn);
			PathCombineA(metafnnew, g_viewsDir, metafn);
			if (!PathFileExistsA(metafnnew) && PathFileExistsA(metafnold))
			{
				MoveFile(metafnold, metafnnew);
			}
		}

		// see if we've got a name match to one of the defaults...
		for(int j = 0; j < sizeof(def_names)/sizeof(def_names[0]); j++)
		{
			if(!lstrcmpiW(unicodeName, def_names[j]))
			{
				WASABI_API_LNGSTRINGW_BUF(def_str_ids[j], unicodeNameLoc, 256);
				break;
			}
		}

		addQueryItem((unicodeNameLoc[0]?unicodeNameLoc:unicodeName), unicodeVal, mode, 0, metafn, max(g_config->ReadInt(qbmp, -1), -1));
	}

	int aapos = g_config->ReadInt("view_autoadd_pos", 0);

	if (aapos < 1)
	{
		if (!nb) // lame defaults added
		{
			meta_add_dirty = 1;
			addQueryItem(WASABI_API_LNGSTRINGW(IDS_AUDIO), L"type = 0", 1, 0, "", TREE_IMAGE_LOCAL_AUDIO); // new defaults
		}

		typedef struct
		{
			int title;
			wchar_t *query;
			char sort_by;
			char sort_dir;
			char *columns; //xff terminated list :)
			int imageIndex;
		}
		addstruct;

		addstruct m[] =
		{
			{IDS_VIDEO, L"type = 1", 10, 0, "\x7\1\5\x1E\6\3\x20\x8\x9\xA\xff", TREE_IMAGE_LOCAL_VIDEO},
			{IDS_MOST_PLAYED, L"playcount > 0", 9, 0, "\x9\0\1\2\3\xA\xff", TREE_IMAGE_LOCAL_MOSTPLAYED},
			{IDS_RECENTLY_ADDED, L"dateadded > [3 days ago]", 33, 0, "\x21\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYADDED},
			{IDS_RECENTLY_MODIFIED, L"lastupd > [3 days ago]", 11, 0, "\xB\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYMODIFIED},
			{IDS_RECENTLY_PLAYED, L"lastplay > [2 weeks ago]", 10, 0, "\xA\x9\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYPLAYED},
			{IDS_NEVER_PLAYED, L"playcount = 0 | playcount isempty", 0, 0, "\0\1\2\3\xff", TREE_IMAGE_LOCAL_NEVERPLAYED},
			{IDS_TOP_RATED, L"rating >= 3", 8, 0, "\x8\x9\0\1\2\3\xff", TREE_IMAGE_LOCAL_TOPRATED},
		};
		if (aapos < 1)
		{
			int x;
			for (x = 0; x < sizeof(m) / sizeof(m[0]); x++)
			{
				if (!x && nb) continue;

				char filename[1024 + 256];
				char *ptr = 0;
				makemetafn(filename, &ptr);

				if (filename[0])
				{
					C_Config foo(filename);
					foo.WriteInt("mv_sort_by", m[x].sort_by);
					foo.WriteInt("mv_sort_dir", m[x].sort_dir);
					int cnt = 0;
					while ((unsigned char)m[x].columns[cnt] != 0xff)
					{
						char buf[32];
						wsprintf(buf, "column%d", cnt);
						foo.WriteInt(buf, (unsigned char)m[x].columns[cnt]);
						cnt++;
					}
					foo.WriteInt("nbcolumns", cnt);
					meta_add_dirty = 1;
					addQueryItem(WASABI_API_LNGSTRINGW(m[x].title), m[x].query, 0, 0, ptr, m[x].imageIndex);
				}
				free(ptr);
			}
		}
		g_config->WriteInt("view_autoadd_pos", 1);
	}

	if (meta_add_dirty) saveQueryTree();
}

void saveQueryTree()
{
	int nb = g_config->ReadInt("query_num", 0);
	QueryList::iterator iter;
	int i = 1;
	char qn[128], qv[128], qm[128], qmet[128], qbmp[128];
	for (int curId = mediaLibrary.GetChildId(m_query_tree); curId != 0; curId = mediaLibrary.GetNextId(curId), i++)
	{
		iter = m_query_list.find(curId);
		if (iter == NULL || iter == m_query_list.end()) continue;

		if (i <= nb)
		{
			do
			{
				wsprintf(qm, "query%i_mode", i);
			}
			while (g_config->ReadInt(qm, -1) == 32 && i++);
		}

		wsprintf(qn, "query%i_name", i);
		wsprintf(qv, "query%i_val", i);
		g_config->WriteString(qn, 0); // erase these old config items
		g_config->WriteString(qv, 0); // erase these old config items

		wsprintf(qn, "query%i_name_utf8", i);
		wsprintf(qv, "query%i_val_utf8", i);
		wsprintf(qm, "query%i_mode", i);
		wsprintf(qmet, "query%i_meta", i);
		wsprintf(qbmp, "query%i_image", i);

		queryItem *thisitem = iter->second;
		if (thisitem == NULL) continue;

		char charNameLoc[256] = {0};
		// see if we've got a name match to one of the defaults...
		for(int j = 0; j < sizeof(def_names)/sizeof(def_names[0]); j++)
		{
			if(!lstrcmpiW(thisitem->name, WASABI_API_LNGSTRINGW(def_str_ids[j])))
			{
				lstrcpyn(charNameLoc,AutoChar(def_names[j]),256);
				break;
			}
		}

		g_config->WriteString(qn, (charNameLoc[0]?charNameLoc:AutoChar(thisitem->name, CP_UTF8)));
		g_config->WriteString(qv, AutoChar(thisitem->query, CP_UTF8));
		g_config->WriteInt(qm, thisitem->mode);
		g_config->WriteString(qmet, thisitem->metafn);
		g_config->WriteInt(qbmp, max(thisitem->imgIndex, -1));
	}

	i--;
	if (i < nb)
	{
		for (int k = i + 1; k <= nb; k++)
		{
			wsprintf(qm, "query%i_mode", k);
			int mode = g_config->ReadInt(qm, -1);
			if (32 == mode) i++;
			else
			{
				wsprintf(qn, "query%i_name", k);
				wsprintf(qv, "query%i_val", k);
				wsprintf(qm, "query%i_mode", k);
				wsprintf(qmet, "query%i_meta", k);
				wsprintf(qbmp, "query%i_image", k);
				g_config->WriteString(qn, NULL);
				g_config->WriteString(qv, NULL);
				g_config->WriteString(qm, NULL);
				g_config->WriteString(qmet, NULL);
				g_config->WriteString(qbmp, NULL);
			}
		}
	}
	g_config->WriteInt("query_num", i);
	g_config->Flush();
}

HTREEITEM g_treedrag_lastSel;

HWND onTreeViewSelectChange(HWND hwnd)
{
	if (!g_table) openDb();
	bgQuery_Stop();

	int par = mediaLibrary.GetSelectedTreeItem();

	// defaults
	m_query_mode = par;
	m_query = L"";

	m_query_metafile = "";
	m_query_mode = 0;
	if (par == m_query_tree)	// set up default media view
	{
		m_query_metafile = "default.vmd";
	}
	else
	{
		QueryList::iterator iter;
		iter = m_query_list.find(par);
		if (iter != m_query_list.end())
		{
			m_query = iter->second->query;
			m_query_mode = iter->second->mode;
			m_query_metafile = iter->second->metafn;
		}
		else
		{
			m_query_metafile = "default.vmd";
		}
	}

	return updateCurrentView(hwnd);
}

void add_pledit_to_library()
{
	SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITEPLAYLIST);
	wchar_t *m3udir = (wchar_t *) SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW);
	wchar_t filename[MAX_PATH] = {0};
	PathCombineW(filename, m3udir, L"winamp.m3u8");

	PLCallBackW plCB;
	if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(filename, &plCB))
	{
		mediaLibrary.AddToMediaLibrary(filename);
	}
}

void add_to_library(HWND wndparent)
{
	ScanFolderBrowser browser;
	browser.SetBckScanChecked(g_config->ReadInt("addinbg", 0));

	if (browser.Browse(wndparent))
	{
		wchar_t path[MAX_PATH];
		g_config->WriteInt("addinbg", browser.GetBckScanChecked());
		SHGetPathFromIDListW(browser.GetPIDL(), path);
		int guess = -1, meta = -1, rec = 1;
		autoscan_add_directory(path, &guess, &meta, &rec, 0);
		if (guess == -1) guess = g_config->ReadInt("guessmode", 0);
		if (meta == -1)	meta = g_config->ReadInt("usemetadata", 1);
		if (g_config->ReadInt("addinbg", 0))
		{
			Scan_ScanFolderBackground(path, guess, meta, rec);	// add our dir to scan :)
		}
		else
		{
			Scan_ScanFolder(wndparent, path, guess, meta, rec);

			int par = mediaLibrary.GetSelectedTreeItem();
			mediaLibrary.SelectTreeItem(par - 1);
			mediaLibrary.SelectTreeItem(par);
			if (m_curview_hwnd) SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
		}
	}
}

void nukeLibrary(HWND hwndDlg)
{
	wchar_t titleStr[32];
	if (MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_REMOVE_ALL_ITEMS_IN_LIBRARY),
		WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION,titleStr,32),
		MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		wchar_t *last_query = NULL;
		EnterCriticalSection(&g_db_cs);
		if (m_media_scanner)
		{
			const wchar_t *lq = NDE_Scanner_GetLastQuery(m_media_scanner);
			if (lq)	last_query = _wcsdup(lq);
			NDE_Table_DestroyScanner(g_table, m_media_scanner);
		}
		LeaveCriticalSection(&g_db_cs);
		bgQuery_Stop();
		Scan_Cancel();

		int count = 0;
		wchar_t **filenames = 0;
		nde_scanner_t clearedScanner = NDE_Table_CreateScanner(g_table);
		if (clearedScanner)
		{
			filenames = new wchar_t * [(count = NDE_Scanner_GetRecordsCount(clearedScanner))];
			int i = 0;
			for (NDE_Scanner_First(clearedScanner); !NDE_Scanner_EOF(clearedScanner); NDE_Scanner_Next(clearedScanner))
			{
				nde_field_t fileName = NDE_Scanner_GetFieldByID(clearedScanner, MAINTABLE_ID_FILENAME);
				if (fileName)
				{
					filenames[i] = NDE_StringField_GetString(fileName);
					ndestring_retain(filenames[i]);
					i++;
				}
			}
		}
		NDE_Table_DestroyScanner(g_table, clearedScanner);

		closeDb();

		char tmp[MAX_PATH];
		StringCchPrintfA(tmp, MAX_PATH, "%s\\main.dat", g_tableDir);
		DeleteFileA(tmp);
		StringCchPrintfA(tmp, MAX_PATH, "%s\\main.idx", g_tableDir);
		DeleteFileA(tmp);

		openDb();
		EnterCriticalSection(&g_db_cs);
		if (m_media_scanner)
		{
			m_media_scanner = NDE_Table_CreateScanner(g_table);
			if (last_query != NULL)
			{
				NDE_Scanner_Query(m_media_scanner, last_query);
				free(last_query);
			}
		}
		LeaveCriticalSection(&g_db_cs);
		DumpArtCache();
		if (IsWindow(m_curview_hwnd))
			SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view

		// Wasabi event callback when the media library is cleared
		WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_CLEARED, (size_t)filenames, count);
		if (filenames) delete[] filenames;

		// trigger a refresh of the current view
		PostMessage(lMedia.hwndLibraryParent, WM_USER + 30, 0, 0);
	}
}

extern int main_sendto_mode;

extern HMENU main_sendto_hmenu;


wchar_t *itemrecordWTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	// TODO we can put more tags in here
	itemRecordW *t = (itemRecordW *)p;
	bool copy=false;
	wchar_t buf[128];
	wchar_t *value = NULL;

	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename"))	value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			wsprintfW(buf, L"%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			if (t->tracks > 0)
				wsprintfW(buf, L"%02d/%02d", t->track, t->tracks);
			else
				wsprintfW(buf, L"%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"disc"))
	{
		if (t->disc > 0)
		{
			if (t->discs > 0)
				wsprintfW(buf, L"%d/%d", t->disc, t->discs);
			else
				wsprintfW(buf, L"%d", t->disc);

			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating"))
	{
		if (t->rating > 0)
		{
			wsprintfW(buf, L"%d", t->rating);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"playcount"))
	{
		if (t->playcount > 0)
		{
			wsprintfW(buf, L"%d", t->playcount);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bitrate"))
	{
		if (t->bitrate > 0)
		{
			wsprintfW(buf, L"%d", t->bitrate);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bpm"))
	{
		if (t->bpm > 0)
		{
			wsprintfW(buf, L"%d", t->bpm);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"albumartist"))	value = t->albumartist;
	else if (!_wcsicmp(tag, L"publisher"))	value = t->publisher;
	else if (!_wcsicmp(tag, L"composer"))	value = t->composer;
	else if (!_wcsicmp(tag, L"replaygain_album_gain"))	value = t->replaygain_album_gain;
	else if (!_wcsicmp(tag, L"replaygain_track_gain"))	value = t->replaygain_track_gain;
	else if (!_wcsicmp(tag, L"GracenoteFileID"))	
	{
		value = getRecordExtendedItem_fast(t, extended_fields.GracenoteFileID);
	}
	else if (!_wcsicmp(tag, L"GracenoteExtData"))
	{
		value = getRecordExtendedItem_fast(t, extended_fields.GracenoteExtData);
	}
	else
		return 0;

	if (!value)
		return reinterpret_cast<wchar_t *>(-1);
	else
	{
		if (copy || value == buf)
			return ndestring_wcsdup(value);
		else
		{
			ndestring_retain(value);
			return value;
		}
	}
}

static bool TagNameToFieldID(const wchar_t *tag, int *id)
{
	if (!_wcsicmp(tag, L"artist"))	*id = MAINTABLE_ID_ARTIST;
	else if (!_wcsicmp(tag, L"album"))	*id = MAINTABLE_ID_ALBUM;
	else if (!_wcsicmp(tag, L"filename")) *id = MAINTABLE_ID_FILENAME;
	else if (!_wcsicmp(tag, L"title"))	*id = MAINTABLE_ID_TITLE;
	else if (!_wcsicmp(tag, L"year")) *id = MAINTABLE_ID_YEAR;
	else if (!_wcsicmp(tag, L"genre"))	*id = MAINTABLE_ID_GENRE;
	else if (!_wcsicmp(tag, L"comment")) *id = MAINTABLE_ID_COMMENT;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track")) *id = MAINTABLE_ID_TRACKNB;
	else if (!_wcsicmp(tag, L"rating")) *id = MAINTABLE_ID_RATING;
	else if (!_wcsicmp(tag, L"playcount")) *id = MAINTABLE_ID_PLAYCOUNT;
	else if (!_wcsicmp(tag, L"bitrate")) *id = MAINTABLE_ID_BITRATE;
	else if (!_wcsicmp(tag, L"disc")) *id = MAINTABLE_ID_DISC;
	else if (!_wcsicmp(tag, L"bpm")) *id = MAINTABLE_ID_BPM;
	else if (!_wcsicmp(tag, L"albumartist")) *id = MAINTABLE_ID_ALBUMARTIST;
	else if (!_wcsicmp(tag, L"publisher")) *id = MAINTABLE_ID_PUBLISHER;
	else if (!_wcsicmp(tag, L"composer")) *id = MAINTABLE_ID_COMPOSER;
	else if (!_wcsicmp(tag, L"replaygain_album_gain")) *id = MAINTABLE_ID_ALBUMGAIN;
	else if (!_wcsicmp(tag, L"replaygain_track_gain")) *id = MAINTABLE_ID_TRACKGAIN;
	else if (!_wcsicmp(tag, L"GracenoteFileID")) *id = MAINTABLE_ID_GRACENOTEFILEID;
	else if (!_wcsicmp(tag, L"GracenoteExtData")) *id = MAINTABLE_ID_GRACENOTEEXTDATA;
	//else if (!_wcsicmp(tag, L"lossless")) *id = MAINTABLE_ID_LOSSLESS;
	else return false;
	return true;
}

wchar_t *fieldTagFunc(wchar_t * tag, void * p)	//return 0 if not found
{
	nde_scanner_t s = (nde_scanner_t)p;
	int id = -1;

	if (!TagNameToFieldID(tag, &id))
		return 0;

	if (id >= 0)
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
		if (f)
			switch (id)
		{
			case MAINTABLE_ID_YEAR:
				{
					wchar_t buf[32];
					int l = NDE_IntegerField_GetValue(f);
					if (l < 0) return reinterpret_cast<wchar_t *>(-1);
					wsprintfW(buf, L"%04d", l);
					return ndestring_wcsdup(buf);
				}
			case MAINTABLE_ID_TRACKNB:
				{
					wchar_t buf[32];
					int l = NDE_IntegerField_GetValue(f);
					if (l < 0) return reinterpret_cast<wchar_t *>(-1);
					int tracks = db_getFieldInt(s, MAINTABLE_ID_TRACKS, -1);
					if (tracks > 0)
						wsprintfW(buf, L"%02d/%02d", l, tracks);
					else
						wsprintfW(buf, L"%02d", l);
					return ndestring_wcsdup(buf);
				}
			case MAINTABLE_ID_DISC:
				{
					wchar_t buf[32];
					int l = NDE_IntegerField_GetValue(f);
					if (l < 0) return reinterpret_cast<wchar_t *>(-1);
					int discs = db_getFieldInt(s, MAINTABLE_ID_DISCS, -1);
					if (discs > 0)
						wsprintfW(buf, L"%d/%d", l, discs);
					else
						wsprintfW(buf, L"%d", l);
					return ndestring_wcsdup(buf);
				}
			case MAINTABLE_ID_PLAYCOUNT:
				asked_for_playcount = 1;
				// fall through :)
			case MAINTABLE_ID_BPM:
			case MAINTABLE_ID_RATING:
			case MAINTABLE_ID_BITRATE:
				{
					wchar_t buf[32];
					int l = NDE_IntegerField_GetValue(f);
					if (l < 0) return reinterpret_cast<wchar_t *>(-1);
					wsprintfW(buf, L"%d", l);
					return ndestring_wcsdup(buf);
				}
			default:
				{
					wchar_t *p = NDE_StringField_GetString(f);
					if (!p || !*p) return reinterpret_cast<wchar_t *>(-1);;

					ndestring_retain(p);
					return p;
				}
		}

	}
	return 0;
}

void ndeTagFuncFree(wchar_t * tag, void * p)
{
	ndestring_release(tag);
}

void RetypeFilename(nde_table_t table);
void RefreshFileSizeAndDateAddedTable(nde_table_t table);
void ReindexTable(nde_table_t table);

static void CreateFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumnW(table, MAINTABLE_ID_FILENAME, L"filename", FIELD_FILENAME);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_TITLE, L"title", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_ARTIST, L"artist", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_ALBUM, L"album", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_YEAR, L"year", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_GENRE, L"genre", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_COMMENT, L"comment", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_TRACKNB, L"trackno", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_LENGTH, L"length", FIELD_LENGTH);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_TYPE, L"type", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_LASTUPDTIME, L"lastupd", FIELD_DATETIME);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_LASTPLAY, L"lastplay", FIELD_DATETIME);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_RATING, L"rating", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_GRACENOTE_ID, L"tuid2", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_PLAYCOUNT, L"playcount", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_FILETIME, L"filetime", FIELD_DATETIME);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_FILESIZE, L"filesize", FIELD_INT64);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_BITRATE, L"bitrate", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_DISC, L"disc", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_ALBUMARTIST, L"albumartist", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_ALBUMGAIN, L"replaygain_album_gain", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_TRACKGAIN, L"replaygain_track_gain", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_PUBLISHER, L"publisher", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_COMPOSER, L"composer", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_BPM, L"bpm", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_DISCS, L"discs", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_TRACKS, L"tracks", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_ISPODCAST, L"ispodcast", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_PODCASTCHANNEL, L"podcastchannel", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_PODCASTPUBDATE, L"podcastpubdate", FIELD_DATETIME);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_GRACENOTEFILEID, L"GracenoteFileID", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_GRACENOTEEXTDATA, L"GracenoteExtData", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_LOSSLESS, L"lossless", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_CATEGORY, L"category", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_CODEC, L"codec", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_DIRECTOR, L"director", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_PRODUCER, L"producer", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_WIDTH, L"width", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_HEIGHT, L"height", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_MIMETYPE, L"mimetype", FIELD_STRING);
	NDE_Table_NewColumnW(table, MAINTABLE_ID_DATEADDED, L"dateadded", FIELD_DATETIME);

	NDE_Table_PostColumns(table);
	NDE_Table_AddIndexByIDW(table, MAINTABLE_ID_FILENAME, L"filename");
}

int openDb()
{
	// TODO: fix!! this is a Double-Checked Lock Pattern and can have strange results
	// in weird conditions because g_table is assigned before fully initialized
	if (g_table) return 0;

	EnterCriticalSection(&g_db_cs);

	// benski> i know this looks redundant, but we might have sat and blocked at the above Critical Section for a while
	if (g_table) 
	{
		LeaveCriticalSection(&g_db_cs);
		return 0; 
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

	const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
	wchar_t tableName[MAX_PATH], indexName[MAX_PATH];
	PathCombineW(indexName, inidir, L"Plugins\\ml");
	PathCombineW(tableName, indexName, L"main.dat");
	PathAppendW(indexName, L"main.idx");
	g_table = NDE_Database_OpenTable(g_db, tableName, indexName, NDE_OPEN_ALWAYS, NDE_CACHE);
	if (g_table)
	{
		((Table *)g_table)->EnableRowCache(); // TODO: don't use c++ NDE API
		CreateFields(g_table);
		RetypeFilename(g_table);
		#define REINDEX_KEY "reindex_561"
		if (!g_config->ReadInt(REINDEX_KEY, 0)) // do we need to reindex?
		{
			ReindexTable(g_table);
		}
		g_config->WriteInt(REINDEX_KEY, 1);

		#undef REINDEX_KEY
		#define REINDEX_KEY "reindex_564"
		if (g_config->ReadInt(REINDEX_KEY, 0)!=2) // do we need to update the filesizes and date added?
		{
			RefreshFileSizeAndDateAddedTable(g_table);
		}
		g_config->WriteInt(REINDEX_KEY, 2);

		PostMessage(lMedia.hwndWinampParent, WM_WA_IPC, NDE_Table_GetRecordsCount(g_table), IPC_STATS_LIBRARY_ITEMCNT);
	}

	LeaveCriticalSection(&g_db_cs);
	return (g_table != 0);
}

// TODO make sure we're only ever saving if there was an actual change!!
void closeDb()
{
	if (g_db)
	{
		__try
		{
			if (g_table)
			{
				if (g_table_dirty)
				{
					NDE_Table_Sync(g_table);
				}
				NDE_Database_CloseTable(g_db, g_table);
			}
			NDE_DestroyDatabase(g_db);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	g_db = NULL;
	g_table = NULL;
}

const char *WINAMP_INI;
api_service *serviceManager = 0;
WNDPROC ml_oldWndProc;

int init()
{
	g_table = NULL;
	g_db = NULL;
	g_bgscan_last_rescan = time(NULL);

	char *dir = (char*)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
	if ((INT_PTR)(dir) < 65536)	return 1;
	PathCombine(g_path, dir, "Plugins");
	CreateDirectory(g_path, NULL);

	WINAMP_INI = (const char*)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIFILE);

	char configName[MAX_PATH];
	PathCombine(configName, g_path, "gen_ml.ini");
	g_config = new C_Config(configName);

	g_bgrescan_int = g_config->ReadInt("bgrescan_int", g_bgrescan_int);
	g_bgrescan_do = g_config->ReadInt("bgrescan_do", g_bgrescan_do);
	g_bgrescan_force = g_config->ReadInt("bgrescan_startup", 0); // temporarily used
	g_guessifany = g_config->ReadInt("guessifany", g_guessifany);
	g_viewnotplay = g_config->ReadInt("viewnotplay", g_viewnotplay);

	// this allows an override of the delay from making a change in the search box
	// in the views to when the search will be run - sometimes needs tweaking for
	// either older machines or some of the less powerful 'portable' type machines
	g_querydelay = g_config->ReadInt("querydelay", g_querydelay);
	if(g_querydelay < 1 || g_querydelay > 5000) g_querydelay = 250;

	PathCombine(g_tableDir, g_path, "ml");
	PathCombine(g_viewsDir, g_tableDir, "views");

	if (!g_config->ReadInt("artdbmig", 0))
	{
		MigrateArtCache();
		g_config->WriteInt("artdbmig", 1);
	}

	wa_oldWndProc = (WNDPROC) SetWindowLongPtrW(lMedia.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)wa_newWndProc);

	if (g_bgrescan_force || g_config->ReadInt("dbloadatstart", 1))
	{
		openDb();
	}

	HMENU wa_plcontext_menu = GetSubMenu((HMENU)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, -1, IPC_GET_HMENU), 2);
	if (wa_plcontext_menu)
		wa_playlists_cmdmenu = GetSubMenu(wa_plcontext_menu, 4);

	wa_play_menu = GetSubMenu((HMENU)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_HMENU), 2);

	// lets extend menu that called on button press
	IPC_GET_ML_HMENU = (int)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&"LibraryGetHmenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
	g_context_menus = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);

	HMENU rate_hmenu = GetSubMenu(GetSubMenu(g_context_menus,1),4);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

	HMENU context_menu = (HMENU) SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_ML_HMENU);

	if (context_menu)
	{
		HMENU btnMenu = GetSubMenu(context_menu, 0);
		if (btnMenu)
		{
			MENUITEMINFOW mii = {sizeof(MENUITEMINFOW)};

			mii.fMask = MIIM_FTYPE;
			mii.fType = MFT_SEPARATOR;
			mii.fState = MFS_ENABLED;
			InsertMenuItemW(btnMenu, 0, TRUE, &mii);

			mii.fMask = MIIM_TYPE | MIIM_ID;
			mii.fType = MFT_STRING;

			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_NEW_SMART_VIEW);
			mii.cch = (unsigned int) wcslen(mii.dwTypeData);
			mii.wID = IDM_DOSHITMENU_ADDNEWVIEW;
			InsertMenuItemW(btnMenu, 1, TRUE, &mii);

			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_RESCAN_WATCH_FOLDERS);
			mii.cch = (unsigned int) wcslen(mii.dwTypeData);
			mii.wID = IDM_RESCANFOLDERSNOW;
			InsertMenuItemW(btnMenu, 0, TRUE, &mii);

			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ADD_PLEDIT_TO_LOCAL_MEDIA);
			mii.cch = (unsigned int) wcslen(mii.dwTypeData);
			mii.wID = IDM_ADD_PLEDIT;
			InsertMenuItemW(btnMenu, 0, TRUE, &mii);

			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ADD_MEDIA_TO_LIBRARY);
			mii.cch = (unsigned int) wcslen(mii.dwTypeData);
			mii.wID = IDM_ADD_DIRS;
			InsertMenuItemW(btnMenu, 0, TRUE, &mii);

			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_REMOVE_MISSING_FILES_FROM_ML);
			mii.cch = (unsigned int) wcslen(mii.dwTypeData);
			mii.wID = IDM_REMOVE_UNUSED_FILES;
			InsertMenuItemW(btnMenu, 0, TRUE, &mii);
		}
	}

	IPC_GET_CLOUD_HINST = (INT)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_GET_CLOUD_ACTIVE = (INT)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudActive", IPC_REGISTER_WINAMP_IPCMESSAGE);

	ml_oldWndProc = (WNDPROC) SetWindowLongPtrW(lMedia.hwndLibraryParent, GWLP_WNDPROC, (LONG_PTR)ml_newWndProc);

	HookPlaylistEditor();
	hDragNDropCursor = LoadCursor(GetModuleHandle("gen_ml.dll"), MAKEINTRESOURCE(ML_IDC_DRAGDROP));

	// rescan timer
	SetTimer(lMedia.hwndLibraryParent, 200, 1000, NULL);
	return 0;
}

void quit()
{
	/*char a[256];
	DWORD now = GetTickCount();*/
	UnhookPlaylistEditor();
	/*sprintf(a, "\r\nUnhookPlaylistEditor - %dms\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
	Scan_Kill();
	/*sprintf(a, "Scan_Kill - %dms\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
	closeDb();
	/*sprintf(a, "closeDb - %dms\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
	delete(g_view_metaconf);
	g_view_metaconf = 0;
	/*sprintf(a, "g_view_metaconf - %dms\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
	delete g_config;
	g_config = NULL;
	/*sprintf(a, "g_config - %dms\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
	KillArtThread();
	/*sprintf(a, "KillArtThread - %dms\r\n\r\n", (GetTickCount() - now));
	OutputDebugString(a);*/
}

int OnLocalMediaItemClick(int action, int item, HWND parent)
{
	switch (action)
	{
	case ML_ACTION_ENTER:
	case ML_ACTION_DBLCLICK:
		{
			queryItem *qitem = m_query_list[item];
			if (qitem != NULL)
			{
				char configDir[MAX_PATH];
				PathCombineA(configDir, g_viewsDir, qitem->metafn);
				C_Config viewconf(configDir);
				main_playQuery(&viewconf, qitem->query, ((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^(!!g_config->ReadInt("enqueuedef", 0))));
			}
		}
		return 1;
	}
	return 0;
}
int OnLocalMediaClick(int action, HWND parent)
{
	switch (action)
	{
	case ML_ACTION_ENTER:
	case ML_ACTION_DBLCLICK:
		return 1;
	}
	return 0;
}