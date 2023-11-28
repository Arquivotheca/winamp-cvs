#pragma once
#include "foundation/types.h"
#include "foundation/error.h"
#include "nx/nxapi.h"
#include <CoreFoundation/CoreFoundation.h>
#ifdef __cplusplus
extern "C" {
#endif

	typedef CFStringRef nx_string_t;

#define nx_charset_system ((nx_charset_t)CFStringGetSystemEncoding())
	typedef enum 
	{
		nx_charset_ascii = kCFStringEncodingASCII,
		nx_charset_latin1 = kCFStringEncodingISOLatin1,
		nx_charset_utf8 = kCFStringEncodingUTF8,
		nx_charset_utf16le = kCFStringEncodingUTF16LE,
		nx_charset_utf16be = kCFStringEncodingUTF16BE,
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
	
	

	NX_API nx_string_t NXStringRetain(nx_string_t string);
	NX_API void NXStringRelease(nx_string_t string);

	/* Creation */
	NX_API ns_error_t NXStringCreateWithUTF8(nx_string_t *new_value, const char *str);
	NX_API ns_error_t NXStringCreateWithUTF16(nx_string_t *new_value, const UniChar *str);
	NX_API ns_error_t NXStringCreateWithBytes(nx_string_t *new_string, const void *data, size_t len, nx_charset_t charset);
	NX_API ns_error_t NXStringCreateEmpty(nx_string_t *new_string);
	NX_API ns_error_t NXStringCreateWithInt64(nx_string_t *new_value, int64_t value);
	NX_API ns_error_t NXStringCreateWithUInt64(nx_string_t *new_value, uint64_t value);
	NX_API ns_error_t NXStringCreateWithCString(nx_string_t *new_value, const char *str, nx_charset_t charset);
	NX_API ns_error_t NXStringCreateWithFormatting(nx_string_t *new_string, const char *format, ...);

	NX_API size_t NXStringGetLength(nx_string_t string);
	
	NX_API ns_error_t NXStringGetDoubleValue(nx_string_t string, double *value);
	NX_API ns_error_t NXStringGetIntegerValue(nx_string_t string, int *value);
	
	NX_API const char * NXStringGetCStringPtrUsingCharset(nx_string_t string, nx_charset_t charset);
	NX_API ns_error_t NXStringGetCStringCopyUsingCharset(nx_string_t string, nx_charset_t charset, char *user_buffer, size_t user_buffer_length);
	NX_API ns_error_t NXStringGetCStringUsingCharset(nx_string_t string, nx_charset_t charset, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length);
	
	/* ascii charset */ 
	NX_API const char * NXStringGetCStringPtr(nx_string_t string);
	NX_API ns_error_t NXStringGetCStringCopy(nx_string_t string, char *user_buffer, size_t user_buffer_length);
	NX_API ns_error_t NXStringGetCString(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length);
	
	/* utf8 charset */ 
	NX_API const char * NXStringGetUTF8StringPtr(nx_string_t string);
	NX_API ns_error_t NXStringGetUTF8StringCopy(nx_string_t string, char *user_buffer, size_t user_buffer_length);
	NX_API ns_error_t NXStringGetUTF8String(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length);

	/* UniChar */
	NX_API const UniChar *NXStringGetCharactersPtr(nx_string_t string);
	NX_API void NXStringGetCharacters(nx_string_t string, long first, long length, UniChar *buffer);


	/* returns strcmp style return.	compare_to is treated as an ASCII string.  
	if compare_to has non-ASCII characters, results are undetermined */
	NX_API int NXStringKeywordCompare(nx_string_t string, nx_string_t compare_to);
	NX_API int NXStringKeywordCompareWithCString(nx_string_t string, const char *compare_to);
	
	NX_API int NXStringKeywordCaseCompare(nx_string_t string, nx_string_t compare_to);
	NX_API int NXStringKeywordCaseCompareWithCString(nx_string_t string, const char *compare_to);

	static const int nx_string_get_bytes_size_null_terminate = 1; // pass this 

	/* returns byte count with enough room to store a converted string
	note: if this returns NErr_DirectPointer, you can call NXStringGetBytesDirect to directly retrieve a pointer. */
	NX_API ns_error_t NXStringGetBytesSize(size_t *byte_count, nx_string_t string, nx_charset_t charset, int flags);
	/* if possible, retrieves a pointer to bytes.  
	the length returned depends on whether or not you passed nx_string_get_bytes_size_null_terminate 
	note: the pointer you get will be invalid after you call NXStringRelease on the string passed in */
	NX_API ns_error_t NXStringGetBytesDirect(const void **bytes, size_t *length, nx_string_t string, nx_charset_t charset, int flags);
	NX_API ns_error_t NXStringGetBytes(size_t *bytes_copied, nx_string_t string, void *bytes, size_t length, nx_charset_t charset, int flags);
	
	NX_API size_t NXStringGetMaximumSizeForCharset(size_t length, nx_charset_t charset);
	
	NX_API nx_compare_result NXStringCompare(nx_string_t string1, nx_string_t string2, nx_compare_options options);
	
#ifdef __cplusplus
}
#endif