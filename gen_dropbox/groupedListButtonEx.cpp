#include "./groupedList.h"


#include <tmschema.h>
#include <strsafe.h>


GLButtonEx::GLButtonEx(INT itemId, LPCTSTR itemTitle, LPCTSTR itemDescription, UINT itemFlags) :
	GLButton(itemId, itemTitle, itemFlags), description(NULL)
{
	
	if (NULL != itemDescription)
	{
		INT cchText = lstrlen(itemDescription);
		cchText++;
		description = (LPTSTR)malloc(sizeof(TCHAR) * (cchText));
		if (NULL != description)
			StringCchCopy(description, cchText, itemDescription);
	}
}

GLButtonEx::~GLButtonEx()
{
	if (NULL != description)
		free(description);
}

GLButtonEx *GLButtonEx::CreateRadiobutton(INT itemId, LPCTSTR itemTitle, LPCTSTR itemDescription, BOOL checked)
{
	UINT itemFlags = FlagTypeRadiobutton | ((FALSE != checked) ? FlagButtonChecked : FlagButtonUnchecked);
	GLButtonEx *button = new GLButtonEx(itemId, itemTitle, itemDescription, itemFlags);
	return button;
}

BOOL GLButtonEx::Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags)
{
	
	INT cchText = (NULL != pszText) ? lstrlen(pszText) : 0;
			
	INT savedDC = SaveDC(hdc);

	UINT uiColor, uiColorTxt;
	INT buttonState;

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
			buttonState = (FlagButtonChecked == GetValue()) ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
			break;
		
		case FlagStateHot:
			buttonState = (FlagButtonChecked == GetValue()) ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
			break;

		case FlagStatePressed:
		case (FlagStateHot | FlagStatePressed):
			buttonState = (FlagButtonChecked == GetValue()) ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
			break;

		default:
			buttonState = (FlagButtonChecked == GetValue()) ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
			break;
	}


	RECT rcPart;
	CopyRect(&rcPart, prcItem);

	if (NULL != parent && 
		(GLGroup::FlagTypeGroup != parent->GetType() || this != ((GLGroup*)parent)->First()))
	{
		INT bottom = rcPart.bottom;
		rcPart.bottom = rcPart.top + 1;
		SetBkColor(hdc, style->GetColor(GLStyle::uiColorGroupLine));
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		rcPart.top = rcPart.bottom;
		rcPart.bottom = bottom;
	}

	SetBkColor(hdc, style->GetColor(uiColor));
	SetTextColor(hdc, style->GetColor(uiColorTxt));

	

	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);

	rcPart.left += radioButtonMarginLeft;
	rcPart.right = rcPart.left + style->GetMetrics(GLStyle::uiMetricRadiobuttonCX);
	rcPart.bottom -= (radioButtonMarginBottom);
	
	style->DrawThemeBackground(hdc, BP_RADIOBUTTON, buttonState, &rcPart, NULL);


	SetTextAlign(hdc, TA_LEFT | TA_TOP);

	rcPart.left = rcPart.right + radioButtonTextOffset;
	rcPart.top = prcItem->top + radioButtonMarginTop;
	rcPart.right = prcItem->right - radioButtonMarginRight;
	rcPart.bottom = rcPart.top  + style->GetMetrics(GLStyle::uiMetricTitleFontCY);

	if (0 != cchText)
	{				
		SelectObject(hdc, style->GetFont(GLStyle::uiFontTitle));
		ExtTextOut(hdc, rcPart.left, rcPart.top, ETO_CLIPPED, &rcPart, pszText, cchText, NULL);
	}
	
	if (NULL != description)
	{
		rcPart.left += 8;
		rcPart.top = rcPart.bottom;
		rcPart.bottom = prcItem->bottom - radioButtonMarginBottom;

		SelectObject(hdc, style->GetFont(GLStyle::uiFontItem));
		DrawText(hdc, description, -1, &rcPart, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_WORDBREAK);
	}

	if (0 != (DrawFlagFocus & drawFlags))
	{
		CopyRect(&rcPart, prcItem);
		rcPart.left += radioButtonMarginLeft + style->GetMetrics(GLStyle::uiMetricRadiobuttonCX) +
						radioButtonTextOffset/2;
		rcPart.top	+= radioButtonMarginTop/2;
		rcPart.right -= radioButtonMarginRight/2;
		rcPart.bottom -= radioButtonMarginBottom/2;
		DrawFocusRect(hdc, &rcPart);
	}

	RestoreDC(hdc, savedDC);
	
	return TRUE;
}

INT GLButtonEx::GetHeight(GLStyle *style)
{
	INT titleCY = style->GetMetrics(GLStyle::uiMetricTitleFontCY);
	INT textCY = style->GetMetrics(GLStyle::uiMetricItemFontCY);
	INT imageCY = style->GetMetrics(GLStyle::uiMetricRadiobuttonCY);
	INT h = titleCY + 3 * textCY;
	if (h < imageCY) h = imageCY;

	h += radioButtonMarginTop + radioButtonMarginBottom;
	
	return h;
}

BOOL GLButtonEx::AdjustRect(GLStyle *style, RECT *prcItem)
{
	if (NULL == prcItem)
		return FALSE;

	prcItem->left += style->GetMetrics(GLStyle::uiMetricLevelOffset) * GetLevel() + 2;
	prcItem->right -= 2;
	if (prcItem->left > prcItem->right)
		prcItem->left = prcItem->right;

	INT h = GetHeight(style);
	if ((prcItem->bottom - prcItem->top) > h)
		prcItem->top = prcItem->bottom  - h;

	return TRUE;
}

void GLButtonEx::MouseEnter(GLView *view)
{
	OnEnter(view);
}

void GLButtonEx::MouseLeave(GLView *view)
{
	OnLeave(view);
}

void GLButtonEx::MouseMove(GLView *view, const RECT *prcItem, POINT pt)
{

}

BOOL GLButtonEx::LButtonDown(GLView *view, const RECT *prcItem, POINT pt)
{
	if (0 != (FlagStateDisabled & GetState()))
		return FALSE;
			
	OnButtonDown(view, FlagStateLButton);
	return TRUE;
}

void GLButtonEx::LButtonUp(GLView *view, const RECT *prcItem, POINT pt)
{
	OnButtonUp(view, FlagStateLButton);
}

void GLButtonEx::LButtonClick(GLView *view)
{
	if (0 == (FlagStateDisabled & GetState()))
		MarkRadioChecked(view);
}

