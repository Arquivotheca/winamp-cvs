#include "main.h"
#include "./wasabiApi.h"
#include "./toolbarItem.h"
#include "./toolbar.h"
#include "./resource.h"
#include <strsafe.h>


#define CHEVRON_WIDTH			13
#define SEPARATOR_WIDTH			8

BOOL ToolbarItem::DrawIcon(Toolbar *toolbar, HDC hdc, INT imageId, const RECT *prc, UINT stateFlags)
{
	HIMAGELIST himl = toolbar->GetImageList();

	if  (NULL == himl || 
		imageId < 0 || 
		imageId >= ImageList_GetImageCount(himl))
	{
		return FALSE;
	}
	
	INT height, frameHeight, frameWidth;

	height = toolbar->GetHeight();
	if (!ImageList_GetIconSize(himl, &frameWidth, &frameHeight))
	{
		frameHeight = 0;
		frameWidth = 0;
	}
	frameHeight = frameHeight/4;

	IMAGELISTDRAWPARAMS param;
	param.cbSize = sizeof(IMAGELISTDRAWPARAMS) - sizeof(DWORD) * 3;
	param.himl = himl;
	param.i = imageId;
	param.hdcDst = hdc;
	param.x = prc->left;
	param.y = prc->top;
	param.cx = (width > frameWidth) ? frameWidth : width;
	param.cy = (height > frameHeight) ? frameHeight : height;
	param.xBitmap = 0;
	param.yBitmap = 0;
	param.rgbBk = CLR_NONE;
	param.rgbFg = CLR_NONE;
	param.fStyle = ILD_NORMAL;
	param.dwRop = SRCCOPY;
	param.fState = ILS_NORMAL;
	param.Frame = 0;
	param.crEffect = 0;

	if (ToolbarItem::FlagDisabled & stateFlags)
		param.yBitmap = frameHeight* 3;
	else
	{
		if (ToolbarItem::FlagPressed & stateFlags)
			param.yBitmap = frameHeight * 2;
		else if (ToolbarItem::FlagHighlighted & stateFlags)
			param.yBitmap = frameHeight * 1;
	}			
	return ImageList_DrawIndirect(&param);
}

ToolbarButton::ToolbarButton(Toolbar *toolbar, UINT commandId, INT imageId, LPCTSTR pszTitle, LPCTSTR pszDescription, UINT buttonFlags) :
	ToolbarItem(toolbar, 0, buttonFlags)
{
	if (NULL != toolbar)
		width = toolbar->GetHeight();
	this->pszTitle = (IS_INTRESOURCE(pszTitle)) ? (LPTSTR)pszTitle : lfh_strdup(pszTitle);
	this->pszDescription = (IS_INTRESOURCE(pszDescription)) ? (LPTSTR)pszDescription : lfh_strdup(pszDescription);
	
	this->commandId = commandId;
	this->imageId = imageId;
}

ToolbarButton::~ToolbarButton()
{
	if (NULL != pszTitle && !IS_INTRESOURCE(pszTitle))
		lfh_free(pszTitle);
	if (NULL != pszDescription && !IS_INTRESOURCE(pszDescription))
		lfh_free(pszDescription);
}

BOOL ToolbarButton::Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags)
{
	if (DrawIcon(toolbar, hdc, imageId, prcItem, stateFlags))
		return TRUE;
	
	COLORREF rgbOld, rgb;
	
	rgbOld = GetBkColor(hdc);
	
	if (ToolbarItem::FlagDisabled & stateFlags)
		rgb = RGB(180, 180, 180);
	else
	{
		if (ToolbarItem::FlagPressed & stateFlags)
			rgb = RGB(0, 240, 0);
		else if (ToolbarItem::FlagHighlighted & stateFlags)
			rgb = RGB(0, 0, 240);
		else
			rgb = rgbOld;
	}
	if (rgb != rgbOld)
		SetBkColor(hdc, rgb);

	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcItem, NULL, 0, NULL);
	
	if (NULL != pszTitle)
	{
		TCHAR szText[128];
		LPTSTR pszText;
		if (IS_INTRESOURCE(pszTitle))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszTitle, szText, ARRAYSIZE(szText));
			pszText = szText;
		}
		else
			pszText = pszTitle;
		

		if (NULL != pszText && TEXT('\0') != pszText)
		{
			RECT rcText;
			CopyRect(&rcText, prcItem);
			InflateRect(&rcText, -1, -1);
			
			INT cchText = lstrlen(pszText);
			UINT textAlign = SetTextAlign(hdc, TA_BOTTOM | TA_LEFT);
			ExtTextOut(hdc, rcText.left, rcText.bottom, ETO_CLIPPED, &rcText, pszText, cchText, NULL);
			SetTextAlign(hdc, textAlign);
		}
	}

	if (rgb != rgbOld)
		SetBkColor(hdc, rgbOld);

	return TRUE;
}

INT ToolbarButton::GetTip(LPTSTR pszBuffer, INT cchBufferMax)
{
	if (IS_INTRESOURCE(pszDescription))
	{
		if (NULL == pszBuffer || cchBufferMax < 1) 
			return 0;
        WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszDescription, pszBuffer, cchBufferMax);
		return lstrlen(pszBuffer);
	}
	
	size_t remaining;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, pszDescription, NULL, &remaining, STRSAFE_IGNORE_NULLS);
	return SUCCEEDED(hr) ? (cchBufferMax - (INT)remaining) : 0;
	
}


BOOL ToolbarButton::ButtonDown(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags)
{
	switch(mouseButton)
	{
		case Toolbar::MouseButtonLeft:
			flags |= ToolbarItem::FlagPressed;
			return TRUE;
	}
	return FALSE;
}

BOOL ToolbarButton::ButtonUp(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags)
{
	switch(mouseButton)
	{
		case Toolbar::MouseButtonLeft:
			if (0 != (ToolbarItem::FlagPressed & flags))
				OnClick(toolbar);
			
			flags &= ~ToolbarItem::FlagPressed;
			return TRUE;
	}
	return FALSE;
}

void ToolbarButton::OnClick(Toolbar *toolbar)
{
	INT result = commandId;
	
	if (0 != (FlagDropdownButton & flags))
		result = toolbar->TrackPopupMenuEx(this, commandId);
		
	if (0 != result)
		toolbar->FireCommand(result);
}

BOOL ToolbarButton::FillMenuInfo(Toolbar *toolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	pmii->fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
	pmii->wID = commandId;
	pmii->fState = MFS_UNHILITE;
	pmii->fState |= ((0 == (FlagDisabled & flags)) ? MFS_ENABLED : MFS_DISABLED);
	pmii->dwTypeData = pszBuffer;

	if (IS_INTRESOURCE(pszTitle))
	{			
        WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszTitle, pszBuffer, cchBufferMax);
	}
	else
	{
		if (FAILED(StringCchCopyEx(pszBuffer, cchBufferMax, pszTitle, NULL, NULL, STRSAFE_IGNORE_NULLS)))
			pszBuffer[0] = L'\0';
	}

	if (0 != (FlagDropdownButton & flags))
	{
		pmii->fMask |= MIIM_SUBMENU;
		pmii->hSubMenu = toolbar->GetMenuCopy(commandId);
	}
	return TRUE;
}

ToolbarChevron::ToolbarChevron() : 
	ToolbarButton(NULL, 0, 1, NULL, MAKEINTRESOURCE(IDS_MORE), 0)
{
	width = CHEVRON_WIDTH;
}

void ToolbarChevron::OnClick(Toolbar *toolbar)
{	
	if (NULL != toolbar)
		toolbar->DisplayButtonsMenu(this);
	
}

ToolbarSpacer::ToolbarSpacer(Toolbar *toolbar, BOOL flexMode) :
	ToolbarItem(NULL, 0, FlagSpacer | ((0 != flexMode) ? FlagFlexSpacer : 0))
{
	if (NULL != toolbar)
		width = toolbar->GetHeight();
}

ToolbarSeparator::ToolbarSeparator() :
	ToolbarItem(NULL, SEPARATOR_WIDTH, FlagSpacer)
{
}

BOOL ToolbarSeparator::Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags)
{
	return DrawIcon(toolbar, hdc, 0, prcItem, stateFlags);
}


BOOL ToolbarSeparator::FillMenuInfo(Toolbar *toolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	pmii->fMask = MIIM_FTYPE;
	pmii->fType = MFT_MENUBREAK;
	return TRUE;
}