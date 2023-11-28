#include "nxstring.h"
#include "foundation/atomics.h"
#include <stdlib.h>
#include "nu/utf.h"
#include "foundation/error.h"
#include <stdio.h>

nx_string_t NXStringRetain(nx_string_t string)
{
	if (!string)
		return 0;

	nx_atomic_inc(&string->ref_count);
	return string;
}

void NXStringRelease(nx_string_t string)
{
	if (string)
	{
		if (nx_atomic_dec(&string->ref_count) == 0)
		{
			free(string);
		}
	}
}

/* don't include null terminator here */
static size_t NXStringMallocSize(size_t characters)
{
	/* TODO: overflow check? */
	const nx_string_t dummy=0;
	size_t header = (size_t)&dummy->string[0] - (size_t)dummy;
	return header + (characters+1) * sizeof(char);
}

/* don't include null terminator here */
nx_string_t NXStringMalloc(size_t characters)
{
	nx_string_t str = (nx_string_t)malloc(NXStringMallocSize(characters));
	if (str)
	{
		str->ref_count = 1;
		str->len = characters;
	}
	return str;
}

nx_string_t NXStringRealloc(nx_string_t str, size_t characters)
{
	nx_string_t new_str = (nx_string_t)realloc(str, NXStringMallocSize(characters));
	return new_str;
}

int NXStringCreateEmpty(nx_string_t *new_string)
{
	nx_string_t nxstr = NXStringMalloc(0);
	if (nxstr)
	{
		nxstr->string[0]=0;
		*new_string = nxstr;
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

nx_string_t NXStringCreateFromUTF8(const char *str)
{
	size_t size;
	nx_string_t nxstr;
	if (!str)
		return 0;

	size=strlen(str);
	nxstr=NXStringMalloc(size);
	if (nxstr)
	{
		memcpy(nxstr->string, str, size);
		nxstr->string[size]=0;
		nxstr->len = size;
	}
	return nxstr;
}

int NXStringCreateWithUTF8(nx_string_t *new_value, const char *str)
{
	size_t size;
	nx_string_t nxstr;
	if (!str)
		return NErr_Empty;
	size = strlen(str);
	nxstr = NXStringMalloc(size);
	if (!nxstr)
		return NErr_OutOfMemory;
	memcpy(nxstr->string, str, size);
	nxstr->string[size]=0;
	nxstr->len = size;
	*new_value = nxstr;
	return NErr_Success;
}

int NXStringCreateWithCString(nx_string_t *new_value, const char *str, nx_charset_t charset)
{
	if (!str)
		return NErr_Empty;

	switch(charset)
	{
	case nx_charset_ascii:
	case nx_charset_latin1:
		return NXStringCreateWithBytes(new_value, str, strlen(str), charset);
	case nx_charset_utf8:
		return NXStringCreateWithUTF8(new_value, str);
	}
	return NErr_Unknown;
}

nx_string_t NXStringCreateFromUInt64(uint64_t value)
{
	nx_string_t intstr = NXStringMalloc(21); 
	if (intstr)
	{
		sprintf(intstr->string, "%llu", value);
		intstr->len = strlen(intstr->string);
	}
	return intstr;
}

int NXStringCreateWithUInt64(nx_string_t *new_value, uint64_t value)
{
	nx_string_t intstr = NXStringMalloc(21); 
	if (!intstr)
		return NErr_OutOfMemory;

	sprintf(intstr->string, "%llu", value);
	intstr->len = strlen(intstr->string);
	*new_value = intstr;
	return NErr_Success;
}

int NXStringCreateWithInt64(nx_string_t *new_value, int64_t value)
{
	nx_string_t intstr = NXStringMalloc(22); 
	if (!intstr)
		return NErr_OutOfMemory;

	sprintf(intstr->string, "%lld", value);
	intstr->len = strlen(intstr->string);
	*new_value = intstr;
	return NErr_Success;
}

size_t NXStringGetLength(nx_string_t string)
{
	return string->len;
}

/* --- Keyword (ASCII) comparison --- */
int NXStringKeywordCompareWithCString(nx_string_t string, const char *compare_to)
{
	return strcasecmp(string->string, compare_to);
}

int NXStringKeywordCaseCompare(nx_string_t string, nx_string_t compare_to)
{
	const char *src;
	const char *dst;
	int ret = 0 ;

	if (!string)
		return 1;
	if (!compare_to)
		return -1;

  src = string->string;
  dst = compare_to->string;	

	while( ! (ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int NXStringKeywordCompare(nx_string_t string, nx_string_t compare_to)
{
	const char *src;
	const char *dst;
	int ret = 0 ;

	if (!string)
		return 1;
	if (!compare_to)
		return -1;

  src = string->string;
  dst = compare_to->string;	

	while( ! (ret = (int)((*src & ~0x20) - (*dst & ~0x20))) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int NXStringCreateWithBytes(nx_string_t *new_string, const void *data, size_t len, nx_charset_t charset)
{
	switch(charset)
	{
	case nx_charset_ascii:
		{
			size_t character_count = ASCII_to_utf8((const char *)data, len, 0, 0);
			if (!character_count)
				return NErr_Error;

			nx_string_t ret = NXStringMalloc(character_count);
			if (ret)
			{
				ASCII_to_utf8((const char *)data, len, ret->string, character_count);
				ret->string[character_count]=0;
				ret->len = character_count;
				*new_string = ret;
				return NErr_Success;
			}
			else
				return NErr_OutOfMemory;

		}
		break;
	case nx_charset_latin1:
		{
			size_t character_count = ISO_8859_1_to_utf8((const char *)data, len, 0, 0);
			if (!character_count)
				return NErr_Error;

			nx_string_t ret = NXStringMalloc(character_count);
			if (ret)
			{
				ISO_8859_1_to_utf8((const char *)data, len, ret->string, character_count);
				ret->string[character_count]=0;
				ret->len = character_count;
				*new_string = ret;
				return NErr_Success;
			}
			else
				return NErr_OutOfMemory;

		}
		break;
	case nx_charset_system:
		// TODO!!
		break;
	case nx_charset_utf8:
		{
			nx_string_t ret = NXStringMalloc(len);
			if (ret)
			{
				memcpy(ret->string, data, len);
				ret->string[len]=0;
				ret->len = len;
				*new_string = ret;
				return NErr_Success;
			}
			else
				return NErr_OutOfMemory;
		}
		break;

	case nx_charset_utf16le:
		{
			if (len & 1)
				return NErr_Error;
			len/=2;
			size_t character_count = utf16LE_to_utf8((const uint16_t *)data, len, 0, 0);
			if (!character_count)
				return NErr_Error;
			nx_string_t ret = NXStringMalloc(character_count);
			if (ret)
			{
				utf16LE_to_utf8((const uint16_t *)data, len, ret->string, character_count);
				ret->string[character_count]=0;
				ret->len = character_count;
				*new_string = ret;
				return NErr_Success;
			}
			else
				return NErr_OutOfMemory;
		}
		break;

	case nx_charset_utf16be:
		{
			if (len & 1)
				return NErr_Error;
			len/=2;
			size_t character_count = utf16BE_to_utf8((const uint16_t *)data, len, 0, 0);
			if (!character_count)
				return NErr_Error;
			nx_string_t ret = NXStringMalloc(character_count);
			if (ret)
			{
				utf16BE_to_utf8((const uint16_t *)data, len, ret->string, character_count);
				ret->string[character_count]=0;
				ret->len = character_count;
				*new_string = ret;
				return NErr_Success;
			}
			else
				return NErr_OutOfMemory;
		}
		break;
	}

	return NErr_Unknown;
}

nx_string_t NXStringCreateFromPath(const char *folder, const char *filename)
{
	size_t i = 0;
	size_t na = strlen(folder);
	size_t nb = strlen(filename);
	size_t extra=0;
	nx_string_t retstr;
	char *r;
	if (na && folder[na-1] != '/')
		extra=1;

	retstr = NXStringMalloc(na + nb + extra);
	if (!retstr)
		return 0;
	r = retstr->string;

	for (i = 0; i < na; i++) 
		r[i] = folder[i];

	if (extra)
		r[na] = '/';

	for (i = 0; i < nb; i++) 
		r[na + extra + i] = filename[i];

	r[na+nb+extra]=0;

	retstr->len = na+nb+extra;
	return retstr;
}

int NXStringCreateWithJString(JNIEnv *env, jstring jstr, nx_string_t *out_nxstring)
{
	/* TODO: error checking */
	if (!jstr)
		return NErr_NullPointer;

	int i;
	jboolean is_copy=JNI_FALSE;
	jsize utf16_length = (*env)->GetStringLength(env, jstr);
	if (!utf16_length)
		return NErr_Empty;
	const jchar *utf16_bytes = (*env)->GetStringChars(env, jstr, &is_copy);
	if (!utf16_bytes)
		return NErr_Error;

	size_t utf8_length = utf16LE_to_utf8(utf16_bytes, utf16_length, 0, 0);
	nx_string_t ret = NXStringMalloc(utf8_length);
	if (!ret)
	{
		(*env)->ReleaseStringChars(env, jstr, utf16_bytes);
		return NErr_OutOfMemory;
	}
	ret->len = utf8_length;
	utf16LE_to_utf8(utf16_bytes, utf16_length, ret->string, utf8_length);
	(*env)->ReleaseStringChars(env, jstr, utf16_bytes);
	ret->string[utf8_length]=0;
	*out_nxstring = ret;
	return NErr_Success;
}


int NXStringCreateJString(JNIEnv *env, nx_string_t str, jstring *out_jstring)
{
	if (!str)
		return NErr_NullPointer;

	size_t r = utf8_to_utf16LE(str->string, str->len, 0, 0);
	if (!r)
		return NErr_Empty;

	if (r < 1024) // optimization for small strings
	{
		uint16_t utf16[1024];
		utf8_to_utf16LE(str->string, str->len, utf16, r);

		jstring jret = (*env)->NewString(env, utf16, r);
		if (!jret)
			return NErr_OutOfMemory;

		*out_jstring = jret;
		return NErr_Success;
	}
	else // large strings, use malloc to be safe
	{
		uint16_t *utf16 = malloc(r*2);
		if (!utf16)
			return NErr_OutOfMemory;
		utf8_to_utf16LE(str->string, str->len, utf16, r);

		jstring jret = (*env)->NewString(env, utf16, r);
		free(utf16);
		if (!jret)
			return NErr_OutOfMemory;
		*out_jstring = jret;
		return NErr_Success;
	}
}

int NXStringCreateBasePathFromFilename(nx_string_t filename, nx_string_t *basepath)
{
	size_t len = filename->len;
	while (len && filename->string[len-1] != '/')
		len--;

	if (!len)
		return NErr_Empty;

	nx_string_t str = NXStringMalloc(len);
	if (!str)
		return NErr_OutOfMemory;

	memcpy(str->string, filename->string, len);
	str->string[len]=0;
	*basepath = str;
	return NErr_Success;
}

int NXStringGetCString(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length)
{
	if (!string)
		return NErr_NullPointer;

	*out_cstring = string->string;
	*out_cstring_length = string->len;
	return NErr_Success;
}

int NXStringGetDoubleValue(nx_string_t string, double *value)
{
	if (!string)
		return NErr_NullPointer;

	*value = strtod(string->string, 0);
	return NErr_Success;
}

int NXStringGetBytesSize(size_t *byte_count, nx_string_t string, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf8)
	{
		if (flags & nx_string_get_bytes_size_null_terminate)
			*byte_count = string->len + 1;
		else
			*byte_count = string->len;
		return NErr_DirectPointer;
	}
	else if (charset == nx_charset_utf16be)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*byte_count = utf8_to_utf16BE(string->string, len, 0, 0) * 2;
		return NErr_Success;
	}
	else if (charset == nx_charset_utf16le)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*byte_count = utf8_to_utf16LE(string->string, len, 0, 0) * 2;
		return NErr_Success;
	}
	else if (charset == nx_charset_latin1)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*byte_count = utf8_to_ISO_8859_1(string->string, len, 0, 0);
		return NErr_Success;
	}
	else if (charset == nx_charset_ascii)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*byte_count = utf8_to_ASCII(string->string, len, 0, 0);
		return NErr_Success;
	}
	else
		return NErr_NotImplemented; // TODO
}

int NXStringGetBytesDirect(const void **bytes, size_t *length, nx_string_t string, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf8)
	{
		if (length)
		{
			if (flags & nx_string_get_bytes_size_null_terminate)
				*length = string->len + 1;
			else
				*length = string->len;
		}
		*bytes = string->string;
		return NErr_DirectPointer;
	}
	else
		return NErr_NotImplemented; // TODO
}

int NXStringGetBytes(size_t *bytes_copied, nx_string_t string, void *bytes, size_t length, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf8)
	{
		if (flags & nx_string_get_bytes_size_null_terminate)
		{
			if (length == 0)
				return NErr_Insufficient;

			size_t copy_count = length-1;
			if (copy_count > string->len)
				copy_count = string->len;

			memcpy(bytes, string->string, copy_count);
			((char *)bytes)[copy_count]=0;
			*bytes_copied = copy_count+1;
			return NErr_Success;
		}
		else
		{
			size_t copy_count = length;
			if (copy_count > string->len)
				copy_count = string->len;

			memcpy(bytes, string->string, copy_count);
			*bytes_copied = copy_count;
			return NErr_Success;
		}
	}
	else if (charset == nx_charset_utf16be)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*bytes_copied = utf8_to_utf16BE(string->string, len,bytes, length/2) * 2;
		return NErr_Success;
	}
	else if (charset == nx_charset_utf16le)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*bytes_copied = utf8_to_utf16LE(string->string, len, bytes, length/2) * 2;
		return NErr_Success;
	}
	else if (charset == nx_charset_latin1)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*bytes_copied = utf8_to_ISO_8859_1(string->string, len, bytes, length);
		return NErr_Success;
	}
	else if (charset == nx_charset_ascii)
	{
		size_t len = string->len;
		if (flags & nx_string_get_bytes_size_null_terminate)
			len++;
		*bytes_copied = utf8_to_ASCII(string->string, len, bytes, length);
		return NErr_Success;
	}
	else
		return NErr_NotImplemented; // TODO
}

int NXStringGetIntegerValue(nx_string_t string, int *value)
{
	*value = strtol(string->string, 0, 10);
	return NErr_Success;
}

nx_compare_result NXStringCompare(nx_string_t string1, nx_string_t string2, nx_compare_options options)
{
	if (0 != (nx_compare_case_insensitive & options))
		return strcasecmp(string1->string, string2->string);
	
	return strcmp(string1->string, string2->string);
}

int NXStringCreateWithFormatting(nx_string_t *new_string, const char *format, ...)
{
	char *temp;
	int cch;
	va_list v;
	int ret;

	va_start(v, format);

	cch = vasprintf(&temp, format, v);
	if (cch == -1)
		return NErr_Error;

	ret  = NXStringCreateWithUTF8(new_string, temp);
	free(temp);
	va_end(v);
}
