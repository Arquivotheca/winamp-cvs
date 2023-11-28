#ifndef NULLOSFT_DROPBOX_PLUGIN_METERBAR_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_METERBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


class Document;
class DropboxView;
class Profile;

class MeterbarCallback;

class Meterbar
{
public:
	typedef enum
	{
		FlagVisible		= 0x00000001,
		FlagDisabled	= 0x00000002,

		FlagDocumentInvalid = 0x00000010,
		FlagViewInvalid		= 0x00000020,

		FlagUnitMask		= 0x00FF0000,
		
		FlagUnitCount	= 0x00000000,
		FlagUnitLength	= 0x00010000,
		FlagUnitSize	= 0x00020000,
		
		_FlagUnitFirst	= FlagUnitCount,
		_FlagUnitLast	= FlagUnitSize,

	} MeterbarFlags;

	
	typedef enum
	{
		MouseButtonLeft = 0,
		MouseButtonRight = 1,
		MouseButtonMiddle = 2,
		MouseButtonX = 3,
	} MouseButton;

	typedef enum
	{
		MetricsNone			= 0x0000,
		MetricsDocument		= 0x0001,
		MetricsSelection		= 0x0002,
	} MetricsSource;

public: 
	Meterbar(UINT meterbarFlags);
	~Meterbar();

	static UINT ParseUnit(LPCTSTR pszBuffer);
	static Meterbar *Load(Profile *profile);

public:

	UINT GetFlags() { return flags; }
	void SetFlags(UINT newFlags, UINT flagsMask);

	void SetBounds(LONG left, LONG top, LONG right, LONG bottom);
	void SetBoundsIndirect(const RECT *prc);
	BOOL GetBounds(RECT *prc);
	INT GetPrefferedHeight(HWND hHost);
	BOOL Draw(HDC hdc, RECT *prc);
	
	void UpdateMetrics();
	void InvalidateMetrics(UINT metricsType);
	BOOL IsInvalid();
	
	void SetLimit(ULONGLONG limit);

	BOOL HitTest(POINT pt);

	BOOL ButtonDown(UINT mouseButton, POINT pt, UINT mouseFlags);
	BOOL ButtonUp(UINT mouseButton, POINT pt, UINT mouseFlags);
	
	void RegisterCallback(MeterbarCallback *callbackInstance) { callback = callbackInstance; }
	void TimerElapsed();

	HRESULT Save(Profile *profile);

private:
	void DrawValueBar(HDC hdc, RECT *prcBar, ULONGLONG value, BOOL approximate, COLORREF rgbFg, HBRUSH brushBk);

public:
	UINT flags;
	ULONGLONG limit;
	ULONGLONG total;
	size_t totalUnknown;
	ULONGLONG selected;
	size_t selectedUnknown;
	INT		textHeight;
	RECT boundsRect;
	HBITMAP hbmp;
	CRITICAL_SECTION lockInvalidate;
	MeterbarCallback *callback;
	static const LPCTSTR szUnitName[_FlagUnitLast - _FlagUnitFirst + 1];
};


#define Meterbar_GetUnitName(__unitFlag) ( Meterbar::szUnitName[ ((HIWORD(__unitFlag) < ARRAYSIZE(Meterbar::szUnitName)) ? HIWORD(__unitFlag) : 0) ] )

EXTERN_C const GUID meterbarSettingsGuid;
#define CFG_METERUNIT		MAKEINTRESOURCEA(1)

#endif // NULLOSFT_DROPBOX_PLUGIN_METERBAR_HEADER