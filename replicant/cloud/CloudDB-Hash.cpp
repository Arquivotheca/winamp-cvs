#include "CloudDB.h"
#include "sha1.h"
#include "AutoNormalize.h"
#include "CloudSocket.h"
#include "nswasabi/ReferenceCounted.h"
#include <ctype.h>

#define SQLPARAM(x) x, sizeof(x)
const char sql_idmap_get_for_metahash[] = "SELECT artist, album, trackno, title FROM [idmap] WHERE media_id=?";
const char sql_idmap_get_for_albumhash[] = "SELECT albumartist, artist, album FROM [idmap] WHERE media_id=?";

// this function assumes ALL CAPS string
static void StripTheAndWhitespace(const char *&utf8, size_t &cch)
{
	while (cch)
	{
		if (*utf8 == ' ' || *utf8 == '\t')
		{
			utf8++;
			cch--;
		}
		else
			break;
	}

	if (cch >= 4)
	{
		if (utf8[0] == 'T' && utf8[1] == 'H' && utf8[2] == 'E' && utf8[3] == ' ')
		{
			utf8 += 4;
			cch -= 4;
		}
	}

	while (cch)
	{
		if (*utf8 == ' ' || *utf8 == '\t')
		{
			utf8++;
			cch--;
		}
		else
			break;
	}

	while (cch && (utf8[cch-1] == ' ' || utf8[cch-1] == '\t'))
	{
		cch--;
	}
}

void Cloud_DBConnection::Internal_Compute(nx_string_t value, SHA1_CTX *sha1)
{
	if (!value)
		return;
#ifdef _WIN32
	const wchar_t *normalized_string = normalizer.Normalize(NormalizationC, value->string);
	if (normalized_string)
	{
		size_t cch;
		const wchar_t *utf16 = normalized_string;
		cch = wcslen(utf16);

		int s = LCMapString(LOCALE_INVARIANT, LCMAP_UPPERCASE, utf16, (int)cch+1, 0, 0);
		wchar_t *temp = (wchar_t *)malloc(s * sizeof(wchar_t));
		s = LCMapString(LOCALE_INVARIANT, LCMAP_UPPERCASE, utf16, (int)cch+1, temp, s);
		const char *utf8 = converter.Convert(temp, CP_UTF8, 0, &cch);
		StripTheAndWhitespace(utf8, cch);
		SHA1Update(sha1, (unsigned char *)utf8, cch);
		free(temp);
	}
#elif defined(__APPLE__)
	//	AutoNormalize normalizer;
	//	AutoCharGrow converter;
	// const wchar_t *normalized_string = normalizer.Normalize(NormalizationC, value->string);

	// TODO: convert to uppercase!!!
	//	const char *utf8 = value->string;
	// size_t cch = value->len;
	const char *utf8 = NULL;
	size_t cch = 0;
	StripTheAndWhitespace(utf8, cch);
	SHA1Update(sha1, (unsigned char *)utf8, (unsigned int)cch);
#else
	const char *utf8 = value->string;
	size_t cch = value->len;
	char *u = (char *)malloc(cch);
	if (u)
	{
		// TODO: only works for ASCII!
		for (size_t i=0;i<cch;i++)
			u[i] = toupper(utf8[i]);

		utf8 = u;
		StripTheAndWhitespace(utf8, cch);

		SHA1Update(sha1, (unsigned char *)utf8, cch);
		free(u);
	}

#endif
}

int Cloud_DBConnection::ComputeMetaHash(int internal_id, nx_string_t *out_metahash)
{
	ReferenceCountedNXString artist, album, trackno, title;

	int sqlite_ret=Step(statement_idmap_get_for_metahash, SQLPARAM(sql_idmap_get_for_metahash), internal_id);
	Columns(statement_idmap_get_for_metahash, &artist, &album, &trackno, &title);

	SHA1_CTX metahash_sha1;
	SHA1Init(&metahash_sha1);

	Internal_Compute(artist, &metahash_sha1);		
	SHA1Update(&metahash_sha1, (unsigned char *)",", 1);
	Internal_Compute(album, &metahash_sha1);
	SHA1Update(&metahash_sha1, (unsigned char *)",", 1);
	Internal_Compute(trackno, &metahash_sha1);
	SHA1Update(&metahash_sha1, (unsigned char *)",", 1);
	Internal_Compute(title, &metahash_sha1);

	uint8_t sha1_hash[20];
	SHA1Final(sha1_hash, &metahash_sha1);

	char temp[41];
	sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3],
		sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7],
		sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11],
		sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
		sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

	return NXStringCreateWithUTF8(out_metahash, temp);
}

int Cloud_DBConnection::ComputeAlbumHash(int internal_id, nx_string_t *out_albumhash)
{
	ReferenceCountedNXString albumartist, artist, album;

	int sqlite_ret=Step(statement_idmap_get_for_albumhash, SQLPARAM(sql_idmap_get_for_albumhash), internal_id);
	Columns(statement_idmap_get_for_albumhash, &albumartist, &artist, &album);

	SHA1_CTX albumhash_sha1;
	SHA1Init(&albumhash_sha1);

	if (albumartist != (nx_string_t)NULL)
		Internal_Compute(albumartist, &albumhash_sha1);
	else
		Internal_Compute(artist, &albumhash_sha1);

	SHA1Update(&albumhash_sha1, (unsigned char *)",", 1);

	Internal_Compute(album, &albumhash_sha1);

	uint8_t sha1_hash[20];
	SHA1Final(sha1_hash, &albumhash_sha1);

	char temp[41];
	sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
		sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
		sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
		sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
		sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

	return NXStringCreateWithUTF8(out_albumhash, temp);
}

int Cloud_DBConnection::ComputeIDHash(int internal_id, nx_string_t *out_idhash)
{
	ReferenceCountedNXString metahash, mediahash;
	AutoCharUTF8 converter;

	SHA1_CTX idhash;
	SHA1Init(&idhash);

	if (IDMap_GetMetaHash(internal_id, &metahash) != NErr_Success || IDMap_GetMediaHash(internal_id, &mediahash) != NErr_Success)
		return NErr_Error;

	converter.Set(metahash);
	SHA1Update(&idhash, (unsigned char *)(const char *)converter, (unsigned int)converter.size());
	converter.Set(mediahash);
	SHA1Update(&idhash, (unsigned char *)(const char *)converter, (unsigned int)converter.size());

	uint8_t sha1_hash[20];
	SHA1Final(sha1_hash, &idhash);

	char temp[41];
	sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
		sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
		sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
		sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
		sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

	return NXStringCreateWithUTF8(out_idhash, temp);
}

int Cloud_DBConnection::ComputePlaylistHash(nx_string_t playlist, nx_string_t *out_playlisthash)
{
	AutoCharUTF8 converter;

	SHA1_CTX idhash;
	SHA1Init(&idhash);

	converter.Set(playlist);
	SHA1Update(&idhash, (unsigned char *)(const char *)converter, (unsigned int)converter.size());

	uint8_t sha1_hash[20];
	SHA1Final(sha1_hash, &idhash);

	char temp[41];
	sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
		sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
		sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
		sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
		sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

	return NXStringCreateWithUTF8(out_playlisthash, temp);
}