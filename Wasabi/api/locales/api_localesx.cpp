// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Fri Oct 17 16:03:31 2003]
// 
// File        : api_localesx.cpp
// Class       : api_locales
// class layer : Dispatchable Receiver
// ----------------------------------------------------------------------------
#include <precomp.h>

#include "api_localesx.h"
#include "api_localesi.h"

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS api_localesX
START_DISPATCH;
  CB(API_LOCALES_LOCALES_GETTRANSLATION, locales_getTranslation);
  VCB(API_LOCALES_LOCALES_ADDTRANSLATION, locales_addTranslation);
  CB(API_LOCALES_LOCALES_GETBINDFROMACTION, locales_getBindFromAction);
  //CB(API_LOCALES_LOCALES_GETNUMENTRIES, locales_getNumEntries);
  //CB(API_LOCALES_LOCALES_ENUMENTRY, locales_enumEntry);
  VCB(API_LOCALES_LOCALES_REGISTERACCELERATORSECTION, locales_registerAcceleratorSection);
END_DISPATCH;
#undef CBCLASS