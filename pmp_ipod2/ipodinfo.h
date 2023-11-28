#ifndef _IPOD_INFO_H_
#define _IPOD_INFO_H_

#define RGB_565 0
#define RGB_555 1
#define RGB_555_REC 2
typedef enum {
  THUMB_INVALID = -1,
  THUMB_COVER_SMALL,
	THUMB_COVER_MEDIUM1,
	THUMB_COVER_MEDIUM2,
	THUMB_COVER_MEDIUM3,
	THUMB_COVER_MEDIUM4,
  THUMB_COVER_LARGE,
  THUMB_PHOTO_SMALL,
  THUMB_PHOTO_LARGE,
  THUMB_PHOTO_FULL_SCREEN,
  THUMB_PHOTO_TV_SCREEN,
} ThumbType;

typedef enum {
	IPOD_COLOR_WHITE,
	IPOD_COLOR_BLACK,
	IPOD_COLOR_SILVER,
	IPOD_COLOR_BLUE,
	IPOD_COLOR_PINK,
	IPOD_COLOR_GREEN,
	IPOD_COLOR_ORANGE,
	IPOD_COLOR_GOLD,
	IPOD_COLOR_RED,
	IPOD_COLOR_U2,
} iPodColor;


/*************************************************************************************************************/
/* ------------------------------------------------------------ *\
 *
 * iPod model-relevant definitions
 *
\* ------------------------------------------------------------ */

/**
 * Itdb_IpodGeneration:
 * @ITDB_IPOD_GENERATION_UNKNOWN:   Unknown iPod
 * @ITDB_IPOD_GENERATION_FIRST:     First Generation iPod
 * @ITDB_IPOD_GENERATION_SECOND:    Second Generation iPod
 * @ITDB_IPOD_GENERATION_THIRD:     Third Generation iPod
 * @ITDB_IPOD_GENERATION_FOURTH:    Fourth Generation iPod
 * @ITDB_IPOD_GENERATION_PHOTO:     Photo iPod
 * @ITDB_IPOD_GENERATION_MOBILE:    Mobile iPod
 * @ITDB_IPOD_GENERATION_MINI_1:    First Generation iPod Mini
 * @ITDB_IPOD_GENERATION_MINI_2:    Second Generation iPod Mini
 * @ITDB_IPOD_GENERATION_SHUFFLE_1: First Generation iPod Shuffle
 * @ITDB_IPOD_GENERATION_SHUFFLE_2: Second Generation iPod Shuffle
 * @ITDB_IPOD_GENERATION_SHUFFLE_3: Third Generation iPod Shuffle
 * @ITDB_IPOD_GENERATION_SHUFFLE_4: Third Generation iPod Shuffle
 * @ITDB_IPOD_GENERATION_NANO_1:    First Generation iPod Nano
 * @ITDB_IPOD_GENERATION_NANO_2:    Second Generation iPod Nano
 * @ITDB_IPOD_GENERATION_NANO_3:    Third Generation iPod Nano
 * @ITDB_IPOD_GENERATION_NANO_4:    Fourth Generation iPod Nano
 * @ITDB_IPOD_GENERATION_NANO_5:    Fifth Generation iPod Nano (with camera)
 * @ITDB_IPOD_GENERATION_VIDEO_1:   First Generation iPod Video (aka 5g)
 * @ITDB_IPOD_GENERATION_VIDEO_2:   Second Generation iPod Video (aka 5.5g)
 * @ITDB_IPOD_GENERATION_CLASSIC_1: First Generation iPod Classic
 * @ITDB_IPOD_GENERATION_CLASSIC_2: Second Generation iPod Classic
 * @ITDB_IPOD_GENERATION_CLASSIC_3: Third Generation iPod Classic
 * @ITDB_IPOD_GENERATION_TOUCH_1:   First Generation iPod Touch
 * @ITDB_IPOD_GENERATION_TOUCH_2:   Second Generation iPod Touch
 * @ITDB_IPOD_GENERATION_TOUCH_3:   Third Generation iPod Touch
 * @ITDB_IPOD_GENERATION_IPHONE_1:  First Generation iPhone
 * @ITDB_IPOD_GENERATION_IPHONE_2:  Second Generation iPhone (aka iPhone 3G)
 * @ITDB_IPOD_GENERATION_IPHONE_3:  Third Generation iPhone (aka iPhone 3GS)
 *
 * iPod generation information
 *
 * See http://support.apple.com/kb/HT1353 and http://en.wikipedia.org/wiki/IPod
 * for more details.
 *
 * Since: 0.4.0
 */
typedef enum {
    ITDB_IPOD_GENERATION_UNKNOWN,
    ITDB_IPOD_GENERATION_FIRST,
    ITDB_IPOD_GENERATION_SECOND,
    ITDB_IPOD_GENERATION_THIRD,
    ITDB_IPOD_GENERATION_FOURTH,
    ITDB_IPOD_GENERATION_PHOTO,
    ITDB_IPOD_GENERATION_MOBILE,
    ITDB_IPOD_GENERATION_MINI_1,
    ITDB_IPOD_GENERATION_MINI_2,
    ITDB_IPOD_GENERATION_SHUFFLE_1,
    ITDB_IPOD_GENERATION_SHUFFLE_2,
    ITDB_IPOD_GENERATION_SHUFFLE_3,
    ITDB_IPOD_GENERATION_NANO_1,
    ITDB_IPOD_GENERATION_NANO_2,
    ITDB_IPOD_GENERATION_NANO_3,
    ITDB_IPOD_GENERATION_NANO_4,
    ITDB_IPOD_GENERATION_VIDEO_1,
    ITDB_IPOD_GENERATION_VIDEO_2,
    ITDB_IPOD_GENERATION_CLASSIC_1,
    ITDB_IPOD_GENERATION_CLASSIC_2,
    ITDB_IPOD_GENERATION_TOUCH_1,
    ITDB_IPOD_GENERATION_IPHONE_1,
    ITDB_IPOD_GENERATION_SHUFFLE_4,
    ITDB_IPOD_GENERATION_TOUCH_2,
    ITDB_IPOD_GENERATION_IPHONE_2,
    ITDB_IPOD_GENERATION_IPHONE_3,
    ITDB_IPOD_GENERATION_CLASSIC_3,
    ITDB_IPOD_GENERATION_NANO_5,
    ITDB_IPOD_GENERATION_TOUCH_3
} iPodGeneration;

/**
 * Itdb_IpodModel:
 * @ITDB_IPOD_MODEL_INVALID:        Invalid model
 * @ITDB_IPOD_MODEL_UNKNOWN:        Unknown model
 * @ITDB_IPOD_MODEL_COLOR:          Color iPod
 * @ITDB_IPOD_MODEL_COLOR_U2:       Color iPod (U2)
 * @ITDB_IPOD_MODEL_REGULAR:        Regular iPod
 * @ITDB_IPOD_MODEL_REGULAR_U2:     Regular iPod (U2)
 * @ITDB_IPOD_MODEL_MINI:           iPod Mini
 * @ITDB_IPOD_MODEL_MINI_BLUE:      iPod Mini (Blue)
 * @ITDB_IPOD_MODEL_MINI_PINK:      iPod Mini (Pink)
 * @ITDB_IPOD_MODEL_MINI_GREEN:     iPod Mini (Green)
 * @ITDB_IPOD_MODEL_MINI_GOLD:      iPod Mini (Gold)
 * @ITDB_IPOD_MODEL_SHUFFLE:        iPod Shuffle
 * @ITDB_IPOD_MODEL_NANO_WHITE:     iPod Nano (White)
 * @ITDB_IPOD_MODEL_NANO_BLACK:     iPod Nano (Black)
 * @ITDB_IPOD_MODEL_VIDEO_WHITE:    iPod Video (White)
 * @ITDB_IPOD_MODEL_VIDEO_BLACK:    iPod Video (Black)
 * @ITDB_IPOD_MODEL_MOBILE_1:       Mobile iPod
 * @ITDB_IPOD_MODEL_VIDEO_U2:       iPod Video (U2)
 * @ITDB_IPOD_MODEL_NANO_SILVER:    iPod Nano (Silver)
 * @ITDB_IPOD_MODEL_NANO_BLUE:      iPod Nano (Blue)
 * @ITDB_IPOD_MODEL_NANO_GREEN:     iPod Nano (Green)
 * @ITDB_IPOD_MODEL_NANO_PINK:      iPod Nano (Pink)
 * @ITDB_IPOD_MODEL_NANO_RED:       iPod Nano (Red)
 * @ITDB_IPOD_MODEL_NANO_YELLOW:    iPod Nano (Yellow)
 * @ITDB_IPOD_MODEL_NANO_PURPLE:    iPod Nano (Purple)
 * @ITDB_IPOD_MODEL_NANO_ORANGE:    iPod Nano (Orange)
 * @ITDB_IPOD_MODEL_IPHONE_1:       iPhone
 * @ITDB_IPOD_MODEL_SHUFFLE_SILVER: iPod Shuffle (Silver)
 * @ITDB_IPOD_MODEL_SHUFFLE_BLACK:  iPod Shuffle (Black)
 * @ITDB_IPOD_MODEL_SHUFFLE_PINK:   iPod Shuffle (Pink)
 * @ITDB_IPOD_MODEL_SHUFFLE_BLUE:   iPod Shuffle (Blue)
 * @ITDB_IPOD_MODEL_SHUFFLE_GREEN:  iPod Shuffle (Green)
 * @ITDB_IPOD_MODEL_SHUFFLE_ORANGE: iPod Shuffle (Orange)
 * @ITDB_IPOD_MODEL_SHUFFLE_PURPLE: iPod Shuffle (Purple)
 * @ITDB_IPOD_MODEL_SHUFFLE_RED:    iPod Shuffle (Red)
 * @ITDB_IPOD_MODEL_CLASSIC_SILVER: iPod Classic (Silver)
 * @ITDB_IPOD_MODEL_CLASSIC_BLACK:  iPod Classic (Black)
 * @ITDB_IPOD_MODEL_TOUCH_SILVER:   iPod Touch (Silver)
 * @ITDB_IPOD_MODEL_IPHONE_WHITE:   iPhone (White)
 * @ITDB_IPOD_MODEL_IPHONE_BLACK:   iPhone (Black)
 *
 * iPod model information
 *
 * Since: 0.4.0
 */
typedef enum {
    ITDB_IPOD_MODEL_INVALID,
    ITDB_IPOD_MODEL_UNKNOWN,
    ITDB_IPOD_MODEL_COLOR,
    ITDB_IPOD_MODEL_COLOR_U2,
    ITDB_IPOD_MODEL_REGULAR,
    ITDB_IPOD_MODEL_REGULAR_U2,
    ITDB_IPOD_MODEL_MINI,
    ITDB_IPOD_MODEL_MINI_BLUE,
    ITDB_IPOD_MODEL_MINI_PINK,
    ITDB_IPOD_MODEL_MINI_GREEN,
    ITDB_IPOD_MODEL_MINI_GOLD,
    ITDB_IPOD_MODEL_SHUFFLE,
    ITDB_IPOD_MODEL_NANO_WHITE,
    ITDB_IPOD_MODEL_NANO_BLACK,
    ITDB_IPOD_MODEL_VIDEO_WHITE,
    ITDB_IPOD_MODEL_VIDEO_BLACK,
    ITDB_IPOD_MODEL_MOBILE_1,
    ITDB_IPOD_MODEL_VIDEO_U2,
    ITDB_IPOD_MODEL_NANO_SILVER,
    ITDB_IPOD_MODEL_NANO_BLUE,
    ITDB_IPOD_MODEL_NANO_GREEN,
    ITDB_IPOD_MODEL_NANO_PINK,
    ITDB_IPOD_MODEL_NANO_RED,
    ITDB_IPOD_MODEL_NANO_YELLOW,
    ITDB_IPOD_MODEL_NANO_PURPLE,
    ITDB_IPOD_MODEL_NANO_ORANGE,
    ITDB_IPOD_MODEL_IPHONE_1,
    ITDB_IPOD_MODEL_SHUFFLE_SILVER,
    ITDB_IPOD_MODEL_SHUFFLE_PINK,
    ITDB_IPOD_MODEL_SHUFFLE_BLUE,
    ITDB_IPOD_MODEL_SHUFFLE_GREEN,
    ITDB_IPOD_MODEL_SHUFFLE_ORANGE,
    ITDB_IPOD_MODEL_SHUFFLE_PURPLE,
    ITDB_IPOD_MODEL_SHUFFLE_RED,
    ITDB_IPOD_MODEL_CLASSIC_SILVER,
    ITDB_IPOD_MODEL_CLASSIC_BLACK,
    ITDB_IPOD_MODEL_TOUCH_SILVER,
    ITDB_IPOD_MODEL_SHUFFLE_BLACK,
    ITDB_IPOD_MODEL_IPHONE_WHITE,
    ITDB_IPOD_MODEL_IPHONE_BLACK,
} iPodModel;
/*********************************************************************************************************/

typedef struct {
  ThumbType type;
  int width;
  int height;
  int correlation_id;
	int format;
	int row_align;
	int image_align;
} ArtworkFormat;

/**
 * iPodInfo:
 * @model_number:    The model number.  This is abbreviated.  If the first
 *                   character is not numeric, it is ommited. e.g.
 *                   "MA350 -> A350", "M9829 -> 9829"
 * @capacity:        The iPod's capacity in gigabytes
 * @ipod_model:      The iPod model
 * @ipod_generation: The iPod generation
 * @musicdirs:       The number of music (Fnn) dirs created by iTunes. The
 *                   exact number seems to be version dependent. Therefore, the
 *                   numbers here represent a mixture of reported values and
 *                   common sense.  Note: this number does not necessarily
 *                   represent the number of dirs present on a particular iPod.
 *                   It is used when setting up a new iPod from scratch.
 * @reserved_int1:   Reserved for future use
 * @reserved_int2:   Reserved for future use
 * @reserved1:       Reserved for future use
 * @reserved2:       Reserved for future use
 *
 * Structure representing information about an iPod
 *
 * Since: 0.4.0
 */
typedef struct {
    const wchar_t* model_number;
    const double capacity;
    const iPodModel model;
    const iPodGeneration generation;
    const unsigned int musicdirs;
} iPodInfo;

struct _iPodSerialToModel {
    const wchar_t *serial;
    const wchar_t *model_number;
};
typedef struct _iPodSerialToModel iPodSerialToModel;

const iPodInfo *GetiPodInfo(wchar_t drive);
const ArtworkFormat* GetArtworkFormats(const iPodInfo* info);

#endif //_IPOD_INFO_H_
