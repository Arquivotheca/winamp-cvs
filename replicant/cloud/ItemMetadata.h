#pragma once
#include "metadata/ifc_metadata.h"

class ItemMetadata : public ifc_metadata
{
public:
	ItemMetadata() : year(-1), track(1), tracks(-1), length(-1), rating(-1),
					 playcount(-1), lastplay(-1), lastupd(-1), filetime(-1),
					 filesize(-1), bitrate(-1), disc(-1), discs(-1), bpm(-1),
					 cloud(0), dateadded(-1), filename(0), title(0), album(0),
					 artist(0), comment(0), genre(0), albumartist(0),
					 replaygain_album_gain(0), replaygain_track_gain(0),
					 publisher(0), composer(0), category(0), producer(0),
					 director(0), metahash(0), mime(0) {}
	~ItemMetadata();

	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);

	ns_error_t WASABICALL Metadata_SetField(int field, unsigned int index, nx_string_t value);
	ns_error_t WASABICALL Metadata_SetInteger(int field, unsigned int index, int64_t value);
	ns_error_t WASABICALL Metadata_SetReal(int field, unsigned int index, double value);

	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data);
	ns_error_t WASABICALL Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata);

	ns_error_t WASABICALL Metadata_Serialize(nx_data_t *data);

private:
	nx_string_t filename;
	nx_string_t title;
	nx_string_t album;
	nx_string_t artist;
	nx_string_t comment;
	nx_string_t genre;
	nx_string_t albumartist;
	nx_string_t replaygain_album_gain;
	nx_string_t replaygain_track_gain;
	nx_string_t publisher;
	nx_string_t composer;
	nx_string_t category;
	nx_string_t producer;
	nx_string_t director;
	nx_string_t metahash;
	nx_string_t mime;

	int year;
	int track;
	int tracks;
	int length;
	int rating; // file rating. can be 1-5, or 0 for undefined
	int playcount; // number of file plays.
	int64_t lastplay; // last time played, in standard time_t format
	int64_t lastupd; // last time updated in library, in standard time_t format
	int64_t filetime; // last known file time of file, in standard time_t format
	int filesize; // last known file size, in kilobytes.
	int bitrate; // file bitrate, in kbps
	int disc; // disc number
	int discs; // number of discs
	int bpm;
	int64_t dateadded;
	int cloud;
};