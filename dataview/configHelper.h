#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONFIG_HELPER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONFIG_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewconfig.h"

HRESULT
Config_QueryGroup(ifc_viewconfig *self, const char *name, ifc_viewconfig **group);
	
HRESULT 
Config_DeleteGroup(ifc_viewconfig *self);

HRESULT 
Config_DeleteKey(ifc_viewconfig *self, const char *key);

int 
Config_ReadInt(ifc_viewconfig *self, const char *key, int defaultValue);

size_t 
Config_ReadString(ifc_viewconfig *self, const char *key, const char *defaultValue, char *buffer, size_t bufferSize);

BOOL 
Config_ReadBool(ifc_viewconfig *self, const char *key, BOOL defaultValue);

HRESULT 
Config_WriteInt(ifc_viewconfig *self, const char *key, int value);

HRESULT 
Config_WriteString(ifc_viewconfig *self, const char *key, const char *value);

HRESULT 
Config_WriteBool(ifc_viewconfig *self, const char *key, BOOL value);

#endif //_NULLSOFT_WINAMP_DATAVIEW_CONFIG_HELPER_HEADER