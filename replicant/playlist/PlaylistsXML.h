#pragma once
#include "xml/obj_xml.h"
#include "xml/ifc_xmlcallback.h"
#include "nx/nxuri.h"

class Playlists;

class PlaylistsXML : public ifc_xmlcallback
{
public:
	PlaylistsXML(Playlists *_playlists);
	~PlaylistsXML();
	int LoadFile(nx_uri_t filename);

private:
	/* XML callbacks */
	void WASABICALL XMLCallback_OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes);

	obj_xml *parser;
	Playlists *playlists;
	nx_uri_t root_path;
	struct
	{
		nx_string_t filename, title, songs, seconds, iTunesID, id;
	} attributes;
};

