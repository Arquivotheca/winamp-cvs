// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Thu May 15 21:07:09 2003]
// 
// File        : xmlparamsx.cpp
// Class       : skin_xmlreaderparams
// class layer : Dispatchable Receiver
// ----------------------------------------------------------------------------
#include <precomp.h>

#include "xmlparamsx.h"
#include "xmlparamsi.h"

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS XmlReaderParamsX
START_DISPATCH;
  CB(XMLREADERPARAMS_GETITEMNAME, getItemName);
  CB(XMLREADERPARAMS_GETITEMVALUE, getItemValue);
  CB(XMLREADERPARAMS_GETITEMVALUE2, getItemValue2);
  CB(XMLREADERPARAMS_ENUMITEMVALUES, enumItemValues);
  CB(XMLREADERPARAMS_GETITEMVALUEINT, getItemValueInt);
  CB(XMLREADERPARAMS_GETNBITEMS, getNbItems);
  VCB(XMLREADERPARAMS_ADDITEM, addItem);
  VCB(XMLREADERPARAMS_REMOVEITEM, removeItem);
  VCB(XMLREADERPARAMS_REPLACEITEM, replaceItem);
  CB(XMLREADERPARAMS_FINDITEM, findItem);
END_DISPATCH;
#undef CBCLASS

const wchar_t *XmlReaderParamsX::getItemValue2(const wchar_t *name) { return static_cast<XmlReaderParamsI *>(this)->getItemValue(name); }
