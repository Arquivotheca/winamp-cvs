#include "./main.h"
#include "./meterbar.h"
#include "./skinWindow.h"
#include "./document.h"
#include "./formatData.h"
#include "./guiObjects.h"
#include "./itemView.h"
#include "./meterbarCallback.h"
#include "./profile.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include <strsafe.h>

#define METERBAR_LINEHEIGHT		2

const LPCTSTR Meterbar::szUnitName[] = 
{
	TEXT("Count"),
	TEXT("Length"),
	TEXT("Size"),
};

// {F9622BD3-478B-4818-A071-C716FC8DB73F}
EXTERN_C const GUID meterbarSettingsGuid = 
{ 0xf9622bd3, 0x478b, 0x4818, { 0xa0, 0x71, 0xc7, 0x16, 0xfc, 0x8d, 0xb7, 0x3f } };

#define CONFIGITEM_STR(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_STRING, ((LPCTSTR)(__defaultValue))}
#define CONFIGITEM_INT(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_INT, ((LPCTSTR)(INT_PTR)(__defaultValue))}

static ConfigIniSection::CONFIGITEM  meterbarSettings[] = 
{
	CONFIGITEM_STR(CFG_METERUNIT, TEXT("unit"), Meterbar_GetUnitName(Meterbar::FlagUnitCount)), 
};


void STDMETHODCALLTYPE Meterbar_RegisterConfig(ConfigurationManager *pcm)
{
	HRESULT hr;
	ConfigIniSection *pConfig;
	
	hr = ConfigIniSection::CreateConfig(meterbarSettingsGuid, FILEPATH_PROFILEINI, TEXT("Meterbar"), meterbarSettings, ARRAYSIZE(meterbarSettings), &pConfig);
	if (SUCCEEDED(hr)){ pcm->Add(pConfig); pConfig->Release(); }
}

static BOOL GetDocumentMetrics(Document *document, UINT meterbarUnit, ULONGLONG *totalOut, size_t *totalUnknownOut)
{
	Document::METRICS metrics;
	metrics.cbSize = sizeof(Document::METRICS);

	ULONGLONG total = 0;
	size_t totalUnknown = 0;

	if (NULL != document)
	{
		switch(meterbarUnit)
		{
			case Meterbar::FlagUnitCount:
				total = document->GetItemCount();
				break;
			case Meterbar::FlagUnitLength:
				metrics.flags = Document::FlagMetricLength;
				if (document->GetMetrics(-1, -1, &metrics))
				{
					total = metrics.length;
					totalUnknown = metrics.unknownData;
				}
				break;
			case Meterbar::FlagUnitSize:
				metrics.flags = Document::FlagMetricSize;
				if (document->GetMetrics(-1, -1, &metrics))
				{
					total = metrics.size;
				}
				break;
		}
	}

	BOOL modified = FALSE;
	if (NULL != totalOut && *totalOut != total)
	{
		*totalOut = total;
		modified = TRUE;
	}
	if (NULL != totalUnknownOut && *totalUnknownOut != totalUnknown)
	{
		*totalUnknownOut = totalUnknown;
		modified = TRUE;
	}

	return modified;
}

static BOOL GetViewMetrics(DropboxView *view, UINT meterbarUnit, ULONGLONG *selectedOut, size_t *selectedUnknownOut)
{
	Document::METRICS metrics;
	metrics.cbSize = sizeof(Document::METRICS);

	ULONGLONG selected = 0;
	size_t selectedCount = 0;
	size_t selectedUnknown = 0;

	if (NULL != view)
	{
		switch(meterbarUnit)
		{
			case Meterbar::FlagUnitCount:
				if (SUCCEEDED(view->GetSelectionCount(&selectedCount)))
					selected = selectedCount;
				break;
			case Meterbar::FlagUnitLength:
				metrics.flags = Document::FlagMetricLength;
				if (SUCCEEDED(view->GetSelectionMetrics(&metrics)))
				{
					selected = metrics.length;
					selectedUnknown = metrics.unknownData;
				}
				break;
			case Meterbar::FlagUnitSize:
				metrics.flags = Document::FlagMetricSize;
				if (SUCCEEDED(view->GetSelectionMetrics(&metrics)))
				{
					selected = metrics.size;
				}
				break;
		}
	}

	BOOL modified = FALSE;
	if (NULL != selectedOut && *selectedOut != selected)
	{
		*selectedOut = selected;
		modified = TRUE;
	}
	if (NULL != selectedUnknownOut && *selectedUnknownOut != selectedUnknown)
	{
		*selectedUnknownOut = selectedUnknown;
		modified = TRUE;
	}

	return modified;

}



static LPCTSTR FormatBarValue(LPTSTR pszBuffer, INT cchBufferMax, UINT meterbarUnit, ULONGLONG value, BOOL bApproximate)
{
	LPTSTR p = pszBuffer;

	if (NULL == pszBuffer || cchBufferMax < 1)
		return NULL;

	if (bApproximate)
	{
		*p = TEXT('~');
		p++;
		cchBufferMax--;
	}

	switch(meterbarUnit)
	{
		case Meterbar::FlagUnitCount:
			StringCchPrintf(p, cchBufferMax, TEXT("%d"), value);
			break;
		case Meterbar::FlagUnitLength:
			FormatLength((INT)value, p, cchBufferMax);
			break;
		case Meterbar::FlagUnitSize:
			FormatFileSize(&value, p, cchBufferMax);
			break;
		default:
			*p = TEXT('\0');
			break;
	}

	return pszBuffer;
}


Meterbar::Meterbar(UINT meterbarFlags) 
	: flags(meterbarFlags), limit(0), total(0), selected(0), 
		totalUnknown(0), selectedUnknown(0), hbmp(NULL), callback(NULL)
{
	SetBoundsIndirect(NULL);
	InitializeCriticalSection(&lockInvalidate);
}

Meterbar::~Meterbar()
{

	if (NULL != hbmp)
		DeleteObject(hbmp);
	
	if (NULL != callback)
		callback->OnDestroy(this);

	DeleteCriticalSection(&lockInvalidate);
}

UINT Meterbar::ParseUnit(LPCTSTR pszBuffer)
{
	if (NULL == pszBuffer)
		return 0;

	while (TEXT('\0') != *pszBuffer && TEXT(' ') == *pszBuffer) pszBuffer++;
	INT cchLen = lstrlen(pszBuffer);

	while(cchLen > 0  && TEXT(' ') == pszBuffer[cchLen - 1]) cchLen--;
	if (0 == cchLen)
		return 0;

	for (INT i = 0; i < ARRAYSIZE(szUnitName); i++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
								pszBuffer, cchLen, szUnitName[i], -1))
		{
			return (i << 16);
		}
	}

	return 0;
	
}

Meterbar *Meterbar::Load(Profile *profile)
{	
	if (NULL == profile)
		return NULL;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(meterbarSettingsGuid, &pConfig);
	if (FAILED(hr))
		return NULL;

	TCHAR szBuffer[512];
	Meterbar *instance = NULL;
	
	if (SUCCEEDED(pConfig->ReadString(CFG_METERUNIT, szBuffer, ARRAYSIZE(szBuffer))))
	{
		UINT unitMode = ParseUnit(szBuffer);
		instance = new Meterbar(Meterbar::FlagVisible | unitMode);
	}
	
	pConfig->Release();
	return instance;
}

HRESULT Meterbar::Save(Profile *profile)
{
	if (NULL == profile)
		return E_POINTER;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(meterbarSettingsGuid, &pConfig);
	if (FAILED(hr))
		return hr;

	hr = pConfig->WriteString(CFG_METERUNIT, Meterbar_GetUnitName(flags));

	pConfig->Release();

	return hr;
}

void Meterbar::SetFlags(UINT newFlags, UINT flagsMask)
{	
	flags &= ~flagsMask;
	flags |= (newFlags & flagsMask);
}

void Meterbar::SetLimit(ULONGLONG limit)
{
	this->limit = limit;
}


INT Meterbar::GetPrefferedHeight(HWND hHost)
{	
	textHeight = 0;

	HDC hdc = GetDCEx(hHost, NULL, DCX_CACHE);
	if (NULL != hdc)
	{
		HFONT hfo, hf;
		TEXTMETRIC tm;

		hf = GetPluginFont(PLUGINFONT_METERBARTEXT);
		if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

		if (GetTextMetrics(hdc, &tm))
		{
			textHeight = tm.tmAscent - 2;
		}

		if (NULL != hfo)
			SelectObject(hdc, hfo);

		ReleaseDC(hHost, hdc);
	}
		
	//INT height = textHeight + 4;
	INT height = 2 + textHeight/2 + METERBAR_LINEHEIGHT/2 + 1 + textHeight;
	if (height < METERBAR_LINEHEIGHT)
		height = METERBAR_LINEHEIGHT;

	return height;
}

void Meterbar::SetBounds(LONG left, LONG top, LONG right, LONG bottom)
{
	SetRect(&boundsRect, left, top, right, bottom);
}

void Meterbar::SetBoundsIndirect(const RECT *prc)
{
	if (NULL == prc)
		SetRectEmpty(&boundsRect);
	else
		CopyRect(&boundsRect, prc);
}

BOOL Meterbar::GetBounds(RECT *prc)
{
	if (NULL == prc)
		return FALSE;
	return CopyRect(prc, &boundsRect);
}

void Meterbar::DrawValueBar(HDC hdc, RECT *prcBar, ULONGLONG value, BOOL approximate, COLORREF rgbFg, HBRUSH brushBk)
{
	TCHAR szText[64];
	RECT rcText;
	SetRect(&rcText, prcBar->right, prcBar->top, prcBar->right, prcBar->bottom);

	FillRect(hdc, prcBar, brushBk);

	FormatBarValue(szText, ARRAYSIZE(szText), (FlagUnitMask & flags), value, approximate);
	INT cchText = lstrlen(szText);
	if (cchText)
	{
		SIZE sizeText;
		if (GetTextExtentPoint32(hdc, szText, cchText, &sizeText) &&
			(textHeight + 4) <= (boundsRect.bottom - boundsRect.top) &&
			(sizeText.cx + 4) <= (boundsRect.right - boundsRect.left))
		{
			if ((prcBar->right - prcBar->left) > (sizeText.cx + 4))
				rcText.left = prcBar->left + ((prcBar->right - prcBar->left) - (sizeText.cx + 4)) / 2 ;
			else
				rcText.left = prcBar->left;
			
			rcText.right = rcText.left + (sizeText.cx + 4);
			if (rcText.right > boundsRect.right)
			{
				rcText.right = boundsRect.right;
				rcText.left = rcText.right - (sizeText.cx + 4);
			}

			rcText.top = boundsRect.top;
			rcText.bottom = rcText.top + textHeight + 4;

			if (prcBar->right < rcText.right)
				prcBar->right = rcText.right;
			
			FillRect(hdc, &rcText, brushBk);
			COLORREF oldColor = SetTextColor(hdc, rgbFg);
			UINT textAlign = SetTextAlign(hdc, TA_BASELINE | TA_LEFT);
		
			//	FrameRect(hdc, &rcText, GetSkinBrush(COLOR_WINDOW));
			
			InflateRect(&rcText, -2, -2);
			
			ExtTextOut(hdc, rcText.left, rcText.bottom, 0, &rcText, szText, cchText, NULL);
			
			SetTextColor(hdc, oldColor);
			SetTextAlign(hdc, textAlign);
		}
	}
}

BOOL Meterbar::Draw(HDC hdc, RECT *prc)
{
	RECT rcPaint, part;
	HFONT hfo;

	if (0 == (FlagVisible & flags))
		return FALSE;

	if (IntersectRect(&rcPaint, &boundsRect, prc))
        FillRect(hdc, &rcPaint, GetSkinBrush(COLOR_BTNFACE));

	if (total > 0 || selected > 0 || limit > 0)
	{
		HFONT hf = GetPluginFont(PLUGINFONT_METERBARTEXT);
		if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;
		SetBkMode(hdc, TRANSPARENT);
	}
	else hfo = NULL;

	SetRect(&part, boundsRect.left, boundsRect.top, boundsRect.left, boundsRect.bottom);
	
	if ((part.bottom - part.top) > METERBAR_LINEHEIGHT)
	{
		part.top += ((4 + textHeight) - METERBAR_LINEHEIGHT) / 2;
		part.bottom = part.top + METERBAR_LINEHEIGHT;
	}
	
	if (total > 0)
	{
		TCHAR szText[64];
		FormatBarValue(szText, ARRAYSIZE(szText), (FlagUnitMask & flags), total, (0 != totalUnknown));

		INT cchText = lstrlen(szText);
		if (cchText)
		{
			SIZE sizeText;
			if (GetTextExtentPoint32(hdc, szText, cchText, &sizeText) &&
				textHeight <= (boundsRect.bottom - part.bottom) &&
				sizeText.cx < (boundsRect.right - boundsRect.left))
			{
				RECT rcText;
				SetRect(&rcText, boundsRect.right - sizeText.cx, part.bottom + 1, boundsRect.right, part.bottom + textHeight + 1);
			
				UINT textAlign = SetTextAlign(hdc, TA_BASELINE | TA_LEFT);
				ExtTextOut(hdc, rcText.left, rcText.bottom, 0, &rcText, szText, cchText, NULL);
				SetTextAlign(hdc, textAlign);
			}
		}
	}

	if (selected > 0 || 0 != selectedUnknown)
	{
		if (total > 0)
			part.right = part.left + (LONG)((boundsRect.right - boundsRect.left) * selected/total);
		else
			part.right = part.left;
		
		if (part.right > boundsRect.right)
			part.right = boundsRect.right;

		if (part.right >= part.left)
			DrawValueBar(hdc, &part, selected, (0 != selectedUnknown), 
				GetSkinColor(COLOR_WINDOW), GetSkinBrush(COLOR_WINDOWTEXT));

	}
	
	if (part.right < boundsRect.right)
	{
		part.left = part.right;
		part.right = boundsRect.right;

		if (IntersectRect(&rcPaint, &part, prc))
			FillRect(hdc, &rcPaint, GetSkinBrush(COLOR_WINDOW));
	}
	
	
	
	if (NULL != hfo)
		SelectObject(hdc, hfo);

	return TRUE;
}

void Meterbar::InvalidateMetrics(UINT metricsType)
{
	BOOL notifyInvalid = FALSE;

	EnterCriticalSection(&lockInvalidate);
	
	if (0 == ((FlagDocumentInvalid | FlagViewInvalid) & flags))
		notifyInvalid = TRUE;

	if (0 != (MetricsDocument & metricsType))
		flags |= FlagDocumentInvalid;
	if (0 != (MetricsSelection & metricsType))
		flags |= FlagViewInvalid;

	if (0 == ((FlagDocumentInvalid | FlagViewInvalid) & flags))
		notifyInvalid = FALSE;

	LeaveCriticalSection(&lockInvalidate);

	if (FALSE != notifyInvalid && NULL != callback)
		callback->MetricsInvalid();

}

BOOL Meterbar::IsInvalid()
{
	BOOL invalid;

	EnterCriticalSection(&lockInvalidate);
	invalid = (0 != ((FlagDocumentInvalid | FlagViewInvalid) & flags));
	LeaveCriticalSection(&lockInvalidate);

	return invalid;
}

void Meterbar::UpdateMetrics()
{	
	BOOL invalidate = FALSE;
	UINT updateFlags;

	EnterCriticalSection(&lockInvalidate);
	updateFlags = (FlagDocumentInvalid | FlagViewInvalid) & flags;
	flags &= ~(FlagDocumentInvalid | FlagViewInvalid);
	LeaveCriticalSection(&lockInvalidate);

	if (0 != (FlagDocumentInvalid & updateFlags))
	{			
		Document *document = (NULL != callback) ? callback->GetDocument() : NULL;
		if (GetDocumentMetrics(document, (FlagUnitMask & flags), &total, &totalUnknown))
			invalidate = TRUE;
	}

	if (0 != (FlagViewInvalid & updateFlags))
	{		
		DropboxView *view = (NULL != callback) ? callback->GetView() : NULL;
		if (GetViewMetrics(view, (FlagUnitMask & flags), &selected, &selectedUnknown))
			invalidate = TRUE;
	}

	if (NULL != invalidate && NULL != callback)
	{
		callback->Invalidate(&boundsRect);
	}
}

BOOL Meterbar::HitTest(POINT pt)
{
	return (PtInRect(&boundsRect, pt));
}

BOOL Meterbar::ButtonDown(UINT mouseButton, POINT pt, UINT mouseFlags)
{
	if (!HitTest(pt))
		return FALSE;
	
	if (FlagVisible != ((FlagDisabled | FlagVisible) & flags))
		return TRUE;

	UINT szModes[] = { FlagUnitCount, FlagUnitLength, FlagUnitSize, };
	INT index;
	for (index = 0; index < ARRAYSIZE(szModes) && szModes[index] != (FlagUnitMask & flags); index++);
	
	if (0 != (MK_SHIFT & mouseFlags)) index--;
	else index++;

	if (index < 0) index = ARRAYSIZE(szModes) - 1;
	if (index >= ARRAYSIZE(szModes)) index = 0;

	UINT newFlags = (flags & ~FlagUnitMask) | szModes[index];
	if (newFlags != flags)
	{
		flags = newFlags;
		InvalidateMetrics(MetricsDocument | MetricsSelection);
		UpdateMetrics();
	}
	return TRUE;
}

BOOL Meterbar::ButtonUp(UINT mouseButton, POINT pt, UINT mouseFlags)
{
	return FALSE;
}
