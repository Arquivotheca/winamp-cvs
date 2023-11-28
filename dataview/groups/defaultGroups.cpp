#include "main.h"
#include "./defaultGroups.h"

#include "./ifc_groupprovider.h"

#include "./artistGroupProvider.h"
#include "./albumGroupProvider.h"
#include "./genreGroupProvider.h"

typedef HRESULT (*GroupProviderCreator)(ifc_groupprovider ** /*instance*/);


static const GroupProviderCreator defaultProviders[] = 
{
	ArtistGroupProvider::CreateInstance,
	AlbumGroupProvider::CreateInstance,
	GenreGroupProvider::CreateInstance,
};


HRESULT 
RegisterDefaultGroups(ifc_groupmanager *manager)
{
	HRESULT hr;
	size_t index, count;
	ifc_groupprovider *providers[ARRAYSIZE(defaultProviders)];

	if (NULL == manager)
		return E_INVALIDARG;

	count = 0;
	for (index = 0; index < ARRAYSIZE(defaultProviders); index++)
	{
		if (SUCCEEDED(defaultProviders[index](&providers[count])))
			count++;
	}

	index = manager->Register(providers, count);
	if (index == count)
		hr = S_OK;
	else
		hr = E_FAIL;

	while(count--)
		providers[count]->Release();
	
	return hr;
}
