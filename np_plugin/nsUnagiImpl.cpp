/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// ==============================
// ! Scriptability related code !
// ==============================

/////////////////////////////////////////////////////
//
// This file implements the nsWinamp object
// The native methods of this class are supposed to
// be callable from JavaScript
//

#include "nsIUnagiImpl.h" // this is where some useful macros defined
#include "plugin.h"
#include "nsISupportsUtils.h" // this is where some useful macros defined

#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIProtocolProxyService.h"
#include "nsIIOService.h"
#include "nsEmbedString.h"
#include "nsIURI.h"
#include "nsIProxyInfo.h"
#include "nsXPCOM.h"
#include "nsNetCID.h"
#include "nsmemory.h"
#include <malloc.h>

#define BUFFER_LEN 1024

#if defined(XP_MACOSX) || defined(linux)
// strncasecmp first appeared in 4.4BSD.
#define strnicmp strncasecmp
#define stricmp  strcasecmp
#endif

static NS_DEFINE_IID(kIUnagiIID, NS_IWINAMP_IID);
static NS_DEFINE_IID(kIClassInfoIID, NS_ICLASSINFO_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

//********************************************************************
nsWinamp::nsWinamp(nsPluginInstance* aPlugin) : mPlugin(aPlugin),
                                                      mRefCnt(0)
{

}

//********************************************************************
nsWinamp::~nsWinamp()
{

}

//********************************************************************
// AddRef, Release and QueryInterface are common methods and must 
// be implemented for any interface
NS_IMETHODIMP_(nsrefcnt) nsWinamp::AddRef() 
{ 
  ++mRefCnt; 
  return mRefCnt; 
} 

//********************************************************************
NS_IMETHODIMP_(nsrefcnt) nsWinamp::Release() 
{ 
  --mRefCnt; 
  if (mRefCnt == 0) { 
    delete this;
    return 0; 
  } 
  return mRefCnt; 
} 

//********************************************************************
// here nsWinamp should return three interfaces it can be asked for by their iid's
// static casts are necessary to ensure that correct pointer is returned
NS_IMETHODIMP nsWinamp::QueryInterface(const nsIID& aIID, void** aInstancePtr) 
{ 
  if(!aInstancePtr) 
    return NS_ERROR_NULL_POINTER; 

  if(aIID.Equals(kIUnagiIID)) {
    *aInstancePtr = NS_STATIC_CAST(nsIWinamp*, this); 
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kIClassInfoIID)) {
    *aInstancePtr = NS_STATIC_CAST(nsIClassInfo*, this); 
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kISupportsIID)) {
    *aInstancePtr = NS_STATIC_CAST(nsISupports*,(NS_STATIC_CAST(nsIWinamp*, this))); 
    AddRef();
    return NS_OK;
  }

  return NS_NOINTERFACE; 
}

//********************************************************************
void nsWinamp::SetInstance(nsPluginInstance* plugin)
{
  //REVIEW: Could we leak here? Probably not, but worth checking.
  if (plugin != NULL)
    mPlugin = plugin;
}

#if 0
//********************************************************************
void nsWinamp::SetWindow(NPWindow* aWindow)
{
#ifdef _WIN32
  if (aWindow)
  {
    mWindow.setHwnd((HWND) aWindow->window);
  }
#endif
  if (mUnagi)
  {
    mUnagi->setWindow(&mWindow);
  }
}
#endif 


//********************************************************************
//********************************************************************
//********************************************************************
/*
UNAGI METHODS
*/


//********************************************************************
NS_IMETHODIMP nsWinamp::GetVersion(char **_retval)
{
	nsresult hr = NS_OK;
	char csVersion[BUFFER_LEN];
	memset(&csVersion[0], '\0', BUFFER_LEN);

	DWORD BufferSize = BUFFER_LEN;
    DWORD cbData;
	bool keyFound = false;

	wchar_t exeName[] = L"\\winamp.exe";
    wchar_t fileName[BUFFER_LEN]; 
	memset(&fileName[0],'\0',BUFFER_LEN);
    wchar_t fileNameTemp[BUFFER_LEN]; 

	HKEY hKey;
	cbData = BUFFER_LEN;

	// first check the protocol handler registry key, we're looking for
	// the winamp:// protocol handler. If we find this, then this is the
	// "right" exe for winamp we need to get the version number on
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("winamp\\shell\\open\\command"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileNameTemp,
								 &cbData ) != ERROR_SUCCESS) {
			return NS_ERROR_FILE_UNRECOGNIZED_PATH;
		}

		RegCloseKey (hKey);
		if (wcsstr(fileNameTemp,L"winamp.exe")) {
			int indexOfFirstQuote = wcscspn(fileNameTemp, L"\"");
			int indexOfSecondQuote = wcscspn(&fileNameTemp[indexOfFirstQuote+1], L"\"");
			if (indexOfFirstQuote >= 0) {
				keyFound = true;
				wcsncpy(fileName,&fileNameTemp[indexOfFirstQuote+1], indexOfSecondQuote);
			} 
		} else {
			// some other app (itunes ??) controlling the winamp:// protocol
			// return error
			return NS_ERROR_FILE_UNRECOGNIZED_PATH;
		}
	}

	if (!keyFound) {
		// See if the reg key exists
		if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Winamp"), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
			return NS_ERROR_FILE_UNRECOGNIZED_PATH;
		}

		cbData = BUFFER_LEN;
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileName,
								 &cbData ) != ERROR_SUCCESS) {
			return NS_ERROR_FILE_UNRECOGNIZED_PATH;
		}
		RegCloseKey (hKey);
		keyFound = true;
		wcscat(fileName,exeName);
	}

	if (!keyFound) {
		return NS_ERROR_FILE_UNRECOGNIZED_PATH;
	}

	static TCHAR sBackSlash[] = {'\\','\0'};
	DWORD dwVersionDataLen = GetFileVersionInfoSize(fileName, NULL);

	if (dwVersionDataLen) {
		char* fvBuf = (char *)alloca(dwVersionDataLen);
		if (GetFileVersionInfo(fileName, 0, dwVersionDataLen, fvBuf)) {
			
			LPVOID pVal;
			UINT nValLen;
			if (VerQueryValue(fvBuf, sBackSlash, &pVal, &nValLen)) {
				if (nValLen == sizeof(VS_FIXEDFILEINFO)) {
					VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)pVal;
					//sprintf(csVersion, "%d.%d.%d.%d",
						//HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
						//HIWORD(pFixedFileInfo->dwFileVersionLS), LOWORD(pFixedFileInfo->dwFileVersionLS));
					sprintf(csVersion, "%d.%d%d",
						HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
						HIWORD(pFixedFileInfo->dwFileVersionLS));
				}
			}
		}
	}

	//Copy version number to return value.
	char*& targetResult = *_retval;
	targetResult = static_cast<char*>(NPN_MemAlloc((uint32)strlen(&csVersion[0]) + 1));

	//REVIEW: use strncpy instead of strcpy (in general too)
	if (targetResult != NULL)
		strcpy(targetResult, &csVersion[0]);
	else
		hr = NS_ERROR_UNEXPECTED;

	return hr;
}

#if defined(XP_MACOSX) || defined(linux)
// strncasecmp first appeared in 4.4BSD.
#undef strnicmp
#undef stricmp
#endif