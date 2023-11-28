#include "main.h"
#include "nx/nxstring.h"

int logMode = 3;
nx_string_t cloud_url_override=0;

bool Config_GetLogging()
{
	return !!(logMode & 1);
}

bool Config_GetSlimLogging()
{
	return !!(logMode & 2);
}

bool Config_GetLogBinary()
{
	return !!(logMode & 4);
}

void Config_SetCloudURL(const char *new_url)
{
	NXStringRelease(cloud_url_override);
	NXStringCreateWithUTF8(&cloud_url_override, new_url);
}