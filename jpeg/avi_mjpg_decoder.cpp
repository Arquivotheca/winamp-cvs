#include "api.h"
#include "avi_mjpg_decoder.h"
#include "../Winamp/wa_ipc.h"
#include <limits.h>
#include <intsafe.h>

#include <setjmp.h>

extern "C"
{
#undef FAR
#include "../ijg/jpeglib.h"
};


uint8_t mjpeg_header[0x1A4] = 
{
	/* JPEG DHT Segment for YCrCb omitted from MJPG data */
	0xFF,0xC4,0x01,0xA2,
	0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
	0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
	0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
	0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
	0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
	0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
	0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
	0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
	0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
	0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
	0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
	0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
	0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
	0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
	0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
	0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
	0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
	0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
	0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
	0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
	0xF9,0xFA
};

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == 'GPJM') // mjpg
		{
			*decoder = AVIMJPEG::CreateDecoder(format);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
			else
				return CREATEDECODER_FAILURE;
		}
	}

	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

AVIMJPEG *AVIMJPEG::CreateDecoder(nsavi::video_format *stream_format)
{
	AVIMJPEG *decoder = new AVIMJPEG(stream_format);
	if (!decoder)
	{
		return 0;	
	}

	return decoder;
}


AVIMJPEG::AVIMJPEG(nsavi::video_format *stream_format) : stream_format(stream_format)
{

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpegLoader=0;
	width=0;
	height=0;
	decoded_image = 0;
	image_size = 0;
	image_outputted = true;
}



int AVIMJPEG::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	*x = stream_format->width;
	*y = stream_format->height;
	*color_format = '23GR'; // RGB32
	*aspect_ratio = 1.0;
	*flip=0;
	return AVI_SUCCESS;
}

struct mjpeg_source_mgr
{
	jpeg_source_mgr src;
	int state;
	const JOCTET *input_buffer;
	size_t input_buffer_bytes;

};

static void init_source(j_decompress_ptr cinfo)
{
	int x;
	x=0;
}

static const JOCTET jpeg_eof[] = {(JOCTET) 0xFF, (JOCTET) JPEG_EOI};

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
	mjpeg_source_mgr *msrc = (mjpeg_source_mgr *)cinfo->src;
	jpeg_source_mgr *src = &msrc->src;


	switch(msrc->state++)
	{
	case 0:
		src->next_input_byte = mjpeg_header;
		src->bytes_in_buffer = sizeof(mjpeg_header);
		return TRUE;
	case 1:
		src->next_input_byte = msrc->input_buffer+2;
		src->bytes_in_buffer = msrc->input_buffer_bytes-2;
		return TRUE;
	case 2:
		src->next_input_byte = jpeg_eof;
		src->bytes_in_buffer = 2;
		return TRUE;
	}
	return TRUE;
}

void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes > 0)
	{
		if (num_bytes > (long) cinfo->src->bytes_in_buffer)
		{
			fill_input_buffer(cinfo);
		}
		else
		{
			cinfo->src->next_input_byte += (size_t) num_bytes;
			cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
		}
	}
}
void term_source(j_decompress_ptr cinfo) 
{
}


static void wasabi_jpgload_error_exit(j_common_ptr cinfo)
{
	jmp_buf *stack_env = (jmp_buf *)cinfo->client_data;
	longjmp(*stack_env, 1);
}

int AVIMJPEG::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	bool change_in_size=false;

	//int decode_width, decode_height;



	mjpeg_source_mgr src = {{(const JOCTET *)inputBuffer,2,init_source,fill_input_buffer,skip_input_data,jpeg_resync_to_restart,term_source}, 0, (const JOCTET *)inputBuffer, inputBufferBytes};

	cinfo.src = &src.src;

	/* set up error handling.  basically C style exceptions :) */
	jmp_buf stack_env;
	cinfo.client_data=&stack_env;
	cinfo.err->error_exit = wasabi_jpgload_error_exit;
	if (setjmp(stack_env))
	{
		// longjmp will goto here
		jpeg_destroy_decompress(&cinfo);
		return AVI_FAILURE;
	}

	if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK)
	{
		cinfo.out_color_space = JCS_RGB;

		int ret = jpeg_start_decompress(&cinfo);

		int this_image_size = cinfo.output_width * cinfo.output_height * 4;
		if (this_image_size > image_size)
		{
			image_size = this_image_size;
			decoded_image = malloc(image_size);
		}
		ARGB32 *p = (ARGB32 *)decoded_image;// + (cinfo.output_width * cinfo.output_height);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			//p -= cinfo.output_width;
			jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&p, 1);

			// this is pretty homo, but it has to be done
			for (unsigned int i=0; i < cinfo.output_width; i++)
				//      full alpha   shift red to correct place  leave green alone   shift blue to correct place
				p[i] = (0xff000000 | ((p[i] & 0x0000ff) << 16) | (p[i] & 0x00ff00) | ((p[i] & 0xff0000) >> 16));

			p += cinfo.output_width;
		}

		//if (w) *w = cinfo.output_width;
		//if (h) *h = cinfo.output_height;
		jpeg_finish_decompress(&cinfo);

	}
	//jpeg_destroy_decompress(&cinfo);

	image_outputted=false;
	return AVI_SUCCESS;
}

void AVIMJPEG::Flush()
{

}

int AVIMJPEG::GetPicture(void **data, void **decoder_data)
{
	if (image_outputted)
		return AVI_FAILURE;

	*data = decoded_image;
	*decoder_data = 0;
	image_outputted = true;
	return AVI_SUCCESS;
}

void AVIMJPEG::FreePicture(void *data, void *decoder_data)
{

}

void AVIMJPEG::Close()
{
	delete jpegLoader;
	delete this;
}


#define CBCLASS AVIMJPEG
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
END_DISPATCH;
#undef CBCLASS
