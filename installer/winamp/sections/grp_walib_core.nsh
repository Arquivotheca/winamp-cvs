SectionGroup $(IDS_GRP_WALIB_CORE) IDX_GRP_WALIB_CORE  ;  Core Media Library Components

  ${WinampSection} "mediaLibraryLocalMedia" $(sec_ML_LOCAL) IDX_SEC_ML_LOCAL                            ; >>> [Local Media]
    ${SECTIONIN_STD}
    SetOutPath $INSTDIR
    File ${FILES_PATH}\nde.dll
    File ${FILES_PATH}\nxlite.dll
    File ${FILES_PATH}\jnetlib.dll
    SetOutPath $INSTDIR\System
    File ${FILES_PATH}\system\dlmgr.w5s
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\ml_local.dll
    ${If} ${FileExists} "$INSTDIR\Plugins\gen_pcfix.dll"
    Rename "$INSTDIR\Plugins\gen_pcfix.dll" "$INSTDIR\Plugins\gen_pcfix.dll.off" ; disable ShaneH's playcount tracking plugin
    ${EndIf}
  ${WinampSectionEnd}                                                             ; <<< [Local Media]

   ${WinampSection} "mediaLibraryPlaylists" $(sec_ML_PLAYLISTS) IDX_SEC_ML_PLAYLISTS                   ; >>> [Playlists]
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_playlists.dll
     SetOutPath $INSTDIR
     File ${FILES_PATH}\nxlite.dll
   ${WinampSectionEnd}                                                            ; <<< [Playlists]
  
!ifndef WINAMP64
  !ifdef full
   ${WinampSection} "mediaLibraryRipBurn" $(sec_ML_DISC) IDX_SEC_ML_DISC                             ; >>> [CD Rip  & Burn]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR
!ifndef WINAMP64
     File ${FILES_PATH}\burnlib.dll
     SetOutPath $INSTDIR\System
     File ${FILES_PATH}\system\primo.w5s
!endif
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_disc.dll
   ${WinampSectionEnd}                                                            ; <<< [CD Rip  & Burn]
   !endif
!endif

!ifndef WINAMP64
  !ifdef full
   ${WinampSection} "mediaLibraryBookmarks" $(sec_ML_BOOKMARKS) IDX_SEC_ML_BOOKMARKS                   ; >>> [Bookmarks]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_bookmarks.dll
   ${WinampSectionEnd}                                                  ; <<< [Bookmarks]
   !endif
!endif ;WINAMP64

!ifndef WINAMP64
  !ifdef full
   ${WinampSection} "mediaLibraryHistory" $(sec_ML_HISTORY) IDX_SEC_ML_HISTORY                       ; >>> [History]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR
     File ${FILES_PATH}\nde.dll
     File ${FILES_PATH}\nxlite.dll
     File ${FILES_PATH}\jnetlib.dll
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_history.dll
   ${WinampSectionEnd}                                                           ; <<< [History]
   !endif
!endif ;WINAMP64

  !ifndef WINAMP64
  !ifdef full
   ${WinampSection} "mediaLibraryNowPlaying" $(sec_ML_NOWPLAYING) IDX_SEC_ML_NOWPLAYING                 ; >>> [Now playing]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\ml_nowplaying.dll
	 SetOutPath $INSTDIR\System
	 File /nonfatal ${FILES_PATH}\system\omBrowser.w5s
	 File /nonfatal ${FILES_PATH}\system\auth.w5s
   ${WinampSectionEnd}                                                           ;<<< [Now playing]
   !endif
  !endif ; WINAMP64

SectionGroupEnd ; Core Media Library Components
