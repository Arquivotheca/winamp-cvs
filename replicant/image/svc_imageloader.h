#pragma once
#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "nx/nxuri.h"
#include "nx/nximage.h"
#include "nx/nxdata.h"

/* benski> this is a work in progress and subject to change */

// {86371C14-D2D9-49a1-9B96-159ADFE43B9C}
static const GUID svc_imageloader_type_guid = 
{ 0x86371c14, 0xd2d9, 0x49a1, { 0x9b, 0x96, 0x15, 0x9a, 0xdf, 0xe4, 0x3b, 0x9c } };

class NOVTABLE svc_imageloader : public Wasabi2::Dispatchable
{
protected:
	svc_imageloader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_imageloader() {}
public:
	static GUID GetServiceType() { return svc_imageloader_type_guid; }

  // assuming there is an extension of this type, is it yours?
	ns_error_t IsMine(nx_uri_t filename) { return ImageLoader_IsMine(filename); }

  // returns how many bytes needed to get image info
	unsigned int GetHeaderSize() { return ImageLoader_GetHeaderSize(); }

  // test image data, return NErr_True if you can load it
	ns_error_t TestData(nx_data_t data) { return ImageLoader_TestData(data); }

  // just gets the width and height from the data, if possible
	ns_error_t GetDimensions(nx_data_t data, unsigned int *w, unsigned int *h) { return ImageLoader_GetDimensions(data, w, h); }

  // converts the data into pixels + premultiply, use api->sysFree to deallocate
	ns_error_t LoadImage(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h) { return ImageLoader_LoadImage(image, data, w, h); }
  
  // converts the data into pixels, use api->sysFree to deallocate
  ns_error_t LoadImageData(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h) { return ImageLoader_LoadImageData(image, data, w, h); }

private:
	virtual ns_error_t WASABICALL ImageLoader_IsMine(nx_uri_t filename)=0;
  virtual unsigned int WASABICALL ImageLoader_GetHeaderSize()=0;
  virtual ns_error_t WASABICALL ImageLoader_TestData(nx_data_t data)=0;
  virtual ns_error_t WASABICALL ImageLoader_GetDimensions(nx_data_t data, unsigned int *w, unsigned int *h)=0;
  virtual ns_error_t WASABICALL ImageLoader_LoadImage(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h)=0;
  virtual ns_error_t WASABICALL ImageLoader_LoadImageData(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h)=0;

	enum 
	{
		DISPATCHABLE_VERSION=0,		
	};
};

