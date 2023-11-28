#include "main.h"
#include "Playlist.h"

#include "resource.h"
#include "../nu/listview.h"
#include <shlobj.h>
#include "CurrentPlaylist.h"
#include "../nu/AutoCharFn.h"
#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc.h"
#include "SendTo.h"
#include "PlaylistView.h"
#include "PlaylistDirectoryCallback.h"
#include "api.h"
#include "../gen_ml/menufucker.h"
#include "../nu/menushortcuts.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../ml_local/api_mldb.h"
#include "../ml_pmp/pmp.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxstring.h"
#include "../playlist/plstring.h"
#include <strsafe.h>


using namespace Nullsoft::Utility;

Playlist currentPlaylist;
wchar_t currentPlaylistFilename[MAX_PATH];
wchar_t currentPlaylistTitle[400];
W_ListView playlist_list;
static GUID playlist_guid = INVALID_GUID;
int IPC_LIBRARY_SENDTOMENU;
int we_are_drag_and_dropping = 0;
static void AutoSizePlaylistColumns();
static SendToMenu sendTo;

typedef enum 
{
	SCROLLDIR_NONE = 0,
	SCROLLDIR_UP = 1,
	SCROLLDIR_DOWN = -1,
} SCROLLDIR;

#define SCROLLTIMER_ID		100

static INT scrollDelay = 0;
static INT scrollTimerElapse = 0;
static int scrollDirection = SCROLLDIR_NONE;


static ChildWndResizeItem playlistwnd_rlist[] =
{
	{IDC_PLAYLIST_EDITOR, 0x0011},
	{IDC_PLAY, 0x0101},
	{IDC_BURN, 0x0101},
	{IDC_ADD, 0x0101},
	{IDC_SEL, 0x0101},
	{IDC_REM, 0x0101},
	{IDC_MISC, 0x0101},
	{IDC_LIST, 0x0101},
	{IDC_PLSTATUS, 0x0111},
	{IDC_SAVE_PL, 0x0101},
};


void UpdatePlaylistTime(HWND hwndDlg);
static bool opened = false, changed = false;
HWND activeHWND = 0, saveHWND = 0;

void Changed(bool _changed = true)
{
	changed = _changed;
	EnableWindow(saveHWND, changed);
}

void SyncPlaylist()
{
	if (opened)
	{
		playlist_list.SetVirtualCount((INT)currentPlaylist.GetNumItems());
		playlist_list.RefreshAll();
		UpdatePlaylistTime(GetParent(playlist_list.getwnd()));
	}
}

void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu)
{
	HACCEL szAccel[24];
	INT c = WASABI_API_APP->app_getAccelerators(hwndDlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
	AppendMenuShortcuts(menu, szAccel, c, MSF_REPLACE);
}

void TagEditor(HWND hwnd)
{
	int x, v;
	wchar_t fn[1024];
	wchar_t ft[1024];
	v = playlist_list.GetCount();
	for (x = 0; x < v; x ++)
	{
		if (playlist_list.GetSelected(x))
		{
			currentPlaylist.GetItem(x, fn, 1024);
			infoBoxParamW p;
			p.filename = fn;
			p.parent = hwnd;
			if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW)) break;

			int length = -1;
			mediaLibrary.GetFileInfo(fn, ft, 1024, &length);

			currentPlaylist.SetItemTitle(x, ft);
			currentPlaylist.SetItemLengthMilliseconds(x, length*1000);

			playlist_list.RefreshItem(x);
			Changed();
		}
	}
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
}

void myOpenURL(HWND hwnd, wchar_t *loc);
void Playlist_GenerateHtmlPlaylist(void)
{
	FILE *fp = 0;
	wchar_t filename[MAX_PATH], tp[MAX_PATH];
	if (!GetTempPathW(MAX_PATH,tp)) StringCchCopyW(tp, MAX_PATH, L".");
	if (GetTempFileNameW(tp, L"WHT", 0, filename)){
		DeleteFileW(filename);
		StringCchCatW(filename, MAX_PATH, L".html");
	}
	else StringCchCopyW(filename, MAX_PATH, L"wahtml_tmp.html");

	fp = _wfopen(filename, L"wt");
	if (!fp)
	{
		//MessageBox(activeHWND, IDS_HTML_ERROR_WRITE, IDS_ERROR, MB_OK | MB_ICONWARNING);
		return;
	}

	fprintf(fp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
				"<html><head>"
				"<META http-equiv=Content-Type content=\"text/html; charset=UTF-8\">"
				"<style type=\"text/css\">body{background:#000040;font-family:arial,helvetica;font-size:9pt;font-weight:normal;}"
				".name{margin-top:-1em;margin-left:15px;font-size:40pt;color:#004080;text-align:left;font-weight:900;}"
				".name-small{margin-top:-3em;margin-left:140px;font-size:22pt;color:#E1E1E1;text-align:left;}"
				"table{font-size:9pt;color:#004080;text-align:left;}"
				"hr{border:0;background-color:#FFBF00;height:1px;}"
				"ol{color:#FFFFFF;font-size:11pt;}"
				"table{margin-left:15px;color:#409FFF;}"
				".val{color:#FFBF00;}"
				".header{color:#FFBF00;font-size:14pt;}"
				"</style>"
				"<title>Winamp Generated PlayList</title></head>"
				"<body>"
				"<div>"
				"<div class=\"name\" align=\"center\"><p>WINAMP</p></div>"
				"<div class=\"name-small\" align=\"center\"><p>playlist</p></div>"
				"</div>"
				"<hr><div align=\"left\">"
				"<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\"><tr><td>");

	int x, t = playlist_list.GetCount(), t_in_pl = 0, old_t_in_pl = 0, n_un = 0;
	for (x = 0; x < t; x ++) 
	{
		int a = currentPlaylist.GetItemLengthMilliseconds(x);
		if (a >= 0) t_in_pl += (a / 1000);
		else n_un++;
	}
	if (t != n_un)
	{
		old_t_in_pl=t_in_pl;
		t_in_pl += (n_un * t_in_pl) / (t - n_un);

		fprintf(fp, "<font class=\"val\">%d</font> track%s in playlist, ", t, t == 1 ? "" : "s");
		fprintf(fp, "average track length: <font class=\"val\">%d:%02d",
				   old_t_in_pl / (t-n_un) / 60, (old_t_in_pl / (t - n_un)) % 60);

		fprintf(fp, "</font><br>%slaylist length: ",
				n_un ? "Estimated p" : "P");

		if (t_in_pl / 3600)
		{
			fprintf(fp, "<font class=\"val\">%d</font> hour%s ",
					t_in_pl / 3600, t_in_pl / 3600 == 1 ? "" : "s");
			t_in_pl %= 3600;
		}

		if (t_in_pl / 60)
		{
			fprintf(fp, "<font class=\"val\">%d</font> minute%s ",
					t_in_pl / 60, t_in_pl / 60 == 1 ? "" : "s");
			t_in_pl %= 60;
		}
		fprintf(fp, "<font class=\"val\">%d</font> second%s %s",
				t_in_pl, t_in_pl == 1 ? "" : "s", n_un ? "<br>(" : "");
		if (n_un) fprintf(fp, "<font class=\"val\">%d</font> track%s of unknown length)",
						  n_un, n_un == 1 ? "" : "s");

		fprintf(fp,
				"<br>Right-click <a href=\"file://%s\">here</a> to save this HTML file."
				"</td></tr>",
				(char *)AutoChar(filename, CP_UTF8));
	}
	else
	{
		fprintf(fp, "There are no tracks in the current playlist.<br>");
	}

	fprintf(fp, "</table></div>");

	if (t > 0)
	{
		fprintf(fp,
				"<blockquote><font class=\"header\">Playlist files:</font><ol>");

		for (x = 0; x < t; x++)
		{
			#define FILENAME_SIZE (MAX_PATH*4)
			#define FILETITLE_SIZE 400
			wchar_t ft[FILETITLE_SIZE];
			currentPlaylist.GetItemTitle(x, ft, FILENAME_SIZE);

			AutoChar narrowFt(ft, CP_UTF8);
			char *p = narrowFt;
			int l = currentPlaylist.GetItemLengthMilliseconds(x);
			if (l > 0) l /= 1000;
			fprintf(fp, "<li>");
			while (*p)
			{
				if (*p == '&') fprintf(fp, "&amp;");
				else if (*p == '<') fprintf(fp, "&lt;");
				else if (*p == '>') fprintf(fp, "&gt;");
				else if (*p == '\'') fprintf(fp, "&#39;");
				else if (*p == '"') fprintf(fp, "&quot;");
				else fputc(*p, fp);
				p++;
			}

			if(l > 0) fprintf(fp, " (%d:%02d)</li>", l / 60, l % 60);
			else fprintf(fp, "</li>");
		}

		fprintf(fp, "</ol></blockquote>");
	}
	fprintf(fp, "<hr></body></html>");
	fclose(fp);

	myOpenURL(activeHWND, filename);
}

void Playlist_ResetSelected()
{
	int i = playlist_list.GetCount();
	while (i--)
	{
		if (playlist_list.GetSelected(i))
		{
			currentPlaylist.ClearCache(i);
		}
	}
	Changed();
	SyncPlaylist();
}

void Playlist_FindSelected()
{
	if (playlist_list.GetSelectionMark() >= 0)
	{
		int l=playlist_list.GetCount();
		for(int i=0;i<l;i++)
		{
			if (playlist_list.GetSelected(i))
			{
				WASABI_API_EXPLORERFINDFILE->AddFile((wchar_t*)currentPlaylist.ItemName(i));
			}
		}
		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
}

void Playlist_DeleteSelected(int selected)
{
	selected = !!selected; // convert to 0 or 1
	int i = playlist_list.GetCount();
	while (i--)
	{
		if (!playlist_list.GetSelected(i) ^ selected)
		{
			currentPlaylist.Remove(i);
			playlist_list.Unselect(i);
		}
	}
	Changed();
	SyncPlaylist();
}

void Playlist_RecycleSelected(HWND hwndDlg, int selected)
{
	SHFILEOPSTRUCTW fileOp;
	fileOp.hwnd = hwndDlg;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = 0;
	fileOp.pTo = 0;
	fileOp.fFlags = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_USES_RECYCLEBIN) ? FOF_ALLOWUNDO : 0;
	fileOp.fAnyOperationsAborted = 0;
	fileOp.hNameMappings = 0;
	fileOp.lpszProgressTitle = 0;

	selected = !!selected; // convert to 0 or 1
	int i = playlist_list.GetCount();
	wchar_t *files = new wchar_t[i *(MAX_PATH + 1) + 1];  // need room for each file name, null terminated. then have to null terminate the whole list
	if (files)
	{
		wchar_t *curFile = files;
		for (int x = 0;x < i;x++)
		{
			if (!playlist_list.GetSelected(x) ^ selected)
			{
				lstrcpynW(curFile, currentPlaylist.ItemName(x), MAX_PATH);
				curFile += lstrlenW(currentPlaylist.ItemName(x)) + 1;
			}
		}
		if (curFile != files)
		{
			curFile[0] = 0; // null terminate

			fileOp.pFrom = files;

			if (SHFileOperationW(&fileOp))
			{
				wchar_t titleStr[32];
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_ERROR_DELETING_FILES),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32), MB_OK);
			}
			else if (!fileOp.fAnyOperationsAborted)
				while (i--)	if (!playlist_list.GetSelected(i) ^ selected)
				{
					currentPlaylist.Remove(i);
					playlist_list.Unselect(i);
				}
		}
		delete [] files;
	}
	else // if malloc failed ... maybe because there's too many items.
	{
		while (i--)	if (!playlist_list.GetSelected(i) ^ selected)
		{
			fileOp.pFrom = currentPlaylist.ItemName(i);

			if (SHFileOperationW(&fileOp))
				continue;
			if (fileOp.fAnyOperationsAborted)
				break;
			currentPlaylist.Remove(i);
			playlist_list.Unselect(i);
		}
	}
	Changed();
	SyncPlaylist();
}

int GetSelectedLength()
{
	int length = 0;
	int selected = -1;

	while ((selected = playlist_list.GetNextSelected(selected)) != -1)
	{
		int thisLen = currentPlaylist.GetItemLengthMilliseconds(selected);
		if (thisLen > 0)
			length += thisLen / 1000;
	}
	return length;
}

int GetTotalLength()
{
	int length = 0;
	int len = playlist_list.GetCount();
	for (int i = 0;i < len;i++)
	{
		int thisLen = currentPlaylist.GetItemLengthMilliseconds(i);
		if (thisLen > 0)
			length += thisLen / 1000;
	}
	return length;
}

void FormatLength(wchar_t *str, int length, int buf_len)
{
	if (!length)
		lstrcpynW(str, L"0:00", buf_len);
	else if (length < 60*60)
		StringCchPrintfW(str, buf_len, L"%d:%02d", length / 60, length % 60);
	else
	{
		int total_days = length / (60 * 60 * 24);
		if (total_days)
		{
			length -= total_days * 60 * 60 * 24;
			StringCchPrintfW(str, buf_len, L"%d %s+%d:%02d:%02d", total_days,
							 WASABI_API_LNGSTRINGW((total_days == 1 ? IDS_DAY : IDS_DAYS)),
							 length / 60 / 60, (length / 60) % 60, length % 60);
		}
		else
			StringCchPrintfW(str, buf_len, L"%d:%02d:%02d", length / 60 / 60, (length / 60) % 60, length % 60);
	}
}

void UpdatePlaylistTime(HWND hwndDlg)
{
	wchar_t str[64] = L"", str2[32] = L"";
	int selitems = playlist_list.GetSelectedCount();

	int seltime = GetSelectedLength(), ttime = GetTotalLength();

	FormatLength(str, seltime, 64);
	FormatLength(str2, ttime, 32);

	wchar_t buf2[128], sStr[16];
	if (selitems)
		StringCchPrintf(buf2, 128, WASABI_API_LNGSTRINGW(IDS_X_OF_X_SELECTED),
		selitems, playlist_list.GetCount(),
		WASABI_API_LNGSTRINGW_BUF(playlist_list.GetCount() == 1 ? IDS_ITEM : IDS_ITEMS_LOWER, sStr, 16),
		str, str2);
	else
		StringCchPrintf(buf2, 128, WASABI_API_LNGSTRINGW(IDS_X_SELECTED),
		playlist_list.GetCount(),
		WASABI_API_LNGSTRINGW_BUF(playlist_list.GetCount() == 1 ? IDS_ITEM : IDS_ITEMS_LOWER, sStr, 16),
		str2);

	SetDlgItemText(hwndDlg, IDC_PLSTATUS, buf2);
}

static wchar_t *BuildFilenameList(int is_all)
{
	int num = 0;
	wchar_t filename[MAX_PATH];

	size_t len = MAX_PATH;
	wchar_t *str = (wchar_t *)malloc(len*sizeof(wchar_t));
	size_t sofar = 0;

	int numTracks = playlist_list.GetCount();
	for (int i = 0;i < numTracks;i++)
	{
		if (is_all || playlist_list.GetSelected(i))
		{
			if (currentPlaylist.GetItem(i, filename, MAX_PATH))
			{
				int filenameLen = lstrlen(filename)+1;
				if ((filenameLen + sofar) > len)
				{
					int newLen = sofar*2; // add some cushion
					wchar_t *newStr = (wchar_t *)realloc(str, newLen * sizeof(wchar_t));
					if (!newStr)
					{
						newLen = sofar + filenameLen;
						// try the minimum possible size to get this to work
						newStr = (wchar_t *)realloc(str, newLen * sizeof(wchar_t));
						if (!newStr)
						{
							free(str);
							return 0;
						}
					}
					str = newStr;
				}

				lstrcpyn(str + sofar, filename, filenameLen);
				sofar += filenameLen;
			}
		}
	}
	*(str + sofar) = 0;

	return str;
}

void PlaySelection(int enqueue, int is_all)
{
	if (!enqueue)
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);

	int numTracks = playlist_list.GetCount();
	for (int i = 0;i < numTracks;i++)
	{
		if (is_all || playlist_list.GetSelected(i))
		{
			const wchar_t *filename = currentPlaylist.ItemName(i);
			if (filename)
			{
				enqueueFileWithMetaStructW s;
				s.filename = filename;
				if (currentPlaylist.IsCached(i))
				{
					s.title = currentPlaylist.ItemTitle(i);
					plstring_retain((wchar_t *)s.title);
					s.length = currentPlaylist.GetItemLengthMilliseconds(i) / 1000;
				}
				else
				{
					s.title = 0;
					s.length = 0;
				}
				plstring_retain((wchar_t *)s.filename);
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW_NDE_TITLE);
			}
		}
	}

	if (!enqueue)
	{
		if (is_all)
		{
			int pos = playlist_list.GetNextSelected(-1);
			if (pos != -1)
			{
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, pos, IPC_SETPLAYLISTPOS);
				SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40047, 0); // stop button, literally
				SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40045, 0); // play button, literally
				return ;
			}
		}
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
}

int playlist_Load(const wchar_t *playlistFileName)
{
	currentPlaylist.Clear();
	return AGAVE_API_PLAYLISTMANAGER->Load(playlistFileName, &currentPlaylist);
}

static void playlist_SizeDialogItemsToText(HWND hwndDlg)
{
	if (opened)
	{
		/* get ideal height for play button (we'll adjust everything according to this */
		DWORD size = MLSkinnedButton_GetIdealSize(GetDlgItem(hwndDlg, IDC_PLAY), NULL);
		int buttonHeight = HIWORD(size);	
		int initialButtonPosition = 0;
		ChildWndResizeItem *playItem = ChildSizer::Lookup(IDC_PLAY, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
		if (playItem)
		{
			initialButtonPosition = playItem->rinfo.bottom;
			int width = LOWORD(size);
			playItem->rinfo.right = playItem->rinfo.left + width + 8;
			playItem->rinfo.top = playItem->rinfo.bottom + buttonHeight;
		}

		int last_right = 0;
		const int buttonids[] = { IDC_PLAY, IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST, IDC_SAVE_PL};
		for (size_t i = 1; i < sizeof(buttonids)/sizeof(buttonids[0]); i++)
		{
			HWND controlHWND = GetDlgItem(hwndDlg, buttonids[i]);
			DWORD size = MLSkinnedButton_GetIdealSize(controlHWND, NULL);
			int width = LOWORD(size) - 2;
			int height = HIWORD(size);
			// do dynamic look of ID, slower than hardcoding but easier to maintain
			ChildWndResizeItem *thisItem = ChildSizer::Lookup(buttonids[i], playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
			ChildWndResizeItem *prevItem = ChildSizer::Lookup(buttonids[i-1], playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
			if (thisItem && prevItem) // shouldn't fail but just in case
			{
				thisItem->rinfo.left = prevItem->rinfo.right + 4;
				thisItem->rinfo.right = thisItem->rinfo.left + width;
				thisItem->rinfo.top = thisItem->rinfo.bottom + buttonHeight;
				if (i == sizeof(buttonids)/sizeof(buttonids[0])-1) last_right = (thisItem->rinfo.right += 6);
			}
		}

		/* move status bar according to button height */
		ChildWndResizeItem *statusItem = ChildSizer::Lookup(IDC_PLSTATUS, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
		if (statusItem)
		{
			/* TODO: benski> resize status bar according to font size */
			int statusHeight = statusItem->rinfo.top - statusItem->rinfo.bottom;
			statusItem->rinfo.bottom = initialButtonPosition + 3;
			statusItem->rinfo.top = buttonHeight - 2;
			statusItem->rinfo.left = last_right + 4;

			ChildWndResizeItem *listItem = ChildSizer::Lookup(IDC_PLAYLIST_EDITOR, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
			if (listItem)
			{
				listItem->rinfo.bottom = buttonHeight + 4;
			}
		}
		childSizer.Resize(hwndDlg, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
	}
}

LRESULT playlist_cloud_listview(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NOTIFY)
	{
		LPNMHDR l=(LPNMHDR)lParam;
		switch (l->code)
		{
			case TTN_SHOW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				if (cloud_avail && lvh.iItem != -1 && lvh.iSubItem == 1)
				{
					LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
					RECT r = {0};
					if (lvh.iSubItem)
						ListView_GetSubItemRect(hwnd, lvh.iItem, lvh.iSubItem, LVIR_BOUNDS, &r);
					else
					{
						ListView_GetItemRect(hwnd, lvh.iItem, &r, LVIR_BOUNDS);
						r.right = r.left + ListView_GetColumnWidth(hwnd, 1);
					}

					MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&r, 2);
					SetWindowPos(tt->hdr.hwndFrom, HWND_TOPMOST, r.right, r.top + 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
					return 1;
				}
			}
			break;

			case TTN_NEEDTEXTW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				static wchar_t tt_buf2[256] = {L""};
				static int last_item2 = -1;
				if (cloud_avail && lvh.iItem != -1 && lvh.iSubItem == 1)
				{
					LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;

					if (last_item2 == lvh.iItem)
					{
						lpnmtdi->lpszText = tt_buf2;
						return 0;
					}

					wchar_t info[16] = {0};
					currentPlaylist.GetItemExtendedInfo(lvh.iItem, L"cloud_status", info, 16);

					int status = _wtoi(info);
					if (status == 4)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_TO_SOURCE, tt_buf2, ARRAYSIZE(tt_buf2));
					}
					else
					{
						winampMediaLibraryPlugin *(*gp)();
						gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(cloud_hinst, "winampGetMediaLibraryPlugin");
						if (gp)
						{
							winampMediaLibraryPlugin *mlplugin = gp();
							if (mlplugin && (mlplugin->version == MLHDR_VER || mlplugin->version == MLHDR_VER_OLD))
							{
								// TODO handle case when not in a device
								WASABI_API_LNGSTRINGW_BUF(IDS_TRACK_AVAILABLE, tt_buf2, ARRAYSIZE(tt_buf2));

								wchar_t filepath[1024] = {0};
								currentPlaylist.GetItem(lvh.iItem, filepath, 1024);

								nx_string_t *out_devicenames = 0;
								size_t num_names = mlplugin->MessageProc(0x405, (INT_PTR)&filepath, (INT_PTR)&out_devicenames, 0);
								if (num_names > 0)
								{
									for (size_t i = 0; i < num_names; i++)
									{
										if (i > 0) StringCchCatW(tt_buf2, ARRAYSIZE(tt_buf2), L", ");
										StringCchCatW(tt_buf2, ARRAYSIZE(tt_buf2), out_devicenames[i]->string);
									}
								}
								else
								{
									WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_TO_SOURCE, tt_buf2, ARRAYSIZE(tt_buf2));
								}
								if (out_devicenames)
									free(out_devicenames);
							}
						}
					}
					last_item2 = lvh.iItem;
					lpnmtdi->lpszText = tt_buf2;

					// bit of a fiddle but it allows for multi-line tooltips
					//SendMessage(l->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 0);
				}
				else
					return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"cloud_list_proc"), hwnd, uMsg, wParam, lParam);
			}
			return 0;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"cloud_list_proc"), hwnd, uMsg, wParam, lParam);
}

static void playlist_Init(HWND hwndDlg, LPARAM lParam)
{
	HACCEL accel = LoadAccelerators(plugin.hDllInstance, MAKEINTRESOURCE(IDR_VIEW_PL_ACCELERATORS));
	if (accel)
		WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

	opened = true;
	activeHWND = hwndDlg;
	saveHWND = GetDlgItem(hwndDlg, IDC_SAVE_PL);
	Changed(false);

	cloud_avail = playlists_CloudAvailable();

	/* skin dialog */
	MLSKINWINDOW sw = {0};
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwndDlg;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin status bar */
	sw.hwndToSkin = GetDlgItem(hwndDlg, IDC_PLSTATUS);
	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin listview */
	HWND list = sw.hwndToSkin = GetDlgItem(hwndDlg, IDC_PLAYLIST_EDITOR);
	sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
	sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);
	MLSkinnedScrollWnd_ShowHorzBar(sw.hwndToSkin, FALSE);

	/* skin buttons */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwndDlg, IDC_PLAY);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin dropdown buttons */
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_DROPDOWNBUTTON;
	const int buttonids[] = { IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST};
	for (size_t i=0;i!=sizeof(buttonids)/sizeof(buttonids[0]);i++)
	{
		HWND controlHWND = GetDlgItem(hwndDlg, buttonids[i]);
		sw.hwndToSkin = controlHWND;
		MLSkinWindow(plugin.hwndLibraryParent, &sw);
	}

	sw.style -= SWBS_DROPDOWNBUTTON;
	sw.hwndToSkin = GetDlgItem(hwndDlg, IDC_SAVE_PL);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	// REVIEW: it'd be really nice to pass in a pointer to an ifc_playlist instead...
	// at this point, the main issue is how to delete/release it when we're done
	playlist_guid = tree_to_guid_map[lParam];
	{ // scope for lock
		AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);

		PlaylistInfo info(playlist_guid);
		if (info.Valid())
		{
			// will check if the playlist file exists and update the view as needed
			const wchar_t *filename = info.GetFilename();
			if (!PathFileExistsW(filename))
			{
				opened = false;

				RECT r = {0};
				HWND status = GetDlgItem(hwndDlg, IDC_PLSTATUS);
				GetWindowRect(hwndDlg, &r);
				MoveWindow(status, 20, 0, r.right - r.left - 40, r.bottom - r.top, FALSE);
				playlistwnd_rlist[8].type = 0x0011;
				
				const int ids[] = { IDC_PLAYLIST_EDITOR, IDC_PLAY, IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST, IDC_SAVE_PL};
				for (size_t i = 0; i < sizeof(ids)/sizeof(ids[0]); i++)
				{
					ShowWindow(GetDlgItem(hwndDlg, ids[i]), FALSE);
				}

				// adjust the styles without needing extra resources, etc
				DWORD style = GetWindowLongPtr(status, GWL_STYLE) - SS_ENDELLIPSIS;
				SetWindowLongPtr(status, GWL_STYLE, style | SS_CENTER | 0x2000);
				wchar_t buf[1024] = {0};
				StringCchPrintfW(buf, 1024, WASABI_API_LNGSTRINGW(IDS_SOURCE_PL_MISSING), filename);
				SetWindowTextW(status, buf);
			}
			else
			{
				playlistwnd_rlist[8].type = 0x0111;
			}

			lstrcpynW(currentPlaylistFilename, filename, MAX_PATH);
			playlist_Load(currentPlaylistFilename);

			lstrcpynW(currentPlaylistTitle, info.GetName(), 400);
		}
	}
	SetPropW(hwndDlg, L"TITLE", currentPlaylistTitle);

	playlist_list.setwnd(list);

	playlist_list.AddCol(WASABI_API_LNGSTRINGW(IDS_TITLE), 400);

	int width = 27;
	MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &width);
	playlist_list.AddCol(L"", (cloud_avail ? width : 0));
	playlist_list.AddAutoCol(WASABI_API_LNGSTRINGW(IDS_TIME));
	playlist_list.JustifyColumn(2, LVCFMT_RIGHT);

	MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(playlist_list.getwnd()), (cloud_avail ? 1 : -1));

	if (!GetPropW(list, L"cloud_list_proc")) {
		SetPropW(list, L"cloud_list_proc", (HANDLE)SetWindowLongPtrW(list, GWLP_WNDPROC, (LONG_PTR)playlist_cloud_listview));
	}

	childSizer.Init(hwndDlg, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
	playlist_SizeDialogItemsToText(hwndDlg);

	SyncPlaylist();

	// delay the calculation of the column widths until the playlist has finished processing so column widths will be correct
	PostMessage(hwndDlg, WM_APP+100, 0, 0);
}

static void playlist_Paint(HWND hwndDlg)
{
	int tab[] = { IDC_PLAYLIST_EDITOR | DCW_SUNKENBORDER };
	dialogSkinner.Draw(hwndDlg, tab, (opened ? 1 : 0));
}

static void AutoSizePlaylistColumns()
{
	playlist_list.AutoSizeColumn(2);
	RECT channelRect;
	GetClientRect(playlist_list.getwnd(), &channelRect);
	ListView_SetColumnWidth(playlist_list.getwnd(), 0, channelRect.right - playlist_list.GetColumnWidth(1) - playlist_list.GetColumnWidth(2));
}

static void playlist_Size(HWND hwndDlg, WPARAM wParam)
{
	if (wParam != SIZE_MINIMIZED)
	{
		childSizer.Resize(hwndDlg, playlistwnd_rlist, sizeof(playlistwnd_rlist) / sizeof(playlistwnd_rlist[0]));
		AutoSizePlaylistColumns();
	}
}

enum
{
	BPM_ECHO_WM_COMMAND=0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND = 0x2, // just send WM_COMMAND
};

static BOOL playlist_ButtonPopupMenu(HWND hwndDlg, int buttonId, HMENU menu, int flags=0)
{
	RECT r;
	HWND buttonHWND = GetDlgItem(hwndDlg, buttonId);
	GetWindowRect(buttonHWND, &r);
	SyncMenuWithAccelerators(hwndDlg, menu);
	MLSkinnedButton_SetDropDownState(buttonHWND, TRUE);
	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	if (!(flags & BPM_WM_COMMAND))
		tpmFlags |= TPM_RETURNCMD;
	int x = Menu_TrackPopup(menu, tpmFlags, r.left, r.top, hwndDlg, NULL);
	if ((flags & BPM_ECHO_WM_COMMAND) && x)
		SendMessage(hwndDlg, WM_COMMAND, x, 0);
	MLSkinnedButton_SetDropDownState(buttonHWND, FALSE);
	return x;
}

static void playlist_Burn(HWND hwndDlg)
{
	HMENU blah = CreatePopupMenu();

	sendToIgnoreID = lastActiveID;
	sendTo.AddHere(hwndDlg, blah, ML_TYPE_FILENAMES);

	int x = playlist_ButtonPopupMenu(hwndDlg, IDC_BURN, blah);
	if (sendTo.WasClicked(x))
	{
		int is_all = playlist_list.GetSelectedCount() == 0;
		wchar_t *names = BuildFilenameList(is_all);
		sendTo.SendFilenames(names);
		free(names);
	}

	sendTo.Cleanup();

	sendToIgnoreID = 0;
}

static void playlist_Sel(HWND hwndDlg, HWND from)
{
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 1);
	UINT menuStatus;
	if (playlist_list.GetNextSelected(-1) == -1)	menuStatus = MF_BYCOMMAND | MF_GRAYED;
	else menuStatus = MF_BYCOMMAND | MF_ENABLED;
	EnableMenuItem(listMenu, IDC_PLAYLIST_INVERT_SELECTION, menuStatus);

	if (playlist_list.GetCount() > 0)	menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else menuStatus = MF_BYCOMMAND | MF_GRAYED;
	EnableMenuItem(listMenu, IDC_PLAYLIST_SELECT_ALL, menuStatus);
	playlist_ButtonPopupMenu(hwndDlg, IDC_SEL, listMenu, BPM_WM_COMMAND);
	UpdatePlaylistTime(hwndDlg);
}

static void playlist_Rem(HWND hwndDlg, HWND from)
{
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 2);
	UINT menuStatus;
	if (playlist_list.GetNextSelected(-1) == -1)	menuStatus = MF_BYCOMMAND | MF_GRAYED;
	else menuStatus = MF_BYCOMMAND | MF_ENABLED;

	EnableMenuItem(listMenu, IDC_DELETE, menuStatus);
	EnableMenuItem(listMenu, IDC_CROP, menuStatus);

	if (playlist_list.GetCount() > 0) menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else menuStatus = MF_BYCOMMAND | MF_GRAYED;
	EnableMenuItem(listMenu, IDC_PLAYLIST_REMOVE_DEAD, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_REMOVE_ALL, menuStatus);

	playlist_ButtonPopupMenu(hwndDlg, IDC_REM, listMenu, BPM_WM_COMMAND);
	UpdatePlaylistTime(hwndDlg);
}

static void playlist_Add(HWND hwndDlg, HWND from)
{
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 3);

	playlist_ButtonPopupMenu(hwndDlg, IDC_ADD, listMenu, BPM_WM_COMMAND);
	UpdatePlaylistTime(hwndDlg);
}

static void playlist_Misc(HWND hwndDlg, HWND from)
{
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 4);

	UINT menuStatus;
	if (playlist_list.GetCount() > 0)
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
	EnableMenuItem(listMenu, IDC_PLAYLIST_RANDOMIZE, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_REVERSE, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_SORT_PATH, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_SORT_FILENAME, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_SORT_TITLE, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_RESET_CACHE, menuStatus);

	playlist_ButtonPopupMenu(hwndDlg, IDC_MISC, listMenu, BPM_WM_COMMAND);

	UpdatePlaylistTime(hwndDlg);
}

static void playlist_List(HWND hwndDlg, HWND from)
{
	sendToIgnoreID = lastActiveID;
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 5);
	sendTo.AddHere(hwndDlg, GetSubMenu(listMenu, 1), ML_TYPE_FILENAMES);

	int x = playlist_ButtonPopupMenu(hwndDlg, IDC_LIST, listMenu, BPM_ECHO_WM_COMMAND);
	if (sendTo.WasClicked(x))
	{
		wchar_t *names = BuildFilenameList(1);
		sendTo.SendFilenames(names);
		free(names);
	}
	sendTo.Cleanup();
	UpdatePlaylistTime(hwndDlg);
	sendToIgnoreID = 0;
}

static void playlist_Command(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDC_BURN:
			playlist_Burn(hwndDlg);
			break;
		case IDC_PLAY:
			{
				// if it's from an accelerator, use the appropriate setting
				int action = (HIWORD(wParam) == 1)?g_config->ReadInt("enqueuedef", 0):0;

				if (playlist_list.GetSelectedCount() > 0)
					PlaySelection(action, g_config->ReadInt("plplaymode", 1));
				else
					PlaySelection(action, 1);
			}
			break;
		case IDC_ENQUEUE:
			{
				// if it's from an accelerator, use the appropriate setting
				int action = (HIWORD(wParam) == 1)?(!g_config->ReadInt("enqueuedef", 0)):1;

				PlaySelection(action, 0);
			}
			break;
		case IDC_SEL:
			playlist_Sel(hwndDlg, (HWND)lParam);
			break;
		case IDC_REM:
			playlist_Rem(hwndDlg, (HWND)lParam);
			break;
		case IDC_ADD:
			playlist_Add(hwndDlg, (HWND)lParam);
			break;
		case IDC_MISC:
			playlist_Misc(hwndDlg, (HWND)lParam);
			break;
		case IDC_LIST:
			playlist_List(hwndDlg, (HWND)lParam);
			break;
		case IDC_DELETE:
			Playlist_DeleteSelected(1);
			break;
		case IDC_CROP:
			Playlist_DeleteSelected(0);
			break;
		case IDC_PLAYLIST_EXPLOREITEMFOLDER:
			Playlist_FindSelected();
			break;
		case IDC_PLAYLIST_VIEW_FILE_INFO:
			TagEditor(hwndDlg);
			break;
		case IDC_PLAYLIST_EDIT_ENTRY:
			EditEntry(hwndDlg);
			break;
		case IDC_PLAYLIST_RANDOMIZE:
			AGAVE_API_PLAYLISTMANAGER->Randomize(&currentPlaylist);
			playlist_list.RefreshAll();
			Changed();
			break;
		case IDC_PLAYLIST_REVERSE:
			AGAVE_API_PLAYLISTMANAGER->Reverse(&currentPlaylist);
			playlist_list.RefreshAll();
			Changed();
			break;
		case IDC_PLAYLIST_RESET_CACHE:
			Playlist_ResetSelected();
			break;
		case IDC_PLAYLIST_INVERT_SELECTION: 
			playlist_list.InvertSelection(); 
			break;
		case IDC_PLAYLIST_SELECT_ALL: 
			playlist_list.SelectAll(); 
			break;
		case IDC_ADD_FILES:
			if (CurrentPlaylist_AddFiles(hwndDlg)) Changed();
			SyncPlaylist();
			break;
		case IDC_ADD_DIRECTORY:
			if (CurrentPlaylist_AddDirectory(hwndDlg)) Changed();
			SyncPlaylist();
			break;
		case IDC_ADD_LOCATION:
			if (CurrentPlaylist_AddLocation(hwndDlg)) Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_SELECT_NONE: 
			playlist_list.UnselectAll();	
			break;
		case IDC_PLAYLIST_REMOVE_DEAD:
			if (CurrentPlaylist_DeleteMissing()) Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_REMOVE_ALL:
			currentPlaylist.Clear();
			Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_RECYCLE_SELECTED:
			Playlist_RecycleSelected(hwndDlg, 1);
			break;
		case IDC_PLAYLIST_SORT_PATH:
			currentPlaylist.SortByDirectory();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_PLAYLIST_SORT_FILENAME:
			currentPlaylist.SortByFilename();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_PLAYLIST_SORT_TITLE:
			currentPlaylist.SortByTitle();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_EXPORT_PLAYLIST:
			CurrentPlaylist_Export(hwndDlg);
			break;
		case IDC_IMPORT_PLAYLIST_FROM_FILE:
			if (currentPlaylist_ImportFromDisk(hwndDlg)) Changed();
			SyncPlaylist();
			break;
		case IDC_IMPORT_WINAMP_PLAYLIST:
			if (currentPlaylist_ImportFromWinamp(hwndDlg)) Changed();
			SyncPlaylist();
			break;
		case ID_PLAYLIST_GENERATE_HTML:
			Playlist_GenerateHtmlPlaylist();
			break;
		case IDC_SAVE_PL:
			if (changed)
			{
				playlist_Save(hwndDlg);
				Changed(false);
			}
			break;
	}
}

void playlist_Save(HWND hwndDlg)
{
	if (opened)
	{
		if (currentPlaylistFilename[0])
		{
			if (AGAVE_API_PLAYLISTMANAGER->Save(currentPlaylistFilename, &currentPlaylist) == PLAYLISTMANAGER_FAILED)
			{
				wchar_t msg[512];
				MessageBox(hwndDlg, WASABI_API_LNGSTRINGW_BUF(IDS_PLAYLIST_ERROR, msg, 512),
						   WASABI_API_LNGSTRINGW(IDS_PLAYLIST_ERROR_TITLE), MB_OK | MB_ICONWARNING);
			}
		}

		PlaylistInfo info(playlist_guid);
		info.SetSize(currentPlaylist.GetNumItems());
		info.SetLength(GetTotalLength());
		info.IssueSaveCallback();
	}
}

void playlist_SaveGUID(GUID _guid)
{
	if (playlist_guid == _guid)
	{
		if (currentPlaylistFilename[0])
		{
			AGAVE_API_PLAYLISTMANAGER->Save(currentPlaylistFilename, &currentPlaylist);
		}

		PlaylistInfo info(playlist_guid);
		info.SetSize(currentPlaylist.GetNumItems());
		info.SetLength(GetTotalLength());
		info.IssueSaveCallback();
	}
}

void playlist_Destroy(HWND hwndDlg)
{
	WASABI_API_APP->app_removeAccelerators(hwndDlg);
	if (changed) playlist_Save(hwndDlg);

	currentPlaylistFilename[0] = 0;
	currentPlaylist.Clear();
	playlist_list.setwnd(NULL);

	RemovePropW(hwndDlg, L"TITLE");
	opened = false;
	activeHWND = 0;
}

void SwapPlayEnqueueInMenu(HMENU listMenu)
{
	int playPos=-1, enqueuePos=-1;
	MENUITEMINFOW playItem={sizeof(MENUITEMINFOW), 0,}, enqueueItem={sizeof(MENUITEMINFOW), 0,};

	int numItems = GetMenuItemCount(listMenu);

	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == IDC_PLAY)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
		}
		else if (id == IDC_ENQUEUE)
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos= i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
		}
	}

	playItem.wID = IDC_ENQUEUE;
	enqueueItem.wID = IDC_PLAY;		
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

void playlist_ContextMenu(HWND hwndDlg, HWND from, int x, int y)
{
	if (from != playlist_list.getwnd())
		return ;

	POINT pt = {x,y};

	if (x == -1 || y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT channelRect = {0};
		int selected = playlist_list.GetNextSelected();
		if (selected != -1) // if something is selected we'll drop the menu from there
		{
			playlist_list.GetItemRect(selected, &channelRect);
			ClientToScreen(hwndDlg, (POINT *)&channelRect);
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect(hwndDlg, &channelRect);

			HWND hHeader = (HWND)SNDMSG(from, LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				channelRect.top += (headerRect.bottom - headerRect.top);
			}
		}
		x = channelRect.left;
		y = channelRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(from, LVM_GETHEADER, 0, 0L);
	RECT headerRect;
	if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
	{
		SetRectEmpty(&headerRect);
	}

	if (FALSE != PtInRect(&headerRect, pt))
	{
		return; 
	}

	sendToIgnoreID = lastActiveID;
	HMENU listMenu = GetSubMenu(GetSubMenu(g_context_menus, 3), 0);

	menufucker_t mf = {sizeof(mf),MENU_MLPLAYLIST,listMenu,0x3000,0x4000,0};

	UINT menuStatus, do_mf = 0;
	if (playlist_list.GetNextSelected(-1) == -1)
	{
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
		EnableMenuItem(listMenu, 2, MF_BYPOSITION | MF_GRAYED);
	}
	else
	{
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
		EnableMenuItem(listMenu, 2, MF_BYPOSITION | MF_ENABLED);
		sendTo.AddHere(hwndDlg, GetSubMenu(listMenu, 2), ML_TYPE_FILENAMES, 1);

		mf.extinf.mlplaylist.pl = &currentPlaylist;
		mf.extinf.mlplaylist.list = playlist_list.getwnd();
		pluginMessage message_build = {SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE),(intptr_t)&mf,0};
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_build, ML_IPC_SEND_PLUGIN_MESSAGE);
		do_mf = 1;
	}

	EnableMenuItem(listMenu, IDC_PLAYLIST_EXPLOREITEMFOLDER, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_VIEW_FILE_INFO, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAYLIST_EDIT_ENTRY, menuStatus);
	EnableMenuItem(listMenu, IDC_DELETE, menuStatus);
	EnableMenuItem(listMenu, IDC_CROP, menuStatus);
	EnableMenuItem(listMenu, IDC_PLAY, menuStatus);
	EnableMenuItem(listMenu, IDC_ENQUEUE, menuStatus);

	bool swapPlayEnqueue=false;
	if (g_config->ReadInt("enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(listMenu);
		swapPlayEnqueue=true;
	}

	HMENU cloud_hmenu = 0;
	if (playlists_CloudAvailable())
	{
		int mark = playlist_list.GetSelectionMark();
		if (mark != -1)
		{
			wchar_t filename[1024] = {0};
			currentPlaylist.entries[mark]->GetFilename(filename, 1024);

			cloud_hmenu = CreatePopupMenu();
			WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)&filename, (intptr_t)&cloud_hmenu);
			if (cloud_hmenu)
			{
				MENUITEMINFOW m = {sizeof(m), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0};
				m.wID = CLOUD_SOURCE_MENUS - 1;
				InsertMenuItemW(listMenu, 3, TRUE, &m);

				wchar_t a[100] = {0};
				m.fType = MFT_STRING;
				m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_SOURCES, a, 100);
				m.wID = CLOUD_SOURCE_MENUS;
				m.hSubMenu = cloud_hmenu;
				InsertMenuItemW(listMenu, 4, TRUE, &m);
			}
		}
	}

	SyncMenuWithAccelerators(hwndDlg, listMenu);
	if (swapPlayEnqueue)
		SwapPlayEnqueueInMenu(listMenu);
	int r = Menu_TrackPopup(listMenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, x, y, hwndDlg, NULL);

	if (r)
		SendMessage(hwndDlg, WM_COMMAND, r, 0);

	if (do_mf)
	{
		pluginMessage message_result = {SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE), (intptr_t)&mf, r, 0};
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_result, ML_IPC_SEND_PLUGIN_MESSAGE);
	}

	switch (r)
	{
		case 0:
			break;
		case IDC_PLAYLIST_EXPLOREITEMFOLDER:
		case IDC_PLAYLIST_VIEW_FILE_INFO:
		case IDC_PLAYLIST_EDIT_ENTRY:
			SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)from, (LPARAM)TRUE);
			break;
		default:
			if (!(menuStatus & MF_GRAYED) && sendTo.WasClicked(r))
			{
				wchar_t *names = BuildFilenameList(0);
				sendTo.SendFilenames(names);
				free(names);
			}
			else
			{
				if (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_PL_UPPER)	// deals with cloud specific menus
				{
					// 0 = no change
					// 1 = adding to cloud
					// 2 = added locally
					// 4 = removed
					int mode = 0;	// deals with cloud specific menus
					WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode);
					// TODO
					/*switch (mode)
					{
						case 1:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"5");
						break;

						case 2:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
						break;

						case 4:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
						break;
					}
					InvalidateRect(resultlist.getwnd(), NULL, TRUE);*/
				}
			}
			break;
	}
	if (!(menuStatus & MF_GRAYED))
		sendTo.Cleanup();
	sendToIgnoreID = 0;

	if (cloud_hmenu)
	{
		DeleteMenu(listMenu, CLOUD_SOURCE_MENUS - 1, MF_BYCOMMAND);
		DeleteMenu(listMenu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND);
		DestroyMenu(cloud_hmenu);
	}
}

static void playlist_LeftButtonUp(HWND hwndDlg, WPARAM wParam, POINTS pts)
{
	if (SCROLLDIR_NONE != scrollDirection)
	{
		KillTimer(hwndDlg, SCROLLTIMER_ID);
		scrollDirection = SCROLLDIR_NONE;
	}

	if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
	{
		ReleaseCapture();

		BOOL handled = FALSE;
		POINT pt;
		POINTSTOPOINT(pt, pts);

		MapWindowPoints(hwndDlg, HWND_DESKTOP, &pt, 1);
		HWND hTarget = WindowFromPoint(pt);

		if (hTarget == playlist_list.getwnd())
		{
			LVHITTESTINFO hitTest = {0};
			POINTSTOPOINT(hitTest.pt, pts);
			MapWindowPoints(hwndDlg, playlist_list.getwnd(), &hitTest.pt, 1);
			ListView_HitTest(playlist_list.getwnd(), &hitTest);

			size_t position = hitTest.iItem;
			if ((hitTest.flags & (LVHT_ONITEM)));
			else if (hitTest.flags & LVHT_ABOVE) position = 0;
			else if (hitTest.flags & (LVHT_BELOW | LVHT_NOWHERE)) position = playlist_list.GetCount();

			if (position != -1)
			{
				RECT itemRect;
				playlist_list.GetItemRect(position, &itemRect);
				if (hitTest.pt.y > (itemRect.bottom + (itemRect.top - itemRect.bottom) / 2))
					position++;

				Playlist tempList;
				size_t selected = -1, numDeleted = 0;
				// first, make a temporary list with all the selected items
				// being careful to deal with the discrepancy between the listview and the real playlist
				// as we remove items
				while ((selected = playlist_list.GetNextSelected(selected)) != -1)
				{
					tempList.entries.push_back(currentPlaylist.entries.at(selected - numDeleted));
					currentPlaylist.entries.eraseindex(selected - numDeleted);
					if ((selected - numDeleted) < position)
						position--;
					numDeleted++;
				}
				playlist_list.UnselectAll();
				// if dragging to the end of the playlist, handle things a bit differently from normal
				int pos = position;
				if (position > currentPlaylist.entries.size())
				{
					position--;
					while (numDeleted--)
					{
						currentPlaylist.entries.insert(currentPlaylist.entries.end(), tempList.entries.at(0));
						playlist_list.SetSelected(position++); // we want the same filenames to be selected
						tempList.entries.eraseindex(0);
					}
				}
				else
				{
					while (numDeleted--)
					{
						playlist_list.SetSelected(position); // we want the same filenames to be selected
						currentPlaylist.entries.insertBefore(position++, tempList.entries.at(0));
						tempList.entries.eraseindex(0);
					}
				}

				Changed();
				SyncPlaylist();
				handled = TRUE;
			}
		}

		we_are_drag_and_dropping = 0;

		if (!handled)
		{
			mlDropItemStruct m = {0};
			m.type = ML_TYPE_FILENAMESW;
			m.p = pt;
			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
			if (m.result > 0)
			{
				wchar_t *names = BuildFilenameList(0);
				m.flags = 0;
				m.result = 0;
				m.data = (void*) names;
				pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
				free(names);
			}
		}
	}
}

static void CALLBACK playlist_OnScrollTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	HWND hList = GetDlgItem(hwnd, IDC_PLAYLIST_EDITOR);

	if (SCROLLDIR_NONE == scrollDirection || 
		NULL == hList)
	{
		KillTimer(hwnd, idEvent);
		return;
	}

	RECT rc;
	rc.left = LVIR_BOUNDS;
	if (SendMessage(hList, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rc))
	{
		INT height = rc.bottom - rc.top;
		if (SCROLLDIR_UP == scrollDirection)
			height= -height;
		SendMessage(hList, LVM_SCROLL, 0, (LPARAM)height);
	}

	if (scrollTimerElapse == scrollDelay)
	{
		static INT scrollInterval = 0;
		if(0 == scrollInterval)
			scrollInterval = GetProfileInt(TEXT("windows"), TEXT("DragScrollInterval"), DD_DEFSCROLLINTERVAL);

		if (0 != scrollInterval)
			SetTimer(hwnd, idEvent, scrollTimerElapse, playlist_OnScrollTimer);
		else
			KillTimer(hwnd, idEvent);
	}
}

static INT playlist_GetScrollDirection(HWND hList, POINT pt)
{
	static INT scrollZone = 0;
	if (0 == scrollZone)
		scrollZone = GetProfileInt(TEXT("windows"), TEXT("DragScrollInset"), DD_DEFSCROLLINSET);

	RECT rc, rcTest;
	if (0 == scrollZone || !GetClientRect(playlist_list.getwnd(), &rc))
		return SCROLLDIR_NONE;

	CopyRect(&rcTest, &rc);

	rcTest.top = rcTest.bottom - scrollZone;
	if (PtInRect(&rcTest, pt))
		return SCROLLDIR_DOWN;

	rcTest.top = rc.top;
	rcTest.bottom = rcTest.top + scrollZone;

	if (0 == (LVS_NOCOLUMNHEADER & GetWindowLongPtr(hList, GWL_STYLE)))
	{
		HWND hHeader = (HWND)SendMessage(hList, LVM_GETHEADER, 0, 0L);
		if (NULL != hHeader && 0 != (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)))
		{
			RECT rcHeader;
			if (GetWindowRect(hHeader, &rcHeader))
			{
				MapWindowPoints(HWND_DESKTOP, hList, ((POINT*)&rcHeader) + 1, 1);
				INT offset = rcHeader.bottom - rc.top;
				if (0 != offset)
					OffsetRect(&rcTest, 0, offset);
			}
		}
	}

	if (PtInRect(&rcTest, pt))
		return SCROLLDIR_UP;

	return SCROLLDIR_NONE;
}

static void playlist_MouseMove(HWND hwndDlg, POINTS pts)
{
	if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
	{
		BOOL handled = FALSE;
		POINT pt;
		POINTSTOPOINT(pt, pts);

		MapWindowPoints(hwndDlg, HWND_DESKTOP, &pt, 1);
		HWND hTarget = WindowFromPoint(pt);

		INT scroll = SCROLLDIR_NONE;

		if (hTarget == playlist_list.getwnd())
		{
			LVHITTESTINFO hitTest = {0};
			POINTSTOPOINT(hitTest.pt, pts);
			MapWindowPoints(hwndDlg, playlist_list.getwnd(), &hitTest.pt, 1);

			int position = ListView_HitTest(playlist_list.getwnd(), &hitTest);

			if (position != -1)
			{
				scroll = playlist_GetScrollDirection(playlist_list.getwnd(), hitTest.pt);
				handled = TRUE;
			}
		}

		if (scroll != scrollDirection)
		{
			if (SCROLLDIR_NONE == scroll)
			{
				KillTimer(hwndDlg, SCROLLTIMER_ID);
			}
			else
			{				
				if (SCROLLDIR_NONE == scrollDirection)
				{
					if (0 == scrollDelay)
						scrollDelay = GetProfileInt(TEXT("windows"), TEXT("DragScrollDelay"), DD_DEFSCROLLDELAY);
					if (0 != scrollDelay)
					{
						scrollTimerElapse = scrollDelay;
						SetTimer(hwndDlg, SCROLLTIMER_ID, scrollTimerElapse, playlist_OnScrollTimer);
					}
				}
			}
			scrollDirection = scroll;
		}
		if (!handled)
		{
			mlDropItemStruct m = {0};
			m.type = ML_TYPE_FILENAMES;
			m.p = pt;
			m.flags = 0; //ML_HANDLEDRAG_FLAG_NOCURSOR;

			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
		}
		else 
			SetCursor(hDragNDropCursor);
	}
}

void playlist_Reload(bool forced)
{
	if (opened || forced)
	{
		if (!opened && forced)
		{
			PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
			return;
		}

		playlist_Load(currentPlaylistFilename);
		// reverted back to known state so any
		// of our current changes are now gone
		Changed(false);
		SyncPlaylist();
	}
}

void playlist_ReloadGUID(GUID _guid)
{
	if (playlist_guid == _guid)
		playlist_Reload(true);
}

void playlist_Unload(HWND hwndDlg)
{
	currentPlaylist.Clear();
	currentPlaylistFilename[0] = 0x0000;
	ListView_SetItemCount(playlist_list.getwnd(), 0);
	ListView_RedrawItems(playlist_list.getwnd(), 0, 0);
	UpdatePlaylistTime(hwndDlg);
}

void playlist_DropFiles(HDROP hDrop)
{
	wchar_t temp[2048];
	int x, y;

	y = DragQueryFileW(hDrop, 0xffffffff, temp, 2048);

	Playlist newPlaylist;
	for (x = 0; x < y; x ++)
	{
		DragQueryFileW(hDrop, x, temp, 2048);
		if (PathIsDirectory(temp))
		{
			PlaylistDirectoryCallback dirCallback(mediaLibrary.GetExtensionList());
			AGAVE_API_PLAYLISTMANAGER->LoadDirectory(temp, &newPlaylist, &dirCallback);
		}
		else
		{
			if (AGAVE_API_PLAYLISTMANAGER->Load(temp, &newPlaylist) != PLAYLISTMANAGER_SUCCESS)
			{
				//wchar_t title[400];
				//int length;
				//mediaLibrary.GetFileInfo(temp, title, 400, &length);
				//newPlaylist.AppendWithInfo(temp, title, length*1000);
				newPlaylist.AppendWithInfo(temp, 0, 0); // add with NULL info, will be fetched as needed
			}
		}
	}

	LVHITTESTINFO hitTest;
	DragQueryPoint(hDrop, &hitTest.pt);

	ListView_HitTest(playlist_list.getwnd(), &hitTest);

	if (hitTest.iItem != -1)
	{
		RECT itemRect;
		playlist_list.GetItemRect(hitTest.iItem, &itemRect);
		if (hitTest.pt.y > (itemRect.bottom + (itemRect.top - itemRect.bottom) / 2))
		{
			hitTest.iItem++;
			if (hitTest.iItem >= playlist_list.GetCount())
				hitTest.iItem = -1;
		}
	}

	if (hitTest.flags & LVHT_BELOW || hitTest.iItem == -1)
		currentPlaylist.AppendPlaylist(newPlaylist);
	else
		currentPlaylist.InsertPlaylist(newPlaylist, hitTest.iItem);

	DragFinish(hDrop);
	Changed();
	SyncPlaylist();
}

INT_PTR CALLBACK view_playlistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);	if (a)	return a;

	switch (uMsg)
	{
		case WM_INITMENUPOPUP: sendTo.InitPopupMenu(wParam); return 0;
		case WM_MOUSEMOVE: playlist_MouseMove(hwndDlg, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONUP: playlist_LeftButtonUp(hwndDlg, wParam, MAKEPOINTS(lParam)); return 0;
		case WM_PAINT: playlist_Paint(hwndDlg); return 0;
		case WM_INITDIALOG: playlist_Init(hwndDlg, lParam); return TRUE;
		case WM_DESTROY: playlist_Destroy(hwndDlg); return 0;
		case WM_COMMAND: playlist_Command(hwndDlg, wParam, lParam); return 0;
		case WM_SIZE: playlist_Size(hwndDlg, wParam); return 0;
		case WM_NOTIFY: return playlist_Notify(hwndDlg, wParam, lParam);
		case WM_CONTEXTMENU: playlist_ContextMenu(hwndDlg, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
		case WM_ERASEBKGND: return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
		case WM_PLAYLIST_RELOAD: playlist_Reload(); return 0;
		case WM_PLAYLIST_UNLOAD: playlist_Unload(hwndDlg); return 0;
		case WM_DROPFILES: playlist_DropFiles((HDROP)wParam); return 0;
		case WM_APP+102:
		{
			if (cloud_avail)
			{
				int width = 27;
				MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &width);
				playlist_list.SetColumnWidth(1, width);
				MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(playlist_list.getwnd()), 1);
			}
		}
		case WM_APP+100: playlist_Size(hwndDlg, SIZE_RESTORED); return 0;
		case WM_SETFONT: playlist_SizeDialogItemsToText(hwndDlg); return 0;
		case WM_ML_CHILDIPC:
		{
			if (lParam == ML_CHILDIPC_DROPITEM && wParam)
			{
				mlDropItemStruct *dis = (mlDropItemStruct *)wParam;
				if(dis)
				{
					switch (dis->type)
					{
						case ML_TYPE_FILENAMES:
						case ML_TYPE_STREAMNAMES:
						case ML_TYPE_FILENAMESW:
						case ML_TYPE_STREAMNAMESW:
						case ML_TYPE_ITEMRECORDLIST:
						case ML_TYPE_ITEMRECORDLISTW:
						case ML_TYPE_CDTRACKS:
							dis->result=1;
							break;
						default:
							dis->result=-1;
							break;
					}
				}
				return 0;
			}
		}
	}
	return 0;
}

static wchar_t entryFN[1040];
static INT_PTR CALLBACK entryProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hwndDlg, IDC_OLD, entryFN);
			SetDlgItemTextW(hwndDlg, IDC_NEW, entryFN);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDOK:
				GetDlgItemTextW(hwndDlg, IDC_NEW, entryFN, 1040);
				EndDialog(hwndDlg, 1);
				return 0;
			case IDCANCEL:
				EndDialog(hwndDlg, 0);
				return 0;
			}
			return 0;
	}
	return 0;
}

void EditEntry(HWND parent)
{
	int i = playlist_list.GetCount();
	while (i--) if (playlist_list.GetSelected(i))
	{
		currentPlaylist.GetItem(i, entryFN, 1040);
		INT_PTR res = WASABI_API_DIALOGBOXW(IDD_EDIT_FN, parent, entryProc);
		if (res == 1)
		{
			currentPlaylist.SetItemFilename(i, entryFN);
			currentPlaylist.ClearCache(i);
			Changed();
		}
		else
			break;
	}

	SyncPlaylist();
}