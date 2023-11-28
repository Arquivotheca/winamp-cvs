#include <stdio.h>
#include <windows.h>
#include "../h264dec/dec_api.h"
namespace conformance_itrans4x4 {
#include "../h264dec/lcommon/src/transform.c"
}

#if 0
static h264_int_macroblock_t coefficients = {
{272, 583, 394, 1595, 1832, 1044, 41, 1417, 507, 674, 937, 212, 1281, 30, 1997, 1501, },
{966, 881, 812, 1777, 1330, 1651, 1171, 701, 1038, 1115, 1957, 1636, 196, 1147, 1220, 1487, },
{102, 1322, 53, 1605, 1266, 769, 1150, 1851, 725, 74, 1043, 1389, 1965, 181, 1586, 1069, },
{1619, 1357, 780, 251, 1049, 1195, 805, 1367, 848, 873, 1533, 723, 1806, 218, 1008, 1388, },
{1070, 1016, 1883, 26, 919, 1182, 256, 1958, 450, 1272, 940, 1980, 1910, 1847, 791, 1601, },
{2006, 905, 523, 230, 1950, 584, 968, 254, 426, 1649, 1192, 397, 1208, 1113, 1535, 929, },
{1646, 155, 325, 1253, 1951, 853, 1655, 1139, 1482, 916, 442, 490, 1507, 1804, 1090, 704, },
{1110, 525, 1138, 474, 1610, 946, 324, 1370, 34, 604, 1260, 1721, 1354, 1118, 696, 277, },
{1328, 250, 1083, 460, 400, 1421, 426, 1865, 501, 774, 390, 542, 125, 1948, 891, 1107, },
{988, 193, 649, 766, 675, 337, 609, 1556, 963, 16, 1042, 639, 76, 595, 1508, 625, },
{635, 1789, 644, 2039, 1524, 1453, 866, 1489, 1356, 1590, 600, 641, 202, 1104, 1676, 418, },
{623, 909, 151, 1915, 819, 1934, 487, 724, 1426, 1142, 865, 264, 327, 927, 1867, 92, },
{1112, 140, 618, 1167, 1362, 543, 103, 435, 726, 1036, 1912, 315, 1108, 1166, 1852, 1364, },
{23, 601, 1766, 1657, 642, 1743, 1693, 1857, 665, 118, 35, 636, 1987, 426, 693,1919, },
{1229, 1935, 1587, 188, 1075, 1354, 385, 150, 411, 1905, 45, 115, 1187, 959, 850, 1554, },
{220, 269, 444, 799, 1814, 893, 844, 772, 991, 249, 926, 771, 1862, 476, 2026, 129, },
};

static void print_4x4_results(const h264_imgpel_macroblock_t output, int pos_y, int pos_x)
{
		for (int i=0;i<4;i++)
	{
		for (int j=0;j<4;j++)
		{
			printf("%03u ", output[i+pos_y][j+pos_x]);
		}
		printf("\r\n");
	}
}

static bool match_4x4_results(const h264_imgpel_macroblock_t a,const h264_imgpel_macroblock_t b,int pos_y, int pos_x)
{
		for (int i=0;i<4;i++)
	{
		for (int j=0;j<4;j++)
		{
			if (a[i+pos_y][j+pos_x] != b[i+pos_y][j+pos_x])
				return false;
		}
	}
		return true;
}
extern "C" void itrans4x4_sse2(const h264_int_macroblock_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y);
static bool test_4x4(const h264_int_macroblock_t coefficients, const h264_imgpel_macroblock_t predictors)
{
	bool any_failed=false;
		h264_imgpel_macroblock_t output1, output2;
		__declspec(align(32)) h264_int_macroblock_t coefficients_output;
		for (int y=0;y<12;y++)
		for (int x=0;x<12;x+=4)
		{
			/*conformance_itrans4x4::*/itrans4x4_sse2(coefficients, predictors, output2, x, y);
			conformance_itrans4x4::inverse4x4(coefficients,coefficients_output,y,x);
			conformance_itrans4x4::sample_reconstruct(output1, predictors, coefficients_output, y, x, 255);
			
			if (!match_4x4_results(output1, output2, y, x))
			{
				printf(" *** conformance failed ***\r\n");
				printf("reference: \r\n");
				print_4x4_results(output1, y, x);
				printf("optimized: \r\n");
				print_4x4_results(output2, y, x);
				any_failed=true;
			}
		}
		return !any_failed;
}

static void print_predictors(const h264_imgpel_macroblock_t coefficients)
{
for (int i=0;i<16;i++)
	{
		printf("{");
		for (int j=0;j<16;j++)
		{
			printf("0x%02x,", coefficients[i][j]);
		}
		printf("},\r\n");
	}

}
static void print_coefficients(const h264_int_macroblock_t coefficients)
{
for (int i=0;i<16;i++)
	{
		printf("{");
		for (int j=0;j<16;j++)
		{
			printf("0x%04x, ", coefficients[i][j]);
		}
		printf("},\r\n");
	}
}
static void generate_random_coefficients(h264_int_macroblock_t coefficients)
{
	for (int i=0;i<16;i++)
	{
		for (int j=0;j<16;j++)
		{
			coefficients[i][j] = rand()%8192;
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

bool test_itrans4x4()
{
	
	bool any_failed=false;
	__declspec(align(32)) h264_int_macroblock_t rand_coefficients;
	h264_imgpel_macroblock_t rand_predictors;
	/*
	printf("    [testing known coefficients]\r\n");
	if (test_4x4(coefficients) == false)
	{
		printf(" *** coefficients for failure: ***\r\n");
		print_coefficients(coefficients);
		any_failed = true;
	}
*/
	printf("    [testing random coefficients]\r\n");
	for (int i=0;i<100;i++)
	{
		generate_random_coefficients(rand_coefficients);
		generate_random_predictors(rand_predictors);
		if (test_4x4(rand_coefficients, rand_predictors) == false)
		{
			printf(" *** coefficients for failure: ***\r\n");
			print_coefficients(rand_coefficients);
			printf(" *** predictors for failure: ***\r\n");
			print_predictors(rand_predictors);
			any_failed = true;
		}
	}
	return !any_failed;

	return true;

}
#endif