#pragma once
#include <../gen_ml/ml.h>
#include "../ml_pmp/pmp.h"
#include <yajl/yajl_parse.h>
class ItemParser
{
public:
	~ItemParser();
	yajl_handle parser;
	itemRecordListW record_list;
};

ItemParser *Parse();

int PostJSON(const char *url, const char *json_data, yajl_handle parser);
int PostAlbumArt(const char *url, const wchar_t *filename, const char *json_data, yajl_handle parser);
int PostFile(const char *url, const wchar_t *filename, const char *json_data, yajl_handle parser);
extern PMPDevicePlugin plugin;
extern int winampVersion;