#pragma once

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <sys/endian.h>


#ifndef GUID_DEFINED
  #define GUID_DEFINED

  typedef struct _GUID 
	{
		uint32_t Data1;
		uint16_t Data2;
		uint16_t Data3;
		uint8_t Data4[8];
	} GUID;
#endif


#ifdef NULL
  #undef NULL
#endif
#ifndef NULL
  #define NULL 0
#endif

typedef uint32_t FOURCC;
typedef int64_t intptr2_t;
typedef char nsxml_char_t;
typedef char ns_char_t;
typedef char nsfilename_char_t;