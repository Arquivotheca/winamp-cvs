SectionGroup $(IDS_GRP_MMEDIA_AUDIO_DEC) IDX_GRP_MMEDIA_AUDIO_DEC ; Audio Playback

  ${WinampSection} "decoderMp3" $(IDS_SEC_MP3_DEC) IDX_SEC_MP3_DEC
    SectionIn 1 2 3 4 5 6 7 8 RO
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\in_mp3.dll
    SetOutPath "$INSTDIR\System"
!ifndef WINAMP64
    File "${FILES_PATH}\System\vlb.w5s"
    File /nonfatal "${FILES_PATH}\System\vlb.wbm"
!endif
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayMusicFilesOnArrival" "${WINAMP}PlayMediaOnArrival" ""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival" "Action" "$(AutoplayHandler)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival" "DefaultIcon" "$INSTDIR\${WINAMPEXE},0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival" "InvokeProgid" "${WINAMP}.File"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival" "InvokeVerb" "Play"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival" "Provider" "${WINAMP}"
  ${WinampSectionEnd}

  ${WinampSection} "decoderWma" $(secWMA) IDX_SEC_WMA_DEC
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\in_wm.dll
  ${WinampSectionEnd}

  !ifndef WINAMP64
   ${WinampSection} "decoderMidi" $(secMIDI) IDX_SEC_MIDI_DEC
     ${SECTIONIN_LITE}
     SetOutPath $INSTDIR\Plugins
     Delete $INSTDIR\Plugins\in_dm.dll
     Delete $INSTDIR\Plugins\in_midi_dm.dll
     File ${FILES_PATH}\plugins\in_midi.dll
     File ..\..\resources\libraries\read_file.dll
   ${WinampSectionEnd}
  !endif ;WINAMP64

  !ifndef WINAMP64
    ${WinampSection} "decoderMod" $(secMOD) IDX_SEC_MOD_DEC
     ${SECTIONIN_LITE}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\in_mod.dll
     File ..\..\resources\libraries\read_file.dll
   ${WinampSectionEnd}
  !endif ;WINAMP64

  !ifndef WINAMP64
   !ifndef NOKIA
    ${WinampSection} "decoderOgg" $(secOGGPlay) IDX_SEC_OGG_DEC
      ${SECTIONIN_LITE}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_vorbis.dll
    ${WinampSectionEnd}
   !endif ; NOKIA
  !endif ; WINAMP64

  ${WinampSection} "decoderMp4" $(secMP4E) IDX_SEC_MP4_DEC
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\in_mp4.dll
    SetOutPath $INSTDIR
    Delete $INSTDIR\Plugins\libmp4v2.dll
    File ${FILES_PATH}\libmp4v2.dll
    SetOutPath $INSTDIR\System
    ; aacPlusDecoder.w5s is installed by default (see winamp.nsh)
    ; File ${FILES_PATH}\System\aacPlusDecoder.w5s
    ; File /nonfatal "${FILES_PATH}\system\aacPlusDecoder.wbm"
    SetOutPath $INSTDIR\System
    File ${FILES_PATH}\System\alac.w5s
    File /nonfatal ${FILES_PATH}\System\alac.wbm
  ${WinampSectionEnd}

 !ifndef NOKIA
  ${WinampSection} "decoderFlac" $(IDS_SEC_FLAC_DEC) IDX_SEC_FLAC_DEC
    ${SECTIONIN_LITE}
    SetOutPath $INSTDIR
    File ${FILES_PATH}\libFLAC.dll
    File ${FILES_PATH}\nxlite.dll
    File ${FILES_PATH}\jnetlib.dll
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\in_flac.dll
  ${WinampSectionEnd}
 !endif ; NOKIA

   ${WinampSection} "decoderCdda" $(secCDDA) IDX_SEC_CDDA_DEC
     ${SECTIONIN_LITE}
     SetOutPath $INSTDIR
     File ${FILES_PATH}\nde.dll
     File ${FILES_PATH}\nxlite.dll
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\in_cdda.dll
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival" "${WINAMP}PlayMediaOnArrival" ""
   ${WinampSectionEnd}

   ${WinampSection} "decoderWav" $(secWAV) IDX_SEC_WAV_DEC
     ${SECTIONIN_LITE}
     !ifdef old_in_wave_plugin
      SetOutPath $INSTDIR\Plugins
      File ..\..\resources\plugins\in_wave.dll
     !else
      SetOutPath $INSTDIR
      File ${FILES_PATH}\libsndfile.dll
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_wave.dll
     !endif ; old_in_wave_plugin
   ${WinampSectionEnd}

SectionGroupEnd ;  Audio Playback

