#include "Application.h"
#include "nx/nxstring.h"
#include "foundation/error.h"
#include "nu/strsafe.h"
#include <android/log.h>

extern JavaVM *g_jvm;
Application::Application()
{
	application_name = 0;
	android_release=0;
	product_short_name=0;
	
	application_version=0;
	sdk_version=0;
	uname(&linux_info);
}

int Application::Init()
{
	int ret = ApplicationBase::Initialize();
	if (ret != NErr_Success)
		return ret;

	/* WAFA/1.01 (Linux; Android 2.2.1; Winamp) Replicant/1.0 */
	snprintf(user_agent, 128, "%s/%s (%s; Android %s; Winamp) Replicant/1.0", product_short_name->string, application_version->string, linux_info.sysname, android_release->string);
	return NErr_Success;
}

const char *Application::Application_GetUserAgent()
{
	return user_agent;	
}

void Application::SetSDK(int version, nx_string_t release)
{
	sdk_version=version;
	android_release = NXStringRetain(release);
}

void Application::SetApplication(nx_string_t appname, nx_string_t short_name, nx_string_t version)
{
	application_name = NXStringRetain(appname);
	product_short_name = NXStringRetain(short_name);
	application_version = NXStringRetain(version);
}

int Application::Application_GetProductShortName(nx_string_t *name)
{
	*name = NXStringRetain(product_short_name);
	if (product_short_name)
		return NErr_Success;
	else
		return NErr_Empty;
}

int Application::GetRelease(nx_string_t *release)
{
		*release = NXStringRetain(android_release);
	if (android_release)
		return NErr_Success;
	else
		return NErr_Empty;
}

int Application::Application_GetVersionString(nx_string_t *version)
{
			*version = NXStringRetain(application_version);
	if (android_release)
		return NErr_Success;
	else
		return NErr_Empty;
}

/* Android API implementation */
AndroidAPI::AndroidAPI()
{
}

int AndroidAPI::Android_GetSDKVersion()
{
	return application.GetSDKVersion();
}

JavaVM *AndroidAPI::Android_GetJVM()
{
	return g_jvm;
}


int AndroidAPI::Android_GetRelease(nx_string_t *release)
{
	return application.GetRelease(release);
}

