#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"
#if 0//syntax highlighting
#include "compiler.h"
#include "richedit.h"
#endif

char* GetTextResource(UINT id){
	void* data = 0;
	HRSRC rsrc = FindResource(WASABI_API_LNG_HINST,MAKEINTRESOURCE(id),"TEXT");
	if(rsrc){
	HGLOBAL resourceHandle = LoadResource(WASABI_API_LNG_HINST,rsrc);
		data = LockResource(resourceHandle);
	}
	return (char*)data;
}

void loadComboBox(HWND dlg, char *ext, char *selectedName) 
{
	char path[MAX_PATH];
	int a;
	HANDLE ff;
	WIN32_FIND_DATA fd;

	wsprintf(path,"%s\\%s",g_path,ext);

	ff=FindFirstFile(path, &fd);
	if (ff == INVALID_HANDLE_VALUE) return;

	do
	{
		SendMessage(dlg, CB_ADDSTRING, 0, (LPARAM)(fd.cFileName));
	}
	while (FindNextFile(ff, &fd));
	FindClose(ff);

	a = SendMessage(dlg, CB_FINDSTRINGEXACT, 0, (LPARAM)(selectedName));
	if (a != CB_ERR) SendMessage(dlg, CB_SETCURSEL, (WPARAM) a, 0);
}

void GR_SelectColor(HWND hwnd, int *a)
{
	static COLORREF custcolors[16];
	CHOOSECOLOR cs;
	cs.lStructSize = sizeof(cs);
	cs.hwndOwner = hwnd;
	cs.hInstance = 0;
	cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
	cs.lpCustColors = custcolors;
	cs.Flags = CC_RGBINIT|CC_FULLOPEN;
	if (ChooseColor(&cs))
	{
		*a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
	}
}

void GR_DrawColoredButton(DRAWITEMSTRUCT *di, COLORREF color)
{
	color = ((color>>16)&0xff)|(color&0xff00)|((color<<16)&0xff0000);

	char wt[123];
	HPEN hPen,hOldPen;
	HBRUSH hBrush,hOldBrush;
	hPen = (HPEN)GetStockObject(BLACK_PEN);
	LOGBRUSH lb={BS_SOLID,color,0};
	hBrush = CreateBrushIndirect(&lb);
	hOldPen=(HPEN)SelectObject(di->hDC,hPen);
	hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);

	Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);

	GetWindowText(di->hwndItem,wt,sizeof(wt));
	SetBkColor(di->hDC,color);
	SetTextColor(di->hDC,~color & 0xffffff);
	DrawText(di->hDC,wt,-1,&di->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	
	DeleteObject(hBrush);
	SelectObject(di->hDC,hOldPen);
	SelectObject(di->hDC,hOldBrush);
}

static int m_help_lastpage=4;
static char *m_localtext;
static void _dosetsel(HWND hwndDlg)
{
	HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
	int sel=TabCtrl_GetCurSel(tabwnd);
	char *text="";

	m_help_lastpage=sel;

	if (sel == 0)
		text=GetTextResource(IDR_HELP_1);
	else if (sel == 1)
		text=GetTextResource(IDR_HELP_2);
	else if (sel == 2)
		text=GetTextResource(IDR_HELP_3);
	else if (sel == 3)
		text=GetTextResource(IDR_HELP_4);
	else if (sel == 4 && m_localtext)
		text=m_localtext;

	SetDlgItemText(hwndDlg,IDC_EDIT1,text);
}

static BOOL CALLBACK evalHelpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				TCITEM item;
				HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
				item.mask=TCIF_TEXT;
				item.pszText=WASABI_API_LNGSTRING(IDS_GENERAL);
				TabCtrl_InsertItem(tabwnd,0,&item);
				item.pszText=WASABI_API_LNGSTRING(IDS_OPERATORS);
				TabCtrl_InsertItem(tabwnd,1,&item);
				item.pszText=WASABI_API_LNGSTRING(IDS_FUNCTIONS);
				TabCtrl_InsertItem(tabwnd,2,&item);
				item.pszText=WASABI_API_LNGSTRING(IDS_CONSTANTS);
				TabCtrl_InsertItem(tabwnd,3,&item);
				// fucko: context specific stuff
				m_localtext=0;
				if (lParam)
				{
					item.pszText=(char *)lParam;
					m_localtext=item.pszText + strlen(item.pszText)+1;
					TabCtrl_InsertItem(tabwnd,4,&item);
				}
				else if (m_help_lastpage > 3) m_help_lastpage=0;

				TabCtrl_SetCurSel(tabwnd,m_help_lastpage);
				_dosetsel(hwndDlg);
			}
			return 0;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
				EndDialog(hwndDlg,1);
			return 0;

		case WM_NOTIFY:
			{
				LPNMHDR p=(LPNMHDR) lParam;
				if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE) _dosetsel(hwndDlg);
			}
			return 0;
	}
	return 0;
}

void compilerfunctionlist(HWND hwndDlg, char *localinfo)
{
	WASABI_API_DIALOGBOXPARAM(IDD_EVAL_HELP,hwndDlg,evalHelpDlgProc, (LONG)localinfo);
}

#if 0//syntax highlighting
// If you include richedit boxes, you need to load the richlib at the beginning:
// HANDLE hRichLib = LoadLibrary("RICHED32.DLL");
// When quitting:
// FreeLibrary(hRichLib);


#define M_WORD 1
#define M_NUM  2
#define M_COMM 3

#define is_alpha(a)  ((((a) >= 'A') && ((a) <= 'Z')) || (((a) >= 'a') && ((a) <= 'z')))
#define is_num(a)  (((a) >= '0') && ((a) <= '9'))

#define is_op(a) (((a) == '=') || ((a) == '+') || ((a) == '-') || ((a) == '*') || ((a) == '/') || ((a) == '%'))

// Colors for bracket pairs (loops around, ugly colors for now), can be any number of colors
static int bcol[] = { RGB(192, 0, 0), RGB(64, 128, 128), RGB(128, 0, 255), RGB(128, 128, 255) };

#define COLOR_COMMENT RGB(0, 128, 0)
#define COLOR_FUNC RGB(0, 0, 192)
#define COLOR_VAR RGB(96, 96, 96)
#define COLOR_OP RGB(0, 0, 0)
#define COLOR_NUMBER RGB(0, 0, 128)

// Actual syntax highlighting
// 'hwnd' is a richedit box, 'data' is the code of 'size' characters
void doAVSEvalHighLight(HWND hwndDlg, UINT sub, char *data) {
	int size=strlen(data);
	HWND hwnd=GetDlgItem(hwndDlg,sub);
	CHARRANGE cr, cro;
	
	CHARFORMAT cf;
	cf.cbSize    = sizeof(CHARFORMAT);
	cf.dwMask    = CFM_COLOR;
	cf.dwEffects = 0;

	SendMessage(hwnd, WM_SETREDRAW, false, 0);
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&cro);
	int mode = 0;
	int pos = 0;
	int brackets = 0;
	for (int i = 0; i < size; ++i)
	{
		if (mode == M_COMM)
		{
			// We're inside a comment, check for its end
			if ((data[i] == ';') || ((data[i] == '*') && ((i+1) < size) && (data[++i] == '/'))) {
				mode = 0;
				cf.crTextColor = COLOR_COMMENT;
				cr.cpMin = pos;
				cr.cpMax = i+1;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		}
		else if (is_alpha(data[i]) || ((mode == M_WORD) && is_num(data[i])))
		{
			// Letter tokens (func calls, vars)
			if (mode != M_WORD)
			{
				// Enter word-mode if we haven't yet
				mode = M_WORD;
				pos = i;
			}
			// Stop condition
			bool valid = (i != (size-1)); // Check if this isn't the last character
			if (valid)
			{
				valid = is_num(data[i+1]) || is_alpha(data[i+1]);
			}
			if (!valid) {
				// We have reached the end of this word
				cr.cpMin = pos;
				cr.cpMax = i+1;
				// Check if its a function
				bool func = false;
				for (int j = 0; j < (sizeof(fnTable) / sizeof(fnTable[0])); ++j)
				{
					if ((i - pos + 1) == (signed)strlen(fnTable[j].name))
					{
						if (_strnicmp(fnTable[j].name, &data[pos], strlen(fnTable[j].name)) == 0)
						{
							func = true;
							break;
						}
					}
				}
				cf.crTextColor = func ? COLOR_FUNC : COLOR_VAR;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		}
		else if (is_op(data[i]))
		{
			// This is an operator
			mode = 0;
			cf.crTextColor = COLOR_OP;
			cr.cpMin = i;
			cr.cpMax = i+1;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		else if ((data[i] & 0x80) || ((data[i] == '/') && ((i+1) < size) && (data[++i] == '*')))
		{
			// This is a comment marker, enter comment mode
			mode = M_COMM;
			pos = i;
		}
		else if ((data[i] == '(') || (data[i] == ')') || (data[i] == ','))
		{
			// Reached brackets: we count them and color them in pairs
			mode = 0;
			if (data[i] == '(') ++brackets;
			cf.crTextColor = bcol[brackets % (sizeof(bcol) / sizeof(bcol[0]))];
			if (data[i] == ')') --brackets;
			cr.cpMin = i;
			cr.cpMax = i+1;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		else if (is_num(data[i]) || (data[i] == '.'))
		{
			// constants
			if (mode != M_NUM)
			{
				pos = i;
				mode = M_NUM;
			}
			// Stop condition
			bool valid = (i != (size-1));
			if (valid)
			{
				valid = is_num(data[i+1]) || (data[i+1] == '.');
			}
			if (!valid)
			{
				cf.crTextColor = COLOR_NUMBER;
				cr.cpMin = pos;
				cr.cpMax = i+1;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		}
		else if (data[i] == ';')
		{
			// Reset bracket count and mode for every statement.
			mode = 0;
			brackets = 0;
		}
	}
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cro);
	SendMessage(hwnd, WM_SETREDRAW, true, 0);
	InvalidateRect(hwnd, 0, true);
}
#endif