#pragma once

#include "mp4.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"

#ifdef __cplusplus
extern "C" {
#endif

	

	/* iTunes Metadata API */
	static const char *nsmp4_metadata_itunes_artist = "\251ART";
	static const char *nsmp4_metadata_itunes_album = "\251alb";
	static const char *nsmp4_metadata_itunes_name = "\251nam";
	static const char *nsmp4_metadata_itunes_writer = "\251wrt";
	static const char *nsmp4_metadata_itunes_tool = "\251too";
	static const char *nsmp4_metadata_itunes_comment = "\251cmt";
	static const char *nsmp4_metadata_itunes_year = "\251day";
	static const char *nsmp4_metadata_itunes_track = "trkn";
	static const char *nsmp4_metadata_itunes_disk = "disk";
	static const char *nsmp4_metadata_itunes_grouping = "\251grp";
	static const char *nsmp4_metadata_itunes_tempo = "tmpo";
	static const char *nsmp4_metadata_itunes_compilation = "cpil";
	static const char *nsmp4_metadata_itunes_gapless = "pgap";
	static const char *nsmp4_metadata_itunes_albumartist = "aART";
	static const char *nsmp4_metadata_itunes_genre = "\251gen";
	static const char *nsmp4_metadata_itunes_genre_numeric = "gnre";
	static const char *nsmp4_metadata_itunes_cover_art = "covr";
	
	enum 
	{
		nsmp4_metadata_itunes_type_binary = 0,
		nsmp4_metadata_itunes_type_utf8 = 1,
		nsmp4_metadata_itunes_type_utf16be = 2,
		nsmp4_metadata_itunes_type_latin1 = 3,
		nsmp4_metadata_itunes_type_gif = 12,
		nsmp4_metadata_itunes_type_jpeg = 13,
		nsmp4_metadata_itunes_type_png = 14,
		nsmp4_metadata_itunes_type_signed_integer_be = 21,
		nsmp4_metadata_itunes_type_unsigned_integer_be = 22,
		nsmp4_metadata_itunes_type_float32be = 23,
		nsmp4_metadata_itunes_type_float64be = 24,
		nsmp4_metadata_itunes_type_bmp = 27,
	};

	typedef struct nsmp4_metadata_itunes_struct_t {} *nsmp4_metadata_itunes_atom_t;

	int NSMP4_Metadata_iTunes_Enumerate(MP4FileHandle mp4_file, uint32_t index, nsmp4_metadata_itunes_atom_t *atom);
	int NSMP4_Metadata_iTunes_FindKey(MP4FileHandle mp4_file, const char *key, nsmp4_metadata_itunes_atom_t *atom);
	int NSMP4_Metadata_iTunes_EnumerateKey(MP4FileHandle mp4_file, const char *key, unsigned int index, nsmp4_metadata_itunes_atom_t *atom);
	int NSMP4_Metadata_iTunes_NewKey(MP4FileHandle mp4_file, const char *key, nsmp4_metadata_itunes_atom_t *atom, uint32_t flags);
	int NSMP4_Metadata_iTunes_FindFreeform(MP4FileHandle mp4_file, const char *name, const char *mean, nsmp4_metadata_itunes_atom_t *atom);
	int NSMP4_Metadata_iTunes_NewFreeform(MP4FileHandle mp4_file, const char *name, const char *mean, nsmp4_metadata_itunes_atom_t *atom, uint32_t flags);
	int NSMP4_Metadata_iTunes_GetInformation(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t metadata_atom, char atom[5], uint32_t *flags);	
	
	int NSMP4_Metadata_iTunes_GetString(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t *value);
	int NSMP4_Metadata_iTunes_GetBinary(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length); 
	int NSMP4_Metadata_iTunes_GetSigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, int64_t *value);
	int NSMP4_Metadata_iTunes_GetUnsigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, uint64_t *value);
	int NSMP4_Metadata_iTunes_GetFreeform(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t *name, nx_string_t *mean);

	int NSMP4_Metadata_iTunes_SetString(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t value);
	int NSMP4_Metadata_iTunes_SetUnsigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, uint64_t value, size_t byte_count);
	int NSMP4_Metadata_iTunes_SetBinary(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, const void *data, size_t length);
	
	int NSMP4_Metadata_iTunes_DeleteAtom(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom);
#ifdef __cplusplus
}
#endif