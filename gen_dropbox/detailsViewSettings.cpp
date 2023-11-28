#include "./main.h"
#include "./detailsView.h"
#include "./profile.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./formatData.h"

// {2AA72BFB-8616-4fea-AEA8-A2C2D2CDB39D}
static const GUID detailsViewSettingsGuid = 
{ 0x2aa72bfb, 0x8616, 0x4fea, { 0xae, 0xa8, 0xa2, 0xc2, 0xd2, 0xcd, 0xb3, 0x9d } };

#define CONFIGITEM_STR(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_STRING, ((LPCTSTR)(__defaultValue))}
#define CONFIGITEM_INT(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_INT, (__defaultValue)}



#define CFG_COLUMNS	MAKEINTRESOURCEA(1)


static ConfigIniSection::CONFIGITEM  viewSettings[] = 
{
	CONFIGITEM_STR(CFG_COLUMNS, TEXT("columns"), TEXT("")), 
};

void STDMETHODCALLTYPE DetailsView_RegisterConfig(ConfigurationManager *pcm)
{
	HRESULT hr;
	ConfigIniSection *pConfig;
	
	hr = ConfigIniSection::CreateConfig(detailsViewSettingsGuid, FILEPATH_PROFILEINI, TEXT("DetailsView"), viewSettings, ARRAYSIZE(viewSettings), &pConfig);
	if (SUCCEEDED(hr)){ pcm->Add(pConfig); pConfig->Release(); }
	
}

STDMETHODIMP DetailsView::Load(Profile *profile)
{
	if (NULL == profile)
		return E_POINTER;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(detailsViewSettingsGuid, &pConfig);
	if (FAILED(hr))
		return hr;
	

	columns[0].id = -1;
	
	TCHAR szBuffer[4096];
	if (S_OK == pConfig->ReadString(CFG_COLUMNS, szBuffer, ARRAYSIZE(szBuffer)))
	{
		INT count = ParseActiveColumns(columns, ARRAYSIZE(columns), szBuffer);
		columns[count].id = -1;
	}
		
	pConfig->Release();

	return S_OK;
}

STDMETHODIMP DetailsView::Save(Profile *profile)
{
	if (NULL == profile)
		return E_POINTER;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(detailsViewSettingsGuid, &pConfig);
	if (FAILED(hr))
		return hr;
	
	TCHAR szBuffer[4096];
	
	UpdateColumnsData();

	INT count = 0;
	ACTIVECOLUMN columnsCopy[ARRAYSIZE(columns)];
	for(; -1 != columns[count].id; count++)
	{
		INT i = columns[count].order;
		columnsCopy[i].order = i;
		columnsCopy[i].id = columns[count].id;
		columnsCopy[i].width = columns[count].width;
	}

	pConfig->WriteString(CFG_COLUMNS, FormatActiveColumns(szBuffer, ARRAYSIZE(szBuffer), columnsCopy, count));
	pConfig->Release();

	return S_OK;
	
}
