#include "ComponentManager.h"
#include "foundation/error.h"
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>

int ComponentManager::AddComponent(nx_uri_t filename)
{
	int ret;
	printf("full: %s\n", filename->string);
	void *wacLib = dlopen(filename->string, RTLD_NOW|RTLD_LOCAL);
	if (wacLib)
	{
		printf("loaded\n");
		GETCOMPONENT_FUNC f= (GETCOMPONENT_FUNC)dlsym(wacLib, "GetWasabi2Component");
		if (f)
		{
			printf("got function pointer\n");
			ifc_component *component = f();
			if (component)
			{
								if (component->component_info.wasabi_version != wasabi2_component_version
					|| component->component_info.nx_api_version != nx_api_version
					|| component->component_info.nx_platform_guid != nx_platform_guid)
				{
		
		dlclose(wacLib);
					return NErr_IncompatibleVersion;
				}

				component->component_info.dl_handle = wacLib;
				component->component_info.filename = NXURIRetain(filename);
				ret = component->Initialize(service_api);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
		
		dlclose(wacLib);
					return ret;
				}

				/* if the component was added late, we'll need to run some extra stages */
				ret = LateLoad(component);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
		
		dlclose(wacLib);
					return ret;
				}

				components.push_back(component);
				return NErr_Success;
			}
		}
		// for some reason this crashes
		dlclose(wacLib);
		return NErr_Error;
	}
	else
	{
		printf("%s\n", dlerror());
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
		return NErr_FileNotFound;
	}
	while((entry = readdir(dir)))
	{
		if (match_extension(entry->d_name, "w6c"))
		{
			printf("Trying to load %s\n",entry->d_name);
			nx_string_t nx_filename;
			NXStringCreateWithUTF8(&nx_filename, entry->d_name);
			nx_uri_t uri_filename;
			NXURICreateWithNXString(&uri_filename, nx_filename);
			NXStringRelease(nx_filename);
			nx_uri_t w6c_filepath;
			NXURICreateWithPath(&w6c_filepath, uri_filename, directory);
			NXURIRelease(uri_filename);
			int ret = AddComponent(w6c_filepath);
			
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