#ifndef NULLSOFT_MEDIALIBRARYPLUGINH
#define NULLSOFT_MEDIALIBRARYPLUGINH

#include <windows.h>
#include "ml.h"
#include "../nu/MakeThunk.h"

class MediaLibraryPlugin
{
public:
	virtual int _cdecl Init() = 0;
	virtual void _cdecl Quit() = 0;
	virtual int _cdecl MessageProc(int message_type, int param1, int param2, int param3) = 0;
	MediaLibraryPlugin(char *description)
	{
		plugin = new winampMediaLibraryPlugin;
		memset(plugin, 0, sizeof(winampMediaLibraryPlugin));
		plugin->version = MLHDR_VER;
		plugin->description = _strdup(description);
		thunk(this, plugin->init, &MediaLibraryPlugin::Init);
		thunk(this, plugin->quit, &MediaLibraryPlugin::Quit);
		thunk(this, plugin->MessageProc, &MediaLibraryPlugin::MessageProc);
	}


	winampMediaLibraryPlugin* operator &()	{ return plugin; }
protected:
	winampMediaLibraryPlugin *plugin;
private:
	ThunkHolder thunk;

};


#endif
