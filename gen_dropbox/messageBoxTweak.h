#ifndef NULLOSFT_DROPBOX_PLUGIN_MESSAGEBOX_TWEAK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_MESSAGEBOX_TWEAK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/vector.h"

class MessageBoxTweak
{
public: 
	typedef enum
	{
		TWEAK_NORMAL = 0x0000,
		TWEAK_CENTERPARENT = 0x0001,
		TWEAK_OVERRIDEBUTTONTEXT = 0x0002, // will try to override buttons text. this happens before values set in OverrideCtrlText will be used
		TWEAK_APPLICATIONMODAL	= 0x0004, // this will work only if passed to ShowEx
		TWEAK_SHOWCHECKBOX	= 0x0008,
	} TWEAKFLAGS;

	typedef struct __CTRLOVERRIDE
	{
		UINT ctrlId;
		LPWSTR pszText;
	} CTRLOVERRIDE;

public:
	MessageBoxTweak();
	virtual ~MessageBoxTweak();

public:
	static INT Show(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
	static INT ShowEx(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, 
						DWORD tweakFlags, CTRLOVERRIDE* pOverrides, size_t overridesCount);

public:
	BOOL BeginSubclass();
	void EndSubclass();
	void SetFlags(DWORD flags, DWORD flagsMask);
	DWORD GetFlags(DWORD flagsMask);
	void SetCenterOn(HWND hwndToCenterOn);	// this will be used with TWEAK_CENTERPARENT and override parent
	void OverrideCtrlText(UINT ctrlId, LPCWSTR pszText); // set pszText to NULL to remove. pszText can be MAKEINTRESOURCE and will be resolved with API_LNG

protected:
	virtual BOOL Attach(HWND hTarget);
	virtual void Detach();
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnWindowActivate(UINT nState, HWND hwndOther, BOOL bMinimized);
	
	BOOL CenterParent();
	void OverrideText();
	BOOL ReplaceCtrlText(UINT ctrlId, LPCWSTR pszText, LPWSTR pszBuffer, INT cchBufferMax);

	LRESULT PreviousWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT DefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	friend static INT CALLBACK MessageBoxTweak_SubclassProc(HWND hTarget, CREATESTRUCT *pcs, HWND hInsertAfter, ULONG_PTR user);
	friend static LRESULT CALLBACK MessageBoxTweak_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	typedef Vector<CTRLOVERRIDE> OverrideList;

protected:
	HWND		hwnd;
	WNDPROC originalProc;
	BOOL	bUnicode;
	DWORD	flags;
	HWND		hwndCenter;
	BOOL	firstActivate;
	OverrideList overrideList;
};

#endif // NULLOSFT_DROPBOX_PLUGIN_MESSAGEBOX_TWEAK_HEADER