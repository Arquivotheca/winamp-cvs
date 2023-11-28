#include "main.h"
#include "./configHelper.h"

#include <strsafe.h>

HRESULT
Config_QueryGroup(ifc_viewconfig *self, const char *name, ifc_viewconfig **group)
{
	if (NULL == self)
		return E_POINTER;

	return self->QueryGroup(name, group);
}
	
HRESULT 
Config_DeleteGroup(ifc_viewconfig *self)
{
	if (NULL == self)
		return E_POINTER;

	return self->DeleteGroup();
}

HRESULT 
Config_DeleteKey(ifc_viewconfig *self, const char *key)
{
	if (NULL == self)
		return E_POINTER;

	return self->DeleteKey(key);
}

int 
Config_ReadInt(ifc_viewconfig *self, const char *key, int defaultValue)
{
	if (NULL == self)
		return defaultValue;

	return self->ReadInt(key, defaultValue);
}

size_t 
Config_ReadString(ifc_viewconfig *self, const char *key, const char *defaultValue, char *buffer, size_t bufferSize)
{
	if (NULL == self)
	{
		size_t remaining;

		if (NULL == buffer)
			return -1;
		
		if (FAILED(StringCchCopyExA(buffer, bufferSize, defaultValue, NULL, &remaining, 
					STRSAFE_IGNORE_NULLS)))
		{
			return bufferSize - 1;
		}

		return bufferSize - remaining;
	}

	return self->ReadString(key, defaultValue, buffer, bufferSize);
}

BOOL 
Config_ReadBool(ifc_viewconfig *self, const char *key, BOOL defaultValue)
{
	if (NULL == self)
		return defaultValue;

	return self->ReadBool(key, defaultValue);
}

HRESULT 
Config_WriteInt(ifc_viewconfig *self, const char *key, int value)
{
	if (NULL == self)
		return E_POINTER;

	return self->WriteInt(key, value);
}

HRESULT 
Config_WriteString(ifc_viewconfig *self, const char *key, const char *value)
{
	if (NULL == self)
		return E_POINTER;

	return self->WriteString(key, value);
}

HRESULT 
Config_WriteBool(ifc_viewconfig *self, const char *key, BOOL value)
{
	if (NULL == self)
		return E_POINTER;

	return self->WriteBool(key, value);
}