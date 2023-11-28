#include <stdio.h>
#include <windows.h>
#include "../h264dec/dec_api.h"
namespace conformance_itrans8x8 {
#include "../h264dec/ldecod/src/transform8x8.c"
}

 __declspec(align(32)) h264_short_8x8block_t known_coef =
 {
{0x0654, 0x04fa, 0xfcaf, 0x0472, 0x02b2, 0x00ea, 0xfeee, 0xffb3, },
{0xfe3b, 0xfc7c, 0xfed9, 0x004d, 0x0270, 0xf813, 0x04b8, 0x00da, },
{0x0580, 0xfec9, 0x05f3, 0xfd6f, 0xfe5e, 0xfac8, 0x0795, 0xfc29, },
{0xfa96, 0x030f, 0x04fb, 0xf939, 0xfad0, 0x06a5, 0xfedf, 0xf9de, },
{0xf935, 0xf8aa, 0x0667, 0xfae4, 0xfe83, 0x01e5, 0xf8b3, 0xfe48, },
{0xf8d6, 0xfba0, 0xfe81, 0xfc08, 0xf93d, 0x079d, 0x0433, 0xfa58, },
{0xfb34, 0xf943, 0xfdfe, 0x0109, 0xfa61, 0x044e, 0xfc47, 0xfc15, },
{0xfad5, 0x047e, 0x0476, 0x01a1, 0x0734, 0x028a, 0x01d8, 0xff09, },
 };
 __declspec(align(32))  h264_imgpel_macroblock_t known_pred=
 {
{0x77,0x82,0xdb,0x52,0x97,0x41,0x87,0xd6,},
{0x50,0xa2,0x78,0x80,0xfc,0x3f,0xfd,0x1d,},
{0x3f,0x10,0x7d,0x88,0xa3,0x0a,0x60,0xae,},
{0x09,0x7d,0xd5,0xa4,0xe0,0xa1,0xac,0x93,},
{0xb4,0xd9,0xaf,0x4c,0x49,0x46,0x1f,0x14,},
{0x83,0x56,0x77,0x39,0xb2,0x7a,0x35,0xbb,},
{0xfd,0x64,0xda,0x64,0x31,0xfe,0xac,0x50,},
{0xe6,0xb5,0xc6,0x07,0x19,0xd3,0x80,0xdd,},
 };


static void print_8x8_results(const h264_imgpel_macroblock_t output)
{
		for (int i=0;i<8;i++)
	{
		for (int j=0;j<8;j++)
		{
			printf("0x%02x ", output[i][j]);
		}
		printf("\r\n");
	}
}


static bool match_8x8_results(const h264_imgpel_macroblock_t a,const h264_imgpel_macroblock_t b)
{
		for (int i=0;i<8;i++)
	{
		for (int j=0;j<8;j++)
		{
			if (a[i][j] != b[i][j])
				return false;
		}
	}
		return true;
}

static bool test_8x8_sse2(const h264_short_8x8block_t coefficients, const h264_imgpel_macroblock_t predictors)
{
	bool any_failed=false;
		h264_imgpel_macroblock_t output1, output2;
		__declspec(align(32)) h264_short_8x8block_t coefficients_output;

			memcpy(coefficients_output, coefficients, sizeof(h264_short_8x8block_t));
			conformance_itrans8x8::inverse8x8(coefficients_output);
			conformance_itrans8x8::sample_reconstruct8x8(output1, predictors, coefficients_output, 0, 255);
			conformance_itrans8x8::itrans8x8_sse2(output2, predictors, coefficients, 0);
			if (!match_8x8_results(output1, output2))
			{
				printf(" *** conformance failed ***\r\n");
				printf("reference: \r\n");
				print_8x8_results(output1);
				printf("optimized: \r\n");
				print_8x8_results(output2);
				any_failed=true;
			}

		return !any_failed;
}

static bool test_8x8_mmx(const h264_short_8x8block_t coefficients, const h264_imgpel_macroblock_t predictors)
{
	bool any_failed=false;
		h264_imgpel_macroblock_t output1, output2;
		__declspec(align(32)) h264_short_8x8block_t coefficients_output;

			memcpy(coefficients_output, coefficients, sizeof(h264_short_8x8block_t));
			conformance_itrans8x8::inverse8x8(coefficients_output);
			conformance_itrans8x8::sample_reconstruct8x8(output1, predictors, coefficients_output, 0, 255);
			conformance_itrans8x8::sample_reconstruct8x8_mmx(output2, predictors, coefficients_output, 0);
			if (!match_8x8_results(output1, output2))
			{
				printf(" *** conformance failed ***\r\n");
				printf("reference: \r\n");
				print_8x8_results(output1);
				printf("optimized: \r\n");
				print_8x8_results(output2);
				any_failed=true;
			}

		return !any_failed;
}

static void print_predictors(const h264_imgpel_macroblock_t coefficients)
{
for (int i=0;i<8;i++)
	{
		printf("{");
		for (int j=0;j<8;j++)
		{
			printf("0x%02x,", coefficients[i][j]);
		}
		printf("},\r\n");
	}

}

static void print_coefficients(const h264_short_8x8block_t coefficients)
{
for (int i=0;i<8;i++)
	{
		printf("{");
		for (int j=0;j<8;j++)
		{
			printf("0x%04hx, ", coefficients[i][j]);
		}
		printf("},\r\n");
	}
}

#define DCT_COEF_RANGE 2048
static void generate_random_coefficients(h264_short_8x8block_t coefficients)
{
	for (int i=0;i<8;i++)
	{
		for (int j=0;j<8;j++)
		{
			coefficients[i][j] = (rand()%(DCT_COEF_RANGE*2-1)) - DCT_COEF_RANGE; 
		}
	}
}

static void generate_random_predictors(h264_imgpel_macroblock_t mb_pred)
{
	for (int i=0;i<16;i++)
	{
		for (int j=0;j<16;j++)
		{
			mb_pred[i][j] = rand()%256; 
		}
	}
}

bool test_itrans8x8()
{
	
	bool any_failed=false;
	__declspec(align(32)) h264_short_8x8block_t rand_coefficients;
	h264_imgpel_macroblock_t rand_predictors;
	
	printf("    [testing known coefficients]\r\n");
		if (test_8x8_sse2(known_coef, known_pred) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(rand_coefficients);
			printf(" *** predictors for failure: ***\r\n");
			print_predictors(rand_predictors);
			any_failed = true;
		}
	

	
	printf("    [testing random coefficients]\r\n");
	for (int i=0;i<1000;i++)
	{
		generate_random_coefficients(rand_coefficients);
		generate_random_predictors(rand_predictors);
		if (test_8x8_sse2(rand_coefficients, rand_predictors) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(rand_coefficients);
			printf(" *** predictors for failure: ***\r\n");
			print_predictors(rand_predictors);
			any_failed = true;
		}
		/*
		if (test_8x8_mmx(rand_coefficients, rand_predictors) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(rand_coefficients);
			printf(" *** predictors for failure: ***\r\n");
			print_predictors(rand_predictors);
			any_failed = true;
		}*/
	}
	return !any_failed;

	return true;

}