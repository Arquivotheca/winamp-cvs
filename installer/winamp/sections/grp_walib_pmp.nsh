SectionGroup $(IDS_GRP_WALIB_PORTABLE) IDX_GRP_WALIB_PORTABLE  ;  Portable Media Player Support

  ${WinampSection} "mediaLibraryPortable" $(secPmp) IDX_SEC_ML_PMP                                    ; >>> [pmp main]
    ${SECTIONIN_STD}
    SectionGetFlags ${IDX_GRP_WALIB_PORTABLE} $1
    IntOp $1 $1 & 0x0041
    StrCmp $1 "0" done
    SetOutPath $INSTDIR\Plugins
    File ${FILES_PATH}\plugins\ml_pmp.dll
    File ${FILES_PATH}\plugins\ml_devices.dll
    SetOutPath $INSTDIR\System
    File ${FILES_PATH}\system\devices.w5s
    SetOutPath $INSTDIR
    File ${FILES_PATH}\nde.dll
    File ${FILES_PATH}\nxlite.dll
    File ${FILES_PATH}\jnetlib.dll
   done:
  ${WinampSectionEnd}                                                           ; <<< [pmp main]

   ${WinampSection} "portableDeviceIPod" $(secPmpiPod) IDX_SEC_PMP_IPOD                            ; >>> [iPod support]
     ${SECTIONIN_STD}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_ipod.dll
     Delete $INSTDIR\Plugins\ml_ipod.dll
   ${WinampSectionEnd}                                                          ; <<< [iPod support]

  !ifdef full
   ${WinampSection} "portableDeviceCreative" $(secPmpCreative) IDX_SEC_PMP_CREATIVE                    ; >>> [Creative]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_njb.dll
   ${WinampSectionEnd}                                                          ; <<< [Creative]
   !endif

  !ifndef WINAMP64
  !ifdef full
   ${WinampSection} "portableDeviceP4S" $(secPmpP4S) IDX_SEC_PMP_P4S                              ; >>> [PlayForSure]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_p4s.dll
     ${If} $WinVer == "XP"
     ${OrIf} $WinVer == "2003"
     ${OrIf} $WinVer == "VISTA"
     ${OrIf} $WinVer == "7"
     ${OrIf} $WinVer == "8"
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\MTPMediaPlayerArrival" "${WINAMP}MTPHandler" ""
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "Action" "Open with ${WINAMP}"
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "DefaultIcon" "$INSTDIR\${WINAMPEXE},0"
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "InitCmdLine" "$INSTDIR\${WINAMPEXE}"
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "ProgID" "Shell.HWEventHandlerShellExecute"
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "Provider" "${WINAMP}"
   ; ToDo (required on Win7/8)
   ; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "CLSIDForCancel" "{Insert.Winamp.AppID_GUID}"
   ; WriteRegStr HKEY_CLASSES_ROOT "AppID\{Insert.Winamp.AppID_GUID}" "" "${WINAMP}"
   ; WriteRegStr HKEY_CLASSES_ROOT "AppID\{Insert.Winamp.AppID_GUID}" "RunAs" "Interactive User"
   ; If no "AccessPermission" & "LaunchPermission" RegStr (REG_BINARY) values exist, then Windows will use default values instead ~ Ref: http://msdn.microsoft.com/en-gb/library/windows/desktop/ms688679%28v%3Dvs.85%29.aspx
     ${EndIf}
   ${WinampSectionEnd}                                                      ; <<< [PlayForSure]
   !endif
  !endif ; WINAMP64

  !ifdef full
   ${WinampSection} "portableDeviceUsb" $(secPmpUSB) IDX_SEC_PMP_USB                         ; >>> [USB]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_usb.dll
   ${WinampSectionEnd}                                                           ; <<< [USB]
  !endif
  
  !ifdef full
   ${WinampSection} "portableDeviceAndroid" $(secPmpAndroid) IDX_SEC_PMP_ANDROID              ; >>> [Android]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_android.dll
   ${WinampSectionEnd}                                                           ; <<< [Android]
  !endif

   ${WinampSection} "portableDeviceWifi" $(secPmpWifi) IDX_SEC_PMP_WIFI                            ; >>> [Wifi support]
     ${SECTIONIN_STD}
	 SetOutPath $INSTDIR
     File ${FILES_PATH}\jnetlib.dll
	 SetOutPath $INSTDIR\System
     File ${FILES_PATH}\System\wasabi2.w5s
	 SetOutPath $INSTDIR\Components
	 File ${FILES_PATH}\Components\ssdp.w6c
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_wifi.dll
	 ${If} ${FileExists} "$SETTINGSDIR\pmp_wifi.ini"
	 Rename "$SETTINGSDIR\pmp_wifi.ini" "$SETTINGSDIR\Plugins\ml\pmp_wifi.ini"
	 ${EndIf}
   ${WinampSectionEnd}                                                          ; <<< [Wifi support]

  !ifndef WINAMP64
  !ifdef full
   ${WinampSection} "portableDeviceActiveSync" $(secPmpActiveSync) IDX_SEC_PMP_ACTIVESYNC                ; >>> [ActiveSync]
     ${SECTIONIN_FULL}
     SetOutPath $INSTDIR\Plugins
     File ${FILES_PATH}\plugins\pmp_activesync.dll
   ${WinampSectionEnd}                                                ; <<< [ActiveSync]
   !endif
  !endif ; WINAMP64
  
SectionGroupEnd