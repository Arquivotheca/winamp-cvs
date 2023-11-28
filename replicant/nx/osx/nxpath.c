#include "nxpath.h"
#include "nxfile.h"
#include "foundation/error.h"
#include <CoreFoundation/CoreFoundation.h>

int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension)
{
    CFStringRef filename_extension;
    
    if (NULL == filename 
		|| NULL == extension)
	{
        return NErr_False;
	}
    
    filename_extension=CFURLCopyPathExtension(filename);
    if (filename_extension == NULL)
        return NErr_False;
    
	int err;
	
	CFIndex extension_len = CFStringGetLength(extension);
	CFIndex filename_extension_len = CFStringGetLength(filename_extension);
	
	if (extension_len == filename_extension_len 
		&& kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(extension, filename_extension, 
																	CFRangeMake(0, extension_len), 
																	kCFCompareCaseInsensitive, NULL))
	{
		err = NErr_True;
	}
	else 
	{
		err = NErr_False;
	}
	
	CFRelease(filename_extension);
	return err;
}

int NXPathIsURL(nx_uri_t path)
{
	if (NULL == path)
		return NErr_False;
	
    CFStringRef scheme = CFURLCopyScheme(path);
    if (!scheme)
        return NErr_False;
    
	int err;
	
	const CFStringRef file_scheme_name = CFSTR("file");
	const CFIndex file_scheme_name_length = CFStringGetLength(file_scheme_name);
	
	if (kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(file_scheme_name, scheme, 
															 CFRangeMake(0, file_scheme_name_length), 
															 kCFCompareCaseInsensitive, NULL))
	{
		err = NErr_False;
	}
	else 
	{
		err = NErr_True;
	}
	
	CFRelease(scheme);
	
	return err;
}

static char *nx_temporary_directory_path = NULL;
NX_API ns_error_t NSPathSetTemporaryDirectory(nx_uri_t dir)
{
	free(nx_temporary_directory_path);
	
	if (NULL != dir)
	{
		char *buffer;
		CFStringRef path = CFURLGetString(dir);
		CFIndex buffer_size = CFStringGetMaximumSizeOfFileSystemRepresentation(path);
		buffer = (char *)malloc(buffer_size);
		if (NULL == buffer)
			return NErr_OutOfMemory;
		
		if (false == CFStringGetFileSystemRepresentation(path, buffer, buffer_size))
			return NErr_Error;
		
		CFIndex buffer_real_size = strlen(buffer) + 1;
		if (buffer_real_size < buffer_size)
			buffer = realloc(buffer, buffer_real_size);
		
		nx_temporary_directory_path = buffer;
	}
	else
	{
		nx_temporary_directory_path = NULL;
	}

	return NErr_Success;
}

NX_API ns_error_t NXPathGetTemporaryDirectory(nx_uri_t *dir)
{
	static const char *directory_list[] = 
	{
		NULL,
		NULL,
		"/var/tmp",
		"/usr/tmp",
		"/tmp",
	};
	
	struct stat buffer;
	
	directory_list[0] = nx_temporary_directory_path;
	if (NULL == directory_list[1])
		directory_list[1] = getenv("TMPDIR");
	
	const char *directory_path;
	
	for(unsigned int index = 0; index < sizeof(directory_list)/sizeof(directory_list[0]); index++)
	{
		directory_path = directory_list[index];
		if(NULL == directory_path)
			continue;

		if (0 != stat(directory_path, &buffer))
			continue;
		
		if( false == S_ISDIR(buffer.st_mode))
			continue;
		
		if(0 !=  access(directory_path, 07))
			continue;
		
		break;
	}
	
	if (NULL == directory_path)
		return NErr_Error;
	
	if (NULL != dir)
	{
		CFURLRef URL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*)directory_path, strlen(directory_path), true);
		if (NULL == URL)
			return NErr_Error;
		
		*dir = URL;
	}

	return NErr_Success;
}