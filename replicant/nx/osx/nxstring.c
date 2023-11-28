#include "nxstring.h"
#include "foundation/error.h"

nx_string_t NXStringRetain(nx_string_t string)
{
    if (string)
        return (nx_string_t)CFRetain(string);
    else
        return 0;
}

void NXStringRelease(nx_string_t string)
{
    if (string)
        CFRelease(string);
}

ns_error_t NXStringCreateWithUTF8(nx_string_t *new_value, const char *str)
{
    *new_value = CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8);
    return NErr_Success;
}

ns_error_t NXStringCreateWithUTF16(nx_string_t *new_value, const UniChar *str)
{
	CFIndex numChars = 0;
	for(const UniChar *p = str; 0 != p; p++)
	{
		numChars++;
	}
	
	*new_value = CFStringCreateWithCharacters(kCFAllocatorDefault, str, numChars);
    return NErr_Success;
}

ns_error_t NXStringCreateWithBytes(nx_string_t *new_string, const void *data, size_t len, nx_charset_t charset)
{
    CFStringRef cfstr = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)data, len, charset, false);
    if (!cfstr)
        return NErr_OutOfMemory; /* TODO: maybe there's other reasons */
	
    *new_string = cfstr;
    return NErr_Success;
}

ns_error_t NXStringCreateEmpty(nx_string_t *new_string)
{
    *new_string=CFSTR("");
    return NErr_Success;
}

ns_error_t NXStringCreateWithInt64(nx_string_t *new_value, int64_t value)
{
    CFStringRef cfstr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%lld"), value);
    if (!cfstr)
        return NErr_OutOfMemory; /* TODO: maybe there's other reasons */
    
    *new_value = cfstr;
    return NErr_Success;
}

ns_error_t NXStringCreateWithUInt64(nx_string_t *new_value, uint64_t value)
{
    CFStringRef cfstr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%llu"), value);
    if (!cfstr)
        return NErr_OutOfMemory; /* TODO: maybe there's other reasons */
    
    *new_value = cfstr;
    return NErr_Success;
}

ns_error_t NXStringCreateWithCString(nx_string_t *new_value, const char *str, nx_charset_t charset)
{
    CFStringRef cfstr = CFStringCreateWithCString(NULL, str, charset);
    if (!cfstr)
        return NErr_OutOfMemory; /* TODO: maybe there's other reasons */
    
    *new_value = cfstr;
    return NErr_Success;
}

ns_error_t NXStringCreateWithFormatting(nx_string_t *new_string, const char *format, ...)
{
	char *temp;
	int cch;
	va_list v;
	ns_error_t	err;
	
	va_start(v, format);
	
	cch = vasprintf(&temp, format, v);
	if (cch == -1)
		return NErr_Error;
	
	va_end(v);
	
	err = NXStringCreateWithUTF8(new_string, temp);
	free(temp);
	
	return err;
}

size_t NXStringGetLength(nx_string_t string)
{
	return CFStringGetLength(string);
}

ns_error_t NXStringGetDoubleValue(nx_string_t string, double *value)
{
    if (!string)
        return NErr_Empty;
    
    *value = CFStringGetDoubleValue(string);
    return NErr_Success;
}

const char * NXStringGetCStringPtrUsingCharset(nx_string_t string, nx_charset_t charset)
{
	return CFStringGetCStringPtr(string, charset);
}

ns_error_t NXStringGetCStringCopyUsingCharset(nx_string_t string, nx_charset_t charset, char *user_buffer, size_t user_buffer_length)
{
    if (false != CFStringGetCString(string, user_buffer, user_buffer_length, charset))
		return NErr_Success;
	
	return NErr_Error;
}

ns_error_t NXStringGetCStringUsingCharset(nx_string_t string, nx_charset_t charset, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length)
{
    const char *quick_ptr = NXStringGetCStringPtrUsingCharset(string, charset);
    if (quick_ptr)
    {
        *out_cstring = quick_ptr;
        if (out_cstring_length)
            *out_cstring_length = NXStringGetLength(string);
        return NErr_Success;
    }
	
    ns_error_t err = NXStringGetCStringCopyUsingCharset(string, charset, user_buffer, user_buffer_length);
	if (NErr_Success == err)
    {
        *out_cstring = user_buffer;
        if (out_cstring_length)
            *out_cstring_length = NXStringGetLength(string);
    }
	
    return err;
}

const char * NXStringGetCStringPtr(nx_string_t string)
{
	return NXStringGetCStringPtrUsingCharset(string, nx_charset_ascii);
}

ns_error_t NXStringGetCStringCopy(nx_string_t string, char *user_buffer, size_t user_buffer_length)
{
	return NXStringGetCStringCopyUsingCharset(string, nx_charset_ascii, user_buffer, user_buffer_length);
}

int NXStringGetCString(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length)
{
	return NXStringGetCStringUsingCharset(string, nx_charset_ascii, user_buffer, user_buffer_length, out_cstring, out_cstring_length);
}

const char * NXStringGetUTF8StringPtr(nx_string_t string)
{
	return NXStringGetCStringPtrUsingCharset(string, nx_charset_utf8);
}

ns_error_t NXStringGetUTF8StringCopy(nx_string_t string, char *user_buffer, size_t user_buffer_length)
{
	return NXStringGetCStringCopyUsingCharset(string, nx_charset_utf8, user_buffer, user_buffer_length);
}

int NXStringGetUTF8String(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length)
{
	return NXStringGetCStringUsingCharset(string, nx_charset_utf8, user_buffer, user_buffer_length, out_cstring, out_cstring_length);
}

const UniChar *NXStringGetCharactersPtr(nx_string_t string)
{
	return CFStringGetCharactersPtr(string);
}

void NXStringGetCharacters(nx_string_t string, long first, long length, UniChar *buffer)
{
	CFRange range = CFRangeMake(first, length);
	CFStringGetCharacters(string, range, buffer);
}

/* --- Keyword (ASCII) comparison --- */
int NXStringKeywordCompare(nx_string_t string, nx_string_t compare_to)
{
	CFComparisonResult result = CFStringCompareWithOptionsAndLocale(string, 
																	compare_to, 
																	CFRangeMake(0, CFStringGetLength(string)), 
																	kCFCompareCaseInsensitive, 
																	NULL);
	if (kCFCompareEqualTo == result)
		return NErr_True;
	
	return NErr_False;
}

int NXStringKeywordCompareWithCString(nx_string_t string, const char *compare_to)
{
	const UniChar *src = CFStringGetCharactersPtr(string);
	if (NULL != src)
	{
		const char *dst = compare_to;
		
		int ret = 0 ;
		
		while( ! (ret = (int)((*src & ~0x20) - (wchar_t)(*dst & ~0x20))) && *dst)
			++src, ++dst;
		
		if ( ret < 0 )
			ret = -1 ;
		else if ( ret > 0 )
			ret = 1 ;
		
		return( ret );
	}
	else 
	{
		nx_string_t dst;
		if (NErr_Success != NXStringCreateWithCString(&dst, compare_to, nx_charset_ascii))
			return NErr_False;
		
		CFComparisonResult result = CFStringCompareWithOptionsAndLocale(string, 
																		dst, 
																		CFRangeMake(0, CFStringGetLength(string)), 
																		kCFCompareCaseInsensitive, 
																		NULL);
		
		NXStringRelease(dst);
		
		if (kCFCompareEqualTo == result)
			return NErr_True;
	}

	return NErr_False;
}

int NXStringKeywordCaseCompare(nx_string_t string, nx_string_t compare_to)
{
	CFComparisonResult result = CFStringCompareWithOptionsAndLocale(string, 
																	compare_to, 
																	CFRangeMake(0, CFStringGetLength(string)), 
																	0, 
																	NULL);
	if (kCFCompareEqualTo == result)
		return NErr_True;
	
	return NErr_False;
}

int NXStringKeywordCaseCompareWithCString(nx_string_t string, const char *compare_to)
{
	const UniChar *src = CFStringGetCharactersPtr(string);
	if (NULL != src)
	{
		const char *dst = compare_to;
		
		int ret = 0 ;
		
		while( ! (ret = (int)(*src - (wchar_t)*dst)) && *dst)
			++src, ++dst;
		
		if ( ret < 0 )
			ret = -1 ;
		else if ( ret > 0 )
			ret = 1 ;
		
		return( ret );
	}
	else 
	{
		nx_string_t dst;
		if (NErr_Success != NXStringCreateWithCString(&dst, compare_to, nx_charset_ascii))
			return NErr_False;
		
		CFComparisonResult result = CFStringCompareWithOptionsAndLocale(string, 
																		dst, 
																		CFRangeMake(0, CFStringGetLength(string)), 
																		0, 
																		NULL);
		
		NXStringRelease(dst);
		
		if (kCFCompareEqualTo == result)
			return NErr_True;
	}
	
	return NErr_False;
}

ns_error_t NXStringGetIntegerValue(nx_string_t string, int *value)
{
	*value = CFStringGetIntValue(string);
	return NErr_Success;
}

ns_error_t NXStringGetBytesSize(size_t *byte_count, nx_string_t string, nx_charset_t charset, int flags)
{
	if (NErr_Success == NXStringGetBytesDirect(NULL, byte_count, string, charset, flags))
		return NErr_DirectPointer;
	
    CFIndex bytes_required = CFStringGetMaximumSizeForEncoding(NXStringGetLength(string), charset);
	if (0 != (nx_string_get_bytes_size_null_terminate & flags))
		bytes_required += 1;
	
	if (NULL != byte_count)
		*byte_count = (size_t)bytes_required;
	
	return NErr_Success;
}

static ns_error_t NXStringGetByteSizeHelper(nx_string_t string, const char *cString, size_t *length, nx_charset_t charset, int flags)
{
		
	if (nx_charset_utf8 == charset)
	{
		size_t bytes_count = strlen(cString);
		if (0 != (nx_string_get_bytes_size_null_terminate & flags))
			bytes_count += 1;
		*length = bytes_count;
		return NErr_Success;
	}	
	
	if (nx_charset_utf16be == charset
		|| nx_charset_utf16le == charset)
	{
		size_t bytes_count = NXStringGetLength(string);
		if (0 != (nx_string_get_bytes_size_null_terminate & flags))
			bytes_count += 1;
			
		bytes_count = bytes_count * sizeof(UniChar);
		*length = bytes_count;
		return NErr_Success;
	}
	
	if (nx_charset_ascii == charset
		|| nx_charset_latin1 == charset)
	{
		size_t bytes_count = NXStringGetLength(string);
		if (0 != (nx_string_get_bytes_size_null_terminate & flags))
			bytes_count += 1;

		*length = bytes_count;
		return NErr_Success;
	}
	
	CFIndex count = NXStringGetLength(string);
	CFRange range = CFRangeMake(0, count);
	CFIndex bytes_required;
	
	if (count != CFStringGetBytes(string, range, charset, 0, false, NULL, 0, &bytes_required))
		return NErr_Error;
	
	if (0 != (nx_string_get_bytes_size_null_terminate & flags))
		bytes_required += 1;
	
	*length = bytes_required;
	
	return NErr_Success;
	
}

ns_error_t NXStringGetBytesDirect(const void **bytes, size_t *length, nx_string_t string, nx_charset_t charset, int flags)
{
	const char *cString = NXStringGetCStringPtrUsingCharset(string, charset);
	if (NULL == cString)
		return NErr_Error;
	
	if (NULL != bytes)
		*bytes = cString;
	
	ns_error_t err;
	
	if (NULL != length)
		err =  NXStringGetByteSizeHelper(string, cString, length, charset, flags);
	else
		err = NErr_Success;
		
	return err;
}

ns_error_t NXStringGetBytes(size_t *bytes_copied, nx_string_t string, void *bytes, size_t length, nx_charset_t charset, int flags)
{
    
	ns_error_t err = NXStringGetCStringCopyUsingCharset(string, charset, bytes, length);
	if (NErr_Success != err)
		return err;
	
	
	if (NULL != bytes_copied)
		err = NXStringGetByteSizeHelper(string, bytes, bytes_copied, charset, flags);
	
	return err;
}

NX_API size_t NXStringGetMaximumSizeForCharset(size_t length, nx_charset_t charset)
{
	return CFStringGetMaximumSizeForEncoding(length, charset);
}

nx_compare_result NXStringCompare(nx_string_t string1, nx_string_t string2, nx_compare_options options)
{
	CFStringCompareFlags compareFlags = 0;
	
	if (0 != (nx_compare_case_insensitive & options))
		compareFlags |= kCFCompareCaseInsensitive;
	
	return (nx_compare_result)CFStringCompare(string1, string2, compareFlags);
}
