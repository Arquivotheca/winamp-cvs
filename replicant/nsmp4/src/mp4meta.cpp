/*
* The contents of this file are subject to the Mozilla Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
* rights and limitations under the License.
*
* The Original Code is MPEG4IP.
*
* The Initial Developer of the Original Code is Cisco Systems Inc.
* Portions created by Cisco Systems Inc. are
* Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
*
* Contributor(s):
*      M. Bakker     mbakker at nero.com
*
* Apple iTunes Metadata handling
*/

/**

The iTunes tagging seems to support any tag field name
but there are some predefined fields, also known from the QuickTime format

predefined fields (the ones I know of until now):
- �nam : Name of the song/movie (string)
- �ART : Name of the artist/performer (string)
- aART : Album artist
- �wrt : Name of the writer (string)
- �alb : Name of the album (string)
- �day : Year (4 bytes, e.g. "2003") (string)
- �too : Tool(s) used to create the file (string)
- �cmt : Comment (string)
- �gen : Custom genre (string)
- �grp : Grouping (string)
- trkn : Tracknumber (8 byte string)
16 bit: empty
16 bit: tracknumber
16 bit: total tracks on album
16 bit: empty
- disk : Disknumber (8 byte string)
16 bit: empty
16 bit: disknumber
16 bit: total number of disks
16 bit: empty
- gnre : Genre (16 bit genre) (ID3v1 index + 1)
- cpil : Part of a compilation (1 byte, 1 or 0)
- tmpo : Tempo in BPM (16 bit)
- covr : Cover art (xx bytes binary data)
- ---- : Free form metadata, can have any name and any data
- pgap : gapless - 8 bit boolean

- apID : purchaser name.
- cprt : copyright
- purd : purchase date.

**/


#include "mp4common.h"
#include "foundation/error.h"
#include "nx/nxstring.h"

static int NXStringCreateFromAtom(MP4Atom *metadata_atom, const char *property_name, nx_string_t *value)
{
	MP4BytesProperty *bytes_property=0;
	if (metadata_atom->FindProperty(property_name, (MP4Property **)&bytes_property) && bytes_property)
	{
		const uint8_t *val;
		uint32_t val_size;
		bytes_property->GetPointer(&val, &val_size);

		if (val && val_size > 0)
		{
			return NXStringCreateWithBytes(value, val, val_size, nx_charset_utf8);
		} 
	}
	*value = 0;
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_Create(MP4Atom **atom)
{
	MP4Atom *pMetaAtom = AddDescendantAtoms("moov", "udta.meta.ilst");

	if (!pMetaAtom)
		return NErr_Error;

	MP4Atom *pHdlrAtom = m_pRootAtom->FindAtom("moov.udta.meta.hdlr");
	MP4StringProperty *pStringProperty = NULL;
	MP4BytesProperty *pBytesProperty = NULL;
	ASSERT(pHdlrAtom);

	ASSERT(pHdlrAtom->FindProperty("hdlr.handlerType", (MP4Property**)&pStringProperty));
	ASSERT(pStringProperty);
	pStringProperty->SetValue("mdir");

	uint8_t val[12];
	memset(val, 0, 12*sizeof(uint8_t));
	val[0] = 0x61;
	val[1] = 0x70;
	val[2] = 0x70;
	val[3] = 0x6c;
	ASSERT(pHdlrAtom->FindProperty("hdlr.reserved2", (MP4Property**)&pBytesProperty));
	ASSERT(pBytesProperty);
	pBytesProperty->SetReadOnly(false);
	pBytesProperty->SetValue(val, 12);
	pBytesProperty->SetReadOnly(true);

	*atom = pMetaAtom;

	return NErr_Success;
}

int MP4File::Metadata_iTunes_Enumerate(uint32_t index, MP4Atom **atom)
{
	char s[256];

	snprintf(s, 256, "moov.udta.meta.ilst.*[%u]", index);
	MP4Atom *metadata_atom = m_pRootAtom->FindAtom(s);

	*atom = metadata_atom;
	if (metadata_atom == NULL)
		return NErr_EndOfEnumeration;
	else
		return NErr_Success;
}

int MP4File::Metadata_iTunes_FindKey(const char *key, MP4Atom **atom)
{
	char atomstring[60];
	snprintf(atomstring, 60, "moov.udta.meta.ilst.%s", key);

	MP4Atom *metadata_atom = m_pRootAtom->FindAtom(atomstring);
	*atom = metadata_atom;

	if (!metadata_atom)
		return NErr_Empty;
	else
		return NErr_Success;	
}

int MP4File::Metadata_iTunes_EnumerateKey(const char *key, uint32_t index, MP4Atom **atom)
{
	char s[256];

	snprintf(s, 256, "moov.udta.meta.ilst.%s[%u]", key, index);
	MP4Atom *metadata_atom = m_pRootAtom->FindAtom(s);

	*atom = metadata_atom;
	if (metadata_atom == NULL)
		return NErr_EndOfEnumeration;
	else
		return NErr_Success;
}

int MP4File::Metadata_iTunes_NewKey(const char *key, MP4Atom **atom, uint32_t flags)
{
	MP4Atom *itunes_metadata_atom;
	int ret = Metadata_iTunes_Create(&itunes_metadata_atom);
	if (ret != NErr_Success)
		return ret;

	char t[256];
	snprintf(t, 256, "moov.udta.meta.ilst.%s", key);

	MP4Atom *count_atom = m_pRootAtom->FindAtom("moov.udta.meta.ilst.covr");
	if (count_atom)
	{
		uint32_t index = count_atom->GetNumberOfChildAtoms();
		snprintf(t, 256, "udta.meta.ilst.%s[%u]", key, index);	
	}
	else
	{
		snprintf(t, 256, "udta.meta.ilst.%s", key);	
	}

	MP4Atom *metadata_atom = AddDescendantAtoms("moov", t);
	if (!metadata_atom)
		return NErr_Error;
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (!data_atom)
		return NErr_Error;

	data_atom->SetFlags(flags);

	*atom = metadata_atom;
		
	return NErr_Success;	
}

int MP4File::Metadata_iTunes_FindFreeform(const char *name, const char *mean, MP4Atom **atom)
{
	char s[256];
	uint32_t i = 0;

	size_t name_len = strlen(name);
	size_t mean_len = mean?strlen(mean):0;

	while (1)
	{
		MP4BytesProperty *pMetadataProperty=0;

		snprintf(s, 256,"moov.udta.meta.ilst.----[%u]", i);
		MP4Atom *metadata_atom = m_pRootAtom->FindAtom(s);
		if (!metadata_atom)
			return NErr_Empty;

		MP4Atom *name_atom = metadata_atom->FindChildAtom("name");
		if (!name_atom)
			return NErr_Empty;

		MP4Atom *mean_atom = metadata_atom->FindChildAtom("mean");

		if (name_atom->FindProperty("name.metadata", (MP4Property**)&pMetadataProperty) && pMetadataProperty) 
		{
			const uint8_t *name_value;
			uint32_t name_value_length = 0;

			pMetadataProperty->GetPointer(&name_value, &name_value_length);

			if (name_value_length == name_len && !memcmp(name_value, name, name_value_length)) 
			{
				const uint8_t* mean_value=0;
				uint32_t mean_value_length = 0;

				if (!mean) /* if they didn't care about the mean atom, go ahead and return */
				{
					*atom = metadata_atom;
					return NErr_Success;
				}

				pMetadataProperty=0;
				if (mean_atom->FindProperty("mean.metadata", (MP4Property**)&pMetadataProperty) && pMetadataProperty) 
				{
					pMetadataProperty->GetPointer(&mean_value, &mean_value_length);
					if (mean_value && mean_len == mean_value_length && !memcmp(mean, mean_value, mean_value_length))
					{
						*atom = metadata_atom;
						return NErr_Success;
					}
				}
			}
		}

		i++;
	}
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_NewFreeform(const char *name, const char *mean, MP4Atom **atom, uint32_t flags)
{
	MP4Atom *itunes_metadata_atom;
	int ret = Metadata_iTunes_Create(&itunes_metadata_atom);
	if (ret != NErr_Success)
		return ret;

	/* count existing ---- atoms */
		uint32_t i = 0;
		char s[256];
	while (1)
	{
		MP4BytesProperty *pMetadataProperty=0;

		snprintf(s, 256,"moov.udta.meta.ilst.----[%u]", i);
		MP4Atom *metadata_atom = m_pRootAtom->FindAtom(s);
		if (!metadata_atom)
			break;
	i++;
		}

	char t[256];

	snprintf(t, 256, "udta.meta.ilst.----[%u]", i);
	MP4Atom *metadata_atom = AddDescendantAtoms("moov", t);
	if (!metadata_atom)
		return NErr_Error;
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (!data_atom)
		return NErr_Error;

	data_atom->SetFlags(flags);

	MP4Atom *name_atom = metadata_atom->FindChildAtom("name");
	if (!name_atom)
		return NErr_Error;

	MP4BytesProperty *bytes_property=0;
	if (name_atom->FindProperty("name.metadata", (MP4Property **)&bytes_property) && bytes_property)
	{
		bytes_property->SetValue((const uint8_t*)name, strlen(name));
	}
	else
		return NErr_Error;

		MP4Atom *mean_atom = metadata_atom->FindChildAtom("mean");
	if (!mean_atom)
		return NErr_Error;

	bytes_property=0;
	if (mean_atom->FindProperty("mean.metadata", (MP4Property **)&bytes_property) && bytes_property)
	{
		if (!mean || !*mean)
		bytes_property->SetValue((uint8_t*)"com.apple.iTunes", 16); /* com.apple.iTunes is the default*/
	else
		bytes_property->SetValue((const uint8_t*)mean, strlen((const char *)mean));
	}
	else
		return NErr_Error;

	*atom = metadata_atom;
		
	return NErr_Success;
}

int MP4File::Metadata_iTunes_GetInformation(MP4Atom *metadata_atom, char atom[5], uint32_t *flags)
{
	if (atom)
	{
		const char *atom_type = metadata_atom->GetType();
		memcpy(atom, atom_type, 4);
		atom[4]=0;
	}

	if (flags)
	{
		*flags = 0;
		MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
		if (data_atom)
		{
			MP4Integer24Property *flags_property=0;
			if (data_atom->FindProperty("data.flags", (MP4Property **)&flags_property) && flags_property)
			{
				*flags = flags_property->GetValue();
			}
		}
		else
			return NErr_Error;
	}
	return NErr_Success;	
}

int MP4File::Metadata_iTunes_GetString(MP4Atom *metadata_atom, nx_string_t *value)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		return NXStringCreateFromAtom(data_atom, "data.metadata", value);
	}
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_GetBinary(MP4Atom *metadata_atom, const uint8_t **value, size_t *length)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		const uint8_t *val = NULL;
		uint32_t val_size = 0;

		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			uint32_t length32=0;
			bytes_property->GetPointer(value, &length32);
			*length = length32;
			return NErr_Success;
		}
	}
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_GetSigned(MP4Atom *metadata_atom, int64_t *value)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		const uint8_t *val = NULL;
		uint32_t val_size = 0;

		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			int64_t number=0;
			const uint8_t *data;
			uint32_t length;
			bytes_property->GetPointer(&data, &length);

			if (length > 8)
				return NErr_Insufficient;

			if (length == 0)
			{
				*value = 0;
				return NErr_Success;
			}

			int8_t first = *(const int8_t *)data;
			number = (int64_t)first;
			uint8_t *dest = (uint8_t *)&number;
			for (size_t i=1;i<length;i++)
			{
				number <<= 8;
				number |= (uint64_t)data[i];
			}


			*value = number;
			return NErr_Success;
		}
	}
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_GetUnsigned(MP4Atom *metadata_atom, uint64_t *value)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		const uint8_t *val = NULL;
		uint32_t val_size = 0;


		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			uint64_t number=0;
			const uint8_t *data;
			uint32_t length;
			bytes_property->GetPointer(&data, &length);
			if (length > 8)
				return NErr_Insufficient;
			for (size_t i=0;i<length;i++)
			{
				number <<= 8;
				number |= data[i];
			}
			*value = number;
			return NErr_Success;
		}
	}
	return NErr_Empty;
}

int MP4File::Metadata_iTunes_GetFreeform(MP4Atom *metadata_atom, nx_string_t *name, nx_string_t *mean)
{
	MP4Atom *name_atom = metadata_atom->FindChildAtom("name");
	if (!name_atom)
		return NErr_Empty;

	int ret = NXStringCreateFromAtom(name_atom, "name.metadata", name);
	if (ret && ret != NErr_Empty)
		return ret;

	MP4Atom *mean_atom = metadata_atom->FindChildAtom("mean");
	if (mean_atom)
	{
		ret = NXStringCreateFromAtom(mean_atom, "mean.metadata", mean);
		if (ret && ret != NErr_Empty)
			return ret;
	}

	return NErr_Success;
}

int MP4File::Metadata_iTunes_SetString(MP4Atom *metadata_atom, nx_string_t value)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			uint8_t *data;
			size_t byte_count;
			int ret = NXStringGetBytesSize(&byte_count, value, nx_charset_utf8, 0);
			if (ret == NErr_Success || ret == NErr_DirectPointer)
			{
				ret = bytes_property->ModifyPointer(&data, byte_count);
				if (ret != NErr_Success)
					return ret;

				return NXStringGetBytes(&byte_count, value, data, byte_count, nx_charset_utf8, 0);
			}

			return ret;
		}
	}
	
	return NErr_Error;
}

int MP4File::Metadata_iTunes_SetUnsigned(MP4Atom *metadata_atom, uint64_t value, size_t byte_count)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			uint8_t *data;
			int ret = bytes_property->ModifyPointer(&data, byte_count);
			if (ret != NErr_Success)
				return ret;

			while (byte_count--)
			{
				*data++ = (value >> (byte_count*8)) & 0xFF;
			}
			return NErr_Success;
		}
	}
	
	return NErr_Error;
}

int MP4File::Metadata_iTunes_SetBinary(MP4Atom *metadata_atom, const void *data, size_t length)
{
	MP4Atom *data_atom = metadata_atom->FindChildAtom("data");
	if (data_atom)
	{
		MP4BytesProperty *bytes_property=0;
		if (data_atom->FindProperty("data.metadata", (MP4Property **)&bytes_property) && bytes_property)
		{
			uint8_t *new_data;
			int ret = bytes_property->ModifyPointer(&new_data, length);
			if (ret != NErr_Success)
				return ret;

			memcpy(new_data, data, length);
			return NErr_Success;
		}
	}
	
	return NErr_Error;
}

int MP4File::Metadata_iTunes_DeleteAtom(MP4Atom *metadata_atom)
{
	if (metadata_atom)
	{
		MP4Atom *pParent = metadata_atom->GetParentAtom();
		pParent->DeleteChildAtom(metadata_atom);
		delete metadata_atom;
		return NErr_Success;
	}
	return NErr_BadParameter;
}

/* --- */
bool MP4File::CreateMetadataAtom(const char* name)
{
	char s[256];
	char t[256];

	snprintf(t, 256, "udta.meta.ilst.%s.data", name);
	snprintf(s, 256, "moov.udta.meta.ilst.%s.data", name);
	(void)AddDescendantAtoms("moov", t);
	MP4Atom *pMetaAtom = m_pRootAtom->FindAtom(s);

	if (!pMetaAtom)
		return false;

	/* some fields need special flags set */
	if ((uint8_t)name[0] == 0251 || ATOMID(name) == ATOMID("aART"))
	{
		pMetaAtom->SetFlags(0x1);
	} else if ((memcmp(name, "cpil", 4) == 0) || (memcmp(name, "tmpo", 4) == 0)) {
		pMetaAtom->SetFlags(0x15);
	}

	MP4Atom *pHdlrAtom = m_pRootAtom->FindAtom("moov.udta.meta.hdlr");
	MP4StringProperty *pStringProperty = NULL;
	MP4BytesProperty *pBytesProperty = NULL;
	ASSERT(pHdlrAtom);

	ASSERT(pHdlrAtom->FindProperty("hdlr.handlerType", 
		(MP4Property**)&pStringProperty));
	ASSERT(pStringProperty);
	pStringProperty->SetValue("mdir");

	uint8_t val[12];
	memset(val, 0, 12*sizeof(uint8_t));
	val[0] = 0x61;
	val[1] = 0x70;
	val[2] = 0x70;
	val[3] = 0x6c;
	ASSERT(pHdlrAtom->FindProperty("hdlr.reserved2", 
		(MP4Property**)&pBytesProperty));
	ASSERT(pBytesProperty);
	pBytesProperty->SetReadOnly(false);
	pBytesProperty->SetValue(val, 12);
	pBytesProperty->SetReadOnly(true);

	return true;
}

bool MP4File::DeleteMetadataAtom(const char* name, bool try_udta)
{
	MP4Atom *pMetaAtom = NULL;
	char s[256];

	snprintf(s, 256, "moov.udta.meta.ilst.%s", name);
	pMetaAtom = m_pRootAtom->FindAtom(s);

	if (pMetaAtom == NULL && try_udta) {
		snprintf(s, 256, "moov.udta.%s", name);
		pMetaAtom = m_pRootAtom->FindAtom(s);
	}
	/* if it exists, delete it */
	if (pMetaAtom)
	{
		MP4Atom *pParent = pMetaAtom->GetParentAtom();

		pParent->DeleteChildAtom(pMetaAtom);

		delete pMetaAtom;

		return true;
	}

	return false;
}

bool MP4File::SetMetadataTrack(uint16_t track, uint16_t totalTracks)
{
	unsigned char t[9];
	const char *s = "moov.udta.meta.ilst.trkn.data";
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	pMetaAtom = m_pRootAtom->FindAtom(s);

	if (!pMetaAtom)
	{
		if (!CreateMetadataAtom("trkn"))
			return false;

		pMetaAtom = m_pRootAtom->FindAtom(s);
		if (pMetaAtom == NULL) return false;
	}

	memset(t, 0, 9*sizeof(unsigned char));
	t[2] = (unsigned char)(track>>8)&0xFF;
	t[3] = (unsigned char)(track)&0xFF;
	t[4] = (unsigned char)(totalTracks>>8)&0xFF;
	t[5] = (unsigned char)(totalTracks)&0xFF;

	ASSERT(pMetaAtom->FindProperty("data.metadata", 
		(MP4Property**)&pMetadataProperty));
	ASSERT(pMetadataProperty);

	pMetadataProperty->SetValue((uint8_t*)t, 8);

	return true;
}

bool MP4File::SetMetadataDisk(uint16_t disk, uint16_t totalDisks)
{
	unsigned char t[7];
	const char *s = "moov.udta.meta.ilst.disk.data";
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	pMetaAtom = m_pRootAtom->FindAtom(s);

	if (!pMetaAtom)
	{
		if (!CreateMetadataAtom("disk"))
			return false;

		pMetaAtom = m_pRootAtom->FindAtom(s);
		if (pMetaAtom == NULL) return false;
	}

	memset(t, 0, 7*sizeof(unsigned char));
	t[2] = (unsigned char)(disk>>8)&0xFF;
	t[3] = (unsigned char)(disk)&0xFF;
	t[4] = (unsigned char)(totalDisks>>8)&0xFF;
	t[5] = (unsigned char)(totalDisks)&0xFF;

	ASSERT(pMetaAtom->FindProperty("data.metadata", 
		(MP4Property**)&pMetadataProperty));
	ASSERT(pMetadataProperty);

	pMetadataProperty->SetValue((uint8_t*)t, 6);

	return true;
}

static const char* ID3v1GenreList[] = {
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
	"Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
	"Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
	"Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
	"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
	"Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
	"Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
	"Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
	"Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
	"Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
	"Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
	"New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
	"Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
	"Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
	"Fast-Fusion", "Bebop", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
	"Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
	"Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
	"Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
	"Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
	"Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
	"Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
	"Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
	"Indie", "BritPop", "Afro-Punk", "Polsk Punk", "Beat",
	"Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
	"Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
	"SynthPop", "Abstract", "Art Rock", "Baroque", "Bhangra", "Big Beat", "Breakbeat", "Chillout", "Downtempo", "Dub", "EBM", "Eclectic", "Electro",
	"Electroclash", "Emo", "Experimental", "Garage", "Global", "IDM", "Illbient", "Industro-Goth", "Jam Band", "Krautrock", "Leftfield", "Lounge",
	"Math Rock", "New Romantic", "Nu-Breakz", "Post-Punk", "Post-Rock", "Psytrance", "Shoegaze", "Space Rock", "Trop Rock", "World Music", "Neoclassical",
	"Audiobook", "Audio Theatre", "Neue Deutsche Welle", "Podcast", "Indie Rock", "G-Funk", "Dubstep", "Garage Rock", "Psybient",
};

void GenreToString(char** GenreStr, const int genre)
{
	if (genre > 0 && 
		genre <= (int)(sizeof(ID3v1GenreList)/sizeof(*ID3v1GenreList)))
	{
		unsigned int len = strlen(ID3v1GenreList[genre-1])+1;
		*GenreStr = (char*)malloc(len);
		if (*GenreStr == NULL) return;
		// no need for strncpy; enough was malloced
		strcpy(*GenreStr, ID3v1GenreList[genre-1]);
		return;
	} 
	*GenreStr = (char*)malloc(2*sizeof(char));
	if (*GenreStr == NULL) return;
	memset(*GenreStr, 0, 2*sizeof(char));
	return;
}

int StringToGenre(const char* GenreStr)
{
	unsigned int i;

	for (i = 0; i < sizeof(ID3v1GenreList)/sizeof(*ID3v1GenreList); i++)
	{
		if (strcasecmp(GenreStr, ID3v1GenreList[i]) == 0)
			return i+1;
	}
	return 0;
}

bool MP4File::SetMetadataGenre(const char* value)
{
	uint16_t genreIndex = 0;
	unsigned char t[3];
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	genreIndex = StringToGenre(value);

	const char *s = "moov.udta.meta.ilst.gnre.data";
	const char *sroot = "moov.udta.meta.ilst.gnre";
	const char *s2 = "moov.udta.meta.ilst.\251gen.data";
	const char *s2root = "moov.udta.meta.ilst.\251gen";
	if (genreIndex != 0)
	{
		pMetaAtom = m_pRootAtom->FindAtom(s);
		if (!pMetaAtom)
		{
			if (!CreateMetadataAtom("gnre"))
				return false;

			pMetaAtom = m_pRootAtom->FindAtom(s);
			if (pMetaAtom == NULL) return false;
		}

		memset(t, 0, 3*sizeof(unsigned char));
		t[0] = (unsigned char)(genreIndex>>8)&0xFF;
		t[1] = (unsigned char)(genreIndex)&0xFF;

		ASSERT(pMetaAtom->FindProperty("data.metadata", 
			(MP4Property**)&pMetadataProperty));
		ASSERT(pMetadataProperty);

		pMetadataProperty->SetValue((uint8_t*)t, 2);

		// remove other style of genre atom, if this one is added
		pMetaAtom = m_pRootAtom->FindAtom(s2root);
		if (pMetaAtom != NULL) {
			MP4Atom *pParent = pMetaAtom->GetParentAtom();
			if (pParent != NULL) {
				pParent->DeleteChildAtom(pMetaAtom);
				delete pMetaAtom;
			}
		}


		(void)DeleteMetadataAtom( "\251gen" );

		return true;
	} else {
		pMetaAtom = m_pRootAtom->FindAtom(s2);

		if (!pMetaAtom)
		{
			if (!CreateMetadataAtom("\251gen"))
				return false;

			pMetaAtom = m_pRootAtom->FindAtom(s2);
		}

		ASSERT(pMetaAtom->FindProperty("data.metadata", 
			(MP4Property**)&pMetadataProperty));
		ASSERT(pMetadataProperty);

		pMetadataProperty->SetValue((uint8_t*)value, strlen(value));

		// remove other gnre atom if this one is entered
		pMetaAtom = m_pRootAtom->FindAtom(sroot);
		if (pMetaAtom != NULL) {
			MP4Atom *pParent = pMetaAtom->GetParentAtom();
			pParent->DeleteChildAtom(pMetaAtom);
			delete pMetaAtom;
		}
		return true;
	}

	return false;
}


bool MP4File::DeleteMetadataGenre()
{
	bool val1 = DeleteMetadataAtom("\251gen");
	bool val2 = DeleteMetadataAtom("gnre");
	return val1 || val2;
}
#if 0
bool MP4File::SetMetadataTempo(uint16_t tempo)
{
	unsigned char t[3];
	const char *s = "moov.udta.meta.ilst.tmpo.data";
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	pMetaAtom = m_pRootAtom->FindAtom(s);

	if (!pMetaAtom)
	{
		if (!CreateMetadataAtom("tmpo"))
			return false;

		pMetaAtom = m_pRootAtom->FindAtom(s);
		if (pMetaAtom == NULL) return false;
	}

	memset(t, 0, 3*sizeof(unsigned char));
	t[0] = (unsigned char)(tempo>>8)&0xFF;
	t[1] = (unsigned char)(tempo)&0xFF;

	ASSERT(pMetaAtom->FindProperty("data.metadata", 
		(MP4Property**)&pMetadataProperty));
	ASSERT(pMetadataProperty);

	pMetadataProperty->SetValue((uint8_t*)t, 2);

	return true;
}
#endif
bool MP4File::SetMetadataUint8 (const char *atom, uint8_t value)
{
	char atompath[36];
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	snprintf(atompath, 36, "moov.udta.meta.ilst.%s.data", atom);

	pMetaAtom = m_pRootAtom->FindAtom(atompath);

	if (pMetaAtom == NULL) {
		if (!CreateMetadataAtom(atom))
			return false;

		pMetaAtom = m_pRootAtom->FindAtom(atompath);
		if (pMetaAtom == NULL) return false;
	}

	ASSERT(pMetaAtom->FindProperty("data.metadata", 
		(MP4Property**)&pMetadataProperty));
	ASSERT(pMetadataProperty);

	pMetadataProperty->SetValue(&value, 1);

	return true;
}



bool MP4File::SetMetadataCoverArt(uint8_t *coverArt, uint32_t size)
{
	const char *s = "moov.udta.meta.ilst.covr.data";
	MP4BytesProperty *pMetadataProperty = NULL;
	MP4Atom *pMetaAtom = NULL;

	pMetaAtom = m_pRootAtom->FindAtom(s);

	if (!pMetaAtom)
	{
		if (!CreateMetadataAtom("covr"))
			return false;

		pMetaAtom = m_pRootAtom->FindAtom(s);
		if (pMetaAtom == NULL) return false;
	}

	ASSERT(pMetaAtom->FindProperty("data.metadata", 
		(MP4Property**)&pMetadataProperty));
	ASSERT(pMetadataProperty);

	pMetadataProperty->SetValue(coverArt, size);

	return true;
}

bool MP4File::GetMetadataCoverArt(uint8_t **coverArt, uint32_t *size,	uint32_t index)
{
	char buffer[256];
	if (size == NULL || coverArt == NULL) return false;

	if (index > 0 && index > GetMetadataCoverArtCount()) return false;

	snprintf(buffer, 256, "moov.udta.meta.ilst.covr.data[%d].metadata", index);

	*coverArt = NULL;
	*size = 0;

	GetBytesProperty(buffer, coverArt, size);

	if (*size == 0)
		return false;

	return true;
}

uint32_t MP4File::GetMetadataCoverArtCount(void)
{
	MP4Atom *pMetaAtom = m_pRootAtom->FindAtom("moov.udta.meta.ilst.covr");
	if (!pMetaAtom)
		return 0;

	return pMetaAtom->GetNumberOfChildAtoms();
}

bool MP4File::MetadataDelete()
{
	MP4Atom *pMetaAtom = NULL;
	char s[256];

	snprintf(s, 256, "moov.udta.meta");
	pMetaAtom = m_pRootAtom->FindAtom(s);

	/* if it exists, delete it */
	if (pMetaAtom)
	{
		MP4Atom *pParent = pMetaAtom->GetParentAtom();

		pParent->DeleteChildAtom(pMetaAtom);

		delete pMetaAtom;

		return true;
	}

	return false;
}
