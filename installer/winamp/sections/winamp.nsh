${WinampSection} "winampApplication" $(secWinamp) IDX_SEC_WINAMP           ; <<< [Winamp]
	SectionIn 1 2 3 4 5 6 7 8 RO
  
	DetailPrint "$(IDS_CLEANUP_PLUGINS)"
	SetDetailsPrint none
	
	; cleanup old shit
	UnRegDLL "$INSTDIR\Plugins\in_asfs.dll"
	Delete /REBOOTOK "$INSTDIR\Plugins\in_asfs.dll"

	;Temp lines for 5.58 beta (will remove for final)
	Delete "$INSTDIR\System\vp8x.w5s"
	Delete "$INSTDIR\System\vp8x.wbm"
	;End Temp lines

	Delete "$INSTDIR\System\watcher.w5s" ; watcher is unused now that Music Now is gone  
	Delete "$INSTDIR\System\db.w5s"
	Delete "$INSTDIR\Plugins\gen_ff.dll"
	Delete "$INSTDIR\Plugins\Freeform\wacs\freetype\freetype.wac"
	Delete "$INSTDIR\System\timer.w5s"
	Delete "$INSTDIR\System\filereader.w5s"
	Delete "$INSTDIR\System\aacPlusDecoder.w5s"
	Delete "$INSTDIR\System\aacPlusDecoder.wbm"
	Delete "$INSTDIR\Plugins\gen_ml.dll" ; make sure gen_ff and gen_ml are not kept if unchecked

	; delete ML plugins so that unselecting them does not keep the old versions around
	Delete "$INSTDIR\Plugins\ml_local.dll"
	Delete "$INSTDIR\Plugins\ml_playlists.dll"
	Delete "$INSTDIR\Plugins\ml_disc.dll"
	Delete "$INSTDIR\Plugins\ml_bookmarks.dll"
	Delete "$INSTDIR\Plugins\ml_history.dll"
	Delete "$INSTDIR\Plugins\ml_impex.dll"
	Delete "$INSTDIR\Plugins\ml_nowplaying.dll"
	Delete "$INSTDIR\Plugins\ml_rg.dll"
	Delete "$INSTDIR\Plugins\ml_plg.dll"
	Delete "$INSTDIR\Plugins\ml_online.dll"
	Delete "$INSTDIR\Plugins\ml_dash.dll"
	Delete "$INSTDIR\Plugins\ml_wire.dll"
	Delete "$INSTDIR\Plugins\ml_transcode.dll"
	Delete "$INSTDIR\Plugins\ml_autotag.dll"
	Delete "$INSTDIR\Plugins\ml_addons.dll"
	Delete "$INSTDIR\Plugins\ml_downloads.dll"
	Delete "$INSTDIR\Plugins\ml_cloud.dll"
	Delete "$INSTDIR\Plugins\gen_orgler.dll"
	Delete "$INSTDIR\System\auth.w5s"
	Delete "$INSTDIR\System\wasabi2.w5s"
	Delete "$INSTDIR\Components\cloud.w6c"
	Delete "$INSTDIR\Components\ssdp.w6c"
	
	!ifndef BETA
		Delete "$INSTDIR\Plugins\ml_xpdxs.dll"
	!endif ; BETA

	Delete "$INSTDIR\Plugins\ml_pmp.dll"
	Delete "$INSTDIR\Plugins\ml_devices.dll"
	Delete "$INSTDIR\Plugins\pmp_p4s.dll"
	Delete "$INSTDIR\Plugins\pmp_ipod.dll"
	Delete "$INSTDIR\Plugins\pmp_wifi.dll"
	Delete "$INSTDIR\Plugins\pmp_njb.dll"
	Delete "$INSTDIR\Plugins\pmp_usb.dll"
	Delete "$INSTDIR\Plugins\pmp_usb2.dll"
	Delete "$INSTDIR\Plugins\pmp_android.dll"
	Delete "$INSTDIR\Plugins\pmp_activesync.dll"
	Delete "$INSTDIR\Plugins\pmp_cloud.dll"
	Delete "$INSTDIR\System\devices.w5s"

	Delete "$INSTDIR\Plugins\gen_b4s2m3u.dll"
	; Delete "$INSTDIR\pxsdkpls.dll" ; Egg: will re-enable this if we ever get another update from Sonic
	Delete "$INSTDIR\primosdk.dll"
    Delete "$INSTDIR\burnlib.dll"
	Delete "$INSTDIR\system\primo.w5s"
	Delete "$INSTDIR\pconfig.dcf"
	Delete "$INSTDIR\demoedit.aac"
	Delete "$INSTDIR\jnetlib.dll"
	Delete "$INSTDIR\nxlite.dll"
	Delete "$INSTDIR\Winamp.exe.manifest"
	Delete "$SETTINGSDIR\winamp.pic" ; deprecated plug-in cache file

	; delete any other plugins so that unselecting them does not keep the old versions around
	Delete "$INSTDIR\Plugins\gen_tray.dll"
	Delete "$INSTDIR\Plugins\gen_hotkeys.dll"
	Delete "$INSTDIR\Plugins\in_wm.dll"
	Delete "$INSTDIR\Plugins\in_mp4.dll"
	Delete "$INSTDIR\Plugins\in_cdda.dll"
	Delete "$INSTDIR\Plugins\in_midi.dll"
	Delete "$INSTDIR\Plugins\in_mod.dll"
	Delete "$INSTDIR\Plugins\in_vorbis.dll"
	Delete "$INSTDIR\Plugins\in_flac.dll"
	Delete "$INSTDIR\Plugins\in_wave.dll"
	Delete "$INSTDIR\Plugins\in_nsv.dll"
	Delete "$INSTDIR\Plugins\in_flv.dll"
	Delete "$INSTDIR\Plugins\in_swf.dll"
	Delete "$INSTDIR\Plugins\in_dshow.dll"
	Delete "$INSTDIR\Plugins\in_avi.dll"
	Delete "$INSTDIR\Plugins\in_mkv.dll"
	Delete "$INSTDIR\Plugins\in_linein.dll"
	Delete "$INSTDIR\Plugins\enc_lame.dll"
	Delete "$INSTDIR\Plugins\enc_wma.dll"
	Delete "$INSTDIR\Plugins\enc_wav.dll"
	Delete "$INSTDIR\Plugins\enc_flac.dll"
	Delete "$INSTDIR\Plugins\enc_aacplus.dll"
	Delete "$INSTDIR\Plugins\out_wave.dll"
	Delete "$INSTDIR\Plugins\out_disk.dll"
	Delete "$INSTDIR\Plugins\vis_nsfs.dll"
	Delete "$INSTDIR\Plugins\vis_avs.dll"
	Delete "$INSTDIR\Plugins\vis_milk2.dll"
	Delete "$INSTDIR\Plugins\dsp_sps.dll"
  ;	Delete "$INSTDIR\System\ombrowser.w5s"  ; potential 3rd party dependency
  ; Delete "$INSTDIR\Plugins\gen_jumpex.dll" ; don't delete JTFE incase user has a newer internal dev version
  
  ; Delete DrO's gen_ipc_stopplaying_blocker.dll which was a workaround for 5.57x only
    Delete "$INSTDIR\Plugins\gen_ipc_stopplaying_blocker.dll"

  ; Delete DrO's gen_find_on_disk.dll which is now fully integrated into Winamp in 5.7
  ; The plug-ins core had been present as api_explorerfindfile.h for a while, just not
  ; done fully as a native menu item and global hotkey (does preseve any prior hotkey)
    Delete "$INSTDIR\Plugins\gen_find_on_disk.dll"

  ; Delete DrO's gen_nunzio.dll (remember to remove this line later if/when plugin gets updated to be 5.7+ compatible)
    Delete "$INSTDIR\Plugins\gen_nunzio.dll"

  ; Delete DrO's gen_cd_menu.dll which is now fully integrated into Winamp in 5.7
  ; The functionality was in winamp agent (showing cd drive volume name in menus)
    Delete "$INSTDIR\Plugins\gen_cd_menu.dll"

  ; Delete DrO's gen_wolfgang.dll which is now fully integrated into gen_hotkeys in 5.7
    Delete "$INSTDIR\Plugins\gen_wolfgang.dll"

  ; Delete DrO's gen_os_diag.dll which is now partially integrated (saving main dialog positions) in 5.7
  ; and is causing issues with the Alt+3 dialog and the combobox so better safe than sorry, it now dies.
    Delete "$INSTDIR\Plugins\gen_os_diag.dll"

  ; Disable buggy 3rd-party gen_msn7 plugin (causes v5.57+ to crash on load)
    ${If} ${FileExists} "$INSTDIR\Plugins\gen_msn7.dll"
	MessageBox MB_OK "$(IDS_MSN7_PLUGIN_DISABLE)"
    Rename "$INSTDIR\Plugins\gen_msn7.dll" "$INSTDIR\Plugins\gen_msn7.dll.off"
	${EndIf}

  ; Warn about 3rd-party gen_lyrics plugin (old version causes v5.59+ to crash on load)
    ${If} ${FileExists} "$INSTDIR\Plugins\gen_lyrics.dll"
    MessageBox MB_OK "$(IDS_LYRICS_PLUGIN_WARNING)"
  ; Rename "$INSTDIR\Plugins\gen_lyrics.dll" "$INSTDIR\Plugins\gen_lyrics.dll.off"
    ${EndIf}

  ; Disable 3rd-party gen_lyrics_ie plugin (causes Winamp to crash)
    ${If} ${FileExists} "$INSTDIR\Plugins\gen_lyrics_ie.dll"
    MessageBox MB_OK "$(IDS_LYRICS_IE_PLUGIN_DISABLE)"
    Rename "$INSTDIR\Plugins\gen_lyrics_ie.dll" "$INSTDIR\Plugins\gen_lyrics_ie.off"
    ${EndIf}

	DeleteINIStr "$WINAMPINI" "Winamp" "mbdefloc"
	SetDetailsPrint lastused
  
	!ifdef PRO
		WriteINIStr "$WINAMPINI" "Winamp" "PromptForRegKey" "1"
	!endif ; PRO
	SetOutPath "$INSTDIR"

	SetOverwrite off
	File "/oname=$WINAMPM3U" "..\..\resources\media\winamp.m3u"
	SetOverwrite ${OVERWRITEMODE}

	File "/oname=${WINAMPEXE}" "${FILES_PATH}\winamp.exe"

 !ifndef WINAMP64
	SetOutPath "$INSTDIR\Microsoft.VC90.CRT"
	File ..\..\resources\libraries\msvcr90.dll
	File ..\..\resources\libraries\Microsoft.VC90.CRT.manifest
	SetOutPath "$INSTDIR"
 !endif
 
	${If} $WinVer == "Vista"
	${OrIf} $WinVer == "7"
	${OrIf} $WinVer == "8"
		SetOutPath "$INSTDIR"
		${If} ${FileExists} "$INSTDIR\Elevator.exe"
		KillProcDLL::KillProc "Elevator.exe"
		Sleep 1000
		${EndIf}
		File ..\..\output\winamp\Elevator.exe
		ExecWait '"$INSTDIR\Elevator.exe" /RegServer'
		File ..\..\output\winamp\elevatorps.dll
		RegDll "$INSTDIR\elevatorps.dll"
	${EndIf}
  
	File ..\..\output\winamp\nsutil.dll

	SetOutPath "$M3UBASEDIR"
	;File "..\..\resources\media\demoedit.aac"
	File "..\..\resources\media\demo.mp3"
	SetOutPath "$INSTDIR"

	File "/oname=whatsnew.txt" "..\..\resources\data\whatsnew.txt"
	; File ..\..\resources\data\winampmb.htm - deprecated

	WriteINIStr "$WINAMPINI" "CDDA/Line Input Driver" "rip_veritas" "0"
	WriteINIStr "$WINAMPINI" "CDDA/Line Input Driver" "use_veritas" "0"


	Call GetSkinDir
	Pop $R0
	SetOutPath $R0

	DetailPrint "$(IDS_REMOVE_SKINS)"
	SetDetailsPrint none
	Delete "$R0\${MODERNSKINNAME}.*"
	RMDir /r "$R0\${MODERNSKINNAME}"

	RMDir /r "$R0\Winamp Bento"
	Delete "$R0\Winamp Bento.*"

	RMDir /r "$R0\Bento"
	RMDir /r "$R0\Big Bento"
	SetDetailsPrint lastused

	SetOutPath $INSTDIR
	File  "${FILES_PATH}\tataki.dll"
	File  "${FILES_PATH}\zlib.dll"

	SetOutPath "$INSTDIR\System"
	File "${FILES_PATH}\System\jnetlib.w5s"
	File "${FILES_PATH}\System\aacdec.w5s"
	; File "${FILES_PATH}\System\aacPlusDecoder.w5s"
	File "${FILES_PATH}\system\dlmgr.w5s"
	
   !ifndef WINAMP64
	 File /nonfatal "${FILES_PATH}\System\aacdec.wbm"
   !endif
	File "${FILES_PATH}\system\tagz.w5s"
	File "${FILES_PATH}\system\albumart.w5s"
	File "${FILES_PATH}\system\playlist.w5s"
	File "${FILES_PATH}\system\xml.w5s"
	File "${FILES_PATH}\system\jpeg.w5s"
	File "${FILES_PATH}\system\png.w5s"
	File "${FILES_PATH}\system\bmp.w5s"
	File "${FILES_PATH}\system\gif.w5s"

	Push $0
	StrCpy $0 "$INSTDIR\${WINAMPEXE}"
	WriteRegStr HKCR "UVOX" "" "URL: Ultravox Protocol"
	WriteRegStr HKCR "UVOX" "URL Protocol" ""
	WriteRegStr HKCR "UVOX\shell\open\command" "" "$0 %1"

	WriteRegStr HKCR "SC" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "SC" "URL Protocol" ""
	WriteRegStr HKCR "SC\shell\open\command" "" "$0 %1"
	WriteRegStr HKCR "ICY" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "ICY" "URL Protocol" ""
	WriteRegStr HKCR "ICY\shell\open\command" "" "$0 %1"
	WriteRegStr HKCR "SHOUT" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "SHOUT" "URL Protocol" ""
	WriteRegStr HKCR "SHOUT\shell\open\command" "" "$0 %1"
  	Pop $0
  ;SetDetailsPrint lastused
${WinampSectionEnd} ;                                                                   <<< [Winamp]