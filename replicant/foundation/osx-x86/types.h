#ifndef WASABI2_FOUNDATION_TYPES_H
#define WASABI2_FOUNDATION_TYPES_H
#pragma once

// first, some standard int types
typedef unsigned int UINT;
typedef signed int SINT;

typedef unsigned char UCHAR;
typedef signed char SCHAR;

typedef unsigned long ARGB32;
typedef unsigned long RGB32;

typedef unsigned long ARGB24;
typedef unsigned long RGB24;

typedef unsigned short ARGB16;
typedef unsigned short RGB16;

typedef unsigned long FOURCC;

typedef char nsxml_char_t;
typedef char ns_char_t;
typedef char nsfilename_char_t;

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// this is for GUID == and !=
#include <objbase.h>
#ifndef GUID_EQUALS_DEFINED
#define GUID_EQUALS_DEFINED
#endif //GUID_EQUALS_DEFINED


#ifdef NULL
#undef NULL
#endif //NULL
#ifndef NULL
#define NULL 0
#endif //NULL

#endif //WIN32

#ifndef GUID_DEFINED
  #define GUID_DEFINED

  typedef struct _GUID 
	{
		uint32_t Data1;
		uint16_t Data2;
		uint16_t Data3;
		uint8_t Data4[8];
	} GUID;
/*
#ifndef _REFCLSID_DEFINED
#define REFGUID const GUID &
#define _REFCLSID_DEFINED
#endif
*/
#endif //GUID_DEFINED


#endif
