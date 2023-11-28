#include "main.h"

#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>


ARGB32*
Image_Load(const void *data, size_t length, int *width, int *height, BOOL premultiply) 
{
	size_t count, index;
	FOURCC imgload;
	waServiceFactory *sf;
	svc_imageLoader *loader;
	ARGB32* result;

	imgload = svc_imageLoader::getServiceType();
	count = WASABI_API_SVC->service_getNumServices(imgload);
	
	for(index = 0; index < count; index++) 
	{
		sf = WASABI_API_SVC->service_enumService(imgload, index);
		if(NULL != sf) 
		{
			loader = (svc_imageLoader*)sf->getInterface();
			if(NULL != loader) 
			{
				if(FALSE != loader->testData(data, length)) 
				{					
					if(FALSE == premultiply) 
						result = loader->loadImageData(data, length, width, height);
					else 
						result = loader->loadImage(data, length, width, height);
					
					sf->releaseInterface(loader);
					return result;
				}
				sf->releaseInterface(loader);
			}
		}
	}
	return NULL;
}

ARGB32* 
Image_LoadFromResource(const wchar_t *type, const wchar_t *name, int *width, int *height, BOOL premultiply)
{
	unsigned long size;
	HGLOBAL handle;
	ARGB32 *result;
	
	handle = WASABI_API_LOADRESFROMFILEW(type, name, &size);
	if(NULL == handle)
		return NULL;
	
	result = Image_Load(handle, size, width, height, premultiply);
	UnlockResource(handle);
	
	return result;
}

void 
Image_Colorize(ARGB32 *data, size_t length, COLORREF frontColor)
{
	unsigned int a, b, g, r, px;
	unsigned int frontB, frontG, frontR;
	ARGB32 *end;
	
	
	frontB = frontColor & 0xff;
	frontG = (frontColor >> 8) & 0xff;
	frontR = (frontColor >> 16) & 0xff;

	for(end = data + length; data < end; data++)
	{
		px = *data;
		a = (px >> 24) & 0xff;
		b = a * ((px & 0xff) * frontB) / (0xff*0xff);
		g = a * (((px >> 8) & 0xff) * frontG) / (0xff*0xff);
		r = a * (((px >> 16) & 0xff) * frontR) / (0xff*0xff);
		
		*data = (a << 24) | (r & 0xff) | ((g & 0xff) << 8) | ((b & 0xff) << 16);
	}
}