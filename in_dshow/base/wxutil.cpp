//------------------------------------------------------------------------------
// File: WXUtil.cpp
//
// Desc: DirectShow base classes - implements helper classes for building
//       multimedia filters.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>

//
//  Declare function from largeint.h we need so that PPC can build
//

//
// Enlarged integer divide - 64-bits / 32-bits > 32-bits
//

#ifndef _X86_

#define LLtoU64(x) (*(unsigned __int64*)(void*)(&(x)))

__inline
ULONG
WINAPI
EnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
        // return remainder if necessary
        if (Remainder != NULL)
                *Remainder = (ULONG)(LLtoU64(Dividend) % Divisor);
        return (ULONG)(LLtoU64(Dividend) / Divisor);
}

#else
__inline
ULONG
WINAPI
EnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
    ULONG ulResult;
    _asm {
        mov eax,Dividend.LowPart
        mov edx,Dividend.HighPart
        mov ecx,Remainder
        div Divisor
        or  ecx,ecx
        jz  short label
        mov [ecx],edx
label:
        mov ulResult,eax
    }
    return ulResult;
}
#endif

// --- CAMEvent -----------------------
CAMEvent::CAMEvent(BOOL fManualReset)
{
    m_hEvent = CreateEvent(NULL, fManualReset, FALSE, NULL);
}

CAMEvent::~CAMEvent()
{
    if (m_hEvent) {
	EXECUTE_ASSERT(CloseHandle(m_hEvent));
    }
}



// Helper function - convert int to WSTR
void WINAPI IntToWstr(int i, LPWSTR wstr, size_t len)
{
#ifdef UNICODE
    (void)StringCchPrintf(wstr, len, L"%d", i);
#else
    TCHAR temp[32];
    (void)StringCchPrintf(temp, NUMELMS(temp), "%d", i);
    MultiByteToWideChar(CP_ACP, 0, temp, -1, wstr, int(len) );
#endif
} // IntToWstr


#if 0
void * memchrInternal(const void *pv, int c, size_t sz)
{
    BYTE *pb = (BYTE *) pv;
    while (sz--) {
	if (*pb == c)
	    return (void *) pb;
	pb++;
    }
    return NULL;
}
#endif


#define MEMORY_ALIGNMENT        4
#define MEMORY_ALIGNMENT_LOG2   2
#define MEMORY_ALIGNMENT_MASK   MEMORY_ALIGNMENT - 1

void * __stdcall memmoveInternal(void * dst, const void * src, size_t count)
{
    void * ret = dst;

#ifdef _X86_
    if (dst <= src || (char *)dst >= ((char *)src + count)) {

        /*
         * Non-Overlapping Buffers
         * copy from lower addresses to higher addresses
         */
        _asm {
            mov     esi,src
            mov     edi,dst
            mov     ecx,count
            cld
            mov     edx,ecx
            and     edx,MEMORY_ALIGNMENT_MASK
            shr     ecx,MEMORY_ALIGNMENT_LOG2
            rep     movsd
            or      ecx,edx
            jz      memmove_done
            rep     movsb
memmove_done:
        }
    }
    else {

        /*
         * Overlapping Buffers
         * copy from higher addresses to lower addresses
         */
        _asm {
            mov     esi,src
            mov     edi,dst
            mov     ecx,count
            std
            add     esi,ecx
            add     edi,ecx
            dec     esi
            dec     edi
            rep     movsb
            cld
        }
    }
#else
    MoveMemory(dst, src, count);
#endif

    return ret;
}

/*  Arithmetic functions to help with time format conversions
*/

#ifdef _M_ALPHA
// work around bug in version 12.00.8385 of the alpha compiler where
// UInt32x32To64 sign-extends its arguments (?)
#undef UInt32x32To64
#define UInt32x32To64(a, b) (((ULONGLONG)((ULONG)(a)) & 0xffffffff) * ((ULONGLONG)((ULONG)(b)) & 0xffffffff))
#endif

/*   Compute (a * b + d) / c */
LONGLONG WINAPI llMulDiv(LONGLONG a, LONGLONG b, LONGLONG c, LONGLONG d)
{
    /*  Compute the absolute values to avoid signed arithmetic problems */
    ULARGE_INTEGER ua, ub;
    DWORDLONG uc;

    ua.QuadPart = (DWORDLONG)(a >= 0 ? a : -a);
    ub.QuadPart = (DWORDLONG)(b >= 0 ? b : -b);
    uc          = (DWORDLONG)(c >= 0 ? c : -c);
    BOOL bSign = (a < 0) ^ (b < 0);

    /*  Do long multiplication */
    ULARGE_INTEGER p[2];
    p[0].QuadPart  = UInt32x32To64(ua.LowPart, ub.LowPart);

    /*  This next computation cannot overflow into p[1].HighPart because
        the max number we can compute here is:

                 (2 ** 32 - 1) * (2 ** 32 - 1) +  // ua.LowPart * ub.LowPart
    (2 ** 32) *  (2 ** 31) * (2 ** 32 - 1) * 2    // x.LowPart * y.HighPart * 2

    == 2 ** 96 - 2 ** 64 + (2 ** 64 - 2 ** 33 + 1)
    == 2 ** 96 - 2 ** 33 + 1
    < 2 ** 96
    */

    ULARGE_INTEGER x;
    x.QuadPart     = UInt32x32To64(ua.LowPart, ub.HighPart) +
                     UInt32x32To64(ua.HighPart, ub.LowPart) +
                     p[0].HighPart;
    p[0].HighPart  = x.LowPart;
    p[1].QuadPart  = UInt32x32To64(ua.HighPart, ub.HighPart) + x.HighPart;

    if (d != 0) {
        ULARGE_INTEGER ud[2];
        if (bSign) {
            ud[0].QuadPart = (DWORDLONG)(-d);
            if (d > 0) {
                /*  -d < 0 */
                ud[1].QuadPart = (DWORDLONG)(LONGLONG)-1;
            } else {
                ud[1].QuadPart = (DWORDLONG)0;
            }
        } else {
            ud[0].QuadPart = (DWORDLONG)d;
            if (d < 0) {
                ud[1].QuadPart = (DWORDLONG)(LONGLONG)-1;
            } else {
                ud[1].QuadPart = (DWORDLONG)0;
            }
        }
        /*  Now do extended addition */
        ULARGE_INTEGER uliTotal;

        /*  Add ls DWORDs */
        uliTotal.QuadPart  = (DWORDLONG)ud[0].LowPart + p[0].LowPart;
        p[0].LowPart       = uliTotal.LowPart;

        /*  Propagate carry */
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

        /*  Add 2nd most ls DWORDs */
        uliTotal.QuadPart += (DWORDLONG)ud[0].HighPart + p[0].HighPart;
        p[0].HighPart      = uliTotal.LowPart;

        /*  Propagate carry */
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

        /*  Add MS DWORDLONGs - no carry expected */
        p[1].QuadPart     += ud[1].QuadPart + uliTotal.QuadPart;

        /*  Now see if we got a sign change from the addition */
        if ((LONG)p[1].HighPart < 0) {
            bSign = !bSign;

            /*  Negate the current value (ugh!) */
            p[0].QuadPart  = ~p[0].QuadPart;
            p[1].QuadPart  = ~p[1].QuadPart;
            p[0].QuadPart += 1;
            p[1].QuadPart += (p[0].QuadPart == 0);
        }
    }

    /*  Now for the division */
    if (c < 0) {
        bSign = !bSign;
    }


    /*  This will catch c == 0 and overflow */
    if (uc <= p[1].QuadPart) {
        return bSign ? (LONGLONG)0x8000000000000000 :
                       (LONGLONG)0x7FFFFFFFFFFFFFFF;
    }

    DWORDLONG ullResult;

    /*  Do the division */
    /*  If the dividend is a DWORD_LONG use the compiler */
    if (p[1].QuadPart == 0) {
        ullResult = p[0].QuadPart / uc;
        return bSign ? -(LONGLONG)ullResult : (LONGLONG)ullResult;
    }

    /*  If the divisor is a DWORD then its simpler */
    ULARGE_INTEGER ulic;
    ulic.QuadPart = uc;
    if (ulic.HighPart == 0) {
        ULARGE_INTEGER uliDividend;
        ULARGE_INTEGER uliResult;
        DWORD dwDivisor = (DWORD)uc;
        // ASSERT(p[1].HighPart == 0 && p[1].LowPart < dwDivisor);
        uliDividend.HighPart = p[1].LowPart;
        uliDividend.LowPart = p[0].HighPart;
#ifndef USE_LARGEINT
        uliResult.HighPart = (DWORD)(uliDividend.QuadPart / dwDivisor);
        p[0].HighPart = (DWORD)(uliDividend.QuadPart % dwDivisor);
        uliResult.LowPart = 0;
        uliResult.QuadPart = p[0].QuadPart / dwDivisor + uliResult.QuadPart;
#else
        /*  NOTE - this routine will take exceptions if
            the result does not fit in a DWORD
        */
        if (uliDividend.QuadPart >= (DWORDLONG)dwDivisor) {
            uliResult.HighPart = EnlargedUnsignedDivide(
                                     uliDividend,
                                     dwDivisor,
                                     &p[0].HighPart);
        } else {
            uliResult.HighPart = 0;
        }
        uliResult.LowPart = EnlargedUnsignedDivide(
                                 p[0],
                                 dwDivisor,
                                 NULL);
#endif
        return bSign ? -(LONGLONG)uliResult.QuadPart :
                        (LONGLONG)uliResult.QuadPart;
    }


    ullResult = 0;

    /*  OK - do long division */
    for (int i = 0; i < 64; i++) {
        ullResult <<= 1;

        /*  Shift 128 bit p left 1 */
        p[1].QuadPart <<= 1;
        if ((p[0].HighPart & 0x80000000) != 0) {
            p[1].LowPart++;
        }
        p[0].QuadPart <<= 1;

        /*  Compare */
        if (uc <= p[1].QuadPart) {
            p[1].QuadPart -= uc;
            ullResult += 1;
        }
    }

    return bSign ? - (LONGLONG)ullResult : (LONGLONG)ullResult;
}

LONGLONG WINAPI Int64x32Div32(LONGLONG a, LONG b, LONG c, LONG d)
{
    ULARGE_INTEGER ua;
    DWORD ub;
    DWORD uc;

    /*  Compute the absolute values to avoid signed arithmetic problems */
    ua.QuadPart = (DWORDLONG)(a >= 0 ? a : -a);
    ub = (DWORD)(b >= 0 ? b : -b);
    uc = (DWORD)(c >= 0 ? c : -c);
    BOOL bSign = (a < 0) ^ (b < 0);

    /*  Do long multiplication */
    ULARGE_INTEGER p0;
    DWORD p1;
    p0.QuadPart  = UInt32x32To64(ua.LowPart, ub);

    if (ua.HighPart != 0) {
        ULARGE_INTEGER x;
        x.QuadPart     = UInt32x32To64(ua.HighPart, ub) + p0.HighPart;
        p0.HighPart  = x.LowPart;
        p1   = x.HighPart;
    } else {
        p1 = 0;
    }

    if (d != 0) {
        ULARGE_INTEGER ud0;
        DWORD ud1;

        if (bSign) {
            //
            //  Cast d to LONGLONG first otherwise -0x80000000 sign extends
            //  incorrectly
            //
            ud0.QuadPart = (DWORDLONG)(-(LONGLONG)d);
            if (d > 0) {
                /*  -d < 0 */
                ud1 = (DWORD)-1;
            } else {
                ud1 = (DWORD)0;
            }
        } else {
            ud0.QuadPart = (DWORDLONG)d;
            if (d < 0) {
                ud1 = (DWORD)-1;
            } else {
                ud1 = (DWORD)0;
            }
        }
        /*  Now do extended addition */
        ULARGE_INTEGER uliTotal;

        /*  Add ls DWORDs */
        uliTotal.QuadPart  = (DWORDLONG)ud0.LowPart + p0.LowPart;
        p0.LowPart       = uliTotal.LowPart;

        /*  Propagate carry */
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

        /*  Add 2nd most ls DWORDs */
        uliTotal.QuadPart += (DWORDLONG)ud0.HighPart + p0.HighPart;
        p0.HighPart      = uliTotal.LowPart;

        /*  Add MS DWORDLONGs - no carry expected */
        p1 += ud1 + uliTotal.HighPart;

        /*  Now see if we got a sign change from the addition */
        if ((LONG)p1 < 0) {
            bSign = !bSign;

            /*  Negate the current value (ugh!) */
            p0.QuadPart  = ~p0.QuadPart;
            p1 = ~p1;
            p0.QuadPart += 1;
            p1 += (p0.QuadPart == 0);
        }
    }

    /*  Now for the division */
    if (c < 0) {
        bSign = !bSign;
    }


    /*  This will catch c == 0 and overflow */
    if (uc <= p1) {
        return bSign ? (LONGLONG)0x8000000000000000 :
                       (LONGLONG)0x7FFFFFFFFFFFFFFF;
    }

    /*  Do the division */

    /*  If the divisor is a DWORD then its simpler */
    ULARGE_INTEGER uliDividend;
    ULARGE_INTEGER uliResult;
    DWORD dwDivisor = uc;
    uliDividend.HighPart = p1;
    uliDividend.LowPart = p0.HighPart;
    /*  NOTE - this routine will take exceptions if
        the result does not fit in a DWORD
    */
    if (uliDividend.QuadPart >= (DWORDLONG)dwDivisor) {
        uliResult.HighPart = EnlargedUnsignedDivide(
                                 uliDividend,
                                 dwDivisor,
                                 &p0.HighPart);
    } else {
        uliResult.HighPart = 0;
    }
    uliResult.LowPart = EnlargedUnsignedDivide(
                             p0,
                             dwDivisor,
                             NULL);
    return bSign ? -(LONGLONG)uliResult.QuadPart :
                    (LONGLONG)uliResult.QuadPart;
}

#ifdef DEBUG
/******************************Public*Routine******************************\
* Debug CCritSec helpers
*
* We provide debug versions of the Constructor, destructor, Lock and Unlock
* routines.  The debug code tracks who owns each critical section by
* maintaining a depth count.
*
* History:
*
\**************************************************************************/

CCritSec::CCritSec()
{
    InitializeCriticalSection(&m_CritSec);
    m_currentOwner = m_lockCount = 0;
    m_fTrace = FALSE;
}

CCritSec::~CCritSec()
{
    DeleteCriticalSection(&m_CritSec);
}

void CCritSec::Lock()
{
    UINT tracelevel=3;
    DWORD us = GetCurrentThreadId();
    DWORD currentOwner = m_currentOwner;
    if (currentOwner && (currentOwner != us)) {
        // already owned, but not by us
        if (m_fTrace) {
            DbgLog((LOG_LOCKING, 2, TEXT("Thread %d about to wait for lock %x owned by %d"),
                GetCurrentThreadId(), &m_CritSec, currentOwner));
            tracelevel=2;
	        // if we saw the message about waiting for the critical
	        // section we ensure we see the message when we get the
	        // critical section
        }
    }
    EnterCriticalSection(&m_CritSec);
    if (0 == m_lockCount++) {
        // we now own it for the first time.  Set owner information
        m_currentOwner = us;

        if (m_fTrace) {
            DbgLog((LOG_LOCKING, tracelevel, TEXT("Thread %d now owns lock %x"), m_currentOwner, &m_CritSec));
        }
    }
}

void CCritSec::Unlock() {
    if (0 == --m_lockCount) {
        // about to be unowned
        if (m_fTrace) {
            DbgLog((LOG_LOCKING, 3, TEXT("Thread %d releasing lock %x"), m_currentOwner, &m_CritSec));
        }

        m_currentOwner = 0;
    }
    LeaveCriticalSection(&m_CritSec);
}

void WINAPI DbgLockTrace(CCritSec * pcCrit, BOOL fTrace)
{
    pcCrit->m_fTrace = fTrace;
}

BOOL WINAPI CritCheckIn(CCritSec * pcCrit)
{
    return (GetCurrentThreadId() == pcCrit->m_currentOwner);
}

BOOL WINAPI CritCheckIn(const CCritSec * pcCrit)
{
    return (GetCurrentThreadId() == pcCrit->m_currentOwner);
}

BOOL WINAPI CritCheckOut(CCritSec * pcCrit)
{
    return (GetCurrentThreadId() != pcCrit->m_currentOwner);
}

BOOL WINAPI CritCheckOut(const CCritSec * pcCrit)
{
    return (GetCurrentThreadId() != pcCrit->m_currentOwner);
}
#endif


STDAPI WriteBSTR(BSTR *pstrDest, LPCWSTR szSrc)
{
    *pstrDest = SysAllocString( szSrc );
    if( !(*pstrDest) ) return E_OUTOFMEMORY;
    return NOERROR;
}


STDAPI FreeBSTR(BSTR* pstr)
{
    if( *pstr == NULL ) return S_FALSE;
    SysFreeString( *pstr );
    return NOERROR;
}


// Return a wide string - allocating memory for it
// Returns:
//    S_OK          - no error
//    E_POINTER     - ppszReturn == NULL
//    E_OUTOFMEMORY - can't allocate memory for returned string
STDAPI AMGetWideString(LPCWSTR psz, LPWSTR *ppszReturn)
{
    CheckPointer(ppszReturn, E_POINTER);
    ValidateReadWritePtr(ppszReturn, sizeof(LPWSTR));
    DWORD nameLen = sizeof(WCHAR) * (lstrlenW(psz)+1);
    *ppszReturn = (LPWSTR)CoTaskMemAlloc(nameLen);
    if (*ppszReturn == NULL) {
       return E_OUTOFMEMORY;
    }
    CopyMemory(*ppszReturn, psz, nameLen);
    return NOERROR;
}

// Waits for the HANDLE hObject.  While waiting messages sent
// to windows on our thread by SendMessage will be processed.
// Using this function to do waits and mutual exclusion
// avoids some deadlocks in objects with windows.
// Return codes are the same as for WaitForSingleObject
DWORD WINAPI WaitDispatchingMessages(
    HANDLE hObject,
    DWORD dwWait,
    HWND hwnd,
    UINT uMsg,
    HANDLE hEvent)
{
    BOOL bPeeked = FALSE;
    DWORD dwResult;
    DWORD dwStart;
    DWORD dwThreadPriority;

    static UINT uMsgId = 0;

    HANDLE hObjects[2] = { hObject, hEvent };
    if (dwWait != INFINITE && dwWait != 0) {
        dwStart = GetTickCount();
    }
    for (; ; ) {
        DWORD nCount = NULL != hEvent ? 2 : 1;

        //  Minimize the chance of actually dispatching any messages
        //  by seeing if we can lock immediately.
        dwResult = WaitForMultipleObjects(nCount, hObjects, FALSE, 0);
        if (dwResult < WAIT_OBJECT_0 + nCount) {
            break;
        }

        DWORD dwTimeOut = dwWait;
        if (dwTimeOut > 10) {
            dwTimeOut = 10;
        }
        dwResult = MsgWaitForMultipleObjects(
                             nCount,
                             hObjects,
                             FALSE,
                             dwTimeOut,
                             hwnd == NULL ? QS_SENDMESSAGE :
                                            QS_SENDMESSAGE + QS_POSTMESSAGE);
        if (dwResult == WAIT_OBJECT_0 + nCount ||
            dwResult == WAIT_TIMEOUT && dwTimeOut != dwWait) {
            MSG msg;
            if (hwnd != NULL) {
                while (PeekMessage(&msg, hwnd, uMsg, uMsg, PM_REMOVE)) {
                    DispatchMessage(&msg);
                }
            }
            // Do this anyway - the previous peek doesn't flush out the
            // messages
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

            if (dwWait != INFINITE && dwWait != 0) {
                DWORD dwNow = GetTickCount();

                // Working with differences handles wrap-around
                DWORD dwDiff = dwNow - dwStart;
                if (dwDiff > dwWait) {
                    dwWait = 0;
                } else {
                    dwWait -= dwDiff;
                }
                dwStart = dwNow;
            }
            if (!bPeeked) {
                //  Raise our priority to prevent our message queue
                //  building up
                dwThreadPriority = GetThreadPriority(GetCurrentThread());
                if (dwThreadPriority < THREAD_PRIORITY_HIGHEST) {
                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
                }
                bPeeked = TRUE;
            }
        } else {
            break;
        }
    }
    if (bPeeked) {
        SetThreadPriority(GetCurrentThread(), dwThreadPriority);
        if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE) {
            if (uMsgId == 0) {
                uMsgId = RegisterWindowMessage(TEXT("AMUnblock"));
            }
            if (uMsgId != 0) {
                MSG msg;
                //  Remove old ones
                while (PeekMessage(&msg, (HWND)-1, uMsgId, uMsgId, PM_REMOVE)) {
                }
            }
            PostThreadMessage(GetCurrentThreadId(), uMsgId, 0, 0);
        }
    }
    return dwResult;
}

HRESULT AmGetLastErrorToHResult()
{
    DWORD dwLastError = GetLastError();
    if(dwLastError != 0)
    {
        return HRESULT_FROM_WIN32(dwLastError);
    }
    else
    {
        return E_FAIL;
    }
}

/******************************************************************************

CompatibleTimeSetEvent

    CompatibleTimeSetEvent() sets the TIME_KILL_SYNCHRONOUS flag before calling
timeSetEvent() if the current operating system supports it.  TIME_KILL_SYNCHRONOUS
is supported on Windows XP and later operating systems.

Parameters:
- The same parameters as timeSetEvent().  See timeSetEvent()'s documentation in 
the Platform SDK for more information.

Return Value:
- The same return value as timeSetEvent().  See timeSetEvent()'s documentation in 
the Platform SDK for more information.

******************************************************************************/
MMRESULT CompatibleTimeSetEvent( UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent )
{
    #if WINVER >= 0x0501
    {
        static bool fCheckedVersion = false;
        static bool fTimeKillSynchronousFlagAvailable = false; 

        if( !fCheckedVersion ) {
            fTimeKillSynchronousFlagAvailable = TimeKillSynchronousFlagAvailable();
            fCheckedVersion = true;
        }

        if( fTimeKillSynchronousFlagAvailable ) {
            fuEvent = fuEvent | TIME_KILL_SYNCHRONOUS;
        }
    }
    #endif // WINVER >= 0x0501

    return timeSetEvent( uDelay, uResolution, lpTimeProc, dwUser, fuEvent );
}

bool TimeKillSynchronousFlagAvailable( void )
{
    OSVERSIONINFO osverinfo;

    osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);

    if( GetVersionEx( &osverinfo ) ) {
        
        // Windows XP's major version is 5 and its' minor version is 1.
        // timeSetEvent() started supporting the TIME_KILL_SYNCHRONOUS flag
        // in Windows XP.
        if( (osverinfo.dwMajorVersion > 5) || 
            ( (osverinfo.dwMajorVersion == 5) && (osverinfo.dwMinorVersion >= 1) ) ) {
            return true;
        }
    }

    return false;
}
