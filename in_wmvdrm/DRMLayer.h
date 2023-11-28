#ifndef NULLSOFT_DRMLAYERH
#define NULLSOFT_DRMLAYERH

#include "WMHandler.h"
#include <wmsdk.h>
#include "../nu/AutoLock.h"
#include <wininet.h>
class DRMLayer : public WMHandler
{
public:
	DRMLayer(IWMReader *reader);
	~DRMLayer();

private:
	// WMHandler
	void LicenseAcquired();
	void AcquireLicense(WM_GET_LICENSE_DATA *&licenseData);
	void NoRights(wchar_t *licenseURL);
	void NoRightsEx(WM_GET_LICENSE_DATA *&licenseData);
	void SignatureState(WMT_DRMLA_TRUST *&state);
	void Individualize();
	void Opened();
	void LicenseRequired();
	void IndividualizeStatus(WM_INDIVIDUALIZE_STATUS *status);
	void NeedsIndividualization();
	void Stopping();
	void BrowserClosed();
	void MonitorCancelled();
	void SilentCancelled();
	void Closed();
	void DRMExpired();
	// helpers
	void DoSilentLicense();
	void DoLicenseDone();
	void DoUpdate();
	void DoUpdateDone();
	void DoStopping();
	void NonSilent7(WM_GET_LICENSE_DATA *licenseData);
	bool agreedLicense;
	// our data
	IWMDRMReader2 *outputProtection;
	IWMDRMReader *drm;
  wchar_t url[INTERNET_MAX_URL_LENGTH];
	enum
	{
		STATUS_OFF,
		STATUS_ON,
		STATUS_CANCELLED
	} monitoring, silentLicenseStarted, updateStatus;
	bool  updating;
	HANDLE licenseEvent, updateEvent;
	
	Nullsoft::Utility::LockGuard licenseGuard, updateGuard;
};
#endif
