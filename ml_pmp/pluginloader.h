#ifndef __PLUGINLOADER_H_
#define __PLUGINLOADER_H_

#include <windows.h>
#include "../gen_ml/ml.h"
#include "pmp.h"
#include "../gen_ml/itemlist.h"

BOOL loadDevPlugins();
void unloadDevPlugins();
int wmDeviceChange(WPARAM wParam, LPARAM lParam);

#endif