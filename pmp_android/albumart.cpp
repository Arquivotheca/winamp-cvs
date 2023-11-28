#include "api.h"
#include "../agave/albumart/svc_albumartprovider.h"
#include <api/service/waservicefactory.h>


static svc_albumArtProvider *FindProvider(const wchar_t *filename, int providerType, waServiceFactory **factory)
{
	FOURCC albumartprovider = svc_albumArtProvider::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(albumartprovider);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(albumartprovider,i);
		if (sf)
		{
			svc_albumArtProvider * provider = (svc_albumArtProvider*)sf->getInterface();
			if (provider)
			{
				if (provider->ProviderType() == providerType && provider->IsMine(filename))
				{
					*factory = sf;
					return provider;
				}
				sf->releaseInterface(provider);
			}
		}
	}
	return NULL;
}

void CopyAlbumArt(const wchar_t *source, const wchar_t *destination)
{


	void *data=0;
	size_t datalen=0;
	wchar_t *mimeType=0;

	
	waServiceFactory *destinationFactory=0;
	svc_albumArtProvider *destinationProvider = FindProvider(destination, ALBUMARTPROVIDER_TYPE_EMBEDDED, &destinationFactory);
	if (destinationFactory)
	{
		/* First, look to see if there's already embedded album art */
		if (destinationProvider->GetAlbumArtData(destination, L"cover", &data, &datalen, &mimeType) == ALBUMARTPROVIDER_SUCCESS && data && datalen)
		{
			destinationFactory->releaseInterface(destinationProvider);
			WASABI_API_MEMMGR->sysFree(data);
			WASABI_API_MEMMGR->sysFree(mimeType);
			return;
		}
		else if (AGAVE_API_ALBUMART->GetAlbumArtData(source, L"cover", &data, &datalen, &mimeType) == ALBUMART_SUCCESS && data && datalen)
		{
			destinationProvider->SetAlbumArtData(destination, L"cover", data, datalen, mimeType);
			WASABI_API_MEMMGR->sysFree(data);
			WASABI_API_MEMMGR->sysFree(mimeType);

			destinationFactory->releaseInterface(destinationProvider);
		}
	}
}