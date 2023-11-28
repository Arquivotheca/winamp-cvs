#pragma once

#if defined(_WIN32)
#include "win/XMLParser.h"
#elif defined (__ANDROID__)
#include "android/XMLParser.h"
#endif