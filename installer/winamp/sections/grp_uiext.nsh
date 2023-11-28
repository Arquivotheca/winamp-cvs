 SectionGroup $(IDS_GRP_UIEXTENSION) IDX_GRP_UIEXTENSION ;  User Interface Extensions

  ${WinampSection} "GlobalHotkeys" $(secHotKey) IDX_SEC_HOTKEY      ; >>> [Global Hotkey Support]
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\gen_hotkeys.dll
  ${WinampSectionEnd}                                               ; <<< [Global Hotkey Support]

 !ifndef WINAMP64
  ${WinampSection} "JTFE" $(secJmp) IDX_SEC_JUMPEX                  ; >>> [Extended Jump To File Support]
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR\Plugins
    File ..\..\resources\plugins\gen_jumpex.dll
    ; temp section to remove build 2940-specific bug (will be removing this before 5.58 final)
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.SkinZip\shell\Enqueue"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.SkinZip\shell\EnqueueAndPlay"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.SkinZip\shell\Play"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.SkinZip\shell\WinampLibrary"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.LangZip\shell\Enqueue"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.LangZip\shell\EnqueueAndPlay"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.LangZip\shell\Play"
    DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.LangZip\shell\WinampLibrary"
  ${WinampSectionEnd}                                               ; <<< [Extended Jump To File Support
 !endif ; WINAMP64

  ${WinampSection} "TrayControl" $(secTray) IDX_SEC_TRAYCTRL        ; >>> [Nullsoft Tray Control]
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\gen_tray.dll
  ${WinampSectionEnd}                                               ; <<< [Nullsoft Tray Control]

  !ifndef WINAMP64
   !ifdef full | std
    ${WinampSection} "FreeformSkins" $(compModernSkin) IDX_SEC_FREEFORM      ; >>> [Modern Skin Support]
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\System
      File "${FILES_PATH}\system\filereader.w5s"
      File "${FILES_PATH}\system\dlmgr.w5s"
      File "${FILES_PATH}\system\timer.w5s"

      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\gen_ff.dll
      File ${FILES_PATH}\tataki.dll ; copy this in here to eliminate problems with third party utilities like virus scanners

     DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_DEFAULT_SKIN)..."
     SetDetailsPrint none

      File /r /x CVS /x *.psd "..\..\resources\data\freeform"
      Delete $INSTDIR\Plugins\freeform\wacs\jpgload\jpgload.wac
      SetOutPath $INSTDIR\Plugins\freeform\wacs\freetype
      File ${FILES_PATH}\plugins\freeform\wacs\freetype\freetype.wac

      SetOutPath $SETTINGSDIR
      File "..\..\resources\data\links.xml"

      Call GetSkinDir
      Pop $R0
      
     !ifdef NOKIA
      SetOutPath "$R0"
      File "/oname=Nokia Edition.wal" "..\..\resources\skins\Nokia\Nokia_Edition.wal"
     !else
      SetOutPath "$R0\${MODERNSKINNAME}"
      File /r /x CVS /x *.mi /x *.bat /x *.m "..\..\resources\skins\${MODERNSKINNAME}\*.*"

      SetOutPath "$R0\Bento"
      File /r /x CVS /x *.mi /x *.m /x *.psd "..\..\resources\skins\Bento\*.*"
      
      SetOutPath "$R0\Big Bento"
      File /r /x CVS /x *.mi /x *.bat /x *.m /x *.psd "..\..\resources\skins\Big Bento\*.*"
     !endif
      SetDetailsPrint lastused
      SetOutPath "$INSTDIR\Plugins"
      
      ReadIniStr $0 "$WINAMPINI" "Winamp" "skin"
      ${If} "$0" == "Winamp Bento"
        WriteIniStr "$WINAMPINI" "Winamp" "skin" "Bento"
      ${EndIf}
    ${WinampSectionEnd}                                            ; <<< [Modern Skin Support]
   !endif ; full | std
  !endif ; WINAMP64

  !ifdef ENABLE_DROPBOX  
  !ifndef WINAMP64
   !ifdef FULL
    ${WinampSection} "DropBox" $(IDS_SEC_GEN_DROPBOX)  IDX_SEC_GEN_DROPBOX   ; >>> [DropBox plugin]
	  ${SECTIONIN_FULL}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\gen_dropbox.dll
    ${WinampSectionEnd}                                                      ; <<< [DropBox plugin]
  !endif ; FULL
 !endif ; WINAMP64
 !endif
 
 ${FrenchRadio_InsertInstallSections}
 
SectionGroupEnd ;  User Interface Extensions