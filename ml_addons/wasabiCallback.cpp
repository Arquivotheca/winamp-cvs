#include "main.h"
#include "./wasabiCallback.h"
#include "./navigation.h"
#include "./service.h"
#include <strsafe.h>

static uint8_t quickhex(wchar_t c)
{
	int hexvalue = c;
	if (hexvalue & 0x10)
		hexvalue &= ~0x30;
	else
	{
		hexvalue &= 0xF;
		hexvalue += 9;
	}
	return hexvalue;
}

static uint8_t DecodeEscape(const wchar_t *&str)
{
	uint8_t a = quickhex(*++str);
	uint8_t b = quickhex(*++str);
	str++;
	return a * 16 + b;
}

static void DecodeEscapedUTF8(wchar_t *&output, const wchar_t *&input)
{
	uint8_t utf8_data[1024]; // hopefully big enough!!
	int num_utf8_words=0;
	bool error=false;

	while (*input == '%' && num_utf8_words < sizeof(utf8_data))
	{
		if (iswxdigit(input[1]) && iswxdigit(input[2]))
		{
			utf8_data[num_utf8_words++]=DecodeEscape(input);
		}
		else if (input[1] == '%')
		{
			input+=2;
			utf8_data[num_utf8_words++]='%';
		}
		else
		{
			error = true;
			break;
		}
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, 0, 0);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, output, len);
	output += len;

	if (error)
	{
		*output++ = *input++;
	}
}

static void UrlDecode(const wchar_t *input, wchar_t *output, size_t len)
{
	const wchar_t *stop = output+len-4; // give ourself a cushion large enough to hold a full UTF-16 sequence
	const wchar_t *itr = input;
	while (*itr)
	{
		if (output >= stop)
		{
			*output=0;
			return;
		}

		switch (*itr)
		{
			case '%':
				DecodeEscapedUTF8(output, itr);
				break;
			case '&':
				*output = 0;
				return;
			default:
				*output++ = *itr++;
				break;
		}
	}
	*output = 0;
}

static bool ParamCompare(const wchar_t *url_param, const wchar_t *param_name)
{
	while (*url_param && *param_name && *url_param!=L'=')
	{
		if (*url_param++ != *param_name++)
			return false;
	}
	return true;
}

static bool get_request_parm(const wchar_t *params, const wchar_t *param_name, wchar_t *value, size_t value_len)
{
	size_t param_name_len = wcslen(param_name);
  const wchar_t *t=params;
  while (*t && *t != L'?')  // find start of parameters
		t++;
  
  while (*t)
  {
		t++; // skip ? or &
		if (ParamCompare(t, param_name))
		{
			while (*t && *t != L'=' && *t != '&')  // find start of value
				t++;
			switch(*t)
			{
			case L'=':
				UrlDecode(++t, value, value_len);
					return true;
					case 0:
			case L'&': // no value
				*value=0;
				return true;
			default: // shouldn't get here
				return false;
			}
		}
  while (*t && *t != L'&')  // find next parameter
		t++;
  }
	
  return false;
}

WasabiCallback::WasabiCallback()	
	: ref(1)
{
}

WasabiCallback::~WasabiCallback()	
{
}

HRESULT WasabiCallback::CreateInstance(WasabiCallback **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = new WasabiCallback();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t WasabiCallback::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t WasabiCallback::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int WasabiCallback::QueryInterface(GUID interface_guid, void **object)
{
	return 0;
}

FOURCC WasabiCallback::GetEventType()
{ 
	return SysCallback::BROWSER; 
}

int WasabiCallback::Notify(int msg, intptr_t param1, intptr_t param2) 
{
	switch (msg) 
	{
		case BrowserCallback::ONOPENURL:
			return OpenURL(reinterpret_cast<const wchar_t*>(param1), reinterpret_cast<bool *>(param2));
	}
	return 0;
}


int WasabiCallback::OpenURL(const wchar_t *url, bool *override)
{
	WCHAR szTemplate[] = L"http://www.winamp.com";
	INT cchTemplate = ARRAYSIZE(szTemplate) - 1;

	wchar_t addons[512]=L"";
	get_request_parm(url, L"loadaddons", addons, 512);

	if (NULL != url && 
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, url, cchTemplate, szTemplate, cchTemplate)
	    && addons[0])
	{
		wchar_t addonsUrl[1024];
		StringCchPrintfW(addonsUrl,sizeof(addonsUrl)/sizeof(addonsUrl[0]), L"http://client.winamp.com/addons/%s", addons);

		if (SUCCEEDED(Navigation_ShowService(SERVICE_ID, addonsUrl, 
						NAVFLAG_FORCEACTIVE | NAVFLAG_ENSUREMLVISIBLE | NAVFLAG_ENSUREITEMVISIBLE)))
		{
			*override = true;
			return 1;
		}
	}
	return 0;
}



#define CBCLASS WasabiCallback
START_DISPATCH;
  CB(ADDREF, AddRef);
  CB(RELEASE, Release);
  CB(QUERYINTERFACE, QueryInterface);
  CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
  CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS
