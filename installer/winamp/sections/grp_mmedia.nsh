SectionGroup $(IDS_GRP_MMEDIA) IDX_GRP_MMEDIA ; Multimedia Engine

  !include ".\sections\grp_mmedia_decaudio.nsh"
  !include ".\sections\grp_mmedia_decvideo.nsh"
  !include ".\sections\grp_mmedia_encaudio.nsh"
  !include ".\sections\grp_mmedia_output.nsh"

  !ifndef WINAMP64
   ${WinampSection} "cddb" $(secCDDB) IDX_SEC_CDDB                                          ; >>> [CDDB for recognizing CDs]
     ${SECTIONIN_LITE}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\in_cdda.dll
   ${WinampSectionEnd}                                                   ; <<< [CDDB for recognizing CDs]
  !endif ; WINAMP64
  
  !ifndef WINAMP64
   !ifdef FULL
    ${WinampSection} "sonicLibrary" $(secSonicBurning)  IDX_SEC_SONIC_SUPPORT          ; >>> [Sonic Ripping/Burning support]
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR
      Delete $INSTDIR\pxsdkpls.dll
      Delete $INSTDIR\primosdk.dll
      Delete $INSTDIR\pconfig.dcf
      Delete $INSTDIR\Plugins\ml_databurner.dll
      
      SetOutPath $INSTDIR\System
      File ${FILES_PATH}\system\primo.w5s
      
      SetOutPath $INSTDIR
      ;StrCmp $IsNT "0" veritas_done ; install only on nt systems
      File "${PRIMOSDK_PATH}\pxsdkpls.dll"
	  File "${PRIMOSDK_PATH}\pconfig.dcf"
      
      WriteINIStr "$WINAMPINI" "CDDA/Line Input Driver" "rip_veritas" "1"
      WriteINIStr "$WINAMPINI" "CDDA/Line Input Driver" "use_veritas" "1"

      DetailPrint "$(IDS_RUN_INSTALL) Sonic Runtime..."
      SetDetailsPrint none

      Push $R1

      SetOutPath "$PLUGINSDIR\PrimoRedist"
      File "${PRIMOSDK_REDIST_PATH}\*"
      SetOutPath $INSTDIR\Plugins

      ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" "SonicRef"
      ${If} $0 > 0
        ExecWait '"$PLUGINSDIR\PrimoRedist\pxsetup.exe" /NOSHARECOUNT' $R1
      ${Else}
        ExecWait '"$PLUGINSDIR\PrimoRedist\pxsetup.exe"' $R1
      ${EndIf}

      SetDetailsPrint lastused
      
      ${If} $R1 = -1
        DetailPrint "$(IDS_RUN_INSTALLFIALED)"
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(msgCDError). '$R1'"
      ${Else}
        IntOp $0 $0 + 1
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" "SonicRef" $0
        ${If} $R1 = 3010
          ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" "CurrentVersion"
          StrCpy $0 $0 1
          ${If} $0 <= 4 
            SetRebootFlag TRUE
          ${EndIf}
        ${EndIf}
        DetailPrint "$(IDS_RUN_DONE)"
      ${EndIf}
      Pop $R1
    ${WinampSectionEnd}                                                ; <<< [Sonic Ripping/Burning support]
   !endif ; full
  !endif ; WINAMP64


 !ifndef WINAMP64
  !ifdef STD | FULL
   ${WinampSection} "digitalSignalProcessing" $(secDSP) IDX_SEC_DSP                                       ; >>> [Signal Processor Studio Plug-in]
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File "..\..\resources\plugins\dsp_sps.dll"
     DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_DSP_PRESETS)..." ; Extracting presets...
     SetDetailsPrint none
     SetOutPath $INSTDIR\Plugins\DSP_SPS
     File "..\..\resources\data\dsp_sps\*.sps"
     SetOutPath $INSTDIR\Plugins
     SetDetailsPrint lastused
   ${WinampSectionEnd}                                                  ; <<< [Signal Processor Studio Plug-in]
  !endif ; std | full
 !endif ; WINAMP64

SectionGroupEnd ; Multimedia Engine
