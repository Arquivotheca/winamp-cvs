!include "logicLib.nsh"

!macro DllUnregisterAndDelete __dllPath
	${If} ${FileExists} "${__dllPath}"
		UnRegdll "${__dllPath}"
		Delete /REBOOTOK "${__dllPath}"
    ${EndIf}
!macroend
!define DllUnregisterAndDelete "!insertmacro 'DllUnregisterAndDelete'"

Section -HiddenGracenote
  ; check if at least one dependent section selected
	${IfNot} ${SectionIsSelected} ${IDX_SEC_CDDB}
  !ifdef IDX_SEC_ML_AUTOTAG
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_ML_AUTOTAG}
  !endif
  !ifdef IDX_SEC_ML_NOWPLAYING
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_ML_NOWPLAYING}
  !endif
  !ifdef IDX_SEC_ML_PLG
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_ML_PLG}
  !endif
  !ifdef IDX_SEC_ML_LOCAL
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_ML_LOCAL}
  !endif
	; Gracenote doesn't need installing, remove any existing files
		
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\cddbcontrolwinamp.dll"
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\cddbcontrolwinamp.dll"
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\cddbuiwinamp.dll"
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
		${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
		Delete "$INSTDIR\Plugins\Gracenote\CddbWOManagerWinamp.dll"
		Delete "$INSTDIR\Plugins\Gracenote\Cddbx*.dll"
		Delete "$INSTDIR\System\gracenote.w5s"
		
		Goto GracenoteInstall_SectionEnd
    ${EndIf}

  ; CDDBControl required for all gracenote stuff
    DetailPrint "$(IDS_RUN_INSTALL) Gracenote..."
    SetDetailsPrint none

    SetOutPath "$INSTDIR\System"
    File "${FILES_PATH}\system\gracenote.w5s"

    SetOutPath "$INSTDIR\Plugins"
    ${If} ${FileExists} "$OUTDIR\cddbcontrolwinamp.dll"
      UnRegdll "$OUTDIR\cddbcontrolwinamp.dll"
      Delete "$OUTDIR\cddbcontrolwinamp.dll"
    ${EndIf}
    SetOutPath "$INSTDIR\Plugins\Gracenote"
    ${If} ${FileExists} "$OUTDIR\cddbcontrolwinamp.dll"
      UnRegdll "$OUTDIR\cddbcontrolwinamp.dll"
      Delete "$OUTDIR\cddbcontrolwinamp.dll"
    ${EndIf}
    File "..\..\gracenote\cddbcontrolwinamp.dll"
    Regdll "$OUTDIR\cddbcontrolwinamp.dll"

  ; GracenoteCddbUI_Section - CDDB UI only required for in_cdda/CDDB
  ; First, remove cddbuiwinamp.dll if CDDB not selected
    ${IfNot} ${SectionIsSelected} ${IDX_SEC_CDDB}
      ${If} ${FileExists} "$INSTDIR\Plugins\cddbuiwinamp.dll"
        UnRegdll "$INSTDIR\Plugins\cddbuiwinamp.dll"
        Delete "$INSTDIR\Plugins\cddbuiwinamp.dll"
      ${EndIf}
      ${If} ${FileExists} "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
        UnRegdll "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
        Delete "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
      ${EndIf}
		Goto GracenoteCddbUI_SectionEnd
    ${EndIf}
  ; CDDB is selected, install cddbuiwinamp.dll
    ${If} ${SectionIsSelected} ${IDX_SEC_CDDB}
      SetOutPath "$INSTDIR\Plugins"
      ${If} ${FileExists} "$OUTDIR\cddbuiwinamp.dll"
        UnRegdll "$OUTDIR\cddbuiwinamp.dll"
        Delete "$OUTDIR\cddbuiwinamp.dll"
      ${EndIf}
      SetOutPath "$INSTDIR\Plugins\Gracenote"
      ${If} ${FileExists} "$OUTDIR\cddbuiwinamp.dll"
        UnRegdll "$OUTDIR\cddbuiwinamp.dll"
        Delete "$OUTDIR\cddbuiwinamp.dll"
      ${EndIf}
      File "..\..\gracenote\cddbuiwinamp.dll"
      Regdll "$OUTDIR\cddbuiwinamp.dll"
	${EndIf}
	
    GracenoteCddbUI_SectionEnd:

  ; GracenoteMusicIDSubmit_Section - install MusicID-Submit if both ml_disc and sonic are present
    !ifdef full
  ; First, remove MusicID-Submit DLL's if all dependent sections not selected
    ${IfNot} ${SectionIsSelected} ${IDX_SEC_ML_DISC}
    ${OrIfNot} ${SectionIsSelected} ${IDX_SEC_SONIC_SUPPORT}
      ${If} ${FileExists} "$INSTDIR\Plugins\Gracenote\CddbWOManagerWinamp.dll"
        Delete "$INSTDIR\Plugins\Gracenote\CddbWOManagerWinamp.dll"
        Delete "$INSTDIR\Plugins\Gracenote\Cddbx*.dll"
      ${EndIf}
		Goto GracenoteMusicIDSubmit_SectionEnd
    ${EndIf}
  ; All dependent sections selected, install MusicID-Submit DLL's
    ${If} ${SectionIsSelected} ${IDX_SEC_CDDB}
    ${AndIf} ${SectionIsSelected} ${IDX_SEC_ML_DISC}
    ${AndIf} ${SectionIsSelected} ${IDX_SEC_SONIC_SUPPORT}
      SetOutPath "$INSTDIR\Plugins\Gracenote"
      File "..\..\gracenote\CddbWOManagerWinamp.dll"
      File "..\..\gracenote\Cddbx1.dll"
      File "..\..\gracenote\Cddbx2.dll"
      File "..\..\gracenote\Cddbx3.dll"
      File "..\..\gracenote\Cddbx4.dll"
      File "..\..\gracenote\Cddbx5.dll"
    ${EndIf}
    GracenoteMusicIDSubmit_SectionEnd:
    !endif ; full

  ; GracenoteMusicID_Section - CddbMusicIDWinamp required for autotag and playlist generator
    !ifdef std | full
  ; First, remove CddbMusicIDWinamp if dependent sections not selected
    ${IfNot} ${SectionIsSelected} ${IDX_SEC_ML_AUTOTAG}
    ${AndIfNot} ${SectionIsSelected} ${IDX_SEC_ML_PLG}
      ${If} ${FileExists} "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
      UnRegdll "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
      Delete "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
      ${EndIf}
      Goto GracenoteMusicID_SectionEnd
    ${EndIf}
  ; At least one dependent section selected, install CddbMusicIDWinamp.dll
    ${If} ${SectionIsSelected} ${IDX_SEC_ML_AUTOTAG}
    ${OrIf} ${SectionIsSelected} ${IDX_SEC_ML_PLG}
      SetOutPath "$INSTDIR\Plugins\Gracenote"
      ${If} ${FileExists} "$OUTDIR\CddbMusicIDWinamp.dll"
        UnRegdll "$OUTDIR\CddbMusicIDWinamp.dll"
        Delete "$OUTDIR\CddbMusicIDWinamp.dll"
      ${EndIf}
      File "..\..\gracenote\CddbMusicIDWinamp.dll"
      Regdll "$OUTDIR\CddbMusicIDWinamp.dll"
    ${EndIf}
    GracenoteMusicID_SectionEnd:
    !endif ; std | full

  ; GracenotePLG_Section - playlist generator needs this
    !ifdef full
  ; First, remove CddbPlaylist2Winamp if ml_plg not selected
    ${IfNot} ${SectionIsSelected} ${IDX_SEC_ML_PLG}
      ${If} ${FileExists} "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
      UnRegdll "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
      Delete "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
      ${EndIf}
      Goto GracenotePLG_SectionEnd
    ${EndIf}
  ; Dependent section selected, install CddbPlaylist2Winamp.dll
    ${If} ${SectionIsSelected} ${IDX_SEC_ML_PLG}
      SetOutPath "$INSTDIR\Plugins\Gracenote"
      ${If} ${FileExists} "$OUTDIR\CddbPlaylist2Winamp.dll"
        UnRegdll "$OUTDIR\CddbPlaylist2Winamp.dll"
        Delete "$OUTDIR\CddbPlaylist2Winamp.dll"
      ${EndIf}
      File "..\..\gracenote\CddbPlaylist2Winamp.dll"
      Regdll "$OUTDIR\CddbPlaylist2Winamp.dll"
    ${EndIf}
    GracenotePLG_SectionEnd:
    !endif ; full

    SetDetailsPrint lastused
    DetailPrint "$(IDS_RUN_DONE)"
  
  GracenoteInstall_SectionEnd:

SectionEnd