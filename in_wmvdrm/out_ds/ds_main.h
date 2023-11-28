#ifndef NULLSOFT_OUT_DS_MAIN_H
#define NULLSOFT_OUT_DS_MAIN_H

#ifdef UNICODE
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#else
#define WIDEN(x) x
#endif

#define DS2_ENGINE_VER "2.62 (d)"

#ifndef DS2_NO_DEVICES
#define DS2_HAVE_DEVICES
#endif

#ifndef DS2_NO_FADES
#define DS2_HAVE_FADES
#endif

#endif