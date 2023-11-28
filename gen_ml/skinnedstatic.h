#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedStatic : public SkinnedWnd
{

protected:
	SkinnedStatic(void);
	virtual ~SkinnedStatic(void);

protected:
	virtual BOOL Attach(HWND hwndStatic);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER