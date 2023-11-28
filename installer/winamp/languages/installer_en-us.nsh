; Language-Country:	EN-US
; LangId:			1033
; CodePage:			1252
; Revision:			6
; Last udpdated:		03.07.2008
; Author:			Nullsoft
; Email:			

; Notes:
; use ';' or '#' for comments
; strings must be in double quotes.
; only edit the strings in quotes:
# example: ${LangFileString} installFull "Edit This Value Only!"
# Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

; History
; 04.10 > barabanger:  added 360 after Microsoft Xbox.
; 05.10 > barabanger:  in IDS_SEC_FREEFORM_DESC Winamp Bento changed to  "Bento"
; 05.10 > djegg: fixed typos in header comments, added extra notes
; 06.10 > barabanger: milkdrop2 strings added
; 27.10 > djegg: removed some trailing spaces
; 01.11 > benski: added in_flv
; 02.11 > djegg: added description for in_flv
; 15.11 > barabanger: added old os message - IDS_MSG_WINDOWS_TOO_OLD
; 14.01 > barabanger: changed winamp remote bundle text (IDS_BUNDLE1_DESCRIPTION).
; 20.03 > barabanger: added toolbar search (IDS_BUNDLE21_XXX).
; 21.03 > barabanger: added winamp search (IDS_WINAMP_SEARCH).
; 26.03 > djegg: removed "(enhanced by Google®)" from IDS_BUNDLE21_DESCRIPTION
; 02.05 > koopa: moved text "(default vis plugin) " from IDS_SEC_AVS_DESC to IDS_SEC_MILKDROP2_DESC
; 20.05 > djegg: added secSWF and IDS_SEC_SWF_DEC_DESC (possibly subject to change)
; 13.06 > barabanger: added IDS_SEC_GEN_DROPBOX & IDS_SEC_GEN_DROPBOX_DESC (subject to change)
; 17.06 > djegg: added missing SEC_ML_PLG item for Playlist Generator
; 03.07 > barabanger: changed emusic bundle text
; 24.11 > djegg: added Winamp3 section for upgrade messages
; 01/01 > djegg: added localized "built on" and "at" strings for branding text
; 15/01 > djegg: added ml_impex entry & description
; 01/02 > djegg: added AutoplayHandler
; 09/02 > djegg: added Language Selection dialog section
; 20/02 > djegg: added MB & KB to Bundle page
; 17.06 > barabanger: added orgler section name and description (IDS_SEC_ML_ORGLER)
; aug 27 2009 > benski: added IDS_RUN_OPTIMIZING
; sep 08 2009 > smontgo: changed Welcome screen per US1145 and Powerpoint deck.
; sep 17 2009 > benski: added sections & descriptions for MP4V & MKV
; sep 21 2009 > djegg: changed sec_ml_impex description
; oct 30 2009 > benski: changed in_dshow desc to IDS_SEC_DSHOW_DEC_DESC, removed AVI from desc
; oct 30 2009 > benski: added IDS_SEC_AVI_DEC_DESC for in_avi
; oct 30 2009 > djegg: changed secDSHOW description (removed AVI)
; oct 30 2009 > djegg: added secAVI
; nov 4 2009 > barabanger: added IDS_SEC_ML_ADDONS & IDS_SEC_ML_ADDONS_DESC, lines 232 & 469 (nov 6 2009 > ychen: modified description)
; nov 13 2009 > barabanger: added IDS_SEC_MUSIC_BUNDLE & DESC, lines 233 & 470. Edited "Downloading" string
; nov 22 2009 > barabanger: updated music bundle related text (see prev rec)
; nov 24 2009 > djegg: updated IDS_SEC_ML_ONLINE_DESC
; nov 30 2009 > smontgo: added IDS_DXDIST for download of DirectX9 web installer (for d3dx9 libs for osd)
; dec 01 2009 > smontgo: added IDS_DIRECTX_INSTALL_ERR to report directx download or install error
; dec 04 2009 > barabanger: removed IDS_DXDIST and IDS_DIRECTX_ISNTALL_ERR
; dec 04 2009 > barabanger: added DirectX Section: IDS_DIRECTX_*
; dec 11 2009 > smontgo: edited IDS_DIRECTX_EMBED_CONNECT_FAILED string (Your computer is missing a)
; jan 22 2010 > djegg: added IDS_CLEANUP_PLUGINS & IDS_REMOVE_SKINS to 'installation strings' (lines 259-260)
; mar 15 2010 > barabanger: new uninstaller strings (lines 474-493) // oct 4 2010 > djegg: edited lines 475+477
; may 26 2010 > djegg: added pmp_android (lines 187 & 464)
; sep 29 2010 > benski: added pmp_wifi (lines 188 & 465)
; nov 08 2010 > barabanger: updated IDS_PAGE_WELCOME_TEXT // nov 12 2010 > added extra line inbetween welcome text and bullet points // nov 19 2010 > updated welcome text
; nov 12 2010 > barabanger: Commented-out Winamp Remote from bundle page (lines 323-4)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_DISABLE for disabling incompatible gen_lyrics plugin (line 350)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_WARNING for warning about incompatible gen_lyrics plugin (line 351)
; jun 23 2011 > djegg: changed AAC/aacPlus to HE-AAC (secAACE, line 124)
; jun 27 2011 > barabanger: added Live Media plugin for French installer only (lines 234, 389 & 471)

!insertmacro LANGFILE_EXT "English"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Installer Language"
${LangFileString} LANGUAGE_DLL_INFO "Please select a language."
 
${LangFileString} installFull "Full"
${LangFileString} installStandart "Standard"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "Minimal"
${LangFileString} installPrevious "Previous Installation"

; BrandingText
${LangFileString} BuiltOn "built on"
${LangFileString} at "at"

${LangFileString} installWinampTop "This will install Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}."
${LangFileString} installerContainsBundle "This installer contains the full bundle install."
${LangFileString} installerContainsPro "This installer contains the pro install."
${LangFileString} installerContainsFull "This installer contains the full install."
${LangFileString} installerContainsStandard "This installer contains the standard install."
${LangFileString} installerContainsLite "This installer contains the lite install."
${LangFileString} licenseTop "Please read and agree to the license terms below before installing."
${LangFileString} directoryTop "The installer has determined the optimal location for $(^NameDA). If you would like to change the folder, do so now."
${LangFileString} verHistHeader "Version History"
${LangFileString} verHistHeaderSub "Winamp versions history"
${LangFileString} verHistTop "Please review version changes before continuing:"
${LangFileString} verHistBottom "Updates marked as red need your special attention."
${LangFileString} verHistButton "Finish"

${LangFileString} uninstallPrompt "This will uninstall Winamp. Continue?"

${LangFileString} msgCancelInstall "Cancel install?"
${LangFileString} msgReboot "A reboot is required to complete the installation.$\r$\nReboot now? (If you wish to reboot later, select No)"
${LangFileString} msgCloseWinamp "You must close Winamp before you can continue.$\r$\n$\r$\n	After you have closed Winamp, select Retry.$\r$\n$\r$\n	If you wish to try to install anyway, select Ignore.$\r$\n$\r$\n	If you wish to abort the installation, select Abort."
${LangFileString} msgInstallAborted "Install aborted by user"

${LangFileString} secWinamp "Winamp (required)"
${LangFileString} compAgent "Winamp Agent"
${LangFileString} compModernSkin "Modern Skin Support"
${LangFileString} whatNew "What's New"
${LangFileString} uninstallWinamp "Uninstall Winamp"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Download and Install Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis Playback"
${LangFileString} secOGGEnc "OGG Vorbis Encoding"
${LangFileString} secAACE "HE-AAC encoding"
${LangFileString} secMP3E "MP3 encoding"
${LangFileString} secMP4E "MP4 support"
${LangFileString} secWMAE "WMA encoding"
${LangFileString} msgWMAError "There was a problem installing components. WMA Encoder will not be installed. Please visit http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , download the encoder and try again."
${LangFileString} secCDDA "CD playback and extraction"
${LangFileString} secSonicBurning "Sonic Ripping/Burning support"
${LangFileString} msgCDError "There was a problem installing components. CD Ripping/Burning may not function properly."
${LangFileString} secCDDB "CDDB for recognizing CDs"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"
${LangFileString} secDVD "DVD Playback (3rd party dvd decoder required)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "eMusic Icon and Offer for 50 Free MP3s!"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "AOL Music Now Icon and Offer for 30 day free trial!"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Get Free AV protection with Active Virus Shield!"
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Get Active Security Monitor - it's FREE!"
!endif

${LangFileString} secDSP "Signal Processor Studio Plug-in"
${LangFileString} secWriteWAV "Old-school WAV writer"
${LangFileString} secLineInput "Line Input Support"
${LangFileString} secDirectSound "DirectSound output support"

${LangFileString} secHotKey "Global Hotkey Support"
${LangFileString} secJmp "Extended Jump To File Support"
${LangFileString} secTray "Nullsoft Tray Control"

${LangFileString} msgRemoveMJUICE "Remove MJuice support from your system?$\r$\n$\r$\nSelect YES unless you use MJF files in programs other than Winamp."
${LangFileString} msgNotAllFiles "Not all files were removed.$\r$\nIf you wish to remove the files yourself, please do so."


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "DirectShow Formats (MPG, M2V)"
${LangFileString} secAVI "AVI Video"
${LangFileString} secFLV "Flash Video (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF "Flash Media Protocol (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp Media Library"
${LangFileString} secOM "Online Media"
${LangFileString} secOrb "Remote Media"
${LangFileString} secWire "Podcast Directory"
${LangFileString} secMagic "MusicIP Mix"
${LangFileString} secPmp "Portable Media Players"
${LangFileString} secPmpIpod "iPod® support"
${LangFileString} secPmpCreative "Support for Creative® players"
${LangFileString} secPmpP4S "Support for Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "USB Devices Support"
${LangFileString} secPmpActiveSync "Support for Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Android device support"
${LangFileString} secPmpWifi "Android Wifi support"

${LangFileString} sec_ML_LOCAL "Local Media"
${LangFileString} sec_ML_PLAYLISTS "Playlists"
${LangFileString} sec_ML_DISC "CD Rip & Burn"
${LangFileString} sec_ML_BOOKMARKS "Bookmarks"
${LangFileString} sec_ML_HISTORY "History"
${LangFileString} sec_ML_NOWPLAYING "Now Playing"
${LangFileString} sec_ML_RG "Replay Gain Analysis Tool"
${LangFileString} sec_ML_DASH "Winamp Dashboard"
${LangFileString} sec_ML_TRANSCODE "Transcoding Tool"
${LangFileString} sec_ML_PLG "Playlist Generator"
${LangFileString} sec_ML_IMPEX "Database Import/Export Tool"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) Installer"
${LangFileString} IDS_SELECT_LANGUAGE  "Please select the language of the installer"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Multimedia Engine"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Output"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Audio Playback"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Audio Encoders"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Video Playback"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualization"
${LangFileString} IDS_GRP_UIEXTENSION		"User Interface Extensions"
${LangFileString} IDS_GRP_WALIB				"Winamp Library"
${LangFileString} IDS_GRP_WALIB_CORE		"Core Media Library Components"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Portable Media Player Support"
${LangFileString} IDS_GRP_LANGUAGES 	    "Languages"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME Output"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC encoding"
${LangFileString} IDS_SEC_MILKDROP2 	"Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Auto-Tagger"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (alpha)"
;${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Winamp Add-ons"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Download MP3 Bundle"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "French Radio Plugin"
${LangFileString} IDS_SEC_CLOUD       "Winamp Cloud"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configuring online services..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Checking if another instance of Winamp is running..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Opening internet connection..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Checking if internet available..."
${LangFileString} IDS_RUN_NOINET				"No internet connection"
${LangFileString} IDS_RUN_EXTRACT				"Extracting"
${LangFileString} IDS_RUN_DOWNLOAD				"Downloading"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Download completed."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Download failed. Reason:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Download canceled."
${LangFileString} IDS_RUN_INSTALL				"Installing"
${LangFileString} IDS_RUN_INSTALLFIALED			"Installation failed."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"File not found. Scheduling download."
${LangFileString} IDS_RUN_DONE					"Done."
${LangFileString} IDS_RUN_OPTIMIZING			"Optimizing..."

${LangFileString} IDS_DSP_PRESETS 	"SPS presets"
${LangFileString} IDS_DEFAULT_SKIN	"default skins"
${LangFileString} IDS_AVS_PRESETS	"AVS presets"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop presets"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 presets"

${LangFileString} IDS_CLEANUP_PLUGINS	"Cleanup plugins..."
${LangFileString} IDS_REMOVE_SKINS		"Remove default skins..."


; download
${LangFileString} IDS_DOWNLOADING	"Downloading"
${LangFileString} IDS_CONNECTING	"Connecting ..."
${LangFileString} IDS_SECOND		" (1 second remaining)"
${LangFileString} IDS_MINUTE		" (1 minute remaining)"
${LangFileString} IDS_HOUR			" (1 hour remaining)"
${LangFileString} IDS_SECONDS		" (%u seconds remaining)"
${LangFileString} IDS_MINUTES		" (%u minutes remaining)"
${LangFileString} IDS_HOURS			" (%u hours remaining)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) of %skB @ %u.%01ukB/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Play"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installation Complete"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) has been installed on your computer.$\r$\n$\r$\n\
													Click Finish to close this wizard."
${LangFileString} IDS_PAGE_FINISH_RUN		"Launch $(^NameDA) after the installer closes"
${LangFileString} IDS_PAGE_FINISH_LINK		"Click here to visit Winamp.com"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Add MP3 bundle to Winamp playlist"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Welcome to the $(^NameDA) installer"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) allows you to manage your media library and listen to internet radio.  \
											Now with Winamp Cloud you can listen at home, at work, in the car or anywhere you go.$\r$\n$\r$\n\
											Features include:$\r$\n$\r$\n  \
												•  Winamp Cloud conveniently unifies your music library$\r$\n$\r$\n  \
												•  Manage your entire music library across all your devices$\r$\n$\r$\n  \
												•  Wirelessly sync media to the Winamp for Android app$\r$\n$\r$\n  \
												•  Build playlists using the Automatic Playlist generator$\r$\n$\r$\n  \
												•  Listen and subscribe to over 30,000 podcasts"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTE: To enjoy the new features and \
															design of the Bento skin (recommended), all \
															components must be checked."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Choose Start Options"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Select from the following start options."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Choose from the following options to configure your Winamp start options."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Create Start menu entry"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Create Quick Launch icon"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Create Desktop icon"

; bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"Get the Most Out of $(^NameDA)"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"Choose the additional features below to get the most out of $(^NameDA)."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Get the best $(^NameDA) experience with these additional features."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"download size:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"already installed"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"only Windows XP or newer"
${LangFileString} IDS_PAGE_BUNDLE_MB		"MB"
${LangFileString} IDS_PAGE_BUNDLE_KB		"KB"

; bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote" ; keep for uninstaller
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Enjoy your music and videos on the go. Stream your home music from Winamp.com, \
;                                            compatible mobile devices, Nintendo® Wii™, Sony® PlayStation® 3 and Microsoft® Xbox™ 360."
${LangFileString} IDS_BUNDLE2_NAME			"Winamp® Toolbar"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Control Winamp directly from your web browser and get instant access \
													to SHOUTcast."
${LangFileString} IDS_BUNDLE21_NAME			"Set AOL® Search as my default search engine"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Let Winamp help you search the web with Winamp Search."
${LangFileString} IDS_WINAMP_SEARCH			"Winamp Search"

${LangFileString} IDS_BUNDLE3_NAME			"50 free MP3 Downloads +1 free Audiobook from eMusic"
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Start an eMusic 2-week free trial and enjoy 50 MP3 music downloads for free plus 1 free \
                                             audiobook. The music downloads and audiobook are yours to keep, even if you cancel."

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Unable to close Winamp Agent.$\r$\n\
                                                   Make sure that another user is not logged into Windows.\
                                                   $\r$\n$\r$\n	After you have closed WinampAgent, select Retry.\
                                                   $\r$\n$\r$\n	If you wish to try to install anyway, select Ignore.\
                                                   $\r$\n$\r$\n	If you wish to abort the installation, select Abort."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"This version of Windows is no longer supported.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} requires a minimum of Windows XP or newer."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Incompatible 3rd-party gen_msn7.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp 5.57 and later to crash on load.$\r$\nPlugin will now be disabled. Click OK to proceed."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Incompatible 3rd-party gen_lyrics.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp 5.59 and later to crash on load.$\r$\nPlugin will now be disabled. Click OK to proceed."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "3rd-party gen_lyrics plugin detected!$\r$\n$\r$\nOld versions of this plugin are incompatible with Winamp 5.6 and newer. Make sure you've got the latest version from http://lyricsplugin.com before proceeding."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"Incompatible 3rd-party gen_lyrics_ie.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp to crash.$\r$\nPlugin will now be disabled. Click OK to proceed."

;Winamp3
${LangFileString} msgWA3			"Winamp3 detected..."
${LangFileString} msgWA3_UPGRADE		"Winamp3 detected. Upgrade Winamp3 install to Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} and migrate skins to Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}?$\r$\n(Selecting Yes will upgrade and is recommended, selecting No will let you keep separate Winamp3 and Winamp 5.x installs)"
${LangFileString} msgWA3_MIGRATE		"Migrating Skins..."
${LangFileString} msgWA3_REMOVE			"Removing Winamp3 install..."
${LangFileString} msgWA3_REMOVE2		"Files in '$0' were not all removed.$\r$\nIf you wish to remove the files from this directory, you can manually delete them."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Detected ${DIRECTXINSTAL_WINVER_LO} or lower"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Detected ${DIRECTXINSTAL_WINVER_HI} or higher"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Checking ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Required minimal ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Unable to detect ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Detected ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Unsupported ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Checking if $0 present"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Download required"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Checking internet connection"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Latest version of ${DIRECTXINSTAL_DIRECTXNAME} available at:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Downloading ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FOUND						"Found"
${LangFileString} IDS_DIRECTX_MISSING					"Missing"
${LangFileString} IDS_DIRECTX_SUCCESS					"Success"
${LangFileString} IDS_DIRECTX_ABORTED					"Aborted"
${LangFileString} IDS_DIRECTX_FAILED					"Failed"
${LangFileString} IDS_DIRECTX_DONE						"Done"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Running ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly.$\r$\nInstall it right now?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Unable to download ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Unable to install ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Your computer is missing a ${DIRECTXINSTAL_DIRECTXNAME} component required by ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Unable to download missing ${DIRECTXINSTAL_DIRECTXNAME} component"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Unable to install missing ${DIRECTXINSTAL_DIRECTXNAME} component"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Installing $(IDS_SEC_GEN_FRENCHRADIO)..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp core (required)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp Agent, for quick system tray access and maintaining filetype associations"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Multimedia Engine (Input/Output system)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB support, for automatically fetching the CD titles from the online Gracenote database"
${LangFileString} IDS_SEC_SONIC_DESC			"Sonic Library, required for Ripping && Burning Audio CDs"
${LangFileString} IDS_SEC_DSP_DESC				"DSP plugin, for applying extra effects such as chorus, flanger, tempo and pitch control"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Audio Playback Support (Input Plugins: Audio Decoders)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Playback support for MP3, MP2, MP1, AAC formats (required)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Playback support for WMA format (including DRM support)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Playback support for MIDI formats (MID, RMI, KAR, MUS, CMF and more)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Playback support for Module formats (MOD, XM, IT, S3M, ULT and more)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Playback support for Ogg Vorbis format (OGG, OGA)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Playback support for MPEG-4 Audio formats (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Playback support for FLAC format"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Playback support for Audio CDs"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Playback support for Waveform formats (WAV, VOC, AU, AIFF and more)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Video Playback Support (Input Plugins: Video Decoders)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Playback support for Windows Media video formats (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Playback support for Nullsoft Video format (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Playback support for MPEG-1/2 and other video formats"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Playback support for AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Playback support for Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Playback support for Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Playback support for MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Playback support for Adobe Flash streaming format (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Encoding and Transcoding Support (required for CD Ripping and converting from one file format to another)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Support for ripping and transcoding to WMA format"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Support for ripping and transcoding to WAV format"
${LangFileString} IDS_SEC_MP3_ENC_DESC			"Support for ripping and transcoding to MP3 format"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Support for ripping and transcoding to M4A and AAC formats"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Support for ripping and transcoding to FLAC format"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Support for ripping and transcoding to Ogg Vorbis format"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Output Plugins (which control how audio is processed and sent to your soundcard)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Old-school WAV/MME Writer (deprecated, but some users still prefer to use it instead of the Encoder plugins)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound Output (required / default Output plugin)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Old-school WaveOut Output (optional, and no longer recommended or required)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"User Interface Extensions"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Global Hotkeys plugin, for controlling Winamp with the keyboard when other applications are in focus"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Extended Jump to File support, for queuing up songs in the playlist, and much much more"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft Tray Control plugin, for adding Play control icons in the system tray"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Modern Skin Support, required for using freeform skins such as Winamp Modern and Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Visualization Plugins"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Fullscreen visualization plugin"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio plugin"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop visualization plugin"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 visualization plugin (default vis plugin)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Line Input Support using linein:// command (applies visualizer to mic/line input)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp Library"
${LangFileString} IDS_SEC_ML_DESC				"Winamp Media Library (required)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Transcoding Tool, used for converting from one file format to another"
${LangFileString} IDS_SEC_ML_RG_DESC			"Replay Gain Analysis Tool, used for volume-levelling"
${LangFileString} IDS_SEC_ML_DASH_DESC			"Dashboard, an optional home page portal for the Media Library"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-Tagger (Powered by Gracenote), for filling in missing metadata"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Podcast Directory, for subscribing to and downloading podcasts"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Online Services, including SHOUTcast Radio && TV, AOL Radio feat. CBS Radio, Winamp Charts, and more"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp Playlist Generator (powered by Gracenote), for creating acoustically dynamic playlists"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Core Media Library Components"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Local Media database, with powerful query system and custom smart views"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Playlists Manager, for creating, editing and storing all your playlists"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD Rip && Burn, the media library interface for ripping && burning Audio CDs"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Bookmarks Manager, for bookmarking your favorite streams, files or folders"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"History, for instant access to all recently played local or remote files and streams"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Now Playing, for viewing information about the currently playing track"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Portable Media Player Support"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Core Portable Media Player Support plugin (required)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod® support"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Support for Creative® portables (for managing Nomad™, Zen™ and MuVo™ players)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Support for Microsoft® PlaysForSure® (for managing all MTP && P4S compatible players)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB Devices support (for managing generic usb thumbdrives and players)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Support for Microsoft® ActiveSync® (for managing Windows Mobile®, Smartphone && Pocket PC devices)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Support for Android devices"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Android Wifi support"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Alpha version of soon coming dropbox plugin. Use ctrl-shift-d to activate"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"iTunes-compatible Media Library database import/export plugin"
;${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Winamp Orgler™ lets you track, chart && share your listening history"
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Discover and add extensions to your Winamp media player with Add-ons"
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Downloads and installs free music from Winamp Music"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Listen to more than 300 French radio stations, live in $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"
${LangFileString} IDS_SEC_CLOUD_DESC			"That Cloud thing everyone is talking about"


${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Choose related programs to uninstall."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Remove $(^NameDA) from your computer."

;${LangFileString} IDS_UNINSTALL_COMPONENTS_HEADER		"The following $(^NameDA) related programs will be removed. To change, please uncheck the corresponding box:"
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Uninstall path:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Media Player"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Uninstall all $(^NameDA) Media Player components including bundled third party plug-ins."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"User Preferences"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Remove all $(^NameDA) preferences and plug-ins."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"The following Winamp related programs will be removed in this uninstall:"
; ${LangFileString} IDS_UNINSTALL_BUNDLES_FOOTER		"Note: You can uninstall these programs at any time via Windows Control Panel."
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Control $(^NameDA) directly from your web browser and get instant access to SHOUTcast."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Enjoy your music and videos on the go."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Winamp Detector Plug-ins for Mozilla Firefox and Internet Explorer."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"Music Downloads, MP3 Downloads, MP3 songs from eMusic.com"
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"Uninstalling $2..."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Help $(^NameDA) by submitting feedback"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Open $(^NameDA) folder"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNote:  Not all files were removed from this uninstall. To view, open the Winamp folder."
