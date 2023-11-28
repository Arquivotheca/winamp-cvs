/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"

static LRESULT WINAPI jumpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static LRESULT WINAPI jumpFileDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

HWND jump_hwnd,jump_hwnd2;

int jump_dialog(HWND hwnd)
{
	if (jump_hwnd && IsWindow(jump_hwnd)) { SetForegroundWindow(jump_hwnd); return -1; }
	LPCreateDialogW(IDD_JUMPDLG,DIALOG_PARENT(hwnd), jumpDlgProc);
	return 0;
}

int jump_file_dialog(HWND hwnd)
{
	if (jump_hwnd2 && IsWindow(jump_hwnd2)) { SetForegroundWindow(jump_hwnd2); return -1; }
		LPCreateDialogW(IDD_JUMPFILEDLG,DIALOG_PARENT(hwnd),jumpFileDlgProc);
	return 0;
}

static LRESULT WINAPI jumpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			ShowWindow(jump_hwnd=hwndDlg,SW_SHOW);
			{
				char text[128];
				int len=0;
				if (in_mod)	len=in_mod->GetLength()/1000;
				StringCchPrintf(text,128,"%d:%02d",len/60,len%60);

				if (in_mod)	len = in_mod->GetOutputTime()/1000;
				else len=0;
				SetDlgItemText(hwndDlg,IDC_TRACKLEN,text);
				StringCchPrintf(text,128,"%d:%02d",len/60,len%60);
				SetDlgItemText(hwndDlg,IDC_MINUTES,text);

				// show jump to time window and restore last position as applicable
				POINT pt = {time_rect.left, time_rect.top};
				if (!windowOffScreen(hwndDlg, pt))
					SetWindowPos(hwndDlg, HWND_TOP, time_rect.left, time_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
			}
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					char text[129];
					int m=0,s=0,bt=0;
					char *p=text;
					GetDlgItemText(hwndDlg,IDC_MINUTES,text,sizeof(text));
					while (*p == ' ') p++;
					while (*p >= '0' && *p <= '9') {bt=1;s=s*10+*p++-'0';}
					if (*p++ == ':')
					{
						m=s;s=0;
						while (*p >= '0' && *p <= '9') {bt=1;s=s*10+*p++-'0';}
					}
					if (bt) 
					{
						int time=m*60000+s*1000;
  						if (time >= 0 && !PlayList_ishidden(PlayList_getPosition()))
						{
							if (in_seek(time) < 0)
								SendMessage(hMainWindow,WM_WA_MPEG_EOF,0,0);
							else
							{
								ui_drawtime(in_getouttime()/1000,0);
							}
						}
						Sleep(100);
						while (1)
						{
							MSG msg;
							if (!PeekMessage(&msg,hMainWindow,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE)) 
								break;
						}
					}
				}
				case IDCANCEL:
					GetWindowRect(hwndDlg, &time_rect);
					DestroyWindow(hwndDlg);
				return FALSE;
			}
		return FALSE;
		case WM_DESTROY:
			jump_hwnd=0;
		return 0;
	}
	return 0;
}

static void parselist(char *out, const char *in)
{
	int inquotes=0, neednull=0;
	while (*in)
	{
		char c=*in++;
		if (c >= 'A' && c <= 'Z') c+='a'-'A';

		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
		{
			neednull=1;
			*out++=c;
		}
		else if (c == '\"') 
		{
			inquotes=!inquotes;
			if (!inquotes) 
			{
				*out++=0;
				neednull=0;
			}
		}
		else
		{
			if (inquotes) *out++=c;
			else if (neednull)
			{
				*out++=0;
				neednull=0;
			}
		}
	}
	*out++=0;
	*out++=0;
}

extern "C"
{
	static int __cdecl substr_search(const char *bigtext, const char *littletext)
	{
		char littletext_list[128], *plist;
		char bigtext_list[MAX_PATH*8];
		 
		parselist(littletext_list,littletext);

		StringCchCopy(bigtext_list,MAX_PATH*8,bigtext);
		plist = bigtext_list;
		while (*plist)
		{
			if (*plist >= 'A' && *plist <= 'Z') *plist+='a'-'A';
			plist++;
		}

		plist=littletext_list;
		while (*plist)
		{
			if (!strstr(bigtext_list,plist)) return 0;
			plist+=lstrlen(plist)+1;
		}
		return 1;
	}

	static int (__cdecl *jtf_comparator)(const char *, const char *)=substr_search;

	static int (__cdecl *jtf_comparatorW)(const wchar_t *, const wchar_t *)=0;
}

void SetJumpComparator(void *functionPtr)
{
	if (functionPtr)
		jtf_comparator=(int (__cdecl *)(const char *, const char *))(functionPtr);
	else
		jtf_comparator=substr_search;
}

void SetJumpComparatorW(void *functionPtr)
{
	if (functionPtr)
		jtf_comparatorW=(int (__cdecl *)(const wchar_t *, const wchar_t *))(functionPtr);
	else
		jtf_comparatorW=0;
}

static WNDPROC oldWndProc;

static LRESULT WINAPI newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)&&(wParam == VK_UP || wParam == VK_DOWN|| wParam == VK_PRIOR || wParam == VK_NEXT))
	{
		SendMessage(GetDlgItem(GetParent(hwndDlg),IDC_SELBOX),uMsg,wParam,lParam);
		return 0;
	}
	return CallWindowProc(oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

static LRESULT WINAPI jumpFileDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			ShowWindow(jump_hwnd2=hwndDlg,SW_SHOW);
			{
				HWND hw;
				int v,x,t=PlayList_getPosition();
				wchar_t fn[FILENAME_SIZE], ft[FILETITLE_SIZE];
				v = PlayList_getlength();
				hw = GetDlgItem(hwndDlg,IDC_SELBOX);
				SendMessage(hw,WM_SETREDRAW,FALSE,0);
				for (x = 0; x < v; x ++)
				{
					PlayList_getitem2W(x,fn,ft);
					SendMessage(hw,LB_SETITEMDATA,SendMessageW(hw,LB_ADDSTRING,0,(LPARAM) ft),x);
				}
				for (x = 0; x < v; x ++)
				{
					if (SendMessage(hw,LB_GETITEMDATA,x,0)==t) 
						break;
				}
				SendMessage(hw,WM_SETREDRAW,TRUE,0);
				SendMessage(hw,LB_SETCURSEL,x,0);
		        oldWndProc=(WNDPROC) SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_EDIT1),GWLP_WNDPROC,(LONG_PTR)newWndProc);
			}
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					LRESULT x=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0),0);

					if (x >= 0 && x != PlayList_getPosition())
					{
						PlayList_setposition(x);
						PlayList_getcurrent(FileName,FileTitle,FileTitleNum);
						StartPlaying();
					}

					Sleep(100);
					while (1)
					{
						MSG msg;
						if (!PeekMessage(&msg,hMainWindow,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE)) 
							break;
					}
				}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
				return FALSE;
				case IDC_SELBOX:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(hwndDlg,WM_COMMAND,IDOK,0);
					}
				return FALSE;
					case IDC_EDIT1:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						wchar_t s[64];
						HWND hw;
						int v,x;//,t=PlayList_getPosition();
						if(jtf_comparatorW) GetDlgItemTextW(hwndDlg,IDC_EDIT1,s,64);
						else GetDlgItemText(hwndDlg,IDC_EDIT1,(char*)s,64);
						v = PlayList_getlength();
						hw = GetDlgItem(hwndDlg,IDC_SELBOX);
						SendMessage(hw,WM_SETREDRAW,FALSE,0);
						SendMessage(hw,LB_RESETCONTENT,0,0);
						for (x = 0; x < v; x ++)
						{
							// DRO (15/02/09) - added jtf_comparatorW support with reworking as needed
							int addtolist = 1;
							if(jtf_comparatorW){
								wchar_t buf[FILENAME_SIZE+FILETITLE_SIZE+1],b2[FILETITLE_SIZE];
								PlayList_getitem2W(x,buf,b2);
								StringCchCatW(buf,FILENAME_SIZE+FILETITLE_SIZE+1,L" ");
								StringCchCatW(buf,FILENAME_SIZE+FILETITLE_SIZE+1,b2);
								addtolist = jtf_comparatorW(buf,s);
							}
							else{
								char buf[FILENAME_SIZE+FILETITLE_SIZE+1],b2[FILETITLE_SIZE];
								PlayList_getitem2(x,buf,b2);
								StringCchCat(buf,FILENAME_SIZE+FILETITLE_SIZE+1," ");
								StringCchCat(buf,FILENAME_SIZE+FILETITLE_SIZE+1,b2);
								addtolist = jtf_comparator(buf,(char*)s);
							}

							if(!s[0] || addtolist){
								// DRO (15/02/09) - disabled code working on 'buf' as isn't used
								//                  and cannot see what/why it is/was enabled for
								wchar_t /*buf[FILENAME_SIZE+FILETITLE_SIZE+1],*/b2[FILETITLE_SIZE];
								PlayList_getitem2W(x,0/*buf*/,b2);
								/*StringCchCatW(buf,FILENAME_SIZE+FILETITLE_SIZE+1,L" ");
								StringCchCatW(buf,FILENAME_SIZE+FILETITLE_SIZE+1,b2);*/
								SendMessage(hw,LB_SETITEMDATA,SendMessageW(hw,LB_ADDSTRING,0,(LPARAM) b2),x);
							}
						}
						SendMessage(hw,LB_SETCURSEL,0,x);
						SendMessage(hw,WM_SETREDRAW,TRUE,0);
					}
					return FALSE;
				}
			return FALSE;
		case WM_DESTROY:
			jump_hwnd2=0;
		return 0;
	}
	return 0;
}