#pragma once
#include "image/svc_imageloader.h"
#include "nswasabi/ServiceName.h"

// {5E04FB28-53F5-4032-BD29-032B87EC3725}
static const GUID pngGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };


class PNGLoader : public svc_imageloader
{
public:
	PNGLoader();
	WASABI_SERVICE_NAME("PNG Loader");
	WASABI_SERVICE_GUID(pngGUID);
  // service

	ns_error_t WASABICALL ImageLoader_IsMine(nx_uri_t filename);
  unsigned int WASABICALL ImageLoader_GetHeaderSize();
  ns_error_t WASABICALL ImageLoader_TestData(nx_data_t data);
  ns_error_t WASABICALL ImageLoader_GetDimensions(nx_data_t data, unsigned int *w, unsigned int *h);
  ns_error_t WASABICALL ImageLoader_LoadImage(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h);
  ns_error_t WASABICALL ImageLoader_LoadImageData(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h);

  
private:
  nx_image_t read_png(const void *data, int datalen, int *w, int *h, int dimensions_only);


};
