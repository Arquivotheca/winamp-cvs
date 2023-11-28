#include "Main.h"
#include "DRMLayer.h"
#include <uuids.h>
#include "util.h"
#include <strsafe.h>
#include "NonSilentDRMWindow.h"
#include "resource.h"
using namespace Nullsoft::Utility;

DRMLayer::DRMLayer(IWMReader *reader)
: drm(0), outputProtection(0),
silentLicenseStarted(STATUS_OFF),
updating(false),
monitoring(STATUS_OFF),
updateStatus(STATUS_OFF),
agreedLicense(false),
licenseGuard(GUARDNAME("DRMLayer::licenseGuard")),
updateGuard(GUARDNAME("DRMLayer::updateGuard"))
{
  url[0]=0;
	updateEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	licenseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (FAILED(reader->QueryInterface(&drm)))
		drm=0;
 
	if (FAILED(reader->QueryInterface(&outputProtection)))
		outputProtection=0;

	if (outputProtection)
		outputProtection->SetEvaluateOutputLevelLicenses(TRUE);

}

DRMLayer::~DRMLayer()
{
	if (drm)
		drm->Release();
	drm = 0;

	if (outputProtection)
		outputProtection->Release();
	outputProtection = 0;
}
bool WebsiteLaunchAllowed()
{
	wchar_t title[64];
	return  MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_PROTECTED_CONTENT_NEED_LICENSE),
					   WASABI_API_LNGSTRINGW_BUF(IDS_LICENSE_REQUIRED,title,64), MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONQUESTION) == IDYES;
}

void DRMLayer::NonSilent7(WM_GET_LICENSE_DATA *licenseData)
{
	licenseGuard.Lock();
	if (silentLicenseStarted == STATUS_CANCELLED)
	{
		// license cancelled, bailing out
		DoLicenseDone();
		licenseGuard.Unlock();
	}
	else if (silentLicenseStarted == STATUS_ON
		&& WebsiteLaunchAllowed())
	{
		// launching website to acquire license
		DoLicenseDone();
		monitoring = STATUS_ON;
		LaunchNonSilentDRMWindow(licenseData->wszURL, licenseData->pbPostData, licenseData->dwPostDataSize, this);
		drm->MonitorLicenseAcquisition();
		licenseGuard.Unlock();
	}
	else
	{
		// license failed or user said no
		DoLicenseDone();
		licenseGuard.Unlock();
		First().OpenFailed();		
	}

}
void DRMLayer::IndividualizeStatus(WM_INDIVIDUALIZE_STATUS *status)
{
	{
		AutoLock lock(updateGuard LOCKNAME("DRMLayer::IndividualizeStatus"));
		//		updating=true;
		if (updating)
			ResetEvent(updateEvent);
	}
	if (INDI_BEGIN & status->enIndiStatus)
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_BEGINNING_UPDATE));
	else if (INDI_SUCCEED & status->enIndiStatus)
	{
		DoUpdateDone();
		First().ReOpen();
	}
	else if (INDI_FAIL & status->enIndiStatus)
	{
		DoUpdateDone();
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_UPDATE_FAILED));
		First().OpenFailed();
	}
	else if (INDI_CANCEL & status->enIndiStatus)
	{
		DoUpdateDone();
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_UPDATE_CANCELLED));
	}

	if (INDI_DOWNLOAD & status->enIndiStatus)
	{
		switch (status->enHTTPStatus)
		{
		case HTTP_NOTINITIATED:
			winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_IDLE));
			break;
		case HTTP_CONNECTING:
			winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_CONNECTING));
			break;
		case HTTP_REQUESTING:
			winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_REQUESTING));
			break;
		case HTTP_RECEIVING:
			{
				if (status->dwHTTPReadTotal)
				{
					int x = (status->dwHTTPReadProgress * 100) / status->dwHTTPReadTotal;
					winamp.Buffering(x, WASABI_API_LNGSTRINGW(IDS_RECEIVING_DATA));
				}
				else
					winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_RECEIVING_DATA));
			}
			break;
		case HTTP_COMPLETED:
			winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_COMPLETED));
			break;
		}
	}

	if (INDI_INSTALL & status->enIndiStatus)
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_INSTALLING));
}

void DRMLayer::MonitorCancelled()
{

	if (monitoring == STATUS_CANCELLED)
	{
		monitoring=STATUS_OFF;
		DoLicenseDone();
		First().OpenFailed();
	}
	else
		monitoring=STATUS_OFF;
}

void DRMLayer::SilentCancelled()
{
	licenseGuard.Lock();
	if (silentLicenseStarted == STATUS_CANCELLED)
	{
		DoLicenseDone();
		licenseGuard.Unlock();
		First().OpenFailed();
	}
	else
	{
		DoLicenseDone();
		licenseGuard.Unlock();
	}
}
void DRMLayer::LicenseAcquired()
{
	licenseGuard.Lock();
	if (silentLicenseStarted == STATUS_CANCELLED)
	{
		DoLicenseDone();
		licenseGuard.Unlock();
	}
	else
	{
		DoLicenseDone();
		licenseGuard.Unlock();
		First().ReOpen();
	}
}

void DRMLayer::AcquireLicense(WM_GET_LICENSE_DATA *&licenseData)
{
	if (licenseData->dwSize == 0)
	{
		if (monitoring == STATUS_ON)
		{
			// monitoring was on, so we must have succeeded, re-open
			monitoring = STATUS_OFF;
			First().ReOpen();
			return ;
		}
		else if (monitoring == STATUS_CANCELLED)
		{
			// monitoring cancelled, so we must have failed
			AutoLock lock (licenseGuard LOCKNAME("DRMLayer::AcquireLicense"));
			if (silentLicenseStarted == STATUS_CANCELLED)
			{
				//license cancelled, so we must have failed
				DoLicenseDone();
				return;
			}
			First().OpenFailed();
			return;
		}
		{
			AutoLock lock (licenseGuard LOCKNAME("DRMLayer::AcquireLicense"));
			if (silentLicenseStarted == STATUS_CANCELLED)
			{
				//license cancelled, so we must have failed
				DoLicenseDone();
				return;
			}
			if (silentLicenseStarted == STATUS_ON
				&& WebsiteLaunchAllowed())
			{
				//monitoring = true;
				LaunchNonSilentDRMWindow(url, 0, 0, this);
				//drm->MonitorLicenseAcquisition(); // TODO: monitor this somehow
			}
		}
		DoLicenseDone();
		First().OpenFailed();
		return ;
	}

	
	switch (licenseData->hr)
	{
	case NS_E_DRM_LICENSE_STORE_ERROR:
	case NS_E_DRM_LICENSE_NOTACQUIRED:
		NonSilent7(licenseData);
		break;

	case NS_E_DRM_LICENSE_APPSECLOW:
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_FILE_PROTECTED_CANNOT_PLAY_IN_WINAMP));
	case NS_S_DRM_MONITOR_CANCELLED:
		MonitorCancelled(); 
		break;
	default:
		if (SUCCEEDED(licenseData->hr))
	{
		winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_LICENSE_ACQUIRED));
		DoLicenseDone();
		First().ReOpen();
		return ;
	}
		else
		{
		DoLicenseDone();
		First().OpenFailed();
		}
		break;
	}

}
void DRMLayer::NoRightsEx(WM_GET_LICENSE_DATA *&licenseData)
{
	switch (licenseData->hr)
	{
	WMTCASE(NS_E_LICENSE_OUTOFDATE)
	WMTCASE(NS_E_LICENSE_INCORRECT_RIGHTS)
	WMTCASE(NS_E_LICENSE_REQUIRED)
		NonSilent7(licenseData);
		break;
	default:
		DoSilentLicense();
		break;
	}

}

void DRMLayer::SignatureState(WMT_DRMLA_TRUST *&state)
{
	switch (*state)
	{
	case WMT_DRMLA_UNTRUSTED:
		{
		wchar_t title[64];
		if (MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_PROTECTED_CONTENT_UNTRUSTED_SOURCE),
			WASABI_API_LNGSTRINGW_BUF(IDS_UNTRUSTED_LICENSE,title,64), MB_YESNO | MB_SETFOREGROUND | MB_DEFBUTTON2 | MB_TOPMOST | MB_ICONQUESTION) == IDYES)
			agreedLicense=true;
		else
			agreedLicense=false;
		}
		break;
	case WMT_DRMLA_TRUSTED:
		agreedLicense=true;
		break;
	case WMT_DRMLA_TAMPERED:
		{
		wchar_t title[64];
		if (MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_PROTECTED_CONTENT_TAMPERED_LICENSE),
			WASABI_API_LNGSTRINGW_BUF(IDS_TAMPERED_LICENSE,title,64), MB_YESNO | MB_SETFOREGROUND | MB_DEFBUTTON2 | MB_TOPMOST | MB_ICONQUESTION) == IDYES)
			agreedLicense=true;
		else
			agreedLicense=false;
		}
		break;
	}

}
void DRMLayer::Individualize()
{
	wchar_t temp[256];
	wchar_t title[256];
	if (MessageBox(NULL, WASABI_API_LNGSTRINGW_BUF(IDS_WINDOWS_MEDIA_PROTECTED_CONTENT_SECURITY_UPDATE, temp, 256),
		WASABI_API_LNGSTRINGW_BUF(IDS_SECURITY_UPDATE_REQUIRED,title,256), MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONQUESTION) == IDYES)
	{
		{
			AutoLock lock (updateGuard LOCKNAME("DRMLayer::Individualize"));
			winamp.SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_UPDATING,temp,256));
			updating = true;
		}
		drm->Individualize(0);
	}
}
#ifdef _DEBUG
#include <iostream>
#endif
void DRMLayer::Opened()
{
	DoLicenseDone();
	DoUpdateDone();
	monitoring = STATUS_OFF;
	if (outputProtection)
	{
		DRM_PLAY_OPL *opl=0;
		DWORD oplSize = 0;
		DWORD level = -1;
		HRESULT hr = outputProtection->GetPlayOutputLevels(NULL, &oplSize, &level);
		if (hr != NS_E_DRM_UNSUPPORTED_PROPERTY)
		{
			if (!oplSize)
				oplSize = sizeof(DRM_PLAY_OPL);
			opl = (DRM_PLAY_OPL *)new unsigned char[oplSize];
			hr = outputProtection->GetPlayOutputLevels(opl, &oplSize, &level);
#ifdef _DEBUG
			std::cerr << "hr = " << std::hex << hr << std::dec << std::endl;
			std::cerr << "oplSize = " << oplSize << " and sizeof(opl) = " << sizeof(opl) << std::endl;
			std::cerr << "output protection level is: " << level << std::endl;
			std::cerr << "audio, compressed: " << opl->minOPL.wCompressedDigitalAudio << " video, compressed:  " << opl->minOPL.wCompressedDigitalAudio << std::endl;
			std::cerr << "audio, uncompressed: " <<opl->minOPL.wUncompressedDigitalAudio << " video, uncompressed: " << opl->minOPL.wUncompressedDigitalAudio << std::endl;
#endif
			if (opl->minOPL.wUncompressedDigitalAudio > 100 || opl->minOPL.wUncompressedDigitalVideo > 100)
			{
				wchar_t title[128];
				MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_ENCRYPTED_IN_OP_LEVEL_HIGHER_THAN_SUPPORTED),
						   WASABI_API_LNGSTRINGW_BUF(IDS_WINDOWS_MEDIA_PLAYBACK_FAILURE,title,128), MB_OK);
				First().OpenFailed();
				return;
			}
			
		}
	}

	WMHandler::Opened();
}

void DRMLayer::LicenseRequired()
{
	//winamp.SetStatus(L"License Required");
}

void DRMLayer::NoRights(wchar_t *licenseURL)
{
  lstrcpyn(url, licenseURL, INTERNET_MAX_URL_LENGTH);
	DoSilentLicense();
}

void DRMLayer::NeedsIndividualization()
{
	if (!updating)
	{
		wchar_t temp[1024];
		winamp.SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_UPDATE_REQUIRED, temp, 1024));
		First().OpenFailed();
	}
}

void DRMLayer::DoStopping()
{
	{
		AutoLock lock (updateGuard LOCKNAME("DRMLayer::DoStopping"));

		if (updating)
		{
			drm->CancelIndividualization();
			updating=false;
		}
	}

	WaitForSingleObject(updateEvent, INFINITE);
	{
		AutoLock lock (licenseGuard LOCKNAME("DRMLayer::DoStopping"));
		if (silentLicenseStarted == STATUS_ON)
		{
			silentLicenseStarted = STATUS_CANCELLED;
			drm->CancelLicenseAcquisition();
		}
		else
			silentLicenseStarted = STATUS_CANCELLED;
	}

	WaitForSingleObject(licenseEvent, INFINITE);

	if (monitoring == STATUS_ON)
	{
		monitoring = STATUS_CANCELLED;
		drm->CancelMonitorLicenseAcquisition();
	}
	else
		monitoring = STATUS_CANCELLED;

}

void DRMLayer::Stopping()
{
	//KillWindow();
	DoStopping();
	DoLicenseDone();
	DoUpdateDone();
	monitoring = STATUS_OFF;
	WMHandler::Stopping();
}

void DRMLayer::DoSilentLicense()
{ 
	AutoLock lock (licenseGuard);
	if (silentLicenseStarted == STATUS_CANCELLED)
		return;
	ResetEvent(licenseEvent);
	winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_ACQUIRING_LICENSE));
	silentLicenseStarted = STATUS_ON;
	drm->AcquireLicense(1);
}

void DRMLayer::DoUpdateDone()
{
	AutoLock lock (updateGuard);
	updating = false;
	SetEvent(updateEvent);
}

void DRMLayer::DoLicenseDone()
{
	AutoLock lock (licenseGuard);
	SetEvent(licenseEvent);
	silentLicenseStarted = STATUS_OFF;
}

void DRMLayer::BrowserClosed()
{
	DoStopping();
#if 0
	{
		AutoLock lock (updateGuard LOCKNAME("DRMLayer::DoStopping"));

		if (updating)
		{
			drm->CancelIndividualization();
			updating=false;
		}
	}

	{
		AutoLock lock (licenseGuard LOCKNAME("DRMLayer::DoStopping"));
		if (silentLicenseStarted == STATUS_ON)
		{
			silentLicenseStarted = STATUS_CANCELLED;
			drm->CancelLicenseAcquisition();
		}
		else
			silentLicenseStarted = STATUS_CANCELLED;
	}

	if (monitoring == STATUS_ON)
	{
		monitoring = STATUS_CANCELLED;
		drm->CancelMonitorLicenseAcquisition();
	}
	else
		monitoring = STATUS_CANCELLED;
#endif
}

void DRMLayer::Closed()
{
	DoLicenseDone();
	DoUpdateDone();
	monitoring = STATUS_OFF;
	WMHandler::Closed();
}

void DRMLayer::DRMExpired()
{
	wchar_t title[32];
	MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_DRM_LICENSE_EXPIRED_VISIT_WINAMP_COM),
			   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title,32), MB_OK);
}