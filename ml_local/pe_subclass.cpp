#include "main.h"
#include "api.h"
#include "resource.h"

static WNDPROC PE_oldWndProc;
static HMENU last_viewmenu = NULL;
static WORD waCmdMenuID;

static INT_PTR CALLBACK PE_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND && wParam > 45000 && wParam < 57000)
	{
		int n = wParam - 45000;
		if (m_query_list[n])
		{
			queryItem *item = m_query_list[n];
			char configDir[MAX_PATH];
			PathCombineA(configDir, g_viewsDir, item->metafn);
			C_Config viewconf(configDir);
			main_playQuery(&viewconf, item->query, 0, 0);

			return 0;
		}
	}
	else if (uMsg == WM_INITMENUPOPUP)
	{
		HMENU hmenuPopup = (HMENU) wParam;
		if (hmenuPopup == wa_playlists_cmdmenu)
		{
			if (!waCmdMenuID)
			{
				waCmdMenuID = (WORD)SendMessage(lMedia.hwndWinampParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);
			}
			if (last_viewmenu)
			{
				RemoveMenu(wa_playlists_cmdmenu, waCmdMenuID, MF_BYCOMMAND);
				DestroyMenu(last_viewmenu);
				last_viewmenu = NULL;
			}

			mlGetTreeStruct mgts = { 1000, 45000, -1};
			last_viewmenu = (HMENU)SendMessage(lMedia.hwndLibraryParent, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
			if (last_viewmenu)
			{
				MENUITEMINFOW menuItem = {sizeof(MENUITEMINFOW), MIIM_SUBMENU | MIIM_ID | MIIM_TYPE, MFT_STRING,
										  MFS_ENABLED, waCmdMenuID, last_viewmenu, NULL, NULL, NULL,
										  WASABI_API_LNGSTRINGW(IDS_OPEN_MEDIA_LIBRARY_VIEW_RESULTS), 0};

				if (GetMenuItemCount(last_viewmenu) > 0)
				{
					InsertMenuItemW(wa_playlists_cmdmenu, 1, TRUE, &menuItem);
				}
				else
				{
					DestroyMenu(last_viewmenu);
					last_viewmenu=0;
				}
			}
		}
	}
	return CallWindowProc(PE_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

static HWND hwnd_pe = NULL;
void HookPlaylistEditor()
{
	hwnd_pe = (HWND)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND);

	if (hwnd_pe)
		PE_oldWndProc = (WNDPROC) SetWindowLongPtr(hwnd_pe, GWLP_WNDPROC, (LONG_PTR)PE_newWndProc);
}

void UnhookPlaylistEditor()
{
	SetWindowLongPtr(hwnd_pe, GWLP_WNDPROC, (LONG_PTR)PE_oldWndProc);
}
