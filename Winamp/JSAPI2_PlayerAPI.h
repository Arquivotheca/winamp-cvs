#pragma once

#include <ocidl.h>
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class PlayerAPI : public IDispatch
	{
	public:
		PlayerAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
	private:
		const wchar_t *key;
		volatile LONG refCount;
		JSAPI::ifc_info *info;

		STDMETHOD (Play)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (Enqueue)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (Insert)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (ClearQueue)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetURL)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetTitle)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetMetadata)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

		STDMETHOD (length)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (cursor)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		
	};
}