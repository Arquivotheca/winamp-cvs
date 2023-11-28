#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./viewColumn.h"

class ViewColumnEnum
{
protected:
	ViewColumnEnum(ViewColumn **list, size_t size);
	~ViewColumnEnum();

public:
	static HRESULT CreateInstance(ViewColumn **list, 
								 size_t count,
								 ViewColumnEnum **instance);

public:
	size_t AddRef();
	size_t Release();
	
	HRESULT Next(ViewColumn **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);
	
protected:
	size_t ref;
	ViewColumn **list;
	size_t size;
	size_t cursor;
	
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_DEVICE_DATA_OBJECT_ENUM_HEADER