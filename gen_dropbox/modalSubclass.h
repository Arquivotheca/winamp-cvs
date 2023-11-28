#ifndef NULLOSFT_MODAL_SUBCLASS_HOOK_HEADER
#define NULLOSFT_MODAL_SUBCLASS_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef INT (CALLBACK *MODALSUBCLASSPROC)(HWND /*hTarget*/, CREATESTRUCT* /*pcs*/, HWND /*hInsertAfter*/, ULONG_PTR /*user*/); // return 0 if ok

BOOL BeginModalSubclass(MODALSUBCLASSPROC callback, ULONG_PTR user);
void EndModalSubclass(void);



#endif // NULLOSFT_MODAL_SUBCLASS_HOOK_HEADER