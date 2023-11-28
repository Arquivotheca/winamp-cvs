#pragma once
#include "nu/PtrDeque.h"
#include "nu/PtrList.h"
namespace JSON
{
	class Value_Key;
	class Value;
	typedef nu::PtrList<Value_Key> Map;
	typedef nu::PtrList<Value> Array;

	enum DataType
		{
			DATA_NULL,
			DATA_STRING,
			DATA_INTEGER,
			DATA_BOOLEAN,
			DATA_DOUBLE,
			DATA_ARRAY,
			DATA_MAP,
			DATA_KEY,
		};

	class Value_Null;
	class Value_String;
	class Value_Integer;
	class Value_Boolean;
	class Value_Double;
	class Value_Array;
	class Value_Map;
};