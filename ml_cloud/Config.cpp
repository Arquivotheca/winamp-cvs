#include "main.h"
#include "../nu/ns_wc.h"

const wchar_t *ConfigurationKeys::provider[4] = { L"provider", L"dev_provider", L"qa_provider", L"stage_provider" };
const wchar_t *ConfigurationKeys::auth_token[4] = { L"auth_token", L"dev_auth_token", L"qa_auth_token", L"stage_auth_token" };
const wchar_t *ConfigurationKeys::username[4] = { L"username", L"dev_username", L"qa_username", L"stage_username" };
const wchar_t *ConfigurationKeys::friendly[4] = { L"friendly", L"dev_friendly", L"qa_friendly", L"stage_friendly" };
const wchar_t *ConfigurationKeys::web_url = L"web_url";
const wchar_t *ConfigurationKeys::cloud_api_url = L"cloud_url";
const wchar_t *ConfigurationKeys::cloud_last_tab = L"last_tab";
const wchar_t *ConfigurationKeys::show_local = L"show_local";
const wchar_t *ConfigurationKeys::cloud_last_scan = L"last_scan";
const wchar_t *ConfigurationKeys::cloud_dev_last_scan = L"dev_last_scan";
const wchar_t *ConfigurationKeys::nuke[4] = { L"nuke", L"dev_nuke", L"qa_nuke", L"stage_nuke" };
const wchar_t *ConfigurationKeys::nuke_ignored[4] = { L"nuke_ignored", L"dev_nuke_ignored", L"qa_nuke_ignored", L"stage_nuke_ignored" };
const wchar_t *ConfigurationKeys::first_run[4] = { L"first_run", L"dev_first_run", L"qa_first_run", L"stage_first_run" };
const wchar_t *ConfigurationKeys::first_login[4] = { L"first_login", L"dev_first_login", L"qa_first_login", L"stage_first_login" };
const wchar_t *ConfigurationKeys::show_status = L"show_status";
const wchar_t *ConfigurationKeys::x_status = L"x_status";
const wchar_t *ConfigurationKeys::y_status = L"y_status";
const wchar_t *ConfigurationKeys::dev_mode = L"dev_mode";
const wchar_t *ConfigurationKeys::pl_remove = L"pl_remove";

const wchar_t *Config_GetDefaultAPIUrl()
{
	switch(Config_GetDevMode())
	{
	default:
	case 0: return L"https://cloud.winamp.com/api/1/";
	case 1: return L"https://devcloud.winamp.com/api/1/";
	case 2: return L"https://qacloud.winamp.com/api/1/";
	case 3: return L"https://stagecloud.winamp.com/api/1/";
	}
	
}

const wchar_t *Config_GetDefaultWebUrl()
{
	switch(Config_GetDevMode())
	{
	default:
	case 0: return L"https://cloud.winamp.com/";
	case 1: return L"https://devcloud.winamp.com/";
	case 2: return L"https://qacloud.winamp.com/";
	case 3: return L"https://stagecloud.winamp.com/";
	}
}

void Config_SetDevMode(int mode)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", mode);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::dev_mode, temp, ini_file);
}

int Config_GetDevMode()
{
	static int read, dev_mode;
	if (!read)
	{
		dev_mode = GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::dev_mode, 0, ini_file);
		if (dev_mode > 3)
			dev_mode = 3;
		if (dev_mode < 0)
			dev_mode = 0;
		read = 1;
	}
	
	return dev_mode;
}

void Config_GetProvider(wchar_t *provider, size_t provider_cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME,
								 ConfigurationKeys::provider[Config_GetDevMode()],
								 L"", provider, provider_cch, ini_file))
	{
		// just in-case
		wcsncpy(provider, L"", provider_cch);
	}
}

void Config_SetProvider(const wchar_t *provider)
{
	WritePrivateProfileString(INI_APP_NAME,
							  ConfigurationKeys::provider[Config_GetDevMode()],
							  provider, ini_file);
}

void Config_GetAuthToken(wchar_t *auth_token, size_t auth_token_cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME,
								 ConfigurationKeys::auth_token[Config_GetDevMode()],
								 L"", auth_token, auth_token_cch, ini_file))
	{
		// just in-case
		wcsncpy(auth_token, L"", auth_token_cch);
	}
}

void Config_SetAuthToken(const wchar_t *auth_token)
{
	WritePrivateProfileString(INI_APP_NAME,
							  ConfigurationKeys::auth_token[Config_GetDevMode()],
							  auth_token, ini_file);
}

void Config_GetUsername(wchar_t *username, size_t username_cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME,
								 ConfigurationKeys::username[Config_GetDevMode()],
								 L"", username, username_cch, ini_file))
	{
		// just in-case
		wcsncpy(username, L"", username_cch);
	}
}

void Config_SetUsername(const wchar_t *username)
{
	WritePrivateProfileString(INI_APP_NAME,
							  ConfigurationKeys::username[Config_GetDevMode()],
							  username, ini_file);
}

void Config_GetFriendlyUsername(wchar_t *username, size_t username_cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME,
								 ConfigurationKeys::friendly[Config_GetDevMode()],
								 L"", username, username_cch, ini_file))
	{
		// just in-case
		wcsncpy(username, L"", username_cch);
	}
}

void Config_SetFriendlyUsername(const wchar_t *username)
{
	WritePrivateProfileString(INI_APP_NAME,
							  ConfigurationKeys::friendly[Config_GetDevMode()],
							  username, ini_file);
}

void Config_SetShowLocal(int show)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", show);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::show_local, temp, ini_file);
}

int Config_GetShowLocal()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::show_local, 0, ini_file);
}

void Config_SetFirstRun(int first)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", first);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::first_run[Config_GetDevMode()], temp, ini_file);
}

int Config_GetFirstRun()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::first_run[Config_GetDevMode()], 0, ini_file);
}

void Config_SetFirstLogin()
{
	wchar_t temp[64] = {L"1"};
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::first_login[Config_GetDevMode()], temp, ini_file);
}

int Config_GetFirstLogin()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::first_login[Config_GetDevMode()], 0, ini_file);
}

void Config_SetPlRemoveMode(int mode)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", mode);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::pl_remove, temp, ini_file);
}

int Config_GetPlRemoveMode()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::pl_remove, 0, ini_file);
}

void Config_SetNukeVersion(int version)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", version);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::nuke[Config_GetDevMode()], temp, ini_file);
}

int Config_GetNukeVersion()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::nuke[Config_GetDevMode()], 0, ini_file);
}

void Config_SetNukeIgnoredVersion(int version)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", version);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::nuke_ignored[Config_GetDevMode()], temp, ini_file);
}

int Config_GetNukeIgnoredVersion()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::nuke_ignored[Config_GetDevMode()], 0, ini_file);
}

void Config_SetShowStatus(int show_status)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", show_status);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::show_status, temp, ini_file);
}

int Config_GetShowStatus()
{
	return GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::show_status, 2, ini_file);
}

void Config_SaveShowPos(LONG left, LONG top)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", left);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::x_status, temp, ini_file);
	swprintf(temp, 64, L"%d", top);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::y_status, temp, ini_file);
}

void Config_GetShowPos(LONG *left, LONG *top)
{
	*left = GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::x_status, -1, ini_file);
	*top = GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::y_status, -1, ini_file);
}

void Config_GetWebURL(wchar_t *url, size_t cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME, ConfigurationKeys::web_url, Config_GetDefaultWebUrl(), url, cch, ini_file))
	{
		// just in-case
		wcsncpy(url, Config_GetDefaultWebUrl(), cch);
	}
}

void Config_SetWebURL(const wchar_t *url)
{
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::web_url, url, ini_file);
}

void Config_GetCloudAPIURL(wchar_t *url, size_t cch)
{
	if (!GetPrivateProfileString(INI_APP_NAME, ConfigurationKeys::cloud_api_url, Config_GetDefaultAPIUrl(), url, cch, ini_file))
	{
		// just in-case
		wcsncpy(url, Config_GetDefaultAPIUrl(), cch);
	}
}

void Config_SetCloudAPIURL(const wchar_t *url)
{
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::cloud_api_url, url, ini_file);
}

int Config_GetLastTab()
{
	return (debug_tab_sel = GetPrivateProfileInt(INI_APP_NAME, ConfigurationKeys::cloud_last_tab, 0, ini_file));
}

void Config_SetLastTab()
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", debug_tab_sel);
	WritePrivateProfileString(INI_APP_NAME, ConfigurationKeys::cloud_last_tab, temp, ini_file);
}

time_t Config_GetLastScan()
{
	return GetPrivateProfileInt(INI_APP_NAME, (!Config_GetDevMode() ? ConfigurationKeys::cloud_last_scan : ConfigurationKeys::cloud_dev_last_scan), 0, ini_file);
}

void Config_SetLastScan(time_t scan_time)
{
	wchar_t temp[64];
	swprintf(temp, 64, L"%d", scan_time);
	WritePrivateProfileString(INI_APP_NAME, (!Config_GetDevMode() ? ConfigurationKeys::cloud_last_scan : ConfigurationKeys::cloud_dev_last_scan), temp, ini_file);
}

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