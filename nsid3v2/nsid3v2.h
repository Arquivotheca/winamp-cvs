#pragma once

#include <bfc/platform/types.h>
#include <bfc/platform/export.h>
#include <bfc/error.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NSID3V2_EXPORTS
#define NSID3V2_EXPORT __declspec(dllexport)
#else
#define NSID3V2_EXPORT __declspec(dllimport)
#endif

	typedef struct nsid3v2_header_struct_t { } *nsid3v2_header_t;
	typedef struct nsid3v2_tag_struct_t { } *nsid3v2_tag_t;
	typedef struct nsid3v2_frame_struct_t { } *nsid3v2_frame_t;

	// must be exactly 10 bytes
	NSID3V2_EXPORT int NSID3v2_Header_Valid(const void *header_data);
	NSID3V2_EXPORT int NSID3v2_Header_Create(nsid3v2_header_t *header, const void *header_data, size_t header_len);
	NSID3V2_EXPORT int NSID3v2_Header_TagSize(const nsid3v2_header_t header, uint32_t *tag_size);
	NSID3V2_EXPORT int NSID3v2_Header_HasFooter(const nsid3v2_header_t header);
	NSID3V2_EXPORT int NSID3v2_Header_Destroy(nsid3v2_header_t header);

	NSID3V2_EXPORT int NSID3v2_Tag_Create(nsid3v2_tag_t *tag, nsid3v2_header_t header, const void *bytes, size_t bytes_len);


	/* 
	get specific information out of a tag.  returns the first one found that matches the requirements.
	*/
	enum
	{
		NSID3V2_TEXT_SYSTEM=1, // use system code page instead of ISO-8859-1
	};
	NSID3V2_EXPORT int NSID3v2_Tag_Text_GetUTF16(const nsid3v2_tag_t tag, int frame_enum, wchar_t *buf, size_t buf_cch, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_URL_GetUTF16(const nsid3v2_tag_t tag, int frame_enum, wchar_t *buf, size_t buf_cch, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Comments_GetUTF16(const nsid3v2_tag_t tag, const wchar_t *description, wchar_t *buf, size_t buf_cch, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_TXXX_GetUTF16(const nsid3v2_tag_t tag, const wchar_t *description, wchar_t *buf, size_t buf_cch, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Popularimeter_GetRatingPlaycount(const nsid3v2_tag_t tag, const char *email, uint8_t *rating, uint64_t *playcount);
	NSID3V2_EXPORT int NSID3v2_Tag_APIC_GetPicture(const nsid3v2_tag_t t, uint8_t picture_type, void *_memmgr, wchar_t **mime_type, void **picture_data, size_t *picture_bytes);
	NSID3V2_EXPORT int NSID3v2_Tag_APIC_GetFirstPicture(const nsid3v2_tag_t t, void *_memmgr, wchar_t **mime_type, void **picture_data, size_t *picture_bytes);
	NSID3V2_EXPORT int NSID3v2_Tag_APIC_GetFrame(const nsid3v2_tag_t t, uint8_t picture_type, nsid3v2_frame_t *f);
	NSID3V2_EXPORT int NSID3v2_Tag_APIC_GetFirstFrame(const nsid3v2_tag_t t, nsid3v2_frame_t *f);

	NSID3V2_EXPORT int NSID3v2_Tag_GetFrame(const nsid3v2_tag_t tag, int frame_enum, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_GetNextFrame(const nsid3v2_tag_t tag, const nsid3v2_frame_t start_frame, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_RemoveFrame(nsid3v2_tag_t tag, nsid3v2_frame_t frame);
	NSID3V2_EXPORT int NSID3v2_Tag_CreateFrame(nsid3v2_tag_t tag, int frame_enum, int flags, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_AddFrame(nsid3v2_tag_t tag, nsid3v2_frame_t frame);

	NSID3V2_EXPORT int NSID3v2_Frame_Text_SetUTF16(nsid3v2_frame_t frame, const wchar_t *value);
	NSID3V2_EXPORT int NSID3v2_Frame_UserText_SetUTF16(nsid3v2_frame_t frame, const wchar_t *description, const wchar_t *value);
	NSID3V2_EXPORT int NSID3v2_Frame_UserText_SetLatin(nsid3v2_frame_t frame, const char *description, const char *value);


	enum
	{
		NSID3V2_FRAME_PICTURE, // APIC
		NSID3V2_FRAME_COMMENTS, // COMM
		NSID3V2_FRAME_POPULARIMETER, // POPM
		NSID3V2_FRAME_ALBUM, // TALB
		NSID3V2_FRAME_BPM, // TBPM
		NSID3V2_FRAME_COMPOSER, // TCOM
		NSID3V2_FRAME_CONTENTTYPE, // TCON
		NSID3V2_FRAME_COPYRIGHT, // TCOP
		NSID3V2_FRAME_DATE, // TDAT
		NSID3V2_FRAME_PLAYLISTDELAY, // TDLY
		NSID3V2_FRAME_RECORDINGTIME, // TDRC
		NSID3V2_FRAME_ENCODEDBY, // TENC
		NSID3V2_FRAME_LYRICIST, // TEXT
		NSID3V2_FRAME_FILETYPE, // TFLT
		NSID3V2_FRAME_TIME, // TIME
		NSID3V2_FRAME_CONTENTGROUP, // TIT1
		NSID3V2_FRAME_TITLE, // TIT2		
		NSID3V2_FRAME_SUBTITLE, // TIT3
		NSID3V2_FRAME_KEY, // TKEY
		NSID3V2_FRAME_LANGUAGE, // TLAN
		NSID3V2_FRAME_LENGTH, // TLEN
		NSID3V2_FRAME_MEDIATYPE, // TMED
		NSID3V2_FRAME_MOOD, // TMOO
		NSID3V2_FRAME_ORIGINALALBUM, // TOAL

		NSID3V2_FRAME_ORIGINALARTIST, // TOPE		

		NSID3V2_FRAME_LEADARTIST, // TPE1
		NSID3V2_FRAME_BAND, // TPE2
		NSID3V2_FRAME_CONDUCTOR, // TPE3
		NSID3V2_FRAME_REMIXER, // TPE4
		NSID3V2_FRAME_PARTOFSET, // TPOS
		NSID3V2_FRAME_PUBLISHER, // TPUB
		NSID3V2_FRAME_TRACK, // TRCK
		NSID3V2_FRAME_RECORDINGDATES, // TRDA

		NSID3V2_FRAME_ISRC, // TSRC
		NSID3V2_FRAME_ENCODERSETTINGS, // TSSE
		NSID3V2_FRAME_YEAR, // TYER

		NSID3V2_FRAME_USER_TEXT, // TXXX

	};
#ifdef __cplusplus
}
#endif