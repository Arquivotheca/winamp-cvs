//------------------------------------------------------------------------------
// File: WXUtil.h
//
// Desc: DirectShow base classes - defines helper classes and functions for
//       building multimedia filters.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __WXUTIL__
#define __WXUTIL__

// eliminate spurious "statement has no effect" warnings.
#pragma warning(disable: 4705)

// wrapper for whatever critical section we have
class CCritSec {

    // make copy constructor and assignment operator inaccessible

    CCritSec(const CCritSec &refCritSec);
    CCritSec &operator=(const CCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

#ifdef DEBUG
public:
    DWORD   m_currentOwner;
    DWORD   m_lockCount;
    BOOL    m_fTrace;        // Trace this one
public:
    CCritSec();
    ~CCritSec();
    void Lock();
    void Unlock();
#else

public:
    CCritSec() {
        InitializeCriticalSection(&m_CritSec);
    };

    ~CCritSec() {
        DeleteCriticalSection(&m_CritSec);
    };

    void Lock() {
        EnterCriticalSection(&m_CritSec);
    };

    void Unlock() {
        LeaveCriticalSection(&m_CritSec);
    };
#endif
};

//
// To make deadlocks easier to track it is useful to insert in the
// code an assertion that says whether we own a critical section or
// not.  We make the routines that do the checking globals to avoid
// having different numbers of member functions in the debug and
// retail class implementations of CCritSec.  In addition we provide
// a routine that allows usage of specific critical sections to be
// traced.  This is NOT on by default - there are far too many.
//

#ifdef DEBUG
    BOOL WINAPI CritCheckIn(CCritSec * pcCrit);
    BOOL WINAPI CritCheckIn(const CCritSec * pcCrit);
    BOOL WINAPI CritCheckOut(CCritSec * pcCrit);
    BOOL WINAPI CritCheckOut(const CCritSec * pcCrit);
    void WINAPI DbgLockTrace(CCritSec * pcCrit, BOOL fTrace);
#else
    #define CritCheckIn(x) TRUE
    #define CritCheckOut(x) TRUE
    #define DbgLockTrace(pc, fT)
#endif


// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock {

    // make copy constructor and assignment operator inaccessible

    CAutoLock(const CAutoLock &refAutoLock);
    CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
    CCritSec * m_pLock;

public:
    CAutoLock(CCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CAutoLock() {
        m_pLock->Unlock();
    };
};



// wrapper for event objects
class CAMEvent
{

    // make copy constructor and assignment operator inaccessible

    CAMEvent(const CAMEvent &refEvent);
    CAMEvent &operator=(const CAMEvent &refEvent);

protected:
    HANDLE m_hEvent;
public:
    CAMEvent(BOOL fManualReset = FALSE);
    ~CAMEvent();

    // Cast to HANDLE - we don't support this as an lvalue
    operator HANDLE () const { return m_hEvent; };

    void Set() {EXECUTE_ASSERT(SetEvent(m_hEvent));};
    BOOL Wait(DWORD dwTimeout = INFINITE) {
	return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
    };
    void Reset() { ResetEvent(m_hEvent); };
    BOOL Check() { return Wait(0); };
};


// wrapper for event objects that do message processing


// old name supported for the time being
#define CTimeoutEvent CAMEvent



extern "C"
void * __stdcall memmoveInternal(void *, const void *, size_t);

inline void * __cdecl memchrInternal(const void *buf, int chr, size_t cnt)
{
#ifdef _X86_
    void *pRet = NULL;

    _asm {
        cld                 // make sure we get the direction right
        mov     ecx, cnt    // num of bytes to scan
        mov     edi, buf    // pointer byte stream
        mov     eax, chr    // byte to scan for
        repne   scasb       // look for the byte in the byte stream
        jnz     exit_memchr // Z flag set if byte found
        dec     edi         // scasb always increments edi even when it
                            // finds the required byte
        mov     pRet, edi
exit_memchr:
    }
    return pRet;

#else
    while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) ) {
        buf = (unsigned char *)buf + 1;
        cnt--;
    }

    return(cnt ? (void *)buf : NULL);
#endif
}

void WINAPI IntToWstr(int i, LPWSTR wstr, size_t len);

#define WstrToInt(sz) _wtoi(sz)
#define atoiW(sz) _wtoi(sz)
#define atoiA(sz) atoi(sz)

// These are available to help managing bitmap VIDEOINFOHEADER media structures

extern const DWORD bits555[3];
extern const DWORD bits565[3];
extern const DWORD bits888[3];



// Compares two interfaces and returns TRUE if they are on the same object
BOOL WINAPI IsEqualObject(IUnknown *pFirst, IUnknown *pSecond);

// This is for comparing pins
#define EqualPins(pPin1, pPin2) IsEqualObject(pPin1, pPin2)


// Arithmetic helper functions

// Compute (a * b + rnd) / c
LONGLONG WINAPI llMulDiv(LONGLONG a, LONGLONG b, LONGLONG c, LONGLONG rnd);
LONGLONG WINAPI Int64x32Div32(LONGLONG a, LONG b, LONG c, LONG rnd);


// Avoids us dyna-linking to SysAllocString to copy BSTR strings
STDAPI WriteBSTR(BSTR * pstrDest, LPCWSTR szSrc);
STDAPI FreeBSTR(BSTR* pstr);

// Return a wide string - allocating memory for it
// Returns:
//    S_OK          - no error
//    E_POINTER     - ppszReturn == NULL
//    E_OUTOFMEMORY - can't allocate memory for returned string
STDAPI AMGetWideString(LPCWSTR pszString, LPWSTR *ppszReturn);

// Special wait for objects owning windows
DWORD WINAPI WaitDispatchingMessages(
    HANDLE hObject,
    DWORD dwWait,
    HWND hwnd = NULL,
    UINT uMsg = 0,
    HANDLE hEvent = NULL);

// HRESULT_FROM_WIN32 converts ERROR_SUCCESS to a success code, but in
// our use of HRESULT_FROM_WIN32, it typically means a function failed
// to call SetLastError(), and we still want a failure code.
//
#define AmHresultFromWin32(x) (MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, x))

// call GetLastError and return an HRESULT value that will fail the
// SUCCEEDED() macro.
HRESULT AmGetLastErrorToHResult(void);



MMRESULT CompatibleTimeSetEvent( UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent );
bool TimeKillSynchronousFlagAvailable( void );

#endif /* __WXUTIL__ */
