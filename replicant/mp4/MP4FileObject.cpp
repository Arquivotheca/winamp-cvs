#include "MP4FileObject.h"
#include "foundation/error.h"
#include "nsmp4.h"


MP4FileObject::MP4FileObject()
{
	file_handle=MP4_INVALID_FILE_HANDLE;
}

MP4FileObject::~MP4FileObject()
{
	if (file_handle)
		MP4Close(file_handle);
}

void MP4FileObject::Initialize(nx_uri_t filename, MP4FileHandle file_handle)
{
	this->file_handle = file_handle;
	MP4MetadataBase::Initialize(filename, file_handle);
}

int MP4FileObject::MP4File_Free(void *buffer)
{
	MP4Free(buffer);
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_GetESConfiguration(TrackID track_number, uint8_t **buffer, uint32_t *buffer_size)
{
	if (MP4GetTrackESConfiguration(file_handle, track_number, buffer, buffer_size))
		return NErr_Success;
	else
		return NErr_Error;
}

int MP4FileObject::MP4File_Track_GetMaxSampleSize(TrackID track_number, uint32_t *max_sample_size)
{
	uint32_t ret=MP4GetTrackMaxSampleSize(file_handle, track_number);
	if (ret)
	{
		*max_sample_size = ret;
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

int MP4FileObject::MP4File_Track_ConvertFromTimestamp(TrackID track_number, Timestamp timestamp, double *seconds)
{
	*seconds = (double)MP4ConvertFromTrackTimestamp(file_handle, track_number, timestamp, MP4_USECS_TIME_SCALE) / (double)MP4_USECS_TIME_SCALE;
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_ConvertToDuration(TrackID track_number, double seconds, Duration *duration)
{
	MP4Duration local_duration = MP4ConvertToTrackDuration(file_handle, track_number, (uint64_t) (seconds * (double)MP4_USECS_TIME_SCALE), MP4_USECS_TIME_SCALE);
	if (local_duration == MP4_INVALID_DURATION)
		return NErr_Error;

	*duration=local_duration;
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_GetMediaDataName(TrackID track_number, const char **name)
{
	const char *media_data_name = MP4GetTrackMediaDataName(file_handle, track_number);
	if (!media_data_name)
		return NErr_Error;

	*name = media_data_name;
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_GetESDSObjectTypeID(TrackID track_number, uint8_t *type)
{
	*type = MP4GetTrackEsdsObjectTypeId(file_handle, track_number);
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_GetAudioMPEG4Type(TrackID track_number, uint8_t *type)
{
	*type = MP4GetTrackAudioMpeg4Type(file_handle, track_number);
	return NErr_Success;
}

int MP4FileObject::MP4File_Track_GetBytesProperty(TrackID track_number, const char *property_name, uint8_t **buffer, uint32_t *buffer_size)
{
		if (MP4GetTrackBytesProperty(file_handle, track_number, property_name, buffer, buffer_size))
		return NErr_Success;
	else
		return NErr_Error;
}
	/* ------------------------------------------------- */
int MP4FileObject::MP4File_Metadata_iTunes_FindFreeform(const char *name, const char *mean, metadata_itunes_atom_t *atom)
{
	return NSMP4_Metadata_iTunes_FindFreeform(file_handle, name, mean, (nsmp4_metadata_itunes_atom_t *)atom);
}

int MP4FileObject::MP4File_Metadata_iTunes_GetBinary(metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length)
{
	return  NSMP4_Metadata_iTunes_GetBinary(file_handle, (nsmp4_metadata_itunes_atom_t)atom, value, value_length);
}

int MP4FileObject::MP4File_Sample_Read(TrackID track_number, SampleID sample_number, uint8_t **bytes, uint32_t *bytes_length, Timestamp *start_time, Duration *duration, Duration *offset, int *is_sync)
{
	bool sync;
	if (MP4ReadSample(file_handle, track_number, sample_number, bytes, bytes_length, start_time, duration, offset, &sync))
	{
		if (is_sync)
			*is_sync = sync?1:0;
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

int MP4FileObject::MP4File_Sample_GetFromDuration(TrackID track_number, Duration duration, SampleID *sample_id)
{
	MP4SampleId local_sample_id;
	local_sample_id = MP4GetSampleIdFromTime(file_handle, track_number, duration);
	if (local_sample_id == MP4_INVALID_SAMPLE_ID)
		return NErr_Error;

	*sample_id = local_sample_id;
	return NErr_Success;
}
