#pragma once
#if defined(_WIN32)
#include "x86/eq_4front.h"
#elif defined(__ANDROID__)
#include "arm/eq_4front.h"
#endif