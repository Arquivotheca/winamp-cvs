!include "MUI2.nsh"

;------------------------

SetCompressor /SOLID /FINAL lzma
SetCompressorDictSize 16
FileBufSize 64

RequestExecutionLevel admin
!define PLUGIN_NAME "Winamp 5 Cloud Client Plug-in"
!define PLUGIN_BUILD "Build 34"
Name "${PLUGIN_NAME} (${PLUGIN_BUILD})"
OutFile "winamp-cloud.exe"

InstallDir $PROGRAMFILES\Winamp
InstProgressFlags smooth
XPStyle on
ShowInstDetails hide
AutoCloseWindow true


LangString IDS_PAGE_FINISH_TEXT ${LANG_ENGLISH} "Close Winamp and Re-open to use Plugin.  Go to Plugins -> Media Library -> Winamp Cloud in Winamp Preferences to login."
LangString IDS_PAGE_FINISH_RUN ${LANG_ENGLISH} "Launch Winamp after the installer closes"

!define MUI_ICON ".\resources\install.ico"
!define MUI_UNICON ".\resources\uninstall.ico"
!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP ".\resources\header.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP ".\resources\welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\resources\welcome.bmp"

!insertmacro MUI_PAGE_WELCOME
;!insertmacro MUI_PAGE_LICENSE ".\resources\license.rtf"

; detect Winamp path from uninstall string if available
InstallDirRegKey HKLM \
          "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
          "UninstallString"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TEXT             $(IDS_PAGE_FINISH_TEXT)
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT         $(IDS_PAGE_FINISH_RUN)
!define MUI_FINISHPAGE_RUN_FUNCTION     FinishPage_Run
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

!include "FileFunc.nsh"

Function .onInit
  ;Detect running Winamp instances and close them
  !define WINAMP_FILE_EXIT 40001

  FindWindow $R0 "Winamp v1.x"
  IntCmp $R0 0 ok
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "Please close all instances of Winamp before installing$\n\
               ${PLUGIN_NAME}. Attempt to close Winamp now?" IDYES checkagain IDNO no
    checkagain:
      FindWindow $R0 "Winamp v1.x"
      IntCmp $R0 0 ok
      SendMessage $R0 ${WM_COMMAND} ${WINAMP_FILE_EXIT} 0
      Goto checkagain
    no:
       Abort
  ok:
FunctionEnd

Function FinishPage_Run

  HideWindow
  Exec "$INSTDIR\winamp.exe"
  Sleep 500

FunctionEnd

Section ""

  SetOutPath "$INSTDIR"
  File "C:\Program Files (x86)\Winamp\nxlite.dll"
  File "C:\Program Files (x86)\Winamp\nde.dll"
  File "C:\Program Files (x86)\Winamp\jnetlib.dll"

  SetOutPath "$INSTDIR\plugins"
  File "C:\Program Files (x86)\Winamp\plugins\ml_local.dll"
  File "C:\Program Files (x86)\Winamp\plugins\ml_devices.dll"
  File "C:\Program Files (x86)\Winamp\plugins\ml_cloud.dll"
  File "C:\Program Files (x86)\Winamp\plugins\pmp_cloud.dll"
  File "C:\Program Files (x86)\Winamp\plugins\in_flac.dll"

  SetOutPath "$INSTDIR\system"
  File "C:\Program Files (x86)\Winamp\system\jnetlib.w5s"
  File "C:\Program Files (x86)\Winamp\system\wasabi2.w5s"

  SetOutPath "$INSTDIR\components"
  File "C:\Program Files (x86)\Winamp\components\cloud.w6c"

SectionEnd