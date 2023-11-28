#include "main.h"
#include "./toolbar.h"

Toolbar::Toolbar(const char *_name, ifc_viewconfig *_config, ifc_viewcontroller *_controller)
	: ref(1), name(NULL), config(_config), controller(_controller), hwnd(NULL),
	  backColor(RGB(255, 0, 255))
{
	name = AnsiString_Duplicate(_name);

	if (NULL != config)
		config->AddRef();

	if (NULL != controller)
		controller->AddRef();

	

}

Toolbar::~Toolbar()
{
	AnsiString_Free(name);
	
	SafeRelease(config);
	SafeRelease(controller);
}

HRESULT Toolbar::CreateInstance(const char *name, ifc_viewconfig *config, ifc_viewcontroller *controller,
							 Toolbar **instance)
{	
	if (NULL == instance)
		return E_POINTER;

	if (IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	if (NULL == controller)
		return E_INVALIDARG;

	if (NULL != config)
	{	
		HRESULT hr;
		hr = config->QueryGroup(name, &config);
		if (FAILED(hr))
			return hr;
	}

	*instance = new (std::nothrow) Toolbar(name, config, controller);

	SafeRelease(config);

	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
	
}

size_t Toolbar::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Toolbar::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Toolbar::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewToolbar))
		*object = static_cast<ifc_viewtoolbar*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

const char *Toolbar::GetName()
{
	return name;
}

HRESULT Toolbar::GetConfig(ifc_viewconfig **_config)
{
	if (NULL == _config)
		return E_POINTER;

	(*_config) = config;
	
	if (NULL != config)
		config->AddRef();

	return S_OK;

}

HRESULT Toolbar::SetHost(HWND _hwnd)
{
	hwnd = _hwnd;
	return S_OK;
}

HWND Toolbar::GetHost()
{
	return hwnd;
}

void Toolbar::Paint(HDC hdc, const RECT *paintRect, BOOL erase)
{
	if (FALSE != erase)
	{
		COLORREF prevBkColor;
		
		prevBkColor = SetBkColor(hdc, backColor);
		
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, paintRect, NULL, 0, NULL);
		
		SetBkColor(hdc, prevBkColor);
	}
}

void Toolbar::Layout(BOOL redraw)
{

}

long Toolbar::GetIdealHeight()
{
	return 40;
}

void Toolbar::UpdateColors()
{
	Theme *theme;

	if (SUCCEEDED(Plugin_GetTheme(&theme)))
	{
		backColor = theme->GetColor(WADLG_ITEMBG);
		theme->Release();
	}
}

#define CBCLASS Toolbar
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETCONFIG, GetConfig)
END_DISPATCH;
#undef CBCLASS
