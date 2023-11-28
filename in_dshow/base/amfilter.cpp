//------------------------------------------------------------------------------
// File: AMFilter.cpp
//
// Desc: DirectShow base classes - implements class hierarchy for streams
//       architecture.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//=====================================================================
//=====================================================================
// The following classes are declared in this header:
//
//
// CBaseMediaFilter            Basic IMediaFilter support (abstract class)
// CBaseFilter                 Support for IBaseFilter (incl. IMediaFilter)
// CEnumPins                   Enumerate input and output pins
// CEnumMediaTypes             Enumerate the preferred pin formats
// CBasePin                    Abstract base class for IPin interface
//    CBaseOutputPin           Adds data provider member functions
//    CBaseInputPin            Implements IMemInputPin interface
// CMediaSample                Basic transport unit for IMemInputPin
// CBaseAllocator              General list guff for most allocators
//    CMemAllocator            Implements memory buffer allocation
//
//=====================================================================
//=====================================================================

#include <streams.h>



//=====================================================================
// Helpers
//=====================================================================
STDAPI CreateMemoryAllocator(IMemAllocator **ppAllocator)
{
    return CoCreateInstance(CLSID_MemoryAllocator,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IMemAllocator,
                            (void **)ppAllocator);
}

//  Put this one here rather than in ctlutil.cpp to avoid linking
//  anything brought in by ctlutil.cpp
STDAPI CreatePosPassThru(
    LPUNKNOWN pAgg,
    BOOL bRenderer,
    IPin *pPin,
    IUnknown **ppPassThru
)
{
    *ppPassThru = NULL;
    IUnknown *pUnkSeek;
    HRESULT hr = CoCreateInstance(CLSID_SeekingPassThru,
                                  pAgg,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IUnknown,
                                  (void **)&pUnkSeek
                                 );
    if (FAILED(hr)) {
        return hr;
    }

    ISeekingPassThru *pPassThru;
    hr = pUnkSeek->QueryInterface(IID_ISeekingPassThru, (void**)&pPassThru);
    if (FAILED(hr)) {
        pUnkSeek->Release();
        return hr;
    }
    hr = pPassThru->Init(bRenderer, pPin);
    pPassThru->Release();
    if (FAILED(hr)) {
        pUnkSeek->Release();
        return hr;
    }
    *ppPassThru = pUnkSeek;
    return S_OK;
}



#define CONNECT_TRACE_LEVEL 3

//=====================================================================
//=====================================================================
// Implements CBaseFilter
//=====================================================================
//=====================================================================


/* Override this to say what interfaces we support and where */

STDMETHODIMP CBaseFilter::NonDelegatingQueryInterface(REFIID riid,
                                                      void **ppv)
{
    /* Do we have this interface */

    if (riid == IID_IBaseFilter) {
        return GetInterface((IBaseFilter *) this, ppv);
    } else if (riid == IID_IMediaFilter) {
        return GetInterface((IMediaFilter *) this, ppv);
    } else if (riid == IID_IPersist) {
        return GetInterface((IPersist *) this, ppv);
    } else if (riid == IID_IAMovieSetup) {
        return GetInterface((IAMovieSetup *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

#ifdef DEBUG
STDMETHODIMP_(ULONG) CBaseFilter::NonDelegatingRelease()
{
    if (m_cRef == 1) {
        KASSERT(m_pGraph == NULL);
    }
    return CUnknown::NonDelegatingRelease();
}
#endif


/* Constructor */

CBaseFilter::CBaseFilter(const TCHAR    *pName,
             LPUNKNOWN  pUnk,
             CCritSec   *pLock,
             REFCLSID   clsid) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{

    ASSERT(pLock != NULL);
}

/* Passes in a redundant HRESULT argument */

CBaseFilter::CBaseFilter(TCHAR     *pName,
                         LPUNKNOWN  pUnk,
                         CCritSec  *pLock,
                         REFCLSID   clsid,
                         HRESULT   *phr) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{

    ASSERT(pLock != NULL);
    UNREFERENCED_PARAMETER(phr);
}

#ifdef UNICODE
CBaseFilter::CBaseFilter(const CHAR *pName,
             LPUNKNOWN  pUnk,
             CCritSec   *pLock,
             REFCLSID   clsid) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{

    ASSERT(pLock != NULL);
}
CBaseFilter::CBaseFilter(CHAR     *pName,
                         LPUNKNOWN  pUnk,
                         CCritSec  *pLock,
                         REFCLSID   clsid,
                         HRESULT   *phr) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{

    ASSERT(pLock != NULL);
    UNREFERENCED_PARAMETER(phr);
}
#endif

/* Destructor */

CBaseFilter::~CBaseFilter()
{

    // NOTE we do NOT hold references on the filtergraph for m_pGraph or m_pSink
    // When we did we had the circular reference problem.  Nothing would go away.

    delete[] m_pName;

    // must be stopped, but can't call Stop here since
    // our critsec has been destroyed.

    /* Release any clock we were using */
    if (m_pClock) {
        m_pClock->Release();
        m_pClock = NULL;
    }
}

/* Return the filter's clsid */
STDMETHODIMP
CBaseFilter::GetClassID(CLSID *pClsID)
{
    CheckPointer(pClsID,E_POINTER);
    ValidateReadWritePtr(pClsID,sizeof(CLSID));
    *pClsID = m_clsid;
    return NOERROR;
}

/* Override this if your state changes are not done synchronously */
STDMETHODIMP
CBaseFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    return S_OK;
}


/* Set the clock we will use for synchronisation */

STDMETHODIMP
CBaseFilter::SetSyncSource(IReferenceClock *pClock)
{
    CAutoLock cObjectLock(m_pLock);

    // Ensure the new one does not go away - even if the same as the old
    if (pClock) {
        pClock->AddRef();
    }

    // if we have a clock, release it
    if (m_pClock) {
        m_pClock->Release();
    }

    // Set the new reference clock (might be NULL)
    // Should we query it to ensure it is a clock?  Consider for a debug build.
    m_pClock = pClock;

    return NOERROR;
}

/* Return the clock we are using for synchronisation */
STDMETHODIMP
CBaseFilter::GetSyncSource(IReferenceClock **pClock)
{
    CheckPointer(pClock,E_POINTER);
    ValidateReadWritePtr(pClock,sizeof(IReferenceClock *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pClock) {
        // returning an interface... addref it...
        m_pClock->AddRef();
    }
    *pClock = (IReferenceClock*)m_pClock;
    return NOERROR;
}



// override CBaseMediaFilter Stop method, to deactivate any pins this
// filter has.
STDMETHODIMP
CBaseFilter::Stop()
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = NOERROR;

    // notify all pins of the state change
    if (m_State != State_Stopped) {
        int cPins = GetPinCount();
        for (int c = 0; c < cPins; c++) {

            CBasePin *pPin = GetPin(c);

            // Disconnected pins are not activated - this saves pins worrying
            // about this state themselves. We ignore the return code to make
            // sure everyone is inactivated regardless. The base input pin
            // class can return an error if it has no allocator but Stop can
            // be used to resync the graph state after something has gone bad

            if (pPin->IsConnected()) {
                HRESULT hrTmp = pPin->Inactive();
                if (FAILED(hrTmp) && SUCCEEDED(hr)) {
                    hr = hrTmp;
                }
            }
        }
    }


    m_State = State_Stopped;
    return hr;
}


// override CBaseMediaFilter Pause method to activate any pins
// this filter has (also called from Run)

STDMETHODIMP
CBaseFilter::Pause()
{
    CAutoLock cObjectLock(m_pLock);

    // notify all pins of the change to active state
    if (m_State == State_Stopped) {
        int cPins = GetPinCount();
        for (int c = 0; c < cPins; c++) {

            CBasePin *pPin = GetPin(c);

            // Disconnected pins are not activated - this saves pins
            // worrying about this state themselves

            if (pPin->IsConnected()) {
                HRESULT hr = pPin->Active();
                if (FAILED(hr)) {
                    return hr;
                }
            }
        }
    }



    m_State = State_Paused;
    return S_OK;
}

// Put the filter into a running state.

// The time parameter is the offset to be added to the samples'
// stream time to get the reference time at which they should be presented.
//
// you can either add these two and compare it against the reference clock,
// or you can call CBaseFilter::StreamTime and compare that against
// the sample timestamp.

STDMETHODIMP
CBaseFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);

    // remember the stream time offset
    m_tStart = tStart;

    if (m_State == State_Stopped){
    HRESULT hr = Pause();

    if (FAILED(hr)) {
        return hr;
    }
    }
    // notify all pins of the change to active state
    if (m_State != State_Running) {
        int cPins = GetPinCount();
        for (int c = 0; c < cPins; c++) {

            CBasePin *pPin = GetPin(c);

            // Disconnected pins are not activated - this saves pins
            // worrying about this state themselves

            if (pPin->IsConnected()) {
                HRESULT hr = pPin->Run(tStart);
                if (FAILED(hr)) {
                    return hr;
                }
            }
        }
    }


    m_State = State_Running;
    return S_OK;
}

//
// return the current stream time - samples with start timestamps of this
// time or before should be rendered by now
HRESULT
CBaseFilter::StreamTime(CRefTime& rtStream)
{
    // Caller must lock for synchronization
    // We can't grab the filter lock because we want to be able to call
    // this from worker threads without deadlocking

    if (m_pClock == NULL) {
        return VFW_E_NO_CLOCK;
    }

    // get the current reference time
    HRESULT hr = m_pClock->GetTime((REFERENCE_TIME*)&rtStream);
    if (FAILED(hr)) {
        return hr;
    }

    // subtract the stream offset to get stream time
    rtStream -= m_tStart;

    return S_OK;
}


/* Create an enumerator for the pins attached to this filter */

STDMETHODIMP
CBaseFilter::EnumPins(IEnumPins **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumPins *));

    /* Create a new ref counted enumerator */

    *ppEnum = new CEnumPins(this,
                        NULL);

    return *ppEnum == NULL ? E_OUTOFMEMORY : NOERROR;
}


// default behaviour of FindPin is to assume pins are named
// by their pin names
STDMETHODIMP
CBaseFilter::FindPin(
    LPCWSTR Id,
    IPin ** ppPin
)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

    //  We're going to search the pin list so maintain integrity
    CAutoLock lck(m_pLock);
    int iCount = GetPinCount();
    for (int i = 0; i < iCount; i++) {
        CBasePin *pPin = GetPin(i);
        ASSERT(pPin != NULL);

        if (0 == lstrcmpW(pPin->Name(), Id)) {
            //  Found one that matches
            //
            //  AddRef() and return it
            *ppPin = pPin;
            pPin->AddRef();
            return S_OK;
        }
    }
    *ppPin = NULL;
    return VFW_E_NOT_FOUND;
}

/* Return information about this filter */

STDMETHODIMP
CBaseFilter::QueryFilterInfo(FILTER_INFO * pInfo)
{
    CheckPointer(pInfo,E_POINTER);
    ValidateReadWritePtr(pInfo,sizeof(FILTER_INFO));

    if (m_pName) {
        lstrcpynW(pInfo->achName, m_pName, sizeof(pInfo->achName)/sizeof(WCHAR));
    } else {
        pInfo->achName[0] = L'\0';
    }
    pInfo->pGraph = m_pGraph;
    if (m_pGraph)
        m_pGraph->AddRef();
    return NOERROR;
}


/* Provide the filter with a filter graph */

STDMETHODIMP
CBaseFilter::JoinFilterGraph(
    IFilterGraph * pGraph,
    LPCWSTR pName)
{
    CAutoLock cObjectLock(m_pLock);

    // NOTE: we no longer hold references on the graph (m_pGraph, m_pSink)

    m_pGraph = pGraph;
    if (m_pGraph) {
        HRESULT hr = m_pGraph->QueryInterface(IID_IMediaEventSink,
                        (void**) &m_pSink);
        if (FAILED(hr)) {
            ASSERT(m_pSink == NULL);
        }
        else m_pSink->Release();        // we do NOT keep a reference on it.
    } else {
        // if graph pointer is null, then we should
        // also release the IMediaEventSink on the same object - we don't
        // refcount it, so just set it to null
        m_pSink = NULL;
    }


    if (m_pName) {
        delete[] m_pName;
        m_pName = NULL;
    }

    if (pName) {
        DWORD nameLen = lstrlenW(pName)+1;
        m_pName = new WCHAR[nameLen];
        if (m_pName) {
            CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
        } else {
            // !!! error here?
            ASSERT(FALSE);
        }
    }


    return NOERROR;
}


// return a Vendor information string. Optional - may return E_NOTIMPL.
// memory returned should be freed using CoTaskMemFree
// default implementation returns E_NOTIMPL
STDMETHODIMP
CBaseFilter::QueryVendorInfo(
    LPWSTR* pVendorInfo)
{
    UNREFERENCED_PARAMETER(pVendorInfo);
    return E_NOTIMPL;
}


// send an event notification to the filter graph if we know about it.
// returns S_OK if delivered, S_FALSE if the filter graph does not sink
// events, or an error otherwise.
HRESULT
CBaseFilter::NotifyEvent(
    long EventCode,
    LONG_PTR EventParam1,
    LONG_PTR EventParam2)
{
    // Snapshot so we don't have to lock up
    IMediaEventSink *pSink = m_pSink;
    if (pSink) {
        if (EC_COMPLETE == EventCode) {
            EventParam2 = (LONG_PTR)(IBaseFilter*)this;
        }

        return pSink->Notify(EventCode, EventParam1, EventParam2);
    } else {
        return E_NOTIMPL;
    }
}

// Request reconnect
// pPin is the pin to reconnect
// pmt is the type to reconnect with - can be NULL
// Calls ReconnectEx on the filter graph
HRESULT
CBaseFilter::ReconnectPin(
    IPin *pPin,
    AM_MEDIA_TYPE const *pmt
)
{
    IFilterGraph2 *pGraph2;
    if (m_pGraph != NULL) {
        HRESULT hr = m_pGraph->QueryInterface(IID_IFilterGraph2, (void **)&pGraph2);
        if (SUCCEEDED(hr)) {
            hr = pGraph2->ReconnectEx(pPin, pmt);
            pGraph2->Release();
            return hr;
        } else {
            return m_pGraph->Reconnect(pPin);
        }
    } else {
        return E_NOINTERFACE;
    }
}



/* This is the same idea as the media type version does for type enumeration
   on pins but for the list of pins available. So if the list of pins you
   provide changes dynamically then either override this virtual function
   to provide the version number, or more simply call IncrementPinVersion */

LONG CBaseFilter::GetPinVersion()
{
    return m_PinVersion;
}


/* Increment the current pin version cookie */

void CBaseFilter::IncrementPinVersion()
{
    InterlockedIncrement(&m_PinVersion);
}

/* register filter */

STDMETHODIMP CBaseFilter::Register()
{
    // get setup data, if it exists
    //
    LPAMOVIESETUP_FILTER psetupdata = GetSetupData();

    // check we've got data
    //
    if( NULL == psetupdata ) return S_FALSE;

    // init is ref counted so call just in case
    // we're being called cold.
    //
    HRESULT hr = CoInitialize( (LPVOID)NULL );
    ASSERT( SUCCEEDED(hr) );

    // get hold of IFilterMapper
    //
    IFilterMapper *pIFM;
    hr = CoCreateInstance( CLSID_FilterMapper
                             , NULL
                             , CLSCTX_INPROC_SERVER
                             , IID_IFilterMapper
                             , (void **)&pIFM       );
    if( SUCCEEDED(hr) )
    {
        hr = AMovieSetupRegisterFilter( psetupdata, pIFM, TRUE );
        pIFM->Release();
    }

    // and clear up
    //
    CoFreeUnusedLibraries();
    CoUninitialize();

    return NOERROR;
}


/* unregister filter */

STDMETHODIMP CBaseFilter::Unregister()
{
    // get setup data, if it exists
    //
    LPAMOVIESETUP_FILTER psetupdata = GetSetupData();

    // check we've got data
    //
    if( NULL == psetupdata ) return S_FALSE;

    // OLE init is ref counted so call
    // just in case we're being called cold.
    //
    HRESULT hr = CoInitialize( (LPVOID)NULL );
    ASSERT( SUCCEEDED(hr) );

    // get hold of IFilterMapper
    //
    IFilterMapper *pIFM;
    hr = CoCreateInstance( CLSID_FilterMapper
                             , NULL
                             , CLSCTX_INPROC_SERVER
                             , IID_IFilterMapper
                             , (void **)&pIFM       );
    if( SUCCEEDED(hr) )
    {
        hr = AMovieSetupRegisterFilter( psetupdata, pIFM, FALSE );

        // release interface
        //
        pIFM->Release();
    }

    // clear up
    //
    CoFreeUnusedLibraries();
    CoUninitialize();

    // handle one acceptable "error" - that
    // of filter not being registered!
    // (couldn't find a suitable #define'd
    // name for the error!)
    //
    if( 0x80070002 == hr)
      return NOERROR;
    else
      return hr;
}


//=====================================================================
//=====================================================================
// Implements CEnumPins
//=====================================================================
//=====================================================================


CEnumPins::CEnumPins(CBaseFilter *pFilter,
             CEnumPins *pEnumPins) :
    m_Position(0),
    m_PinCount(0),
    m_pFilter(pFilter),
    m_cRef(1),               // Already ref counted
    m_PinCache(NAME("Pin Cache"))
{

#ifdef DEBUG
    m_dwCookie = DbgRegisterObjectCreation("CEnumPins", 0);
#endif

    /* We must be owned by a filter derived from CBaseFilter */

    ASSERT(pFilter != NULL);

    /* Hold a reference count on our filter */
    m_pFilter->AddRef();

    /* Are we creating a new enumerator */

    if (pEnumPins == NULL) {
        m_Version = m_pFilter->GetPinVersion();
        m_PinCount = m_pFilter->GetPinCount();
    } else {
        ASSERT(m_Position <= m_PinCount);
        m_Position = pEnumPins->m_Position;
        m_PinCount = pEnumPins->m_PinCount;
        m_Version = pEnumPins->m_Version;
        m_PinCache.AddTail(&(pEnumPins->m_PinCache));
    }
}


/* Destructor releases the reference count on our filter NOTE since we hold
   a reference count on the filter who created us we know it is safe to
   release it, no access can be made to it afterwards though as we have just
   caused the last reference count to go and the object to be deleted */

CEnumPins::~CEnumPins()
{
    m_pFilter->Release();

#ifdef DEBUG
    DbgRegisterObjectDestruction(m_dwCookie);
#endif
}


/* Override this to say what interfaces we support where */

STDMETHODIMP
CEnumPins::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv, E_POINTER);

    /* Do we have this interface */

    if (riid == IID_IEnumPins || riid == IID_IUnknown) {
        return GetInterface((IEnumPins *) this, ppv);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CEnumPins::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CEnumPins::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

/* One of an enumerator's basic member functions allows us to create a cloned
   interface that initially has the same state. Since we are taking a snapshot
   of an object (current position and all) we must lock access at the start */

STDMETHODIMP
CEnumPins::Clone(IEnumPins **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumPins *));
    HRESULT hr = NOERROR;

    /* Check we are still in sync with the filter */
    if (AreWeOutOfSync() == TRUE) {
        *ppEnum = NULL;
        hr =  VFW_E_ENUM_OUT_OF_SYNC;
    } else {

        *ppEnum = new CEnumPins(m_pFilter,
                                this);
        if (*ppEnum == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


/* Return the next pin after the current position */

STDMETHODIMP
CEnumPins::Next(ULONG cPins,        // place this many pins...
        IPin **ppPins,      // ...in this array
        ULONG *pcFetched)   // actual count passed returned here
{
    CheckPointer(ppPins,E_POINTER);
    ValidateReadWritePtr(ppPins,cPins * sizeof(IPin *));

    ASSERT(ppPins);

    if (pcFetched!=NULL) {
        ValidateWritePtr(pcFetched, sizeof(ULONG));
        *pcFetched = 0;           // default unless we succeed
    }
    // now check that the parameter is valid
    else if (cPins>1) {   // pcFetched == NULL
        return E_INVALIDARG;
    }
    ULONG cFetched = 0;           // increment as we get each one.

    /* Check we are still in sync with the filter */
    if (AreWeOutOfSync() == TRUE) {
    // If we are out of sync, we should refresh the enumerator.
    // This will reset the position and update the other members, but
    // will not clear cache of pins we have already returned.
    Refresh();
    }

    /* Calculate the number of available pins */

    int cRealPins = min(m_PinCount - m_Position, (int) cPins);
    if (cRealPins == 0) {
        return S_FALSE;
    }

    /* Return each pin interface NOTE GetPin returns CBasePin * not addrefed
       so we must QI for the IPin (which increments its reference count)
       If while we are retrieving a pin from the filter an error occurs we
       assume that our internal state is stale with respect to the filter
       (for example someone has deleted a pin) so we
       return VFW_E_ENUM_OUT_OF_SYNC                            */

    while (cRealPins && (m_PinCount - m_Position)) {

        /* Get the next pin object from the filter */

        CBasePin *pPin = m_pFilter->GetPin(m_Position++);
        if (pPin == NULL) {
            // If this happend, and it's not the first time through, then we've got a problem,
            // since we should really go back and release the iPins, which we have previously
            // AddRef'ed.
            ASSERT( cFetched==0 );
            return VFW_E_ENUM_OUT_OF_SYNC;
        }

        /* We only want to return this pin, if it is not in our cache */
        if (0 == m_PinCache.Find(pPin))
        {
            /* From the object get an IPin interface */

            *ppPins = pPin;
            pPin->AddRef();

            cFetched++;
            ppPins++;

            m_PinCache.AddTail(pPin);

            cRealPins--;

        }
    }

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    return (cPins==cFetched ? NOERROR : S_FALSE);
}


/* Skip over one or more entries in the enumerator */

STDMETHODIMP
CEnumPins::Skip(ULONG cPins)
{
    /* Check we are still in sync with the filter */
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    /* Work out how many pins are left to skip over */
    /* We could position at the end if we are asked to skip too many... */
    /* ..which would match the base implementation for CEnumMediaTypes::Skip */

    ULONG PinsLeft = m_PinCount - m_Position;
    if (cPins > PinsLeft) {
        return S_FALSE;
    }
    m_Position += cPins;
    return NOERROR;
}


/* Set the current position back to the start */
/* Reset has 4 simple steps:
 *
 * Set position to head of list
 * Sync enumerator with object being enumerated
 * Clear the cache of pins already returned
 * return S_OK
 */

STDMETHODIMP
CEnumPins::Reset()
{
    m_Version = m_pFilter->GetPinVersion();
    m_PinCount = m_pFilter->GetPinCount();

    m_Position = 0;

    // Clear the cache
    m_PinCache.RemoveAll();

    return S_OK;
}


/* Set the current position back to the start */
/* Refresh has 3 simple steps:
 *
 * Set position to head of list
 * Sync enumerator with object being enumerated
 * return S_OK
 */

STDMETHODIMP
CEnumPins::Refresh()
{
    m_Version = m_pFilter->GetPinVersion();
    m_PinCount = m_pFilter->GetPinCount();

    m_Position = 0;
    return S_OK;
}


//=====================================================================
//=====================================================================
// Implements CEnumMediaTypes
//=====================================================================
//=====================================================================


CEnumMediaTypes::CEnumMediaTypes(CBasePin *pPin,
                 CEnumMediaTypes *pEnumMediaTypes) :
    m_Position(0),
    m_pPin(pPin),
    m_cRef(1)
{

#ifdef DEBUG
    m_dwCookie = DbgRegisterObjectCreation("CEnumMediaTypes", 0);
#endif

    /* We must be owned by a pin derived from CBasePin */

    ASSERT(pPin != NULL);

    /* Hold a reference count on our pin */
    m_pPin->AddRef();

    /* Are we creating a new enumerator */

    if (pEnumMediaTypes == NULL) {
        m_Version = m_pPin->GetMediaTypeVersion();
        return;
    }

    m_Position = pEnumMediaTypes->m_Position;
    m_Version = pEnumMediaTypes->m_Version;
}


/* Destructor releases the reference count on our base pin. NOTE since we hold
   a reference count on the pin who created us we know it is safe to release
   it, no access can be made to it afterwards though as we might have just
   caused the last reference count to go and the object to be deleted */

CEnumMediaTypes::~CEnumMediaTypes()
{
#ifdef DEBUG
    DbgRegisterObjectDestruction(m_dwCookie);
#endif
    m_pPin->Release();
}


/* Override this to say what interfaces we support where */

STDMETHODIMP
CEnumMediaTypes::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv, E_POINTER);

    /* Do we have this interface */

    if (riid == IID_IEnumMediaTypes || riid == IID_IUnknown) {
        return GetInterface((IEnumMediaTypes *) this, ppv);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CEnumMediaTypes::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CEnumMediaTypes::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

/* One of an enumerator's basic member functions allows us to create a cloned
   interface that initially has the same state. Since we are taking a snapshot
   of an object (current position and all) we must lock access at the start */

STDMETHODIMP
CEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));
    HRESULT hr = NOERROR;

    /* Check we are still in sync with the pin */
    if (AreWeOutOfSync() == TRUE) {
        *ppEnum = NULL;
        hr = VFW_E_ENUM_OUT_OF_SYNC;
    } else {

        *ppEnum = new CEnumMediaTypes(m_pPin,
                                      this);

        if (*ppEnum == NULL) {
            hr =  E_OUTOFMEMORY;
        }
    }
    return hr;
}


/* Enumerate the next pin(s) after the current position. The client using this
   interface passes in a pointer to an array of pointers each of which will
   be filled in with a pointer to a fully initialised media type format
   Return NOERROR if it all works,
          S_FALSE if fewer than cMediaTypes were enumerated.
          VFW_E_ENUM_OUT_OF_SYNC if the enumerator has been broken by
                                 state changes in the filter
   The actual count always correctly reflects the number of types in the array.
*/

STDMETHODIMP
CEnumMediaTypes::Next(ULONG cMediaTypes,          // place this many types...
              AM_MEDIA_TYPE **ppMediaTypes,   // ...in this array
              ULONG *pcFetched)           // actual count passed
{
    CheckPointer(ppMediaTypes,E_POINTER);
    ValidateReadWritePtr(ppMediaTypes,cMediaTypes * sizeof(AM_MEDIA_TYPE *));
    /* Check we are still in sync with the pin */
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    if (pcFetched!=NULL) {
        ValidateWritePtr(pcFetched, sizeof(ULONG));
        *pcFetched = 0;           // default unless we succeed
    }
    // now check that the parameter is valid
    else if (cMediaTypes>1) {     // pcFetched == NULL
        return E_INVALIDARG;
    }
    ULONG cFetched = 0;           // increment as we get each one.

    /* Return each media type by asking the filter for them in turn - If we
       have an error code retured to us while we are retrieving a media type
       we assume that our internal state is stale with respect to the filter
       (for example the window size changing) so we return
       VFW_E_ENUM_OUT_OF_SYNC */

    while (cMediaTypes) {

        CMediaType cmt;

        HRESULT hr = m_pPin->GetMediaType(m_Position++, &cmt);
        if (S_OK != hr) {
            break;
        }

        /* We now have a CMediaType object that contains the next media type
           but when we assign it to the array position we CANNOT just assign
           the AM_MEDIA_TYPE structure because as soon as the object goes out of
           scope it will delete the memory we have just copied. The function
           we use is CreateMediaType which allocates a task memory block */

        /*  Transfer across the format block manually to save an allocate
            and free on the format block and generally go faster */

        *ppMediaTypes = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
        if (*ppMediaTypes == NULL) {
            break;
        }

        /*  Do a regular copy */
        **ppMediaTypes = (AM_MEDIA_TYPE)cmt;

        /*  Make sure the destructor doesn't free these */
        cmt.pbFormat = NULL;
        cmt.cbFormat = NULL;
        cmt.pUnk     = NULL;


        ppMediaTypes++;
        cFetched++;
        cMediaTypes--;
    }

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    return ( cMediaTypes==0 ? NOERROR : S_FALSE );
}


/* Skip over one or more entries in the enumerator */

STDMETHODIMP
CEnumMediaTypes::Skip(ULONG cMediaTypes)
{
    //  If we're skipping 0 elements we're guaranteed to skip the
    //  correct number of elements
    if (cMediaTypes == 0) {
        return S_OK;
    }

    /* Check we are still in sync with the pin */
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    m_Position += cMediaTypes;

    /*  See if we're over the end */
    CMediaType cmt;
    return S_OK == m_pPin->GetMediaType(m_Position - 1, &cmt) ? S_OK : S_FALSE;
}


/* Set the current position back to the start */
/* Reset has 3 simple steps:
 *
 * set position to head of list
 * sync enumerator with object being enumerated
 * return S_OK
 */

STDMETHODIMP
CEnumMediaTypes::Reset()

{
    m_Position = 0;

    // Bring the enumerator back into step with the current state.  This
    // may be a noop but ensures that the enumerator will be valid on the
    // next call.
    m_Version = m_pPin->GetMediaTypeVersion();
    return NOERROR;
}


//=====================================================================
//=====================================================================
// Implements CBasePin
//=====================================================================
//=====================================================================


/* NOTE The implementation of this class calls the CUnknown constructor with
   a NULL outer unknown pointer. This has the effect of making us a self
   contained class, ie any QueryInterface, AddRef or Release calls will be
   routed to the class's NonDelegatingUnknown methods. You will typically
   find that the classes that do this then override one or more of these
   virtual functions to provide more specialised behaviour. A good example
   of this is where a class wants to keep the QueryInterface internal but
   still wants its lifetime controlled by the external object */

/* Constructor */

CBasePin::CBasePin(TCHAR *pObjectName,
           CBaseFilter *pFilter,
           CCritSec *pLock,
           HRESULT *phr,
           LPCWSTR pName,
           PIN_DIRECTION dir) :
    CUnknown( pObjectName, NULL ),
    m_pFilter(pFilter),
    m_pLock(pLock),
    m_pName(NULL),
    m_Connected(NULL),
    m_dir(dir),
    m_bRunTimeError(FALSE),
    m_pQSink(NULL),
    m_TypeVersion(1),
    m_tStart(),
    m_tStop(MAX_TIME),
    m_bCanReconnectWhenActive(false),
    m_bTryMyTypesFirst(false),
    m_dRate(1.0)
{
    /*  WARNING - pFilter is often not a properly constituted object at
        this state (in particular QueryInterface may not work) - this
        is because its owner is often its containing object and we
        have been called from the containing object's constructor so
        the filter's owner has not yet had its CUnknown constructor
        called
    */

    ASSERT(pFilter != NULL);
    ASSERT(pLock != NULL);

    if (pName) {
        DWORD nameLen = lstrlenW(pName)+1;
        m_pName = new WCHAR[nameLen];
        if (m_pName) {
            CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
        }
    }

#ifdef DEBUG
    m_cRef = 0;
#endif
}

#ifdef UNICODE
CBasePin::CBasePin(CHAR *pObjectName,
           CBaseFilter *pFilter,
           CCritSec *pLock,
           HRESULT *phr,
           LPCWSTR pName,
           PIN_DIRECTION dir) :
    CUnknown( pObjectName, NULL ),
    m_pFilter(pFilter),
    m_pLock(pLock),
    m_pName(NULL),
    m_Connected(NULL),
    m_dir(dir),
    m_bRunTimeError(FALSE),
    m_pQSink(NULL),
    m_TypeVersion(1),
    m_tStart(),
    m_tStop(MAX_TIME),
    m_bCanReconnectWhenActive(false),
    m_bTryMyTypesFirst(false),
    m_dRate(1.0)
{
    /*  WARNING - pFilter is often not a properly constituted object at
        this state (in particular QueryInterface may not work) - this
        is because its owner is often its containing object and we
        have been called from the containing object's constructor so
        the filter's owner has not yet had its CUnknown constructor
        called
    */

    ASSERT(pFilter != NULL);
    ASSERT(pLock != NULL);

    if (pName) {
        DWORD nameLen = lstrlenW(pName)+1;
        m_pName = new WCHAR[nameLen];
        if (m_pName) {
            CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
        }
    }

#ifdef DEBUG
    m_cRef = 0;
#endif
}
#endif

/* Destructor since a connected pin holds a reference count on us there is
   no way that we can be deleted unless we are not currently connected */

CBasePin::~CBasePin()
{

    //  We don't call disconnect because if the filter is going away
    //  all the pins must have a reference count of zero so they must
    //  have been disconnected anyway - (but check the assumption)
    ASSERT(m_Connected == FALSE);

    delete[] m_pName;

    // check the internal reference count is consistent
    ASSERT(m_cRef == 0);
}


/* Override this to say what interfaces we support and where */

STDMETHODIMP
CBasePin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    /* Do we have this interface */

    if (riid == IID_IPin) {
        return GetInterface((IPin *) this, ppv);
    } else if (riid == IID_IQualityControl) {
        return GetInterface((IQualityControl *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


/* Override to increment the owning filter's reference count */

STDMETHODIMP_(ULONG)
CBasePin::NonDelegatingAddRef()
{
    ASSERT(InterlockedIncrement(&m_cRef) > 0);
    return m_pFilter->AddRef();
}


/* Override to decrement the owning filter's reference count */

STDMETHODIMP_(ULONG)
CBasePin::NonDelegatingRelease()
{
    ASSERT(InterlockedDecrement(&m_cRef) >= 0);
    return m_pFilter->Release();
}


/* Displays pin connection information */

#ifdef DEBUG
void
CBasePin::DisplayPinInfo(IPin *pReceivePin)
{

    if (DbgCheckModuleLevel(LOG_TRACE, CONNECT_TRACE_LEVEL)) {
        PIN_INFO ConnectPinInfo;
        PIN_INFO ReceivePinInfo;

        if (FAILED(QueryPinInfo(&ConnectPinInfo))) {
            (void)StringCchCopyW(ConnectPinInfo.achName, NUMELMS(ConnectPinInfo.achName),L"Bad Pin");
        } else {
            QueryPinInfoReleaseFilter(ConnectPinInfo);
        }

        if (FAILED(pReceivePin->QueryPinInfo(&ReceivePinInfo))) {
            (void)StringCchCopyW(ReceivePinInfo.achName, NUMELMS(ReceivePinInfo.achName),L"Bad Pin");
        } else {
            QueryPinInfoReleaseFilter(ReceivePinInfo);
        }

        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Trying to connect Pins :")));
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    <%ls>"), ConnectPinInfo.achName));
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    <%ls>"), ReceivePinInfo.achName));
    }
}
#endif


/* Displays general information on the pin media type */

#ifdef DEBUG
void CBasePin::DisplayTypeInfo(IPin *pPin, const CMediaType *pmt)
{
    UNREFERENCED_PARAMETER(pPin);
    if (DbgCheckModuleLevel(LOG_TRACE, CONNECT_TRACE_LEVEL)) {
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Trying media type:")));
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    major type:  %hs"),
               GuidNames[*pmt->Type()]));
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    sub type  :  %hs"),
               GuidNames[*pmt->Subtype()]));
    }
}
#endif

/* Asked to connect to a pin. A pin is always attached to an owning filter
   object so we always delegate our locking to that object. We first of all
   retrieve a media type enumerator for the input pin and see if we accept
   any of the formats that it would ideally like, failing that we retrieve
   our enumerator and see if it will accept any of our preferred types */

STDMETHODIMP
CBasePin::Connect(
    IPin * pReceivePin,
    const AM_MEDIA_TYPE *pmt   // optional media type
)
{
    CheckPointer(pReceivePin,E_POINTER);
    ValidateReadPtr(pReceivePin,sizeof(IPin));
    CAutoLock cObjectLock(m_pLock);
    DisplayPinInfo(pReceivePin);

    /* See if we are already connected */

    if (m_Connected) {
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Already connected")));
        return VFW_E_ALREADY_CONNECTED;
    }

    /* See if the filter is active */
    if (!IsStopped() && !m_bCanReconnectWhenActive) {
        return VFW_E_NOT_STOPPED;
    }


    // Find a mutually agreeable media type -
    // Pass in the template media type. If this is partially specified,
    // each of the enumerated media types will need to be checked against
    // it. If it is non-null and fully specified, we will just try to connect
    // with this.

    const CMediaType * ptype = (CMediaType*)pmt;
    HRESULT hr = AgreeMediaType(pReceivePin, ptype);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Failed to agree type")));

        // Since the procedure is already returning an error code, there
        // is nothing else this function can do to report the error.
        EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );


        return hr;
    }

    DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Connection succeeded")));


    return NOERROR;
}

// given a specific media type, attempt a connection (includes
// checking that the type is acceptable to this pin)
HRESULT
CBasePin::AttemptConnection(
    IPin* pReceivePin,      // connect to this pin
    const CMediaType* pmt   // using this type
)
{
    // The caller should hold the filter lock becasue this function
    // uses m_Connected.  The caller should also hold the filter lock
    // because this function calls SetMediaType(), IsStopped() and
    // CompleteConnect().
    ASSERT(CritCheckIn(m_pLock));

    // Check that the connection is valid  -- need to do this for every
    // connect attempt since BreakConnect will undo it.
    HRESULT hr = CheckConnect(pReceivePin);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("CheckConnect failed")));

        // Since the procedure is already returning an error code, there
        // is nothing else this function can do to report the error.
        EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

        return hr;
    }

    DisplayTypeInfo(pReceivePin, pmt);

    /* Check we will accept this media type */

    hr = CheckMediaType(pmt);
    if (hr == NOERROR) {

        /*  Make ourselves look connected otherwise ReceiveConnection
            may not be able to complete the connection
        */
        m_Connected = pReceivePin;
        m_Connected->AddRef();
        hr = SetMediaType(pmt);
        if (SUCCEEDED(hr)) {
            /* See if the other pin will accept this type */

            hr = pReceivePin->ReceiveConnection((IPin *)this, pmt);
            if (SUCCEEDED(hr)) {
                /* Complete the connection */

                hr = CompleteConnect(pReceivePin);
                if (SUCCEEDED(hr)) {
                    return hr;
                } else {
                    DbgLog((LOG_TRACE,
                            CONNECT_TRACE_LEVEL,
                            TEXT("Failed to complete connection")));
                    pReceivePin->Disconnect();
                }
            }
        }
    } else {
        // we cannot use this media type

        // return a specific media type error if there is one
        // or map a general failure code to something more helpful
        // (in particular S_FALSE gets changed to an error code)
        if (SUCCEEDED(hr) ||
            (hr == E_FAIL) ||
            (hr == E_INVALIDARG)) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    // BreakConnect and release any connection here in case CheckMediaType
    // failed, or if we set anything up during a call back during
    // ReceiveConnection.

    // Since the procedure is already returning an error code, there
    // is nothing else this function can do to report the error.
    EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

    /*  If failed then undo our state */
    if (m_Connected) {
        m_Connected->Release();
        m_Connected = NULL;
    }

    return hr;
}

/* Given an enumerator we cycle through all the media types it proposes and
   firstly suggest them to our derived pin class and if that succeeds try
   them with the pin in a ReceiveConnection call. This means that if our pin
   proposes a media type we still check in here that we can support it. This
   is deliberate so that in simple cases the enumerator can hold all of the
   media types even if some of them are not really currently available */

HRESULT CBasePin::TryMediaTypes(
    IPin *pReceivePin,
    const CMediaType *pmt,
    IEnumMediaTypes *pEnum)
{
    /* Reset the current enumerator position */

    HRESULT hr = pEnum->Reset();
    if (FAILED(hr)) {
        return hr;
    }

    CMediaType *pMediaType = NULL;
    ULONG ulMediaCount = 0;

    // attempt to remember a specific error code if there is one
    HRESULT hrFailure = S_OK;

    for (;;) {

        /* Retrieve the next media type NOTE each time round the loop the
           enumerator interface will allocate another AM_MEDIA_TYPE structure
           If we are successful then we copy it into our output object, if
           not then we must delete the memory allocated before returning */

        hr = pEnum->Next(1, (AM_MEDIA_TYPE**)&pMediaType,&ulMediaCount);
        if (hr != S_OK) {
            if (S_OK == hrFailure) {
                hrFailure = VFW_E_NO_ACCEPTABLE_TYPES;
            }
            return hrFailure;
        }


        ASSERT(ulMediaCount == 1);
        ASSERT(pMediaType);

        // check that this matches the partial type (if any)

        if ((pmt == NULL) ||
            pMediaType->MatchesPartial(pmt)) {

            hr = AttemptConnection(pReceivePin, pMediaType);

            // attempt to remember a specific error code
            if (FAILED(hr) &&
            SUCCEEDED(hrFailure) &&
            (hr != E_FAIL) &&
            (hr != E_INVALIDARG) &&
            (hr != VFW_E_TYPE_NOT_ACCEPTED)) {
                hrFailure = hr;
            }
        } else {
            hr = VFW_E_NO_ACCEPTABLE_TYPES;
        }

        DeleteMediaType(pMediaType);

        if (S_OK == hr) {
            return hr;
        }
    }
}


/* This is called to make the connection, including the taask of finding
   a media type for the pin connection. pmt is the proposed media type
   from the Connect call: if this is fully specified, we will try that.
   Otherwise we enumerate and try all the input pin's types first and
   if that fails we then enumerate and try all our preferred media types.
   For each media type we check it against pmt (if non-null and partially
   specified) as well as checking that both pins will accept it.
 */

HRESULT CBasePin::AgreeMediaType(
    IPin *pReceivePin,
    const CMediaType *pmt)
{
    ASSERT(pReceivePin);
    IEnumMediaTypes *pEnumMediaTypes = NULL;

    // if the media type is fully specified then use that
    if ( (pmt != NULL) && (!pmt->IsPartiallySpecified())) {

        // if this media type fails, then we must fail the connection
        // since if pmt is nonnull we are only allowed to connect
        // using a type that matches it.

        return AttemptConnection(pReceivePin, pmt);
    }


    /* Try the other pin's enumerator */

    HRESULT hrFailure = VFW_E_NO_ACCEPTABLE_TYPES;

    for (int i = 0; i < 2; i++) {
        HRESULT hr;
        if (i == (int)m_bTryMyTypesFirst) {
            hr = pReceivePin->EnumMediaTypes(&pEnumMediaTypes);
        } else {
            hr = EnumMediaTypes(&pEnumMediaTypes);
        }
        if (SUCCEEDED(hr)) {
            ASSERT(pEnumMediaTypes);
            hr = TryMediaTypes(pReceivePin,pmt,pEnumMediaTypes);
            pEnumMediaTypes->Release();
            if (SUCCEEDED(hr)) {
                return NOERROR;
            } else {
                // try to remember specific error codes if there are any
                if ((hr != E_FAIL) &&
                    (hr != E_INVALIDARG) &&
                    (hr != VFW_E_TYPE_NOT_ACCEPTED)) {
                    hrFailure = hr;
                }
            }
        }
    }

    return hrFailure;
}


/* Called when we want to complete a connection to another filter. Failing
   this will also fail the connection and disconnect the other pin as well */

HRESULT
CBasePin::CompleteConnect(IPin *pReceivePin)
{
    UNREFERENCED_PARAMETER(pReceivePin);
    return NOERROR;
}


/* This is called to set the format for a pin connection - CheckMediaType
   will have been called to check the connection format and if it didn't
   return an error code then this (virtual) function will be invoked */

HRESULT
CBasePin::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr = m_mt.Set(*pmt);
    if (FAILED(hr)) {
        return hr;
    }

    return NOERROR;
}


/* This is called during Connect() to provide a virtual method that can do
   any specific check needed for connection such as QueryInterface. This
   base class method just checks that the pin directions don't match */

HRESULT
CBasePin::CheckConnect(IPin * pPin)
{
    /* Check that pin directions DONT match */

    PIN_DIRECTION pd;
    pPin->QueryDirection(&pd);

    ASSERT((pd == PINDIR_OUTPUT) || (pd == PINDIR_INPUT));
    ASSERT((m_dir == PINDIR_OUTPUT) || (m_dir == PINDIR_INPUT));

    // we should allow for non-input and non-output connections?
    if (pd == m_dir) {
        return VFW_E_INVALID_DIRECTION;
    }
    return NOERROR;
}


/* This is called when we realise we can't make a connection to the pin and
   must undo anything we did in CheckConnect - override to release QIs done */

HRESULT
CBasePin::BreakConnect()
{
    return NOERROR;
}


/* Called normally by an output pin on an input pin to try and establish a
   connection.
*/

STDMETHODIMP
CBasePin::ReceiveConnection(
    IPin * pConnector,      // this is the pin who we will connect to
    const AM_MEDIA_TYPE *pmt    // this is the media type we will exchange
)
{
    CheckPointer(pConnector,E_POINTER);
    CheckPointer(pmt,E_POINTER);
    ValidateReadPtr(pConnector,sizeof(IPin));
    ValidateReadPtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cObjectLock(m_pLock);

    /* Are we already connected */
    if (m_Connected) {
        return VFW_E_ALREADY_CONNECTED;
    }

    /* See if the filter is active */
    if (!IsStopped() && !m_bCanReconnectWhenActive) {
        return VFW_E_NOT_STOPPED;
    }

    HRESULT hr = CheckConnect(pConnector);
    if (FAILED(hr)) {
        // Since the procedure is already returning an error code, there
        // is nothing else this function can do to report the error.
        EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );


        return hr;
    }

    /* Ask derived class if this media type is ok */

    CMediaType * pcmt = (CMediaType*) pmt;
    hr = CheckMediaType(pcmt);
    if (hr != NOERROR) {
        // no -we don't support this media type

        // Since the procedure is already returning an error code, there
        // is nothing else this function can do to report the error.
        EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

        // return a specific media type error if there is one
        // or map a general failure code to something more helpful
        // (in particular S_FALSE gets changed to an error code)
        if (SUCCEEDED(hr) ||
            (hr == E_FAIL) ||
            (hr == E_INVALIDARG)) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }


        return hr;
    }

    /* Complete the connection */

    m_Connected = pConnector;
    m_Connected->AddRef();
    hr = SetMediaType(pcmt);
    if (SUCCEEDED(hr)) {
        hr = CompleteConnect(pConnector);
        if (SUCCEEDED(hr)) {


            return NOERROR;
        }
    }

    DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Failed to set the media type or failed to complete the connection.")));
    m_Connected->Release();
    m_Connected = NULL;

    // Since the procedure is already returning an error code, there
    // is nothing else this function can do to report the error.
    EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );


    return hr;
}


/* Called when we want to terminate a pin connection */

STDMETHODIMP
CBasePin::Disconnect()
{
    CAutoLock cObjectLock(m_pLock);

    /* See if the filter is active */
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }

    return DisconnectInternal();
}

STDMETHODIMP
CBasePin::DisconnectInternal()
{
    ASSERT(CritCheckIn(m_pLock));

    if (m_Connected) {
        HRESULT hr = BreakConnect();
        if( FAILED( hr ) ) {


            // There is usually a bug in the program if BreakConnect() fails.
            DbgBreak( "WARNING: BreakConnect() failed in CBasePin::Disconnect()." );
            return hr;
        }

        m_Connected->Release();
        m_Connected = NULL;


        return S_OK;
    } else {
        // no connection - not an error


        return S_FALSE;
    }
}


/* Return an AddRef()'d pointer to the connected pin if there is one */
STDMETHODIMP
CBasePin::ConnectedTo(
    IPin **ppPin
)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));
    //
    //  It's pointless to lock here.
    //  The caller should ensure integrity.
    //

    IPin *pPin = m_Connected;
    *ppPin = pPin;
    if (pPin != NULL) {
        pPin->AddRef();
        return S_OK;
    } else {
        ASSERT(*ppPin == NULL);
        return VFW_E_NOT_CONNECTED;
    }
}

/* Return the media type of the connection */
STDMETHODIMP
CBasePin::ConnectionMediaType(
    AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(pmt,E_POINTER);
    ValidateReadWritePtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cObjectLock(m_pLock);

    /*  Copy constructor of m_mt allocates the memory */
    if (IsConnected()) {
        CopyMediaType( pmt, &m_mt );
        return S_OK;
    } else {
        ((CMediaType *)pmt)->InitMediaType();
        return VFW_E_NOT_CONNECTED;
    }
}

/* Return information about the filter we are connect to */

STDMETHODIMP
CBasePin::QueryPinInfo(
    PIN_INFO * pInfo
)
{
    CheckPointer(pInfo,E_POINTER);
    ValidateReadWritePtr(pInfo,sizeof(PIN_INFO));

    pInfo->pFilter = m_pFilter;
    if (m_pFilter) {
        m_pFilter->AddRef();
    }

    if (m_pName) {
        lstrcpynW(pInfo->achName, m_pName, sizeof(pInfo->achName)/sizeof(WCHAR));
    } else {
        pInfo->achName[0] = L'\0';
    }

    pInfo->dir = m_dir;

    return NOERROR;
}

STDMETHODIMP
CBasePin::QueryDirection(
    PIN_DIRECTION * pPinDir
)
{
    CheckPointer(pPinDir,E_POINTER);
    ValidateReadWritePtr(pPinDir,sizeof(PIN_DIRECTION));

    *pPinDir = m_dir;
    return NOERROR;
}

// Default QueryId to return the pin's name
STDMETHODIMP
CBasePin::QueryId(
    LPWSTR * Id
)
{
    //  We're not going away because someone's got a pointer to us
    //  so there's no need to lock

    return AMGetWideString(Name(), Id);
}

/* Does this pin support this media type WARNING this interface function does
   not lock the main object as it is meant to be asynchronous by nature - if
   the media types you support depend on some internal state that is updated
   dynamically then you will need to implement locking in a derived class */

STDMETHODIMP
CBasePin::QueryAccept(
    const AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(pmt,E_POINTER);
    ValidateReadPtr(pmt,sizeof(AM_MEDIA_TYPE));

    /* The CheckMediaType method is valid to return error codes if the media
       type is horrible, an example might be E_INVALIDARG. What we do here
       is map all the error codes into either S_OK or S_FALSE regardless */

    HRESULT hr = CheckMediaType((CMediaType*)pmt);
    if (FAILED(hr)) {
        return S_FALSE;
    }
    // note that the only defined success codes should be S_OK and S_FALSE...
    return hr;
}


/* This can be called to return an enumerator for the pin's list of preferred
   media types. An input pin is not obliged to have any preferred formats
   although it can do. For example, the window renderer has a preferred type
   which describes a video image that matches the current window size. All
   output pins should expose at least one preferred format otherwise it is
   possible that neither pin has any types and so no connection is possible */

STDMETHODIMP
CBasePin::EnumMediaTypes(
    IEnumMediaTypes **ppEnum
)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));

    /* Create a new ref counted enumerator */

    *ppEnum = new CEnumMediaTypes(this,
                              NULL);

    if (*ppEnum == NULL) {
        return E_OUTOFMEMORY;
    }

    return NOERROR;
}



/* This is a virtual function that returns a media type corresponding with
   place iPosition in the list. This base class simply returns an error as
   we support no media types by default but derived classes should override */

HRESULT CBasePin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    UNREFERENCED_PARAMETER(iPosition);
    UNREFERENCED_PARAMETER(pMediaType);
    return E_UNEXPECTED;
}


/* This is a virtual function that returns the current media type version.
   The base class initialises the media type enumerators with the value 1
   By default we always returns that same value. A Derived class may change
   the list of media types available and after doing so it should increment
   the version either in a method derived from this, or more simply by just
   incrementing the m_TypeVersion base pin variable. The type enumerators
   call this when they want to see if their enumerations are out of date */

LONG CBasePin::GetMediaTypeVersion()
{
    return m_TypeVersion;
}


/* Increment the cookie representing the current media type version */

void CBasePin::IncrementTypeVersion()
{
    InterlockedIncrement(&m_TypeVersion);
}


/* Called by IMediaFilter implementation when the state changes from Stopped
   to either paused or running and in derived classes could do things like
   commit memory and grab hardware resource (the default is to do nothing) */

HRESULT
CBasePin::Active(void)
{
    return NOERROR;
}

/* Called by IMediaFilter implementation when the state changes from
   to either paused to running and in derived classes could do things like
   commit memory and grab hardware resource (the default is to do nothing) */

HRESULT
CBasePin::Run(REFERENCE_TIME tStart)
{
    UNREFERENCED_PARAMETER(tStart);
    return NOERROR;
}


/* Also called by the IMediaFilter implementation when the state changes to
   Stopped at which point you should decommit allocators and free hardware
   resources you grabbed in the Active call (default is also to do nothing) */

HRESULT
CBasePin::Inactive(void)
{
    m_bRunTimeError = FALSE;
    return NOERROR;
}


// Called when no more data will arrive
STDMETHODIMP
CBasePin::EndOfStream(void)
{
    return S_OK;
}


STDMETHODIMP
CBasePin::SetSink(IQualityControl * piqc)
{
    CAutoLock cObjectLock(m_pLock);
    if (piqc) ValidateReadPtr(piqc,sizeof(IQualityControl));
    m_pQSink = piqc;
    return NOERROR;
} // SetSink


STDMETHODIMP
CBasePin::Notify(IBaseFilter * pSender, Quality q)
{
    UNREFERENCED_PARAMETER(q);
    UNREFERENCED_PARAMETER(pSender);
    DbgBreak("IQualityControl::Notify not over-ridden from CBasePin.  (IGNORE is OK)");
    return E_NOTIMPL;
} //Notify


// NewSegment notifies of the start/stop/rate applying to the data
// about to be received. Default implementation records data and
// returns S_OK.
// Override this to pass downstream.
STDMETHODIMP
CBasePin::NewSegment(
                REFERENCE_TIME tStart,
                REFERENCE_TIME tStop,
                double dRate)
{
    m_tStart = tStart;
    m_tStop = tStop;
    m_dRate = dRate;

    return S_OK;
}

//=====================================================================
//=====================================================================
// Implements CBaseInputPin
//=====================================================================
//=====================================================================


/* Constructor creates a default allocator object */

CBaseInputPin::CBaseInputPin(TCHAR *pObjectName,
                 CBaseFilter *pFilter,
                 CCritSec *pLock,
                 HRESULT *phr,
                 LPCWSTR pPinName) :
    CBasePin(pObjectName, pFilter, pLock, phr, pPinName, PINDIR_INPUT),
    m_pAllocator(NULL),
    m_bReadOnly(FALSE),
    m_bFlushing(FALSE)
{
    ZeroMemory(&m_SampleProps, sizeof(m_SampleProps));
}

#ifdef UNICODE
CBaseInputPin::CBaseInputPin(CHAR *pObjectName,
                 CBaseFilter *pFilter,
                 CCritSec *pLock,
                 HRESULT *phr,
                 LPCWSTR pPinName) :
    CBasePin(pObjectName, pFilter, pLock, phr, pPinName, PINDIR_INPUT),
    m_pAllocator(NULL),
    m_bReadOnly(FALSE),
    m_bFlushing(FALSE)
{
    ZeroMemory(&m_SampleProps, sizeof(m_SampleProps));
}
#endif

/* Destructor releases it's reference count on the default allocator */

CBaseInputPin::~CBaseInputPin()
{
    if (m_pAllocator != NULL) {
    m_pAllocator->Release();
    m_pAllocator = NULL;
    }
}


// override this to publicise our interfaces
STDMETHODIMP
CBaseInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    /* Do we know about this interface */

    if (riid == IID_IMemInputPin) {
        return GetInterface((IMemInputPin *) this, ppv);
    } else {
        return CBasePin::NonDelegatingQueryInterface(riid, ppv);
    }
}


/* Return the allocator interface that this input pin would like the output
   pin to use. NOTE subsequent calls to GetAllocator should all return an
   interface onto the SAME object so we create one object at the start

   Note:
       The allocator is Release()'d on disconnect and replaced on
       NotifyAllocator().

   Override this to provide your own allocator.
*/

STDMETHODIMP
CBaseInputPin::GetAllocator(
    IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pAllocator == NULL) {
        HRESULT hr = CreateMemoryAllocator(&m_pAllocator);
        if (FAILED(hr)) {
            return hr;
        }
    }
    ASSERT(m_pAllocator != NULL);
    *ppAllocator = m_pAllocator;
    m_pAllocator->AddRef();
    return NOERROR;
}


/* Tell the input pin which allocator the output pin is actually going to use
   Override this if you care - NOTE the locking we do both here and also in
   GetAllocator is unnecessary but derived classes that do something useful
   will undoubtedly have to lock the object so this might help remind people */

STDMETHODIMP
CBaseInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    CheckPointer(pAllocator,E_POINTER);
    ValidateReadPtr(pAllocator,sizeof(IMemAllocator));
    CAutoLock cObjectLock(m_pLock);

    IMemAllocator *pOldAllocator = m_pAllocator;
    pAllocator->AddRef();
    m_pAllocator = pAllocator;

    if (pOldAllocator != NULL) {
        pOldAllocator->Release();
    }

    // the readonly flag indicates whether samples from this allocator should
    // be regarded as readonly - if true, then inplace transforms will not be
    // allowed.
    m_bReadOnly = (BYTE)bReadOnly;
    return NOERROR;
}


HRESULT
CBaseInputPin::BreakConnect()
{
    /* We don't need our allocator any more */
    if (m_pAllocator) {
        // Always decommit the allocator because a downstream filter may or
        // may not decommit the connection's allocator.  A memory leak could
        // occur if the allocator is not decommited when a pin is disconnected.
        HRESULT hr = m_pAllocator->Decommit();
        if( FAILED( hr ) ) {
            return hr;
        }

        m_pAllocator->Release();
        m_pAllocator = NULL;
    }

    return S_OK;
}


/* Do something with this media sample - this base class checks to see if the
   format has changed with this media sample and if so checks that the filter
   will accept it, generating a run time error if not. Once we have raised a
   run time error we set a flag so that no more samples will be accepted

   It is important that any filter should override this method and implement
   synchronization so that samples are not processed when the pin is
   disconnected etc
*/

STDMETHODIMP
CBaseInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    ValidateReadPtr(pSample,sizeof(IMediaSample));
    ASSERT(pSample);

    HRESULT hr = CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }



    /* Check for IMediaSample2 */
    IMediaSample2 *pSample2;
    if (SUCCEEDED(pSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2))) {
        hr = pSample2->GetProperties(sizeof(m_SampleProps), (PBYTE)&m_SampleProps);
        pSample2->Release();
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        /*  Get the properties the hard way */
        m_SampleProps.cbData = sizeof(m_SampleProps);
        m_SampleProps.dwTypeSpecificFlags = 0;
        m_SampleProps.dwStreamId = AM_STREAM_MEDIA;
        m_SampleProps.dwSampleFlags = 0;
        if (S_OK == pSample->IsDiscontinuity()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
        }
        if (S_OK == pSample->IsPreroll()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_PREROLL;
        }
        if (S_OK == pSample->IsSyncPoint()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        }
        if (SUCCEEDED(pSample->GetTime(&m_SampleProps.tStart,
                                       &m_SampleProps.tStop))) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_TIMEVALID |
                                           AM_SAMPLE_STOPVALID;
        }
        if (S_OK == pSample->GetMediaType(&m_SampleProps.pMediaType)) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_TYPECHANGED;
        }
        pSample->GetPointer(&m_SampleProps.pbBuffer);
        m_SampleProps.lActual = pSample->GetActualDataLength();
        m_SampleProps.cbBuffer = pSample->GetSize();
    }

    /* Has the format changed in this sample */

    if (!(m_SampleProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED)) {
        return NOERROR;
    }

    /* Check the derived class accepts this format */
    /* This shouldn't fail as the source must call QueryAccept first */

    hr = CheckMediaType((CMediaType *)m_SampleProps.pMediaType);

    if (hr == NOERROR) {
        return NOERROR;
    }

    /* Raise a runtime error if we fail the media type */

    m_bRunTimeError = TRUE;
    EndOfStream();
    m_pFilter->NotifyEvent(EC_ERRORABORT,VFW_E_TYPE_NOT_ACCEPTED,0);
    return VFW_E_INVALIDMEDIATYPE;
}


/*  Receive multiple samples */
STDMETHODIMP
CBaseInputPin::ReceiveMultiple (
    IMediaSample **pSamples,
    long nSamples,
    long *nSamplesProcessed)
{
    CheckPointer(pSamples,E_POINTER);
    ValidateReadPtr(pSamples,nSamples * sizeof(IMediaSample *));

    HRESULT hr = S_OK;
    *nSamplesProcessed = 0;
    while (nSamples-- > 0) {
         hr = Receive(pSamples[*nSamplesProcessed]);

         /*  S_FALSE means don't send any more */
         if (hr != S_OK) {
             break;
         }
         (*nSamplesProcessed)++;
    }
    return hr;
}

/*  See if Receive() might block */
STDMETHODIMP
CBaseInputPin::ReceiveCanBlock()
{
    /*  Ask all the output pins if they block
        If there are no output pin assume we do block
    */
    int cPins = m_pFilter->GetPinCount();
    int cOutputPins = 0;
    for (int c = 0; c < cPins; c++) {
        CBasePin *pPin = m_pFilter->GetPin(c);
        PIN_DIRECTION pd;
        HRESULT hr = pPin->QueryDirection(&pd);
        if (FAILED(hr)) {
            return hr;
        }

        if (pd == PINDIR_OUTPUT) {

            IPin *pConnected;
            hr = pPin->ConnectedTo(&pConnected);
            if (SUCCEEDED(hr)) {
                ASSERT(pConnected != NULL);
                cOutputPins++;
                IMemInputPin *pInputPin;
                hr = pConnected->QueryInterface(
                                              IID_IMemInputPin,
                                              (void **)&pInputPin);
                pConnected->Release();
                if (SUCCEEDED(hr)) {
                    hr = pInputPin->ReceiveCanBlock();
                    pInputPin->Release();
                    if (hr != S_FALSE) {
                        return S_OK;
                    }
                } else {
                    /*  There's a transport we don't understand here */
                    return S_OK;
                }
            }
        }
    }
    return cOutputPins == 0 ? S_OK : S_FALSE;
}

// Default handling for BeginFlush - call at the beginning
// of your implementation (makes sure that all Receive calls
// fail). After calling this, you need to free any queued data
// and then call downstream.
STDMETHODIMP
CBaseInputPin::BeginFlush(void)
{
    //  BeginFlush is NOT synchronized with streaming but is part of
    //  a control action - hence we synchronize with the filter
    CAutoLock lck(m_pLock);

    // if we are already in mid-flush, this is probably a mistake
    // though not harmful - try to pick it up for now so I can think about it
    ASSERT(!m_bFlushing);

    // first thing to do is ensure that no further Receive calls succeed
    m_bFlushing = TRUE;

    // now discard any data and call downstream - must do that
    // in derived classes
    return S_OK;
}

// default handling for EndFlush - call at end of your implementation
// - before calling this, ensure that there is no queued data and no thread
// pushing any more without a further receive, then call downstream,
// then call this method to clear the m_bFlushing flag and re-enable
// receives
STDMETHODIMP
CBaseInputPin::EndFlush(void)
{
    //  Endlush is NOT synchronized with streaming but is part of
    //  a control action - hence we synchronize with the filter
    CAutoLock lck(m_pLock);

    // almost certainly a mistake if we are not in mid-flush
    ASSERT(m_bFlushing);

    // before calling, sync with pushing thread and ensure
    // no more data is going downstream, then call EndFlush on
    // downstream pins.

    // now re-enable Receives
    m_bFlushing = FALSE;

    // No more errors
    m_bRunTimeError = FALSE;

    return S_OK;
}


STDMETHODIMP
CBaseInputPin::Notify(IBaseFilter * pSender, Quality q)
{
    UNREFERENCED_PARAMETER(q);
    CheckPointer(pSender,E_POINTER);
    ValidateReadPtr(pSender,sizeof(IBaseFilter));
    DbgBreak("IQuality::Notify called on an input pin");
    return NOERROR;
} // Notify

/* Free up or unprepare allocator's memory, this is called through
   IMediaFilter which is responsible for locking the object first */

HRESULT
CBaseInputPin::Inactive(void)
{
    m_bRunTimeError = FALSE;
    if (m_pAllocator == NULL) {
        return VFW_E_NO_ALLOCATOR;
    }

    m_bFlushing = FALSE;

    return m_pAllocator->Decommit();
}

// what requirements do we have of the allocator - override if you want
// to support other people's allocators but need a specific alignment
// or prefix.
STDMETHODIMP
CBaseInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    UNREFERENCED_PARAMETER(pProps);
    return E_NOTIMPL;
}

//  Check if it's OK to process data
//
HRESULT
CBaseInputPin::CheckStreaming()
{
    //  Shouldn't be able to get any data if we're not connected!
    ASSERT(IsConnected());

    //  Don't process stuff in Stopped state
    if (IsStopped()) {
        return VFW_E_WRONG_STATE;
    }
    if (m_bFlushing) {
        return S_FALSE;
    }
    if (m_bRunTimeError) {
        return VFW_E_RUNTIME_ERROR;
    }
    return S_OK;
}

// Pass on the Quality notification q to
// a. Our QualityControl sink (if we have one) or else
// b. to our upstream filter
// and if that doesn't work, throw it away with a bad return code
HRESULT
CBaseInputPin::PassNotify(Quality& q)
{
    // We pass the message on, which means that we find the quality sink
    // for our input pin and send it there

    DbgLog((LOG_TRACE,3,TEXT("Passing Quality notification through transform")));
    if (m_pQSink!=NULL) {
        return m_pQSink->Notify(m_pFilter, q);
    } else {
        // no sink set, so pass it upstream
        HRESULT hr;
        IQualityControl * pIQC;

        hr = VFW_E_NOT_FOUND;                   // default
        if (m_Connected) {
            m_Connected->QueryInterface(IID_IQualityControl, (void**)&pIQC);

            if (pIQC!=NULL) {
                hr = pIQC->Notify(m_pFilter, q);
                pIQC->Release();
            }
        }
        return hr;
    }

} // PassNotify


// ------------------------------------------------------------------------
// filter registration through IFilterMapper. used if IFilterMapper is
// not found (Quartz 1.0 install)

STDAPI
AMovieSetupRegisterFilter( const AMOVIESETUP_FILTER * const psetupdata
                         , IFilterMapper *                  pIFM
                         , BOOL                             bRegister  )
{
  DbgLog((LOG_TRACE, 3, TEXT("= AMovieSetupRegisterFilter")));

  // check we've got data
  //
  if( NULL == psetupdata ) return S_FALSE;


  // unregister filter
  // (as pins are subkeys of filter's CLSID key
  // they do not need to be removed separately).
  //
  DbgLog((LOG_TRACE, 3, TEXT("= = unregister filter")));
  HRESULT hr = pIFM->UnregisterFilter( *(psetupdata->clsID) );


  if( bRegister )
  {
    // register filter
    //
    DbgLog((LOG_TRACE, 3, TEXT("= = register filter")));
    hr = pIFM->RegisterFilter( *(psetupdata->clsID)
                             , psetupdata->strName
                             , psetupdata->dwMerit    );
    if( SUCCEEDED(hr) )
    {
      // all its pins
      //
      DbgLog((LOG_TRACE, 3, TEXT("= = register filter pins")));
      for( UINT m1=0; m1 < psetupdata->nPins; m1++ )
      {
        hr = pIFM->RegisterPin( *(psetupdata->clsID)
                              , psetupdata->lpPin[m1].strName
                              , psetupdata->lpPin[m1].bRendered
                              , psetupdata->lpPin[m1].bOutput
                              , psetupdata->lpPin[m1].bZero
                              , psetupdata->lpPin[m1].bMany
                              , *(psetupdata->lpPin[m1].clsConnectsToFilter)
                              , psetupdata->lpPin[m1].strConnectsToPin );

        if( SUCCEEDED(hr) )
        {
          // and each pin's media types
          //
          DbgLog((LOG_TRACE, 3, TEXT("= = register filter pin types")));
          for( UINT m2=0; m2 < psetupdata->lpPin[m1].nMediaTypes; m2++ )
          {
            hr = pIFM->RegisterPinType( *(psetupdata->clsID)
                                      , psetupdata->lpPin[m1].strName
                                      , *(psetupdata->lpPin[m1].lpMediaType[m2].clsMajorType)
                                      , *(psetupdata->lpPin[m1].lpMediaType[m2].clsMinorType) );
            if( FAILED(hr) ) break;
          }
          if( FAILED(hr) ) break;
        }
        if( FAILED(hr) ) break;
      }
    }
  }

  // handle one acceptable "error" - that
  // of filter not being registered!
  // (couldn't find a suitable #define'd
  // name for the error!)
  //
  if( 0x80070002 == hr)
    return NOERROR;
  else
    return hr;
}

//  Remove warnings about unreferenced inline functions
#pragma warning(disable:4514)

