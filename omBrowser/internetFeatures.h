#ifndef NULLSOFT_WINAMP_INTERNET_FEATURES_HELPER_HEADER
#define NULLSOFT_WINAMP_INTERNET_FEATURES_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <urlmon.h>


#ifndef  FEATURE_TABBED_BROWSING
  #define FEATURE_TABBED_BROWSING ((INTERNETFEATURELIST)19)
#endif //FEATURE_TABBED_BROWSING

#ifndef FEATURE_SSLUX
  #define FEATURE_SSLUX ((INTERNETFEATURELIST)20)
#endif // FEATURE_SSLUX

#ifndef FEATURE_DISABLE_NAVIGATION_SOUNDS
  #define FEATURE_DISABLE_NAVIGATION_SOUNDS ((INTERNETFEATURELIST)21)
#endif // FEATURE_DISABLE_NAVIGATION_SOUNDS

#ifndef FEATURE_DISABLE_LEGACY_COMPRESSION
  #define FEATURE_DISABLE_LEGACY_COMPRESSION ((INTERNETFEATURELIST)22)
#endif // FEATURE_DISABLE_LEGACY_COMPRESSION

#ifndef  FEATURE_FORCE_ADDR_AND_STATUS
  #define FEATURE_FORCE_ADDR_AND_STATUS ((INTERNETFEATURELIST)23)
#endif //FEATURE_FORCE_ADDR_AND_STATUS

#ifndef  FEATURE_BLOCK_INPUT_PROMPTS
  #define FEATURE_BLOCK_INPUT_PROMPTS ((INTERNETFEATURELIST)27)
#endif //FEATURE_BLOCK_INPUT_PROMPTS

class InternetFeatures
{
public:
	InternetFeatures();
	~InternetFeatures();


public:
	HRESULT SetEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags, BOOL fEnable);
	HRESULT IsEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags);

	HRESULT SetDWORDFeature(const wchar_t *featureName, BOOL perUser, unsigned long value);
	HRESULT GetDWORDFeature(const wchar_t *featureName, BOOL perUser, unsigned long *value);
	void DeleteFeature(const wchar_t *featureName, BOOL perUser);

protected:
	HRESULT LoadModule();
	const wchar_t *GetProcessName();

private:
	typedef HRESULT (WINAPI *COINTERNETSETFEATUREENABLED)(INTERNETFEATURELIST /*FeatureEntry*/, DWORD /*dwFlags*/, BOOL /*fEnable*/);
	typedef HRESULT (WINAPI *COINTERNETISFEATUREENABLED)(INTERNETFEATURELIST /*FeatureEntry*/, DWORD /*dwFlags*/);
	
private:
	HMODULE module;
	HRESULT loadResult;
	COINTERNETSETFEATUREENABLED CoInternetSetFeatureEnabled;
	COINTERNETISFEATUREENABLED CoInternetIsFeatureEnabled;
	wchar_t *processName_;
};

#endif NULLSOFT_WINAMP_INTERNET_FEATURES_HELPER_HEADER