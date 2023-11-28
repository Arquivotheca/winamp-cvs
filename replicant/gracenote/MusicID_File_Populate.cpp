#include "api.h"
#include "MusicID_File_Populate.h"
#include "foundation/error.h"
#include <stdio.h>
#include "metadata/MetadataKeys.h"
#include "nswasabi/AutoCharNX.h"
#include "metadata/svc_metadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/Benchmark.h"

/*
#if defined(_WIN32)
static uint64_t Benchmark()
{
	return GetTickCount64();
}
#elif defined(__ANDROID__)
#include <time.h>
#include <android/log.h>
static uint64_t Benchmark()
{
	struct timespec ts;
	uint64_t count;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	count=(uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;
	return count;
}
#elif defined(__APPLE__)
#include <mach/mach_time.h>
static uint64_t Benchmark()
{
	uint64_t absoluteTime;
	static mach_timebase_info_data_t timeBase = {0,0};
	
	absoluteTime = mach_absolute_time();
	
	if (0 == timeBase.denom)
	{
		kern_return_t err = mach_timebase_info(&timeBase);
		if (0 != err)
			return 0;
	}
	uint64_t nanoTime = absoluteTime * timeBase.numer / timeBase.denom;
	return nanoTime/(1000*1000);
}
#endif

*/


//static uint64_t decode_start, decode_total=0, fingerprint_total=0;	// BENCHMARKING

class FingerprintCallback : public ifc_audio_decoder_callback::callback
{
public:
	int WASABICALL AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames);
	gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo;
	size_t frame_size;
};

int FingerprintCallback::AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames)
{
	gnsdk_error_t gn_error;
	gnsdk_bool_t fingerprint_acquired=0;
	//	decode_total+=(Benchmark() - decode_start);				// BENCHMARKING
	//uint64_t fingerprint_start = Benchmark();
	gn_error = gnsdk_musicidfile_fileinfo_fingerprint_write(gn_fileinfo, buffer, buffer_frames*frame_size, &fingerprint_acquired);
	//fingerprint_total += (Benchmark() - fingerprint_start);	// BENCHAMARKING
	if (gn_error != GNSDK_SUCCESS)
		printf("[%-40s] %08x %d\n", "gnsdk_musicidfile_fileinfo_fingerprint_write", gn_error, fingerprint_acquired);
	//decode_start = Benchmark();								// BENCHMARKING
	if (gn_error != 0)
		return NErr_Interrupted;

	if (fingerprint_acquired)
		return NErr_Interrupted;

	return NErr_Success;
}

enum
{
	INTEGER_FALLBACK=1,
};

static void FillMetadataField(AutoCharUTF8 &utf8, gnsdk_musicidfile_fileinfo_handle_t fileinfo, ifc_metadata *metadata, const char *display, int key, gnsdk_cstr_t gn_key, int flags=0)
{
	nx_string_t value;
	int ret = metadata->GetField(key, 0, &value);
	if (ret == NErr_Success && value)
	{
		utf8.Set(value);
		gnsdk_musicidfile_fileinfo_metadata_set(fileinfo, gn_key, utf8);
		NXStringRelease(value);
	}
	else if (ret == NErr_Unknown)
	{
		int64_t integer_value;
		ret = metadata->GetInteger(key, 0, &integer_value);
		if (ret == NErr_Success)
		{
			char temp[128];
			sprintf(temp, "%lld", integer_value);
			gnsdk_musicidfile_fileinfo_metadata_set(fileinfo, gn_key, temp);
		}
	}
}

int MusicID_File_Metadata(nx_uri_t filename, gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo)
{
	ifc_metadata *metadata;
	int ret = REPLICANT_API_METADATA->CreateMetadata(&metadata, filename);
	if (ret != NErr_Success)
		return ret;

		AutoCharUTF8 utf8;

		FillMetadataField(utf8, gn_fileinfo, metadata, "Album Artist", MetadataKeys::ALBUM_ARTIST, GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMARTIST);
		FillMetadataField(utf8, gn_fileinfo, metadata, "Artist", MetadataKeys::ARTIST, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKARTIST);
		FillMetadataField(utf8, gn_fileinfo, metadata, "Track", MetadataKeys::TRACK, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKNUMBER, INTEGER_FALLBACK);
		FillMetadataField(utf8, gn_fileinfo, metadata, "Disc", MetadataKeys::DISC, GNSDK_MUSICIDFILE_FILEINFO_VALUE_DISCNUMBER, INTEGER_FALLBACK);
		FillMetadataField(utf8, gn_fileinfo, metadata, "Album", MetadataKeys::ALBUM, GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMTITLE);
		FillMetadataField(utf8, gn_fileinfo, metadata, "Title", MetadataKeys::TITLE, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKTITLE);
		metadata->Release();
		return NErr_Success;
}

int MusicID_File_Fingerprint(nx_uri_t filename, gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo)
{
	/* specify our preferred format to decoding */
	ReferenceCountedPointer<ifc_audio_decoder_callback> decoder;
	nsaudio::Parameters parameters;
	parameters.sample_rate=44100;
	parameters.format_type = nsaudio::format_type_float;
	parameters.format_flags=nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters.bytes_per_sample=4;
	parameters.bits_per_sample=32;
	parameters.number_of_channels=2; 
	parameters.channel_layout=0;

	/* try to open decoder */
	int ret = REPLICANT_API_DECODE->CreateAudioDecoder_Callback(&decoder, filename, &parameters, 0);	
	if (ret != NErr_Success)
		return ret;

	/* verify parameters */
	if (parameters.format_flags != (nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED))
		return NErr_UnsupportedFormat;
	
	/* run decoder */
	FingerprintCallback fingerprint_callback;
	fingerprint_callback.gn_fileinfo = gn_fileinfo;
	gnsdk_error_t gn_error = gnsdk_musicidfile_fileinfo_fingerprint_begin(gn_fileinfo, parameters.sample_rate, parameters.bits_per_sample, parameters.number_of_channels);
	if (gn_error == GNSDK_SUCCESS)
	{
		fingerprint_callback.frame_size = parameters.bytes_per_sample*parameters.number_of_channels;
		//decode_start=Benchmark();			// BENCHMARKING
		ret = decoder->Decode(&fingerprint_callback);

// BENCHMARKING
//#if defined(_WIN32)
//		printf("Benchmark:\n Decode=%llu\n Fingerprint=%llu\n", decode_total, fingerprint_total);
//#elif defined(__ANDROID__)
//		static int counter = 1;
//		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "Benchmark #%d:\n Decode=%llu\n Fingerprint=%llu\n", counter++, decode_total/1000000, fingerprint_total/1000000);
//#elif defined(__APPLE__)
//		printf("Benchmark:\n Decode=%llu\n Fingerprint=%llu\n", decode_total, fingerprint_total);
//#endif

		if (ret != NErr_Interrupted)
		{
			gnsdk_musicidfile_fileinfo_fingerprint_end(gn_fileinfo);
			return ret;
		}
		else
		{
			return NErr_Success;
		}
	}
	else
	{
		return NErr_Error;
	}
}

int MusicID_File_Populate(gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo, nx_uri_t filename)
{
	AutoCharUTF8 utf8;
	utf8.Set(filename);
	gnsdk_musicidfile_fileinfo_metadata_set(gn_fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME, utf8);
	return NErr_Success;
}

static void WriteMetadata(ifc_metadata_editor *metadata, int key, gnsdk_gdo_handle_t gdo, gnsdk_cstr_t field)
{
	gnsdk_uint32_t count=0;
	gnsdk_manager_gdo_value_count(gdo, field, &count);
	for (gnsdk_uint32_t i=0;i<count;i++)
	{
		gnsdk_cstr_t value;
		gnsdk_error_t error = gnsdk_manager_gdo_value_get(gdo, field, i+1, &value);
		if (error == 0)
		{
			printf("%s: %s\n", field, value);
			if (key != -1)
			{
				nx_string_t nx_value;
				if (NXStringCreateWithUTF8(&nx_value, value) == NErr_Success)
				{
					metadata->SetField(key, i, nx_value);
					NXStringRelease(nx_value);
				}
			}
		}
		else
		{
			printf("%s: [error %08x]\n", field, error);
		}
	}
}
