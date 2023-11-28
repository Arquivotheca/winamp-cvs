#ifndef NULLSOFT_ERASEMEDIUM_HEADER
#define NULLSOFT_ERASEMEDIUM_HEADER

#include "./main.h"
#include "./primosdk.h"

#define ERASEMEDIUM_STATUS				0x0000
#define ERASEMEDIUM_ERROR				0x1000

// status messages
#define ERASEMEDIUM_READY				((ERASEMEDIUM_STATUS) + 0x0001)
#define ERASEMEDIUM_INITIALIZING			((ERASEMEDIUM_STATUS) + 0x0002)
#define ERASEMEDIUM_ERASING				((ERASEMEDIUM_STATUS) + 0x0003)
#define ERASEMEDIUM_FINISHING			((ERASEMEDIUM_STATUS) + 0x0004)
#define ERASEMEDIUM_CANCELING			((ERASEMEDIUM_STATUS) + 0x0005)
#define ERASEMEDIUM_COMPLETED			((ERASEMEDIUM_STATUS) + 0x0006)
#define ERASEMEDIUM_ABORTED				((ERASEMEDIUM_STATUS) + 0x0007)


// error messages
#define ERASEMEDIUM_ALREADYSTARTED		((ERASEMEDIUM_ERROR) + 0x0001)
#define ERASEMEDIUM_UNABLEINITPRIMO		((ERASEMEDIUM_ERROR) + 0x0002)
#define ERASEMEDIUM_DEVICENOTREADY		((ERASEMEDIUM_ERROR) + 0x0003)
#define ERASEMEDIUM_DISCINFOERROR		((ERASEMEDIUM_ERROR) + 0x0004)
#define ERASEMEDIUM_DISCNOTERASABLE		((ERASEMEDIUM_ERROR) + 0x0005)
#define ERASEMEDIUM_BEGINBURNFAILED		((ERASEMEDIUM_ERROR) + 0x0006)
#define ERASEMEDIUM_ENDBURNFAILED		((ERASEMEDIUM_ERROR) + 0x0007)
#define ERASEMEDIUM_ERASEMEDIUMFAILED	((ERASEMEDIUM_ERROR) + 0x0008)

// callback returns
#define ERASEMEDIUM_CONTINUE	0
#define ERASEMEDIUM_STOP		1


typedef DWORD (WINAPI *ERASEMEDIUMCALLBACK)(void*, void*, DWORD, DWORD);

class EraseMedium
{
public:
	BURNLIB_API EraseMedium(void);
	BURNLIB_API ~EraseMedium(void);

public:
	BURNLIB_API DWORD Start(DWORD drive, DWORD eraseMode, ERASEMEDIUMCALLBACK notifyCB, void *userParam, int block);
	BURNLIB_API void Stop(void);
	BURNLIB_API DWORD GetErrorCode(void) { return errorCode; }
	BURNLIB_API BOOL IsRunning(void) { return (NULL != hThread); }

public: 
	BURNLIB_API DWORD SetEject(DWORD eject);
protected:
	DWORD OnNotify(DWORD eraseCode, DWORD primoCode);
	static DWORD WINAPI StatusThread(void* parameter);

protected:
	WABURNSTRUCT		bs;
	obj_primo			*primoSDK;
	unsigned int	eject;
	HANDLE			hThread;
	HANDLE			evntStop;
	HANDLE			evntThreadExit;
	DWORD			errorCode;
	void*			userparam;

	ERASEMEDIUMCALLBACK	notifyCB;
};

#endif //NULLSOFT_ERASEMEDIUM_HEADER
