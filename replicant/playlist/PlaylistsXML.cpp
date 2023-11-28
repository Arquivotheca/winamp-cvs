#include "PlaylistsXML.h"
#include "api.h"
#include "Playlists.h"

PlaylistsXML::PlaylistsXML(Playlists *_playlists) : parser(0), playlists(_playlists)
{
	root_path=0;
	NXStringCreateWithUTF8(&attributes.filename, "filename");
	NXStringCreateWithUTF8(&attributes.title, "title");
	NXStringCreateWithUTF8(&attributes.songs, "songs");
	NXStringCreateWithUTF8(&attributes.seconds, "seconds");
	NXStringCreateWithUTF8(&attributes.iTunesID, "iTunesID");
	
	// TODO: append /playlists
	WASABI2_API_APP->GetDataPath(&root_path);

	if (WASABI2_API_SVC->GetService(&parser) == NErr_Success && parser)
	{
		nx_string_t match;
		NXStringCreateWithUTF8(&match, "playlists\fplaylist");
		parser->RegisterCallback(match, obj_xml::MATCH_EXACT, this);
		NXStringRelease(match);

		parser->Open();
	}
}

PlaylistsXML::~PlaylistsXML()
{
	NXStringRelease(attributes.filename);
	NXStringRelease(attributes.title);
	NXStringRelease(attributes.songs);
	NXStringRelease(attributes.seconds);
	NXStringRelease(attributes.iTunesID);
	NXURIRelease(root_path);
	if (parser)
	{
		parser->UnregisterCallback(this);
		parser->Close();
		parser->Release();
	}
}

#if 0
void PlaylistsXML::XMLCallback_OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes)
{
	const wchar_t *filename = params->getItemValue(L"filename");

	const wchar_t *title = params->getItemValue(L"title");

	const wchar_t *countString = params->getItemValue(L"songs");
	int numItems = 0;
	if (countString && *countString)
		numItems = _wtoi(countString);

	const wchar_t *lengthString = params->getItemValue(L"seconds");
	int length = 0;
	if (lengthString && *lengthString)
		length = _wtoi(lengthString);

	const wchar_t *iTunesIDString = params->getItemValue(L"iTunesID");
	uint64_t iTunesID = 0;
	if (iTunesIDString && *iTunesIDString)
		iTunesID = _wtoi64(iTunesIDString);

	// parse GUID
	GUID guid = INVALID_GUID;
	const wchar_t *guidString = params->getItemValue(L"id");
	if (guidString && *guidString)
	{
		int Data1, Data2, Data3;
		int Data4[8];

		int n = swscanf(guidString, L" { %08x - %04x - %04x - %02x%02x - %02x%02x%02x%02x%02x%02x } ",
		                &Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
		                Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7);

		if (n == 11) // GUID was
		{
			// Cross assign all the values
			guid.Data1 = Data1;
			guid.Data2 = Data2;
			guid.Data3 = Data3;
			guid.Data4[0] = Data4[0];
			guid.Data4[1] = Data4[1];
			guid.Data4[2] = Data4[2];
			guid.Data4[3] = Data4[3];
			guid.Data4[4] = Data4[4];
			guid.Data4[5] = Data4[5];
			guid.Data4[6] = Data4[6];
			guid.Data4[7] = Data4[7];
		}
	}

	if (PathIsFileSpecW(filename))
	{
		wchar_t playlistFilename[MAX_PATH];
		PathCombineW(playlistFilename, rootPath, filename);
		playlists->AddPlaylist_internal(playlistFilename, title, guid, numItems, length, iTunesID);
	}
	else
	{
		playlists->AddPlaylist_internal(filename, title, guid, numItems, length, iTunesID);
	}
}

int PlaylistsXML::LoadFile(const wchar_t *filename)
{
	if (!parser)
		return PLAYLISTSXML_NO_PARSER; // no sense in continuing if there's no parser available

	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return PLAYLISTSXML_NO_FILE;

	int8_t data[1024];

	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			if (parser->xmlreader_feed(data, bytesRead) != API_XML_SUCCESS)
			{
				CloseHandle(file);
				return PLAYLISTSXML_XML_PARSE_ERROR;
			}
		}
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);
	return PLAYLISTSXML_SUCCESS;
}




#endif