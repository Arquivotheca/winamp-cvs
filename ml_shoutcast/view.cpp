#include "main.h"
#include "api.h"
#include "../nu/listview.h"
#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "../ml_local/gaystring.h"
#include <bfc/parse/paramparser.h>
#include "../nu/Vector.h"
#include <assert.h>
#include <string>
#include <strsafe.h>

struct Station
{
	int id;
	wchar_t *name;
	wchar_t *genre;
	wchar_t *nowplaying;
	int bitrate;
	int listeners;
	wchar_t *mimetype;
};
static Vector<Station, 32, 2> stations; // STL for now

nde_table_t radio_table = 0;
nde_scanner_t radio_scanner = 0;
static int lastItem = -1;

static W_ListView stationList;
int stationListSkin;
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
	return GetPrivateProfileInt(L"radio_view", value, def, ini_file);
}

static void WriteInt(const wchar_t *value, int _value)
{
	wchar_t temp[64];
	StringCchPrintf(temp, 64, L"%d", _value);
	WritePrivateProfileString(L"radio_view", value, temp, ini_file);
}

static int sortField = 3;
HWND radioHWND = 0;

static wchar_t oldSearch[512] = L"";
static int oldField = 0;

static void SyncTable()
{
	if (radio_table)
	{
		if (radio_scanner)
			NDE_Table_DestroyScanner(radio_table, radio_scanner);
		radio_scanner=0;
		NDE_Database_CloseTable(db, radio_table);
	}
	radio_table = 0;
	wchar_t dbFilename[MAX_PATH], dbIndex[MAX_PATH];
	StringCchPrintfW(dbFilename, MAX_PATH, L"%S\\Plugins\\ml\\shoutcast.dat", mediaLibrary.GetIniDirectory());
	StringCchPrintfW(dbIndex, MAX_PATH, L"%S\\Plugins\\ml\\shoutcast.idx", mediaLibrary.GetIniDirectory());

	radio_table = CreateRadioTable(dbFilename, dbIndex);
	radio_scanner = NDE_Table_CreateScanner(radio_table);
}

void queryStrEscape(const wchar_t *p, GayStringW &str) 
{
  if (!p || !*p) return;
  size_t l = wcslen(p);
  wchar_t *escaped = (wchar_t *)malloc((l*3+1)*sizeof(wchar_t));
  wchar_t *d = escaped;
  while (*p) {
    if (*p == L'%') { *d++ = L'%'; *d++ = L'%'; }
    else if (*p == L'\"') { *d++ = L'%'; *d++ = L'2'; *d++ = L'2'; }
    else if (*p == L'\'') { *d++ = L'%'; *d++ = L'2'; *d++ = L'7'; }
    else if (*p == L'[') { *d++ = L'%'; *d++ = L'5'; *d++ = L'B'; }
    else if (*p == L']') { *d++ = L'%'; *d++ = L'5'; *d++ = L'D'; }
		else if (*p == L'(') { *d++ = L'%'; *d++ = L'2'; *d++ = L'8'; }
		else if (*p == L')') { *d++ = L'%'; *d++ = L'2'; *d++ = L'9'; }
		else if (*p == L'#') { *d++ = L'%'; *d++ = L'2'; *d++ = L'3'; }
    else *d++ = *p;
    p++;
  }
  *d = 0;
  str.Set(escaped);
  free(escaped);
}
// out can never be bigger than in+1
static void parsequicksearch(wchar_t *out, wchar_t *in) // parses a list into a list of terms that we are searching for
{
	int inquotes = 0, neednull = 0;
	while (*in)
	{
		wchar_t c = *in++;
		if (c != ' ' && c != '\t' && c != '\"')
		{
			neednull = 1;
			*out++ = c;
		}
		else if (c == '\"')
		{
			inquotes = !inquotes;
			if (!inquotes)
			{
				*out++ = 0;
				neednull = 0;
			}
		}
		else
		{
			if (inquotes) *out++ = c;
			else if (neednull)
			{
				*out++ = 0;
				neednull = 0;
			}
		}
	}
	*out++ = 0;
	*out++ = 0;
}

void makeQueryStringFromText(GayStringW *query, wchar_t *text, int nf)
{
	if (!text[0])
		return;
	int ispar = 0;
	if (query->Get()[0])
	{
		ispar = 1;
		query->Append(L"&(");
	}
	if (!_wcsnicmp(text, L"query:", 6)) query->Append(text + 6); // copy the query as is
	else if (text[0] == L'?') query->Append(text + 1);
	else // this is ubergay. no wait it isn't anymore. it rocks now due to the GayString
	{
		int isAny = 0;
		if (*text == L'*' && text[1] == L' ')
		{
			isAny = 1;
			text += 2;
		}
		wchar_t tmpbuf[512 + 32];
		parsequicksearch(tmpbuf, text);

		int x;
		wchar_t *fields[] =
		{
			L"name",
				L"genre",
				L"nowplaying",
		};
		wchar_t *p = tmpbuf;
		while (*p)
		{
			size_t lenp = wcslen(p);

			if (p == tmpbuf) query->Append(L"(");
			else if (isAny) query->Append(L")|(");
			else query->Append(L")&(");
			if (p[0] == L'<' && p[wcslen(p) - 1] == L'>' && wcslen(p) > 2)
			{
				wchar_t *op = p;
				while (*op)
				{
					if (*op == L'\'') *op = L'\"';
					op++;
				}

				p[lenp - 1] = 0; // remove >
				query->Append(p + 1);
			}
			else
			{
				for (x = 0; x < (int)min(sizeof(fields) / sizeof(fields[0]), nf); x ++)
				{
					wchar_t *field = fields[x];
					if (x) query->Append(L"|");
					query->Append(field);
					query->Append(L" HAS \"");
					GayStringW escaped;
					queryStrEscape(p, escaped);
					query->Append(escaped.Get());
					query->Append(L"\"");
				}
			}
			p += lenp + 1;
		}
		query->Append(L")");
	}
	if (ispar) query->Append(L")");
}

void ClearStations()
{
	for (size_t i=0;i!=stations.size();i++)
	{
		ndestring_release(stations[i].name);
		ndestring_release(stations[i].genre);
		ndestring_release(stations[i].nowplaying);
		ndestring_release(stations[i].mimetype);
	}
	stations.clear();
}

static void SearchList(HWND hwndDlg, const wchar_t *search, bool force = false)
{
	W_ListView stationList(GetDlgItem(hwndDlg, IDC_STATION_LIST));

	if (!force && !lstrcmpiW(oldSearch, search) && oldField == sortField)
		return ;
	lstrcpynW(oldSearch, search, 512);
	GayStringW parsed_query;
	makeQueryStringFromText(&parsed_query, (wchar_t *)search, 8);
	const wchar_t *query = parsed_query.Get();
	{
	//Nullsoft::Utility::LockGuard radio_lock(radio_guard);
		NDE_Scanner_Query(radio_scanner, *query?query:0);
	
	
	int killswitch = 0;
	ClearStations();
	NDE_Scanner_First(radio_scanner, &killswitch);
	int r;
	do
	{
		Station newStation;
		nde_field_t f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_ID);
		if (f)
		{
			newStation.id = NDE_IntegerField_GetValue(f);

		f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_NAME);
		if (f)
		{
			newStation.name = NDE_StringField_GetString(f);
			ndestring_retain(newStation.name);
		}

			f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_GENRE);
		if (f)
		{
			newStation.genre = NDE_StringField_GetString(f);
			ndestring_retain(newStation.genre);
		}

		f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_NOWPLAYING);
		if (f)
		{
			newStation.nowplaying = NDE_StringField_GetString(f);
			ndestring_retain(newStation.nowplaying);
		}

				f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_BITRATE);
		if (f)
			newStation.bitrate = NDE_IntegerField_GetValue(f);

		f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_LISTENERS);
		if (f)
			newStation.listeners = NDE_IntegerField_GetValue(f);

	
		f = NDE_Scanner_GetFieldByID(radio_scanner, RADIOTABLE_MIMETYPE);
		if (f)
		{
			newStation.mimetype = NDE_StringField_GetString(f);
			ndestring_retain(newStation.mimetype);
		}

		stations.push_back(newStation);
		}
		r = NDE_Scanner_Next(radio_scanner, &killswitch);
	}
	while (r && !NDE_Scanner_EOF(radio_scanner));
	}
	stationList.SetVirtualCount(stations.size());
	stationList.RefreshAll();
}

void DisplayChange(HWND hwndDlg)
{
	W_ListView stationList(GetDlgItem(hwndDlg, IDC_STATION_LIST));

	ListView_SetTextColor(stationList.getwnd(), dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(stationList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(stationList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	stationList.SetFont(dialogSkinner.GetFont());
}

INT_PTR WINAPI StationProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR a = dialogSkinner.Handle(hwndDlg, msg, wParam, lParam);
	if (a)
		return a;

	switch (msg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;
	case WM_INITDIALOG:
		{
			radioHWND = hwndDlg;
			OpenDatabase();
			SyncTable();

			stationList.setwnd(GetDlgItem(hwndDlg, IDC_STATION_LIST));
			stationListSkin = mediaLibrary.SkinList(stationList.getwnd());

			stationList.AddCol(L"Name", ConfigInt(L"name_width", 200));
			stationList.AddCol(L"Genre", ConfigInt(L"genre_width", 100));
			stationList.AddCol(L"Now Playing", ConfigInt(L"nowplaying_width", 100));
			stationList.AddCol(L"Listeners", ConfigInt(L"listeners_width", 50));
			stationList.AddCol(L"Bitrate (kbps)", ConfigInt(L"bitrate_width", 75));
			stationList.AddCol(L"MIME Type", ConfigInt(L"mime_width", 75));

			stationList.SetVirtualCount(0);
			SearchList(hwndDlg, L"", true);
			
			DisplayChange(hwndDlg);

			childSizer.Init(hwndDlg, category_rlist, sizeof(category_rlist) / sizeof(category_rlist[0]));
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

							Station &station = stations[item];
							switch(lpdi->item.iSubItem)
							{
							case 0:
								StringCchCopyW(lpdi->item.pszText, lpdi->item.cchTextMax, station.name);
								break;
							case 1:
								StringCchCopyW(lpdi->item.pszText, lpdi->item.cchTextMax, station.genre);
								break;
							case 2:
								StringCchCopyW(lpdi->item.pszText, lpdi->item.cchTextMax, station.nowplaying);
								break;
							case 3:
								StringCchPrintfW(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", station.listeners);
								break;
							case 4:
								StringCchPrintfW(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", station.bitrate);
								break;
							case 5:
								StringCchCopyW(lpdi->item.pszText, lpdi->item.cchTextMax, station.mimetype);
								break;
							}
						}
					}
					break;

				case LVN_COLUMNCLICK:
					{
						NMLISTVIEW *columnClick = (NMLISTVIEW *)l;
						// TODO: sort based on column click
						if (sortField == columnClick->iSubItem)
							sortField = columnClick->iSubItem + 6;
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
						int id = stations[item].id;
						wchar_t temp[1024];
						wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-station.pls?id=%d", id);
						mediaLibrary.PlayStream(temp, false);

					}
					break;
				}
				break;

			}
		}
		break;
	case WM_TIMER:
		if (wParam = 700)
		{
			KillTimer(hwndDlg, 700);
			wchar_t text[512];
			GetDlgItemText(hwndDlg, IDC_SEARCH, text, 511);
			text[511] = 0;
			SearchList(hwndDlg, text);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SEARCH:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				KillTimer(hwndDlg, 700);
				SetTimer(hwndDlg, 700, 50, 0);
				//wchar_t text[512];
				//GetDlgItemText(hwndDlg, IDC_SEARCH, text, 511);
				//text[511] = 0;
				//SearchList(hwndDlg, text);
			}
			break;
		case IDC_UPDATE:
			{
				NDE_Table_DestroyScanner(radio_table, radio_scanner);
			radio_scanner=0;
			NDE_Database_CloseTable(db, radio_table);
			radio_table = 0;
				SetDlgItemText(hwndDlg, IDC_UPDATE, L"Updating");
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				Download();
			}
			break;
		case IDC_PLAY:
			{
				int item = stationList.GetNextSelected( -1);
				int id = stations[item].id;
				wchar_t temp[1024];
				wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-station.pls?id=%d", id);
				mediaLibrary.PlayStream(temp);
			}
			break;
		case IDC_ENQUEUE:
			{
				int item = stationList.GetNextSelected( -1);
				int id = stations[item].id;
				wchar_t temp[1024];
				wsprintfW(temp, L"http://shoutcast.com/sbin/tunein-station.pls?id=%d", id);
				mediaLibrary.EnqueueStream(temp);
			}
			break;

		}
		break;
	case WM_DESTROY:
		{
			WriteInt(L"name_width", stationList.GetColumnWidth(0));
			WriteInt(L"genre_width", stationList.GetColumnWidth(1));
			WriteInt(L"nowplaying_width", stationList.GetColumnWidth(2));
			WriteInt(L"listeners_width", stationList.GetColumnWidth(3));
			WriteInt(L"bitrate_width", stationList.GetColumnWidth(4));
			WriteInt(L"mime_width", stationList.GetColumnWidth(5));

			mediaLibrary.UnskinList(stationListSkin);
			
			radioHWND = 0;
			NDE_Table_DestroyScanner(radio_table, radio_scanner);
			radio_scanner=0;
			NDE_Database_CloseTable(db, radio_table);
			radio_table = 0;
		}

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
