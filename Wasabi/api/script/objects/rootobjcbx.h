// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Thu May 15 21:06:33 2003]
// 
// File        : rootobjcbx.h
// Class       : RootObjectCallback
// class layer : Dispatchable Receiver
// ----------------------------------------------------------------------------

#ifndef __ROOTOBJECTCALLBACKX_H
#define __ROOTOBJECTCALLBACKX_H

#include "rootobjcb.h"




// ----------------------------------------------------------------------------

class RootObjectCallbackX : public RootObjectCallback {
  protected:
    RootObjectCallbackX() {}
  public:
    virtual void rootobjectcb_onNotify(const wchar_t *a, const wchar_t *b, int c, int d)=0;
  
  protected:
    RECVS_DISPATCH;
};

#endif // __ROOTOBJECTCALLBACKX_H
