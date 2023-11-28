#pragma once

#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "../Agave/DecodeFile/api_decodefile.h"

#include "ExtendedRead.h"
#include "main.h"
#include "WMHandler.h"
#include "../burnlib/manager.h"



class DRMBurnManager : public WMHandler
{
public:
	DRMBurnManager(void);
	~DRMBurnManager();
	void SetDecodeAPI(api_decodefile *decoderAPI);
	api_decodefile *GetDecodeAPI(void);
	void SetFiles(size_t numFiles, const wchar_t **filenames, BurnManagerCallback *callback);
	ifc_audiostream *CreateDecoder(const wchar_t *filename);
	void CloseDecoder(ifc_audiostream *decoder);
	void CancelBurn();
	void BurnFinished();
	void InitPlaylistBurn();

private:
	api_decodefile *decodeFile;
	IWMSyncReader *reader;
	IWMReaderPlaylistBurn *playlistBurner;
	bool internalDecoder;
	WMCallback wmCallback;
	size_t numFiles;
	BurnManagerCallback *callback;
	WRESULT *results;
	bool drmStarted;
};