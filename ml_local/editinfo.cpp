#include "main.h"
#include "../Agave/Language/api_language.h"
#include "../nu/ListView.h"
#include "resource.h"
#include <time.h>
#define MAKESAFE(x) ((x)?(x):L"")

extern W_ListView resultlist;
extern itemRecordListW itemCache;
volatile int no_lv_update = 0;
//////////////// info editor fun


// must be one of OUR item records (since we free it)
static void UpdateItemRecordFromDB(itemRecordW *song)
{
	// look in the database for the updated song info
	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, song->filename))
	{
		// now we can actually update the itemCache itemRecordW from the value in the db
		itemRecordW item;
		itemRecordListW obj = {&item, 0, 1};
		ScannerRefToObjCacheNFNW(s, &obj, false);

		item.filename = song->filename;
		song->filename = NULL; // set to NULL so freeRecord doesn't delete the filename string

		freeRecord(song); // delete old item
		*song = item; // replace with our new (BETTER :) item
	}
	NDE_Table_DestroyScanner(g_table, s);
	LeaveCriticalSection(&g_db_cs);
}

//physically update metadata in a given file
int updateFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *data)
{
	extendedFileInfoStructW efis = {
	                                 filename,
	                                 metadata,
	                                 data ? data : L"",
	                                 data ? wcslen(data) : 0,
	                               };
	return SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
}

static int m_upd_nb, m_stopped, m_upd_nb_all, m_upd_nb_cur;
static nde_scanner_t m_scanner;

// sets part and parts to -1 or 0 on fail/missing (e.g. parts will be -1 on "1", but 0 on "1/")
void ParseIntSlashInt(wchar_t *string, int *part, int *parts)
{
	*part = -1;
	*parts = -1;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (*string && *string != '/')
		{
			string++;
		}
		if (*string == '/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}

// TODO: benski> can this copy-and-paste code be factored?
static INT_PTR CALLBACK updateFiles_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetWindowTextW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UPDATING_FILES));
			SetWindowLong(GetDlgItem(hwndDlg, IDC_STATUS), GWL_STYLE, (GetWindowLong(GetDlgItem(hwndDlg, IDC_STATUS), GWL_STYLE)&~SS_CENTER) | SS_LEFTNOWORDWRAP);
			SetTimer(hwndDlg, 0x123, 30, NULL);
			m_upd_nb = 0;
			EnterCriticalSection(&g_db_cs);
			m_scanner = NDE_Table_CreateScanner(g_table);
			m_stopped = 0;
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, m_upd_nb_all));
			m_upd_nb_cur = 0;
			break;
		case WM_TIMER:
			if (wParam == 0x123 && !m_stopped)
			{
				unsigned int start_t = GetTickCount();
again:
				{
					int l = resultlist.GetCount();

					while (m_upd_nb < l && !resultlist.GetSelected(m_upd_nb))
						m_upd_nb++;

					if (m_upd_nb >= l)
					{
						//done
						EndDialog(hwndDlg, 1);
						break;
					}

					int i = m_upd_nb++;

					itemRecordW *song = itemCache.Items + i;
					HWND hwndParent = GetParent(hwndDlg);

					wchar_t stattmp[512];
					wchar_t *p = scanstr_backW(song->filename, L"\\", song->filename - 1) + 1;
					wsprintfW(stattmp, WASABI_API_LNGSTRINGW(IDS_UPDATING_X), p);
					SetDlgItemTextW(hwndDlg, IDC_STATUS, stattmp);

					SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, m_upd_nb_cur, 0);
					m_upd_nb_cur++;

					int updtagz = !!IsDlgButtonChecked(hwndParent, IDC_CHECK1);
					if (!NDE_Scanner_LocateNDEFilename(m_scanner, MAINTABLE_ID_FILENAME, FIRST_RECORD, song->filename))
					{
						break;
					}

					NDE_Scanner_Edit(m_scanner);

#define CHECK_AND_COPY(IDCHECK, ID, field, item) if (IsDlgButtonChecked(hwndParent, IDCHECK))	{\
						wchar_t blah[2048]; GetDlgItemTextW(hwndParent, ID, blah, 2048);\
						if (wcscmp(MAKESAFE(item), blah))\
						{ if (blah[0]) db_setFieldStringW(m_scanner, field, blah);\
						  else db_removeField(m_scanner, field);\
						  ndestring_release(item);\
						  item = ndestring_wcsdup(blah);}}

					CHECK_AND_COPY(IDC_CHECK_ARTIST,      IDC_EDIT_ARTIST,      MAINTABLE_ID_ARTIST,      song->artist);
					CHECK_AND_COPY(IDC_CHECK_TITLE,       IDC_EDIT_TITLE,       MAINTABLE_ID_TITLE,       song->title);
					CHECK_AND_COPY(IDC_CHECK_ALBUM,       IDC_EDIT_ALBUM,       MAINTABLE_ID_ALBUM,       song->album);
					CHECK_AND_COPY(IDC_CHECK_COMMENT,     IDC_EDIT_COMMENT,     MAINTABLE_ID_COMMENT,     song->comment);
					CHECK_AND_COPY(IDC_CHECK_GENRE,       IDC_EDIT_GENRE,       MAINTABLE_ID_GENRE,       song->genre);
					CHECK_AND_COPY(IDC_CHECK_ALBUMARTIST, IDC_EDIT_ALBUMARTIST, MAINTABLE_ID_ALBUMARTIST, song->albumartist);
					CHECK_AND_COPY(IDC_CHECK_PUBLISHER,   IDC_EDIT_PUBLISHER,   MAINTABLE_ID_PUBLISHER,   song->publisher);
					CHECK_AND_COPY(IDC_CHECK_COMPOSER,    IDC_EDIT_COMPOSER,    MAINTABLE_ID_COMPOSER,    song->composer);
					CHECK_AND_COPY(IDC_CHECK_CATEGORY,    IDC_EDIT_CATEGORY,    MAINTABLE_ID_CATEGORY,    song->category);

#define CHECK_AND_COPY_EXTENDED(IDCHECK, ID, field, name) if (IsDlgButtonChecked(hwndParent, IDCHECK))	{\
						wchar_t blah[2048]; GetDlgItemTextW(hwndParent, ID, blah, 2048);\
						wchar_t *oldData = getRecordExtendedItem_fast(song, name);\
						if (wcscmp(MAKESAFE(oldData), blah))\
						{ if (blah[0]) db_setFieldStringW(m_scanner, field, blah);\
						  else db_removeField(m_scanner, field);\
						  wchar_t *nde_blah = ndestring_wcsdup(blah);\
						  setRecordExtendedItem(song, name, nde_blah);\
						  ndestring_release(nde_blah);}}

					CHECK_AND_COPY_EXTENDED(IDC_CHECK_DIRECTOR,    IDC_EDIT_DIRECTOR,    MAINTABLE_ID_DIRECTOR,    extended_fields.director);
					CHECK_AND_COPY_EXTENDED(IDC_CHECK_PRODUCER,    IDC_EDIT_PRODUCER,    MAINTABLE_ID_PRODUCER,    extended_fields.producer);

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_TRACK))
					{
						wchar_t blah[64];
						GetDlgItemTextW(hwndParent, IDC_EDIT_TRACK, blah, 64);
						int track, tracks;
						ParseIntSlashInt(blah, &track, &tracks);
						if (tracks <= 0) tracks = -1;
						if (track <= 0) track = -1;

						if (song->track != track || song->tracks != tracks)
						{
							if (track>0) db_setFieldInt(m_scanner, MAINTABLE_ID_TRACKNB, track);
							else db_removeField(m_scanner, MAINTABLE_ID_TRACKNB);
							if (tracks>0) db_setFieldInt(m_scanner, MAINTABLE_ID_TRACKS, tracks);
							else db_removeField(m_scanner, MAINTABLE_ID_TRACKS);
							song->track = track;
							song->tracks = tracks;
						}
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_DISC))
					{
						wchar_t blah[64];
						GetDlgItemTextW(hwndParent, IDC_EDIT_DISC, blah, 64);
						int disc, discs;
						ParseIntSlashInt(blah, &disc, &discs);
						if (discs <= 0) discs = -1;
						if (disc <= 0) disc = -1;

						if (song->disc != disc || song->discs != discs)
						{
							if (disc>0) db_setFieldInt(m_scanner, MAINTABLE_ID_DISC, disc);
							else db_removeField(m_scanner, MAINTABLE_ID_DISC);
							if (discs>0) db_setFieldInt(m_scanner, MAINTABLE_ID_DISCS, discs);
							else db_removeField(m_scanner, MAINTABLE_ID_DISCS);
							song->disc = disc;
							song->discs = discs;
						}
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_YEAR))
					{
						char blah[64];
						GetDlgItemText(hwndParent, IDC_EDIT_YEAR, blah, 64);
						int n = atoi(blah);
						if (n <= 0) n = -1;
						if (song->year != n)
						{
							if (n > 0) db_setFieldInt(m_scanner, MAINTABLE_ID_YEAR, n);
							else db_removeField(m_scanner, MAINTABLE_ID_YEAR);
							song->year = n;
						}
					}

					if (updtagz)
					{
						m_stopped = 1;
retry:
						if (updateFileInfo(song->filename, L"title", song->title)) // if this returns 0, then this format doesnt even support extended
						{
							updateFileInfo(song->filename, L"artist", song->artist);
							updateFileInfo(song->filename, L"album", song->album);
							updateFileInfo(song->filename, L"comment", song->comment);
							updateFileInfo(song->filename, L"genre", song->genre);

							wchar_t buf[32];
							if (song->track > 0)
							{
								if (song->tracks > 0)
									wsprintfW(buf, L"%d/%d", song->track, song->tracks);
								else
									wsprintfW(buf, L"%d", song->track);
							}
							else buf[0] = 0;
							updateFileInfo(song->filename, L"track", buf);

							if (song->year > 0) wsprintfW(buf, L"%d", song->year);
							else buf[0] = 0;
							updateFileInfo(song->filename, L"year", buf);

							if (song->disc > 0)
							{
								if (song->discs > 0)
									wsprintfW(buf, L"%d/%d", song->disc, song->discs);
								else
									wsprintfW(buf, L"%d", song->disc);
							}
							else
								buf[0] = 0;
							updateFileInfo(song->filename, L"disc", buf);

							if (song->rating > 0)
							{
								wsprintfW(buf, L"%d", song->rating);
							}
							else
								buf[0] = 0;
							updateFileInfo(song->filename, L"rating", buf);

							updateFileInfo(song->filename, L"albumartist", song->albumartist);
							updateFileInfo(song->filename, L"publisher", song->publisher);
							updateFileInfo(song->filename, L"composer", song->composer);
							updateFileInfo(song->filename, L"category", song->category);
							updateFileInfo(song->filename, L"director", getRecordExtendedItem_fast(song, extended_fields.director));
							updateFileInfo(song->filename, L"producer", getRecordExtendedItem_fast(song, extended_fields.producer));

							if (!SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO))
							{
								wchar_t tmp[1024];
								wsprintfW(tmp, WASABI_API_LNGSTRINGW(IDS_ERROR_UPDATING_FILE), song->filename);
								int ret = MessageBoxW(hwndDlg, tmp, WASABI_API_LNGSTRINGW(IDS_INFO_UPDATING_ERROR), MB_RETRYCANCEL);
								if (ret == IDRETRY) goto retry;
								if (ret == IDCANCEL)
								{
									EndDialog(hwndDlg, 0);
									break;
								}
							}
						}
						m_stopped = 0;
					}

					db_setFieldInt(m_scanner, MAINTABLE_ID_LASTUPDTIME, (int)time(NULL));
					db_setFieldInt(m_scanner, MAINTABLE_ID_FILETIME, (int)time(NULL));
					NDE_Scanner_Post(m_scanner);

					WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_UPDATED, (size_t)song->filename, 0);
				}
				if (GetTickCount() - start_t < 30) goto again;
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg, 0);
			}
			break;
		case WM_DESTROY:
			NDE_Table_DestroyScanner(g_table, m_scanner);
			NDE_Table_Sync(g_table);
			g_table_dirty = 0;
			LeaveCriticalSection(&g_db_cs);
			break;
	}
	return FALSE;
}

static int checkEditInfoClick(HWND hwndDlg, POINT p, int item, int check)
{
	RECT r;
	GetWindowRect(GetDlgItem(hwndDlg, item), &r);
	ScreenToClient(hwndDlg, (LPPOINT)&r);
	ScreenToClient(hwndDlg, (LPPOINT)&r.right);
	if (PtInRect(&r, p) && !IsDlgButtonChecked(hwndDlg, check))
	{
		CheckDlgButton(hwndDlg, check, TRUE);
		EnableWindow(GetDlgItem(hwndDlg, item), TRUE);
		PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, item), (LPARAM)TRUE);
		return 1;
	}
	return 0;
}

void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;
		if (sr) hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd) hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p) hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);
		if (hm)
		{
			MONITORINFOEX mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoA(hm, &mi))
			{
				if (!full) *r = mi.rcWork;
				else *r = mi.rcMonitor;
				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

// TODO: benski> can this copy-and-paste code be factored?
static INT_PTR CALLBACK editInfo_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			wchar_t *last_artist = NULL, *last_title = NULL, *last_album = NULL, *last_genre = NULL, *last_comment = NULL, *last_albumartist = NULL;
			wchar_t *last_composer = NULL, *last_publisher = NULL;
			const wchar_t *last_category = NULL; wchar_t *last_director = NULL; wchar_t *last_producer = NULL;
			int last_year = -1, last_track = -1, last_disc = -1, last_discs = -1, last_tracks=-1;
			int disable_artist = 0, disable_title = 0, disable_album = 0, disable_genre = 0, disable_year = 0, disable_track = 0, disable_comment = 0, disable_disc = 0, disable_albumartist = 0;
			int disable_composer = 0, disable_publisher = 0, disable_discs = 0, disable_tracks=0, disable_category=0;
			int disable_director = 0, disable_producer = 0;
			int l = resultlist.GetCount();
			int nb = 0;

			if (g_config->ReadInt("upd_tagz", 1)) CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);

			for (int i = 0;i < l;i++)
			{
				if (!resultlist.GetSelected(i)) continue;

				itemRecordW *song = itemCache.Items + i;

#define SAVE_LAST_STR(last, check, disable) if (!disable && check && check[0])	{ if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }
#define SAVE_LAST_INT(last, check, disable) if (!disable && check > 0) { if (last == -1) last = check; else if (last != check) disable = 1; }

				SAVE_LAST_STR(last_artist, song->artist, disable_artist);
				SAVE_LAST_STR(last_title, song->title, disable_title);
				SAVE_LAST_STR(last_album, song->album, disable_album);
				SAVE_LAST_STR(last_comment, song->comment, disable_comment);
				SAVE_LAST_STR(last_genre, song->genre, disable_genre);

				SAVE_LAST_INT(last_year, song->year, disable_year);
				SAVE_LAST_INT(last_track, song->track, disable_track);
				SAVE_LAST_INT(last_tracks, song->tracks, disable_tracks);
				SAVE_LAST_INT(last_disc, song->disc, disable_disc);
				SAVE_LAST_INT(last_discs, song->discs, disable_discs);

				SAVE_LAST_STR(last_albumartist, song->albumartist, disable_albumartist);
				SAVE_LAST_STR(last_composer, song->composer, disable_composer);
				SAVE_LAST_STR(last_publisher, song->publisher, disable_publisher);
				SAVE_LAST_STR(last_category, song->category, disable_category);

#define SAVE_LAST_STR_EXTENDED(last, name, disable) if (!disable) { wchar_t *check = getRecordExtendedItem_fast(song, name); if (check && check[0]) { if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }};

				SAVE_LAST_STR_EXTENDED(last_director , extended_fields.director, disable_director);
				SAVE_LAST_STR_EXTENDED(last_producer , extended_fields.producer, disable_producer);

				nb++;
			}

			if (!disable_artist && last_artist) SetDlgItemTextW(hwndDlg, IDC_EDIT_ARTIST, last_artist);
			if (!disable_title && last_title) SetDlgItemTextW(hwndDlg, IDC_EDIT_TITLE, last_title);
			if (!disable_album && last_album) SetDlgItemTextW(hwndDlg, IDC_EDIT_ALBUM, last_album);
			if (!disable_comment && last_comment) SetDlgItemTextW(hwndDlg, IDC_EDIT_COMMENT, last_comment);
			if (!disable_albumartist && last_albumartist) SetDlgItemTextW(hwndDlg, IDC_EDIT_ALBUMARTIST, last_albumartist);
			if (!disable_composer && last_composer) SetDlgItemTextW(hwndDlg, IDC_EDIT_COMPOSER, last_composer);
			if (!disable_publisher && last_publisher) SetDlgItemTextW(hwndDlg, IDC_EDIT_PUBLISHER, last_publisher);
			if (!disable_genre && last_genre) SetDlgItemTextW(hwndDlg, IDC_EDIT_GENRE, last_genre);
			if (!disable_category && last_category) SetDlgItemTextW(hwndDlg, IDC_EDIT_CATEGORY, last_category);
			if (!disable_director && last_director) SetDlgItemTextW(hwndDlg, IDC_EDIT_DIRECTOR, last_director);
			if (!disable_producer && last_producer) SetDlgItemTextW(hwndDlg, IDC_EDIT_PRODUCER, last_producer);
			if (!disable_year && last_year > 0)
			{
				wchar_t tmp[64];
				wsprintfW(tmp, L"%d", last_year);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_YEAR, tmp);
			}
			if (!disable_track && last_track > 0 && !disable_tracks)
			{
				wchar_t tmp[64];
				if (!disable_tracks && last_tracks > 0)
					wsprintfW(tmp, L"%d/%d", last_track, last_tracks);
				else
					wsprintfW(tmp, L"%d", last_track);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_TRACK, tmp); 
			}
			if (!disable_disc && last_disc > 0
			    && !disable_discs)
			{
				wchar_t tmp[64];
				if (!disable_discs && last_discs > 0)
					wsprintfW(tmp, L"%d/%d", last_disc, last_discs);
				else
					wsprintfW(tmp, L"%d", last_disc);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_DISC, tmp);
			}
			wchar_t tmp[512];
			wsprintfW(tmp, WASABI_API_LNGSTRINGW((nb==1?IDS_X_ITEM_SELECTED:IDS_X_ITEMS_SELECTED)), nb);
			SetDlgItemTextW(hwndDlg, IDC_HEADER, tmp);
			m_upd_nb_all = nb;

			// show edit info window and restore last position as applicable
			POINT pt = {g_config->ReadInt("edit_x", -1), g_config->ReadInt("edit_y", -1)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CHECK_ARTIST: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_ARTIST), IsDlgButtonChecked(hwndDlg, IDC_CHECK_ARTIST)); break;
				case IDC_CHECK_TITLE: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TITLE), IsDlgButtonChecked(hwndDlg, IDC_CHECK_TITLE)); break;
				case IDC_CHECK_ALBUM: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_ALBUM), IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALBUM)); break;
				case IDC_CHECK_COMMENT: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_COMMENT), IsDlgButtonChecked(hwndDlg, IDC_CHECK_COMMENT)); break;
				case IDC_CHECK_ALBUMARTIST: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_ALBUMARTIST), IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALBUMARTIST)); break;
				case IDC_CHECK_COMPOSER: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_COMPOSER), IsDlgButtonChecked(hwndDlg, IDC_CHECK_COMPOSER)); break;
				case IDC_CHECK_PUBLISHER: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_PUBLISHER), IsDlgButtonChecked(hwndDlg, IDC_CHECK_PUBLISHER)); break;
				case IDC_CHECK_TRACK: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TRACK), IsDlgButtonChecked(hwndDlg, IDC_CHECK_TRACK)); break;
				case IDC_CHECK_DISC: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DISC), IsDlgButtonChecked(hwndDlg, IDC_CHECK_DISC)); break;
				case IDC_CHECK_GENRE: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_GENRE), IsDlgButtonChecked(hwndDlg, IDC_CHECK_GENRE)); break;
				case IDC_CHECK_YEAR: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_YEAR), IsDlgButtonChecked(hwndDlg, IDC_CHECK_YEAR)); break;
				case IDC_CHECK_CATEGORY: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_CATEGORY), IsDlgButtonChecked(hwndDlg, IDC_CHECK_CATEGORY)); break;
				case IDC_CHECK_DIRECTOR: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DIRECTOR), IsDlgButtonChecked(hwndDlg, IDC_CHECK_DIRECTOR)); break;
				case IDC_CHECK_PRODUCER: EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_PRODUCER), IsDlgButtonChecked(hwndDlg, IDC_CHECK_PRODUCER)); break;
				case IDOK:
				{
					int updtagz = !!IsDlgButtonChecked(hwndDlg, IDC_CHECK1);
					g_config->WriteInt("upd_tagz", updtagz);

					int ret = WASABI_API_DIALOGBOXW(IDD_ADDSTUFF, hwndDlg, updateFiles_dialogProc);

					ListView_RedrawItems(resultlist.getwnd(), 0, resultlist.GetCount() - 1);

					if (!ret) break;
				}
				case IDCANCEL:
					RECT edit_rect = {0};
					GetWindowRect(hwndDlg, &edit_rect);
					g_config->WriteInt("edit_x", edit_rect.left);
					g_config->WriteInt("edit_y", edit_rect.top);
					EndDialog(hwndDlg, 0);
					break;
			}
			break;
		case WM_LBUTTONDOWN:
		{
			POINTS p = MAKEPOINTS(lParam);
			POINT p2 = {p.x, p.y};
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ARTIST, IDC_CHECK_ARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TITLE, IDC_CHECK_TITLE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUM, IDC_CHECK_ALBUM)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_COMMENT, IDC_CHECK_COMMENT)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUMARTIST, IDC_CHECK_ALBUMARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_COMPOSER, IDC_CHECK_COMPOSER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PUBLISHER, IDC_CHECK_PUBLISHER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TRACK, IDC_CHECK_TRACK)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_GENRE, IDC_CHECK_GENRE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_YEAR, IDC_CHECK_YEAR)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_DISC, IDC_CHECK_DISC)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_CATEGORY, IDC_CHECK_CATEGORY)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_DIRECTOR, IDC_CHECK_DIRECTOR)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PRODUCER, IDC_CHECK_PRODUCER)) break;
		}
		break;
	}
	return FALSE;
}

void editInfo(HWND hwndParent)
{
	no_lv_update++;
	bgQuery_Stop();
	WASABI_API_DIALOGBOXW(IDD_EDIT_INFO, hwndParent, editInfo_dialogProc);

	EatKeyboard();
	no_lv_update--;
}

#define REFRESHCB_NUMITEMS (WM_USER)
#define REFRESHCB_ITERATE (WM_USER+1)
#define REFRESHCB_FINISH (WM_USER+2)
static bool refreshKill;
static int RefreshMetadataThread(HANDLE handle, void *_callback, intptr_t id)
{
	HWND callback = (HWND)_callback;
	int l = resultlist.GetCount(), i;
	PostMessage(callback, REFRESHCB_NUMITEMS, 0, resultlist.GetSelectedCount());
	for (i = 0;i < l;i++)
	{
		if (refreshKill)
			break;
		if (!resultlist.GetSelected(i)) continue;
		itemRecordW *song = itemCache.Items + i;
		if (!song->filename || !song->filename[0]) continue;

		EnterCriticalSection(&g_db_cs);
		int guess = -1, meta = -1, rec = 1;
		autoscan_add_directory(song->filename, &guess, &meta, &rec, 1); // use this folder's guess/meta options
		if (guess == -1) guess = g_config->ReadInt("guessmode", 0);
		if (meta == -1)	meta = g_config->ReadInt("usemetadata", 1);
		addFileToDb(song->filename, TRUE, meta, guess, 0, 0, true);

		UpdateItemRecordFromDB(song);

		PostMessage(callback, REFRESHCB_ITERATE, 0, 0);
		LeaveCriticalSection(&g_db_cs);
	}
	PostMessage(callback, REFRESHCB_FINISH, 0, 0);
	return 0;
}

static void WriteStatus(HWND hwndDlg, int dlgId, int numFiles, int totalFiles)
{
	wchar_t temp[1024];
	if (numFiles + 1 > totalFiles)
		WASABI_API_LNGSTRINGW_BUF(IDS_FINISHED, temp, 1024);
	else
		StringCchPrintfW(temp, 1024, WASABI_API_LNGSTRINGW(IDS_REFRESH_MESSAGE), numFiles + 1, totalFiles);
	SetDlgItemTextW(hwndDlg, dlgId, temp);
}

static int numFiles, totalFiles;
static INT_PTR WINAPI RefreshDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			numFiles = 0;
			totalFiles = 0;
			refreshKill = false;
			WASABI_API_THREADPOOL->RunFunction(0, RefreshMetadataThread, (void *)hwndDlg, 0, api_threadpool::FLAG_LONG_EXECUTION);
		}
		break;
		case WM_DESTROY:
		break;
		case REFRESHCB_NUMITEMS:
			totalFiles = lParam;
			WriteStatus(hwndDlg, IDC_REFRESHMETADATA_STATUS, numFiles, totalFiles);
			break;
		case REFRESHCB_ITERATE:
			numFiles++;
			WriteStatus(hwndDlg, IDC_REFRESHMETADATA_STATUS, numFiles, totalFiles);
			break;
		case REFRESHCB_FINISH:
			EndDialog(hwndDlg, 0);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					refreshKill = true;
					break;
			}
			break;
	}
	return 0;
}

void RefreshMetadata(HWND parent)
{
	bgQuery_Stop();

	WASABI_API_DIALOGBOXW(IDD_REFRESH_METADATA, parent, RefreshDlgProc);
	ListView_RedrawItems(resultlist.getwnd(), 0, resultlist.GetCount() - 1);
}

// when the rowcache is enabled, the filename pointers should be identical
void UpdateRating_RowCache(const wchar_t *filename, int new_rating)
{
	itemRecordW *song = NULL;

	int itemcount = itemCache.Size;
	for (int i = 0; i < itemcount; i++)
	{
		song = itemCache.Items + i;
		if (!song->filename || !song->filename[0]) continue;
		if (song->filename == filename || !nde_wcsicmp_fn(song->filename,filename))
		{
			song->rating = new_rating;
			ListView_RedrawItems(resultlist.getwnd(), i, i);
			break;  
		}
	}

}

void UpdateLocalResultsCache(const wchar_t *filename) // perhaps just a itemRecordW parm
{
	itemRecordW *song = NULL;
	
	// Search thru the itemCache looking for the file that was changed
	int itemcount = itemCache.Size;
	for (int i = 0; i < itemcount; i++)
	{
		// TODO: linear search, yuck, look at this later
		song = itemCache.Items + i;
		if (!song->filename || !song->filename[0]) continue;
		if (nde_wcsicmp_fn(song->filename,filename) == 0) 
		{
			UpdateItemRecordFromDB(song);
			SetTimer(GetParent(resultlist.getwnd()), UPDATE_RESULT_LIST_TIMER_ID, 500, 0);
			break;  
		}
	}

}

void fileInfoDialogs(HWND hwndParent)
{
	no_lv_update++; // this might block other attempts from going thru but that's OK
	bgQuery_Stop();
	int l = resultlist.GetCount(), i;
	int needref = 0;
	for (i = 0;i < l;i++)
	{
		if (!resultlist.GetSelected(i)) continue;
		itemRecordW *song = itemCache.Items + i;
		if (!song->filename || !song->filename[0]) continue;

		infoBoxParamW p;
		p.filename = song->filename;
		p.parent = hwndParent;
		if (SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW)) break;

		needref = 1;
		UpdateItemRecordFromDB(song);
	}
	EatKeyboard();
	if (needref) ListView_RedrawItems(resultlist.getwnd(), 0, resultlist.GetCount() - 1);
	no_lv_update--;
}

