#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef enum DataType
{
	DT_EMPTY = 0,
	DT_INT8 = 1,
	DT_INT16 = 2,
	DT_INT32 = 3,
	DT_INT64 = 4,
	DT_UINT8 = 5,
	DT_UINT16 = 6,
	DT_UINT32 = 7,
	DT_UINT64 = 8,
	DT_LONG = 9,
	DT_ULONG = 10,
	DT_LLONG = 11,
	DT_ULLONG = 12,
	DT_BOOL = 13,
	DT_FLOAT = 14,
	DT_DOUBLE = 15,
	DT_CHAR	= 16,
	DT_WCHAR = 17,
	DT_CSTR = 18,
	DT_WSTR = 19,
	DT_BSTR = 20,
	DT_WSTR_BUFFER = 21,
	DT_BYTE_BUFFER = 22,
	DT_INTPTR = 23,
	DT_UINTPTR = 24,
	DT_PTRDIFF = 25,
	DT_SIZE = 26,
	DT_PTR = 27,
	DT_DISPATCHABLE = 28,
	DT_TIME64 = 29,
	DT_TIME32 = 30,
	DT_IUNKNOWN = 31,
	DT_IDISPATCH = 32,
} DataType;

typedef struct DtStringBuffer
{
	size_t size;
	wchar_t *data;
} DtStringBuffer;

typedef struct DtByteBuffer
{
	size_t size;
	unsigned char *data;
} DtByteBuffer;

typedef struct DataValue
{
	DataType type;
	union
	{
		char				i8Val;			// DT_INT8
		short				i16Val;			// DT_INT16
		int					i32Val;			// DT_INT32
		__int64				i64Val;			// DT_INT64
		unsigned char		ui8Val;			// DT_UINT8
		unsigned short		ui16Val;		// DT_UINT16
		unsigned int		ui32Val;		// DT_UINT32
		unsigned __int64	ui64Val;		// DT_UINT64
		long				lVal;			// DT_LONG
		long long			llVal;			// DT_LLONG
		unsigned long		ulVal;			// DT_ULONG
		unsigned long long	ullVal;			// DT_ULLONG
		BOOL				boolVal;		// DT_BOOL
		float				floatVal;		// DT_FLOAT
		double				doubleVal;		// DT_DOUBLE
		char				cVal;			// DT_CHAR
		wchar_t				wVal;			// DT_WCHAR
		const char			*cstrVal;		// DT_CSTR
		const wchar_t		*wstrVal;		// DT_WSTR
		BSTR				bstrVal;		// DT_BSTR
		DtStringBuffer		wstrBuffer;		// DT_WSTR_BUFFER
		DtByteBuffer		byteBuffer;		// DT_BYTE_BUFFER
		intptr_t			intptrVal;		// DT_INTPTR
		uintptr_t			uintptrVal;		// DT_UINTPTR
		ptrdiff_t			ptrdiffVal;		// DT_PTRDIFF
		size_t				sizeVal;		// DT_SIZE
		void *				ptrVal;			// DT_PTR
		Dispatchable		*dispatchVal;	// DT_DISPATCHABLE
		__time64_t			time64Val;		// DT_TIME64
		__time32_t			time32Val;		// DT_TIME32
		IUnknown			*iUnkVal;		// DT_IUNKNOWN
		IDispatch			*iDispVal;		// DT_IDISPATCH
	};
} DataValue;

inline void DataValue_Clear(DataValue *value)
{
	switch(value->type)
	{
		case DT_BSTR: SysFreeString(value->bstrVal); break;
		case DT_DISPATCHABLE: if (NULL != value->dispatchVal) value->dispatchVal->Release(); break;
		case DT_IUNKNOWN: if (NULL != value->iUnkVal) value->iUnkVal->Release(); break;
		case DT_IDISPATCH: if (NULL != value->iDispVal) value->iDispVal->Release(); break;
	}
	value->type = DT_EMPTY;
}

#define IS_VALUE_NAME_EQUAL(_valueName1, _valueName2)\
	(CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (_valueName1), -1, (_valueName2), -1))

#define DATAVALUE_INIT(_value)\
		(_value)->type = DT_EMPTY;

#define DATAVALUE_CLEAR(_value)\
		DataValue_Clear(_value);

#define DATAVALUE_SET_TYPE(_value, _type)\
		(_value)->type = (_type);

#define DATAVALUE_IS_TYPE(_value, _type)\
		((_value)->type == (_type))

#define DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, _type, _error)\
		{if (FALSE == DATAVALUE_IS_TYPE(_value, _type)) return (_error);}

// DT_INT32
#define DATAVALUE_GET_INT32(_value)\
		((_value)->i32Val)

#define DATAVALUE_SET_INT32(_value, _data)\
		((_value)->i32Val = (_data))

#define DATAVALUE_SET_INT32_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_INT32, _error);\
		 DATAVALUE_SET_INT32(_value, _data);}
		
#define DATAVALUE_INIT_INT32(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_INT32); \
		 DATAVALUE_SET_INT32(_value, _data);}

// DT_INT64
#define DATAVALUE_GET_INT64(_value)\
	    ((_value)->i64Val)

#define DATAVALUE_SET_INT64(_value, _data)\
	    ((_value)->i64Val = (_data))

#define DATAVALUE_SET_INT64_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_INT64, _error);\
		 DATAVALUE_SET_INT64(_value, _data);}
		
#define DATAVALUE_INIT_INT64(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_INT64);\
		 DATAVALUE_SET_INT64(_value, _data);}

// DT_UINT32
#define DATAVALUE_GET_UINT32(_value)\
		((_value)->ui32Val)

#define DATAVALUE_SET_UINT32(_value, _data)\
		((_value)->ui32Val = (_data))

#define DATAVALUE_SET_UINT32_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_UINT32, _error);\
		 DATAVALUE_SET_UINT32(_value, _data);}
		
#define DATAVALUE_INIT_UINT32(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_UINT32); \
		 DATAVALUE_SET_UINT32(_value, _data);}

// DT_UINT64
#define DATAVALUE_GET_UINT64(_value)\
		((_value)->ui64Val)

#define DATAVALUE_SET_UINT64(_value, _data)\
		((_value)->ui64Val = (_data))

#define DATAVALUE_SET_UINT64_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_UINT64, _error);\
		 DATAVALUE_SET_UINT64(_value, _data);}
		
#define DATAVALUE_INIT_UINT64(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_UINT64); \
		 DATAVALUE_SET_UINT64(_value, _data);}

// DT_LONG
#define DATAVALUE_GET_LONG(_value)\
	    ((_value)->lVal)

#define DATAVALUE_SET_LONG(_value, _data)\
	    ((_value)->lVal = (_data))

#define DATAVALUE_SET_LONG_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_LONG, _error);\
		 DATAVALUE_SET_LONG(_value, _data);}
		
#define DATAVALUE_INIT_LONG(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_LONG);\
		 DATAVALUE_SET_LONG(_value, _data);}

// DT_LLONG
#define DATAVALUE_GET_LLONG(_value)\
	    ((_value)->llVal)

#define DATAVALUE_SET_LLONG(_value, _data)\
	    ((_value)->llVal = (_data))

#define DATAVALUE_SET_LLONG_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_LLONG, _error);\
		 DATAVALUE_SET_LLONG(_value, _data);}
		
#define DATAVALUE_INIT_LLONG(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_LLONG);\
		 DATAVALUE_SET_LLONG(_value, _data);}

// DT_ULONG
#define DATAVALUE_GET_ULONG(_value)\
	    ((_value)->ulVal)

#define DATAVALUE_SET_ULONG(_value, _data)\
	    ((_value)->ulVal = (_data))

#define DATAVALUE_SET_ULONG_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_ULONG, _error);\
		 DATAVALUE_SET_ULONG(_value, _data);}
		
#define DATAVALUE_INIT_ULONG(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_ULONG);\
		 DATAVALUE_SET_ULONG(_value, _data);}

// DT_ULLONG
#define DATAVALUE_GET_ULLONG(_value)\
	    ((_value)->ullVal)

#define DATAVALUE_SET_ULLONG(_value, _data)\
	    ((_value)->ullVal = (_data))

#define DATAVALUE_SET_ULLONG_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_ULLONG, _error);\
		 DATAVALUE_SET_ULLONG(_value, _data);}
		
#define DATAVALUE_INIT_ULLONG(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_ULLONG);\
		 DATAVALUE_SET_ULLONG(_value, _data);}

// DT_CHAR
#define DATAVALUE_GET_CHAR(_value)\
		((_value)->cVal)

#define DATAVALUE_SET_CHAR(_value, _data)\
		((_value)->cVal = (_data))

#define DATAVALUE_SET_CHAR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_CHAR, _error);\
		 DATAVALUE_SET_CHAR(_value, _data);}

#define DATAVALUE_INIT_CHAR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_CHAR);\
		 DATAVALUE_SET_CHAR(_value, _data);}

// DT_WCHAR
#define DATAVALUE_GET_WCHAR(_value)\
		((_value)->wVal)

#define DATAVALUE_SET_WCHAR(_value, _data)\
		((_value)->wVal = (_data))

#define DATAVALUE_SET_WCHAR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_WCHAR, _error);\
		 DATAVALUE_SET_WCHAR(_value, _data);}

#define DATAVALUE_INIT_WCHAR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_WCHAR);\
		 DATAVALUE_SET_WCHAR(_value, _data);}

// DT_CSTR
#define DATAVALUE_GET_CSTR(_value)\
		((_value)->cstrVal)

#define DATAVALUE_SET_CSTR(_value, _data)\
		((_value)->cstrVal = (_data))

#define DATAVALUE_SET_CSTR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_CSTR, _error);\
		 DATAVALUE_SET_CSTR(_value, _data);}

#define DATAVALUE_INIT_CSTR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_CSTR);\
		 DATAVALUE_SET_CSTR(_value, _data);}

 // DT_WSTR
#define DATAVALUE_GET_WSTR(_value)\
		((_value)->wstrVal)

#define DATAVALUE_SET_WSTR(_value, _data)\
		((_value)->wstrVal = (_data))

#define DATAVALUE_SET_WSTR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_WSTR, _error);\
		 DATAVALUE_SET_WSTR(_value, _data);}

#define DATAVALUE_INIT_WSTR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_WSTR);\
		 DATAVALUE_SET_WSTR(_value, _data);}

// DT_BSTR
#define DATAVALUE_GET_BSTR(_value)\
		((_value)->bstrVal)

#define DATAVALUE_SET_BSTR(_value, _data)\
		((_value)->bstrVal = (_data))

#define DATAVALUE_SET_BSTR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_BSTR, _error);\
		 DATAVALUE_SET_BSTR(_value, _data);}

#define DATAVALUE_INIT_BSTR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_BSTR);\
		 DATAVALUE_SET_BSTR(_value, _data);}

// DT_WSTR_BUFFER
#define DATAVALUE_GET_WSTR_BUFFER(_value)\
		((_value)->wstrBuffer)

#define DATAVALUE_GET_WSTR_BUFFER_DATA(_value)\
		((_value)->wstrBuffer.data)

#define DATAVALUE_GET_WSTR_BUFFER_SIZE(_value)\
		((_value)->wstrBuffer.size)

#define DATAVALUE_SET_WSTR_BUFFER(_value, _buffer, _bufferSize)\
		{(_value)->wstrBuffer.data = (_buffer);\
		 (_value)->wstrBuffer.size = (_bufferSize);}

 #define DATAVALUE_SET_WSTR_BUFFER_RETURN_ERR(_value, _buffer, _bufferSize, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_WSTR_BUFFER, _error);\
		 DATAVALUE_SET_WSTR_BUFFER(_value, _buffer, _bufferSize);}

#define DATAVALUE_INIT_WSTR_BUFFER(_value, _buffer, _bufferSize)\
		{DATAVALUE_SET_TYPE(_value, DT_WSTR_BUFFER);\
		 DATAVALUE_SET_WSTR_BUFFER(_value, _buffer, _bufferSize);}

// DT_BYTE_BUFFER
#define DATAVALUE_GET_BYTE_BUFFER(_value)\
		((_value)->byteBuffer)

#define DATAVALUE_GET_BYTE_BUFFER_DATA(_value)\
		((_value)->byteBuffer.data)

#define DATAVALUE_GET_BYTE_BUFFER_SIZE(_value)\
		((_value)->byteBuffer.size)

#define DATAVALUE_SET_BYTE_BUFFER(_value, _buffer, _bufferSize)\
		{(_value)->byteBuffer.data = (_buffer);\
		 (_value)->byteBuffer.size = (_bufferSize);}

#define DATAVALUE_SET_BYTE_BUFFER_RETURN_ERR(_value, _buffer, _bufferSize, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_BYTE_BUFFER, _error);\
		 DATAVALUE_SET_BYTE_BUFFER(_value, _buffer, _bufferSize);}

#define DATAVALUE_INIT_BYTE_BUFFER(_value, _buffer, _bufferSize)\
		{DATAVALUE_SET_TYPE(_value, DT_BYTE_BUFFER);\
		 DATAVALUE_SET_BYTE_BUFFER(_value, _buffer, _bufferSize);}


// DT_INTPTR
#define DATAVALUE_GET_INTPTR(_value)\
		((_value)->intptrVal)

#define DATAVALUE_SET_INTPTR(_value, _data)\
		((_value)->intptrVal = (_data))

#define DATAVALUE_SET_INTPTR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_INTPTR, _error);\
		 DATAVALUE_SET_INTPTR(_value, _data);}

#define DATAVALUE_INIT_INTPTR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_INTPTR);\
		 DATAVALUE_SET_INTPTR(_value, _data);}

// DT_SIZE
#define DATAVALUE_GET_SIZE(_value)\
		((_value)->sizeVal)

#define DATAVALUE_SET_SIZE(_value, _data)\
		((_value)->sizeVal = (_data))

#define DATAVALUE_SET_SIZE_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_SIZE, _error);\
		 DATAVALUE_SET_SIZE(_value, _data);}

#define DATAVALUE_INIT_SIZE(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_SIZE);\
		 DATAVALUE_SET_SIZE(_value, _data);}


// DT_PTR
#define DATAVALUE_GET_PTR(_value)\
		((_value)->ptrVal)

#define DATAVALUE_SET_PTR(_value, _data)\
		((_value)->ptrVal = (_data))

#define DATAVALUE_SET_PTR_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_PTR, _error);\
		 DATAVALUE_SET_PTR(_value, _data);}

#define DATAVALUE_INIT_PTR(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_PTR);\
		 DATAVALUE_SET_PTR(_value, _data);}

// DT_DISPATCHABLE
#define DATAVALUE_GET_DISPATCHABLE(_value)\
		((_value)->dispatchVal)

#define DATAVALUE_SET_DISPATCHABLE(_value, _data)\
		{((_value)->dispatchVal = (_data));\
		 if (NULL != (_value)->dispatchVal){ (_value)->dispatchVal->AddRef();}}

#define DATAVALUE_SET_DISPATCHABLE_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_DISPATCHABLE, _error);\
		 DATAVALUE_SET_DISPATCHABLE(_value, _data);}

#define DATAVALUE_INIT_DISPATCHABLE(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_DISPATCHABLE);\
		 DATAVALUE_SET_DISPATCHABLE(_value, _data);}

// DT_TIME64
#define DATAVALUE_GET_TIME64(_value)\
		((_value)->time64Val)

#define DATAVALUE_SET_TIME64(_value, _data)\
		((_value)->time64Val = (_data))

#define DATAVALUE_SET_TIME64_RETURN_ERR(_value, _data, _error)\
		{DATAVALUE_CHECK_TYPE_RETURN_ERR(_value, DT_TIME64, _error);\
		 DATAVALUE_SET_TIME64(_value, _data);}

#define DATAVALUE_INIT_TIME64(_value, _data)\
		{DATAVALUE_SET_TYPE(_value, DT_TIME64);\
		 DATAVALUE_SET_TIME64(_value, _data);}


// misc
#define DATAVALUE_SET_XINT32_RETURN_ERR(_value, _data, _error)\
		{int _d = (_data);\
		 if (DATAVALUE_IS_TYPE(_value, DT_INT32)) {DATAVALUE_SET_INT32(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_UINT32)) {DATAVALUE_SET_UINT32(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_LONG)) {DATAVALUE_SET_LONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_ULONG)) {DATAVALUE_SET_ULONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_SIZE)) {DATAVALUE_SET_SIZE(_value, _d);}\
		 else return (_error);}

#define DATAVALUE_SET_XINT64_RETURN_ERR(_value, _data, _error)\
		{__int64 _d = (_data);\
		 if (DATAVALUE_IS_TYPE(_value, DT_INT64)) {DATAVALUE_SET_INT64(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_UINT64)) {DATAVALUE_SET_UINT64(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_LLONG)) {DATAVALUE_SET_LLONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_ULLONG)) {DATAVALUE_SET_ULLONG(_value, _d);}\
		 else return (_error);}


#define DATAVALUE_SET_INT_RETURN_ERR(_value, _data, _error)\
		{int _d = (_data);\
		 if (DATAVALUE_IS_TYPE(_value, DT_INT32)) {DATAVALUE_SET_INT32(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_UINT32)) {DATAVALUE_SET_UINT32(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_INT64)) {DATAVALUE_SET_INT64(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_UINT64)) {DATAVALUE_SET_UINT64(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_LONG)) {DATAVALUE_SET_LONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_ULONG)) {DATAVALUE_SET_ULONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_LLONG)) {DATAVALUE_SET_LLONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_ULLONG)) {DATAVALUE_SET_ULLONG(_value, _d);}\
		 else if (DATAVALUE_IS_TYPE(_value, DT_SIZE)) {DATAVALUE_SET_SIZE(_value, _d);}\
		 else return (_error);}

#define DATAVALUE_COMPARE_INT32(_number, _dataValue)\
		{if (DATAVALUE_IS_TYPE((_dataValue), DT_INT32)){ return COBJ_COMPARE((int)(_number), DATAVALUE_GET_INT32(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_UINT32)){ return COBJ_COMPARE((unsigned int)(_number), DATAVALUE_GET_UINT32(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_LONG)){ return COBJ_COMPARE((long)(_number), DATAVALUE_GET_LONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_ULONG)){ return COBJ_COMPARE((unsigned long)(_number), DATAVALUE_GET_ULONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_SIZE)){ return COBJ_COMPARE((size_t)(_number), DATAVALUE_GET_SIZE(_dataValue)); }\
		 return COBJ_ERROR;}

#define DATAVALUE_COMPARE_INT64(_number, _dataValue)\
		{if (DATAVALUE_IS_TYPE((_dataValue), DT_INT64)){ return COBJ_COMPARE((__int64)(_number), DATAVALUE_GET_INT64(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_UINT64)){ return COBJ_COMPARE((unsigned __int64)(_number), DATAVALUE_GET_UINT64(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_LLONG)){ return COBJ_COMPARE((long long)(_number), DATAVALUE_GET_LLONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_ULLONG)){ return COBJ_COMPARE((unsigned long long)(_number), DATAVALUE_GET_ULLONG(_dataValue)); }\
		 return COBJ_ERROR;}

#define DATAVALUE_COMPARE_INT(_number, _dataValue)\
		{if (DATAVALUE_IS_TYPE((_dataValue), DT_INT32)){ return COBJ_COMPARE((int)(_number), DATAVALUE_GET_INT32(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_UINT32)){ return COBJ_COMPARE((unsigned int)(_number), DATAVALUE_GET_UINT32(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_INT64)){ return COBJ_COMPARE((__int64)(_number), DATAVALUE_GET_INT64(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_UINT64)){ return COBJ_COMPARE((unsigned __int64)(_number), DATAVALUE_GET_UINT64(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_LONG)){ return COBJ_COMPARE((long)(_number), DATAVALUE_GET_LONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_ULONG)){ return COBJ_COMPARE((unsigned long)(_number), DATAVALUE_GET_ULONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_LLONG)){ return COBJ_COMPARE((long long)(_number), DATAVALUE_GET_LLONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_ULLONG)){ return COBJ_COMPARE((unsigned long long)(_number), DATAVALUE_GET_ULLONG(_dataValue)); }\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_SIZE)){ return COBJ_COMPARE((size_t)(_number), DATAVALUE_GET_SIZE(_dataValue)); }\
		 return COBJ_ERROR;}

		
#define DATAVALUE_COMPARE_STRING(_string, _dataValue, _localeId, _compareFn)\
		{if (DATAVALUE_IS_TYPE((_dataValue), DT_WSTR)){ return _compareFn((_localeId), (_string), DATAVALUE_GET_WSTR(_dataValue));}\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_WSTR_BUFFER)){ return _compareFn((_localeId), (_string), DATAVALUE_GET_WSTR_BUFFER_DATA(_dataValue));}\
		 else if (DATAVALUE_IS_TYPE((_dataValue), DT_BSTR)){ return _compareFn((_localeId), (_string), DATAVALUE_GET_BSTR(_dataValue));}\
		 return COBJ_ERROR;}

#define DATAVALUE_COMPARE_TIME64(_time, _dataValue)\
		{if (DATAVALUE_IS_TYPE((_dataValue), DT_TIME64)){ return COBJ_COMPARE((_time), DATAVALUE_GET_TIME64(_dataValue)); }\
		 return COBJ_ERROR;}

#define DATAVALUE_COMPARE_SORTKEY(_key, _dataValue, _localeId)\
		{if (NULL == (_key)) {return COBJ_ERROR;}\
		 if (DATAVALUE_IS_TYPE((_dataValue), DT_BYTE_BUFFER))\
		 { if ((_key)->GetLocaleId() != (_localeId)) {return COBJ_ERROR;}\
		   return (_key)->Compare(DATAVALUE_GET_BYTE_BUFFER_DATA(_dataValue), DATAVALUE_GET_BYTE_BUFFER_SIZE(_dataValue));}\
	 	 if (DATAVALUE_IS_TYPE((_dataValue), DT_DISPATCHABLE))\
		 {ifc_sortkey *_key2;\
		  _key2 = (ifc_sortkey*)DATAVALUE_GET_DISPATCHABLE((_dataValue));\
		  if (NULL == _key2 || (_key)->GetLocaleId() != _key2->GetLocaleId()) {return COBJ_ERROR;}\
		  return (_key)->Compare(_key2->GetValue(), _key2->GetSize());}\
		 return COBJ_ERROR;}

#define DATAVALUE_COPY_WSTR_BUFFER_RETURN_ERR(_value, _data, _error)\
		DATAVALUE_CHECK_TYPE_RETURN_ERR((_value), DT_WSTR_BUFFER, (_error));\
		{if (FAILED(StringCchCopyEx(DATAVALUE_GET_WSTR_BUFFER_DATA((_value)),\
									DATAVALUE_GET_WSTR_BUFFER_SIZE((_value)),\
									(_data), NULL, NULL, STRSAFE_IGNORE_NULLS)))\
		{ return E_OUTOFMEMORY;}}
#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VALUE_HEADER