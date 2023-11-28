!ifndef ROTATION_CONFIG
!define ROTATION_CONFIG
	
	;!define ROTATION_DEBUG_MODE
	
	!ifdef ROTATION_DEBUG_MODE
		!ifndef LANG_USE_ALL
			!define LANG_USE_ALL
		!endif
	!endif
	
	!define ROTATION_DEFAULTPATH		"$MUSIC\$(^NameDA)"
	!define ROTATION_PLAYLIST			"rotation.m3u8"
		

	!define ROTATION_INSTALLER_NAME		"free_mp3s.exe"
	!define ROTATION_INSTALLER_TITLE	"Winamp MP3 Bundle"
	!define ROTATION_INSTALLER_PLTMP	"$PLUGINSDIR\rotation.tpl"
	!define ROTATION_INSTALLER_DLTMP	"$PLUGINSDIR\rotation.tdl"
	!define ROTATION_INSTALLER_LINK		"http://www.winamp.com/music/"
	
	!define ROTATION_LANGUAGE_REGROOT	HKCU
	!define ROTATION_LANGUAGE_REGKEY	"Software\${ROTATION_INSTALLER_TITLE}"
	!define ROTATION_LANGUAGE_REGVALUE	"LangId"
	
	!define ROTATION_INSTALLPATH_REGROOT	HKCU
	!define ROTATION_INSTALLPATH_REGKEY		"Software\Winamp"
	!define ROTATION_INSTALLPATH_REGVALUE	"MusicBundle"
	
	!define ROTATION_WINAMP_PLAYLIST_GUID 	"{FDB55F58-C8E9-4b69-B564-CB975396E25F}"

	
!endif