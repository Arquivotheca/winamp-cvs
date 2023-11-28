#include "PNGLoader.h"
#include "api.h"
#include <libpng/png.h>
#include <wchar.h>
#include <malloc.h>
#include <intsafe.h>
#include <nx/nximage.h>
#include "nx/nxdata.h"

PNGLoader::PNGLoader()
{

}

void premultiplyARGB32(ARGB32 *words, int nwords)
{
  for (; nwords > 0; nwords--, words++) {
    unsigned char *pixel = (unsigned char *)words;
    unsigned int alpha = pixel[3];
    if (alpha == 255) continue;
    pixel[0] = (pixel[0] * alpha) >> 8;	// blue
    pixel[1] = (pixel[1] * alpha) >> 8;	// green
    pixel[2] = (pixel[2] * alpha) >> 8;	// red
  }
}

static bool StringEnds(const wchar_t *a, const wchar_t *b)
{
	size_t aLen = wcslen(a);
	size_t bLen	= wcslen(b);
	if (aLen < bLen) return false;  // too short
	if (!_wcsicmp(a + aLen- bLen, b))
		return true;
	return false;
}

ns_error_t PNGLoader::ImageLoader_IsMine(nx_uri_t filename)
{
	if (filename && StringEnds(filename->string, L".PNG"))
		return 1;
	else
		return 0;
}

unsigned int PNGLoader::ImageLoader_GetHeaderSize()
{
	return 8;
}

ns_error_t PNGLoader::ImageLoader_TestData(nx_data_t data)
{
	const void *ptr;
	size_t length;
	NXDataGet(data, &ptr, &length);
	
	if (!png_sig_cmp((png_const_bytep)ptr, 0, length))
		return NErr_True;
	else
		return NErr_False;
}

typedef struct {
	const unsigned char *data;
	size_t pos;
	size_t datalen;
} my_read_info;

static void my_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	my_read_info *mri = (my_read_info *)png_get_io_ptr(png_ptr);
	if (mri->datalen - mri->pos < length)
		length = mri->datalen - mri->pos;
	memmove(data, mri->data + mri->pos, length);
	mri->pos += length;
}

ns_error_t PNGLoader::ImageLoader_LoadImage(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h)
{
	int w0=0, h0=0;
	const void *ptr;
	size_t length;
	NXDataGet(data, &ptr, &length);

	nx_image_t pixels = read_png(ptr, length, &w0, &h0, FALSE);

	if (pixels == NULL) return NErr_Error;

	premultiplyARGB32(pixels->image, w0 * h0);

	if(w) *w = w0;
	if(h) *h = h0;

	*image = pixels;
	return NErr_Success;
}

ns_error_t PNGLoader::ImageLoader_LoadImageData(nx_image_t *image, nx_data_t data, unsigned int *w, unsigned int *h)
{
	int w0=0, h0=0;
	const void *ptr;
	size_t length;
	NXDataGet(data, &ptr, &length);

	nx_image_t pixels = read_png(ptr, length, &w0, &h0, FALSE);

	if (pixels == NULL) return NErr_Error;

	if(w) *w = w0;
	if(h) *h = h0;

	*image = pixels;
	return NErr_Success;
}

ns_error_t PNGLoader::ImageLoader_GetDimensions(nx_data_t data, unsigned int *w, unsigned int *h)
{
		int w0=0, h0=0;
	const void *ptr;
	size_t length;
	NXDataGet(data, &ptr, &length);

	nx_image_t pixels = read_png(ptr, length, &w0, &h0, FALSE);

	if (pixels == NULL) return NErr_Error;

	if(w) *w = w0;
	if(h) *h = h0;

	return NErr_Success;
}

// From libpng example.c
nx_image_t PNGLoader::read_png(const void *data, int datalen, int *w, int *h, int dimensions_only) 
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	my_read_info mri;
	mri.data = static_cast<const unsigned char *>(data);
	mri.pos = 0;
	mri.datalen = datalen;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		return NULL;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return NULL;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		return NULL;
	}

	png_set_read_fn(png_ptr, &mri, my_png_read_data);

	/* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	if (w) *w = (int)width;
	if (h) *h = (int)height;

	nx_image_t retval = 0;
	png_bytep *row_pointers = NULL;

	if (!dimensions_only) {

		/* tell libpng to strip 16 bit/color files down to 8 bits/color */
		if (bit_depth == 16) png_set_strip_16(png_ptr);
		if (bit_depth < 8) png_set_packing(png_ptr);

		/* flip the RGB pixels to BGR (or RGBA to BGRA) */
		png_set_bgr(png_ptr);

		/* Expand paletted colors into true RGB triplets */
		if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);

		/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand(png_ptr);

		png_set_gray_to_rgb(png_ptr);

		/* Expand paletted or RGB images with transparency to full alpha channels
		* so the data will be available as RGBA quartets.
		*/
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			png_set_expand(png_ptr);
		}

		/* Add filler (or alpha) byte (before/after each RGB triplet) */
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

		/* Optional call to gamma correct and add the background to the palette
		* and update info structure.  REQUIRED if you are expecting libpng to
		* update the palette for you (ie you selected such a transform above).
		*/
		png_read_update_info(png_ptr, info_ptr);

		/* Allocate the memory to hold the image using the fields of info_ptr. */

		/* The easiest way to read the image: */
		//row_pointers = (png_bytep*)malloc(sizeof(png_bytep*)*height);
		size_t row_ptr_size = 0;
		if (SizeTMult(sizeof(png_bytep*), height, &row_ptr_size) == S_OK)
		{
			row_pointers = (png_bytep*)alloca(row_ptr_size);

			nx_image_t image = NXImageMalloc(width, height);
			if (image)
			{
				ARGB32 *bytes = image->image;

				for (unsigned int row = 0; row < height; row++) {
					row_pointers[row] = ((unsigned char *)bytes) + width * 4 * (row);
				}

				/* Now it's time to read the image.  One of these methods is REQUIRED */
				png_read_image(png_ptr, row_pointers);

				/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
				png_read_end(png_ptr, info_ptr);
			}
			retval = image;

		}

		//free(row_pointers);
	}

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	/* that's it */
	return retval;
}
