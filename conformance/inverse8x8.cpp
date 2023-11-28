#include <stdio.h>
#include <windows.h>
#include "../h264dec/dec_api.h"
namespace conformance_inverse8x8 {
#include "../h264dec/ldecod/src/transform8x8.c"
}

static __declspec(align(32)) h264_short_8x8block_t known_coef =
 {
{0xfcb7, 0xfaa7, 0xfb34, 0xfbda, 0xfea1, 0x0534, 0xfddd, 0x0329, },
{0xf809, 0x033c, 0xfc3c, 0xf8d3, 0x05ed, 0xf909, 0x078b, 0xff02, },
{0x0060, 0x02be, 0xfe73, 0x006d, 0x02fb, 0xfaf0, 0xf950, 0xfbed, },
{0xf981, 0x048c, 0xfc4d, 0xfdc1, 0x04df, 0xfc09, 0x0419, 0xf9a3, },
{0xfc73, 0x03c9, 0x00c1, 0xfa6d, 0xff59, 0xffa7, 0x0297, 0xfab2, },
{0xfa46, 0x03f5, 0xfdc7, 0xfaf5, 0x054b, 0xfba3, 0x07b4, 0xf8c7, },
{0xfe51, 0xf976, 0x02ea, 0xfb58, 0x033f, 0xf8e7, 0xfd25, 0xf944, },
{0x00e3, 0x01de, 0x03a4, 0xfb8e, 0xf9d5, 0x03f8, 0x01d8, 0x01d3, },
};
static void print_8x8_results(const h264_short_8x8block_t output)
{
		for (int i=0;i<8;i++)
	{
		for (int j=0;j<8;j++)
		{
			printf("%+06hi ", output[i][j]);
		}
		printf("\r\n");
	}
}

static bool match_8x8_results(const h264_short_8x8block_t a,const h264_short_8x8block_t b)
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

static bool test_8x8(const h264_short_8x8block_t coefficients)
{
	bool any_failed=false;
		__declspec(align(32)) h264_short_8x8block_t output1, output2;

			memcpy(output1, coefficients, sizeof(h264_short_8x8block_t));
			conformance_inverse8x8::inverse8x8(output1);
			memcpy(output2, coefficients, sizeof(h264_short_8x8block_t));
			conformance_inverse8x8::inverse8x8_sse2(output2);
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

bool test_inverse8x8()
{
	bool any_failed=false;
	__declspec(align(32)) h264_short_8x8block_t rand_coefficients;
	
	if (test_8x8(known_coef) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(known_coef);
			any_failed = true;
		}
	

	printf("    [testing random coefficients]\r\n");
	for (int i=0;i<1000;i++)
	{
		generate_random_coefficients(rand_coefficients);
		if (test_8x8(rand_coefficients) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(rand_coefficients);
			any_failed = true;
		}
	}
	return !any_failed;

}