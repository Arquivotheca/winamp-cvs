#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


class FileMetaCache;

interface ITypeInfo;
interface IFileEnumerator;

EXTERN_C const IID IID_IFileInfo;

MIDL_INTERFACE("C714F412-9E9D-42ed-A5F1-352849EEE2BF")
IFileInfo : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetPath(LPCTSTR *pszBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFileName(LPCTSTR *pszBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetExtension(LPCTSTR *pszBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetType(DWORD *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSize(ULONGLONG *pSize) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetAttributes(DWORD *pAttributes) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCreationTime(FILETIME *pTime) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetModifiedTime(FILETIME *pTime) = 0;
	
	virtual HRESULT STDMETHODCALLTYPE ResetCache(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnumItems(IFileEnumerator **ppEnumerator) = 0;
	virtual HRESULT STDMETHODCALLTYPE CanCopy(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE CanPlay(void) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetExtraInfo(ULONG_PTR) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetExtraInfo(ULONG_PTR*) = 0;

};

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_INTERFACE_HEADER