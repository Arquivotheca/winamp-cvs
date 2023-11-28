#include <precomp.h>
#include "gc.h"
#include <api/api.h>

GarbageCollector *garbageCollector=NULL;

GarbageCollector::GarbageCollector() {
  last = 0;
  WASABI_API_SYSCB->syscb_registerCallback(this);
}

GarbageCollector::~GarbageCollector() {
  WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

int GarbageCollector::gccb_onGarbageCollect() {
  uint32_t tc = Std::getTickCount();
  if (tc < last + 10000) return 0;

  last = tc;
#ifdef WIN32
  //SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
#endif
  return 0;
}
