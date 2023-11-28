#pragma once

#include <ocidl.h>
#include "../Winamp/JSAPI_Info.h"
#include "../nu/PtrList.h"

namespace JSAPI2
{
	class MUDAPI : public IDispatch
	{
	public:
		MUDAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		~MUDAPI();
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
		typedef nu::PtrList<IDispatch> EventsList;
		EventsList events;

		STDMETHOD (enabled)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (OpenOptions)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (Login)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (Logout)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (login_status)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (version)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (RegisterForEvents)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (UnregisterFromEvents)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

	};
}
