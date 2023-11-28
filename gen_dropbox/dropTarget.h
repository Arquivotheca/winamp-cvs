#ifndef NULLOSFT_DROPBOX_PLUGIN_DROPTARGET_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DROPTARGET_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

__interface IDropTargetHelper;

class DropTarget : public IDropTarget
{
protected:
	DropTarget(HWND hwndTarget);
	virtual ~DropTarget(void);

public:
	static BOOL RegisterWindow(HWND hwndTarget);

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IDropTarget ***/
	STDMETHOD (DragEnter)(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHOD (DragOver)(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHOD (DragLeave)(void);
    STDMETHOD (Drop)(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);


public:
	HWND GetHostHwnd(void);
	void TraceDataObject(IDataObject *pObject);

protected:
	ULONG ref;
	HWND hTarget;
	IDropTargetHelper	*pDropTargetHerlper;
};

#endif // NULLOSFT_DROPBOX_PLUGIN_DROPTARGET_HEADER