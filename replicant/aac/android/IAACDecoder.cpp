#include "IAACDecoder.h"
#include "PVMP4.h"
#include "FDKAACDecoder.h"
#include "foundation/error.h"
#include <dlfcn.h>
#include <android/log.h>

#include <new>
static bool AACDecoderTest();
static void *aaclib;
int aaclib_kind=NO_AAC;

int AACDecoder_Create(IAACDecoder **out_decoder, int transport)
{
	if (aaclib_kind == PV_AAC)
	{
		PVAACDecoder *decoder = new (std::nothrow) PVAACDecoder;
		if (!decoder)
			return NErr_OutOfMemory;

		int ret = decoder->Initialize();
		if (ret != NErr_Success)
			return ret;

		*out_decoder = decoder;
		return NErr_Success;
	}
	else if (aaclib_kind == FDK_AAC)
	{
				FDKAACDecoder *decoder = new (std::nothrow) FDKAACDecoder;
		if (!decoder)
			return NErr_OutOfMemory;

		int ret = decoder->Initialize(transport);
		if (ret != NErr_Success)
			return ret;

		*out_decoder = decoder;
		return NErr_Success;
	}
	else
		return NErr_NoMatchingImplementation;


}
static bool LoadFunctions_PV(const char *filename)
{
	aaclib = dlopen(filename, RTLD_NOW);

	if (aaclib)
	{
		__AACDecoderGetMemoryRequirements = (GETMEMFUNC)dlsym(aaclib, "PVMP4AudioDecoderGetMemRequirements");
		__AACDecoderInit=(AACIPPFUNC)dlsym(aaclib, "PVMP4AudioDecoderInitLibrary");
		__AACDecoderDecode=(AACIPPFUNC)dlsym(aaclib, "PVMP4AudioDecodeFrame");
		__AACDecoderConfig=(AACIPPFUNC)dlsym(aaclib, "PVMP4AudioDecoderConfig");
		__AACDecoderReset=(AACPFUNC)dlsym(aaclib, "PVMP4AudioDecoderResetBuffer");

		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] %X %X %X %X %X %X", aaclib,  __AACDecoderGetMemoryRequirements, __AACDecoderInit,__AACDecoderDecode,__AACDecoderConfig, __AACDecoderReset);

		if (!(__AACDecoderGetMemoryRequirements && __AACDecoderInit && __AACDecoderDecode && __AACDecoderConfig))
		{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Warning: Cannot find all valid AAC functions in library '%s'", filename);
			return false;
		}

		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Success: Found valid AAC functions in library '%s'", filename);
		if (AACDecoderTest())
		{
			aaclib_kind=PV_AAC;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Warning: Cannot find library '%s'", filename);
		return false;
	}
}

static bool LoadFunctions_FDK(const char *filename)
{
	aaclib = dlopen(filename, RTLD_NOW);

	if (aaclib)
	{
		__aacDecoder_Close=(AACDECODER_CLOSE)dlsym(aaclib, "aacDecoder_Close");
		__aacDecoder_ConfigRaw=(AACDECODER_CONFIGRAW)dlsym(aaclib, "aacDecoder_ConfigRaw");
		__aacDecoder_DecodeFrame=(AACDECODER_DECODEFRAME)dlsym(aaclib, "aacDecoder_DecodeFrame");
		__aacDecoder_Fill=(AACDECODER_FILL)dlsym(aaclib, "aacDecoder_Fill");
		__aacDecoder_GetStreamInfo=(AACDECODER_GETSTREAMINFO)dlsym(aaclib, "aacDecoder_GetStreamInfo");
		__aacDecoder_Open=(AACDECODER_OPEN)dlsym(aaclib, "aacDecoder_Open");	
		__aacDecoder_SetParam=(AACDECODER_SETPARAM)dlsym(aaclib, "aacDecoder_SetParam");
		

		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] %X %X %X %X %X %X %X %X", aaclib,  __aacDecoder_Close, __aacDecoder_ConfigRaw,__aacDecoder_DecodeFrame,__aacDecoder_Fill, __aacDecoder_GetStreamInfo, __aacDecoder_Open, __aacDecoder_SetParam);

		if (!(__aacDecoder_Close && __aacDecoder_ConfigRaw && __aacDecoder_DecodeFrame && __aacDecoder_Fill && __aacDecoder_GetStreamInfo && __aacDecoder_Open && __aacDecoder_SetParam))
		{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Warning: Cannot find all valid AAC functions in library '%s'", filename);
			return false;
		}

		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Success: Found valid AAC functions in library '%s'", filename);
		aaclib_kind=FDK_AAC;
		return true;

	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Warning: Cannot find library '%s'", filename);
		return false;
	}
}

int AACDecoderLibraryInit()
{
	// NXOnce might be nice, but we check this during RegisterServices now, so it's probably not necessary
	if (aaclib == (void *)-1)
		return NErr_Error;

	if (!aaclib)
	{
		if (!LoadFunctions_FDK("libstagefright_soft_aacdec.so")
			&& !LoadFunctions_PV("libstagefright.so")
			&& !LoadFunctions_PV("libomx_aacdec_sharedlibrary.so")
			&& !LoadFunctions_PV("libopencore_common.so")
			&& !LoadFunctions_PV("libstagefright_soft_aacdec.so"))
		{
			aaclib=(void *)-1;
			return NErr_Error;
		}
	}

	return NErr_Success;
}



/* code to test the AAC decoder - it's buggy on some phones */

/* decoder config */
static const uint8_t config_data[] = { 0x12,0x08,0xED,0x40 };

/* 4 frames of 440Hz sine wave */
static const uint8_t sample0[] = { 0x01,0x40,0x60,0x07 };
static const uint8_t sample1[] = {
0x01,0x24,0xB8,0xDA,0xF0,0xEB,0x42,0x43,0xAB,0x42,0x43,0xAB,0x61,0xD0,0x16,0xB7,0x0F,0xD3,0x38,0x50,
0xF2,0xD3,0x00,0x81,0x00,0x25,0x76,0x71,0x0D,0x7E,0xB2,0xD4,0xB5,0xF2,0x5D,0x53,0x85,0x4C,0x30,0xE7,
0x0A,0xD2,0xC8,0xA5,0x17,0xAB,0x54,0x1A,0xB5,0x06,0xAD,0x5A,0x80,0x81,0xEF,0x11,0xCD,0x61,0x49,0x06,
0x29,0x3D,0xC4,0xE2,0x63,0xC3,0x12,0xAD,0x8F,0xF5,0x96,0xCF,0x87,0xBE,0xFB,0x49,0x34,0x22,0xC7,0xAC,
0x21,0xCA,0x50,0x40,0x39,0xDC,0xAF,0xAA,0x27,0x02,0x09,0x2E,0x51,0x02,0x5C,0x2F,0x5E,0x9D,0xC6,0x8E,
0x66,0x2E,0x4A,0x82,0x33,0xFE,0x30,0x86,0x1B,0x31,0x74,0x09,0xC0,0x50,0x7B };
static const uint8_t sample2[] = {
0x01,0x32,0xF5,0x2D,0x44,0x41,0x50,0x10,0x50,0x02,0x12,0x2A,0xA8,0xC4,0x7D,0xDA,0xE0,0x7F,0xED,0x65,
0x9F,0x93,0x0D,0x53,0xB1,0xAC,0x81,0x02,0x84,0x4A,0x1E,0x31,0x12,0x67,0x74,0x65,0x89,0x6E,0x2F,0xAB,
0x64,0xE8,0x55,0xA6,0x9C,0x8F,0x82,0x2B,0x11,0xA0,0x62,0x24,0x07,0xF1,0x7F,0x23,0xDD,0x3F,0x8D,0x92,
0x1E,0x40,0x7C,0x7C,0x7C,0x7C,0x7C,0x7C,0x00,0x27,0x2E,0xFD,0xD3,0x9C,0xF2,0xBB,0xF0,0x27,0xD0,0xFF,
0xE9,0xA5,0x12,0x46,0x10,0x94,0xE3,0x09,0x8B,0x40,0x4A,0x02,0x44,0x93,0x9A,0xB3,0x10,0xB6,0x3B,0xC9,
0x39,0xA3,0x42,0x43,0x59,0x61,0x0A,0x4C,0x98,0x10,0xAC,0x02,0x54,0x85,0x27,0x0C,0x9E,0x38,0x60,0xE4,
0xEF };
static const uint8_t sample3[] = {
0x01,0x2E,0x15,0x2D,0x44,0xA1,0x10,0x08,0x64,0x00,0x35,0x77,0x76,0x59,0x7F,0x1D,0x29,0x29,0x8D,0x59,
0xBC,0xCD,0xAD,0x93,0x21,0x0C,0x08,0x54,0x48,0x54,0x9E,0xF1,0x9C,0x87,0x9E,0x3E,0xBC,0x4B,0x15,0xE3,
0x62,0x7C,0x0F,0xBE,0x5D,0x94,0xF7,0x2D,0x79,0x8D,0x7A,0x7B,0x64,0xCC,0xC9,0xE7,0x9E,0x79,0xE7,0x9E,
0x79,0xE7,0x9E,0x79,0xE7,0x9E,0x71,0x3C,0xF3,0xCE,0xA0,0x2E,0x3D,0xC1,0x4B,0xE9,0x15,0x28,0x80,0x62,
0x06,0x0A,0x2A,0xA0,0x04,0x0A,0x95,0x41,0xC7,0x4A,0xCD,0xDC,0x8A,0xA1,0x70,0x48,0x6A,0x9D,0x15,0x2B,
0x65,0x68,0x52,0xA9,0x40,0x46,0x93,0xA2,0xA8,0x49,0x62,0xC9,0x95,0x21,0x90,0x84,0xD9,0x02,0xB6,0x11,
0x74 };

const int16_t expected_output[] = {
-24208, -24774, -25243, -25617, -25891, -26073, -26155, -26140, -26029, -25820, -25511, -25104, -24596, -23993, -23292, -22499,
-21614, -20646, -19593, -18463, -17257, -15986, -14649, -13254, -11806, -10314,  -8775,  -7208,  -5603,  -3986,  -2347,   -706,
   946,   2589,   4226,   5840,   7437,   8998,  10532,  12014,  13456,  14839,  16172,  17429,  18630,  19750,  20796,  21755,
 22634,  23419,  24118,  24720,  25222,  25627,  25933,  26134,  26235,  26232,  26126,  25916,  25607,  25196,  24685,  24078,
 23376,  22583,  21703,  20734,  19685,  18561,  17362,  16095,  14767,  13378,  11938,  10451,   8921,   7360,   5766,   4154,
  2522,    881,   -763,  -2402,  -4035,  -5648,  -7244,  -8808, -10338, -11825, -13271, -14657, -15993, -17261, -18466, -19593,
-20649, -21616, -22506, -23302, -24008, -24620, -25135, -25550, -25870, -26082, -26194, -26203, -26111, -25912, -25615, -25215,
-24719, -24122, -23433, -22648, -21777, -20819, -19782, -18663, -17473, -16213, -14894, -13510, -12076, -10594,  -9070,  -7511,
 -5921,  -4307,  -2678,  -1038,    606,   2248,   3880,   5499,   7097,   8664,  10199,  11694,  13145,  14544,  15882,  17161,
 18371,  19512,  20574,  21555,  22453,  23260,  23979,  24602,  25130,  25558,  25887,  26111,  26236,  26258,  26174,  25988,
 25704,  25313,  24827,  24240,  23559,  22787,  21925,  20976,  19944,  18835,  17651,  16399,  15080,  13705,  12273,  10797,
  9277,   7719,   6132,   4520,   2890,   1249,   -393,  -2039,  -3672,  -5294,  -6894,  -8467, -10007, -11507, -12964, -14367,
-15717, -17001, -18221, -19366, -20437, -21429, -22335, -23152, -23881, -24514, -25054, -25491, -25830, -26067, -26203, -26234,
-26163, -25990, -25714, -25338, -24861, -24288, -23618, -22857, -22005, -21070, -20048, -18948, -17775, -16535, -15224, -13861,
-12436, -10966,  -9453,  -7903,  -6320,  -4717,  -3091,  -1457,    187,   1825,   3462,   5077,   6681,   8250,   9796,  11294,
 12752,  14159,  15514,  16803,  18028,  19182,  20263,  21261,  22177,  23004,  23747,  24387,  24940,  25391,  25746,  25992,
 26145,  26190,  26133,  25975,  25714,  25351,  24892,  24332,  23677,  22933,  22093,  21169,  20160,  19076,  17913,  16683,
 15383,  14027,  12612,  11151,   9642,   8100,   6522,   4923,   3299,   1668,     24,  -1614,  -3253,  -4872,  -6480,  -8053,
 -9604, -11111, -12576, -13992, -15354, -16652, -17891, -19051, -20144, -21155, -22084, -22921, -23676, -24334, -24897, -25362,
-25728, -25991, -26157, -26213, -26172, -26024, -25776, -25424, -24976, -24427, -23785, -23044, -22217, -21302, -20301, -19222,
-18067, -16839, -15548, -14193, -12783, -11323,  -9821,  -8277,  -6704,  -5101,  -3479,  -1845,   -201,   1443,   3080,   4705,
  6314,   7894,   9446,  10960,  12432,  13850,  15219,  16524,  17769,  18939,  20037,  21053,  21993,  22840,  23602,  24266,
 24837,  25309,  25685,  25954,  26125,  26191,  26156,  26016,  25777,  25431,  24991,  24449,  23813,  23083,  22258,  21349,
 20359,  19284,  18137,  16913,  15627,  14278,  12877,  11418,   9921,   8383,   6809,   5211,   3592,   1960,    320,  -1323,
 -2961,  -4587,  -6193,  -7775,  -9326, -10840, -12314, -13736, -15107, -16414, -17661, -18837, -19939, -20964, -21906, -22759,
-23527, -24200, -24777, -25259, -25641, -25920, -26101, -26175, -26150, -26023, -25791, -25456, -25026, -24496, -23866, -23149,
-22336, -21439, -20457, -19390, -18250, -17042, -15761, -14422, -13026, -11581, -10087,  -8555,  -6986,  -5397,  -3781,  -2153,
  -518,   1125,   2758,   4381,   5990,   7574,   9123,  10640,  12116,  13544,  14920,  16234,  17484,  18668,  19780,  20810,
 21762,  22626,  23405,  24086,  24676,  25165,  25563,  25855,  26049,  26134,  26124,  26006,  25791,  25472,  25051,  24534,
 23920,  23211,  22412,  21527,  20552,  19501,  18373,  17171,  15903,  14572,  13184,  11746,  10258,   8733,   7169,   5583,
  3970,   2344,    704,   -933,  -2569,  -4193,  -5804,  -7386,  -8949, -10468, -11952, -13383, -14771, -16090, -17357, -18545,
-19671, -20714, -21677, -22553, -23345, -24041, -24646, -25150, -25559, -25862, -26074, -26172, -26175, -26071, -25867, -25559,
-25155, -24649, -24047, -23349, -22559, -21683, -20720, -19675, -18554, -17361, -16099, -14773, -13389, -11952, -10470,  -8946,
 -7385,  -5800,  -4185,  -2560,   -918,    722,   2360,   3987,   5604,   7194,   8759,  10287,  11778,  13219,  14611,  15941,
 17215,  18415,  19546,  20598,  21571,  22458,  23259,  23964,  24577,  25095,  25513,  25829,  26046,  26159,  26169,  26078,
 25885,  25586,  25190,  24694,  24100,  23413,  22631,  21761,  20808,  19772,  18659,  17472,  16217,  14897,  13519,  12089,
 10612,   9092,   7537,   5952,   4345,   2719,   1086,   -554,  -2191,  -3818,  -5432,  -7022,  -8585, -10116, -11605, -13047,
-14440, -15775, -17049, -18256, -19391, -20447, -21428, -22320, -23126, -23843, -24465, -24988, -25418, -25744, -25971, -26095,
-26117, -26037, -25856, -25571, -25186, -24702, -24121, -23447, -22680, -21823, -20879, -19856, -18753, -17577, -16332, -15026,
-13657, -12235, -10767,  -9253,  -7703,  -6125,  -4521,  -2900,  -1267,    372,   2009,   3637,   5252,   6845,   8414,   9948,
 11442,  12892,  14293,  15636,  16918,  18135,  19279,  20349,  21338,  22244,  23063,  23788,  24423,  24963,  25403,  25741,
 25981,  26120,  26151,  26083,  25911,  25638,  25264,  24790,  24218,  23555,  22796,  21947,  21011,  19994,  18899,  17729,
 16488,  15185,  13819,  12401,  10930,   9422,   7871,   6293,   4689,   3067,   1431,   -209,  -1850,  -3483,  -5100,  -6698,
 -8272,  -9813, -11314, -12770, -14178, -15531, -16821, -18045, -19201, -20278, -21276, -22192, -23023, -23760, -24406, -24954,
-25405, -25755, -26007, -26154, -26198, -26140, -25980, -25715, -25355, -24889, -24328, -23671, -22923, -22083, -21157, -20146,
-19058, -17897, -16664, -15366, -14004, -12590, -11125,  -9618,  -8071,  -6494,  -4889,  -3268,  -1631,     10,   1652,   3285,
  4909,   6510,   8088,   9632,  11139,  12602,  14018,  15376,  16676,  17910,  19075,  20161,  21173,  22099,  22938,  23685,
 24343,  24902,  25366,  25727,  25987,  26149,  26205,  26159,  26012,  25760,  25411,  24958,  24410,  23766,  23028,  22198,
 21282,  20282,  19202,  18047,  16821,  15528,  14176,  12767,  11308,   9802,   8262,   6684,   5085,   3462,   1831,    188,
 -1455,  -3091,  -4713,  -6319,  -7899,  -9450, -10961, -12431, -13848, -15215, -16521, -17762, -18933, -20030, -21047, -21981,
-22827, -23586, -24253, -24823, -25293, -25669, -25939, -26112, -26179, -26143, -26008, -25766, -25426, -24986, -24449, -23814,
-23086, -22270, -21364, -20375, -19305, -18159, -16943, -15659, -14315, -12912, -11462,  -9964,  -8429,  -6858,  -5264,  -3645,
 -2016,   -376,   1262,   2895,   4518,   6124,   7705,   9258,  10774,  12248,  13673,  15047,  16362,  17610,  18791,  19896,
 20924,  21870,  22729,  23497,  24176,  24760,  25242,  25631,  25916,  26099,  26185,  26162,  26038,  25816,  25488,  25061,
 24534,  23914,  23195,  22389,  21490,  20511,  19449,  18312,  17104,  15826,  14488,  13093,  11646,  10152,   8620,   7054,
  5460,   3845,   2215,    576,  -1063,  -2702,  -4328,  -5938,  -7522,  -9078, -10599, -12079, -13508, -14888, -16208, -17462,
-18650, -19764, -20798, -21753, -22620, -23400, -24085, -24678, -25175, -25571, -25868, -26062, -26157, -26145, -26035, -25816,
-25499, -25085, -24568, -23961, -23251, -22455, -21571, -20602, -19548, -18421, -17223, -15953, -14623, -13234, -11794, -10307,
 -8781,  -7222,  -5634,  -4024,  -2397,   -761,    879,   2512,   4140,   5748,   7337,   8897,  10421,  11906,  13346,  14729,
 16056,  17322,  18514,  19639,  20683,  21646,  22523,  23315,  24010,  24617,  25127,  25535,  25849,  26058,  26164,  26168,
 26069,  25867,  25563,  25159,  24654,  24056,  23358,  22572,  21698,  20732,  19693,  18572,  17382,  16118,  14795,  13412,
 11979,  10496,   8971,   7412,   5822,   4210,   2582,    940,   -698,  -2338,  -3968,  -5583,  -7174,  -8737, -10266, -11756,
-13199, -14589, -15921, -17192, -18397, -19531, -20583, -21560, -22448, -23248, -23958, -24575, -25092, -25510, -25828, -26042,
-26159, -26173, -26080, -25892, -25596, -25206, -24714, -24127, -23443, -22665, -21796, -20839, -19800, -18677, -17484, -16219
};

static bool AACDecoderTest()
{
	PVAACDecoder decoder;
	int16_t decoder_output[4096];
	size_t samples_decoded;

	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Testing Decoder");

	if (decoder.Initialize(1) != NErr_Success)
	{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: initialization failed");
		return false;
	}

	if (decoder.Configure(config_data, sizeof(config_data)) != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: configure failed");
		return false;
	}
	

	if (decoder.Decode(sample0, sizeof(sample0), decoder_output, &samples_decoded) != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: frame 1 failed to decode");
		return false;
	}

	if (decoder.Decode(sample1, sizeof(sample1), decoder_output, &samples_decoded) != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: frame 2 failed to decode");
		return false;
	}

	if (decoder.Decode(sample2, sizeof(sample2), decoder_output, &samples_decoded) != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: frame 3 failed to decode");
		return false;
	}

	if (decoder.Decode(sample3, sizeof(sample3), decoder_output, &samples_decoded) != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: frame 4 failed to decode");
		return false;
	}

	for (int i=0;i<1024;i++)
	{
		int32_t delta = decoder_output[i] - expected_output[i];
		if (delta < -200 || delta > 200)
		{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Failed: sample error too high, probably distorted decoder (e.g. Droid X2)");
			return false;
		}
	}
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AAC] Test Succeeded");
	return true;
}