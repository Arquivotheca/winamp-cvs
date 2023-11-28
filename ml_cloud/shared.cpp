//#pragma comment(lib, "crypt32.lib")
#include "api.h"
#include "sha1.h"
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "shared.h"
//#include <Wincrypt.h>

//static HCRYPTPROV crypt_provider;


ns_error_t ComputeMediaHash(const wchar_t *filename, nx_string_t *out_mediahash)
{
#if 0
	if (!crypt_provider)
		CryptAcquireContext(&crypt_provider, NULL, NULL, PROV_RSA_FULL, 0);

	uint8_t sha1_hash[20];
	waServiceFactory *sf;
	size_t i=0;
	while (sf = WASABI_API_SVC->service_enumService(svc_raw_media_reader::SERVICETYPE, i++))
	{
		svc_raw_media_reader *reader_service = (svc_raw_media_reader *)sf->getInterface();
		if (reader_service)
		{
			ifc_raw_media_reader *reader=0;
			int ret = reader_service->CreateRawMediaReader(filename, &reader);
			sf->releaseInterface(reader_service);
			if (ret == NErr_Success)
			{
				HCRYPTHASH sha1;
				CryptCreateHash(crypt_provider, CALG_SHA1, 0, 0, &sha1);
				
				uint8_t buffer[65536];
				size_t bytes_read;
				
				while (reader->Read(buffer, 65536, &bytes_read) == NErr_Success)
				{
					CryptHashData(sha1, buffer, bytes_read, 0);
				}

				reader->Release();
				
				DWORD hashlen=20;
				HRESULT hr = CryptGetHashParam(sha1, HP_HASHVAL, sha1_hash, &hashlen, 0);
				CryptDestroyHash(sha1);

				char temp[41];
				sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
					sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
					sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
					sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
					sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
					sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

				return NXStringCreateWithUTF8(out_mediahash, temp);
			}
		}
	}
	return NErr_Error;

#else

	uint8_t sha1_hash[20];
	waServiceFactory *sf;
	size_t i=0;
	while (sf = WASABI_API_SVC->service_enumService(svc_raw_media_reader::SERVICETYPE, i++))
	{
		svc_raw_media_reader *reader_service = (svc_raw_media_reader *)sf->getInterface();
		if (reader_service)
		{
			ifc_raw_media_reader *reader=0;
			int ret = reader_service->CreateRawMediaReader(filename, &reader);
			sf->releaseInterface(reader_service);
			if (ret == NErr_Success)
			{
				SHA1_CTX sha1;
				uint8_t buffer[4096];
				size_t bytes_read;
				SHA1Init(&sha1);
				while (reader->Read(buffer, 4096, &bytes_read) == NErr_Success)
				{
					SHA1Update(&sha1, buffer, bytes_read);
				}

				reader->Release();
				SHA1Final(sha1_hash, &sha1);
				char temp[41];
				sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
					sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
					sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
					sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
					sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
					sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

				return NXStringCreateWithUTF8(out_mediahash, temp);
			}
		}
	}
	return NErr_Error;
#endif
}

ns_error_t ComputeArtHash(nx_uri_t filename, int field, nx_string_t *out_arthash)
{
	artwork_t artwork;
	uint64_t file_modified;
	ns_error_t ret = REPLICANT_API_ARTWORK->GetArtwork(filename, field, &artwork, DATA_FLAG_ALL, &file_modified);
	if (ret != NErr_Success)
		return ret;

	SHA1_CTX sha1;	
	uint8_t sha1_hash[20];
	const void *bytes;
	size_t length;

	SHA1Init(&sha1);	
	NXDataGet(artwork.data, &bytes, &length);
	SHA1Update(&sha1, (unsigned char *)bytes, length);
	SHA1Final(sha1_hash, &sha1);
	return NXStringCreateWithFormatting(out_arthash, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
					sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
					sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
					sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
					sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
					sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);
}

int MediaHashMetadata::MetadataKey_CloudMediaHash=-1;
MediaHashMetadata::MediaHashMetadata(nx_string_t mediahash)
{
	this->mediahash = NXStringRetain(mediahash);
}

MediaHashMetadata::~MediaHashMetadata()
{
	NXStringRelease(mediahash);
}

ns_error_t MediaHashMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (field == MetadataKey_CloudMediaHash)
	{
		if (mediahash)
		{
			*value = NXStringRetain(mediahash);
			return NErr_Success;
		}
		else
		{
			return NErr_Empty;
		}
	}
	return NErr_Unknown;
}

int ArtHashMetadata::MetadataKey_CloudArtHashAlbum=-1;
ArtHashMetadata::ArtHashMetadata(nx_string_t arthash_album)
{
	this->arthash_album = NXStringRetain(arthash_album);
}

ArtHashMetadata::~ArtHashMetadata()
{
	NXStringRelease(arthash_album);
}

ns_error_t ArtHashMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (field == MetadataKey_CloudArtHashAlbum)
	{
		if (arthash_album)
		{
			*value = NXStringRetain(arthash_album);
			return NErr_Success;
		}
		else
		{
			return NErr_Empty;
		}
	}
	return NErr_Unknown;
}