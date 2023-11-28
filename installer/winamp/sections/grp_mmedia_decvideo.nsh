!ifdef std | full
 SectionGroup $(IDS_GRP_MMEDIA_VIDEO_DEC)  IDX_GRP_MMEDIA_VIDEO_DEC ; Video Playback

   ${WinampSection} "decoderWmv" $(secWMV) IDX_SEC_WMV_DEC
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\in_wm.dll
     ClearErrors
     ReadINIStr $0 "$WINAMPINI" "in_dshow" "extlist"
     ${IfNot} ${Errors}
       ${If} $0 <> ""
         extstrip::remove "WMV" $0
         Pop $0
         extstrip::remove "ASF" $0
         Pop $0
         WriteINIStr "$WINAMPINI" "in_dshow" "extlist" $0
       ${EndIf}
     ${EndIf}
   ${WinampSectionEnd}
   
   !ifndef WINAMP64
   ${WinampSection} "decoderNsv" $(secNSV) IDX_SEC_NSV_DEC
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\in_nsv.dll
     File ${FILES_PATH}\plugins\nsvdec_vp3.dll
     File ${FILES_PATH}\plugins\nsvdec_vp5.dll
     Delete $INSTDIR\Plugins\nsvdec_vp6.dll ; delete old VP6 plugin
     SetOutPath $INSTDIR\System
     File ${FILES_PATH}\System\vp6.w5s
     File /nonfatal ${FILES_PATH}\System\vp6.wbm
     File ${FILES_PATH}\System\vp8.w5s
     File /nonfatal ${FILES_PATH}\System\vp8.wbm
     WriteRegStr HKEY_CLASSES_ROOT "UNSV" "" "URL: Ultravox Protocol"
     WriteRegStr HKEY_CLASSES_ROOT "UNSV" "URL Protocol" ""
     WriteRegStr HKEY_CLASSES_ROOT "UNSV\shell\open\command" "" "$INSTDIR\${WINAMPEXE} %1"
   ${WinampSectionEnd}
   !endif ; Winamp64

   !ifndef WINAMP64
    ${WinampSection} "decoderDirectShow" $(secDSHOW) IDX_SEC_DSHOW_DEC
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_dshow.dll
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayVideoFilesOnArrival" "${WINAMP}PlayMediaOnArrival" ""
    ${WinampSectionEnd}
   !endif ; WINAMP64
   
    ${WinampSection} "decoderAvi" $(secAVI) IDX_SEC_AVI_DEC
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_avi.dll
      
      ; remove AVI from in_dshow's extension list
      ClearErrors
      ReadINIStr $0 "$WINAMPINI" "in_dshow" "extlist"
      ${IfNot} ${Errors}
        ${If} $0 <> ""
          extstrip::remove "AVI" $0
          Pop $0
          WriteINIStr "$WINAMPINI" "in_dshow" "extlist" $0
        ${EndIf}
      ${EndIf}
      
      SetOutPath $INSTDIR\System
      
      ; AVI video codecs
      File ${FILES_PATH}\system\h264.w5s
      File /nonfatal ${FILES_PATH}\System\h264.wbm
	  File ${FILES_PATH}\System\vp6.w5s
	  File /nonfatal ${FILES_PATH}\System\vp6.wbm
	  File ${FILES_PATH}\System\mp4v.w5s
	  File /nonfatal ${FILES_PATH}\System\mp4v.wbm
	  
	  ; AVI audio codecs
	  ; aacPlusDecoder.w5s is installed by default (see winamp.nsh)
	  ; File ${FILES_PATH}\System\aacPlusDecoder.w5s
	  ; File /nonfatal ${FILES_PATH}\System\aacPlusDecoder.wbm
	  File ${FILES_PATH}\System\adpcm.w5s
	  File /nonfatal ${FILES_PATH}\System\adpcm.wbm
	  File ${FILES_PATH}\System\pcm.w5s
	  File /nonfatal ${FILES_PATH}\System\pcm.wbm
	  
	  ; beta only stuff
	  !ifdef BETA | NIGHT
	    File ${FILES_PATH}\System\a52.w5s
	    File /nonfatal ${FILES_PATH}\System\a52.wbm
	  !endif
    ${WinampSectionEnd}

    ${WinampSection} "decoderFlv" $(secFLV) IDX_SEC_FLV_DEC
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_flv.dll
      SetOutPath $INSTDIR\System

      ; FLV video codecs
      File ${FILES_PATH}\system\h264.w5s
      File /nonfatal ${FILES_PATH}\System\h264.wbm
	  File ${FILES_PATH}\System\vp6.w5s
	  File /nonfatal ${FILES_PATH}\System\vp6.wbm
	  
	  ; FLV audio codecs
	  ; aacPlusDecoder.w5s is installed by default (see winamp.nsh)
	  ; File ${FILES_PATH}\System\aacPlusDecoder.w5s
	  ; File /nonfatal ${FILES_PATH}\System\aacPlusDecoder.wbm
	  File ${FILES_PATH}\System\adpcm.w5s
	  File /nonfatal ${FILES_PATH}\System\adpcm.wbm
	  
	  ; beta only stuff
	  !ifdef BETA | NIGHT
        File ${FILES_PATH}\System\f263.w5s
        File /nonfatal ${FILES_PATH}\System\f263.wbm
      !endif ;  BETA | NIGHT
    ${WinampSectionEnd}
    
    ${WinampSection} "decoderMkv" $(secMKV) IDX_SEC_MKV_DEC
      ${SECTIONIN_STD}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_mkv.dll
      SetOutPath $INSTDIR\System
      
      ; MKV video codecs
      File ${FILES_PATH}\System\h264.w5s
      File /nonfatal ${FILES_PATH}\System\h264.wbm
      File ${FILES_PATH}\System\vp8.w5s
      File /nonfatal ${FILES_PATH}\System\vp8.wbm
      File ${FILES_PATH}\System\theora.w5s
      File /nonfatal ${FILES_PATH}\System\theora.wbm
      
      ; MKV audio codecs
	  ; aacPlusDecoder.w5s is installed by default (see winamp.nsh)
	  ; File ${FILES_PATH}\System\aacPlusDecoder.w5s
	  ; File /nonfatal ${FILES_PATH}\System\aacPlusDecoder.wbm
	  
	  ; beta only stuff
	  !ifdef BETA | NIGHT
	    File ${FILES_PATH}\System\a52.w5s
	    File /nonfatal ${FILES_PATH}\System\a52.wbm
		File ${FILES_PATH}\System\f263.w5s
        File /nonfatal ${FILES_PATH}\System\f263.wbm
        File ${FILES_PATH}\System\dca.w5s
        File /nonfatal ${FILES_PATH}\System\dca.wbm
	  !endif
    ${WinampSectionEnd}

    ${WinampSection} "decoderM4v" $(secM4V) IDX_SEC_M4V_DEC
	  ${SECTIONIN_STD}
	  SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_mp4.dll
      SetOutPath $INSTDIR
      Delete $INSTDIR\Plugins\libmp4v2.dll
      File ${FILES_PATH}\libmp4v2.dll
      SetOutPath $INSTDIR\System
      
      ; MP4 video codecs
      File ${FILES_PATH}\system\h264.w5s
      File /nonfatal ${FILES_PATH}\System\h264.wbm
      File ${FILES_PATH}\system\mp4v.w5s
      File /nonfatal ${FILES_PATH}\System\mp4v.wbm
      
      ; MP4 audio codecs
	  ; aacPlusDecoder.w5s is installed by default (see winamp.nsh)
	  ; File ${FILES_PATH}\System\aacPlusDecoder.w5s
	  ; File /nonfatal "${FILES_PATH}\system\aacPlusDecoder.wbm"
	  File ${FILES_PATH}\System\pcm.w5s
	  File /nonfatal ${FILES_PATH}\System\pcm.wbm
      
      ; beta only stuff
      !ifdef BETA | NIGHT
        SetOutPath $INSTDIR\System
        File ${FILES_PATH}\System\alac.w5s
        File /nonfatal ${FILES_PATH}\System\alac.wbm
		File ${FILES_PATH}\System\a52.w5s
	    File /nonfatal ${FILES_PATH}\System\a52.wbm
      !endif ;  BETA | NIGHT
    ${WinampSectionEnd}
    
    !ifndef WINAMP64
    ${WinampSection} "decoderSwf" $(secSWF) IDX_SEC_SWF_DEC
      ${SECTIONIN_FULL}
      SetOutPath $INSTDIR\Plugins
      File ${FILES_PATH}\plugins\in_swf.dll
      File ..\..\resources\data\winampFLV.swf
    ${WinampSectionEnd}
    !endif ; WINAMP64
    
 SectionGroupEnd ; Video Playback
!endif ; std | full