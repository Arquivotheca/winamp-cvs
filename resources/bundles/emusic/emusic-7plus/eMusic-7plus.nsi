!define EMUSIC "50 FREE MP3s +1 Free Audiobook!"
!define WM_CLOSE 0x0010

SetCompressor /SOLID lzma

Name "eMusic-7plus"
OutFile "eMusic-7plus.exe"
InstallDir "$EXEDIR\eMusic"
ShowInstDetails nevershow
ShowUninstDetails nevershow
ChangeUI all "${NSISDIR}\Contrib\UIs\sdbarker_tiny.exe"



; The stuff to install
Section -HiddenMain IDX_MAIN_SECTION
SetDetailsView hide
  
  SetDetailsPrint textonly
  
  IfFileExists "$INSTDIR\eMusicClient.exe" section_main_end
  IfFileExists "$PROGRAMFILES\eMusic Download Manager" section_main_end

  SetOutPath $INSTDIR
  ; Put files there
  File "eMusicClient.exe"
  File "eMusicClient.ini"

  ; Write Uninstaller
  WriteUninstaller Uninst-eMusic-promotion.exe
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "DisplayName" "50 FREE MP3s +1 Free Audiobook!"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "UninstallString" '"$INSTDIR\Uninst-eMusic-promotion.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "DisplayIcon" "$INSTDIR\eMusicClient.exe,4"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "HelpLink" "http://www.emusic.com/help/index.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "URLInfoAbout" "http://www.emusic.com/about/index.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "Publisher" "eMusic.com Inc"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "DisplayVersion" "1.0.0.1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion" "NoRepair" 1

  SetShellVarContext current
  CreateShortCut "$DESKTOP\${EMUSIC}.lnk" "http://www.emusic.com/?fref=150680" "" "$INSTDIR\eMusicClient.exe" 4
  CreateShortCut "$SMPROGRAMS\${EMUSIC}.lnk" "http://www.emusic.com/?fref=150681" "" "$INSTDIR\eMusicClient.exe" 4

  ; Hit tracking URL
  NSISdl::download_quiet /TIMEOUT=3000 "http://winamp.com/eMusic-7plus-install-v6" "tmp.tmp"

  ; Start Sys-tray executable
  Exec $INSTDIR\eMusicClient.exe

section_main_end:
SectionEnd 


;--------------------------------
; UnInstall Section
;--------------------------------

Section -un.HiddenMain IDX_UNINSTALL_MAIN_SECTION

  SetDetailsPrint textonly
  FindWindow $0 "EMUSICTASKBARAPP" "EMUSICTASKBARAPP"
  IntCmp $0 0 +2
    SendMessage $0 ${WM_CLOSE} 0 0
    
  NSISdl::download_quiet /TIMEOUT=3000 "http://winamp.com/eMusic-7plus-uninstall-v6" "tmp.tmp"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eMusic Promotion"

  SetShellVarContext all
  Delete "$SMPROGRAMS\${EMUSIC}.lnk"
  Delete "$DESKTOP\${EMUSIC}.lnk"
  
  SetShellVarContext current
  Delete "$SMPROGRAMS\${EMUSIC}.lnk"
  Delete "$DESKTOP\${EMUSIC}.lnk"

  Delete $INSTDIR\Uninst-eMusic-promotion.exe 
  RMDir /r $INSTDIR
  MessageBox MB_OK "eMusic was uninstalled successfully." /SD IDOK
SectionEnd

