#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HELPER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./dataValue.h"

HRESULT 
DataValue_InitStackWstrBuffer(DataValue *value, 
							  size_t size);

HRESULT 
DataValue_FreeStackWstrBuffer(DataValue *value);

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HELPER_HEADER