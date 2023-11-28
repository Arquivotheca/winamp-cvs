//
//  main.cpp
//  test
//
//  Created by Maksim Tyrtyshny on 4/4/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//

#include <iostream>
#include "../../osx/ComponentManager.h"

#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <pwd.h>
#include <CoreFoundation/CoreFoundation.h>

#include "Wasabi/Wasabi.h"
#include "Wasabi/api.h"

ComponentManager manager;

static CFURLRef 
CFURLCreateForUserHome(uid_t user_id)
{
	long buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX);
	if(buffer_len < 1)
		buffer_len = 4096;
	
	char *buffer = (char *)malloc(sizeof(char) * buffer_len);
	if (NULL == buffer)
		return NULL;
	
	struct passwd *pw, passwd;
	
	CFURLRef user_home_uri;
	
	if (0 == getpwuid_r(user_id, &passwd, buffer, buffer_len, &pw)
		&& NULL != pw)
	{
		user_home_uri = CFURLCreateFromFileSystemRepresentation(NULL, 
																(const UInt8 *)pw->pw_dir, 
																(CFIndex)strlen(pw->pw_dir), 
																true);
	}
	else 
	{
		user_home_uri = NULL;
	}
	
	free(buffer);
	
	return user_home_uri;

}

static CFURLRef 
CFURLCreateForCurrentUserHome()
{
	return CFURLCreateForUserHome(geteuid());
}

static CFStringRef 
CFStringCreateByReplacingTilde(CFStringRef string)
{
    if (false == CFStringHasPrefix(string,  CFSTR("~/")))
	{
		CFRetain(string);
		return string;
	}
		
	CFURLRef user_home_uri = CFURLCreateForCurrentUserHome();
    if (NULL == user_home_uri)
		return NULL;
	
	CFMutableStringRef expanded_string = CFStringCreateMutableCopy(NULL, MAXPATHLEN, string);
	if (NULL != expanded_string)
	{
		CFStringRef user_home_path = CFURLCopyFileSystemPath(user_home_uri, kCFURLPOSIXPathStyle);
		CFStringReplace(expanded_string, CFRangeMake(0, 1), user_home_path);
		CFRelease(user_home_path);
	}
	
	CFRelease(user_home_uri);
	
	return expanded_string;
}


int main(int argc, const char * argv[])
{
	if (Wasabi_Init() != NErr_Success)
		return -1;
	
	manager.SetServiceAPI(WASABI2_API_SVC);
	
	nx_string_t component_directory_path;
	nx_uri_t component_directory_uri;
	int err;
	
	component_directory_path = CFStringCreateByReplacingTilde(CFSTR("~/Projects/replicant-sdk/lib"));
	err = NXURICreateWithNXString(&component_directory_uri, component_directory_path);
	CFRelease(component_directory_path);
	
	if (NErr_Success != err)
		return -1;

	/*
	nx_uri_t component_uri;
	nx_uri_t component_name_uri;
	
	err = NXURICreateWithNXString(&component_name_uri, CFSTR("libmp3.w6c"));
	if (NErr_Success != err)
		return -1;
	
	err = NXURICreateWithPath(&component_uri, component_name_uri, component_directory_uri);
	if (NErr_Success != err)
		return -1;
	
	manager.AddComponent(component_uri);
	*/
	
	manager.AddDirectory(component_directory_uri);
	manager.AddDirectory(component_directory_uri);
	
	std::cout << "Hello, World!\n";
    return 0;
}

