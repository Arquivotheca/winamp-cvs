// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Wed May 07 00:56:11 2003]
// 
// File        : api_servicex.h
// Class       : api_service
// class layer : Dispatchable Receiver
// ----------------------------------------------------------------------------

#ifndef __API_SERVICEX_H
#define __API_SERVICEX_H

#include "api_service.h"

class waServiceFactory;



// ----------------------------------------------------------------------------

class api_serviceX : public api_service {
  protected:
    api_serviceX() {}
  public:
    virtual int service_register(waServiceFactory *svc)=0;
    virtual int service_deregister(waServiceFactory *svc)=0;
    virtual int service_getNumServices(FOURCC svc_type)=0;
    virtual waServiceFactory *service_enumService(FOURCC svc_type, int n)=0;
    virtual waServiceFactory *service_getServiceByGuid(GUID guid)=0;
    virtual int service_lock(waServiceFactory *owner, void *svcptr)=0;
    virtual int service_clientLock(void *svcptr)=0;
    virtual int service_release(void *svcptr)=0;
    virtual const char *service_getTypeName(FOURCC svc_type)=0;
  #ifdef WASABI_COMPILE_COMPONENTS
    virtual GUID service_getOwningComponent(void *svcptr)=0;
    virtual GUID service_getLockingComponent(void *svcptr)=0;
  #endif // WASABI_COMPILE_COMPONENTS
    virtual int service_unlock(void *svcptr)=0;
    virtual int service_isvalid(FOURCC svctype, waServiceFactory *service)=0;
  
  protected:
    RECVS_DISPATCH;
};

#endif // __API_SERVICEX_H
