#pragma once
#include "nswasabi/ApplicationBase.h"
#include "application/api_application.h"
#include "application/api_android.h"
#include "nx/nxstring.h"
#include <sys/utsname.h>
#include "nswasabi/ServiceName.h"

class AndroidAPI : public api_android
{
public:
	WASABI_SERVICE_NAME("Android API");
	AndroidAPI();
	

	/* api_android implementation */
	int WASABICALL Android_GetSDKVersion();
	JavaVM *WASABICALL Android_GetJVM();
	int WASABICALL Android_GetRelease(nx_string_t *release);
};

extern AndroidAPI android_api;

class Application : public ApplicationBase
{
public:
	WASABI_SERVICE_NAME("Winamp for Android Application API");
	Application();
	
	
	/* api_application implementation */
	const char *WASABICALL Application_GetUserAgent();
	int WASABICALL Application_GetProductShortName(nx_string_t *name);
	int WASABICALL Application_GetVersionString(nx_string_t *version);

	/* code called by JNI layer */
	void SetSDK(int version, nx_string_t release);
	void SetApplication(nx_string_t appname, nx_string_t short_name, nx_string_t version);
	int Init();
	int GetSDKVersion() { return sdk_version; }
	int GetRelease(nx_string_t *release);


private:
	int sdk_version;
	struct utsname linux_info;
	char user_agent[128];
	nx_string_t android_release;
	nx_string_t application_name;
	nx_string_t product_short_name;
	
	nx_string_t application_version;
	nx_string_t plugin_directory;
};

extern Application application;