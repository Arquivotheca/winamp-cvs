//------------------------------------------------------------------------------
// File: CtlUtil.cpp
//
// Desc: DirectShow base classes.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Base classes implementing IDispatch parsing for the basic control dual
// interfaces. Derive from these and implement just the custom method and
// property methods. We also implement CPosPassThru that can be used by
// renderers and transforms to pass by IMediaPosition and IMediaSeeking


#include <streams.h>
#include <limits.h>
#include "seekpt.h"

// 'bool' non standard reserved word
#pragma warning(disable:4237)


// --- CBaseDispatch implementation ----------
CBaseDispatch::~CBaseDispatch()
{
	if (m_pti) {
		m_pti->Release();
	}
}


// return 1 if we support GetTypeInfo

STDMETHODIMP
CBaseDispatch::GetTypeInfoCount(UINT * pctinfo)
{
	CheckPointer(pctinfo,E_POINTER);
	ValidateReadWritePtr(pctinfo,sizeof(UINT *));
	*pctinfo = 1;
	return S_OK;
}


typedef HRESULT (STDAPICALLTYPE *LPLOADTYPELIB)(
	const OLECHAR FAR *szFile,
	ITypeLib FAR* FAR* pptlib);

typedef HRESULT (STDAPICALLTYPE *LPLOADREGTYPELIB)(REFGUID rguid,
																									 WORD wVerMajor,
																									 WORD wVerMinor,
																									 LCID lcid,
																									 ITypeLib FAR* FAR* pptlib);

// attempt to find our type library

STDMETHODIMP
CBaseDispatch::GetTypeInfo(
													 REFIID riid,
													 UINT itinfo,
													 LCID lcid,
													 ITypeInfo ** pptinfo)
{
	CheckPointer(pptinfo,E_POINTER);
	ValidateReadWritePtr(pptinfo,sizeof(ITypeInfo *));
	HRESULT hr;

	*pptinfo = NULL;

	// we only support one type element
	if (0 != itinfo) {
		return TYPE_E_ELEMENTNOTFOUND;
	}

	if (NULL == pptinfo) {
		return E_POINTER;
	}

	// always look for neutral
	if (NULL == m_pti) {

		LPLOADTYPELIB	    lpfnLoadTypeLib;
		LPLOADREGTYPELIB    lpfnLoadRegTypeLib;
		ITypeLib	    *ptlib;
		HINSTANCE	    hInst;

		static const char  szTypeLib[]	  = "LoadTypeLib";
		static const char  szRegTypeLib[] = "LoadRegTypeLib";
		static const WCHAR szControl[]	  = L"control.tlb";

		//
		// Try to get the Ole32Aut.dll module handle.
		//

		hInst = LoadOLEAut32();
		if (hInst == NULL) {
			DWORD dwError = GetLastError();
			return AmHresultFromWin32(dwError);
		}
		lpfnLoadRegTypeLib = (LPLOADREGTYPELIB)GetProcAddress(hInst,
			szRegTypeLib);
		if (lpfnLoadRegTypeLib == NULL) {
			DWORD dwError = GetLastError();
			return AmHresultFromWin32(dwError);
		}

		hr = (*lpfnLoadRegTypeLib)(LIBID_QuartzTypeLib, 1, 0, // version 1.0
			lcid, &ptlib);

		if (FAILED(hr)) {

			// attempt to load directly - this will fill the
			// registry in if it finds it

			lpfnLoadTypeLib = (LPLOADTYPELIB)GetProcAddress(hInst, szTypeLib);
			if (lpfnLoadTypeLib == NULL) {
				DWORD dwError = GetLastError();
				return AmHresultFromWin32(dwError);
			}

			hr = (*lpfnLoadTypeLib)(szControl, &ptlib);
			if (FAILED(hr)) {
				return hr;
			}
		}

		hr = ptlib->GetTypeInfoOfGuid(
			riid,
			&m_pti);

		ptlib->Release();

		if (FAILED(hr)) {
			return hr;
		}
	}

	*pptinfo = m_pti;
	m_pti->AddRef();
	return S_OK;
}


STDMETHODIMP
CBaseDispatch::GetIDsOfNames(
														 REFIID riid,
														 OLECHAR  ** rgszNames,
														 UINT cNames,
														 LCID lcid,
														 DISPID * rgdispid)
{
	// although the IDispatch riid is dead, we use this to pass from
	// the interface implementation class to us the iid we are talking about.

	ITypeInfo * pti;
	HRESULT hr = GetTypeInfo(riid, 0, lcid, &pti);

	if (SUCCEEDED(hr)) {
		hr = pti->GetIDsOfNames(rgszNames, cNames, rgdispid);

		pti->Release();
	}
	return hr;
}




// --- CMediaPosition implementation ----------


CMediaPosition::CMediaPosition(const TCHAR * name,LPUNKNOWN pUnk) :
CUnknown(name, pUnk)
{
}

CMediaPosition::CMediaPosition(const TCHAR * name,
															 LPUNKNOWN pUnk,
															 HRESULT * phr) :
CUnknown(name, pUnk)
{
	UNREFERENCED_PARAMETER(phr);
}


// expose our interfaces IMediaPosition and IUnknown

STDMETHODIMP
CMediaPosition::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	ValidateReadWritePtr(ppv,sizeof(PVOID));
	if (riid == IID_IMediaPosition) {
		return GetInterface( (IMediaPosition *) this, ppv);
	} else {
		return CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
}


// return 1 if we support GetTypeInfo

STDMETHODIMP
CMediaPosition::GetTypeInfoCount(UINT * pctinfo)
{
	return m_basedisp.GetTypeInfoCount(pctinfo);
}


// attempt to find our type library

STDMETHODIMP
CMediaPosition::GetTypeInfo(
														UINT itinfo,
														LCID lcid,
														ITypeInfo ** pptinfo)
{
	return m_basedisp.GetTypeInfo(
		IID_IMediaPosition,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CMediaPosition::GetIDsOfNames(
															REFIID riid,
															OLECHAR  ** rgszNames,
															UINT cNames,
															LCID lcid,
															DISPID * rgdispid)
{
	return m_basedisp.GetIDsOfNames(
		IID_IMediaPosition,
		rgszNames,
		cNames,
		lcid,
		rgdispid);
}


STDMETHODIMP
CMediaPosition::Invoke(
											 DISPID dispidMember,
											 REFIID riid,
											 LCID lcid,
											 WORD wFlags,
											 DISPPARAMS * pdispparams,
											 VARIANT * pvarResult,
											 EXCEPINFO * pexcepinfo,
											 UINT * puArgErr)
{
	// this parameter is a dead leftover from an earlier interface
	if (IID_NULL != riid) {
		return DISP_E_UNKNOWNINTERFACE;
	}

	ITypeInfo * pti;
	HRESULT hr = GetTypeInfo(0, lcid, &pti);

	if (FAILED(hr)) {
		return hr;
	}

	hr = pti->Invoke(
		(IMediaPosition *)this,
		dispidMember,
		wFlags,
		pdispparams,
		pvarResult,
		pexcepinfo,
		puArgErr);

	pti->Release();
	return hr;
}


// --- IMediaPosition and IMediaSeeking pass through class ----------


CPosPassThru::CPosPassThru(const TCHAR *pName,
													 LPUNKNOWN pUnk,
													 HRESULT *phr,
													 IPin *pPin) :
CMediaPosition(pName,pUnk),
m_pPin(pPin)
{
	if (pPin == NULL) {
		*phr = E_POINTER;
		return;
	}
}


// Expose our IMediaSeeking and IMediaPosition interfaces

STDMETHODIMP
CPosPassThru::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
	CheckPointer(ppv,E_POINTER);
	*ppv = NULL;

	if (riid == IID_IMediaSeeking) {
		return GetInterface( static_cast<IMediaSeeking *>(this), ppv);
	}
	return CMediaPosition::NonDelegatingQueryInterface(riid,ppv);
}


// Return the IMediaPosition interface from our peer

HRESULT
CPosPassThru::GetPeer(IMediaPosition ** ppMP)
{
	*ppMP = NULL;

	IPin *pConnected;
	HRESULT hr = m_pPin->ConnectedTo(&pConnected);
	if (FAILED(hr)) {
		return E_NOTIMPL;
	}
	IMediaPosition * pMP;
	hr = pConnected->QueryInterface(IID_IMediaPosition, (void **) &pMP);
	pConnected->Release();
	if (FAILED(hr)) {
		return E_NOTIMPL;
	}

	*ppMP = pMP;
	return S_OK;
}


// Return the IMediaSeeking interface from our peer

HRESULT
CPosPassThru::GetPeerSeeking(IMediaSeeking ** ppMS)
{
	*ppMS = NULL;

	IPin *pConnected;
	HRESULT hr = m_pPin->ConnectedTo(&pConnected);
	if (FAILED(hr)) {
		return E_NOTIMPL;
	}
	IMediaSeeking * pMS;
	hr = pConnected->QueryInterface(IID_IMediaSeeking, (void **) &pMS);
	pConnected->Release();
	if (FAILED(hr)) {
		return E_NOTIMPL;
	}

	*ppMS = pMS;
	return S_OK;
}


// --- IMediaSeeking methods ----------


STDMETHODIMP
CPosPassThru::GetCapabilities(DWORD * pCaps)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->GetCapabilities(pCaps);
	pMS->Release();
	return hr;
}

STDMETHODIMP
CPosPassThru::CheckCapabilities(DWORD * pCaps)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->CheckCapabilities(pCaps);
	pMS->Release();
	return hr;
}

STDMETHODIMP
CPosPassThru::IsFormatSupported(const GUID * pFormat)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->IsFormatSupported(pFormat);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::QueryPreferredFormat(GUID *pFormat)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->QueryPreferredFormat(pFormat);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::SetTimeFormat(const GUID * pFormat)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->SetTimeFormat(pFormat);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::GetTimeFormat(GUID *pFormat)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->GetTimeFormat(pFormat);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::IsUsingTimeFormat(const GUID * pFormat)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->IsUsingTimeFormat(pFormat);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
																LONGLONG    Source, const GUID * pSourceFormat )
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat );
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
													 , LONGLONG * pStop, DWORD StopFlags )
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->SetPositions(pCurrent, CurrentFlags, pStop, StopFlags );
	pMS->Release();
	return hr;
}

STDMETHODIMP
CPosPassThru::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->GetPositions(pCurrent,pStop);
	pMS->Release();
	return hr;
}

HRESULT
CPosPassThru::GetSeekingLongLong
( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * )
 , LONGLONG * pll
 )
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (SUCCEEDED(hr))
	{
		hr = (pMS->*pMethod)(pll);
		pMS->Release();
	}
	return hr;
}

// If we don't have a current position then ask upstream

STDMETHODIMP
CPosPassThru::GetCurrentPosition(LONGLONG *pCurrent)
{
	// Can we report the current position
	HRESULT hr = GetMediaTime(pCurrent,NULL);
	if (SUCCEEDED(hr)) hr = NOERROR;
	else hr = GetSeekingLongLong( &IMediaSeeking::GetCurrentPosition, pCurrent );
	return hr;
}


STDMETHODIMP
CPosPassThru::GetStopPosition(LONGLONG *pStop)
{
	return GetSeekingLongLong( &IMediaSeeking::GetStopPosition, pStop );;
}

STDMETHODIMP
CPosPassThru::GetDuration(LONGLONG *pDuration)
{
	return GetSeekingLongLong( &IMediaSeeking::GetDuration, pDuration );;
}


STDMETHODIMP
CPosPassThru::GetPreroll(LONGLONG *pllPreroll)
{
	return GetSeekingLongLong( &IMediaSeeking::GetPreroll, pllPreroll );;
}


STDMETHODIMP
CPosPassThru::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMS->GetAvailable( pEarliest, pLatest );
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::GetRate(double * pdRate)
{
	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMS->GetRate(pdRate);
	pMS->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::SetRate(double dRate)
{
	if (0.0 == dRate) {
		return E_INVALIDARG;
	}

	IMediaSeeking* pMS;
	HRESULT hr = GetPeerSeeking(&pMS);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMS->SetRate(dRate);
	pMS->Release();
	return hr;
}




// --- IMediaPosition methods ----------


STDMETHODIMP
CPosPassThru::get_Duration(REFTIME * plength)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pMP->get_Duration(plength);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::get_CurrentPosition(REFTIME * pllTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->get_CurrentPosition(pllTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::put_CurrentPosition(REFTIME llTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->put_CurrentPosition(llTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::get_StopTime(REFTIME * pllTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->get_StopTime(pllTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::put_StopTime(REFTIME llTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->put_StopTime(llTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::get_PrerollTime(REFTIME * pllTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->get_PrerollTime(pllTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::put_PrerollTime(REFTIME llTime)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->put_PrerollTime(llTime);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::get_Rate(double * pdRate)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->get_Rate(pdRate);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::put_Rate(double dRate)
{
	if (0.0 == dRate) {
		return E_INVALIDARG;
	}

	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->put_Rate(dRate);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::CanSeekForward(LONG *pCanSeekForward)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->CanSeekForward(pCanSeekForward);
	pMP->Release();
	return hr;
}


STDMETHODIMP
CPosPassThru::CanSeekBackward(LONG *pCanSeekBackward)
{
	IMediaPosition* pMP;
	HRESULT hr = GetPeer(&pMP);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pMP->CanSeekBackward(pCanSeekBackward);
	pMP->Release();
	return hr;
}


// --- Implements the CRendererPosPassThru class ----------


// Media times (eg current frame, field, sample etc) are passed through the
// filtergraph in media samples. When a renderer gets a sample with media
// times in it, it will call one of the RegisterMediaTime methods we expose
// (one takes an IMediaSample, the other takes the media times direct). We
// store the media times internally and return them in GetCurrentPosition.

CRendererPosPassThru::CRendererPosPassThru(const TCHAR *pName,
																					 LPUNKNOWN pUnk,
																					 HRESULT *phr,
																					 IPin *pPin) :
CPosPassThru(pName,pUnk,phr,pPin),
m_StartMedia(0),
m_EndMedia(0),
m_bReset(TRUE)
{
}


// Sets the media times the object should report

HRESULT
CRendererPosPassThru::RegisterMediaTime(IMediaSample *pMediaSample)
{
	ASSERT(pMediaSample);
	LONGLONG StartMedia;
	LONGLONG EndMedia;

	CAutoLock cAutoLock(&m_PositionLock);

	// Get the media times from the sample

	HRESULT hr = pMediaSample->GetTime(&StartMedia,&EndMedia);
	if (FAILED(hr))
	{
		ASSERT(hr == VFW_E_SAMPLE_TIME_NOT_SET);
		return hr;
	}

	m_StartMedia = StartMedia;
	m_EndMedia = EndMedia;
	m_bReset = FALSE;
	return NOERROR;
}


// Sets the media times the object should report

HRESULT
CRendererPosPassThru::RegisterMediaTime(LONGLONG StartTime,LONGLONG EndTime)
{
	CAutoLock cAutoLock(&m_PositionLock);
	m_StartMedia = StartTime;
	m_EndMedia = EndTime;
	m_bReset = FALSE;
	return NOERROR;
}


// Return the current media times registered in the object

HRESULT
CRendererPosPassThru::GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime)
{
	ASSERT(pStartTime);

	CAutoLock cAutoLock(&m_PositionLock);
	if (m_bReset == TRUE) {
		return E_FAIL;
	}

	// We don't have to return the end time

	HRESULT hr = ConvertTimeFormat( pStartTime, 0, m_StartMedia, &TIME_FORMAT_MEDIA_TIME );
	if (pEndTime && SUCCEEDED(hr)) {
		hr = ConvertTimeFormat( pEndTime, 0, m_EndMedia, &TIME_FORMAT_MEDIA_TIME );
	}
	return hr;
}


// Resets the media times we hold

HRESULT
CRendererPosPassThru::ResetMediaTime()
{
	CAutoLock cAutoLock(&m_PositionLock);
	m_StartMedia = 0;
	m_EndMedia = 0;
	m_bReset = TRUE;
	return NOERROR;
}

// Intended to be called by the owing filter during EOS processing so
// that the media times can be adjusted to the stop time.  This ensures
// that the GetCurrentPosition will actully get to the stop position.
HRESULT
CRendererPosPassThru::EOS()
{
	HRESULT hr;

	if ( m_bReset == TRUE ) hr = E_FAIL;
	else
	{
		LONGLONG llStop;
		if SUCCEEDED(hr=GetStopPosition(&llStop))
		{
			CAutoLock cAutoLock(&m_PositionLock);
			m_StartMedia =
				m_EndMedia	 = llStop;
		}
	}
	return hr;
}





