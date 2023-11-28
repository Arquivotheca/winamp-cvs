#include "ComponentManager.h"
#include "foundation/error.h"
#include "nx/nxuri.h"
#include <dlfcn.h>
#include <sys/param.h>

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>


int ComponentManager::AddComponent(nx_uri_t component_uri)
{
	if (phase > PHASE_LOADED)
		return NErr_Error;
	
	if (NULL == component_uri)
		return NErr_BadParameter;
	
	int err;
	nsfilename_char_t path_buffer[MAXPATHLEN];
	nsfilename_char_t *component_path;
	err = NXURIGetFilename(component_uri, path_buffer, sizeof(path_buffer)/sizeof(nsfilename_char_t), &component_path);
	if (NErr_Success != err)
		return NErr_Error;
	
	void *component_lib = dlopen(component_path, RTLD_LAZY|RTLD_LOCAL);
	if (NULL == component_lib)
		return NErr_FileNotFound;
	
	for (ComponentList::iterator itr = components.begin(); itr != components.end(); itr++)
	{
		ifc_component *component = *itr;
		if (component->component_info.dl_handle == component_lib)
		{
			dlclose(component_lib);
			return NErr_Error;
		}
	}
	
	//printf("ComponentManager: Adding componnent '%s", component_path);
	
	GETCOMPONENT_FUNC component_func = (GETCOMPONENT_FUNC)dlsym(component_lib, "GetWasabi2Component");
	if (NULL != component_func)
	{
		ifc_component *component;
		component = component_func();
		if (NULL != component)
		{
			component->component_info.dl_handle = component_lib;
			component->component_info.filename = NXURIRetain(component_uri);
			
			err = ComponentManagerBase::AddComponent(component);
			
			if (NErr_Success != err)
			{
				NXURIRelease(component->component_info.filename);
				if (NULL == component->component_info.dl_handle)
					component_lib = NULL;
			}
		}
		else 
		{
			err = NErr_Error;
		}
	}
	else 
	{
		err = NErr_Error;
	}
	
	if (NErr_Success != err)
	{
		if (NULL != component_lib)
			dlclose(component_lib);
	}
	
	return err;
}

int ComponentManager::AddDirectory(nx_uri_t directory)
{	
	if (NULL == directory)
		return NErr_BadParameter;
	
	FSRef directory_ref;
	if (false == CFURLGetFSRef(directory, &directory_ref))
		return NErr_FileNotFound;
	
	const CFStringRef component_extension = CFSTR("w6c");
	const CFIndex component_extension_len = CFStringGetLength(component_extension);
	
	OSErr os_err;
	FSIterator iterator;

	os_err = FSOpenIterator(&directory_ref, kFSIterateFlat, &iterator);
	if (noErr != os_err)
		return NErr_Error;
	
	FSCatalogInfoBitmap catalog_info_bitmap = kFSCatInfoNodeFlags;
	
	const size_t maximum_count = ((4096 * 4) / (sizeof(FSCatalogInfo) + sizeof(FSRef)));
	
	FSCatalogInfo *catalog_info = (FSCatalogInfo *)malloc(maximum_count * sizeof(FSCatalogInfo));
	FSRef *component_ref = (FSRef *)malloc(maximum_count * sizeof(FSRef));
	if (NULL == component_ref 
		|| NULL == catalog_info)
	{
		if (NULL != catalog_info)
			free(catalog_info);
		
		if (NULL != component_ref)
			free(component_ref);
		
		FSCloseIterator(iterator);
		return NErr_OutOfMemory;
	}
	
	while(noErr == os_err)
	{
		ItemCount actual_count;
		
		os_err = FSGetCatalogInfoBulk(iterator, maximum_count, &actual_count, NULL, 
									  catalog_info_bitmap, catalog_info, 
									  component_ref, NULL, NULL);
		
		// Process all items received
		if(noErr == os_err 
		   || errFSNoMoreItems == os_err)
		{
			UInt32  index;
			for(index = 0; index < actual_count; index += 1)
			{
				if(0 == (catalog_info[index].nodeFlags & kFSNodeIsDirectoryMask))
				{
					CFURLRef component_uri = CFURLCreateFromFSRef(NULL, &component_ref[index]);
					if (NULL != component_uri)
					{
						CFStringRef extension = CFURLCopyPathExtension(component_uri);
						if (NULL != extension)
						{
							CFIndex extension_len = CFStringGetLength(extension);
							if (extension_len == component_extension_len 
								&& kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(extension, component_extension, 
																							CFRangeMake(0, extension_len), 
																							kCFCompareCaseInsensitive, NULL))
							{
								AddComponent(component_uri);
							}
							CFRelease(extension);
						}
						CFRelease(component_uri);
					}
				}
			}
		}
	}
	
	FSCloseIterator(iterator);
	free(component_ref);
	free(catalog_info);
	return NErr_Success;
}



void ComponentManager::CloseComponent(ifc_component *component)
{
	if (NULL != component 
		&& NULL != component->component_info.dl_handle)
	{
		dlclose(component->component_info.dl_handle);
		component->component_info.dl_handle = NULL;
	}
}