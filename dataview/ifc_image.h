#ifndef _NULLSOFT_WINAMP_DATAVIEW_IMAGE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_IMAGE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {FEE0C1EB-DA65-46a8-89E5-52FC8131F775}
static const GUID IFC_Image = 
{ 0xfee0c1eb, 0xda65, 0x46a8, { 0x89, 0xe5, 0x52, 0xfc, 0x81, 0x31, 0xf7, 0x75 } };


#include <bfc/dispatch.h>


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_image : public Dispatchable
{
protected:
	ifc_image() {}
	~ifc_image() {}

public:
	HRESULT Draw(HDC hdc, int x, int y, int width, int height);
	HRESULT GetSize(int *width, int *height);
	HRESULT Resize(int width, int height);
	HRESULT CopyBits(void *data);

public:
	DISPATCH_CODES
	{
		API_DRAW = 10,
		API_GETSIZE = 20,
		API_RESIZE = 30,
		API_COPYBITS = 40,
	};
};


inline HRESULT ifc_image::Draw(HDC hdc, int x, int y, int width, int height)
{
	return _call(API_DRAW, (HRESULT)E_NOTIMPL, hdc, x, y, width, height);
}

inline HRESULT ifc_image::GetSize(int *width, int *height)
{
	return _call(API_GETSIZE, (HRESULT)E_NOTIMPL, width, height);
}

inline HRESULT ifc_image::Resize(int width, int height)
{
	return _call(API_RESIZE, (HRESULT)E_NOTIMPL, width, height);
}

inline HRESULT ifc_image::CopyBits(void *data)
{
	return _call(API_COPYBITS, (HRESULT)E_NOTIMPL, data);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_IMAGE_INTERFACE_HEADER