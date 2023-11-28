#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"

/* Linux implementation  */

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum 
	{
		nx_charset_latin1 = 0,
		nx_charset_system = 1,
		nx_charset_utf8 = 2,
		nx_charset_utf16le = 3,
		nx_charset_utf16be = 4,
		nx_charset_ascii = 5,
	} nx_charset_t;

	enum
	{
		nx_compare_less_than = -1,
		nx_compare_equal_to = 0,
		nx_compare_greater_than = 1,
	}; 
	typedef int nx_compare_result;
	
	enum
	{
		nx_compare_default = 0,
		nx_compare_case_insensitive = ( 1 << 0),
	}; 
	typedef unsigned long nx_compare_options;
	
	typedef struct nx_string_struct_t
	{
		size_t ref_count;
		size_t len;
		char string[1]; // utf-8
	} nx_string_struct_t, *nx_string_t;

	NX_API nx_string_t NXStringRetain(nx_string_t string);
	NX_API void NXStringRelease(nx_string_t string);
	NX_API nx_string_t NXStringRealloc(nx_string_t, size_t characters);

	/* Creeation */
	NX_API int NXStringCreateEmpty(nx_string_t *new_string);
	NX_API int NXStringCreateWithUTF8(nx_string_t *new_value, const char *str);
	NX_API int NXStringCreateWithCString(nx_string_t *new_value, const char *str, nx_charset_t charset);
	NX_API int NXStringCreateWithUInt64(nx_string_t *new_value, uint64_t value);
	NX_API int NXStringCreateWithInt64(nx_string_t *new_value, int64_t value);
	NX_API int NXStringCreateWithBytes(nx_string_t *new_string, const void *data, size_t len, nx_charset_t charset);

	/* deprecated Creation functions */

	NX_API nx_string_t NXStringCreateFromPath(const char *folder, const char *filename);
	NX_API nx_string_t NXStringCreateFromUTF8(const char *str);
	NX_API nx_string_t NXStringCreateFromUInt64(uint64_t value);
	
	NX_API size_t NXStringGetLength(nx_string_t string);

	/* returns strcmp style return.	compare_to is treated as an ASCII string.  
	if compare_to has non-ASCII characters, results are undetermined */
	NX_API int NXStringKeywordCompareWithCString(nx_string_t string, const char *compare_to);
	NX_API int NXStringKeywordCompare(nx_string_t string, nx_string_t compare_to);

	/* creates an NXString with the base path from the passed in filename (retains an appended /) */
	NX_API int NXStringCreateBasePathFromFilename(nx_string_t filename, nx_string_t *basepath);

	NX_API int NXStringGetCString(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length);
	NX_API int NXStringGetDoubleValue(nx_string_t string, double *value);
	NX_API int NXStringGetIntegerValue(nx_string_t string, int *value);

	static const int nx_string_get_bytes_size_null_terminate = 1; // pass this to null terminate the string
	/* returns byte count with enough room to store a converted string
	note: if this returns NErr_DirectPointer, you can call NXStringGetBytesDirect to directly retrieve a pointer. */
	NX_API int NXStringGetBytesSize(size_t *byte_count, nx_string_t string, nx_charset_t charset, int flags);
	/* if possible, retrieves a pointer to bytes.  
	the length returned depends on whether or not you passed nx_string_get_bytes_size_null_terminate 
	note: the pointer you get will be invalid after you call NXStringRelease on the string passed in */
	NX_API int NXStringGetBytesDirect(const void **bytes, size_t *length, nx_string_t string, nx_charset_t charset, int flags);
	NX_API int NXStringGetBytes(size_t *bytes_copied, nx_string_t string, void *bytes, size_t length, nx_charset_t charset, int flags);

	NX_API nx_compare_result NXStringCompare(nx_string_t string1, nx_string_t string2, nx_compare_options options);
	
#ifdef __cplusplus
}
#endif