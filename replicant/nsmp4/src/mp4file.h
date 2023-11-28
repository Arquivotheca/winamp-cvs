#pragma once

#include "mp4array.h"
#include "mp4track.h"
#include "mp4property.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"

// forward declarations
class MP4Track;
class MP4Atom;

class MP4Descriptor;
class MP4DescriptorProperty;
struct Virtual_IO;


class MP4File 
{
public: /* equivalent to MP4 library API */
	MP4File(uint32_t verbosity = 0);
	~MP4File();

	/* file operations */
	void Read(nx_uri_t filename);
	ns_error_t ReadFile(nx_uri_t filename, nx_file_t file);
	
	void Create(nx_uri_t filename, uint32_t flags, 
		    int add_ftyp = 1, int add_iods = 1,
		    char* majorBrand = NULL, 
		    uint32_t minorVersion = 0, char** supportedBrands = NULL, 
		    uint32_t supportedBrandsCount = 0);
	void Modify(nx_uri_t filename);
	void Optimize(nx_uri_t orgFileName, nx_uri_t newFileName = NULL);
	void Dump(FILE* pDumpFile = NULL, bool dumpImplicits = false);
	void Close();
	void CloseFile();
	int GetStat(nx_file_stat_t stats);
	/* library property per file */

	uint32_t GetVerbosity() {
		return m_verbosity;
	}
	void SetVerbosity(uint32_t verbosity) {
		m_verbosity = verbosity;
	}

	bool Use64Bits(const char *atomName);
	void Check64BitStatus(const char *atomName);
	/* file properties */

	uint64_t GetIntegerProperty(const char* name);
	float GetFloatProperty(const char* name);
	const char* GetStringProperty(const char* name);
	void GetBytesProperty(const char* name,
		uint8_t** ppValue, uint32_t* pValueSize);

	void SetIntegerProperty(const char* name, uint64_t value);
	void SetFloatProperty(const char* name, float value);
	void SetStringProperty(const char* name, const char* value);
	void SetBytesProperty(const char* name, 
		const uint8_t* pValue, uint32_t valueSize);

	// file level convenience functions

	MP4Duration GetDuration();
	void SetDuration(MP4Duration value);

	uint32_t GetTimeScale();
	void SetTimeScale(uint32_t value);

	uint8_t GetODProfileLevel();
	void SetODProfileLevel(uint8_t value);

	uint8_t GetSceneProfileLevel();
	void SetSceneProfileLevel(uint8_t value);

	uint8_t GetVideoProfileLevel();
	void SetVideoProfileLevel(uint8_t value);

	uint8_t GetAudioProfileLevel();
	void SetAudioProfileLevel(uint8_t value);

	uint8_t GetGraphicsProfileLevel();
	void SetGraphicsProfileLevel(uint8_t value);

	const char* GetSessionSdp();
	void SetSessionSdp(const char* sdpString);
	void AppendSessionSdp(const char* sdpString);

	/* track operations */

	MP4TrackId AddTrack(const char* type, uint32_t timeScale = 1000);
	void DeleteTrack(MP4TrackId trackId);

	uint32_t GetNumberOfTracks(const char* type = NULL, uint8_t subType = 0);

	MP4TrackId AllocTrackId();
	MP4TrackId FindTrackId(uint16_t trackIndex, 
		const char* type = NULL, uint8_t subType = 0);
	uint16_t FindTrackIndex(MP4TrackId trackId);
	int FindTrakAtomIndex(MP4TrackId trackId, uint16_t *index);

	/* track properties */
	MP4Atom *FindTrackAtom(MP4TrackId trackId, const char *name);
	uint64_t GetTrackIntegerProperty(
		MP4TrackId trackId, const char* name);
	float GetTrackFloatProperty(
		MP4TrackId trackId, const char* name);
	const char* GetTrackStringProperty(
		MP4TrackId trackId, const char* name);
	void GetTrackBytesProperty(
		MP4TrackId trackId, const char* name,
		uint8_t** ppValue, uint32_t* pValueSize);

	void SetTrackIntegerProperty(
		MP4TrackId trackId, const char* name, int64_t value);
	void SetTrackFloatProperty(
		MP4TrackId trackId, const char* name, float value);
	void SetTrackStringProperty(
		MP4TrackId trackId, const char* name, const char* value);
	void SetTrackBytesProperty(
		MP4TrackId trackId, const char* name, 
		const uint8_t* pValue, uint32_t valueSize);

	/* sample operations */

	uint32_t GetSampleSize(MP4TrackId trackId, MP4SampleId sampleId);

	uint32_t GetTrackMaxSampleSize(MP4TrackId trackId);

	MP4SampleId GetSampleIdFromTime(MP4TrackId trackId, 
		MP4Timestamp when, bool wantSyncSample = false);

	MP4ChunkId GetChunkIdFromTime(MP4TrackId trackId, MP4Timestamp when);

	MP4Timestamp GetSampleTime(
		MP4TrackId trackId, MP4SampleId sampleId);

	MP4Duration GetSampleDuration(
		MP4TrackId trackId, MP4SampleId sampleId);

	MP4Duration GetSampleRenderingOffset(
		MP4TrackId trackId, MP4SampleId sampleId);

	bool GetSampleSync(
		MP4TrackId trackId, MP4SampleId sampleId);

	void ReadSample(
		// input parameters
		MP4TrackId trackId, 
		MP4SampleId sampleId,
		// output parameters
		uint8_t** ppBytes, 
		uint32_t* pNumBytes, 
		MP4Timestamp* pStartTime = NULL, 
		MP4Duration* pDuration = NULL,
		MP4Duration* pRenderingOffset = NULL, 
		bool* pIsSyncSample = NULL);

	void ReadChunk(
		// input parameters
		MP4TrackId trackId, 
		MP4ChunkId sampleId,
		// output parameters
		uint8_t** ppBytes, 
		uint32_t* pNumBytes, 
		MP4Timestamp* pStartTime = NULL, 
		MP4Duration* pDuration = NULL);

	void WriteSample(
		MP4TrackId trackId,
		const uint8_t* pBytes, 
		uint32_t numBytes,
		MP4Duration duration = 0,
		MP4Duration renderingOffset = 0, 
		bool isSyncSample = true);

	void SetSampleRenderingOffset(
		MP4TrackId trackId, 
		MP4SampleId sampleId,
		MP4Duration renderingOffset);

	/* track level convenience functions */

	MP4TrackId AddSystemsTrack(const char* type);

	MP4TrackId AddODTrack();

	MP4TrackId AddSceneTrack();

	MP4TrackId AddAudioTrack(
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		uint8_t audioType);

	MP4TrackId AddEncAudioTrack( // ismacryp
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		uint8_t  audioType,
                uint32_t scheme_type,
                uint16_t scheme_version,
                uint8_t  key_ind_len,
                uint8_t  iv_len, 
                bool      selective_enc,
                const char  *kms_uri,
		bool      use_ismacryp);

	void SetAmrVendor(
			MP4TrackId trackId,
			uint32_t vendor);
	
	void SetAmrDecoderVersion(
			MP4TrackId trackId,
			uint8_t decoderVersion);
	
	void SetAmrModeSet(
			MP4TrackId trackId,
			uint16_t modeSet);
	uint16_t GetAmrModeSet(MP4TrackId trackId);

	MP4TrackId AddAmrAudioTrack(
			uint32_t timeScale,
			uint16_t modeSet,
			uint8_t modeChangePeriod,
			uint8_t framesPerSample,
			bool isAmrWB);

	MP4TrackId AddHrefTrack(uint32_t timeScale,
				MP4Duration sampleDuration,
				const char *base_url);

	MP4TrackId AddMP4VideoTrack(
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		uint16_t width, 
		uint16_t height, 
		uint8_t videoType);

	MP4TrackId AddEncVideoTrack( // ismacryp
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		uint16_t width, 
		uint16_t height, 
		uint8_t  videoType,
		mp4v2_ismacrypParams *icPp,
		const char *oFormat);

	void SetH263Vendor(
			MP4TrackId trackId,
			uint32_t vendor);
	
	void SetH263DecoderVersion(
			MP4TrackId trackId,
			uint8_t decoderVersion);
	
	void SetH263Bitrates(
			MP4TrackId,
			uint32_t avgBitrate,
			uint32_t maxBitrate);
	
	MP4TrackId AddH263VideoTrack(
			uint32_t timeScale,
			MP4Duration sampleDuration,
			uint16_t width,
			uint16_t height,
			uint8_t h263Level,
			uint8_t h263Profile,
			uint32_t avgBitrate,
			uint32_t maxBitrate);

	MP4TrackId AddH264VideoTrack(
				     uint32_t timeScale,
				     MP4Duration sampleDuration,
				     uint16_t width,
				     uint16_t height,
				     uint8_t AVCProfileIndication,
				     uint8_t profile_compat,
				     uint8_t AVCLevelIndication,
				     uint8_t sampleLenFieldSizeMinusOne);

	MP4TrackId AddEncH264VideoTrack(
				     uint32_t timeScale,
				     MP4Duration sampleDuration,
				     uint16_t width,
				     uint16_t height,
					MP4Atom *srcAtom,
					mp4v2_ismacrypParams *icPp);

	void AddH264SequenceParameterSet(MP4TrackId trackId,
					 const uint8_t *pSequence,
					 uint16_t sequenceLen);
	void AddH264PictureParameterSet(MP4TrackId trackId,
					const uint8_t *pPicture,
					uint16_t pictureLen);
	MP4TrackId AddHintTrack(MP4TrackId refTrackId);

	MP4TrackId AddTextTrack(MP4TrackId refTrackId);
	MP4TrackId AddChapterTextTrack(MP4TrackId refTrackId, uint32_t timescale = 0); 
 void AddChapter(MP4TrackId chapterTrackId, 
 MP4Duration chapterDuration, 
 uint32_t chapterNr, 
 const char * chapterTitle = 0); 
 void AddChapter(MP4Timestamp chapterStart, 
 const char * chapterTitle = 0); 
 void ConvertChapters(bool toQT = true); 
 void DeleteChapters(MP4TrackId chapterTrackId = 0, bool deleteQT = true); 
 void GetChaptersList(MP4Chapters_t ** chapterList, 
 uint32_t * chapterCount, 
 bool getQT = true); 
 MP4TrackId FindChapterTrack(char * trackName = 0, int trackNameSize = 0); 
 MP4TrackId FindChapterReferenceTrack(MP4TrackId chapterTrackId, char * trackName = 0, size_t trackNameSize = 0); 
 
	MP4SampleId GetTrackNumberOfSamples(MP4TrackId trackId);
	MP4ChunkId GetTrackNumberOfChunks(MP4TrackId trackId);

	const char* GetTrackType(MP4TrackId trackId);

	const char *GetTrackMediaDataName(MP4TrackId trackId);
	bool GetTrackMediaDataOriginalFormat(MP4TrackId trackId,
		char *originalFormat, uint32_t buflen);
	MP4Duration GetTrackDuration(MP4TrackId trackId);

	uint32_t GetTrackTimeScale(MP4TrackId trackId);
	void SetTrackTimeScale(MP4TrackId trackId, uint32_t value);

	// replacement to GetTrackAudioType and GetTrackVideoType	
	uint8_t GetTrackEsdsObjectTypeId(MP4TrackId trackId);

	uint8_t GetTrackAudioMpeg4Type(MP4TrackId trackId);

	MP4Duration GetTrackFixedSampleDuration(MP4TrackId trackId);

	double GetTrackVideoFrameRate(MP4TrackId trackId);
	
	int GetTrackAudioChannels(MP4TrackId trackId);
	void GetTrackESConfiguration(MP4TrackId trackId, 
		uint8_t** ppConfig, uint32_t* pConfigSize);
	void SetTrackESConfiguration(MP4TrackId trackId, 
		const uint8_t* pConfig, uint32_t configSize);

	void GetTrackVideoMetadata(MP4TrackId trackId, 
		uint8_t** ppConfig, uint32_t* pConfigSize);
	void GetTrackH264SeqPictHeaders(MP4TrackId trackId, 
					uint8_t ***pSeqHeader,
					uint32_t **pSeqHeaderSize,
					uint8_t ***pPictHeader,
					uint32_t **pPictHeaderSize);
	const char* GetHintTrackSdp(MP4TrackId hintTrackId);
	void SetHintTrackSdp(MP4TrackId hintTrackId, const char* sdpString);
	void AppendHintTrackSdp(MP4TrackId hintTrackId, const char* sdpString);

	// 3GPP specific functions
	void MakeFtypAtom(char* majorBrand, 
			  uint32_t minorVersion, 
			  char** supportedBrands, 
			  uint32_t supportedBrandsCount);
	void Make3GPCompliant(nx_uri_t fileName, 
			      char* majorBrand, 
			      uint32_t minorVersion, 
			      char** supportedBrands, 
			      uint32_t supportedBrandsCount, 
			      bool deleteIodsAtom);

	// ISMA specific functions

       // true if media track encrypted according to ismacryp
       bool IsIsmaCrypMediaTrack(MP4TrackId trackId);
	
	void MakeIsmaCompliant(bool addIsmaComplianceSdp = true);

	void CreateIsmaIodFromParams(
		uint8_t videoProfile,
		uint32_t videoBitrate,
		uint8_t* videoConfig,
		uint32_t videoConfigLength,
		uint8_t audioProfile,
		uint32_t audioBitrate,
		uint8_t* audioConfig,
		uint32_t audioConfigLength,
		uint8_t** ppBytes,
		uint64_t* pNumBytes);

	// time convenience functions

	uint64_t ConvertFromMovieDuration(
		MP4Duration duration,
		uint32_t timeScale);

	uint64_t ConvertFromTrackTimestamp(
		MP4TrackId trackId, 
		MP4Timestamp timeStamp,
		uint32_t timeScale);

	MP4Timestamp ConvertToTrackTimestamp(
		MP4TrackId trackId, 
		uint64_t timeStamp,
		uint32_t timeScale);

	uint64_t ConvertFromTrackDuration(
		MP4TrackId trackId, 
		MP4Duration duration,
		uint32_t timeScale);

	MP4Duration ConvertToTrackDuration(
		MP4TrackId trackId, 
		uint64_t duration,
		uint32_t timeScale);

	// specialized operations

	void GetHintTrackRtpPayload(
		MP4TrackId hintTrackId,
		char** ppPayloadName = NULL,
		uint8_t* pPayloadNumber = NULL,
		uint16_t* pMaxPayloadSize = NULL,
		char **ppEncodingParams = NULL);

	void SetHintTrackRtpPayload(
		MP4TrackId hintTrackId,
		const char* payloadName,
		uint8_t* pPayloadNumber,
		uint16_t maxPayloadSize,
		const char *encoding_params,
		bool include_rtp_map,
		bool include_mpeg4_esid);

	MP4TrackId GetHintTrackReferenceTrackId(
		MP4TrackId hintTrackId);

	void ReadRtpHint(
		MP4TrackId hintTrackId,
		MP4SampleId hintSampleId,
		uint16_t* pNumPackets = NULL);

	uint16_t GetRtpHintNumberOfPackets(
		MP4TrackId hintTrackId);

	int8_t GetRtpPacketBFrame(
		MP4TrackId hintTrackId,
		uint16_t packetIndex);

	int32_t GetRtpPacketTransmitOffset(
		MP4TrackId hintTrackId,
		uint16_t packetIndex);

	void ReadRtpPacket(
		MP4TrackId hintTrackId,
		uint16_t packetIndex,
		uint8_t** ppBytes, 
		uint32_t* pNumBytes,
		uint32_t ssrc = 0,
		bool includeHeader = true,
		bool includePayload = true);

	MP4Timestamp GetRtpTimestampStart(
		MP4TrackId hintTrackId);

	void SetRtpTimestampStart(
		MP4TrackId hintTrackId,
		MP4Timestamp rtpStart);

	void AddRtpHint(
		MP4TrackId hintTrackId,
		bool isBframe, 
		uint32_t timestampOffset);

	void AddRtpPacket(
		MP4TrackId hintTrackId, 
		bool setMbit,
		int32_t transmitOffset);

	void AddRtpImmediateData(
		MP4TrackId hintTrackId,
		const uint8_t* pBytes,
		uint32_t numBytes);

	void AddRtpSampleData(
		MP4TrackId hintTrackId,
		MP4SampleId sampleId,
		uint32_t dataOffset,
		uint32_t dataLength);

	void AddRtpESConfigurationPacket(
		MP4TrackId hintTrackId);

	void WriteRtpHint(
		MP4TrackId hintTrackId,
		MP4Duration duration,
		bool isSyncSample);

	uint8_t AllocRtpPayloadNumber();

	// edit list related

	char* MakeTrackEditName(
		MP4TrackId trackId,
		MP4EditId editId,
		const char* name);

	MP4EditId AddTrackEdit(
		MP4TrackId trackId,
		MP4EditId editId = MP4_INVALID_EDIT_ID);

	void DeleteTrackEdit(
		MP4TrackId trackId,
		MP4EditId editId);

	uint32_t GetTrackNumberOfEdits(
		MP4TrackId trackId);

	MP4Timestamp GetTrackEditStart(
		MP4TrackId trackId,
		MP4EditId editId);

	MP4Duration GetTrackEditTotalDuration(
		MP4TrackId trackId,
		MP4EditId editId);

	MP4Timestamp GetTrackEditMediaStart(
		MP4TrackId trackId,
		MP4EditId editId);

	void SetTrackEditMediaStart(
		MP4TrackId trackId,
		MP4EditId editId,
		MP4Timestamp startTime);

	MP4Duration GetTrackEditDuration(
		MP4TrackId trackId,
		MP4EditId editId);

	void SetTrackEditDuration(
		MP4TrackId trackId,
		MP4EditId editId,
		MP4Duration duration);

	bool GetTrackEditDwell(
		MP4TrackId trackId,
		MP4EditId editId);

	void SetTrackEditDwell(
		MP4TrackId trackId,
		MP4EditId editId,
		bool dwell);

	MP4SampleId GetSampleIdFromEditTime(
		MP4TrackId trackId,
		MP4Timestamp when,
		MP4Timestamp* pStartTime = NULL,
		MP4Duration* pDuration = NULL);

	/* iTunes metadata handling */
 protected:
	bool CreateMetadataAtom(const char* name);
 public:
	// these are public to remove a lot of unnecessary routines
	bool DeleteMetadataAtom(const char* name, bool try_udta = false);
	
	bool MetadataDelete(void);

	bool SetMetadataUint8(const char *atom, uint8_t compilation);
	
	/* set metadata */
	bool SetMetadataTrack(uint16_t track, uint16_t totalTracks);
	bool SetMetadataDisk(uint16_t disk, uint16_t totalDisks);
	bool SetMetadataGenre(const char *value);
	bool SetMetadataCoverArt(uint8_t *coverArt, uint32_t size);

	/* get metadata */
	bool GetMetadataCoverArt(uint8_t **coverArt, uint32_t* size,
				 uint32_t index = 0);
	uint32_t GetMetadataCoverArtCount(void);

	/* delete metadata */
	bool DeleteMetadataGenre();

	/* iTunes metadata */
	int Metadata_iTunes_Create(MP4Atom **atom); // forces creation of an iTunes-style metadata atom
	int Metadata_iTunes_Enumerate(uint32_t index, MP4Atom **atom);
	int Metadata_iTunes_FindKey(const char *key, MP4Atom **atom);
  int Metadata_iTunes_NewKey(const char *key, MP4Atom **atom, uint32_t flags);
	int Metadata_iTunes_EnumerateKey(const char *key, uint32_t index, MP4Atom **atom);
	int Metadata_iTunes_FindFreeform(const char *name, const char *mean, MP4Atom **atom);
	int Metadata_iTunes_NewFreeform(const char *name, const char *mean, MP4Atom **atom, uint32_t flags);
	int Metadata_iTunes_GetInformation(MP4Atom *metadata_atom, char atom[5], uint32_t *flags);	

	int Metadata_iTunes_GetString(MP4Atom *metadata_atom, nx_string_t *value);
	int Metadata_iTunes_GetBinary(MP4Atom *metadata_atom, const uint8_t **value, size_t *length);
	int Metadata_iTunes_GetSigned(MP4Atom *metadata_atom, int64_t *value);
	int Metadata_iTunes_GetUnsigned(MP4Atom *metadata_atom, uint64_t *value);
	int Metadata_iTunes_GetFreeform(MP4Atom *metadata_atom, nx_string_t *name, nx_string_t *mean);

	int Metadata_iTunes_SetString(MP4Atom *metadata_atom, nx_string_t value);
	int Metadata_iTunes_SetUnsigned(MP4Atom *metadata_atom, uint64_t value, size_t byte_count);
	int Metadata_iTunes_SetBinary(MP4Atom *metadata_atom, const void *data, size_t length);

	int Metadata_iTunes_DeleteAtom(MP4Atom *metadata_atom);

/* 3GP metadata */
	bool Get3GPMetadataString(const char *atom, uint16_t **value);
	bool Set3GPMetadataString(const char *atom, const uint16_t *value);
	bool Get3GPMetadataInteger(const char *atom, uint64_t *value);
	bool Set3GPMetadataInteger(const char *atom, uint64_t value);
	bool Delete3GPMetadataAtom(const char* name);

	/* end of MP4 API */

	/* "protected" interface to be used only by friends in library */

	uint64_t GetPosition(nx_file_t pFile = NULL);
	void SetPosition(uint64_t pos, nx_file_t pFile = NULL);

	uint64_t GetSize();

	void ReadBytes(
		uint8_t* pBytes, uint32_t numBytes, nx_file_t pFile = NULL);
	uint64_t ReadUInt(uint8_t size);
	uint8_t ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt24();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();
	float ReadFixed16();
	float ReadFixed32();
	float ReadFloat();
	char* ReadString();
	uint16_t *ReadUnicodeString();
	char* ReadCountedString(
		uint8_t charSize = 1, bool allowExpandedCount = false);
	uint64_t ReadBits(uint8_t numBits);
	void FlushReadBits();
	uint32_t ReadMpegLength();

	void PeekByte(uint8_t *pByte);

	void WriteBytes(uint8_t* pBytes, uint32_t numBytes, nx_file_t pFile = NULL);
	void WriteUInt(uint64_t value, uint8_t bytes);
	void WriteUInt8(uint8_t value);
	void WriteUInt16(uint16_t value);
	void WriteUInt24(uint32_t value);
	void WriteUInt32(uint32_t value);
	void WriteUInt64(uint64_t value);
	void WriteFixed16(float value);
	void WriteFixed32(float value);
	void WriteFloat(float value);
	void WriteString(char* string);
	void WriteUnicodeString(const uint16_t *string);
	void WriteCountedString(char* string, 
		uint8_t charSize = 1, bool allowExpandedCount = false);
	void WriteBits(uint64_t bits, uint8_t numBits);
	void PadWriteBits(uint8_t pad = 0);
	void FlushWriteBits();
	void WriteMpegLength(uint32_t value, bool compact = false);

	void EnableMemoryBuffer(
		uint8_t* pBytes = NULL, uint64_t numBytes = 0);
	void DisableMemoryBuffer(
		uint8_t** ppBytes = NULL, uint64_t* pNumBytes = NULL);

	nx_file_FILE_flags_t GetMode() {
		return m_mode;
	}

	MP4Track* GetTrack(MP4TrackId trackId);

	void UpdateDuration(MP4Duration duration);

	MP4Atom* FindAtom(const char* name);

	MP4Atom* AddChildAtom(
		const char* parentName, 
		const char* childName);

	MP4Atom* AddChildAtom(
		MP4Atom* pParentAtom, 
		const char* childName);

	MP4Atom* InsertChildAtom(
		const char* parentName, 
		const char* childName, 
		uint32_t index);

	MP4Atom* InsertChildAtom(
		MP4Atom* pParentAtom, 
		const char* childName, 
		uint32_t index);

	MP4Atom* AddDescendantAtoms(
		const char* ancestorName, 
		const char* childName);

	MP4Atom* AddDescendantAtoms(
		MP4Atom* pAncestorAtom,
		const char* childName);

protected:
	void Open(nx_file_FILE_flags_t fmode);
	ns_error_t OpenFile(nx_file_t file, nx_file_FILE_flags_t fmode);
	
	void ReadFromFile();
	void GenerateTracks();
	void BeginWrite();
	void FinishWrite();
	void CacheProperties();
	void RewriteMdat(nx_file_t pReadFile, nx_file_t pWriteFile);
	bool ShallHaveIods();

	void ProtectWriteOperation(const char* where);

	void FindIntegerProperty(const char* name, 
		MP4Property** ppProperty, uint32_t* pIndex = NULL);
	void FindFloatProperty(const char* name, 
		MP4Property** ppProperty, uint32_t* pIndex = NULL);
	void FindStringProperty(const char* name, 
		MP4Property** ppProperty, uint32_t* pIndex = NULL);
	void FindBytesProperty(const char* name, 
		MP4Property** ppProperty, uint32_t* pIndex = NULL);

	bool FindProperty(const char* name,
		MP4Property** ppProperty, uint32_t* pIndex = NULL);

	MP4TrackId AddVideoTrackDefault(
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		uint16_t width, 
		uint16_t height, 
		const char *videoType);
	MP4TrackId AddCntlTrackDefault(
		uint32_t timeScale, 
		MP4Duration sampleDuration,
		const char *videoType);
	void AddTrackToIod(MP4TrackId trackId);

	void RemoveTrackFromIod(MP4TrackId trackId, bool shallHaveIods = true);

	void AddTrackToOd(MP4TrackId trackId);

	void RemoveTrackFromOd(MP4TrackId trackId);

	void GetTrackReferenceProperties(const char* trefName,
		MP4Property** ppCountProperty, MP4Property** ppTrackIdProperty);

	void AddTrackReference(const char* trefName, MP4TrackId refTrackId);

	uint32_t FindTrackReference(const char* trefName, MP4TrackId refTrackId);

	void RemoveTrackReference(const char* trefName, MP4TrackId refTrackId);

	void AddDataReference(MP4TrackId trackId, const char* url);

	char* MakeTrackName(MP4TrackId trackId, const char* name);

	uint8_t ConvertTrackTypeToStreamType(const char* trackType);

	void CreateIsmaIodFromFile(
		MP4TrackId odTrackId,
		MP4TrackId sceneTrackId,
		MP4TrackId audioTrackId, 
		MP4TrackId videoTrackId,
		uint8_t** ppBytes,
		uint64_t* pNumBytes);

	void CreateESD(
		MP4DescriptorProperty* pEsProperty,
		uint32_t esid,
		uint8_t objectType,
		uint8_t streamType,
		uint32_t bufferSize,
		uint32_t bitrate,
		const uint8_t* pConfig,
		uint32_t configLength,
		char* url);

	void CreateIsmaODUpdateCommandFromFileForFile(
		MP4TrackId odTrackId,
		MP4TrackId audioTrackId, 
		MP4TrackId videoTrackId,
		uint8_t** ppBytes,
		uint64_t* pNumBytes);

	void CreateIsmaODUpdateCommandFromFileForStream(
		MP4TrackId audioTrackId, 
		MP4TrackId videoTrackId,
		uint8_t** ppBytes,
		uint64_t* pNumBytes);

	void CreateIsmaODUpdateCommandForStream(
		MP4DescriptorProperty* pAudioEsdProperty, 
		MP4DescriptorProperty* pVideoEsdProperty,
		uint8_t** ppBytes,
		uint64_t* pNumBytes);

	void CreateIsmaSceneCommand(
		bool hasAudio,
		bool hasVideo,
		uint8_t** ppBytes, 
		uint64_t* pNumBytes);

protected:
	nx_uri_t m_filename;
	nx_file_stat_s m_file_stats;
	nx_file_t m_file;
	uint64_t		m_orgFileSize;
	uint64_t		m_fileSize;
	MP4Atom *m_pRootAtom;
	MP4Integer32Array m_trakIds;
	MP4TrackArray	m_pTracks;
	MP4TrackId		m_odTrackId;
	uint32_t		m_verbosity;
	nx_file_FILE_flags_t m_mode;
	uint32_t               m_createFlags;
	bool			m_useIsma;

	// cached properties
	MP4IntegerProperty*		m_pModificationProperty;
	MP4Integer32Property*	m_pTimeScaleProperty;
	MP4IntegerProperty*		m_pDurationProperty;

	// read/write in memory
	uint8_t*	m_memoryBuffer;
	uint64_t	m_memoryBufferPosition;
	uint64_t	m_memoryBufferSize;

	// bit read/write buffering
	uint8_t	m_numReadBits;
	uint8_t	m_bufReadBits;
	uint8_t	m_numWriteBits;
	uint8_t	m_bufWriteBits;

	char m_trakName[1024];
	char *m_editName;
};

