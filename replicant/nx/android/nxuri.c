#include "nxuri.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "foundation/atomics.h"
#include "foundation/error.h"
#include <android/log.h>

	static size_t nonce=0;

static nx_uri_t nx_temp_path = 0;
nx_uri_t NXURIRetain(nx_uri_t string)
{
	if (!string)
		return 0;
    
	nx_atomic_inc(&string->ref_count);
	return string;
}

void NXURIRelease(nx_uri_t string)
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
static size_t NXURIMallocSize(size_t characters)
{
	return sizeof(nx_uri_struct_t) + (characters) * sizeof(char);
}


/* don't include null terminator here */
nx_uri_t NXURIMalloc(size_t characters)
{
	nx_uri_t str = (nx_uri_t)malloc(NXURIMallocSize(characters));
	if (str)
	{
		str->ref_count = 1;
		str->len = characters;
	}
	return str;
}

int NXURIGetFilename(nx_uri_t filename, nsfilename_char_t *buffer, size_t buffer_length, nsfilename_char_t **out_filename)
{
	*out_filename = filename->string;
	return NErr_Success;
}

int NXURICreateWithJString(JNIEnv *env, jstring jstr, nx_uri_t *out_nxstring)
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
	nx_uri_t ret = NXURIMalloc(utf8_length);
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

int NXURIGetNXString(nx_string_t *string, nx_uri_t uri)
{
	*string = (nx_string_t)NXURIRetain(uri);
	return NErr_Success;
}

int NXURICreateWithPath(nx_uri_t *uri, const nx_uri_t filename, const nx_uri_t path)
{
	size_t filename_length = filename->len;
	size_t path_length = path->len;
	size_t total_length = filename_length + path_length; /* TODO: check for overflow */
	int need_slash = 1; 
	nx_uri_t output=0;
	if (path_length && (path->string[path_length-1] == '/' || path->string[path_length-1] == '\\'))
	{
		need_slash=0;
	}
	else
	{
		total_length++; /* TODO: check for overflow */
	}

	output = NXURIMalloc(total_length);
	if (!output)
		return NErr_OutOfMemory;

	memcpy(output->string, path->string, path_length);
	if (need_slash)
	{
		output->string[path_length]='/'; 
		strcpy(&output->string[path_length+1], filename->string);
	}
	else
	{
		strcpy(&output->string[path_length], filename->string);
	}

	*uri = output;
	return NErr_Success;
}

int NXURICreateWithNXString(nx_uri_t *new_value, nx_string_t string)
{
	if (!string)
		return NErr_Empty;

	*new_value = NXURIRetain((nx_uri_t)string);
	return NErr_Success;
}

static const char *FindFilename(nx_uri_t filename)
{
	size_t position;
	if (!filename || !filename->string || !filename->len)
		return 0;

	position=filename->len;
	while (position--)
	{
		char c = filename->string[position];
		if (c == '/' || c == '\\')
			return &filename->string[position+1];
	}
	return 0;
}

int NXURICreateTempForFilepath(nx_uri_t *out_temp, nx_uri_t filename)
{
	nx_uri_t new_uri;
	size_t path_length;
	char temp_part[128];
	struct timespec ts;
	int temp_length;
	uint64_t count;
	size_t this_nonce;
	const char *filepart;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
  count=(uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;

	this_nonce = nx_atomic_inc(&nonce);
	temp_length = sprintf(temp_part, ".%x-%llx-%d.tmp", gettid(), count, this_nonce);
	filepart = FindFilename(filename);
	if (filepart)
	{
		path_length = (filepart - filename->string);
	}
	else
	{
		path_length=0;
	}
	new_uri = NXURIMalloc(path_length+temp_length);
	if (!new_uri)
		return NErr_OutOfMemory;
	memcpy(new_uri->string, filename->string, path_length);
	memcpy(new_uri->string+path_length, temp_part, temp_length);
	new_uri->string[path_length+temp_length]=0;
	*out_temp = new_uri;
	return NErr_Success;
}

int NXURICreateWithUTF8(nx_uri_t *value, const char *utf8)
{
	nx_string_t nx_filename;
	nx_uri_t uri_filename;

	int ret = NXStringCreateWithUTF8(&nx_filename, utf8);
	if (ret != NErr_Success)
		return ret;

	ret = NXURICreateWithNXString(&uri_filename, nx_filename);
	NXStringRelease(nx_filename);
	if (ret != NErr_Success)
		return ret;
	
	*value = uri_filename;
	return NErr_Success;
}

int NXURICreateRemovingFilename(nx_uri_t *out_uri, nx_uri_t filename)
{
	nx_uri_t new_uri;
	size_t path_length;
	
	const char *filepart = FindFilename(filename);
	if (filepart)
	{
		path_length = (filepart - filename->string);
	}
	else
	{
		path_length=0;
	}
	new_uri = NXURIMalloc(path_length);
	if (!new_uri)
		return NErr_OutOfMemory;
	memcpy(new_uri->string, filename->string, path_length);
	new_uri->string[path_length]=0;
	*out_uri = new_uri;
	return NErr_Success;
}

int NXURICreateTemp(nx_uri_t *out_temp)
{
	return NXURICreateTempWithExtension(out_temp, "tmp");
}

int NXURICreateTempWithExtension(nx_uri_t *out_temp, const char *extension)
{

	if (!nx_temp_path)
		return NErr_Empty;

	char temp_part[256];
	struct timespec ts;
	uint64_t count;
	const char *filepart;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
  count=(uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;

	size_t this_nonce = nx_atomic_inc(&nonce);
	sprintf(temp_part, ".%x-%llx-%u.%s", gettid(), count, this_nonce, extension);

	nx_uri_t nx_tempfilespec;
	int ret = NXURICreateWithUTF8(&nx_tempfilespec, temp_part);
	if (ret != NErr_Success)
		return ret;

	ret = NXURICreateWithPath(out_temp, nx_tempfilespec, nx_temp_path);
	NXURIRelease(nx_tempfilespec);
	return ret;
}

int NXURISetTempPath(nx_uri_t path)
{
	/* note: not thread safe! */
	NXURIRelease(nx_temp_path);
	nx_temp_path=NXURIRetain(path);
	return NErr_Success;
}