#include "main.h"
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "../Agave/DecodeFile/api_decodefile.h"

#include "BurnManager.h"
#include "ExtendedRead.h"

#ifdef _DEBUG
#define BURNMGR_SHOW_MSGBOX 
#endif

BurnManager::BurnManager()
{
	context = new DRMBurnManager;
}

BurnManager::~BurnManager()
{
	delete reinterpret_cast<DRMBurnManager *>(context);
}

void BurnManager::SetDecodeAPI(api_decodefile *decoderAPI)
{
	reinterpret_cast<DRMBurnManager *>(context)->SetDecodeAPI(decoderAPI);
}
api_decodefile *BurnManager::GetDecodeAPI(void)
{
	return reinterpret_cast<DRMBurnManager *>(context)->GetDecodeAPI();
}
void BurnManager::SetFiles(size_t numFiles, const wchar_t **filenames, BurnManagerCallback *callback)
{
	reinterpret_cast<DRMBurnManager *>(context)->SetFiles(numFiles, filenames, callback);
}
ifc_audiostream *BurnManager::CreateDecoder(const wchar_t *filename)
{
	return reinterpret_cast<DRMBurnManager *>(context)->CreateDecoder(filename);
}
void BurnManager::CloseDecoder(ifc_audiostream *decoder)
{
	reinterpret_cast<DRMBurnManager *>(context)->CloseDecoder(decoder);
}
void BurnManager::CancelBurn()
{
	reinterpret_cast<DRMBurnManager *>(context)->CancelBurn();
}
void BurnManager::BurnFinished()
{
	reinterpret_cast<DRMBurnManager *>(context)->BurnFinished();
}

DRMBurnManager::DRMBurnManager(void) : decodeFile(NULL), reader(0), internalDecoder(false), playlistBurner(0),
		results(0), drmStarted(false)
{
	wmCallback >> this;
	if (FAILED(WMCreateSyncReader(NULL, WMT_RIGHT_COPY_TO_CD, &reader))
	    || FAILED(reader->QueryInterface(&playlistBurner)))
	{
		// just in case queryinterface sets them to a weird value
		playlistBurner = 0;
		reader = 0;
	}

}

DRMBurnManager::~DRMBurnManager()
{
	if (playlistBurner)
		playlistBurner->Release();
	if (reader)
		reader->Release();
}

void DRMBurnManager::SetDecodeAPI(api_decodefile *decoderAPI)
{
	decodeFile = decoderAPI;
}

api_decodefile *DRMBurnManager::GetDecodeAPI(void)
{
	return decodeFile;
}

void DRMBurnManager::SetFiles(size_t _numFiles, const wchar_t **filenames, BurnManagerCallback *_callback)
{
	numFiles = _numFiles;
	callback = _callback;

	results = new WRESULT[numFiles];
	ZeroMemory(results, sizeof(WRESULT)*numFiles);
	for (size_t i = 0;i != numFiles;i++)
	{
		if (!PathFileExists(filenames[i]))
			results[i] = BURN_FILE_NOT_FOUND;
		else if (!decodeFile->DecoderExists(filenames[i]))
			results[i] = BURN_NO_DECODER;
	}
#ifdef BURNMGR_SHOW_MSGBOX
	wchar_t big[65536] = L"";
	for (size_t i = 0;i != numFiles;i++)
	{
		wsprintf(big + lstrlen(big), L"%i: %s\n", i, filenames[i]);

	}
	MessageBox(NULL, big, L"in_wm burn", MB_OK);
#endif

	if (playlistBurner) 
	{
		drmStarted=true; // have to set this before so we don't have a race condition
		if (FAILED(playlistBurner->InitPlaylistBurn(numFiles, (WCHAR **)filenames, &wmCallback, 0)))
			drmStarted=false;
	}
	else // no playlist burner is OK, we just won't be able to open a decoder for DRM tracks 
	{
		callback->OnLicenseCallback(numFiles, results);
		delete[] results;
	}
	
	
}

void DRMBurnManager::InitPlaylistBurn()
{
	HRESULT *drmResults = new HRESULT[numFiles];
	//ZeroMemory(results, sizeof(HRESULT)*numFiles);
	HRESULT hr = playlistBurner->GetInitResults(numFiles, drmResults);
	if (FAILED(hr))
	{
#ifdef BURNMGR_SHOW_MSGBOX
		MessageBoxA(NULL, HRErrorCode(hr), "Ohh shit, GetInitResults failed :(", MB_OK);
#endif

	}

#ifdef BURNMGR_SHOW_MSGBOX
	char big[65536] = "";
#endif
	for (size_t i = 0;i != numFiles;i++)
	{
#ifdef BURNMGR_SHOW_MSGBOX
		wsprintfA(big + lstrlenA(big), "%i: %s\n", i, HRErrorCode(drmResults[i]));
#endif
		if (results[i] == BURN_OK)
		{
			switch (drmResults[i])
			{
			case NS_S_DRM_BURNABLE_TRACK:
				results[i] = S_OK;
				break;
			case S_OK:  // do nothing
				break;
			case NS_E_DRM_TRACK_EXCEEDED_PLAYLIST_RESTICTION:
				results[i] = BURN_DRM_BURN_COUNT_EXCEEDED;
				break;
			case NS_E_DRM_NO_RIGHTS:
				results[i] = BURN_DRM_NO_LICENSE;
				break;
			default:
				if (SUCCEEDED(results[i]))
					results[i] = S_OK;
				else
					results[i] = BURN_GENERAL_FAILURE;
				break;
			}
		}

	}
#ifdef BURNMGR_SHOW_MSGBOX
	MessageBoxA(NULL, big, "in_wm burn", MB_OK);
#endif

	callback->OnLicenseCallback(numFiles, results);
	delete[] results;
}

ifc_audiostream* DRMBurnManager::CreateDecoder(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtension(filename);
	if (!lstrcmpiW(ext, L".WMA"))
	{
		internalDecoder = true;
		ExtendedReadStruct *wmaDecoder = new ExtendedReadStruct(reader);
		if (wmaDecoder->Open(filename) && wmaDecoder->FindOutput(16, 2))
		{
				return wmaDecoder;
		}
		else
		{
			delete wmaDecoder;
			return 0;
		}
	}
	else
	{
		internalDecoder = false;
		AudioParameters parameters;
		parameters.bitsPerSample = 16;
		parameters.channels = 2;
		parameters.sampleRate = 44100;

		ifc_audiostream *decoder = decodeFile->OpenAudio(filename, &parameters);
		if (decoder && (parameters.bitsPerSample != 16 || parameters.channels != 2 || parameters.sampleRate != 44100))
		{
			parameters.errorCode = API_DECODEFILE_BAD_RESAMPLE;
			decodeFile->CloseAudio(decoder);
			decoder=0;
		}
		return decoder;
	}
}

void DRMBurnManager::CloseDecoder(ifc_audiostream *decoder)
{
	if (internalDecoder)
	{
		ExtendedReadStruct *wmaDecoder = static_cast<ExtendedReadStruct *>(decoder);
		delete wmaDecoder;
	}
	else
	{
		decodeFile->CloseAudio(decoder);
	}
	internalDecoder = false;
}

void DRMBurnManager::CancelBurn()
{
	if (drmStarted && playlistBurner)
		playlistBurner->EndPlaylistBurn(E_ABORT);
		//playlistBurner->Cancel();
	drmStarted=false;
}

void DRMBurnManager::BurnFinished()
{
	if (drmStarted && playlistBurner)
		playlistBurner->EndPlaylistBurn(S_OK);
	//playlistBurner->Cancel();
	drmStarted=false;
}

