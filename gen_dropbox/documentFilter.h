#ifndef NULLOSFT_DROPBOX_DOCUMENT_FILTER_HEADER
#define NULLOSFT_DROPBOX_DOCUMENT_FILTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
interface IFileInfo;
class FilterPolicy;

EXTERN_C const IID IID_IDocumentFilter;

MIDL_INTERFACE("9F278824-34B3-4fd3-B6FE-A05CCA0E943E")
IDocumentFilter :  public IUnknown
{
    virtual BYTE STDMETHODCALLTYPE GetRule(IFileInfo *item) = 0;
};

class ReloadFilter : public IDocumentFilter
{
protected:
	ReloadFilter();
	~ReloadFilter();

public:
	static ReloadFilter* CreateInstance() { return new ReloadFilter(); }

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	/*** IDocumentFilter ***/
	STDMETHOD_(BYTE, GetRule)(IFileInfo *item);

protected:
	ULONG ref;
};

class InsertFilter :public IDocumentFilter
{
protected:
	InsertFilter(FilterPolicy *pFilterPolicy, HWND hwndHost);
	~InsertFilter();

public:
	static InsertFilter* CreateInstance(FilterPolicy *pFilterPolicy, HWND hwndHost);

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	/*** IDocumentFilter ***/
	STDMETHOD_(BYTE, GetRule)(IFileInfo *item);
	
	BYTE ShowDialog(IFileInfo *item);

private:
	static friend INT_PTR CALLBACK InsertFilter_DialogProc(HWND hwmd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	ULONG ref;
	HWND hHost;
	FilterPolicy *filterPolicy;
	POINT	lastPosition;
};

#endif // NULLOSFT_DROPBOX_DOCUMENT_FILTER_HEADER
