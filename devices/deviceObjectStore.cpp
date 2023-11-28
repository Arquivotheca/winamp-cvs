#include "main.h"
#include "./deviceObjectStore.h"

static int 
DeviceObjectStore_NameCompare(const char *name1, size_t name1_length, const char *name2)
{
	int result;
	size_t name2_length, min_length;
	
	name2_length = (NULL != name2) ? (lstrlenA(name2) * sizeof(char)) : 0;

	min_length = (name1_length < name2_length) ? name1_length : name2_length;

	result = (0 != min_length) ? memcmp(name1, name2, min_length) : 0;
	
	if (0 == result)
		result = (name1_length - name2_length);
		
	return result;
}


static int 
DeviceObjectStore_FindComparer(void *context, const void *target, const void *element)
{
	return DeviceObjectStore_NameCompare((const char*)target, ((size_t)context), 
								(*((ifc_deviceobject**)element))->GetName());
}


static ifc_deviceobject ** 
DeviceObjectStore_FindLocation(const char *name, ifc_deviceobject **buffer, size_t length)
{
	size_t name_length;

	if (FALSE != IS_STRING_EMPTY(name))
		return NULL;

	name_length = lstrlenA(name) * sizeof(char);
	
	return (ifc_deviceobject**)bsearch_s(name, buffer, length, 
										sizeof(ifc_deviceobject**),
										DeviceObjectStore_FindComparer, 
										(void*)name_length);

}

static int 
DeviceObjectStore_SortComparer(const void *element1, const void *element2)
{
	const char *name1, *name2;
	size_t name1_length;
	
	name1 = (*((ifc_deviceobject**)element1))->GetName();
	name2 = (*((ifc_deviceobject**)element2))->GetName();

	name1_length = (NULL != name1) ? (lstrlenA(name1) * sizeof(char)) : 0;

	return DeviceObjectStore_NameCompare(name1, name1_length, name2);
}

static ifc_deviceobject *
DeviceObjectStore_FindUnsortedObject(const char *name, ifc_deviceobject **objects, size_t count)
{
	size_t index;
	size_t length;

	if (0 == count)
		return NULL;
	
	length = lstrlenA(name) * sizeof(char);
	
	for(index = 0; index < count; index++)
	{
		if (0 == DeviceObjectStore_NameCompare(name, length, objects[index]->GetName()))
			return objects[index];
	}

	return NULL;
}


DeviceObjectStore::DeviceObjectStore(DeviceObjectCallback addCallback, 
									DeviceObjectCallback removeCallback, void *callbackData)
{
	this->addCallback = addCallback;
	this->removeCallback = removeCallback;
	this->callbackData = callbackData;

	InitializeCriticalSection(&lock);
}

DeviceObjectStore::~DeviceObjectStore()
{
	RemoveAll();
	DeleteCriticalSection(&lock);
}

void DeviceObjectStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceObjectStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

CRITICAL_SECTION *DeviceObjectStore::GetLock()
{
	return &lock;
}

HRESULT DeviceObjectStore::Add(ifc_deviceobject *object)
{
	const char *name;
	
	if (NULL == object)
		return E_POINTER;

	name = object->GetName();

	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	return (1 == AddRange(&object, 1)) ? S_OK : S_FALSE;
}

size_t DeviceObjectStore::AddRange(ifc_deviceobject **objects, size_t count)
{
	const char *name;
	size_t index, registered, added;
	ifc_deviceobject *object, **buffer;

	
	if (NULL == objects || 0 == count)
		return 0;
		
	Lock();

	added = 0;
	registered = list.size();
	buffer = list.begin();

	for(index = 0; index < count; index++)
	{
		object = objects[index];
		if (NULL != object)
		{
			name = object->GetName();
			
			if (NULL != name && 
				'\0' != *name && 
				NULL == DeviceObjectStore_FindLocation(name, buffer, registered) &&
				NULL == DeviceObjectStore_FindUnsortedObject(name, buffer + registered, added))
			{
				list.push_back(object);
				object->AddRef();

				if (NULL != addCallback)
					this->addCallback(this, object, callbackData);

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		qsort(list.begin(), list.size(), 
				sizeof(ifc_deviceobject**), 
				DeviceObjectStore_SortComparer);
	}

	Unlock();

	return added;
}

size_t DeviceObjectStore::AddIndirect(const char **names, size_t count, DeviceObjectCreator callback, void *user)
{
	const char *name;
	size_t index, registered, added;
	ifc_deviceobject *object, **buffer;

	
	if (NULL == names || 0 == count || NULL == callback)
		return 0;
		
	Lock();

	added = 0;
	registered = list.size();
	buffer = list.begin();

	for(index = 0; index < count; index++)
	{
		name = names[index];
		
		if (NULL != name && 
			'\0' != *name && 
			NULL == DeviceObjectStore_FindLocation(name, buffer, registered) &&
			NULL == DeviceObjectStore_FindUnsortedObject(name, buffer + registered, added))
		{
			object = callback(name, user);
			if (NULL != object)
			{
				list.push_back(object);
				
				if (NULL != addCallback)
					this->addCallback(this, object, callbackData);

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		qsort(list.begin(), list.size(), 
				sizeof(ifc_deviceobject**), 
				DeviceObjectStore_SortComparer);
	}

	Unlock();

	return added;
}

HRESULT DeviceObjectStore::Remove(const char *name)
{
	HRESULT hr;
	size_t index;
	ifc_deviceobject **object_ptr, **buffer, *object;

	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	
	Lock();

	buffer = list.begin();
	object_ptr = DeviceObjectStore_FindLocation(name, buffer, list.size());
	if (NULL != object_ptr)
	{
		hr = S_OK;

		object = *object_ptr;

		index = (size_t)(object_ptr - buffer);
		list.eraseindex(index);

		if (NULL != removeCallback)
			removeCallback(this, object, callbackData);

		object->Release();
	}
	else
	{
		hr = S_FALSE;
	}
	
	Unlock();

	return hr;
}

void DeviceObjectStore::RemoveAll()
{
	size_t index;
	ifc_deviceobject *object;
	
	Lock();
	
	index = list.size();
	while(index--)
	{
		object = list[index];
		if (NULL != removeCallback)
			removeCallback(this, object, callbackData);

		object->Release();
	}
	
	list.clear();
	
	Unlock();
}

HRESULT DeviceObjectStore::Find(const char *name, ifc_deviceobject **object)
{
	HRESULT hr;
	ifc_deviceobject **object_ptr;

	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	if (NULL == object)
		return E_POINTER;

	Lock();

	object_ptr = DeviceObjectStore_FindLocation(name, list.begin(), list.size());
	
	if (NULL != object_ptr)
	{
		if (NULL != object)
		{
			*object = *object_ptr;
			(*object)->AddRef();
		}
		hr = S_OK;
	}
	else
	{
		if (NULL != object)
			*object = NULL;
		hr = S_FALSE;
	}
	
	Unlock();

	return hr;
}

HRESULT DeviceObjectStore::Enumerate(DeviceObjectEnum **enumerator)
{
	HRESULT hr;

	if (NULL == enumerator)
		return E_POINTER;

	Lock();

	hr = DeviceObjectEnum::CreateInstance(list.begin(), list.size(), enumerator);
		
	Unlock();

	return hr;
}
