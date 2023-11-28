#if 0
#include <stdio.h>
#include <windows.h>
#include <emmintrin.h>
#include "../h264dec/dec_api.h"
namespace conformance_filter_luma_horiz {
#include "../h264dec/ldecod/src/filter_luma_horiz.c"
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

static bool test_filter(const conformance_image p, const conformance_image q, int64_t &times_tested)
{
	bool any_failed=false;
	int inc_dim = sizeof(p[0]);
	conformance_image p1, q1, p2, q2;
	for (int alpha = 0; alpha<52;alpha++)
	{
		__m128i xmm_alpha = _mm_set1_epi16((uint16_t)conformance_filter_luma_horiz::ALPHA_TABLE[alpha]);
		for (int beta = 0; beta<52;beta++)
		{
			__m128i xmm_beta = _mm_set1_epi16((uint16_t)conformance_filter_luma_horiz::BETA_TABLE[beta]);
			memcpy(p1, p, sizeof(conformance_image));
			memcpy(q1, q, sizeof(conformance_image));
			memcpy(p2, p, sizeof(conformance_image));
			memcpy(q2, q, sizeof(conformance_image));

			for (int row=0;row<250;row++)
			{
				bool this_failed=false;
				for (int column=0;column<256;column+=8)
				{
					int strengths[2] = { rand()%4, rand()%4 };
					const byte *ClipTab = conformance_filter_luma_horiz::CLIP_TAB[alpha];

					__m128i xmm_strength;
					int match1a = conformance_filter_luma_horiz::CalculateMatches(inc_dim, &p1[row+2][column], &q1[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);
					int match1b = conformance_filter_luma_horiz::CalculateMatches(inc_dim, &p1[row+2][column+4], &q1[row][column+4], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);
					int match2=(conformance_filter_luma_horiz::CalculateMatches_sse2(inc_dim, &p2[row+2][column], &q2[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta], &xmm_strength));

					int match1 = (match1b << 8) + match1a;

					if (match1 != match2)
					{
						printf(" *** absolute difference comparisons don't match *** \r\n");
						any_failed=true;
					}

					conformance_filter_luma_horiz::FilterLuma_Horiz(inc_dim, &p1[row+2][column], &q1[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta], ClipTab[strengths[0]], 255);

					conformance_filter_luma_horiz::FilterLuma_Horiz(inc_dim, &p1[row+2][column+4], &q1[row][column+4], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta], ClipTab[strengths[1]], 255);

					if (match2)
					{
						times_tested++;
						int C[2] = {ClipTab[strengths[0]], ClipTab[strengths[1]]};
						conformance_filter_luma_horiz::FilterLuma_Horiz_sse2(inc_dim, &p2[row+2][column], &q2[row][column],
							xmm_beta, C, xmm_strength);
					}

					for (int x = 0;x<8;x++)
					{
						if (match_results(inc_dim, &p1[row][column+x], &p2[row][column+x], 2) == false)
						{
							printf(" *** p pixels don't match *** \r\n");
							this_failed = true;
						}

						if (match_results(inc_dim, &q1[row][column+x], &q2[row][column+x], 2) == false)
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


static bool test_strong_filter(const conformance_image p, const conformance_image q, int64_t &times_tested)
{
	bool any_failed=false;
	int inc_dim = sizeof(p[0]);
	conformance_image p1, q1, p2, q2;
	for (int alpha = 0; alpha<52;alpha++)
	{
		__m128i xmm_alpha = _mm_set1_epi16((uint16_t)conformance_filter_luma_horiz::ALPHA_TABLE[alpha]);
		for (int beta = 0; beta<52;beta++)
		{
			__m128i xmm_beta = _mm_set1_epi16((uint16_t)conformance_filter_luma_horiz::BETA_TABLE[beta]);
			memcpy(p1, p, sizeof(conformance_image));
			memcpy(q1, q, sizeof(conformance_image));
			memcpy(p2, p, sizeof(conformance_image));
			memcpy(q2, q, sizeof(conformance_image));

			for (int row=0;row<250;row++)
			{
				for (int column=0;column<256;column+=8)
				{
					__m128i xmm_strength;
					int match1a = conformance_filter_luma_horiz::CalculateMatches(inc_dim, &p1[row+3][column], &q1[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);
					int match1b = conformance_filter_luma_horiz::CalculateMatches(inc_dim, &p1[row+3][column+4], &q1[row][column+4], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);
					int match2=(conformance_filter_luma_horiz::CalculateMatches_sse2(inc_dim, &p2[row+3][column], &q2[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta], &xmm_strength));

					int match1 = (match1b << 8) + match1a;

					if (match1 != match2)
					{
						printf(" *** absolute difference comparisons don't match *** \r\n");
						any_failed=true;
					}

					conformance_filter_luma_horiz::IntraStrongFilter_Luma_Horiz(inc_dim, &p1[row+3][column], &q1[row][column], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);

					conformance_filter_luma_horiz::IntraStrongFilter_Luma_Horiz(inc_dim, &p1[row+3][column+4], &q1[row][column+4], 
						conformance_filter_luma_horiz::ALPHA_TABLE[alpha], conformance_filter_luma_horiz::BETA_TABLE[beta]);

					if (match2)
					{
						times_tested++;
						conformance_filter_luma_horiz::IntraStrongFilter_Luma_Horiz_sse2(inc_dim, &p2[row+3][column], &q2[row][column], xmm_alpha, xmm_beta, xmm_strength);
					}

					for (int x = 0;x<4;x++)
					{
						if (match_results(inc_dim, &p1[row][column+x], &p2[row][column+x], 3) == false)
						{
							printf(" *** p pixels don't match *** \r\n");
							any_failed = true;
						}

						if (match_results(inc_dim, &q1[row][column+x], &q2[row][column+x], 3) == false)
						{
							printf(" *** q pixels don't match *** \r\n");
							any_failed = true;
						}
					}
				}
			}
		}
	}
	return !any_failed;
}


bool test_filter_luma_horiz()
{
	int64_t times_tested=0;
	bool any_failed=false;
	conformance_image p, q;
	printf("    [testing random pixels]\r\n");
	for (int i=0;i<5;i++)
	{
		fill_image_random(p, 1<<(i+1));
		adjust_image_random(q,p, 1<<(i+1));

		if (test_strong_filter(p, q, times_tested) == false)
		{
			any_failed=true;
		}

		if (test_filter(p, q, times_tested) == false)
		{
			any_failed=true;
		}
	}
	printf("number tested: %I64u\r\n", times_tested);
	return !any_failed;
}
#endif