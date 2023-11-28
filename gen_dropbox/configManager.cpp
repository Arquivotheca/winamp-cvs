#include "./configManager.h"

ConfigurationManager::ConfigurationManager()
{
}

ConfigurationManager::~ConfigurationManager()
{
	Clear();
}

BOOL ConfigurationManager::Add(IConfiguration *pConfig)
{
	if (NULL == pConfig || configList.contains(pConfig))
		return FALSE;
	pConfig->AddRef();
	configList.push_back(pConfig);
	return TRUE;
}

BOOL ConfigurationManager::Remove(IConfiguration *pConfig)
{
	for(size_t index = 0; index < configList.size(); index++)
	{
		if (configList.at(index) != pConfig)
		{
			configList.eraseindex(index);
			pConfig->Release();
			return TRUE;
		}
	}
	
	return FALSE;
}

void ConfigurationManager::Clear()
{
	
	for(size_t index = 0; index < configList.size(); index++)
	{
		configList.at(index)->Release();
	}
	configList.clear();
}

HRESULT ConfigurationManager::QueryConfiguration(REFGUID configId, Profile *profile, IConfiguration **ppConfig)
{
	HRESULT hr;
	size_t index = configList.size();
	while (index--)
	{	
		hr = configList.at(index)->CreateInstance(configId, profile, ppConfig);
		if (SUCCEEDED(hr) || E_NOINTERFACE != hr)
			return hr;
	}
	return E_NOINTERFACE;
}

