#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./itemViewManager.h"
#include "./itemViewMeta.h"

#include "./groupedList.h"
#include "./guiObjects.h"
#include <tmschema.h>

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>

#define IMAGE_FRAME_WIDTH	3
#define IMAGE_WIDTH			104
#define IMAGE_HEIGHT			104
#define IMAGE_OFFSET_X		4
#define IMAGE_OFFSET_Y		4

#define TITLE_OFFSET_X		16
#define TITLE_OFFSET_Y		4

#define DESCRIPTION_OFFSET_X	4
#define DESCRIPTION_OFFSET_Y		2

#define BUTTON_OFFSET_X		4
#define BUTTON_OFFSET_Y		4

class GLButtonComplex : public GLButton
{
public:
	typedef enum
	{
		FlagButtonHot		= 0x00000001,
		FlagButtonPressed	= 0x00000002,
		FlagButtonDisabled	= 0x00000004,
		FlagButtonDown		= 0x00000008,
	} FlagButtonState;

protected:
	GLButtonComplex(DropboxViewMeta *viewMeta);
	~GLButtonComplex();

public:
	static GLItem *CreateInstance(DropboxViewMeta *viewMeta);

public:
	BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags);
	INT GetHeight(GLStyle *style);
	BOOL AdjustRect(GLStyle *style, RECT *prcItem);

	void MouseEnter(GLView *view);
	void MouseLeave(GLView *view);
	void MouseMove(GLView *view, const RECT *prcItem, POINT pt);
	BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonUp(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonClick(GLView *view);

	void KeyDown(GLView *view, UINT vKey);	
	void KeyUp(GLView *view, UINT vKey);
	void KeyPressed(GLView *view, UINT vKey);

	void StyleChanged(GLView *view, UINT updateFlags);

protected:
	BOOL DrawImage(GLStyle *style, HDC hdc, const RECT *prcItem, UINT itemState, BOOL drawFocus);
	BOOL GetImageRect(const RECT *itemRect, RECT *imageRect);

	BOOL GetButtonRect(const RECT *itemRect, RECT *buttonRect);
	BOOL DrawButton(GLStyle *style, HDC hdc, const RECT *prcItem, UINT itemState, BOOL drawFocus);

protected:
	LPTSTR description;
	HBITMAP image;
	UINT buttonState;
	INT buttonWidth;
	INT buttonHeight;
};

static LPTSTR GLButtonComplex_StrDup(LPCTSTR source)
{
	if (NULL == source)
		return NULL;
		
	INT cchText = lstrlen(source);
	
	LPTSTR dest = (LPTSTR)malloc(sizeof(TCHAR) * (cchText + 1));
	if (NULL != dest)
	{
		CopyMemory(dest, source, sizeof(TCHAR) * cchText);
		dest[cchText] = TEXT('\0');
	}
	return dest;
}

static void GLButtonComplex_StrFree(LPTSTR value)
{
	if (NULL != value && !IS_INTRESOURCE(value))
		free(value);
}


typedef struct __VIEWENUMPARAM
{
	GLRoot	*root;
	INT		activeViewId;
	BOOL	selectedOk;
} VIEWENUMPARAM;

static BOOL CALLBACK PreferencesView_EnumProc(DropboxViewMeta *viewMeta, ULONG_PTR param)
{
	VIEWENUMPARAM *enumParam= (VIEWENUMPARAM*)param;
	if (NULL == enumParam)
		return FALSE;

	GLItem *item = GLButtonComplex::CreateInstance(viewMeta);
	
	if (NULL != item)
	{
		if (FALSE == enumParam->selectedOk && 
			viewMeta->GetId() == enumParam->activeViewId)
		{
			item->SetFlags(GLButtonComplex::FlagButtonChecked, GLButtonComplex::FlagButtonChecked);
			enumParam->selectedOk = TRUE;
		}

		enumParam->root->InsertItem(-1, item);
		item->Release();
	}

	return TRUE;
}

GLRoot *PreferencesView_CreateGroup(INT activeViewId)
{	
	GLRoot *root = GLRoot::Create();
	if (NULL == root ) return NULL;
		
	VIEWENUMPARAM param;
	param.activeViewId = activeViewId;
	param.root = root;
	param.selectedOk = FALSE;
	PLUGIN_VIEWMNGR->EnumerateViews(PreferencesView_EnumProc, (ULONG_PTR)&param);

	
	return root;
}


GLButtonComplex::GLButtonComplex(DropboxViewMeta *viewMeta) 
	: GLButton(viewMeta->GetId(), TEXT(""), FlagTypeRadiobutton), buttonWidth(0), buttonHeight(0), buttonState(0)
{
	
	TCHAR szBuffer[4096];
	if (SUCCEEDED(viewMeta->GetTitle(szBuffer, ARRAYSIZE(szBuffer))))
        this->SetTitle(szBuffer);

	if (FAILED(viewMeta->GetDescirption(szBuffer, ARRAYSIZE(szBuffer))))
		szBuffer[0] = TEXT('\0');
	description = GLButtonComplex_StrDup(szBuffer);

	image = viewMeta->LoadPreview();
	
	if (FALSE == viewMeta->HasEditor())
		buttonState |= FlagButtonDisabled;
}

GLButtonComplex::~GLButtonComplex()
{
	GLButtonComplex_StrFree(description);
	if (NULL != image)
		DeleteObject(image);
}

GLItem *GLButtonComplex::CreateInstance(DropboxViewMeta *viewMeta)
{
	if (NULL == viewMeta)
		return NULL;
	
	return new GLButtonComplex(viewMeta);
}
static void GLButtonComplex_DrawFrame(HDC hdc, const RECT *prc, INT width, COLORREF rgbFrame)
{	
	if (width > 0)
	{
		
		COLORREF rgbOld = SetBkColor(hdc, rgbFrame);	

		RECT rcPart;
		SetRect(&rcPart, prc->left, prc->top, prc->right, prc->top + width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->bottom - width, prc->right, prc->bottom); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->top + width, 	prc->left + width, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->right - width, prc->top + width, prc->right, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		
		if (rgbOld != rgbFrame)
			SetBkColor(hdc, rgbOld);
	}
}

BOOL GLButtonComplex::GetImageRect(const RECT *itemRect, RECT *imageRect)
{
	if (NULL == itemRect || NULL == imageRect)
		return FALSE;

	return SetRect(imageRect, itemRect->left + IMAGE_OFFSET_X,  itemRect->top + IMAGE_OFFSET_Y, 
			itemRect->left + IMAGE_OFFSET_X + IMAGE_FRAME_WIDTH * 2 + IMAGE_WIDTH,
			itemRect->top + IMAGE_OFFSET_Y + IMAGE_FRAME_WIDTH * 2 + IMAGE_HEIGHT);
}

BOOL GLButtonComplex::DrawImage(GLStyle *style, HDC hdc, const RECT *prcItem, UINT itemState, BOOL drawFocus)
{	
	RECT rcImage;

	if (!GetImageRect(prcItem, &rcImage))
		return FALSE;
	
	COLORREF rgbFrame;
	
	if (0 != (FlagStateDisabled & itemState))
		rgbFrame = style->GetColor(GLStyle::uiColorItem);
	else if (FlagButtonChecked == GetValue())
		rgbFrame = (0 != (FlagStateHot & itemState)) ? RGB(247, 202, 96) : RGB(251, 230, 148);
	else 
		rgbFrame = (0 != (FlagStateHot & itemState)) ? RGB(0, 0, 0) : RGB(47, 44, 55);
	

	INT frameWidth = IMAGE_FRAME_WIDTH;
	if (drawFocus && IMAGE_FRAME_WIDTH > 0)
		frameWidth--;
	
	GLButtonComplex_DrawFrame(hdc, &rcImage, frameWidth, rgbFrame);
	InflateRect(&rcImage, -frameWidth, -frameWidth);

	if (drawFocus && IMAGE_FRAME_WIDTH > 0)
	{
		COLORREF rgbBk, rgbFg;
		rgbBk = SetBkColor(hdc, 0xFFFFFF);
		rgbFg = SetTextColor(hdc, 0x000000);
		DrawFocusRect(hdc, &rcImage);
		if (rgbBk != 0xFFFFFF) SetBkColor(hdc, rgbBk);
		if (rgbFg != 0x000000) SetTextColor(hdc, rgbFg);
		InflateRect(&rcImage, -1, -1);
	}
	

	DIBSECTION ds;
	if (NULL == image || 
		sizeof(DIBSECTION) != GetObject(image, sizeof(DIBSECTION), &ds))
	{
		return FALSE;
	}
		
	StretchDIBits(hdc, rcImage.left, rcImage.bottom - 1, rcImage.right - rcImage.left, rcImage.top - rcImage.bottom, 
							0, 0, ds.dsBm.bmWidth, ds.dsBm.bmHeight, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);

	
	return TRUE;
}

BOOL GLButtonComplex::GetButtonRect(const RECT *itemRect, RECT *buttonRect)
{
	if (NULL == itemRect || NULL == buttonRect)
		return FALSE;
	return SetRect(buttonRect, 
				itemRect->right - buttonWidth - BUTTON_OFFSET_X,
				itemRect->bottom - buttonHeight - BUTTON_OFFSET_Y, 
				itemRect->right - BUTTON_OFFSET_X, itemRect->bottom - BUTTON_OFFSET_Y);
}

BOOL GLButtonComplex::DrawButton(GLStyle *style, HDC hdc, const RECT *prcItem, UINT itemState, BOOL drawFocus)
{
	UINT buttonDrawState = PBS_NORMAL;
	COLORREF rgbFg = style->GetColor(GLStyle::uiColorItemText);

	switch(buttonState & ~FlagButtonDown)
	{
		case FlagButtonDisabled:
			buttonDrawState = PBS_DISABLED;
			rgbFg = style->GetColor(GLStyle::uiColorItemTextDisabled);
			break;

		case FlagButtonPressed:
		case (FlagButtonHot | FlagButtonPressed):
			buttonDrawState = PBS_PRESSED;
			break;

		case FlagButtonHot:
			buttonDrawState = PBS_HOT;
			break;
	}
	
	RECT rcButton;

	TCHAR szBuffer[128];
	WASABI_API_LNGSTRINGW_BUF(IDS_MODIFY, szBuffer, ARRAYSIZE(szBuffer));

	HFONT fontOrig = (HFONT)SelectObject(hdc, style->GetFont(GLStyle::uiFontItem));

	if (0 == buttonWidth)
	{
		SIZE sizeText;
		INT cchText = lstrlen(szBuffer);
		if (cchText > 0 && 
			GetTextExtentPoint32(hdc, szBuffer, cchText, &sizeText))
		{
			buttonWidth = sizeText.cx + 6 * 2;
		}
		if (buttonWidth < 75) buttonWidth = 75;
	}

	if (0 == buttonHeight)
	{
		buttonHeight = style->GetMetrics(GLStyle::uiMetricItemFontCY) + 5 * 2;
		if (buttonHeight < 20) buttonHeight = 20;
	}

	if (!GetButtonRect(prcItem, &rcButton))
		return FALSE;
	
	style->DrawThemeBackground(hdc, BP_PUSHBUTTON, buttonDrawState, &rcButton, NULL);
	INT modeOrig = SetBkMode(hdc, TRANSPARENT);
	COLORREF rgbFgOrig = SetTextColor(hdc, rgbFg);
	
	InflateRect(&rcButton, -6, -5);
	DrawText(hdc, szBuffer, -1, &rcButton, DT_SINGLELINE | DT_LEFT | DT_CENTER);

	if (TRANSPARENT != modeOrig)
		SetBkMode(hdc, TRANSPARENT);
	if (rgbFgOrig != rgbFg)
		SetTextColor(hdc, rgbFgOrig);

	SelectObject(hdc, fontOrig);
	return TRUE;
}


BOOL GLButtonComplex::Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags)
{
	
	INT savedDC = SaveDC(hdc);

	UINT uiColor, uiColorTxt;

	uiColor = (FlagButtonChecked == GetValue()) ? GLStyle::uiColorItemHighlighted : GLStyle::uiColorItem;
	uiColorTxt = GLStyle::uiColorItemText;

	UINT itemState = GetState();
	if (0 != (DrawFlagDisabled & drawFlags))
		itemState |= FlagStateDisabled;

	switch(itemState & ~FlagStateButtonMask)
	{
		case FlagStateDisabled:
			uiColor = GLStyle::uiColorItemDisabled;
			uiColorTxt = GLStyle::uiColorItemTextDisabled;
			break;
	}


	RECT rcPart;
	CopyRect(&rcPart, prcItem);


	if (NULL != parent && 
		(GLGroup::FlagTypeGroup != parent->GetType() || this != ((GLGroup*)parent)->First()))
	{	/* draw horz line */
		INT bottom = rcPart.bottom;
		rcPart.bottom = rcPart.top + 1;
		SetBkColor(hdc, style->GetColor(GLStyle::uiColorGroupLine));
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		rcPart.top = rcPart.bottom;
		rcPart.bottom = bottom;
	}

	COLORREF rgbBk = style->GetColor(uiColor);
	COLORREF rgbFg = style->GetColor(uiColorTxt);
	
	SetBkColor(hdc, rgbBk);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, rgbFg);

	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
	
	DrawImage(style, hdc, &rcPart, itemState, (0 != (DrawFlagFocus & drawFlags)));

	RECT rcText;
	SetRect(&rcText, rcPart.left + IMAGE_OFFSET_X + IMAGE_FRAME_WIDTH * 2 + IMAGE_WIDTH + TITLE_OFFSET_X, rcPart.top + TITLE_OFFSET_Y, rcPart.right, rcPart.bottom);
	rcText.bottom = rcText.top + style->GetMetrics(GLStyle::uiMetricTitleFontCY);
	SelectObject(hdc, style->GetFont(GLStyle::uiFontTitle));
	
	

	DrawText(hdc, pszText, -1, &rcText, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_SINGLELINE);

	if (NULL != description)
	{
		rcText.left += DESCRIPTION_OFFSET_X;
		rcText.top = rcText.bottom + DESCRIPTION_OFFSET_Y;
		rcText.bottom = rcPart.bottom;

		SelectObject(hdc, style->GetFont(GLStyle::uiFontItem));
		DrawText(hdc, description, -1, &rcText, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_WORDBREAK);
	}

	if(FlagButtonChecked == GetValue())
	{
		DrawButton(style, hdc, &rcPart, itemState, (0 != (DrawFlagFocus & drawFlags)));
	}
	
	RestoreDC(hdc, savedDC);
	return TRUE;
}

INT GLButtonComplex::GetHeight(GLStyle *style)
{
	//return __super::GetHeight(style);
	return 120;
}

BOOL GLButtonComplex::AdjustRect(GLStyle *style, RECT *prcItem)
{
	return __super::AdjustRect(style, prcItem);
}

void GLButtonComplex::MouseEnter(GLView *view)
{
}

void GLButtonComplex::MouseLeave(GLView *view)
{
	if (0 != (FlagStateHot & GetState()) || 
		0 != (FlagButtonHot & buttonState))
	{		
		if (0 != (FlagButtonDown & buttonState))
		{
			if (0 != (FlagButtonPressed & buttonState))
				buttonState &= ~FlagButtonPressed;
		}
		else
			buttonState &= ~FlagButtonHot;

		SetFlags(0, FlagStateHot);
		if (NULL != view)
			view->Invalidate(this);
	}
}

void GLButtonComplex::MouseMove(GLView *view, const RECT *prcItem, POINT pt)
{
	UINT itemState = GetState();
	if (0 != (FlagStateDisabled & itemState))
		return;

	BOOL invalidate = FALSE;
	
	if (0 == (FlagStateHot & itemState))
	{
		SetFlags(FlagStateHot, FlagStateHot);
		invalidate = TRUE;	
	}

	if(FlagButtonChecked == GetValue() && 0 == (FlagButtonDisabled & buttonState))
	{
		RECT buttonRect;
		if (GetButtonRect(prcItem, &buttonRect) && PtInRect(&buttonRect, pt))
		{
			if (FlagButtonDown == ((FlagButtonDown | FlagButtonPressed) & buttonState))
			{
				buttonState |= FlagButtonPressed;
				invalidate = TRUE;
			}
			else if (0 == (FlagButtonHot & buttonState))
			{
				buttonState |= FlagButtonHot;
				invalidate = TRUE;
			}
		}
		else
		{
			if (0 != (FlagButtonDown & buttonState))
			{
				if (0 != (FlagButtonPressed & buttonState))
				{
					buttonState &= ~FlagButtonPressed;
					invalidate = TRUE;
				}
			}
			else
			{
				if (0 != (FlagButtonHot & buttonState))
				{
					buttonState &= ~FlagButtonHot;
					invalidate = TRUE;
				}
			}
		}
	}

	if (FALSE != invalidate && NULL != view)
			view->Invalidate(this);
}

BOOL GLButtonComplex::LButtonDown(GLView *view, const RECT *prcItem, POINT pt)
{
	UINT itemState = GetState();
	if (0 != (FlagStateDisabled & itemState))
		return FALSE;

	if(FlagButtonChecked == GetValue() && 0 == (FlagButtonDisabled & buttonState))
	{
		RECT buttonRect;
		if (GetButtonRect(prcItem, &buttonRect) && PtInRect(&buttonRect, pt))
		{
			if (0 == (FlagButtonPressed & buttonState))
			{
				buttonState |= (FlagButtonPressed | FlagButtonDown);
				if (NULL != view)
					view->Invalidate(this);
			}
			return TRUE;
		}
	}
	
	if (0 == (FlagStatePressed & itemState))
	{
		SetFlags(FlagStatePressed, FlagStatePressed);
		MarkRadioChecked(view);
		if (NULL != view)
			view->Invalidate(this);
	}
	
	return TRUE;

}

void GLButtonComplex::LButtonUp(GLView *view, const RECT *prcItem, POINT pt)
{	
	UINT itemState = GetState();

	if(FlagButtonChecked == GetValue() && 
		0 != ((FlagButtonPressed | FlagButtonDown) & buttonState))
	{
		RECT buttonRect;
		if (GetButtonRect(prcItem, &buttonRect) && PtInRect(&buttonRect, pt))
		{
			NMGLITEM item;
			item.hdr.code = NM_CLICK;
			item.hdr.hwndFrom = view->GetHost();
			item.hdr.idFrom = GetDlgCtrlID(item.hdr.hwndFrom);
			item.item = this;
			item.styleNew = 0;
			item.styleOld = 0;
			HWND hParent = (NULL != item.hdr.hwndFrom) ? ::GetParent(item.hdr.hwndFrom) : NULL;
			if (NULL != hParent)
				SendMessage(hParent, WM_NOTIFY, (WPARAM)item.hdr.idFrom, (LPARAM)&item);
			// buttton click'
		}

		buttonState &= ~(FlagButtonPressed | FlagButtonDown);
		if (NULL != view)
			view->Invalidate(this);
		return;
	}

	if (0 != (FlagStatePressed & itemState))
	{
		SetFlags(0, FlagStatePressed);
		if (NULL != view)
			view->Invalidate(this);
	}
}

void GLButtonComplex::LButtonClick(GLView *view)
{	
}

void GLButtonComplex::KeyDown(GLView *view, UINT vKey)
{
	if (VK_SPACE == vKey)
	{
		if (0 == (FlagStatePressed & GetState()))
		{
			SetFlags(FlagStatePressed, FlagStatePressed);
			MarkRadioChecked(view);
			if (NULL != view)
				view->Invalidate(this);
		}
	}
}

void GLButtonComplex::KeyUp(GLView *view, UINT vKey)
{
	if (0 != (FlagStatePressed & GetState()))
	{
		SetFlags(0, FlagStatePressed);
		if (NULL != view)
				view->Invalidate(this);
	}
}

void GLButtonComplex::KeyPressed(GLView *view, UINT vKey)
{
	
}

void GLButtonComplex::StyleChanged(GLView *view, UINT updateFlags)
{
}
