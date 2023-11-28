#include "./main.h"
#include "./simpleView.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./formatData.h"

#include "./guiObjects.h"
#include "./configIniSection.h"
#include "./configManager.h"


typedef struct __EDITORPARAM
{
	HWND hCenter;
	Profile *profile;
} EDITORPARAM;

#define PROFILE_PROP		TEXT("PROFILE_PROP")
#define GetProfile(__hwnd) ((Profile*)GetProp((__hwnd), PROFILE_PROP))

#define LASTCOLUMN_PROP		TEXT("LASTCOLUMN_PROP")




static INT_PTR CALLBACK SimpleViewEditor_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR SimpleViewEditor_Show(HWND hParent, Profile *profile)
{
	EDITORPARAM param;
	ZeroMemory(&param, sizeof(EDITORPARAM));
	param.hCenter = hParent;
	param.profile = profile;

	return WASABI_API_DIALOGBOXPARAMW(IDD_SIMPLEVIEW_EDITOR, hParent, SimpleViewEditor_DialogProc, (LPARAM)&param);
}

static BOOL SimpleViewEditor_CenterWindow(HWND hwnd, HWND hCenter)
{
	BOOL result = FALSE;
	RECT rcCenter, rcMe;
	
	if (NULL != hCenter && 
		GetClientRect(hCenter, &rcCenter) && GetWindowRect(hwnd, &rcMe))
	{
		MapWindowPoints(hCenter, HWND_DESKTOP, (POINT*)&rcCenter, 2);
		
		POINT center;
		center.x = rcCenter.left + ((rcCenter.right - rcCenter.left) - (rcMe.right - rcMe.left))/2;
		center.y = rcCenter.top + ((rcCenter.bottom - rcCenter.top) - (rcMe.bottom - rcMe.top))/2;

		if (center.x != rcMe.left || center.y != rcMe.top)
		{
			result = SetWindowPos(hwnd, NULL, center.x, center.y, 0, 0, 
						SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	if (NULL != hwnd)
		SendMessage(hwnd, DM_REPOSITION, 0, 0L);

	return result;
}

static HBRUSH SimpleViewEditor_GetDescriptionStyle(HDC hdc)
{
	COLORREF rgbBk = GetSysColor(COLOR_3DFACE);
	COLORREF rgbFg = GetSysColor(COLOR_WINDOWTEXT);
	
	rgbFg = BlendColors(rgbFg, rgbBk, 150);
	SetTextColor(hdc, rgbFg);
	SetBkColor(hdc, rgbBk);
	
	return GetSysColorBrush(COLOR_3DFACE);
}

static INT SimpleViewEditor_GetColumnId(IConfiguration *pConfig)
{
	TCHAR szBuffer[512];
	INT columnId = COLUMN_INVALID;
	if (NULL != pConfig &&
		S_OK == pConfig->ReadString(CFG_RCOLUMNSOURCE, szBuffer, ARRAYSIZE(szBuffer)))
	{
		columnId = ParseColumnName(szBuffer);
	}

	return columnId;
}

static void SimpleViewEditor_SetCheckbox(HWND hwnd, INT checkboxId, IConfiguration *pConfig, LPCSTR configIndex)
{	
	HWND hControl = GetDlgItem(hwnd, checkboxId);
	if (NULL != hControl) 
	{	
		INT iVal;
		BOOL checked = (NULL != pConfig && S_OK == pConfig->ReadInt(configIndex, &iVal) && 0 != iVal);
		SendMessage(hControl, BM_SETCHECK, (FALSE != checked) ? BST_CHECKED : BST_UNCHECKED, 0L);
	}
}

static BOOL SimpleViewEditor_UpdateCheckboxConfig(HWND hwnd, INT checkboxId, LPCSTR configIndex)
{	
	HWND hControl = GetDlgItem(hwnd, checkboxId);
    if (NULL == hControl) return FALSE;

	Profile *profile = GetProfile(hwnd);
	if (NULL == profile) return FALSE;
	
	IConfiguration *pConfig;
	if (FAILED(profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig)))
		return FALSE;
	
	INT iNew, iOld;
	BOOL result = TRUE;

	iNew = (BST_CHECKED == SendMessage(hControl, BM_GETCHECK, 0, 0L));
	if (S_OK != pConfig->ReadInt(configIndex, &iOld) || iNew != iOld)
	{
		result = SUCCEEDED(pConfig->WriteInt(configIndex, iNew));

		if (FALSE != result && NULL != profile)
			profile->Notify(ProfileCallback::eventViewConfigChanged);
	}

	

	pConfig->Release();
	return result;
}

static BOOL SimpleViewEditor_UpdateColumnConfig(HWND hwnd, INT columnId)
{
	Profile *profile = GetProfile(hwnd);
	if (NULL == profile) return FALSE;
	
	IConfiguration *pConfig;
	if (FAILED(profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig)))
		return FALSE;

	BOOL result = TRUE;

	INT columnIdOld = SimpleViewEditor_GetColumnId(pConfig);
	if (columnIdOld != columnId)
	{
		TCHAR szBuffer[512];
		FormatColumnName(columnId, szBuffer, ARRAYSIZE(szBuffer));
		result = SUCCEEDED(pConfig->WriteString(CFG_RCOLUMNSOURCE, szBuffer));
		
		if (FALSE != result && NULL != profile)
		profile->Notify(ProfileCallback::eventViewConfigChanged);
	}
	
	pConfig->Release();
	return result;
}

static BOOL SimpleViewEditor_UpdateComboConfig(HWND hwnd, BOOL fRemember)
{
	HWND hControl = GetDlgItem(hwnd, IDC_COMBO_COLUMN);
    if (NULL == hControl) return FALSE;

	INT selectedIndex = (INT)SendMessage(hControl, CB_GETCURSEL, 0, 0L);
	INT columnId = (CB_ERR != selectedIndex) ? 
					(INT)SendMessage(hControl, CB_GETITEMDATA, (WPARAM)selectedIndex, 0L) :
					COLUMN_INVALID;
		
	if (FALSE != fRemember)
		SetProp(hwnd, LASTCOLUMN_PROP, (HANDLE)(INT_PTR)columnId);

    return SimpleViewEditor_UpdateColumnConfig(hwnd, columnId);
}

static LPCTSTR SimpleViewEditor_ExpandString(LPCTSTR pszText, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (IS_INTRESOURCE(pszText))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszText, pszBuffer, cchBufferMax);
		pszText = (0 != lstrlen(pszBuffer)) ? pszBuffer : NULL;
	}

	return pszText;
}

static void SimpleViewEditor_LoadColumns(HWND hwnd)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_COLUMN);
	if (NULL == hCombo) return;

	TCHAR szBuffer[256];

	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0L);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0L);
	SendMessage(hCombo, CB_INITSTORAGE, (WPARAM)ARRAYSIZE(szRegisteredColumns), (LPARAM)(sizeof(TCHAR) * ARRAYSIZE(szBuffer)));

	
	for(INT i = 0; i < ARRAYSIZE(szRegisteredColumns); i++)
	{
		LPCTSTR pszTitle;

		pszTitle = SimpleViewEditor_ExpandString(szRegisteredColumns[i].pszTitleLong, szBuffer, ARRAYSIZE(szBuffer));
		if (NULL == pszTitle)
			pszTitle = SimpleViewEditor_ExpandString(szRegisteredColumns[i].pszTitle, szBuffer, ARRAYSIZE(szBuffer));

		INT index = (NULL != pszTitle) ? (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)pszTitle) : CB_ERR;
		if (CB_ERR != index)
		{
			SendMessage(hCombo, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
		}
	}


	SendMessage(hCombo, WM_SETREDRAW, TRUE, 0L);
	InvalidateRect(hCombo, NULL, TRUE);

}

static void SimpleViewEditor_UpdateComboboxState(HWND hwnd)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_COLUMN);
	if (NULL == hCombo) return;
	HWND hCheck = GetDlgItem(hwnd, IDC_CHECK_DETAILS);

	Profile *profile = GetProfile(hwnd);

	IConfiguration *pConfig;
	if (NULL == profile || 
		FAILED(profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig)))
	{
		pConfig = NULL;
	}

	INT columnId = SimpleViewEditor_GetColumnId(pConfig);

	if (NULL != pConfig) 
		pConfig->Release();

	
	BOOL useCombo = (NULL != hCheck && 
					BST_CHECKED == SendMessage(hCheck, BM_GETCHECK, 0, 0L));
	
	BOOL enableCombo = useCombo && (0 == (WS_DISABLED & GetWindowLongPtr(hCheck, GWL_STYLE)));
	EnableWindow(hCombo, enableCombo);

	INT selectIndex = -1;
	
	if (useCombo)
	{
		if (COLUMN_INVALID == columnId)
			columnId = (INT)(INT_PTR)GetProp(hwnd, LASTCOLUMN_PROP);
		
		INT count = (INT)SendMessage(hCombo, CB_GETCOUNT, 0, 0L);
		for(INT i = 0; i < count; i++)
		{
			if (columnId == (INT)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)i, 0L))
			{
				selectIndex = i;
				break;
			}
		}
	}

	SendMessage(hCombo, CB_SETCURSEL, (WPARAM)selectIndex, 0L);
	SimpleViewEditor_UpdateComboConfig(hwnd, FALSE);
}

static void SimpleViewEditor_InitializeProfile(HWND hwnd)
{
	Profile *profile = GetProfile(hwnd);
		
	IConfiguration *pConfig;
	if (NULL == profile || 
		FAILED(profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig)))
	{
		pConfig = NULL;
	}

	HWND hControl;
	INT szControls[] = { IDC_CHECK_INDEX,  IDC_CHECK_ICON, IDC_CHECK_DETAILS, IDC_COMBO_COLUMN, };
	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL != hControl)
		{
			EnableWindow(hControl, (NULL != pConfig));
		}
	}

	SimpleViewEditor_SetCheckbox(hwnd, IDC_CHECK_INDEX, pConfig, CFG_SHOWINDEX);
	SimpleViewEditor_SetCheckbox(hwnd, IDC_CHECK_ICON, pConfig, CFG_SHOWTYPEICON);
	
	INT columnId = SimpleViewEditor_GetColumnId(pConfig);
	SetProp(hwnd, LASTCOLUMN_PROP, (HANDLE)(INT_PTR)columnId);

	hControl = GetDlgItem(hwnd, IDC_CHECK_DETAILS);
	if (NULL != hControl)
	{
		SendMessage(hControl, BM_SETCHECK, (COLUMN_INVALID != columnId) ? BST_CHECKED : BST_UNCHECKED, 0L);
		SimpleViewEditor_UpdateComboboxState(hwnd);
	}
		
	if (NULL != pConfig) 
		pConfig->Release();
}

static INT_PTR SimpleViewEditor_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{
	EDITORPARAM *editorParam = (EDITORPARAM*)param;
	if (NULL != editorParam)
	{
		SimpleViewEditor_CenterWindow(hwnd, (HWND)editorParam->hCenter);

		if (NULL != editorParam->profile && 
			SetProp(hwnd, PROFILE_PROP, editorParam->profile))
		{
			editorParam->profile->AddRef();
		}
	}
	SimpleViewEditor_LoadColumns(hwnd);	
	SimpleViewEditor_InitializeProfile(hwnd);
	
	return FALSE;
}

static void SimpleViewEditor_OnDestroy(HWND hwnd)
{
	Profile *profile = GetProfile(hwnd);
	RemoveProp(hwnd, PROFILE_PROP);
	RemoveProp(hwnd, LASTCOLUMN_PROP);
	
	if (NULL != profile)
		profile->Release();
}

static void SimpleViewEditor_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, controlId);
			break;

		case IDC_CHECK_INDEX:
			if (BN_CLICKED == eventId)
				SimpleViewEditor_UpdateCheckboxConfig(hwnd, IDC_CHECK_INDEX, CFG_SHOWINDEX);
			break;

		case IDC_CHECK_ICON:
			if (BN_CLICKED == eventId)
				SimpleViewEditor_UpdateCheckboxConfig(hwnd, IDC_CHECK_ICON, CFG_SHOWTYPEICON);
			break;

		case IDC_CHECK_DETAILS:
			if (BN_CLICKED == eventId)
				SimpleViewEditor_UpdateComboboxState(hwnd);
			break;

		case IDC_COMBO_COLUMN:
			if (CBN_SELCHANGE == eventId)
				SimpleViewEditor_UpdateComboConfig(hwnd, TRUE);
			break;

	}
}

static LRESULT SimpleViewEditor_OnStaticColor(HWND hwnd, HDC hdc, HWND hStatic)
{
	INT controlId = GetDlgCtrlID(hStatic);

	switch(controlId)
	{
		case IDC_LABEL_INDEX:
		case IDC_LABEL_ICON:
		case IDC_LABEL_DETAILS:
			return (LRESULT)SimpleViewEditor_GetDescriptionStyle(hdc);
	}

	return 0;
}

static INT_PTR CALLBACK SimpleViewEditor_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return SimpleViewEditor_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		SimpleViewEditor_OnDestroy(hwnd); break;
		case WM_COMMAND:		SimpleViewEditor_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_CTLCOLORSTATIC:	return SimpleViewEditor_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
	}
	return 0;
}

