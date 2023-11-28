#include "../nde_c.h"
#include "../DBUtils.h"
extern "C" void NDE_HeapInit();
extern "C" void NDE_HeapQuit();
static volatile LONG init_count=0;

/* NDE_Init isn't thread safe, be aware
best to call on the main thread during initialization
*/
void NDE_Init()
{
	if (init_count == 0)
	{
		NDE_HeapInit();
		HMODULE klib = LoadLibraryW(L"Kernel32.dll");
		if (klib)
		{
			void *nls = GetProcAddress(klib, "FindNLSString");
			if (nls)
				*((void **)&findNLSString) = nls;
		}
		FreeModule(klib);
		
	}
	InterlockedIncrement(&init_count);
}

void NDE_Quit()
{
	if (InterlockedDecrement(&init_count) == 0)
		NDE_HeapQuit();
}