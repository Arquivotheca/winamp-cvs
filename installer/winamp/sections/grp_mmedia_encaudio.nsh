!ifndef LITE
SectionGroup $(IDS_GRP_MMEDIA_AUDIO_ENC) IDX_GRP_MMEDIA_AUDIO_ENC ; Audio Encoders

  !ifndef WINAMP64
   !ifdef FULL
    ${WinampSection} "winampWmaEncoder" $(secWMAE) IDX_SEC_WMA_ENC
      ${SECTIONIN_FULL}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\enc_wma.dll
    ${WinampSectionEnd}
   !endif ; FULL
  !endif ; WINAMP64

  !ifndef WINAMP64
    !ifdef STD | FULL
      ${WinampSection} "winampWavEncoder" $(IDS_SEC_WAV_ENC) IDX_SEC_WAV_ENC
        ${SECTIONIN_STD}
        SetOutPath $INSTDIR\Plugins
        File ${FILES_PATH}\plugins\enc_wav.dll
      ${WinampSectionEnd}
    !endif ; STD | FULL
  !endif ; WINAMP64

  !ifndef WINAMP64
   !ifdef STD | FULL
    ${WinampSection} "winampMp3Encoder" $(secMP3E) IDX_SEC_MP3_ENC
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\enc_lame.dll
      File ..\..\resources\libraries\lame_enc.dll
    ${WinampSectionEnd}
   !endif ;  FULL
  !endif ; WINAMP64

  !ifndef WINAMP64
   !ifdef STD | FULL
     ${WinampSection} "encoderAac" $(secAACE) IDX_SEC_AAC_ENC
       ${SECTIONIN_STD}
       SetOutPath $INSTDIR\Plugins
       Delete $INSTDIR\Plugins\enc_aac.dll
       Delete $INSTDIR\Plugins\enc_mp4.dll
       File ${FILES_PATH}\plugins\enc_fhgaac.dll
       SetOutPath $INSTDIR
       File ${FILES_PATH}\libmp4v2.dll
       SetOutPath $INSTDIR\Plugins
     ${WinampSectionEnd}
   !endif ; FULL
  !endif ; WINAMP64

  !ifndef WINAMP64
   !ifndef NOKIA
    !ifdef STD | FULL
     ${WinampSection} "encoderFlac" $(IDS_SEC_FLAC_ENC) IDX_SEC_FLAC_ENC
       ${SECTIONIN_STD}
       SetOutPath $INSTDIR\Plugins
       File ${FILES_PATH}\plugins\enc_flac.dll
       SetOutPath $INSTDIR
       File ${FILES_PATH}\libFLAC.dll
       SetOutPath $INSTDIR\Plugins
     ${WinampSectionEnd}
    !endif ; FULL
   !endif ; NOKIA
  !endif ; WINAMP64

  !ifndef WINAMP64
   !ifndef NOKIA
    !ifdef STD | FULL
;   !ifdef BETA | NIGHT
     ${WinampSection} "encoderOgg" $(secOGGEnc) IDX_SEC_OGG_ENC
       ${SECTIONIN_STD}
       SetOutPath $INSTDIR\Plugins
;      File /oname=enc_vorbis.dll "..\..\resources\plugins\enc_vorbis_lzma.dll"
;      File "..\..\resources\plugins\enc_vorbis.dll"
       File ${FILES_PATH}\plugins\enc_vorbis.dll
     ${WinampSectionEnd}
;   !endif ; BETA
    !endif ; FULL
   !endif ; NOKIA
  !endif ; WINAMP64

SectionGroupEnd ;  Audio Encoders
!endif ; NOT LITE