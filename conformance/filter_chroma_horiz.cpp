#if 0
#include <stdio.h>
#include <windows.h>
#include <emmintrin.h>
#include "../h264dec/dec_api.h"
namespace conformance_filter_chroma_horiz {
#include "../h264dec/ldecod/src/filter_chroma_horiz.c"
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
		a+=inc_dim;
		b+=inc_dim;
	}
	return true;
}

static bool test_filter(const conformance_image p, const conformance_image q)
{
	bool any_failed=false;
	int inc_dim = sizeof(p[0]);
	conformance_image p1, q1, p2, q2;
	for (int alpha = 0; alpha<52;alpha++)
	{
		for (int beta = 0; beta<52;beta++)
		{
			memcpy(p1, p, sizeof(conformance_image));
			memcpy(q1, q, sizeof(conformance_image));
			memcpy(p2, p, sizeof(conformance_image));
			memcpy(q2, q, sizeof(conformance_image));

			for (int row=0;row<250;row++)
			{
				bool this_failed=false;
				for (int column=0;column<256;column+=8)
				{
					byte strengths[16] = { rand()%4, 0, 0, 0, rand()%4, 0, 0, 0, rand()%4, 0, 0, 0, rand()%4, };
					for (int i=0;i<16;i+=4)
					{
						for (int j=1;j<4;j++)
						{
							strengths[i+j] = strengths[i];
						}
					}

					const byte *ClipTab = conformance_filter_chroma_horiz::CLIP_TAB[alpha];

					conformance_filter_chroma_horiz::FilterChroma8_Horiz(256, &p1[row+1][column], &q1[row][column], 
						strengths, ClipTab, conformance_filter_chroma_horiz::ALPHA_TABLE[alpha], conformance_filter_chroma_horiz::BETA_TABLE[beta], 1, 255);

					conformance_filter_chroma_horiz::FilterChroma8_Horiz_sse2(256, &p2[row+1][column], &q2[row][column], 
						strengths, ClipTab, conformance_filter_chroma_horiz::ALPHA_TABLE[alpha], conformance_filter_chroma_horiz::BETA_TABLE[beta], 1, 255);

					for (int x = 0;x<8;x++)
					{
						if (match_results(inc_dim, &p1[row][column+x], &p2[row][column+x], 1) == false)
						{
							printf(" *** p pixels don't match *** \r\n");
							this_failed = true;
						}

						if (match_results(inc_dim, &q1[row][column+x], &q2[row][column+x], 1) == false)
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


static bool test_strong_filter(const conformance_image p, const conformance_image q)
{
	bool any_failed=false;
	int inc_dim = sizeof(p[0]);
	conformance_image p1, q1, p2, q2;
	for (int alpha = 0; alpha<52;alpha++)
	{
		for (int beta = 0; beta<52;beta++)
		{
			memcpy(p1, p, sizeof(conformance_image));
			memcpy(q1, q, sizeof(conformance_image));
			memcpy(p2, p, sizeof(conformance_image));
			memcpy(q2, q, sizeof(conformance_image));

			for (int row=0;row<250;row++)
			{
				bool this_failed=false;
				for (int column=0;column<256;column+=8)
				{
					byte strengths[16];
					memset(strengths, 4, 16);


					const byte *ClipTab = conformance_filter_chroma_horiz::CLIP_TAB[alpha];

					conformance_filter_chroma_horiz::FilterChroma8_Horiz(256, &p1[row+1][column], &q1[row][column], 
						strengths, ClipTab, conformance_filter_chroma_horiz::ALPHA_TABLE[alpha], conformance_filter_chroma_horiz::BETA_TABLE[beta], 1, 255);

					conformance_filter_chroma_horiz::FilterChroma8_Horiz_sse2(256, &p2[row+1][column], &q2[row][column], 
						strengths, ClipTab, conformance_filter_chroma_horiz::ALPHA_TABLE[alpha], conformance_filter_chroma_horiz::BETA_TABLE[beta], 1, 255);

					for (int x = 0;x<8;x++)
					{
						if (match_results(inc_dim, &p1[row][column+x], &p2[row][column+x], 1) == false)
						{
							printf(" *** p pixels don't match *** \r\n");
							this_failed = true;
						}

						if (match_results(inc_dim, &q1[row][column+x], &q2[row][column+x], 1) == false)
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


bool test_filter_chroma_horiz()
{

	bool any_failed=false;
	conformance_image p, q;
	printf("    [testing random pixels]\r\n");
	for (int i=0;i<5;i++)
	{
		fill_image_random(p, 1<<(i+1));
		adjust_image_random(q,p, 1<<(i+1));

		if (test_strong_filter(p, q) == false)
		{
			any_failed=true;
		}

		if (test_filter(p, q) == false)
		{
			any_failed=true;
		}
	}

	return !any_failed;
}
#endif