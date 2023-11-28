#pragma once

// forward declarations
class MP4File;
class MP4Atom;

#include "mp4property.h"

class MP4Track {
public:
	MP4Track(MP4File* pFile, MP4Atom* pTrakAtom);

	virtual ~MP4Track();

	MP4TrackId GetId() {
		return m_trackId;
	}

	const char* GetType();

	void SetType(const char* type);

	MP4File* GetFile() {
		return m_pFile;
	}

	MP4Atom* GetTrakAtom() {
		return m_pTrakAtom;
	}

	void ReadSample(
		// input parameters
		MP4SampleId sampleId,
		// output parameters
		uint8_t** ppBytes, 
		uint32_t* pNumBytes, 
		MP4Timestamp* pStartTime = NULL, 
		MP4Duration* pDuration = NULL,
		MP4Duration* pRenderingOffset = NULL, 
		bool* pIsSyncSample = NULL);

	void WriteSample(
		const uint8_t* pBytes, 
		uint32_t numBytes,
		MP4Duration duration = 0,
		MP4Duration renderingOffset = 0, 
		bool isSyncSample = true);

	virtual void FinishWrite();

	uint64_t 	GetDuration();		// in track timeScale units
	uint32_t	GetTimeScale();
	uint32_t	GetNumberOfSamples();
	uint32_t	GetSampleSize(MP4SampleId sampleId);
	uint32_t	GetMaxSampleSize();
	uint64_t 	GetTotalOfSampleSizes();
	uint32_t	GetAvgBitrate();	// in bps
	uint32_t	GetMaxBitrate();	// in bps

	MP4Duration GetFixedSampleDuration();
	void		SetFixedSampleDuration(MP4Duration duration);

	void		GetSampleTimes(MP4SampleId sampleId,
					MP4Timestamp* pStartTime, MP4Duration* pDuration);

	bool		IsSyncSample(MP4SampleId sampleId);

	MP4SampleId GetSampleIdFromTime(
		MP4Timestamp when, 
		bool wantSyncSample = false);

	MP4ChunkId GetChunkIdFromTime(MP4Timestamp when);

	MP4Duration	GetSampleRenderingOffset(MP4SampleId sampleId);
	void		SetSampleRenderingOffset(MP4SampleId sampleId,
					MP4Duration renderingOffset);

	MP4EditId	AddEdit(
		MP4EditId editId = MP4_INVALID_EDIT_ID);

	void		DeleteEdit(
		MP4EditId editId);

	MP4Timestamp GetEditStart(
		MP4EditId editId);

	MP4Timestamp GetEditTotalDuration(
		MP4EditId editId);

	MP4SampleId GetSampleIdFromEditTime(
		MP4Timestamp editWhen, 
		MP4Timestamp* pStartTime = NULL, 
		MP4Duration* pDuration = NULL);

	// special operation for use during hint track packet assembly
	void ReadSampleFragment(
		MP4SampleId sampleId,
		uint32_t sampleOffset,
		uint16_t sampleLength,
		uint8_t* pDest);

	// special operations for use during optimization

	uint32_t GetNumberOfChunks();

	MP4Timestamp GetChunkTime(MP4ChunkId chunkId);

	void ReadChunk(MP4ChunkId chunkId, 
		uint8_t** ppChunk, uint32_t* pChunkSize, 
		MP4Timestamp* pStartTime = NULL, 
		MP4Duration* pDuration = NULL);

	void RewriteChunk(MP4ChunkId chunkId, 
		uint8_t* pChunk, uint32_t chunkSize);

protected:
	bool		InitEditListProperties();

	ns_error_t GetSampleFile(nx_file_t *sample_file, MP4SampleId sampleId);
	uint64_t	GetSampleFileOffset(MP4SampleId sampleId);
	uint32_t	GetSampleStscIndex(MP4SampleId sampleId);
	uint32_t	GetChunkStscIndex(MP4ChunkId chunkId);
	uint32_t	GetChunkSize(MP4ChunkId chunkId);
	uint32_t	GetSampleCttsIndex(MP4SampleId sampleId, 
					MP4SampleId* pFirstSampleId = NULL);
	MP4SampleId	GetNextSyncSample(MP4SampleId sampleId);

	void UpdateSampleSizes(MP4SampleId sampleId, 
		uint32_t numBytes);
	bool IsChunkFull(MP4SampleId sampleId);
	void UpdateSampleToChunk(MP4SampleId sampleId,
		 MP4ChunkId chunkId, uint32_t samplesPerChunk);
	void UpdateChunkOffsets(uint64_t chunkOffset);
	void UpdateSampleTimes(MP4Duration duration);
	void UpdateRenderingOffsets(MP4SampleId sampleId, MP4Duration renderingOffset);
	void UpdateSyncSamples(MP4SampleId sampleId, bool isSyncSample);

	MP4Atom* AddAtom(const char* parentName, const char* childName);

	void UpdateDurations(MP4Duration duration);
	MP4Duration ToMovieDuration(MP4Duration trackDuration);

	void UpdateModificationTimes();

	void WriteChunkBuffer();

	void CalculateBytesPerSample();
protected:
	MP4File*	m_pFile;
	MP4Atom* 	m_pTrakAtom;		// moov.trak[]
	MP4TrackId	m_trackId;			// moov.trak[].tkhd.trackId
	MP4StringProperty* m_pTypeProperty;	// moov.trak[].mdia.hdlr.handlerType

	uint32_t	m_lastStsdIndex;
	nx_file_t m_lastSampleFile;

	// for efficient construction of hint track packets
	MP4SampleId	m_cachedReadSampleId;
	uint8_t* 	m_pCachedReadSample;
	uint32_t	m_cachedReadSampleSize;

	// for writing
	MP4SampleId m_writeSampleId;
	MP4Duration m_fixedSampleDuration;
	uint8_t* 	m_pChunkBuffer;
	uint32_t	m_chunkBufferSize;
	uint32_t	m_chunkSamples;
	MP4Duration m_chunkDuration;

	// controls for chunking
	uint32_t 	m_samplesPerChunk;
	MP4Duration m_durationPerChunk;

	uint32_t       m_bytesPerSample;

	// controls for AMR chunking
	int		m_isAmr;
	uint8_t	m_curMode;

	MP4Integer32Property* m_pTimeScaleProperty;
	MP4IntegerProperty* m_pTrackDurationProperty;		// 32 or 64 bits
	MP4IntegerProperty* m_pMediaDurationProperty;		// 32 or 64 bits
	MP4IntegerProperty* m_pTrackModificationProperty;	// 32 or 64 bits
	MP4IntegerProperty* m_pMediaModificationProperty;	// 32 or 64 bits

	MP4Integer32Property* m_pStszFixedSampleSizeProperty;
	MP4Integer32Property* m_pStszSampleCountProperty;
	
	void SampleSizePropertyAddValue(uint32_t bytes);
	uint8_t m_stsz_sample_bits;
	bool m_have_stz2_4bit_sample;
	uint8_t m_stz2_4bit_sample_value;
	MP4IntegerProperty* m_pStszSampleSizeProperty;

	MP4Integer32Property* m_pStscCountProperty;
	MP4Integer32Property* m_pStscFirstChunkProperty;
	MP4Integer32Property* m_pStscSamplesPerChunkProperty;
	MP4Integer32Property* m_pStscSampleDescrIndexProperty;
	MP4Integer32Property* m_pStscFirstSampleProperty;

	MP4Integer32Property* m_pChunkCountProperty;
	MP4IntegerProperty*   m_pChunkOffsetProperty;		// 32 or 64 bits

	MP4Integer32Property* m_pSttsCountProperty;
	MP4Integer32Property* m_pSttsSampleCountProperty;
	MP4Integer32Property* m_pSttsSampleDeltaProperty;

	// for improve sequental timestamp index access
	uint32_t	m_cachedSttsIndex;
	MP4SampleId	m_cachedSttsSid;
	MP4Timestamp	m_cachedSttsElapsed;

	MP4Integer32Property* m_pCttsCountProperty;
	MP4Integer32Property* m_pCttsSampleCountProperty;
	MP4Integer32Property* m_pCttsSampleOffsetProperty;

	MP4Integer32Property* m_pStssCountProperty;
	MP4Integer32Property* m_pStssSampleProperty;

	MP4Integer32Property* m_pElstCountProperty;
	MP4IntegerProperty*   m_pElstMediaTimeProperty;		// 32 or 64 bits
	MP4IntegerProperty*   m_pElstDurationProperty;		// 32 or 64 bits
	MP4Integer16Property* m_pElstRateProperty;
	MP4Integer16Property* m_pElstReservedProperty;
};

MP4ARRAY_DECL(MP4Track, MP4Track*);

