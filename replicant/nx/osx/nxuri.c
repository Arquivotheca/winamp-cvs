#include "nxuri.h"
#include "foundation/error.h"
#include "nxpath.h"

nx_uri_t NXURIRetain(nx_uri_t string)
{
	if (string)
		return (nx_uri_t)CFRetain(string);
	else
		return 0;
}

void NXURIRelease(nx_uri_t string)
{
	if (string)
		CFRelease(string);
}


int NXURIGetFilename(nx_uri_t filename, nsfilename_char_t *buffer, size_t buffer_length, nsfilename_char_t **out_filename)
{
	/* TODO: various error checking */
	if (false == CFURLGetFileSystemRepresentation(filename, 0, (UInt8 *)buffer, buffer_length))
	{
		buffer[0] = '\0';
		if (out_filename != NULL)
			*out_filename = NULL;
		return NErr_Error;
	}

	if (out_filename != NULL)
		*out_filename = buffer;
	
	return NErr_Success;
}

int NXURICreateWithPath(nx_uri_t *uri, const nx_uri_t filename, const nx_uri_t path)
{
    *uri=CFURLCreateCopyAppendingPathComponent(NULL, path, CFURLGetString(filename), false);
    return NErr_Success;
}

int NXURICreateWithNXString(nx_uri_t *new_value, nx_string_t string)
{
    *new_value = CFURLCreateWithFileSystemPath(NULL, string, kCFURLPOSIXPathStyle, false);
    return NErr_Success;
}

int NXURIGetNXString(nx_string_t *string, nx_uri_t uri)
{
    *string = (nx_string_t)CFRetain(CFURLGetString(uri));
    return NErr_Success;
}


int NXURICreateTempForDirectory(nx_uri_t *out_temp, nx_uri_t directory)
{
	if (NULL == out_temp)
		return NErr_BadParameter;
	
	char buffer[PATH_MAX];
	strlcpy(buffer, "replicant_XXXXXX.tmp", sizeof(buffer));
	
	if (NULL ==  mktemp(buffer))
		return NErr_Error;
	
	CFStringRef temp_name = CFStringCreateWithFileSystemRepresentation(NULL, buffer);
	CFURLRef URL = CFURLCreateCopyAppendingPathComponent(NULL, directory, temp_name, false);
	CFRelease(temp_name);
	
	if (NULL == URL)
		return NErr_Error;
	
	*out_temp = URL;
	
	return NErr_Success;
}


int NXURICreateTempForFilepath(nx_uri_t *out_temp, nx_uri_t filename)
{
	if (false == CFURLHasDirectoryPath(filename))
	{
		filename = CFURLCreateCopyDeletingLastPathComponent(NULL, filename);
		if (NULL == filename)
			return NErr_Success;
	}
	
	return NXURICreateTempForDirectory(out_temp, filename);
}

int NXURICreateTemp(nx_uri_t *out_temp)
{
	nx_uri_t temporary_dir;
	ns_error_t err = NXPathGetTemporaryDirectory(&temporary_dir);
	if (NErr_Success != err)
		return err;
	
	return NXURICreateTempForDirectory(out_temp, temporary_dir);
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

NX_API int NXURICreateRemovingFilename(nx_uri_t *out_uri, nx_uri_t filename)
{
	if (NULL == out_uri)
		return NErr_BadParameter;
	
	*out_uri = CFURLCreateCopyDeletingLastPathComponent(NULL, filename);
	return NErr_Success;
}