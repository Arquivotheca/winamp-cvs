#ifndef NULLOSFT_DROPBOX_PLUGIN_DROPSOURCE_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DROPSOURCE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define OEMRESOURCE
#include <windows.h>


typedef enum 
{
	DROPSOURCE_NORMALMODE = 0,
	DROPSOURCE_REARRANGEMODE = 1,
} DROPSOURCEFLAGS;

class DropSource: public IDropSource
{
public:
	DropSource(HWND hwndSource);
	~DropSource();

public:

	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IDropSource ***/
   STDMETHOD(GiveFeedback)(DWORD);
   STDMETHOD(QueryContinueDrag)(BOOL, DWORD);

   UINT GetFlags(UINT flagsMask);
   void SetFlags(UINT flags, UINT flagsMask);

   HWND GetSourceHwnd() { return hSource; }
protected:
	ULONG ref;
	UINT flags;
	HCURSOR rearrangeCursor;
	HWND hSource;
};

#endif //NULLOSFT_DROPBOX_PLUGIN_DROPSOURCE_HEADER
