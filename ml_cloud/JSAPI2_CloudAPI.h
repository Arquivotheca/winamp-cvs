#pragma once

#include <ocidl.h>
#include "../Winamp/JSAPI_Info.h"
#include "../nu/PtrList.h"
#include "main.h"

namespace JSAPI2
{
	class CloudAPI : public IDispatch
	{
	public:
		CloudAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		~CloudAPI();
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

		void OnStatusChange(int new_status);
	private:
		const wchar_t *key;
		volatile LONG refCount;
		JSAPI::ifc_info *info;

		STDMETHOD (PropertyFromConfig)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr, Config_StringGetter getter, Config_StringSetter setter);

		STDMETHOD (provider)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (authtoken)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (username)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (ShowCloudPrefs)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (ShowSigninPage)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (ShowCloudSources)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	};
}