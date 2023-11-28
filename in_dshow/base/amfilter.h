//------------------------------------------------------------------------------
// File: AMFilter.h
//
// Desc: DirectShow base classes - efines class hierarchy for streams
//       architecture.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __FILTER__
#define __FILTER__

/* The following classes are declared in this header: */

class CBaseMediaFilter;     // IMediaFilter support
class CBaseFilter;          // IBaseFilter,IMediaFilter support
class CBasePin;             // Abstract base class for IPin interface
class CEnumPins;            // Enumerate input and output pins
class CEnumMediaTypes;      // Enumerate the pin's preferred formats
class CBaseOutputPin;       // Adds data provider member functions
class CBaseInputPin;        // Implements IMemInputPin interface
class CMediaSample;         // Basic transport unit for IMemInputPin
class CBaseAllocator;       // General list guff for most allocators
class CMemAllocator;        // Implements memory buffer allocation


//=====================================================================
//=====================================================================
//
// QueryFilterInfo and QueryPinInfo AddRef the interface pointers
// they return.  You can use the macro below to release the interface.
//
//=====================================================================
//=====================================================================

#define QueryFilterInfoReleaseGraph(fi) if ((fi).pGraph) (fi).pGraph->Release();

#define QueryPinInfoReleaseFilter(pi) if ((pi).pFilter) (pi).pFilter->Release();
//=====================================================================
//=====================================================================
// Defines CBaseFilter
//
// An abstract class providing basic IBaseFilter support for pin
// enumeration and filter information reading.
//
// We cannot derive from CBaseMediaFilter since methods in IMediaFilter
// are also in IBaseFilter and would be ambiguous. Since much of the code
// assumes that they derive from a class that has m_State and other state
// directly available, we duplicate code from CBaseMediaFilter rather than
// having a member variable.
//
// Derive your filter from this, or from a derived object such as
// CTransformFilter.
//=====================================================================
//=====================================================================


class AM_NOVTABLE CBaseFilter : public CUnknown,        // Handles an IUnknown
                    public IBaseFilter,     // The Filter Interface
                    public IAMovieSetup     // For un/registration
{

friend class CBasePin;

protected:
    FILTER_STATE    m_State;            // current state: running, paused
    IReferenceClock *m_pClock;          // this graph's ref clock
    CRefTime        m_tStart;           // offset from stream time to reference time
    CLSID	    m_clsid;            // This filters clsid
                                        // used for serialization
    CCritSec        *m_pLock;           // Object we use for locking

    WCHAR           *m_pName;           // Full filter name
    IFilterGraph    *m_pGraph;          // Graph we belong to
    IMediaEventSink *m_pSink;           // Called with notify events
    LONG            m_PinVersion;       // Current pin version

public:

    CBaseFilter(
        const TCHAR *pName,     // Object description
        LPUNKNOWN pUnk,         // IUnknown of delegating object
        CCritSec  *pLock,       // Object who maintains lock
	REFCLSID   clsid);      // The clsid to be used to serialize this filter

    CBaseFilter(
        TCHAR     *pName,       // Object description
        LPUNKNOWN pUnk,         // IUnknown of delegating object
        CCritSec  *pLock,       // Object who maintains lock
	REFCLSID   clsid,       // The clsid to be used to serialize this filter
        HRESULT   *phr);        // General OLE return code
#ifdef UNICODE
    CBaseFilter(
        const CHAR *pName,     // Object description
        LPUNKNOWN pUnk,         // IUnknown of delegating object
        CCritSec  *pLock,       // Object who maintains lock
	REFCLSID   clsid);      // The clsid to be used to serialize this filter

    CBaseFilter(
        CHAR     *pName,       // Object description
        LPUNKNOWN pUnk,         // IUnknown of delegating object
        CCritSec  *pLock,       // Object who maintains lock
	REFCLSID   clsid,       // The clsid to be used to serialize this filter
        HRESULT   *phr);        // General OLE return code
#endif
    ~CBaseFilter();

    DECLARE_IUNKNOWN

    // override this to say what interfaces we support where
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
#ifdef DEBUG
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
#endif

    //
    // --- IPersist method ---
    //

    STDMETHODIMP GetClassID(CLSID *pClsID);

    // --- IMediaFilter methods ---

    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);

    STDMETHODIMP GetSyncSource(IReferenceClock **pClock);


    // override Stop and Pause so we can activate the pins.
    // Note that Run will call Pause first if activation needed.
    // Override these if you want to activate your filter rather than
    // your pins.
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

    // the start parameter is the difference to be added to the
    // sample's stream time to get the reference time for
    // its presentation
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    // --- helper methods ---

    // return the current stream time - ie find out what
    // stream time should be appearing now
    virtual HRESULT StreamTime(CRefTime& rtStream);

    // Is the filter currently active?
    BOOL IsActive() {
        CAutoLock cObjectLock(m_pLock);
        return ((m_State == State_Paused) || (m_State == State_Running));
    };

    // Is this filter stopped (without locking)
    BOOL IsStopped() {
        return (m_State == State_Stopped);
    };

    //
    // --- IBaseFilter methods ---
    //

    // pin enumerator
    STDMETHODIMP EnumPins(
                    IEnumPins ** ppEnum);


    // default behaviour of FindPin assumes pin ids are their names
    STDMETHODIMP FindPin(
        LPCWSTR Id,
        IPin ** ppPin
    );

    STDMETHODIMP QueryFilterInfo(
                    FILTER_INFO * pInfo);

    STDMETHODIMP JoinFilterGraph(
                    IFilterGraph * pGraph,
                    LPCWSTR pName);

    // return a Vendor information string. Optional - may return E_NOTIMPL.
    // memory returned should be freed using CoTaskMemFree
    // default implementation returns E_NOTIMPL
    STDMETHODIMP QueryVendorInfo(
                    LPWSTR* pVendorInfo
            );

    // --- helper methods ---

    // send an event notification to the filter graph if we know about it.
    // returns S_OK if delivered, S_FALSE if the filter graph does not sink
    // events, or an error otherwise.
    HRESULT NotifyEvent(
        long EventCode,
        LONG_PTR EventParam1,
        LONG_PTR EventParam2);

    // return the filter graph we belong to
    IFilterGraph *GetFilterGraph() {
        return m_pGraph;
    }

    // Request reconnect
    // pPin is the pin to reconnect
    // pmt is the type to reconnect with - can be NULL
    // Calls ReconnectEx on the filter graph
    HRESULT ReconnectPin(IPin *pPin, AM_MEDIA_TYPE const *pmt);

    // find out the current pin version (used by enumerators)
    virtual LONG GetPinVersion();
    void IncrementPinVersion();

    // you need to supply these to access the pins from the enumerator
    // and for default Stop and Pause/Run activation.
    virtual int GetPinCount() PURE;
    virtual CBasePin *GetPin(int n) PURE;

    // --- IAMovieSetup methods ---

    STDMETHODIMP Register();    // ask filter to register itself
    STDMETHODIMP Unregister();  // and unregister itself

    // --- setup helper methods ---
    // (override to return filters setup data)

    virtual LPAMOVIESETUP_FILTER GetSetupData(){ return NULL; }

};


//=====================================================================
//=====================================================================
// Defines CBasePin
//
// Abstract class that supports the basics of IPin
//=====================================================================
//=====================================================================

class  AM_NOVTABLE CBasePin : public CUnknown, public IPin, public IQualityControl
{

protected:

    WCHAR *         m_pName;		        // This pin's name
    IPin            *m_Connected;               // Pin we have connected to
    PIN_DIRECTION   m_dir;                      // Direction of this pin
    CCritSec        *m_pLock;                   // Object we use for locking
    bool            m_bRunTimeError;            // Run time error generated
    bool            m_bCanReconnectWhenActive;  // OK to reconnect when active
    bool            m_bTryMyTypesFirst;         // When connecting enumerate
                                                // this pin's types first
    CBaseFilter    *m_pFilter;                  // Filter we were created by
    IQualityControl *m_pQSink;                  // Target for Quality messages
    LONG            m_TypeVersion;              // Holds current type version
    CMediaType      m_mt;                       // Media type of connection

    CRefTime        m_tStart;                   // time from NewSegment call
    CRefTime        m_tStop;                    // time from NewSegment
    double          m_dRate;                    // rate from NewSegment

#ifdef DEBUG
    LONG            m_cRef;                     // Ref count tracing
#endif

    // displays pin connection information

#ifdef DEBUG
    void DisplayPinInfo(IPin *pReceivePin);
    void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt);
#else
    void DisplayPinInfo(IPin *pReceivePin) {};
    void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt) {};
#endif

    // used to agree a media type for a pin connection

    // given a specific media type, attempt a connection (includes
    // checking that the type is acceptable to this pin)
    HRESULT
    AttemptConnection(
        IPin* pReceivePin,      // connect to this pin
        const CMediaType* pmt   // using this type
    );

    // try all the media types in this enumerator - for each that
    // we accept, try to connect using ReceiveConnection.
    HRESULT TryMediaTypes(
                        IPin *pReceivePin,      // connect to this pin
                        const CMediaType *pmt,        // proposed type from Connect
                        IEnumMediaTypes *pEnum);    // try this enumerator

    // establish a connection with a suitable mediatype. Needs to
    // propose a media type if the pmt pointer is null or partially
    // specified - use TryMediaTypes on both our and then the other pin's
    // enumerator until we find one that works.
    HRESULT AgreeMediaType(
                        IPin *pReceivePin,      // connect to this pin
                        const CMediaType *pmt);       // proposed type from Connect

public:

    CBasePin(
        TCHAR *pObjectName,         // Object description
        CBaseFilter *pFilter,       // Owning filter who knows about pins
        CCritSec *pLock,            // Object who implements the lock
        HRESULT *phr,               // General OLE return code
        LPCWSTR pName,              // Pin name for us
        PIN_DIRECTION dir);         // Either PINDIR_INPUT or PINDIR_OUTPUT
#ifdef UNICODE
    CBasePin(
        CHAR *pObjectName,         // Object description
        CBaseFilter *pFilter,       // Owning filter who knows about pins
        CCritSec *pLock,            // Object who implements the lock
        HRESULT *phr,               // General OLE return code
        LPCWSTR pName,              // Pin name for us
        PIN_DIRECTION dir);         // Either PINDIR_INPUT or PINDIR_OUTPUT
#endif
    virtual ~CBasePin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();

    // --- IPin methods ---

    // take lead role in establishing a connection. Media type pointer
    // may be null, or may point to partially-specified mediatype
    // (subtype or format type may be GUID_NULL).
    STDMETHODIMP Connect(
        IPin * pReceivePin,
        const AM_MEDIA_TYPE *pmt   // optional media type
    );

    // (passive) accept a connection from another pin
    STDMETHODIMP ReceiveConnection(
        IPin * pConnector,      // this is the initiating connecting pin
        const AM_MEDIA_TYPE *pmt   // this is the media type we will exchange
    );

    STDMETHODIMP Disconnect();

    STDMETHODIMP ConnectedTo(IPin **pPin);

    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);

    STDMETHODIMP QueryPinInfo(
        PIN_INFO * pInfo
    );

    STDMETHODIMP QueryDirection(
    	PIN_DIRECTION * pPinDir
    );

    STDMETHODIMP QueryId(
        LPWSTR * Id
    );

    // does the pin support this media type
    STDMETHODIMP QueryAccept(
        const AM_MEDIA_TYPE *pmt
    );

    // return an enumerator for this pins preferred media types
    STDMETHODIMP EnumMediaTypes(
        IEnumMediaTypes **ppEnum
    );

    // return an array of IPin* - the pins that this pin internally connects to
    // All pins put in the array must be AddReffed (but no others)
    // Errors: "Can't say" - FAIL, not enough slots - return S_FALSE
    // Default: return E_NOTIMPL
    // The filter graph will interpret NOT_IMPL as any input pin connects to
    // all visible output pins and vice versa.
    // apPin can be NULL if nPin==0 (not otherwise).
    STDMETHODIMP QueryInternalConnections(
        IPin* *apPin,     // array of IPin*
        ULONG *nPin       // on input, the number of slots
                          // on output  the number of pins
    ) { return E_NOTIMPL; }

    // Called when no more data will be sent
    STDMETHODIMP EndOfStream(void);

    // Begin/EndFlush still PURE

    // NewSegment notifies of the start/stop/rate applying to the data
    // about to be received. Default implementation records data and
    // returns S_OK.
    // Override this to pass downstream.
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

    //================================================================================
    // IQualityControl methods
    //================================================================================

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    STDMETHODIMP SetSink(IQualityControl * piqc);

    // --- helper methods ---

    // Returns true if the pin is connected. false otherwise.
    BOOL IsConnected(void) {return (m_Connected != NULL); };
    // Return the pin this is connected to (if any)
    IPin * GetConnected() { return m_Connected; };

    // Check if our filter is currently stopped
    BOOL IsStopped() {
        return (m_pFilter->m_State == State_Stopped);
    };

    // find out the current type version (used by enumerators)
    virtual LONG GetMediaTypeVersion();
    void IncrementTypeVersion();

    // switch the pin to active (paused or running) mode
    // not an error to call this if already active
    virtual HRESULT Active(void);

    // switch the pin to inactive state - may already be inactive
    virtual HRESULT Inactive(void);

    // Notify of Run() from filter
    virtual HRESULT Run(REFERENCE_TIME tStart);

    // check if the pin can support this specific proposed type and format
    virtual HRESULT CheckMediaType(const CMediaType *) PURE;

    // set the connection to use this format (previously agreed)
    virtual HRESULT SetMediaType(const CMediaType *);

    // check that the connection is ok before verifying it
    // can be overridden eg to check what interfaces will be supported.
    virtual HRESULT CheckConnect(IPin *);

    // Set and release resources required for a connection
    virtual HRESULT BreakConnect();
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

    // returns the preferred formats for a pin
    virtual HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

    // access to NewSegment values
    REFERENCE_TIME CurrentStopTime() {
        return m_tStop;
    }
    REFERENCE_TIME CurrentStartTime() {
        return m_tStart;
    }
    double CurrentRate() {
        return m_dRate;
    }

    //  Access name
    LPWSTR Name() { return m_pName; };

    //  Can reconnectwhen active?
    void SetReconnectWhenActive(bool bCanReconnect)
    {
        m_bCanReconnectWhenActive = bCanReconnect;
    }

    bool CanReconnectWhenActive()
    {
        return m_bCanReconnectWhenActive;
    }

protected:
    STDMETHODIMP DisconnectInternal();
};


//=====================================================================
//=====================================================================
// Defines CEnumPins
//
// Pin enumerator class that works by calling CBaseFilter. This interface
// is provided by CBaseFilter::EnumPins and calls GetPinCount() and
// GetPin() to enumerate existing pins. Needs to be a separate object so
// that it can be cloned (creating an existing object at the same
// position in the enumeration)
//
//=====================================================================
//=====================================================================

class CEnumPins : public IEnumPins      // The interface we support
{
    int m_Position;                 // Current ordinal position
    int m_PinCount;                 // Number of pins available
    CBaseFilter *m_pFilter;         // The filter who owns us
    LONG m_Version;                 // Pin version information
    LONG m_cRef;

    typedef CGenericList<CBasePin> CPinList;

    CPinList m_PinCache;	    // These pointers have not been AddRef'ed and
				    // so they should not be dereferenced.  They are
				    // merely kept to ID which pins have been enumerated.

#ifdef DEBUG
    DWORD m_dwCookie;
#endif

    /* If while we are retrieving a pin for example from the filter an error
       occurs we assume that our internal state is stale with respect to the
       filter (someone may have deleted all the pins). We can check before
       starting whether or not the operation is likely to fail by asking the
       filter what it's current version number is. If the filter has not
       overriden the GetPinVersion method then this will always match */

    BOOL AreWeOutOfSync() {
        return (m_pFilter->GetPinVersion() == m_Version ? FALSE : TRUE);
    };

    /* This method performs the same operations as Reset, except is does not clear
       the cache of pins already enumerated. */

    STDMETHODIMP Refresh();

public:

    CEnumPins(
        CBaseFilter *pFilter,
        CEnumPins *pEnumPins);

    virtual ~CEnumPins();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IEnumPins
    STDMETHODIMP Next(
        ULONG cPins,         // place this many pins...
        IPin ** ppPins,      // ...in this array of IPin*
        ULONG * pcFetched    // actual count passed returned here
    );

    STDMETHODIMP Skip(ULONG cPins);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumPins **ppEnum);


};


//=====================================================================
//=====================================================================
// Defines CEnumMediaTypes
//
// Enumerates the preferred formats for input and output pins
//=====================================================================
//=====================================================================

class CEnumMediaTypes : public IEnumMediaTypes    // The interface we support
{
    int m_Position;           // Current ordinal position
    CBasePin *m_pPin;         // The pin who owns us
    LONG m_Version;           // Media type version value
    LONG m_cRef;
#ifdef DEBUG
    DWORD m_dwCookie;
#endif

    /* The media types a filter supports can be quite dynamic so we add to
       the general IEnumXXXX interface the ability to be signaled when they
       change via an event handle the connected filter supplies. Until the
       Reset method is called after the state changes all further calls to
       the enumerator (except Reset) will return E_UNEXPECTED error code */

    BOOL AreWeOutOfSync() {
        return (m_pPin->GetMediaTypeVersion() == m_Version ? FALSE : TRUE);
    };

public:

    CEnumMediaTypes(
        CBasePin *pPin,
        CEnumMediaTypes *pEnumMediaTypes);

    virtual ~CEnumMediaTypes();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IEnumMediaTypes
    STDMETHODIMP Next(
        ULONG cMediaTypes,          // place this many pins...
        AM_MEDIA_TYPE ** ppMediaTypes,  // ...in this array
        ULONG * pcFetched           // actual count passed
    );

    STDMETHODIMP Skip(ULONG cMediaTypes);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumMediaTypes **ppEnum);
};



//=====================================================================
//=====================================================================
// Defines CBaseInputPin
//
// derive your standard input pin from this.
// you need to supply GetMediaType and CheckConnect etc (see CBasePin),
// and you need to supply Receive to do something more useful.
//
//=====================================================================
//=====================================================================

class AM_NOVTABLE CBaseInputPin : public CBasePin,
                                  public IMemInputPin
{

protected:

    IMemAllocator *m_pAllocator;    // Default memory allocator

    // allocator is read-only, so received samples
    // cannot be modified (probably only relevant to in-place
    // transforms
    BYTE m_bReadOnly;

    // in flushing state (between BeginFlush and EndFlush)
    // if TRUE, all Receives are returned with S_FALSE
    BYTE m_bFlushing;

    // Sample properties - initalized in Receive
    AM_SAMPLE2_PROPERTIES m_SampleProps;

public:

    CBaseInputPin(
        TCHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#ifdef UNICODE
    CBaseInputPin(
        CHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#endif
    virtual ~CBaseInputPin();

    DECLARE_IUNKNOWN

    // override this to publicise our interfaces
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    // return the allocator interface that this input pin
    // would like the output pin to use
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

    // tell the input pin which allocator the output pin is actually
    // going to use.
    STDMETHODIMP NotifyAllocator(
                    IMemAllocator * pAllocator,
                    BOOL bReadOnly);

    // do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);

    // do something with these media samples
    STDMETHODIMP ReceiveMultiple (
        IMediaSample **pSamples,
        long nSamples,
        long *nSamplesProcessed);

    // See if Receive() blocks
    STDMETHODIMP ReceiveCanBlock();

    // Default handling for BeginFlush - call at the beginning
    // of your implementation (makes sure that all Receive calls
    // fail). After calling this, you need to free any queued data
    // and then call downstream.
    STDMETHODIMP BeginFlush(void);

    // default handling for EndFlush - call at end of your implementation
    // - before calling this, ensure that there is no queued data and no thread
    // pushing any more without a further receive, then call downstream,
    // then call this method to clear the m_bFlushing flag and re-enable
    // receives
    STDMETHODIMP EndFlush(void);

    // this method is optional (can return E_NOTIMPL).
    // default implementation returns E_NOTIMPL. Override if you have
    // specific alignment or prefix needs, but could use an upstream
    // allocator
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);

    // Release the pin's allocator.
    HRESULT BreakConnect();

    // helper method to check the read-only flag
    BOOL IsReadOnly() {
        return m_bReadOnly;
    };

    // helper method to see if we are flushing
    BOOL IsFlushing() {
        return m_bFlushing;
    };

    //  Override this for checking whether it's OK to process samples
    //  Also call this from EndOfStream.
    virtual HRESULT CheckStreaming();

    // Pass a Quality notification on to the appropriate sink
    HRESULT PassNotify(Quality& q);


    //================================================================================
    // IQualityControl methods (from CBasePin)
    //================================================================================

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    // no need to override:
    // STDMETHODIMP SetSink(IQualityControl * piqc);


    // switch the pin to inactive state - may already be inactive
    virtual HRESULT Inactive(void);

    // Return sample properties pointer
    AM_SAMPLE2_PROPERTIES * SampleProps() {
        ASSERT(m_SampleProps.cbData != 0);
        return &m_SampleProps;
    }

};



//=====================================================================
//=====================================================================
// Memory allocators
//
// the shared memory transport between pins requires the input pin
// to provide a memory allocator that can provide sample objects. A
// sample object supports the IMediaSample interface.
//
// CBaseAllocator handles the management of free and busy samples. It
// allocates CMediaSample objects. CBaseAllocator is an abstract class:
// in particular it has no method of initializing the list of free
// samples. CMemAllocator is derived from CBaseAllocator and initializes
// the list of samples using memory from the standard IMalloc interface.
//
// If you want your buffers to live in some special area of memory,
// derive your allocator object from CBaseAllocator. If you derive your
// IMemInputPin interface object from CBaseMemInputPin, you will get
// CMemAllocator-based allocation etc for free and will just need to
// supply the Receive handling, and media type / format negotiation.
//=====================================================================
//=====================================================================



//  Make me one from quartz.dll
STDAPI CreateMemoryAllocator(IMemAllocator **ppAllocator);

// helper used by IAMovieSetup implementation
STDAPI
AMovieSetupRegisterFilter( const AMOVIESETUP_FILTER * const psetupdata
                         , IFilterMapper *                  pIFM
                         , BOOL                             bRegister  );


///////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////

#endif /* __FILTER__ */



