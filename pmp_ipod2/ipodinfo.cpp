#include <windows.h>
#include <strsafe.h>
#include <shlobj.h>
#include <stdio.h>

#include <api/service/waservicefactory.h>

#include "api.h"
#include "iPodInfo.h"
#include "resource.h"
#include "../gen_ml/ml.h"
#include "../ml_pmp/pmp.h"

#include "../xml/obj_xml.h"
#include "../plist/loader.h"

extern PMPDevicePlugin plugin;

static const ArtworkFormat ipod_color_artwork_info[] = {
	{THUMB_COVER_SMALL,        56,  56, 1017, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       140, 140, 1016, RGB_565, 4, 4},
	{THUMB_PHOTO_TV_SCREEN,   720, 480, 1019, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,       130,  88, 1015, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 220, 176, 1013, RGB_565, 4, 4},
	{THUMB_PHOTO_SMALL,        42,  30, 1009, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_nano_artwork_info[] = {
	{THUMB_COVER_SMALL,        42,  42, 1031, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       100, 100, 1027, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,        42,  37, 1032, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 176, 132, 1023, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_video_artwork_info[] = {
	{THUMB_COVER_SMALL,       100, 100, 1028, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       200, 200, 1029, RGB_565, 4, 4},
	{THUMB_PHOTO_TV_SCREEN,   720, 480, 1019, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,       130,  88, 1015, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 320, 240, 1024, RGB_565, 4, 4},
	{THUMB_PHOTO_SMALL,        50,  41, 1036, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_7g_artwork_info[] = {
	{THUMB_COVER_SMALL,        55,  55, 1061, RGB_565, 4, 4},
	{THUMB_COVER_MEDIUM1,     128, 128, 1055, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       320, 320, 1060, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_touch_artwork_info[] = {
	{THUMB_COVER_SMALL,        55,  55, 3006, RGB_555, 16, 4096},
	{THUMB_COVER_MEDIUM1,      64,  64, 3003, RGB_555_REC, 16, 4096},
	{THUMB_COVER_MEDIUM2,      88,  88, 3007, RGB_555, 16, 4096},
	{THUMB_COVER_MEDIUM3,     128, 128, 3002, RGB_555_REC, 16, 4096},
	{THUMB_COVER_MEDIUM4,     256, 256, 3001, RGB_555_REC, 16, 4096},
	{THUMB_COVER_LARGE,       320, 320, 3005, RGB_555, 16, 4096},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_555, 4, 4}
};

static const ArtworkFormat ipod_5g_nano_artwork_info[] = {
	{THUMB_COVER_SMALL,        50,  50, 1074, RGB_565, 4, 4},
	{THUMB_COVER_MEDIUM1,      80,  80, 1078, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       128, 128, 1056, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,       240, 240, 1073, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
	//{THUMB_COVER_MEDIUM2,     128, 128, 1068, RGB_565, 4, 4},
};

/*
static const ArtworkFormat ipod_mobile_1_artwork_info[] = {
	{THUMB_COVER_SMALL,        50,  50, 2002},
	{THUMB_COVER_LARGE,       150, 150, 2003},
	{THUMB_INVALID,            -1,  -1,   -1}
};
*/

//maps model to artwork format
static const ArtworkFormat *ipod_artwork_info_table[] = {
	NULL,                      // invalid
	ipod_color_artwork_info,   // color
	NULL,                      // regular
	NULL,                      // mini
	NULL,                      // shuffle
	ipod_video_artwork_info,   // video
	ipod_nano_artwork_info,    // nano
	ipod_7g_artwork_info,      // classic
	ipod_7g_artwork_info,      // fat nano
	ipod_touch_artwork_info,   // touch

	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
	ipod_5g_nano_artwork_info,    // nano
};


static const iPodInfo ipod_info_table [] = {
    /* Handle idiots who hose their iPod file system, or lucky people
       with iPods we don't yet know about*/
    {L"Invalid", 0,  ITDB_IPOD_MODEL_INVALID,     ITDB_IPOD_GENERATION_UNKNOWN, 0},
    {L"Unknown", 0,  ITDB_IPOD_MODEL_UNKNOWN,     ITDB_IPOD_GENERATION_UNKNOWN, 0},

    /* First Generation */
    /* Mechanical buttons arranged around rotating "scroll wheel".
       8513, 8541 and 8709 are Mac types, 8697 is PC */
    {L"8513",  5, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FIRST,  20},
    {L"8541",  5, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FIRST,  20},
    {L"8697",  5, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FIRST,  20},
    {L"8709", 10, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FIRST,  20},

    /* Second Generation */
    /* Same buttons as First Generation but around touch-sensitive
       "touch wheel". 8737 and 8738 are Mac types, 8740 and 8741 * are
       PC */
    {L"8737", 10, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_SECOND, 20},
    {L"8740", 10, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_SECOND, 20},
    {L"8738", 20, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_SECOND, 50},
    {L"8741", 20, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_SECOND, 50},

    /* Third Generation */
    /* Touch sensitive buttons and arranged in a line above "touch
       wheel". Docking connector was introduced here, same models for
       Mac and PC from now on. */
	{L"8976", 10, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  20},
    {L"8946", 15, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  50},
    {L"9460", 15, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  50},
    {L"9244", 20, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  50},
    {L"8948", 30, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  50},
    {L"9245", 40, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_THIRD,  50},

    /* Fourth Generation */
    /* Buttons are now integrated into the "touch wheel". */
    {L"9282", 20, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FOURTH, 50},
    {L"9787", 25, ITDB_IPOD_MODEL_REGULAR_U2,  ITDB_IPOD_GENERATION_FOURTH, 50},
    {L"9268", 40, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FOURTH, 50},

    /* First Generation Mini */
    {L"9160",  4, ITDB_IPOD_MODEL_MINI,        ITDB_IPOD_GENERATION_MINI_1,  6},
    {L"9436",  4, ITDB_IPOD_MODEL_MINI_BLUE,   ITDB_IPOD_GENERATION_MINI_1,  6},
    {L"9435",  4, ITDB_IPOD_MODEL_MINI_PINK,   ITDB_IPOD_GENERATION_MINI_1,  6},
    {L"9434",  4, ITDB_IPOD_MODEL_MINI_GREEN,  ITDB_IPOD_GENERATION_MINI_1,  6},
    {L"9437",  4, ITDB_IPOD_MODEL_MINI_GOLD,   ITDB_IPOD_GENERATION_MINI_1,  6},	

    /* Second Generation Mini */
    {L"9800",  4, ITDB_IPOD_MODEL_MINI,        ITDB_IPOD_GENERATION_MINI_2,  6},
    {L"9802",  4, ITDB_IPOD_MODEL_MINI_BLUE,   ITDB_IPOD_GENERATION_MINI_2,  6},
    {L"9804",  4, ITDB_IPOD_MODEL_MINI_PINK,   ITDB_IPOD_GENERATION_MINI_2,  6},
    {L"9806",  4, ITDB_IPOD_MODEL_MINI_GREEN,  ITDB_IPOD_GENERATION_MINI_2,  6},
    {L"9801",  6, ITDB_IPOD_MODEL_MINI,        ITDB_IPOD_GENERATION_MINI_2, 20},
    {L"9803",  6, ITDB_IPOD_MODEL_MINI_BLUE,   ITDB_IPOD_GENERATION_MINI_2, 20},
    {L"9805",  6, ITDB_IPOD_MODEL_MINI_PINK,   ITDB_IPOD_GENERATION_MINI_2, 20},
    {L"9807",  6, ITDB_IPOD_MODEL_MINI_GREEN,  ITDB_IPOD_GENERATION_MINI_2, 20},	

    /* Photo / Fourth Generation */
    /* Buttons are integrated into the "touch wheel". */
    {L"A079", 20, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},
    {L"A127", 20, ITDB_IPOD_MODEL_COLOR_U2,    ITDB_IPOD_GENERATION_PHOTO,  50},
    {L"9829", 30, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},
    {L"9585", 40, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},
    {L"9830", 60, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},
    {L"9586", 60, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},

    /* Shuffle / Fourth Generation */
    {L"9724", 0.5,ITDB_IPOD_MODEL_SHUFFLE,     ITDB_IPOD_GENERATION_SHUFFLE_1, 3},
    {L"9725", 1,  ITDB_IPOD_MODEL_SHUFFLE,     ITDB_IPOD_GENERATION_SHUFFLE_1, 3},
    /* Shuffle / Sixth Generation */
    /* Square, connected to computer via cable */
    {L"A546", 1,  ITDB_IPOD_MODEL_SHUFFLE_SILVER, ITDB_IPOD_GENERATION_SHUFFLE_2, 3},
    {L"A947", 1,  ITDB_IPOD_MODEL_SHUFFLE_PINK,   ITDB_IPOD_GENERATION_SHUFFLE_2, 3},
    {L"A949", 1,  ITDB_IPOD_MODEL_SHUFFLE_BLUE,   ITDB_IPOD_GENERATION_SHUFFLE_2, 3},
    {L"A951", 1,  ITDB_IPOD_MODEL_SHUFFLE_GREEN,  ITDB_IPOD_GENERATION_SHUFFLE_2, 3},
    {L"A953", 1,  ITDB_IPOD_MODEL_SHUFFLE_ORANGE, ITDB_IPOD_GENERATION_SHUFFLE_2, 3},
    /* Shuffle / Seventh Generation */
    /* Square, connected to computer via cable -- look identicaly to
     * Sixth Generation*/
    {L"B225", 1,  ITDB_IPOD_MODEL_SHUFFLE_SILVER, ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B233", 1,  ITDB_IPOD_MODEL_SHUFFLE_PURPLE, ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B231", 1,  ITDB_IPOD_MODEL_SHUFFLE_RED,    ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B227", 1,  ITDB_IPOD_MODEL_SHUFFLE_BLUE,   ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B228", 1,  ITDB_IPOD_MODEL_SHUFFLE_BLUE,   ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B229", 1,  ITDB_IPOD_MODEL_SHUFFLE_GREEN,  ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B518", 2,  ITDB_IPOD_MODEL_SHUFFLE_SILVER, ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B520", 2,  ITDB_IPOD_MODEL_SHUFFLE_BLUE,   ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B522", 2,  ITDB_IPOD_MODEL_SHUFFLE_GREEN,  ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B524", 2,  ITDB_IPOD_MODEL_SHUFFLE_RED,    ITDB_IPOD_GENERATION_SHUFFLE_3, 3},
    {L"B526", 2,  ITDB_IPOD_MODEL_SHUFFLE_PURPLE, ITDB_IPOD_GENERATION_SHUFFLE_3, 3},

    /* Shuffle / Eigth Generation */
    /* Bar, button-less, speaking */
    {L"B867", 4,  ITDB_IPOD_MODEL_SHUFFLE_SILVER, ITDB_IPOD_GENERATION_SHUFFLE_4, 3},
    {L"C164", 4,  ITDB_IPOD_MODEL_SHUFFLE_BLACK,  ITDB_IPOD_GENERATION_SHUFFLE_4, 3},

    /* Nano / Fifth Generation (first nano generation) */
    /* Buttons are integrated into the "touch wheel". */
    {L"A350",  1, ITDB_IPOD_MODEL_NANO_WHITE,  ITDB_IPOD_GENERATION_NANO_1,   3},
    {L"A352",  1, ITDB_IPOD_MODEL_NANO_BLACK,  ITDB_IPOD_GENERATION_NANO_1,   3},
    {L"A004",  2, ITDB_IPOD_MODEL_NANO_WHITE,  ITDB_IPOD_GENERATION_NANO_1,   3},
    {L"A099",  2, ITDB_IPOD_MODEL_NANO_BLACK,  ITDB_IPOD_GENERATION_NANO_1,   3},
    {L"A005",  4, ITDB_IPOD_MODEL_NANO_WHITE,  ITDB_IPOD_GENERATION_NANO_1,   6},
    {L"A107",  4, ITDB_IPOD_MODEL_NANO_BLACK,  ITDB_IPOD_GENERATION_NANO_1,   6},

    /* Video / Fifth Generation */
    /* Buttons are integrated into the "touch wheel". */
    {L"A002", 30, ITDB_IPOD_MODEL_VIDEO_WHITE, ITDB_IPOD_GENERATION_VIDEO_1,  50},
    {L"A146", 30, ITDB_IPOD_MODEL_VIDEO_BLACK, ITDB_IPOD_GENERATION_VIDEO_1,  50},
    {L"A003", 60, ITDB_IPOD_MODEL_VIDEO_WHITE, ITDB_IPOD_GENERATION_VIDEO_1,  50},
    {L"A147", 60, ITDB_IPOD_MODEL_VIDEO_BLACK, ITDB_IPOD_GENERATION_VIDEO_1,  50},
    {L"A452", 30, ITDB_IPOD_MODEL_VIDEO_U2,    ITDB_IPOD_GENERATION_VIDEO_1,  50},

    /* Video / Sixth Generation */
    /* Pretty much identical to fifth generation with better display,
     * extended battery operation time and gap-free playback */
    {L"A444", 30, ITDB_IPOD_MODEL_VIDEO_WHITE, ITDB_IPOD_GENERATION_VIDEO_2,  50},
    {L"A446", 30, ITDB_IPOD_MODEL_VIDEO_BLACK, ITDB_IPOD_GENERATION_VIDEO_2,  50},
    {L"A664", 30, ITDB_IPOD_MODEL_VIDEO_U2,    ITDB_IPOD_GENERATION_VIDEO_2,  50},
    {L"A448", 80, ITDB_IPOD_MODEL_VIDEO_WHITE, ITDB_IPOD_GENERATION_VIDEO_2,  50},
    {L"A450", 80, ITDB_IPOD_MODEL_VIDEO_BLACK, ITDB_IPOD_GENERATION_VIDEO_2,  50},

    /* Nano / Sixth Generation (second nano generation) */
    /* Pretty much identical to fifth generation with better display,
     * extended battery operation time and gap-free playback */
    {L"A477",  2, ITDB_IPOD_MODEL_NANO_SILVER, ITDB_IPOD_GENERATION_NANO_2,   3},
    {L"A426",  4, ITDB_IPOD_MODEL_NANO_SILVER, ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A428",  4, ITDB_IPOD_MODEL_NANO_BLUE,   ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A487",  4, ITDB_IPOD_MODEL_NANO_GREEN,  ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A489",  4, ITDB_IPOD_MODEL_NANO_PINK,   ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A725",  4, ITDB_IPOD_MODEL_NANO_RED,    ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A726",  8, ITDB_IPOD_MODEL_NANO_RED,    ITDB_IPOD_GENERATION_NANO_2,   6},
    {L"A497",  8, ITDB_IPOD_MODEL_NANO_BLACK,  ITDB_IPOD_GENERATION_NANO_2,  14},

    /* HP iPods, need contributions for this table */
    /* Buttons are integrated into the "touch wheel". */
    {L"E436", 40, ITDB_IPOD_MODEL_REGULAR,     ITDB_IPOD_GENERATION_FOURTH, 50},
    {L"S492", 30, ITDB_IPOD_MODEL_COLOR,       ITDB_IPOD_GENERATION_PHOTO,  50},

    /* iPod Classic G1 */
    /* First generation with "cover flow" */
    {L"B029",  80, ITDB_IPOD_MODEL_CLASSIC_SILVER, ITDB_IPOD_GENERATION_CLASSIC_1, 50},
    {L"B147",  80, ITDB_IPOD_MODEL_CLASSIC_BLACK,  ITDB_IPOD_GENERATION_CLASSIC_1, 50},
    {L"B145", 160, ITDB_IPOD_MODEL_CLASSIC_SILVER, ITDB_IPOD_GENERATION_CLASSIC_1, 50},
    {L"B150", 160, ITDB_IPOD_MODEL_CLASSIC_BLACK,  ITDB_IPOD_GENERATION_CLASSIC_1, 50},

    /* iPod Classic G2 */
    {L"B562", 120, ITDB_IPOD_MODEL_CLASSIC_SILVER, ITDB_IPOD_GENERATION_CLASSIC_2, 50},
    {L"B565", 120, ITDB_IPOD_MODEL_CLASSIC_BLACK,  ITDB_IPOD_GENERATION_CLASSIC_2, 50},

    /* iPod Classic G3 */
    {L"C293", 160, ITDB_IPOD_MODEL_CLASSIC_SILVER, ITDB_IPOD_GENERATION_CLASSIC_3, 50},
    {L"C297", 160, ITDB_IPOD_MODEL_CLASSIC_BLACK,  ITDB_IPOD_GENERATION_CLASSIC_3, 50},

    /* iPod nano video G1 (Third Nano Generation) */
    /* First generation of video support for nano */
    {L"A978",   4, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_3,  6},
    {L"A980",   8, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_3, 14},
    {L"B261",   8, ITDB_IPOD_MODEL_NANO_BLACK,     ITDB_IPOD_GENERATION_NANO_3, 14},
    {L"B249",   8, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_3, 14},
    {L"B253",   8, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_3, 14},
    {L"B257",   8, ITDB_IPOD_MODEL_NANO_RED,       ITDB_IPOD_GENERATION_NANO_3, 14},

    /* iPod nano video G2 (Fourth Nano Generation) */
    {L"B480",   4, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B651",   4, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B654",   4, ITDB_IPOD_MODEL_NANO_PINK,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B657",   4, ITDB_IPOD_MODEL_NANO_PURPLE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B660",   4, ITDB_IPOD_MODEL_NANO_ORANGE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B663",   4, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B666",   4, ITDB_IPOD_MODEL_NANO_YELLOW,    ITDB_IPOD_GENERATION_NANO_4, 14},

    {L"B598",   8, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B732",   8, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B735",   8, ITDB_IPOD_MODEL_NANO_PINK,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B739",   8, ITDB_IPOD_MODEL_NANO_PURPLE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B742",   8, ITDB_IPOD_MODEL_NANO_ORANGE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B745",   8, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B748",   8, ITDB_IPOD_MODEL_NANO_YELLOW,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B751",   8, ITDB_IPOD_MODEL_NANO_RED,       ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B754",   8, ITDB_IPOD_MODEL_NANO_BLACK,     ITDB_IPOD_GENERATION_NANO_4, 14},

    {L"B903",  16, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B905",  16, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B907",  16, ITDB_IPOD_MODEL_NANO_PINK,      ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B909",  16, ITDB_IPOD_MODEL_NANO_PURPLE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B911",  16, ITDB_IPOD_MODEL_NANO_ORANGE,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B913",  16, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B915",  16, ITDB_IPOD_MODEL_NANO_YELLOW,    ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B917",  16, ITDB_IPOD_MODEL_NANO_RED,       ITDB_IPOD_GENERATION_NANO_4, 14},
    {L"B918",  16, ITDB_IPOD_MODEL_NANO_BLACK,     ITDB_IPOD_GENERATION_NANO_4, 14},

    /* iPod nano with camera (Fifth Nano Generation) */
    {L"C027",   8, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C031",   8, ITDB_IPOD_MODEL_NANO_BLACK,     ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C034",   8, ITDB_IPOD_MODEL_NANO_PURPLE,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C037",   8, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C040",   8, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C043",   8, ITDB_IPOD_MODEL_NANO_YELLOW,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C046",   8, ITDB_IPOD_MODEL_NANO_ORANGE,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C049",   8, ITDB_IPOD_MODEL_NANO_RED,       ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C050",   8, ITDB_IPOD_MODEL_NANO_PINK,      ITDB_IPOD_GENERATION_NANO_5, 14},

    {L"C060",  16, ITDB_IPOD_MODEL_NANO_SILVER,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C062",  16, ITDB_IPOD_MODEL_NANO_BLACK,     ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C064",  16, ITDB_IPOD_MODEL_NANO_PURPLE,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C066",  16, ITDB_IPOD_MODEL_NANO_BLUE,      ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C068",  16, ITDB_IPOD_MODEL_NANO_GREEN,     ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C070",  16, ITDB_IPOD_MODEL_NANO_YELLOW,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C072",  16, ITDB_IPOD_MODEL_NANO_ORANGE,    ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C074",  16, ITDB_IPOD_MODEL_NANO_RED,       ITDB_IPOD_GENERATION_NANO_5, 14},
    {L"C075",  16, ITDB_IPOD_MODEL_NANO_PINK,      ITDB_IPOD_GENERATION_NANO_5, 14},

    /* iPod Touch 1st gen */
    {L"A623",   8, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_1, 50},
    {L"A627",  16, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_1, 50},
    {L"B376",  32, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_1, 50},

    /* iPod Touch 2nd gen */
    {L"B528",   8, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_2, 50},
    {L"B531",  16, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_2, 50},
    {L"B533",  32, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_2, 50},

    /* iPod Touch 3rd gen */
    /* The 8GB model is marked as 2nd gen because it's actually what the 
     * hardware is even if Apple markets it the same as the 2 bigger models
     */
    {L"C086",   8, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_2, 50},
    {L"C008",  32, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_3, 50},
    {L"C011",  64, ITDB_IPOD_MODEL_TOUCH_SILVER,   ITDB_IPOD_GENERATION_TOUCH_3, 50},

    /* iPhone, iPhone 3G and iPhone 3GS */
    {L"A501",   4, ITDB_IPOD_MODEL_IPHONE_1,       ITDB_IPOD_GENERATION_IPHONE_1, 50},
    {L"A712",   8, ITDB_IPOD_MODEL_IPHONE_1,       ITDB_IPOD_GENERATION_IPHONE_1, 50},
    {L"B384",  16, ITDB_IPOD_MODEL_IPHONE_1,       ITDB_IPOD_GENERATION_IPHONE_1, 50},
    {L"B046",   8, ITDB_IPOD_MODEL_IPHONE_BLACK,   ITDB_IPOD_GENERATION_IPHONE_2, 50},
    {L"B500",  16, ITDB_IPOD_MODEL_IPHONE_WHITE,   ITDB_IPOD_GENERATION_IPHONE_2, 50},
    {L"B048",  16, ITDB_IPOD_MODEL_IPHONE_BLACK,   ITDB_IPOD_GENERATION_IPHONE_2, 50},
    {L"B496",  16, ITDB_IPOD_MODEL_IPHONE_BLACK,   ITDB_IPOD_GENERATION_IPHONE_2, 50},
    {L"C131",  16, ITDB_IPOD_MODEL_IPHONE_BLACK,   ITDB_IPOD_GENERATION_IPHONE_3, 50},
    {L"C133",  32, ITDB_IPOD_MODEL_IPHONE_BLACK,   ITDB_IPOD_GENERATION_IPHONE_3, 50},

    /* iPhone G2 aka iPhone 3G (yeah, confusing ;) */
    {L"B500",  16, ITDB_IPOD_MODEL_IPHONE_WHITE,       ITDB_IPOD_GENERATION_IPHONE_2, 14},
    {L"B048",  16, ITDB_IPOD_MODEL_IPHONE_BLACK,       ITDB_IPOD_GENERATION_IPHONE_2, 14},

    /* No known model number -- create a Device/SysInfo file with
     * one entry, e.g.:
       ModelNumStr: Mmobile1
    */
    {L"mobile1", -1, ITDB_IPOD_MODEL_MOBILE_1, ITDB_IPOD_GENERATION_MOBILE,  6},

};


/* This table was extracted from ipod-model-table from podsleuth svn trunk
 * on 2008-06-14 (which seems to match podsleuth 0.6.2)
*/
static const iPodSerialToModel serial_to_model_mapping[] = {
    { L"LG6", L"8541" },
    { L"NAM", L"8541" },
    { L"MJ2", L"8541" },
    { L"ML1", L"8709" },
    { L"MME", L"8709" },
    { L"MMB", L"8737" },
    { L"MMC", L"8738" },
    { L"NGE", L"8740" },
    { L"NGH", L"8740" },
    { L"MMF", L"8741" },
    { L"NLW", L"8946" },
    { L"NRH", L"8976" },
    { L"QQF", L"9460" },
    { L"PQ5", L"9244" },
    { L"PNT", L"9244" },
    { L"NLY", L"8948" },
    { L"NM7", L"8948" },
    { L"PNU", L"9245" },
    { L"PS9", L"9282" },
    { L"Q8U", L"9282" },
    { L"V9V", L"9787" },
    { L"S2X", L"9787" },
    { L"PQ7", L"9268" },
    { L"TDU", L"A079" },
    { L"TDS", L"A079" },
    { L"TM2", L"A127" },
    { L"SAZ", L"9830" },
    { L"SB1", L"9830" },
    { L"SAY", L"9829" },
    { L"R5Q", L"9585" },
    { L"R5R", L"9586" },
    { L"R5T", L"9586" },
    { L"PFW", L"9160" },
    { L"PRC", L"9160" },
    { L"QKL", L"9436" },
    { L"QKQ", L"9436" },
    { L"QKK", L"9435" },
    { L"QKP", L"9435" },
    { L"QKJ", L"9434" },
    { L"QKN", L"9434" },
    { L"QKM", L"9437" },
    { L"QKR", L"9437" },
    { L"S41", L"9800" },
    { L"S4C", L"9800" },
    { L"S43", L"9802" },
    { L"S45", L"9804" },
    { L"S47", L"9806" },
    { L"S4J", L"9806" },
    { L"S42", L"9801" },
    { L"S44", L"9803" },
    { L"S48", L"9807" },
    { L"RS9", L"9724" },
    { L"QGV", L"9724" },
    { L"TSX", L"9724" },
    { L"PFV", L"9724" },
    { L"R80", L"9724" },
    { L"RSA", L"9725" },
    { L"TSY", L"9725" },
    { L"C60", L"9725" },
    { L"VTE", L"A546" },
    { L"VTF", L"A546" },
    { L"XQ5", L"A947" },
    { L"XQS", L"A947" },
    { L"XQV", L"A949" },
    { L"XQX", L"A949" },
    { L"YX7", L"A949" },
    { L"XQY", L"A951" },
    { L"YX8", L"A951" },
    { L"XR1", L"A953" },
    { L"YXA", L"B233" },
    { L"YX6", L"B225" },
    { L"YX7", L"B228" },
    { L"YX9", L"B225" },
    { L"UNA", L"A350" },
    { L"UNB", L"A350" },
    { L"UPR", L"A352" },
    { L"UPS", L"A352" },
    { L"SZB", L"A004" },
    { L"SZV", L"A004" },
    { L"SZW", L"A004" },
    { L"SZC", L"A005" },
    { L"SZT", L"A005" },
    { L"TJT", L"A099" },
    { L"TJU", L"A099" },
    { L"TK2", L"A107" },
    { L"TK3", L"A107" },
    { L"VQ5", L"A477" },
    { L"VQ6", L"A477" },
    { L"V8T", L"A426" },
    { L"V8U", L"A426" },
    { L"V8W", L"A428" },
    { L"V8X", L"A428" },
    { L"VQH", L"A487" },
    { L"VQJ", L"A487" },
    { L"VQK", L"A489" },
    { L"VKL", L"A489" },
    { L"WL2", L"A725" },
    { L"WL3", L"A725" },
    { L"X9A", L"A726" },
    { L"X9B", L"A726" },
    { L"VQT", L"A497" },
    { L"VQU", L"A497" },
    { L"Y0P", L"A978" },
    { L"Y0R", L"A980" },
    { L"YXR", L"B249" },
    { L"YXV", L"B257" },
    { L"YXT", L"B253" },
    { L"YXX", L"B261" },
    { L"SZ9", L"A002" },
    { L"WEC", L"A002" },
    { L"WED", L"A002" },
    { L"WEG", L"A002" },
    { L"WEH", L"A002" },
    { L"WEL", L"A002" },
    { L"TXK", L"A146" },
    { L"TXM", L"A146" },
    { L"WEE", L"A146" },
    { L"WEF", L"A146" },
    { L"WEJ", L"A146" },
    { L"WEK", L"A146" },
    { L"SZA", L"A003" },
    { L"SZU", L"A003" },
    { L"TXL", L"A147" },
    { L"TXN", L"A147" },
    { L"V9K", L"A444" },
    { L"V9L", L"A444" },
    { L"WU9", L"A444" },
    { L"VQM", L"A446" },
    { L"V9M", L"A446" },
    { L"V9N", L"A446" },
    { L"WEE", L"A446" },
    { L"V9P", L"A448" },
    { L"V9Q", L"A448" },
    { L"V9R", L"A450" },
    { L"V9S", L"A450" },
    { L"V95", L"A450" },
    { L"V96", L"A450" },
    { L"WUC", L"A450" },
    { L"W9G", L"A664" }, /* 30GB iPod Video U2 5.5g */
    { L"Y5N", L"B029" }, /* Silver Classic 80GB */
    { L"YMV", L"B147" }, /* Black Classic 80GB */
    { L"YMU", L"B145" }, /* Silver Classic 160GB */
    { L"YMX", L"B150" }, /* Black Classic 160GB */
    { L"2C5", L"B562" }, /* Silver Classic 120GB */
    { L"2C7", L"B565" }, /* Black Classic 120GB */
    { L"9ZS", L"C293" }, /* Silver Classic 160GB (2009) */
    { L"9ZU", L"C297" }, /* Black Classic 160GB (2009) */

    { L"37P", L"B663" }, /* 4GB Green Nano 4g */
    { L"37Q", L"B666" }, /* 4GB Yellow Nano 4g */
    { L"37H", L"B654" }, /* 4GB Pink Nano 4g */
    { L"1P1", L"B480" }, /* 4GB Silver Nano 4g */
    { L"37K", L"B657" }, /* 4GB Purple Nano 4g */
    { L"37L", L"B660" }, /* 4GB Orange Nano 4g */
    { L"2ME", L"B598" }, /* 8GB Silver Nano 4g */
    { L"3QS", L"B732" }, /* 8GB Blue Nano 4g */
    { L"3QT", L"B735" }, /* 8GB Pink Nano 4g */
    { L"3QU", L"B739" }, /* 8GB Purple Nano 4g */
    { L"3QW", L"B742" }, /* 8GB Orange Nano 4g */
    { L"3QX", L"B745" }, /* 8GB Green Nano 4g */
    { L"3QY", L"B748" }, /* 8GB Yellow Nano 4g */
    { L"3R0", L"B754" }, /* 8GB Black Nano 4g */
    { L"3QZ", L"B751" }, /* 8GB Red Nano 4g */
    { L"5B7", L"B903" }, /* 16GB Silver Nano 4g */
    { L"5B8", L"B905" }, /* 16GB Blue Nano 4g */
    { L"5B9", L"B907" }, /* 16GB Pink Nano 4g */
    { L"5BA", L"B909" }, /* 16GB Purple Nano 4g */
    { L"5BB", L"B911" }, /* 16GB Orange Nano 4g */
    { L"5BC", L"B913" }, /* 16GB Green Nano 4g */
    { L"5BD", L"B915" }, /* 16GB Yellow Nano 4g */
    { L"5BE", L"B917" }, /* 16GB Red Nano 4g */
    { L"5BF", L"B918" }, /* 16GB Black Nano 4g */

    { L"71V", L"C027" }, /* 8GB Silver Nano 5g */
    { L"71Y", L"C031" }, /* 8GB Black Nano 5g */
    { L"721", L"C034" }, /* 8GB Purple Nano 5g */
    { L"726", L"C037" }, /* 8GB Blue Nano 5g */
    { L"72A", L"C040" }, /* 8GB Green Nano 5g */
    { L"72F", L"C046" }, /* 8GB Orange Nano 5g */
    { L"72L", L"C050" }, /* 8GB Pink Nano 5g */

    { L"72Q", L"C060" }, /* 16GB Silver Nano 5g */
    { L"72R", L"C062" }, /* 16GB Black Nano 5g */
    { L"72S", L"C064" }, /* 16GB Purple Nano 5g */
    { L"72X", L"C066" }, /* 16GB Blue Nano 5g */
    { L"734", L"C068" }, /* 16GB Green Nano 5g */
    { L"738", L"C070" }, /* 16GB Yellow Nano 5g */
    { L"739", L"C072" }, /* 16GB Orange Nano 5g */
    { L"73A", L"C074" }, /* 16GB Red Nano 5g */
    { L"73B", L"C075" }, /* 16GB Pink Nano 5g */

    { L"4NZ", L"B867" }, /* 4GB Silver Shuffle 4g */
    { L"891", L"C164" }, /* 4GB Black Shuffle 4g */
    
    { L"W4T", L"A627" }, /* 16GB Silver iPod Touch (1st gen) */
    { L"0JW", L"B376" }, /* 32GB Silver iPod Touch (1st gen) */
    { L"201", L"B528" }, /* 8GB Silver iPod Touch (2nd gen) */
    { L"203", L"B531" }, /* 16GB Silver iPod Touch (2nd gen) */
    { L"75J", L"C086" }, /* 8GB Silver iPod Touch (3rd gen) */
    { L"6K2", L"C008" }, /* 32GB Silver iPod Touch (3rd gen) */
    { L"6K4", L"C011" }, /* 64GB Silver iPod Touch (3rd gen) */

    { L"VR0", L"A501" }, /* 4GB Silver iPhone 1st gen */
    { L"WH8", L"A712" }, /* 8GB Silver iPhone */
    { L"0KH", L"B384" }, /* 16GB Silver iPhone */
    { L"Y7H", L"B046" }, /* 8GB Black iPhone 3G */
    { L"Y7K", L"B496" }, /* 16GB Black iPhone 3G */
    { L"3NP", L"C131" }, /* 16GB Black iPhone 3GS */
    { L"3NR", L"C133" }  /* 32GB Black iPhone 3GS */
};


static const wchar_t *GetModelStrForFamilyID(unsigned int familyID)
{
	switch(familyID)
	{
	case 4: // iPod 4 
		return L"9282";
	case 5: // iPod 4 (photo)
		return L"9830";
	case 6: // iPod 5
		return L"A002";
	case 7: // nano 1
		return L"A004";
	case 9: // nano 2
		return L"A477";
	case 11: // classic
		return L"B147";
	case 12: // fat nano
		return L"A978"; 
	case 128: // shuffle
		return L"A133";
	case 130: // shuffle 2
		return L"A947";
	default:
		return 0;
	}
}

const iPodInfo *GetiPodInfoForModelStr(const wchar_t *modelstr)
{
	// now locate this ipod in our table
	int l = sizeof(ipod_info_table)/sizeof(iPodInfo);
	
	for(int i=0; i<l; i++) 
	{
		int compareRet = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, ipod_info_table[i].model_number, -1, modelstr, -1)-2;
		if(compareRet==0)
			return &ipod_info_table[i]; // success!
	}
	return 0;
}

const wchar_t* GetModelStrForSerialNumber(const wchar_t *serialNumber)
{
	// now locate this ipod in our table
	int l = sizeof(serial_to_model_mapping)/sizeof(iPodSerialToModel);

	INT serialNumberLen = lstrlen(serialNumber);

	if (serialNumberLen < 3)
	{
		return NULL;
	}

	const wchar_t *last3OfSerialNumber = &serialNumber[serialNumberLen-3];
	
	for(int i=0; i<l; i++) 
	{
		int compareRet = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, last3OfSerialNumber, -1, serial_to_model_mapping[i].serial, -1)-2;
		if(compareRet==0)
			return serial_to_model_mapping[i].model_number; // success!
	}
	return 0;
}

extern bool ParseSysInfoXML(wchar_t drive_letter, char * xml, int xmllen);

const iPodInfo *GetiPodInfo(wchar_t drive) 
{
	static const iPodInfo unknown = {NULL, 0, ITDB_IPOD_MODEL_INVALID, ITDB_IPOD_GENERATION_UNKNOWN, 0};
	
	wchar_t itunes_pref[MAX_PATH];
	char xml[65536];

	if(ParseSysInfoXML(drive, xml, sizeof(xml)/sizeof(char))) 
	{
		// go fetch the FamilyID so we can construct a model string
		DWORD bytesRead = sizeof(xml)/sizeof(char);
		
		// use the plist loader
		plistLoader it;

		obj_xml *parser=0;
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
		if (factory)
			parser = (obj_xml *)factory->getInterface();

		if (parser)
		{
			// load the XML, this creates an iTunes DB in memory, and returns the	root key			
			parser->xmlreader_open();
			parser->xmlreader_registerCallback(L"plist\f*", &it);
			parser->xmlreader_feed(xml, bytesRead);
			parser->xmlreader_unregisterCallback(&it);
			parser->xmlreader_close();
			plistKey *root_key = &it;
			plistData *root_dict = root_key->getData();
			if (root_dict)
			{
				// check for the existance of sqlite
				plistKey *sqliteKey = ((plistDict*)root_dict)->getKey(L"SQLiteDB");
				if (sqliteKey)
				{
					plistData *sqliteData = sqliteKey->getData();
					if (sqliteData)
					{
						//plistRaw *familyDataRaw = (plistRaw *)familyData;
						const wchar_t* sqliteString = sqliteData->getString();
						if (sqliteString)
						{
							int compareRet = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sqliteString, -1, L"1", -1)-2;
							if (compareRet < 0)
							{
								return &unknown;
							}
						}
					}
				} // end sqlite check

				// check for FamilyID
				plistKey *familyKey = ((plistDict*)root_dict)->getKey(L"FamilyID");
				if (familyKey)
				{
					plistData *familyData = familyKey->getData();
					if (familyData)
					{
						const wchar_t* familyIDString = familyData->getString();
						if (familyIDString)
						{
							unsigned int familyID = _wtoi(familyIDString);
							const wchar_t *modelStr = NULL;
							modelStr = GetModelStrForFamilyID(familyID);

							// if modelString not apparent, as the case is in most
							// 5th gen nanos and classics
							if (!modelStr)
							{
								plistKey *serialNumberKey = ((plistDict*)root_dict)->getKey(L"SerialNumber");
								if (serialNumberKey)
								{
									plistData *serialNumberData = serialNumberKey->getData();
									if (serialNumberData)
									{
										const wchar_t* serialNumberString = serialNumberData->getString();

										if (serialNumberString)
										{
											modelStr = GetModelStrForSerialNumber(serialNumberString);
										}
									}
								}
							}

							if (modelStr)
							{
								const iPodInfo *info = GetiPodInfoForModelStr(modelStr);
								if (info)
									return info;
							}

						}
					}
				} // end familyid
			}
		}
	}
	return &unknown;
}

const ArtworkFormat* GetArtworkFormats(const iPodInfo* info) {
	if(!info) return NULL;
	return ipod_artwork_info_table[info->model];
}

