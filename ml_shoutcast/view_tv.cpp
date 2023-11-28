#include "main.h"
#include "api.h"
#include "../nu/listview.h"
#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include <bfc/parse/paramparser.h>
#include <vector>
#include <strsafe.h>

// TODO: make this a virtual listview
W_ListView stationList;

static std::vector<int> ids; // STL for now

static api_db_command *rowCommand = 0;
static api_db_reader *rowReader = 0;
static int lastItem = -1;
static const wchar_t ROW_QUERY[] =L"SELECT name, genre, nowplaying, listeners, bitrate FROM tv_temp WHERE id = ?";
extern int stationListSkin;
static ChildWndResizeItem category_rlist[] =
    {
        {IDC_STATION_LIST, ResizeBottom | ResizeRight},
        {IDC_SEARCH, ResizeRight},
        {IDC_STATUS_TEXT, ResizeRight | DockToBottom },
        {IDC_UPDATE, DockToBottom},
        {IDC_PLAY, DockToBottom},
        {IDC_ENQUEUE, DockToBottom},
    };

static int ConfigInt(const wchar_t *value, int def)
{
	return GetPrivateProfileInt(L"tv_view", value, def, ini_file);

}

static void WriteInt(const wchar_t *value, int _value)
{
	wchar_t temp[64];
	StringCchPrintf(temp, 64, L"%d", _value);
	WritePrivateProfileString(L"tv_view", value, temp, ini_file);
}


static int sortField = 3;
HWND tvHWND = 0;

static wchar_t oldSearch[512] = L"";
static int oldField = 0;
static const wchar_t *sorts[] = { L"ORDER BY name", L"ORDER BY genre", L"ORDER BY nowplaying", L"ORDER BY listeners DESC", L"ORDER BY bitrate DESC", L"ORDER BY name DESC", L"ORDER BY genre DESC", L"ORDER BY nowplaying DESC", L"ORDER BY listeners", L"ORDER BY bitrate" };

static void SyncTable()
{
	if (rowReader) rowCommand->ReleaseReader(rowReader);
	rowReader=0;
	if (rowCommand) database->ReleaseCommand(rowCommand);
	rowCommand=0;

	database->ExecuteNonQuery(L"DROP TABLE tv_temp", -1, DB_UTF16);
	database->ExecuteNonQuery(L"CREATE TEMP TABLE tv_temp AS SELECT * FROM tv", -1, DB_UTF16);
	database->ExecuteNonQuery(L"CREATE INDEX IF NOT EXISTS tv_temp_id ON tv_temp (id)", -1, DB_UTF16);
	database->ExecuteNonQuery(L"CREATE INDEX IF NOT EXISTS tv_temp_listeners ON tv_temp (listeners)", -1, DB_UTF16);
				
	rowCommand = database->CreateCommand(ROW_QUERY, -1, DB_UTF16);
}

static void SearchList(HWND hwndDlg, const wchar_t *search, bool force = false)
{
	W_ListView stationList(GetDlgItem(hwndDlg, IDC_STATION_LIST));

	if (!force && !lstrcmpiW(oldSearch, search) && oldField == sortField)
		return ;
	lstrcpynW(oldSearch, search, 512);
	oldField = sortField;

	wchar_t whereClause[4096] = L"";
	wchar_t temp[1024];
	ParamParser keywords(search, L" ");


	for (int x = 0;x < keywords.getNumItems();x++)
	{
		StringCchPrintfW(temp, 1024, L"%s (name like '%%%s%%' or genre like '%%%s%%' or nowplaying like '%%%s%%')", (x == 0) ? L"WHERE" : L"AND", keywords.enumItem(x), keywords.enumItem(x), keywords.enumItem(x));
		StringCchCatW(whereClause, 4096, temp);
	}

	int rating = SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_GET_PARENTAL_RATING);
	if (rating < 8) // TODO: build this as "rating IN" based on the bits that are set
	{
		if (whereClause[0])
			StringCchCatW(whereClause, 4096, L" AND rating <> 'NC17'");
		else
			StringCchCatW(whereClause, 4096, L" WHERE rating <> 'NC17'");
	}

	wchar_t fullQuery[8192];
	StringCchPrintf(fullQuery, 8192, L"SELECT id FROM tv_temp %s %s", whereClause, sorts[sortField]);

	api_db_command *command = database->CreateCommand(fullQuery, -1, DB_UTF16);
	if (!command)
		return ;

	api_db_reader *reader = command->CreateReader();
	int item = stationList.GetNextSelected( -1);
	int oldid = item == CB_ERR ? 0 : ids[item];
	item = CB_ERR;
	ids.clear();
	while (reader->Read() == SQLITE_ROW)
	{
		int id;
		reader->GetInt(0, &id);
		ids.push_back(id);
		if (oldid == id)
			item = ids.size() - 1;
	}
	command->ReleaseReader(reader);
	database->ReleaseCommand(command);

	StringCchPrintf(fullQuery, 8192, L"SELECT COUNT(*), SUM(listeners) FROM tv_temp %s", whereClause);
	command = database->CreateCommand(fullQuery, -1, DB_UTF16);
	if (!command)
		return ;

	reader = command->CreateReader();
	reader->Read();

	int count, listeners;
	reader->GetInt(0, &count);
	reader->GetInt(1, &listeners);

	wchar_t stationStatus[1024];
	StringCchPrintf(stationStatus, 1024, L"%d stations, %d listeners", count, listeners);
	SetDlgItemText(hwndDlg, IDC_STATUS_TEXT, stationStatus);

	command->ReleaseReader(reader);
	database->ReleaseCommand(command);

	lastItem = -1;

	stationList.SetVirtualCount(ids.size());
	stationList.RefreshAll();
	if (item != CB_ERR)
	{
		stationList.SetSelected(item);
		stationList.ScrollTo(item);
	}
	else
	{
		stationList.UnselectAll();
	}
}

void DisplayChange(HWND hwndDlg);

INT_PTR WINAPI TVProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL a = dialogSkinner.Handle(hwndDlg, msg, wParam, lParam);
	if (a)
		return a;

	switch (msg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;
	case WM_INITDIALOG:
		{
			tvHWND = hwndDlg;
			OpenDatabase();
			SyncTable();

			rowCommand = database->CreateCommand(ROW_QUERY, -1, DB_UTF16);

			stationList.setwnd(GetDlgItem(hwndDlg, IDC_STATION_LIST));

			stationList.AddCol(L"Name", ConfigInt(L"name_width", 200));
			stationList.AddCol(L"Genre", ConfigInt(L"genre_width", 100));
			stationList.AddCol(L"Now Playing", ConfigInt(L"nowplaying_width", 100));
			stationList.AddCol(L"Viewers", ConfigInt(L"listeners_width", 50));
			stationList.AddCol(L"Bitrate (kbps)", ConfigInt(L"bitrate_width", 75));

			childSizer.Init(hwndDlg, category_rlist, sizeof(category_rlist) / sizeof(category_rlist[0]));

			SearchList(hwndDlg, L"", true);
			stationListSkin = mediaLibrary.SkinList(stationList.getwnd());
			DisplayChange(hwndDlg);

		}
		break;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
			childSizer.Resize(hwndDlg, category_rlist, sizeof(category_rlist) / sizeof(category_rlist[0]));
		break;
	case WM_DISPLAYCHANGE:
		{
			DisplayChange(hwndDlg);
		}
		break;
	case WM_PAINT:
		{
			int tab[] = { IDC_STATION_LIST | DCW_SUNKENBORDER, IDC_SEARCH | DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
		}
		return 0;
	case WM_ERASEBKGND:
		return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
	case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lParam;

			switch (l->idFrom)
			{
			case IDC_STATION_LIST:
				switch (l->code)
				{
				case LVN_GETDISPINFO:
					{
						NMLVDISPINFO *lpdi = (NMLVDISPINFO*) l;


						if (lpdi->item.mask & LVIF_TEXT)
						{
							int item = lpdi->item.iItem;
							lpdi->item.pszText[0] = 0;
							if (!rowReader || lastItem != item)
							{
								if (rowReader) rowCommand->ReleaseReader(rowReader);
								rowCommand->BindInt(1, ids[item]);
								rowReader = rowCommand->CreateReader();
								rowReader->Read();
							}
							rowReader->GetStringCpy(lpdi->item.iSubItem, (void *)lpdi->item.pszText, lpdi->item.cchTextMax*sizeof(wchar_t), 0, DB_UTF16);
							//lpdi->item.iSubItem

						}
					}
					break;
				case LVN_COLUMNCLICK:
					{
						NMLISTVIEW *columnClick = (NMLISTVIEW *)l;
						// TODO: sort based on column click
						if (sortField == columnClick->iSubItem)
							sortField = columnClick->iSubItem + 5;
						else
							sortField = columnClick->iSubItem;

						wchar_t text[512];
						GetDlgItemText(hwndDlg, IDC_SEARCH, text, 511);
						text[511] = 0;
						SearchList(hwndDlg, text);
					}
					break;

				case NM_DBLCLK:
					{
						int item = stationList.GetNextSelected( -1);
						int id = ids[item];
						wchar_t temp[1024];
						wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-tvstation.pls?id=%d", id);
						mediaLibrary.PlayStream(temp, false);

					}
					break;
				}
				break;

			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SEARCH:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				wchar_t text[512];
				GetDlgItemText(hwndDlg, IDC_SEARCH, text, 511);
				text[511] = 0;
				SearchList(hwndDlg, text);
			}
			break;
		case IDC_UPDATE:
			{
				SetDlgItemText(hwndDlg, IDC_UPDATE, L"Updating");
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				DownloadTV();
			}
			break;
		case IDC_PLAY:
			{
				int item = stationList.GetNextSelected( -1);
				int id = ids[item];
				wchar_t temp[1024];
				wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-tvstation.pls?id=%d", id);
				mediaLibrary.PlayStream(temp);
			}
			break;
		case IDC_ENQUEUE:
			{
				int item = stationList.GetNextSelected( -1);
				int id = ids[item];
				wchar_t temp[1024];
				wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-tvstation.pls?id=%d", id);
				mediaLibrary.EnqueueStream(temp);
			}
			break;
		}
		break;
	case WM_DESTROY:
		WriteInt(L"name_width", stationList.GetColumnWidth(0));
		WriteInt(L"genre_width", stationList.GetColumnWidth(1));
		WriteInt(L"nowplaying_width", stationList.GetColumnWidth(2));
		WriteInt(L"listeners_width", stationList.GetColumnWidth(3));
		WriteInt(L"bitrate_width", stationList.GetColumnWidth(4));

		mediaLibrary.UnskinList(stationListSkin);
		tvHWND = 0;
		if (rowReader)
			rowCommand->ReleaseReader(rowReader);
		rowReader = 0;
		if (rowCommand)
			database->ReleaseCommand(rowCommand);
		rowCommand = 0;
		break;
	case WM_USER + 10:
		{
			SyncTable();
			wchar_t text[512];
			GetDlgItemText(hwndDlg, IDC_SEARCH, text, 511);
			text[511] = 0;
			SearchList(hwndDlg, text, true);
			SetDlgItemText(hwndDlg, IDC_UPDATE, L"Update");
			EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), TRUE);
		}
		break;
	case WM_USER + 11:
		{
			wchar_t text[512];
			wsprintfW(text, L"Downloading: %u bytes (%u%%)", wParam, MulDiv(wParam, 100, lParam));
			SetDlgItemText(hwndDlg, IDC_STATUS_TEXT, text);
		}
		break;
	}
	return 0;
}
