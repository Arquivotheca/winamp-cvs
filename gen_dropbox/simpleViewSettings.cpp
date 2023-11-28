#include "./main.h"
#include "./simpleView.h"
#include "./profile.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./formatData.h"

// {4FCE2D61-896D-48ce-9BD0-1B09FEED8D54}
EXTERN_C const GUID simpleViewSettingsGuid = 
{ 0x4fce2d61, 0x896d, 0x48ce, { 0x9b, 0xd0, 0x1b, 0x9, 0xfe, 0xed, 0x8d, 0x54 } };


#define CONFIGITEM_STR(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_STRING, ((LPCTSTR)(__defaultValue))}
#define CONFIGITEM_INT(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_INT, ((LPCTSTR)(INT_PTR)(__defaultValue))}



static ConfigIniSection::CONFIGITEM  viewSettings[] = 
{
	CONFIGITEM_INT(CFG_SHOWINDEX, TEXT("showOrderIndex"), TRUE), 
	CONFIGITEM_INT(CFG_SHOWTYPEICON, TEXT("showTypeIcon"), TRUE), 
	CONFIGITEM_STR(CFG_RCOLUMNSOURCE, TEXT("rightColumnSource"), GET_REGISTERED_COLUMN_NAME(COLUMN_TRACKLENGTH)), 
};


void STDMETHODCALLTYPE SimpleView_RegisterConfig(ConfigurationManager *pcm)
{
	HRESULT hr;
	ConfigIniSection *pConfig;
	
	hr = ConfigIniSection::CreateConfig(simpleViewSettingsGuid, FILEPATH_PROFILEINI, TEXT("SimpleView"), viewSettings, ARRAYSIZE(viewSettings), &pConfig);
	if (SUCCEEDED(hr)){ pcm->Add(pConfig); pConfig->Release(); }
}

STDMETHODIMP SimpleView::Load(Profile *profile)
{	
	if (NULL == profile)
		return E_POINTER;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig);
	if (FAILED(hr))
		return hr;

	DWORD painterStyle = 0, painterStyleMask = 0;
	INT iVal;

	painterStyleMask = PAINTER_DRAWINDEX | PAINTER_DRAWTYPEICON | PAINTER_RIGHTCOLUMNMASK;

	if (S_OK == pConfig->ReadInt(CFG_SHOWINDEX, &iVal))
	{
		if (iVal > 0) painterStyle |= PAINTER_DRAWINDEX;
	}

	if (S_OK == pConfig->ReadInt(CFG_SHOWTYPEICON, &iVal))
	{
		if (iVal > 0) painterStyle |= PAINTER_DRAWTYPEICON;
	}

	TCHAR szBuffer[512];
	
	if (S_OK == pConfig->ReadString(CFG_RCOLUMNSOURCE, szBuffer, ARRAYSIZE(szBuffer)))
	{
		iVal = ParseColumnName(szBuffer);
		if (iVal >= 0 && iVal < COLUMN_LAST)
		{
			painterStyle |= (PAINTER_RIGHTCOLUMNMASK & ((USHORT)iVal));
		}
		else 
			painterStyle |= PAINTER_RIGHTCOLUMNMASK;
	}
	
	painter.SetStyle(painterStyle, painterStyleMask); 
	pConfig->Release();

	return S_OK;
}

STDMETHODIMP SimpleView::Save(Profile *profile)
{
	if (NULL == profile)
		return E_POINTER;

	IConfiguration *pConfig;
	HRESULT hr = profile->QueryConfiguration(simpleViewSettingsGuid, &pConfig);
	if (FAILED(hr))
		return hr;

	DWORD painterStyle = painter.GetStyle(0xFFFFFFFF);

	pConfig->WriteInt(CFG_SHOWINDEX, (0 != (PAINTER_DRAWINDEX & painterStyle))); 
	pConfig->WriteInt(CFG_SHOWTYPEICON, (0 != (PAINTER_DRAWTYPEICON & painterStyle))); 
	INT rColumn = ((INT)(SHORT)(PAINTER_RIGHTCOLUMNMASK & painterStyle));
	
	TCHAR szBuffer[512];
	FormatColumnName(rColumn, szBuffer, ARRAYSIZE(szBuffer));
	pConfig->WriteString(CFG_RCOLUMNSOURCE, szBuffer);

	pConfig->Release();

	return S_OK;
}






