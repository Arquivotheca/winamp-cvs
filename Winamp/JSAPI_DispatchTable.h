#pragma once
#include "../nu/PtrList.h"
#include <ocidl.h>
namespace JSAPI
{
	struct Dispatcher
		{
			Dispatcher(){}
			Dispatcher(const wchar_t *_name, DISPID _id, IDispatch *_object);
			~Dispatcher();
			wchar_t name[128];
			DISPID id;
			IDispatch *object;
		};

	typedef nu::PtrList<JSAPI::Dispatcher> DispatchTable;

};