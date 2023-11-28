#pragma once

enum
{
	CONFIG_MODE_ALL = 0,
	CONFIG_MODE_MEDIA_LIBRARY = 1,
	CONFIG_MODE_DIRECTORIES = 2,
};

extern int config_allow_mode;
extern int config_video;
extern int config_collect;
extern int config_avtrack;
extern int config_awaken_on_load;
extern int config_listening_length; // as a percentage
extern int config_log;
extern int config_first;
extern wchar_t config_directories[];
extern wchar_t config_username[512];
