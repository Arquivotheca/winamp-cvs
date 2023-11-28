#include "api.h"
#include "FileMetadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include "svc_filemetadata.h"
#include "service/ifc_servicefactory.h"
#include <stdlib.h>

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

/* ----------------------------------------- */

FileMetadata::FileMetadata()
{
	filename=0;
	start_position=0;
	end_position=0;
	content_length=0;
	memset(&file_stat, 0, sizeof(file_stat));
}

FileMetadata::~FileMetadata()
{
	NXURIRelease(filename);
	if (id3v2.tag)
		NSID3v2_Tag_Destroy(id3v2.tag);

	if (id3v1.tag)
		NSID3v1_Tag_Destroy(id3v1.tag);

	if (apev2.tag)
		NSAPEv2_Tag_Destroy(apev2.tag);

	free(lyrics3.tag);
}

bool FileMetadata::HasMetadata() const
{
	if (id3v2.tag || id3v1.tag || apev2.tag)
		return true;

	return false;
}

ns_error_t FileMetadata::SetFileInformation(nx_uri_t new_filename, nx_file_stat_t new_file_stat)
{
	NXURIRelease(filename);
	filename = NXURIRetain(new_filename);
	if (new_file_stat)
		memcpy(&file_stat, new_file_stat, sizeof(file_stat));

	return NErr_Success;
}

ns_error_t FileMetadata::OwnID3v2(nsid3v2_tag_t new_id3v2, uint64_t position, uint64_t length)
{
	if (id3v2.tag)
		NSID3v2_Tag_Destroy(id3v2.tag);
	id3v2.tag = new_id3v2;
	id3v2.position = position;
	id3v2.length = length;
	id3v2_metadata.Initialize(id3v2.tag);
	return NErr_Success;
}

ns_error_t FileMetadata::OwnID3v1(nsid3v1_tag_t new_id3v1, uint64_t position, uint64_t length)
{
	if (id3v1.tag)
		NSID3v1_Tag_Destroy(id3v1.tag);
	id3v1.tag = new_id3v1;
	id3v1.position = position;
	id3v1.length = length;
	id3v1_metadata.Initialize(id3v1.tag);
	return NErr_Success;
}

ns_error_t FileMetadata::OwnAPEv2(nsapev2_tag_t new_apev2, uint64_t position, uint64_t length)
{
	if (apev2.tag)
		NSAPEv2_Tag_Destroy(apev2.tag);
	apev2.tag = new_apev2;
	apev2.position = position;
	apev2.length = length;
	apev2_metadata.Initialize(apev2.tag);
	return NErr_Success;
}

ns_error_t FileMetadata::OwnLyrics3(void *new_lyrics, uint64_t position, uint64_t length)
{
	free(lyrics3.tag);
	lyrics3.tag = new_lyrics;
	lyrics3.position = position;
	lyrics3.length = length;
	return NErr_Success;
}


ns_error_t FileMetadata::FindMetadata(nx_file_t file)
{
	uint64_t region_start=0;
	uint64_t region_end;
	NXFileLength(file, &region_end);
	nsid3v2_tag_t id3v2=0;

	/* Read pre-pended ID3v2 tag(s) */
	uint8_t id3v2_header_data[10];
	size_t bytes_read;
	while(NXFileRead(file, id3v2_header_data, 10, &bytes_read) == NErr_Success && bytes_read == 10 && NSID3v2_Header_Valid(id3v2_header_data) == NErr_Success)
	{
		nsid3v2_header_t id3v2_header;
		uint32_t id3v2_tag_size;
		if (NSID3v2_Header_Create(&id3v2_header, id3v2_header_data, 10) != NErr_Success)
			break;

		if (NSID3v2_Header_TagSize(id3v2_header, &id3v2_tag_size) != NErr_Success)
		{
			NSID3v2_Header_Destroy(id3v2_header);
			break;
		}

		void *id3v2_data=0;
		if (id3v2)
		{
			// already read a tag
			region_start+=id3v2_tag_size+10;
		}
		else
		{
			// TODO: it'd be better to memory map the file into place but this works for now
			id3v2_data = malloc(id3v2_tag_size);
			if (!id3v2_data)
			{
				// TODO: debatable what we should do here. 
				NSID3v2_Header_Destroy(id3v2_header);
				return NErr_OutOfMemory;
			}

			if (NXFileRead(file, id3v2_data, id3v2_tag_size, &bytes_read) == NErr_Success && bytes_read == id3v2_tag_size
				&& NSID3v2_Tag_Create(&id3v2, id3v2_header, id3v2_data, id3v2_tag_size) == NErr_Success)
			{
				uint64_t length = id3v2_tag_size+10;
				uint64_t position = region_start;
				region_start += id3v2_tag_size+10;

				if (NSID3v2_Header_HasFooter(id3v2_header) == NErr_True)
				{
					length+=10;
					position+=10;
				}
				OwnID3v2(id3v2, position, length);
			}
			else
			{
				free(id3v2_data);
				break;
			}
			free(id3v2_data);
		}
		NSID3v2_Header_Destroy(id3v2_header);
	}


	/* Read ID3v1 tag */
	uint8_t id3v1_data[128];
	if ((region_end-region_start >= 128)
		&& NXFileSeek(file, region_end-128) == NErr_Success 
		&& NXFileRead(file, id3v1_data, 128, &bytes_read) == NErr_Success && bytes_read == 128)
	{
		nsid3v1_tag_t id3v1=0;
		if (NSID3v1_Header_Valid(id3v1_data, 128) == NErr_Success && NSID3v1_Tag_Create(id3v1_data, 128, &id3v1) == NErr_Success)
		{
			uint64_t position = region_end-128;
			region_end-=128;
			OwnID3v1(id3v1, position, 128);
		}		
	}

	/* Read appended ID3v2.4 tag */
	if ((region_end-region_start) >= 10 
		&& NXFileSeek(file, region_end-10) == NErr_Success
		&& NXFileRead(file, id3v2_header_data, 10, &bytes_read) == NErr_Success && bytes_read == 10
		&& NSID3v2_Header_FooterValid(id3v2_header_data) == NErr_Success)
	{
		nsid3v2_header_t id3v2_header;
		uint32_t id3v2_tag_size;
		if (NSID3v2_Header_FooterCreate(&id3v2_header, id3v2_header_data, 10) == NErr_Success)
		{
			if (NSID3v2_Header_TagSize(id3v2_header, &id3v2_tag_size) == NErr_Success)
			{
				void *id3v2_data=0;
				if (id3v2)
				{
					// already read a tag
					region_end -= id3v2_tag_size+20;
					NSID3v2_Header_Destroy(id3v2_header);
				}
				else
				{
					if ((region_end-region_start) >= (20 + id3v2_tag_size) 
						&& NXFileSeek(file, region_end-(20+id3v2_tag_size)) == NErr_Success)
					{
						bool has_header=false;

						while(NXFileRead(file, id3v2_header_data, 10, &bytes_read) == NErr_Success && bytes_read == 10 && NSID3v2_Header_Valid(id3v2_header_data) == NErr_Success)
						{
							// TODO: verify header
						}

						// TODO: it'd be better to memory map the file into place but this works for now
						id3v2_data = malloc(id3v2_tag_size);
						if (!id3v2_data)
						{
							// TODO: debatable what we should do here. 
							NSID3v2_Header_Destroy(id3v2_header);
							return NErr_OutOfMemory;
						}

						if (NXFileRead(file, id3v2_data, id3v2_tag_size, &bytes_read) == NErr_Success && bytes_read == id3v2_tag_size
							&& NSID3v2_Tag_Create(&id3v2, id3v2_header, id3v2_data, id3v2_tag_size) == NErr_Success)
						{
							uint64_t position = region_end-(20+id3v2_tag_size);
							region_end-=id3v2_tag_size+20;
							OwnID3v2(id3v2, position, id3v2_tag_size+20);
						}
					}

					free(id3v2_data);
				}
				NSID3v2_Header_Destroy(id3v2_header);
			}
		}
	}

	/* Read lyrics3 tag */
	uint8_t lyrics3_footer_data[32];
	if ((region_end-region_start) >= 15 
		&& NXFileSeek(file, region_end-15) == NErr_Success
		&& NXFileRead(file, lyrics3_footer_data, 15, &bytes_read) == NErr_Success && bytes_read == 15
		&& !memcmp(lyrics3_footer_data+6, "LYRICS200", 9))
	{
		uint32_t lyrics3_size = strtoul((const char *)lyrics3_footer_data, 0, 10);
		if (lyrics3_size)
		{
			uint8_t lyrics3_data[11];
			uint64_t length = lyrics3_size+15;
			uint64_t position = region_end-lyrics3_size;

			if (NXFileSeek(file, position) == NErr_Success
				&& NXFileRead(file, lyrics3_data, 11, &bytes_read) == NErr_Success && bytes_read == 11
				&& !memcmp(lyrics3_data, "LYRICSBEGIN", 11))
			{

				// TODO: read lyrics3 data (so at least we can re-write it)
				void *lyrics3 = (void *)malloc(lyrics3_size);
				if (!lyrics3)
					return NErr_OutOfMemory;

				if (NXFileSeek(file, position) == NErr_Success
					&& NXFileRead(file, lyrics3, lyrics3_size, &bytes_read) == NErr_Success && bytes_read == lyrics3_size)
				{
					OwnLyrics3(lyrics3, position, length);
					region_end -= length;
				}
			}
		}
	}

	/* Read APEv2 Tag */
	/* TODO: RE-TEST!!! */
	uint8_t apev2_footer_data[32];
	if ((region_end-region_start) >= 32
		&& NXFileSeek(file, region_end-32) == NErr_Success
		&& NXFileRead(file, apev2_footer_data, 32, &bytes_read) == NErr_Success && bytes_read == 32
		&& NSAPEv2_Header_Valid(apev2_footer_data) == NErr_Success)
	{
		uint32_t apev2_tag_size;
		nsapev2_header_t apev2_header;

		if (NSAPEv2_Header_Create(&apev2_header, apev2_footer_data, 32) == NErr_Success)
		{
			if (NSAPEv2_Header_TagSize(apev2_header, &apev2_tag_size) == NErr_Success)
			{
				uint64_t position = region_end - apev2_tag_size;

				if ((region_end-region_start) >= apev2_tag_size 
					&& NXFileSeek(file, region_end-apev2_tag_size) == NErr_Success)
				{
					// TODO: it'd be better to memory map the file into place but this works for now
					void *apev2_data = malloc(apev2_tag_size);
					if (!apev2_data)
					{
						// TODO: debatable what we should do here. 
						NSAPEv2_Header_Destroy(apev2_header);
						return NErr_OutOfMemory;
					}

					nsapev2_tag_t apev2;
					if (NXFileRead(file, apev2_data, apev2_tag_size, &bytes_read) == NErr_Success && bytes_read == apev2_tag_size
						&& NSAPEv2_Tag_Create(&apev2, apev2_header, apev2_data, apev2_tag_size) == NErr_Success)
					{
						OwnAPEv2(apev2, position, apev2_tag_size);
						region_end -= apev2_tag_size; 
					}
					free(apev2_data);
				}
			}
			NSAPEv2_Header_Destroy(apev2_header);
		}
	}

	end_position = region_end;
	content_length = region_end-region_start;
	start_position = region_start;
	NXFileLockRegion(file, region_start, region_end);
	NXFileSeek(file, 0);
	return NErr_Success;
}

