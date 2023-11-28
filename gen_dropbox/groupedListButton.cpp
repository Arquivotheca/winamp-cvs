#include "./groupedList.h"
#include <tmschema.h>


GLButton::GLButton(INT itemId, LPCTSTR itemTitle, UINT itemFlags) :
	GLItem(itemId, itemTitle, itemFlags), textWidth(-1)
{
}

GLButton::~GLButton()
{
}

GLButton *GLButton::CreateRadiobutton(INT itemId, LPCTSTR itemTitle, BOOL checked)
{
	GLButton *button = new GLButton(itemId, itemTitle, 
		FlagTypeRadiobutton | ((FALSE != checked) ? FlagButtonChecked : FlagButtonUnchecked));
	return button;
}

BOOL GLButton::GetRadioBtnRect(GLStyle *style, const RECT *itemRect, RECT *buttonRect)
{
	if (NULL == style || NULL == itemRect || NULL == buttonRect)
		return FALSE;
	buttonRect->top = itemRect->top;
	buttonRect->bottom = itemRect->bottom;
	buttonRect->left = itemRect->left;
	buttonRect->right = buttonRect->left + 
						radioButtonMarginLeft + radioButtonMarginRight +
						style->GetMetrics(GLStyle::uiMetricRadiobuttonCX) + 
						radioButtonTextOffset + textWidth + radioButtonHotTail;
	
	if (buttonRect->right > itemRect->right)
		buttonRect->right = itemRect->right;

	return TRUE;
}

BOOL GLButton::Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags)
{
	INT cchText = (NULL != pszText) ? lstrlen(pszText) : 0;
			
	INT savedDC = SaveDC(hdc);

	UINT uiColor, uiColorTxt;
	INT buttonState;

	uiColor = GLStyle::uiColorItem;
	uiColorTxt = GLStyle::uiColorItemText;
		
	SelectObject(hdc, style->GetFont(GLStyle::uiFontItem));

	if (-1 == textWidth)
	{
		SIZE sizeText;
		if (cchText > 0 && GetTextExtentPoint32(hdc, pszText, cchText, &sizeText))
			textWidth = sizeText.cx;
		else 
			textWidth = 0;
	}

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

	SetBkColor(hdc, style->GetColor(uiColor));
	SetTextColor(hdc, style->GetColor(uiColorTxt));

	RECT rcPart;
	CopyRect(&rcPart, prcItem);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);

	rcPart.right = rcPart.left + style->GetMetrics(GLStyle::uiMetricRadiobuttonCX);
	rcPart.bottom -= radioButtonMarginBottom;
	INT top = rcPart.bottom - style->GetMetrics(GLStyle::uiMetricRadiobuttonCY);
	if (top > rcPart.top) rcPart.top = top;

	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	style->DrawThemeBackground(hdc, BP_RADIOBUTTON, buttonState, &rcPart, NULL);


	SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
	if (0 != cchText)
	{
		rcPart.left = rcPart.right + radioButtonTextOffset;
		rcPart.top = prcItem->top + radioButtonMarginTop;
		rcPart.right = prcItem->right - radioButtonMarginRight;
		rcPart.bottom = prcItem->bottom - radioButtonMarginBottom;
		
		ExtTextOut(hdc, rcPart.left, rcPart.bottom, ETO_CLIPPED, &rcPart, pszText, cchText, NULL);
		if (0 != (DrawFlagFocus & drawFlags))
		{		
			if (textWidth > 0)
			{
				rcPart.top = rcPart.bottom - style->GetMetrics(GLStyle::uiMetricItemFontCY);
				rcPart.right = rcPart.left + textWidth;
				InflateRect(&rcPart, 2, 1);
				if (rcPart.top < prcItem->top) rcPart.top = prcItem->top;
				if (rcPart.right >= prcItem->right) rcPart.right = prcItem->right - 1;
				DrawFocusRect(hdc, &rcPart);
			}
		}
	}
	
	RestoreDC(hdc, savedDC);
	
	return TRUE;
}

void GLButton::MarkRadioChecked(GLView *view)
{
	if (FlagButtonUnchecked == GetValue())
	{
		SetFlags(FlagButtonChecked, FlagButtonChecked);
		if (NULL != view)
			view->Invalidate(this);

		if (NULL != parent && GLGroup::FlagTypeGroup == parent->GetType())
		{
			GLItem *child;
			GLGroup *group = (GLGroup*)parent;
			size_t count = group->GetDirectChildCount();
			
			for (size_t i = 0; i < count; i++)
			{
				child = group->GetChild(i);
				if (child != this && 
					FlagTypeRadiobutton == child->GetType() &&
					FlagButtonUnchecked != child->GetValue())
				{
					child->SetFlags(0, (FlagButtonChecked | FlagButtonMixed));
					if (NULL != view)
						view->Invalidate(child);
				}
			}
		}
	}
}

INT GLButton::GetHeight(GLStyle *style)
{
	INT textCY = style->GetMetrics(GLStyle::uiMetricItemFontCY);
	INT imageCY = style->GetMetrics(GLStyle::uiMetricRadiobuttonCY);
	INT h = (textCY > imageCY) ? textCY : imageCY;
	h += radioButtonMarginTop + radioButtonMarginBottom;
	return h;
}

BOOL GLButton::AdjustRect(GLStyle *style, RECT *prcItem)
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




void GLButton::OnEnter(GLView *view)
{
	UINT itemState = GetState();
	BOOL invalidate = FALSE;

	if (FlagStateLButton == ((FlagStateLButton | FlagStatePressed) & itemState))
	{
		SetFlags(FlagStatePressed, FlagStatePressed);
		invalidate = TRUE;
	}
	else if (0 == (FlagStateHot & itemState))
	{
		SetFlags(FlagStateHot, FlagStateHot);
		invalidate = TRUE;
	}

	if (FALSE != invalidate && NULL != view)
		view->Invalidate(this);

}
void GLButton::OnLeave(GLView *view)
{
	UINT itemState = GetState();
	BOOL invalidate = FALSE;

	if (0 != (FlagStateLButton & itemState))
	{
		if (0 != (FlagStatePressed & itemState))
		{
			SetFlags(0, FlagStatePressed);
			invalidate = TRUE;
		}
	}
	else if (0 != (FlagStateHot & itemState))
	{		
		SetFlags(0, FlagStateHot);
		invalidate = TRUE;
	}

	if (FALSE != invalidate && NULL != view)
		view->Invalidate(this);
}

void GLButton::OnButtonDown(GLView *view, UINT mouseButton)
{
	UINT test = (FlagStateButtonMask & mouseButton) | FlagStatePressed;
	
	if (test != (test &  GetState()))
	{
		SetFlags(test,test);
		if (NULL != view)
			view->Invalidate(this);
	}
}

void GLButton::OnButtonUp(GLView *view, UINT mouseButton)
{
	mouseButton &= FlagStateButtonMask;
	UINT test = mouseButton | FlagStatePressed;

	if (0 != (test & GetState()))
	{
		SetFlags(0, test);
		if (NULL != view)
			view->Invalidate(this);
	}
}

void GLButton::MouseEnter(GLView *view)
{
	if (GLButton::FlagTypePushbutton == GetType())
	{
		OnEnter(view);
	}
	else
	{
		// do nothing
	}
}

void GLButton::MouseLeave(GLView *view)
{
	OnLeave(view);
}

void GLButton::MouseMove(GLView *view, const RECT *prcItem, POINT pt)
{
	UINT itemState = GetState();
	RECT buttonRect;

	if (0 == (FlagStateDisabled & itemState) &&
		GetRadioBtnRect(view->GetStyle(), prcItem, &buttonRect) && PtInRect(&buttonRect, pt))
	{
		OnEnter(view);
	}
	else
	{
		OnLeave(view);
	}
}

BOOL GLButton::LButtonDown(GLView *view, const RECT *prcItem, POINT pt)
{
	if (0 != (FlagStateDisabled & GetState()))
		return FALSE;

	RECT buttonRect;
	if (GetRadioBtnRect(view->GetStyle(), prcItem, &buttonRect) && PtInRect(&buttonRect, pt))
	{		
		OnButtonDown(view, FlagStateLButton);
		return TRUE;
	}

	return FALSE;
}

void GLButton::LButtonUp(GLView *view, const RECT *prcItem, POINT pt)
{
	OnButtonUp(view, FlagStateLButton);
}

void GLButton::LButtonClick(GLView *view)
{
	if (0 == (FlagStateDisabled & GetState()))
		MarkRadioChecked(view);
}


void GLButton::KeyDown(GLView *view, UINT vKey)
{
	OnButtonDown(view, 0);
}

void GLButton::KeyUp(GLView *view, UINT vKey)
{
	OnButtonUp(view, 0);
}

void GLButton::KeyPressed(GLView *view, UINT vKey)
{
	LButtonClick(view);
}

void GLButton::StyleChanged(GLView *view, UINT updateFlags)
{
	if (0 != (GLStyle::updateFlagFonts & updateFlags))
		textWidth = -1;
}