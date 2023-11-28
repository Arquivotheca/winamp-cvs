#include "ComponentManager.h"
#include "foundation/error.h"
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <android/log.h> // TODO: cut once we verify things are working

int ComponentManager::AddComponent(nx_uri_t filename)
{
	int ret;
	void *wacLib = dlopen(filename->string, RTLD_NOW|RTLD_LOCAL);
	if (wacLib)
	{
		GETCOMPONENT_FUNC f= (GETCOMPONENT_FUNC)dlsym(wacLib, "GetWasabi2Component");
		if (f)
		{
			ifc_component *component = f();
			if (component)
			{
								if (component->component_info.wasabi_version != wasabi2_component_version
					|| component->component_info.nx_api_version != nx_api_version
					|| component->component_info.nx_platform_guid != nx_platform_guid)
				{
					// for some reason this crashes
		//dlclose(wacLib);
					return NErr_IncompatibleVersion;
				}

				component->component_info.dl_handle = wacLib;
				component->component_info.filename = NXURIRetain(filename);
				ret = component->Initialize(service_api);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
					// for some reason this crashes
		//dlclose(wacLib);
					return ret;
				}

				/* if the component was added late, we'll need to run some extra stages */
				ret = LateLoad(component);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
					// for some reason this crashes
		//dlclose(wacLib);
					return ret;
				}

				components.push_back(component);
				return NErr_Success;
			}
		}
		// for some reason this crashes
		//dlclose(wacLib);
		return NErr_Error;
	}
	return NErr_FileNotFound;
}

static int match_extension(const char *filename, const char *extension)
{
	size_t a, b;

	if (!filename || !extension)
		return 0;

	a = strlen(filename);
	b = strlen(extension);

	if (!a)
		return 0;

	if (b > a)
		return 0;

	if (filename[a-b-1] != '.')
		return 0;

	do
	{
		if ((filename[--a] & ~0x20) != (extension[--b] & ~0x20))
			return 0;
	} while (b);

	return 1;
}

int ComponentManager::AddDirectory(nx_uri_t directory)
{
	struct dirent *entry;
	DIR *dir = opendir(directory->string);
	if (!dir)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "can't opendir");
		return NErr_FileNotFound;
	}
	while((entry = readdir(dir)))
	{
#if defined(__ARM_ARCH_7A__)
		if (match_extension(entry->d_name, "ARMv7.w6c.so"))
#elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__)
		if (match_extension(entry->d_name, "ARMv5.w6c.so"))
#elif defined(__i386__)
		if (match_extension(entry->d_name, "x86.w6c.so"))
#else
#error port me!
#endif
		{
			nx_string_t nx_filename;
			NXStringCreateWithUTF8(&nx_filename, entry->d_name);
			nx_uri_t uri_filename;
			NXURICreateWithNXString(&uri_filename, nx_filename);
			NXStringRelease(nx_filename);
			nx_uri_t w6c_filepath;
			NXURICreateWithPath(&w6c_filepath, uri_filename, directory);
			NXURIRelease(uri_filename);
			int ret = AddComponent(w6c_filepath);
			if (ret == NErr_Success)
				__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[ComponentManager] successfully loaded %s", w6c_filepath->string);
			else if (ret == NErr_OSNotSupported)
				__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[ComponentManager] skipping %s", w6c_filepath->string);
			else
				__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[ComponentManager] FAILED loading %s", w6c_filepath->string);
			NXURIRelease(w6c_filepath);
		}
	}

	closedir(dir);
	return NErr_Success;
}



void ComponentManager::CloseComponent(ifc_component *component)
{
	// for some reason this crashes
		//dlclose(component->component_info.dl_handle);
}