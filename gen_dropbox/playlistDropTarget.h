#ifndef NULLOSFT_DROPBOX_PLUGIN_PLAYLISTDROPTARGET_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_PLAYLISTDROPTARGET_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "../gen_ml/ml.h"

__interface IDropTargetHelper;
__interface IFileEnumerator;
class PlaylistDropTargetCallback;

class PlaylistDropTarget : public IDropTarget
{
protected:
	PlaylistDropTarget(HWND hwndTarget, HWND hwndDropBox);
	virtual ~PlaylistDropTarget(void);

public:
	static PlaylistDropTarget *RegisterWindow(HWND hwndTarget, HWND hwndDropBox);

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
	void MediaLibraryDragDrop(INT code, mlDropItemStruct *pdis);
	void RegisterCallback(PlaylistDropTargetCallback *newCallback) { callback = newCallback; }

protected:
	HRESULT CanProcessDataObject(IDataObject *pDataObject);

protected:
	ULONG ref;
	HWND hTarget;
	HWND hDropBox;
	BOOL acceptable;
	IDropTargetHelper	*pDropTargetHerlper;
	PlaylistDropTargetCallback	*callback;
};

class __declspec(novtable) PlaylistDropTargetCallback 
{

public:
	virtual void OnDestroy(PlaylistDropTarget *instance) = 0;
	virtual BOOL DragEnter(PlaylistDropTarget *instance, POINT pt, UINT keyState) = 0;
	virtual BOOL DragOver(PlaylistDropTarget *instance, POINT pt, UINT keyState) = 0;
	virtual void DragLeave(PlaylistDropTarget *instance) = 0;
	virtual BOOL Drop(PlaylistDropTarget *instance, POINT pt, UINT keyState) = 0 ;

};

#endif // NULLOSFT_DROPBOX_PLUGIN_PLAYLISTDROPTARGET_HEADER