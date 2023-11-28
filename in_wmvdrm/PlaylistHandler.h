#ifndef NULLSOFT_PLAYLISTS_HANDLER_H
#define NULLSOFT_PLAYLISTS_HANDLER_H

#include "../playlist/svc_playlisthandler.h"
#include <bfc/platform/types.h>

/* macro auto-produces this class defitinion
class YourHandler : public api_playlisthandler
{
public:
	const wchar_t *enumExtensions(size_t n); // returns 0 when it's done
	int SupportedFilename(const wchar_t *filename); // returns SUCCESS and FAILED, so be careful ...
	api_playlistloader *CreateLoader(const wchar_t *filename);
	void ReleaseLoader(api_playlistloader *loader);
	const wchar_t *GetName();  // returns a name suitable for display to user of this playlist form (e.g. PLS Playlist)
protected:
	RECVS_DISPATCH;
};
*/

#define DECLARE_HANDLER(className) class className ## Handler : public svc_playlisthandler {\
public:\
	const wchar_t *enumExtensions(size_t n); \
	int SupportedFilename(const wchar_t *filename); \
	ifc_playlistloader *CreateLoader(const wchar_t *filename);\
	void ReleaseLoader(ifc_playlistloader *loader);\
	const wchar_t *GetName(); \
protected:	RECVS_DISPATCH;}

DECLARE_HANDLER(WPL);
DECLARE_HANDLER(ASX);

// {DC13A85D-7C61-4462-8CB4-9EBBBD86FA7C}
static const GUID wplHandlerGUID = 
{ 0xdc13a85d, 0x7c61, 0x4462, { 0x8c, 0xb4, 0x9e, 0xbb, 0xbd, 0x86, 0xfa, 0x7c } };
// {8909A743-6F00-43ef-B3F6-365000347DE3}

static const GUID asxHandlerGUID = 
{ 0x8909a743, 0x6f00, 0x43ef, { 0xb3, 0xf6, 0x36, 0x50, 0x0, 0x34, 0x7d, 0xe3 } };
// {6F62CBB8-7E1F-43eb-B3F6-01C2601029A3}

#endif