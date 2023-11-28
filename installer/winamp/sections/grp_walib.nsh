!ifdef std | full
 SectionGroup $(IDS_GRP_WALIB) IDX_GRP_WALIB                            ;  Winamp Library

   ${WinampSection} "mediaLibrary" $(secML) IDX_SEC_ML              ; >>> [Media Library]
     ${SECTIONIN_STD}
     SectionGetFlags ${IDX_GRP_WALIB} $1
     IntOp $1 $1 & 0x0041
     StrCmp $1 "0" done
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\gen_ml.dll
     done:
	  SetOutPath "$INSTDIR"
      File "${FILES_PATH}\nxlite.dll"
   ${WinampSectionEnd}                                                           ; <<< [Media Library]

   !include ".\sections\grp_walib_core.nsh"

    ${WinampSection} "mediaLibraryTranscode" $(sec_ML_TRANSCODE) IDX_SEC_ML_TRANSCODE  ; >>> [Trancsoding Tool]
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\ml_transcode.dll
	  ${If} ${FileExists} "$SETTINGSDIR\Plugins\ml_transcode.ini"
	  Rename "$SETTINGSDIR\Plugins\ml_transcode.ini" "$SETTINGSDIR\Plugins\ml\ml_transcode.ini"
	  ${EndIf}
    ${WinampSectionEnd}                                                          ; <<< [Trancsoding Tool]

    ${WinampSection} "mediaLibraryReplayGain" $(sec_ML_RG) IDX_SEC_ML_RG         ; >>> [Replay Gain Analysis Tool]
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\ml_rg.dll
      File ${FILES_PATH}\plugins\ReplayGainAnalysis.dll
    ${WinampSectionEnd}                                                          ; <<< [Replay Gain Analysis Tool]

!ifndef WINAMP64
   !ifdef full
    ${WinampSection} "mediaLibraryiTunesImp" $(sec_ML_IMPEX) IDX_SEC_ML_IMPEX                           ; >>> [iTunes Importer]
      ${SECTIONIN_FULL}
       SetOutPath $INSTDIR\System
       File ${FILES_PATH}\system\xml.w5s
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\ml_impex.dll
    ${WinampSectionEnd}                                                          ; <<< [iTunes Importer]
   !endif
!endif ; WINAMP64

   ${WinampSection} "mediaLibraryAutoTag" $(IDS_SEC_ML_AUTOTAG) IDX_SEC_ML_AUTOTAG                   ; >>> [Auto Tag]
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_autotag.dll
   ${WinampSectionEnd}                                                           ; <<< [Auto Tag]

    !ifndef NOKIA
    !ifdef full
     ${WinampSection} "mediaLibraryPodcast" $(secWire) IDX_SEC_ML_WIRE                                  ; >>> [SHOUTCast Wire]
       ${SECTIONIN_FULL}
       SetOutPath $INSTDIR
       File ${FILES_PATH}\nde.dll
       File ${FILES_PATH}\nxlite.dll
       File ${FILES_PATH}\jnetlib.dll
       SetOutPath $INSTDIR\System
       File ${FILES_PATH}\system\xml.w5s
       SetOutPath $INSTDIR\Plugins
       File ${FILES_PATH}\plugins\ml_wire.dll
	   ;!warning "ml_downloads disabled to make 5.5.2 (1 of 2)"
       File ${FILES_PATH}\plugins\ml_downloads.dll
       WriteRegStr HKEY_CLASSES_ROOT "pcast" "" "URL: Podcast Protocol"
	   WriteRegStr HKEY_CLASSES_ROOT "pcast" "URL Protocol" ""
	   WriteRegStr HKEY_CLASSES_ROOT "pcast\shell\open\command" "" "$INSTDIR\${WINAMPEXE} /HANDLE %1"
	   WriteRegStr HKEY_CLASSES_ROOT "feed" "" "URL: RSS Protocol"
	   WriteRegStr HKEY_CLASSES_ROOT "feed" "URL Protocol" ""
	   WriteRegStr HKEY_CLASSES_ROOT "feed\shell\open\command" "" "$INSTDIR\${WINAMPEXE} /HANDLE %1"
	   SetOutPath $INSTDIR\System
	   File /nonfatal ${FILES_PATH}\system\omBrowser.w5s
     ${WinampSectionEnd}                                                         ; <<< [SHOUTCast Wire]
     !endif
    !endif ; NOKIA

   !ifndef WINAMP64
   !ifdef full
    ${WinampSection} "mediaLibraryWebAddons" $(IDS_SEC_ML_ADDONS) IDX_SEC_ML_ADDONS                ; >>> [Addons]
      ${SECTIONIN_FULL}
	  SetOutPath "$INSTDIR\Plugins"
      File "${FILES_PATH}\Plugins\ml_addons.dll"
	  SetOutPath "$INSTDIR\System"
	  File /nonfatal "${FILES_PATH}\system\omBrowser.w5s"
	  File /nonfatal "${FILES_PATH}\system\auth.w5s"
    ${WinampSectionEnd}                                                    ; <<< [Addons]
    !endif
   !endif

   !ifndef WINAMP64
   !ifdef full
    ${WinampSection} "mediaLibraryOnlineServices" $(secOM) IDX_SEC_ML_ONLINE                                   ; >>> [Online Media]
		${SECTIONIN_FULL}
		!ifdef NOKIA
			SetOutPath "$INSTDIR\Plugins"
			File "/oname=ml_online.dll" "${FILES_PATH}\plugins\ml_online_nokia.dll"
			File "..\..\resources\data\nokia.html"
		!else
			StrCpy $0 "$SETTINGSDIR\Plugins\ml"
			CreateDirectory "$0"
			
			Delete "$0\radio.*"
			Delete "$0\tv.*"
			Delete "$0\waaudio.*"
			Delete "$0\wamedia.*"
			Delete "$0\watv.*"
			Delete "$0\xmmedia.*"
			Delete "$0\ml_win_media.ini"
		  
			SetOutPath "$INSTDIR\Plugins"
			File "${FILES_PATH}\Plugins\ml_online.dll"
			;!warning "ml_downloads disabled to make 5.5.2 (2 of 2)"
			File "${FILES_PATH}\plugins\ml_downloads.dll"
		!endif
	 
		 StrCpy $0 "$SETTINGSDIR\Plugins\ml\ml_online.ini"
		 DeleteINIStr "$0" "Setup" "featuredExtra"
		 DeleteINIStr "$0" "Navigation" "openOnce"
		 DeleteINIStr "$0" "Navigation" "openOnceMode"
		 ${If} "$PROMOTIONCONFIG" != ""
			ReadINIStr $1 "$PROMOTIONCONFIG" "OnlineMedia" "register"
			${If} $1 != ""
				WriteINIStr "$0" "Setup" "featuredExtra" $1
			${EndIf}
			ReadINIStr $1 "$PROMOTIONCONFIG" "OnlineMedia" "open"
			${If} $1 != ""
				WriteINIStr "$0" "Navigation" "openOnce" $1
				ReadINIStr $1 "$PROMOTIONCONFIG" "OnlineMedia" "openMode"
				${If} $1 != ""
					WriteINIStr "$0" "Navigation" "openOnceMode" $1
				${EndIf}
			${EndIf}
		 ${EndIf}
	 
		SetOutPath "$INSTDIR\System"
		File /nonfatal "${FILES_PATH}\system\omBrowser.w5s"
		File /nonfatal "${FILES_PATH}\system\auth.w5s"
	${WinampSectionEnd}                                                             ; <<< [Online Media]
    !endif
   !endif
   
   !ifdef full
    ${WinampSection} "mediaLibraryPlaylistGenerator" $(SEC_ML_PLG) IDX_SEC_ML_PLG                         ; >>> [Playlist Generator]
      ${SECTIONIN_FULL}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\ml_plg.dll
    ${WinampSectionEnd}                                                            ; <<< [Playlist Generator]
    !endif
	
    !ifdef full
    ${WinampSection} "mediaLibraryCloud" $(IDS_SEC_CLOUD) IDX_SEC_CLOUD                ; >>> [Cloud]
      ${SECTIONIN_FULL}
      SetOutPath "$INSTDIR"
      File "${FILES_PATH}\nxlite.dll"
      File "${FILES_PATH}\jnetlib.dll"
	  SetOutPath "$INSTDIR\Plugins"
      File "${FILES_PATH}\Plugins\ml_cloud.dll"
      File "${FILES_PATH}\Plugins\pmp_cloud.dll"
	  SetOutPath "$INSTDIR\System"
	  File /nonfatal "${FILES_PATH}\system\omBrowser.w5s"
	  File /nonfatal "${FILES_PATH}\system\Wasabi2.w5s"
	  SetOutPath "$INSTDIR\Components"
	  File /nonfatal "${FILES_PATH}\components\cloud.w6c"
	  File /nonfatal "${FILES_PATH}\components\ssdp.w6c"
	  
    ${WinampSectionEnd}                                                    ; <<< [Cloud]
    !endif

   !include ".\sections\grp_walib_pmp.nsh"
   
 SectionGroupEnd
!endif  ;  FULL