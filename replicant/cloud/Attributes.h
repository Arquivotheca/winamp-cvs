#pragma once
#include "nx/nxstring.h"
struct Attributes
{
	Attributes()
	{
		device_token=0;
		device_id=0;
	}

	~Attributes()
	{
		NXStringRelease(device_token);
	}

	nx_string_t device_token;
	int device_id;
	int
		artist,
		album,
		trackno,
		albumartist,
		bpm,
		category,
		comment,
		composer,
		director,
		disc,
		discs,
		genre,
		producer,
		publisher,
		tracks,
		year,
		albumgain,
		trackgain,
		rating,
		type,
		lossless
	;
};