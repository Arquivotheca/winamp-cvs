#include <stdio.h>
#include <windows.h>
#include "../h264dec/dec_api.h"

extern bool test_inverse4x4();
extern bool test_itrans4x4();
extern bool test_inverse8x8();
extern bool test_filter_luma_horiz();
extern bool test_filter_luma_vert();
extern bool test_filter_chroma_horiz();
extern bool test_itrans8x8();
bool test_decoded_output(const char *input_filename, const char *output_filename, int mode);

int main(int argc, char *argv[])
{
	srand(GetTickCount());
	if (argc == 1)
	{/*
		printf("<<<< testing inverse 4x4 >>>>\r\n");
		printf("inverse4x4: %s\r\n", test_inverse4x4()?"passed":"failed");
		printf("<<<< testing itrans 4x4 >>>>\r\n");
		printf("itrans4x4: %s\r\n", test_itrans4x4()?"passed":"failed");
		printf("<<<< testing inverse 8x8 >>>>\r\n");
		printf("inverse8x8: %s\r\n", test_inverse8x8()?"passed":"failed");
		printf("<<<< testing itrans 8x8 >>>>\r\n");
		printf("itrans8x8: %s\r\n", test_itrans8x8()?"passed":"failed");*/
		//printf("<<<< testing horizontal luma filter >>>>\r\n");
		//printf("horizontal luma filter: %s\r\n", test_filter_luma_horiz()?"passed":"failed");
		//printf("<<<< testing vertical luma filter >>>>\r\n");
		//printf("vertical luma filter: %s\r\n", test_filter_luma_vert()?"passed":"failed");
		//printf("<<<< testing horizontal chroma filter >>>>\r\n");
		//printf("horizontal chroma filter: %s\r\n", test_filter_chroma_horiz()?"passed":"failed");
	}
	if (argc == 2)
	{
		test_decoded_output(argv[1], 0, 1);
	}
	else if (argc == 3)
	{
		test_decoded_output(argv[1], argv[2], 2);
	}
}