//------------------------------------------------------------------------------
// File: CtlUtil.h
//
// Desc: DirectShow base classes.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Base classes implementing IDispatch parsing for the basic control dual
// interfaces. Derive from these and implement just the custom method and
// property methods. We also implement CPosPassThru that can be used by
// renderers and transforms to pass by IMediaPosition and IMediaSeeking

#ifndef __CTLUTIL__
#define __CTLUTIL__

// OLE Automation has different ideas of TRUE and FALSE

#define OATRUE (-1)
#define OAFALSE (0)


// It's possible that we could replace this class with CreateStdDispatch

class CBaseDispatch
{
    ITypeInfo * m_pti;

public:

    CBaseDispatch() : m_pti(NULL) {}
    ~CBaseDispatch();

    /* IDispatch methods */
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      REFIID riid,
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);
};



class AM_NOVTABLE CMediaPosition :
    public IMediaPosition,
    public CUnknown
{
    CBaseDispatch m_basedisp;


public:

    CMediaPosition(const TCHAR *, LPUNKNOWN);
    CMediaPosition(const TCHAR *, LPUNKNOWN, HRESULT *phr);

    DECLARE_IUNKNOWN

    // override this to publicise our interfaces
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    /* IDispatch methods */
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

};

// A utility class that handles IMediaPosition and IMediaSeeking on behalf
// of single-input pin renderers, or transform filters.
//
// Renderers will expose this from the filter; transform filters will
// expose it from the output pin and not the renderer.
//
// Create one of these, giving it your IPin* for your input pin, and delegate
// all IMediaPosition methods to it. It will query the input pin for
// IMediaPosition and respond appropriately.
//
// Call ForceRefresh if the pin connection changes.
//
// This class no longer caches the upstream IMediaPosition or IMediaSeeking
// it acquires it on each method call. This means ForceRefresh is not needed.
// The method is kept for source compatibility and to minimise the changes
// if we need to put it back later for performance reasons.

class CPosPassThru : public IMediaSeeking, public CMediaPosition
{
    IPin *m_pPin;

    HRESULT GetPeer(IMediaPosition **ppMP);
    HRESULT GetPeerSeeking(IMediaSeeking **ppMS);

public:

    CPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *);
    DECLARE_IUNKNOWN

    HRESULT ForceRefresh() {
        return S_OK;
    };

    // override to return an accurate current position
    virtual HRESULT GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime) {
        return E_FAIL;
    }

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    // IMediaSeeking methods
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsFormatSupported( const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                   LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( LONGLONG * pStop );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetDuration( LONGLONG *pDuration);
    STDMETHODIMP GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest );
    STDMETHODIMP GetPreroll( LONGLONG *pllPreroll );

    // IMediaPosition properties
    STDMETHODIMP get_Duration(REFTIME * plength);
    STDMETHODIMP put_CurrentPosition(REFTIME llTime);
    STDMETHODIMP get_StopTime(REFTIME * pllTime);
    STDMETHODIMP put_StopTime(REFTIME llTime);
    STDMETHODIMP get_PrerollTime(REFTIME * pllTime);
    STDMETHODIMP put_PrerollTime(REFTIME llTime);
    STDMETHODIMP get_Rate(double * pdRate);
    STDMETHODIMP put_Rate(double dRate);
    STDMETHODIMP get_CurrentPosition(REFTIME * pllTime);
    STDMETHODIMP CanSeekForward(LONG *pCanSeekForward);
    STDMETHODIMP CanSeekBackward(LONG *pCanSeekBackward);

private:
    HRESULT GetSeekingLongLong( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * ),
                                LONGLONG * pll );
};


// Adds the ability to return a current position

class CRendererPosPassThru : public CPosPassThru
{
    CCritSec m_PositionLock;    // Locks access to our position
    LONGLONG m_StartMedia;      // Start media time last seen
    LONGLONG m_EndMedia;        // And likewise the end media
    BOOL m_bReset;              // Have media times been set

public:

    // Used to help with passing media times through graph

    CRendererPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *);
    HRESULT RegisterMediaTime(IMediaSample *pMediaSample);
    HRESULT RegisterMediaTime(LONGLONG StartTime,LONGLONG EndTime);
    HRESULT GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime);
    HRESULT ResetMediaTime();
    HRESULT EOS();
};

STDAPI CreatePosPassThru(
    LPUNKNOWN pAgg,
    BOOL bRenderer,
    IPin *pPin,
    IUnknown **ppPassThru
);


#endif // __CTLUTIL__
