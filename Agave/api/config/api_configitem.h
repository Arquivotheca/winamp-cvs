#ifndef NULLSOFT_AGAVE_API_CONFIGITEM_H
#define NULLSOFT_AGAVE_API_CONFIGITEM_H

#include <bfc/dispatch.h>
#include <stddef.h>
/*
notes:
The Set() functions are "public-facing", meaning that they can be called by anyone.  If you want to make your config item read-only,
then simply don't implement these.  You can always make "private" Set functions in your implementation.

SetStringInternal and GetStringInternal are written for use with classes to load and save from INI files (or XML files or whatever).  
It's up to you to figure out a clever way to encode yourself.

*/

enum
{
	CONFIG_ITEM_TYPE_STRING = 0,
	CONFIG_ITEM_TYPE_INT = 1,
	CONFIG_ITEM_TYPE_UNSIGNED =2,
	CONFIG_ITEM_TYPE_BOOL =3,
	CONFIG_ITEM_TYPE_BINARY =4,
	CONFIG_ITEM_TYPE_INT_ARRAY = 5,
};

class api_configitem : public Dispatchable
{
protected:
	api_configitem() {}
	~api_configitem() {}
public:
	const wchar_t *GetName();
	int GetType();

	const wchar_t *GetString();
	void SetString(const wchar_t *stringValue); 

	intptr_t GetInt();
	void SetInt(intptr_t intValue);

	uintptr_t GetUnsigned();
	void SetUnsigned(uintptr_t unsignedValue);

	bool GetBool();
	void SetBool(bool boolValue);

	float GetFloat();
	void SetFloat(float floatValue);

	size_t GetBinarySize();
	size_t GetBinaryData(void *data, size_t bytes); // returns bytes written
	void SetBinaryData(void *data, size_t bytes);

	size_t GetIntArrayElements();
	size_t GetIntArray(intptr_t *array, size_t elements); // returns elements written
	void SetIntArray(intptr_t *array, size_t elements);

	const wchar_t *GetStringInternal(); // gets a string suitable for saving in an INI file or XML
	void SetStringInternal(const wchar_t *internalString); 

public:
	DISPATCH_CODES
	{
		API_CONFIGITEM_GETNAME = 10,
			API_CONFIGITEM_GETTYPE = 20,

			API_CONFIGITEM_GETSTRING= 30,
			API_CONFIGITEM_SETSTRING= 40,

			API_CONFIGITEM_GETINT= 50,
			API_CONFIGITEM_SETINT= 60,

			API_CONFIGITEM_GETUNSIGNED= 70,
			API_CONFIGITEM_SETUNSIGNED= 80,

			API_CONFIGITEM_GETBOOL= 90,
			API_CONFIGITEM_SETBOOL= 100,

			API_CONFIGITEM_GETBINARYSIZE= 110,
			API_CONFIGITEM_GETBINARYDATA= 120,
			API_CONFIGITEM_SETBINARYDATA= 130,

			API_CONFIGITEM_GETINTARRAYELEMENTS= 140,
			API_CONFIGITEM_GETINTARRAY= 150,
			API_CONFIGITEM_SETINTARRAY= 160,

			API_CONFIGITEM_GETSTRINGINTERNAL= 170,
			API_CONFIGITEM_SETSTRINGINTERNAL= 180,

						API_CONFIGITEM_GETFLOAT= 190,
			API_CONFIGITEM_SETFLOAT= 200,
	};
};



inline const wchar_t *api_configitem::GetName()
{
	return _call(API_CONFIGITEM_GETNAME, (const wchar_t *)0);
}

inline int api_configitem::GetType()
{
	return _call(API_CONFIGITEM_GETTYPE, (int)0);
}


inline const wchar_t *api_configitem::GetString()
{
	return _call(API_CONFIGITEM_GETSTRING, (const wchar_t *)0);
}

inline void api_configitem::SetString(const wchar_t *stringValue)
{
	_voidcall(API_CONFIGITEM_SETSTRING, stringValue);
}


inline intptr_t api_configitem::GetInt()
{
	return _call(API_CONFIGITEM_GETINT, (intptr_t)0);
}
#pragma warning(push)
#pragma warning(disable: 4244)
inline void api_configitem::SetInt(intptr_t intValue)
{
	_voidcall(API_CONFIGITEM_SETINT, intValue);
}
#pragma warning(pop)

inline uintptr_t api_configitem::GetUnsigned()
{
	return _call(API_CONFIGITEM_GETUNSIGNED, (uintptr_t)0);
}

inline void api_configitem::SetUnsigned(uintptr_t unsignedValue)
{
	_voidcall(API_CONFIGITEM_SETUNSIGNED, unsignedValue);
}


inline bool api_configitem::GetBool()
{
	return _call(API_CONFIGITEM_GETBOOL, (bool)false);
}

inline void api_configitem::SetBool(bool boolValue)
{
	_voidcall(API_CONFIGITEM_SETBOOL, boolValue);
}

inline size_t api_configitem::GetBinarySize()
{
	return _call(API_CONFIGITEM_GETBINARYSIZE, (size_t)0);
}

inline size_t api_configitem::GetBinaryData(void *data, size_t bytes)
{
	return _call(API_CONFIGITEM_GETBINARYDATA, (size_t)0, data, bytes);
}

inline void api_configitem::SetBinaryData(void *data, size_t bytes)
{
	_voidcall(API_CONFIGITEM_SETBINARYDATA, data, bytes);
}

inline size_t api_configitem::GetIntArrayElements()
{
	return _call(API_CONFIGITEM_GETINTARRAYELEMENTS, (size_t)0);
}

inline size_t api_configitem::GetIntArray(intptr_t *array, size_t elements)
{
	return _call(API_CONFIGITEM_GETINTARRAY, (size_t)0, array, elements);
}
inline void api_configitem::SetIntArray(intptr_t *array, size_t elements)
{
	_voidcall(API_CONFIGITEM_SETINTARRAY, array, elements);
}

inline const wchar_t *api_configitem::GetStringInternal()
{
	return _call(API_CONFIGITEM_GETSTRINGINTERNAL, (const wchar_t *)0);
}
inline void api_configitem::SetStringInternal(const wchar_t *internalString)
{
	_voidcall(API_CONFIGITEM_SETSTRINGINTERNAL, internalString);
}

inline float api_configitem::GetFloat()
{
	return _call(API_CONFIGITEM_GETFLOAT, (float)0);
}

	inline void api_configitem::SetFloat(float floatValue)
		{
	_voidcall(API_CONFIGITEM_SETFLOAT, floatValue);
}


#endif