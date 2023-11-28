#if 0
#include <stdio.h>
#include <windows.h>
#include <emmintrin.h>
#include "../h264dec/dec_api.h"
namespace conformance_filter_luma_vert {
	extern "C"
	{
#include "../h264dec/ldecod/src/filter_luma_vert.c"
	}
}

typedef imgpel conformance_image[256][256];

static void fill_image_random(conformance_image image, int spread)
{
	int value = rand() % 256;
	for (int i=0;i<256;i++)
	{
		for (int j=0;j<256;j++)
		{
			int adjustment = rand()%spread;
			int new_value = value+adjustment;
			new_value = min(255,  new_value);
			new_value = max(0,  new_value);
			image[i][j] = new_value;
		}
	}
}

static void adjust_image_random(conformance_image image, const conformance_image source, int spread)
{
	for (int i=0;i<256;i++)
	{
		for (int j=0;j<256;j++)
		{
			int adjustment = rand()%spread;
			int new_value = source[i][j]+adjustment;
			new_value = min(255,  new_value);
			new_value = max(0,  new_value);
			image[i][j] = new_value;
		}
	}
}

static bool match_results(int inc_dim, const imgpel *a, const imgpel *b, int count)
{
	for (int i=0;i<count;i++)
	{
		if (*a != *b)
			return false;
		a++;
		b++;
	}
	return true;
}

static bool test_filter(const conformance_image p)
{
	bool any_failed=false;
	int inc_dim = sizeof(p[0]);
	conformance_image p1, p2;
	for (int alpha = 0; alpha<52;alpha++)
	{
		for (int beta = 0; beta<52;beta++)
		{
again:
			memcpy(p1, p, sizeof(conformance_image));
			memcpy(p2, p, sizeof(conformance_image));

				for (int column=0;column<250;column++)
				{
							bool this_failed=false;
		
			for (int row=0;row<256;row+=16)
			{
							uint8_t strengths[4] = { rand()%4, rand()%4, rand()%4, rand()%4 };
					const byte *ClipTab = conformance_filter_luma_vert::CLIP_TAB[alpha];


					for (int i=0;i<4;i++)
					{
						if (strengths[i])
					conformance_filter_luma_vert::FilterLuma_Vert(inc_dim, &p1[row+i*4][column+2], &p1[row+i*4][column+3], 
						conformance_filter_luma_vert::ALPHA_TABLE[alpha], conformance_filter_luma_vert::BETA_TABLE[beta], ClipTab[strengths[i]], 255);
					}


					conformance_filter_luma_vert::FilterLuma_Vert_sse2(inc_dim, &p2[row][column+2], 
						conformance_filter_luma_vert::ALPHA_TABLE[alpha], conformance_filter_luma_vert::BETA_TABLE[beta], strengths, ClipTab);

					for (int x = 0;x<16;x++)
					{
						if (match_results(inc_dim, &p1[row+x][column], &p2[row+x][column], 2) == false)
						{
							printf(" *** p pixels don't match *** \r\n");
							this_failed = true;

						}

						if (match_results(inc_dim, &p1[row+x][column+3], &p2[row+x][column+3], 2) == false)
						{
							printf(" *** q pixels don't match *** \r\n");
							this_failed = true;
						}
					}
				}


				if (this_failed)
				{
					any_failed=true;
					// once we've failed, the failures will cascade (since the filter modifies data as it runs)
					// so we need to break out of the for(row) loop so we get fresh data via memcpy
					break;
				}

			}
		}
	}

	return !any_failed;
}


bool test_filter_luma_vert()
{
	bool any_failed=false;
	conformance_image p;
	printf("    [testing random pixels]\r\n");
	for (int i=0;i<5;i++)
	{
		fill_image_random(p, 1<<(i+1));

		if (test_filter(p) == false)
		{
			any_failed=true;
		}
	}
	
	return !any_failed;
}
#endif