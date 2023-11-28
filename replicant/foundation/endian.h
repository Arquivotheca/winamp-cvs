#pragma once
// TODO!
#ifdef __linux__
#if defined(__ANDROID__)
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#elif defined(_WIN32) && (defined(_M_IX86) || defined(_M_X64))
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(__APPLE__)
#include <machine/endian.h>
#else
#error port me!
#endif
