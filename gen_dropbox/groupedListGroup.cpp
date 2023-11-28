#include "./groupedList.h"

#define GROUP_MARGIN_LEFT	4
#define GROUP_MARGIN_TOP		3
#define GROUP_MARGIN_RIGHT	2
#define GROUP_MARGIN_BOTTOM	3


static void SetVertex(TRIVERTEX *pVertex, const POINT *pt, COLORREF rgb)
{
	pVertex->x = pt->x;
	pVertex->y = pt->y;
	pVertex->Red    = GetRValue(rgb) << 8;
	pVertex->Green  = GetGValue(rgb) << 8;
	pVertex->Blue   = GetBValue(rgb) << 8;
	pVertex->Alpha  = 0x0000;
}


GLGroup::GLGroup(INT groupId,  INT iconId, LPCTSTR pszTitle, UINT groupFlags) : 
	GLItem(groupId, pszTitle, (groupFlags & ~FlagTypeMask) | FlagTypeGroup)
{
	this->iconId = iconId;
}

GLGroup::~GLGroup()
{
	size_t index = children.size();
	while(index--)
	{
		GLItem *child = children[index];
		child->Release();
	}

	children.clear();
}


GLGroup *GLGroup::CreateInstance(INT groupId, INT iconId, LPCTSTR pszTitle, UINT groupFlags)
{
	return new GLGroup(groupId, iconId, pszTitle, groupFlags);
}

GLItem *GLGroup::GetChild(size_t index)
{
	return (index < children.size()) ? children[index] : NULL;
}

size_t GLGroup::InsertItem(size_t index, GLItem *item)
{
	if (NULL == item)
		return ((size_t)-1);

	item->AddRef();
	item->parent = this;

	if (index >= children.size())
	{
		index = children.size();
		children.push_back(item);
	}
	else
		children.insertBefore(index, item);
	
	return index;
}

size_t GLGroup::GetDirectChildCount()
{
	return children.size();
}

size_t GLGroup::GetVisibleChildCount()
{
	size_t c = children.size();
	size_t i = c;
	UINT itemFlags;
	while(i--)
	{
		itemFlags = children[i]->GetFlags();
		if (FlagTypeGroup == (FlagTypeMask & itemFlags) &&
			FlagGroupCollapsed != (FlagValueMask & itemFlags))
		{
			c += ((GLGroup*)children[i])->GetVisibleChildCount();
		}
	}
	return c;
}

BOOL GLGroup::LookupItem(size_t flatIndex, size_t *parentIndex, GLItem **itemOut)
{
	size_t base = *parentIndex;
	UINT itemFlags;
	for (size_t i = 0; i < children.size(); i++)
	{
		if (flatIndex == base)
		{
			if (NULL != itemOut)
				*itemOut = children[i];
			return TRUE;
		}		
		base++;

		itemFlags = children[i]->GetFlags();
		if (FlagTypeGroup == (FlagTypeMask & itemFlags) &&
			FlagGroupCollapsed != (FlagValueMask & itemFlags))
		{			
			if (((GLGroup*)children[i])->LookupItem(flatIndex, &base, itemOut))
				return TRUE;
		}
	}

	*parentIndex = base;
	return FALSE;
}

BOOL GLGroup::LookupIndex(const GLItem *item, size_t *index)
{	
	UINT itemFlags;
	for (size_t i = 0; i < children.size(); i++)
	{
		if (item == children[i])
			return TRUE;
			
		(*index)++;

		itemFlags = children[i]->GetFlags();
		if (FlagTypeGroup == (FlagTypeMask & itemFlags) &&
			FlagGroupCollapsed != (FlagValueMask & itemFlags) &&
			((GLGroup*)children[i])->LookupIndex(item, index))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GLGroup::EnumerateChidlren(ChildEnumProc proc, ULONG_PTR user)
{
	if (NULL == proc)
		return FALSE;

	UINT itemFlags;
	for (size_t i = 0; i < children.size(); i++)
	{
		GLItem *item = children[i];
		if (!proc(item, user))
			return FALSE;

		itemFlags = children[i]->GetFlags();
		if (FlagTypeGroup == (FlagTypeMask & itemFlags) &&
			FlagGroupCollapsed != (FlagValueMask & itemFlags) &&
			!((GLGroup*)item)->EnumerateChidlren(proc, user))
		{
			return FALSE;
		}
	}

	return TRUE;
}

GLItem *GLGroup::First()
{	
	return (0 != children.size()) ? children[0] : NULL;
}

GLItem *GLGroup::Last()
{
	size_t count = children.size();
	return (0 != count) ? children[count - 1] : NULL;
}

GLItem *GLGroup::NextChild(GLItem *item)
{
	if (children.size() > 1)
	{
		size_t count = children.size() - 1;
		for (size_t i = 0; i < count; i++)
		{
			if (item == children[i])
				return children[i + 1];
		}
	}
	return NULL;
}

GLItem *GLGroup::PreviousChild(GLItem *item)
{
	if (NULL == item)
		return NULL;

	for (size_t i = children.size() - 1; i > 0; i--)
	{
		if (item == children[i])
			return children[i - 1];
	}

	return NULL;
}



BOOL GLGroup::Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags)
{
	INT cchText = (NULL != pszText) ? lstrlen(pszText) : 0;
			
	INT savedDC = SaveDC(hdc);
	
	COLORREF rgbBkL, rgbBkR;
	rgbBkL = style->GetColor(GLStyle::uiColorGroup);
	rgbBkR = style->GetColor(GLStyle::uiColorGroupRight);
			
	
	RECT rcPart;
	CopyRect(&rcPart, prcItem);

	/* draw line */
	{
		LONG top = rcPart.top;
		rcPart.top = rcPart.bottom - 1;
		SetBkColor(hdc, style->GetColor(GLStyle::uiColorGroupLine));
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		rcPart.top = top;
		rcPart.bottom -= 1;
	}
			
	if (rgbBkL != rgbBkR)
	{
		TRIVERTEX szVertex[2];
		GRADIENT_RECT gradientRect;
		SetVertex(&szVertex[0], ((POINT*)&rcPart), rgbBkL);
		SetVertex(&szVertex[1], ((POINT*)&rcPart) + 1, rgbBkR);
		gradientRect.UpperLeft  = 0;
		gradientRect.LowerRight = 1;
		if (!GradientFill(hdc, szVertex, ARRAYSIZE(szVertex), &gradientRect, 1, GRADIENT_FILL_RECT_V))
			rgbBkR = rgbBkL;
	}

	if (rgbBkL == rgbBkR)
	{
		SetBkColor(hdc, style->GetColor(GLStyle::uiColorGroup));
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
	}

	
	rcPart.left += GROUP_MARGIN_LEFT;
	rcPart.top	+= GROUP_MARGIN_TOP;
	rcPart.bottom -= GROUP_MARGIN_BOTTOM;
	rcPart.right -= GROUP_MARGIN_RIGHT;

	if (iconId >= 0)
	{
		RECT rcIcon;
		rcIcon.left = rcPart.left;
		rcIcon.top = rcPart.top + ((rcPart.bottom - rcPart.top) - style->GetMetrics(GLStyle::uiMetricIconCY)) / 2;
		rcIcon.right = rcIcon.left + style->GetMetrics(GLStyle::uiMetricIconCX);
		rcIcon.bottom = rcIcon.top + style->GetMetrics(GLStyle::uiMetricIconCY);
		
		if (rcIcon.right > rcPart.right) rcIcon.right = rcPart.right;
		if (rcIcon.top < rcPart.top) rcIcon.top = rcPart.top;
		if (rcIcon.bottom > rcPart.bottom) rcIcon.bottom = rcPart.bottom;
		
		rcIcon.top = rcIcon.bottom - style->GetMetrics(GLStyle::uiMetricIconCY);
		if (rcIcon.top < rcPart.top) rcIcon.top = rcPart.top;

		if (rcIcon.top != rcIcon.bottom && rcIcon.left != rcIcon.right
			&& style->DrawIcon(hdc, iconId, &rcIcon))
		{
			rcPart.left = rcIcon.right + GROUP_MARGIN_LEFT;
		}
		
	}
	if (0 != cchText)
	{			
		SelectObject(hdc, style->GetFont(GLStyle::uiFontGroup));
		SetBkMode(hdc, TRANSPARENT);
		SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
		
		
	
		COLORREF rgbText, rgbTextShadow;
		rgbText = style->GetColor(GLStyle::uiColorGroupText);
		rgbTextShadow = style->GetColor(GLStyle::uiColorGroupTextShadow);
		if (rgbTextShadow != rgbText)
		{
			SetTextColor(hdc, rgbTextShadow);
			rcPart.bottom += 1;
			ExtTextOut(hdc, rcPart.left + 1, rcPart.bottom, ETO_CLIPPED, &rcPart, pszText, cchText, NULL);
			rcPart.bottom -= 1;
		}

		SetTextColor(hdc, rgbText);
		ExtTextOut(hdc, rcPart.left, rcPart.bottom, ETO_CLIPPED, &rcPart, pszText, cchText, NULL);
	}
	
	RestoreDC(hdc, savedDC);
	return TRUE;
}

INT GLGroup::GetHeight(GLStyle *style)
{
	INT textHeight = style->GetMetrics(GLStyle::uiMetricGroupFontCY);
	INT iconHeight = style->GetMetrics(GLStyle::uiMetricIconCY);
	INT h = (textHeight > iconHeight) ? textHeight : iconHeight;
	h += GROUP_MARGIN_BOTTOM + GROUP_MARGIN_TOP;
	
	return h;
}

BOOL GLGroup::AdjustRect(GLStyle *style, RECT *prcItem)
{
	if (NULL == prcItem)
		return FALSE;

	prcItem->left += style->GetMetrics(GLStyle::uiMetricLevelOffset) * GetLevel() + 2;
	if (prcItem->left > prcItem->right)
		prcItem->left = prcItem->right;

	INT h = GetHeight(style);
	if ((prcItem->bottom - prcItem->top) > h)
		prcItem->top = prcItem->bottom  - h;

	return TRUE;
}
void GLGroup::MouseEnter(GLView *view)
{
}
void GLGroup::MouseLeave(GLView *view)
{
}
void GLGroup::MouseMove(GLView *view, const RECT *prcItem, POINT pt)
{
}
BOOL GLGroup::LButtonDown(GLView *view, const RECT *prcItem, POINT pt)
{
	return TRUE;
}
void GLGroup::LButtonUp(GLView *view, const RECT *prcItem, POINT pt)
{
}
void GLGroup::LButtonClick(GLView *view)
{
}

void GLGroup::KeyDown(GLView *view, UINT vKey)
{
}

void GLGroup::KeyUp(GLView *view, UINT vKey)
{
}

void GLGroup::KeyPressed(GLView *view, UINT vKey)
{
}

void GLGroup::StyleChanged(GLView *view, UINT updateFlags)
{
}

static int __cdecl GLGroup_SortComparer(const void *elem1, const void *elem2)
{
	return CompareString(LOCALE_USER_DEFAULT, 0, 
		(*((GLItem**)elem1))->pszText, -1, (*((GLItem**)elem2))->pszText, -1) - 2;
}

void GLGroup::Sort()
{
	if (children.size() < 2)
		return;

	qsort(children.begin(), children.size(), sizeof(GLItem*), GLGroup_SortComparer);

}
