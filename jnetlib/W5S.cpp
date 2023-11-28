#include "netinc.h"
#include "api.h"
#include "wa5_jnetlib.h"
#include "httpget_factory.h"
#include "factory_connection.h"
#include "factory_sslconnection.h"
#include "factory_dns.h"
#include "webserv_factory.h"
#include "util.h"

#include <api/syscb/api_syscb.h>
#include "sslconnection.h"
#include <openssl/ssl.h>
#include <bfc/platform/export.h>
#include "../nu/nonewthrow.c"

JNL_HTTPGetFactory HTTPGetService;
JNL_ConnectionFactory connectionService;
JNL_AsyncDNSFactory dnsService;
JNL_WebServFactory webserverService;

WA5_JNetLib wa5_jnetlib;

api_syscb *sysCallbackApi=0;

#ifdef USE_SSL
#include <wincrypt.h>
#include <openssl/rand.h>

static HCRYPTPROV GetKeySet()
{
	HCRYPTPROV   hCryptProv;
	LPCSTR UserName = "WinampKeyContainer";  // name of the key container

	if (CryptAcquireContext(
	      &hCryptProv,               // handle to the CSP
	      UserName,                  // container name
	      NULL,                      // use the default provider
	      PROV_RSA_FULL,             // provider type
	      0))                        // flag values
	{
		return hCryptProv;
	}
	else if (CryptAcquireContext(
	           &hCryptProv,
	           UserName,
	           NULL,
	           PROV_RSA_FULL,
	           CRYPT_NEWKEYSET))
	{
		return hCryptProv;
	}
	else
		return 0;

}

JNL_SSL_ConnectionFactory sslConnectionService;
void InitSSL()
{
	SSL_load_error_strings();
	SSL_library_init();

	HCRYPTPROV hCryptProv = GetKeySet();
	if (hCryptProv)
	{
		BYTE pbData[8*sizeof(unsigned long)];
		if (CryptGenRandom(hCryptProv, 8*sizeof(unsigned long), pbData))
		{
			RAND_seed(pbData, 16);
		}
		CryptReleaseContext(hCryptProv,0);
	}

	sslContext = SSL_CTX_new(SSLv23_client_method());
	SSL_CTX_set_verify(sslContext, SSL_VERIFY_NONE, NULL);
//	SSL_CTX_set_session_cache_mode(sslContext, SSL_SESS_CACHE_OFF);
}


void QuitSSL()
{
	SSL_CTX_free(sslContext);
}

#endif

void WA5_JNetLib::RegisterServices(api_service *service)
{
#ifdef USE_SSL
	InitSSL();
#endif
	WASABI_API_SVC = service;

	waServiceFactory *sf = serviceManager->service_getServiceByGuid(syscbApiServiceGuid);
	if (sf) sysCallbackApi = reinterpret_cast<api_syscb *>(sf->getInterface());

	WASABI_API_SVC->service_register(&HTTPGetService);
	WASABI_API_SVC->service_register(&connectionService);
#ifdef USE_SSL
	WASABI_API_SVC->service_register(&sslConnectionService);
#endif
	WASABI_API_SVC->service_register(&dnsService);
	WASABI_API_SVC->service_register(&webserverService);
}

int WA5_JNetLib::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_JNetLib::DeregisterServices(api_service *service)
{
	DestroyGlobalDNS();
	service->service_deregister(&HTTPGetService);
	service->service_deregister(&connectionService);
#ifdef USE_SSL
	service->service_deregister(&sslConnectionService);
#endif
	service->service_deregister(&dnsService);

#ifdef USE_SSL
	QuitSSL();
#endif
}

extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_jnetlib;
}

#define CBCLASS WA5_JNetLib
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS