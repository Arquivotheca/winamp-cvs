#pragma once
#include <application/api_application.h>
#include <nx/nxstring.h>
#include <nswasabi/ApplicationBase.h>
#include <nswasabi/ServiceName.h>
class Application : public ApplicationBase
{
public:
	enum
	{
		INIT_ERROR_STRING_HEAP=1,
	};
	WASABI_SERVICE_NAME("Replicant/Winamp5 Application API");

	Application();
	~Application();
	int Init();

	/* api_application implementation */
	//HANDLE WASABICALL Application_GetStringHeap();
	const char *WASABICALL Application_GetUserAgent();
	unsigned int WASABICALL Application_GetBuildNumber();
	int WASABICALL Application_GetVersionString(nx_string_t *version);
	int WASABICALL Application_GetProductShortName(nx_string_t *name);
	
private:
	//HANDLE string_heap;
	char user_agent[256];
	nx_string_t version_string;
	unsigned int build_number;
};

extern Application application;