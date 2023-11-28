#ifndef _ADDONS_H
#define _ADDONS_H

#include "xuipreviewrect.h"
#include "xuisetlist.h"
#include "xuigrouplist.h"
#include "xuiinstance.h"


#include <api/syscb/callbacks/skincb.h>
#include <api/wac/wac.h>


// -----------------------------------------------------------------------
#define WACPARENT WAComponentClient
class WACColorEditor : public WACPARENT, public SkinCallbackI 
{
  public:

    WACColorEditor();
    virtual ~WACColorEditor();

    virtual const wchar_t *getName();
    virtual GUID getGUID();

    virtual void onCreate();
    virtual void onDestroy();
    virtual int skincb_onGuiLoaded();

  private:

};

extern WACColorEditor *coloreditorwac;

#endif
