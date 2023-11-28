; Color Editor Installer

;--------------------------------
; Define Sourcedirectory here

!define SOURCEPATH "H:\Program Files\Nullsoft\Winamp5beta\Plugins\freeform\wacs\ColorEditor\"

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

; Uncomment the next line to enable auto Winamp download
; !define WINAMP_AUTOINSTALL

; The name of the installer
Name "Winamp5 ColorEditor 2.2.0"

; The file to write
OutFile "wa5_coloreditor_2_2_0.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Winamp

; detect winamp path from uninstall string if available
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"
# currently doesn't work - DirShow hide

; automatically close the installer when done.
AutoCloseWindow false

; hide the "show details" box
ShowInstDetails show

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "${SOURCEPATH}\_installer\instheader.BMP"
  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

;--------------------------------

Function .onInit
        # the plugins dir is automatically deleted when the installer exits
        InitPluginsDir
        File /oname=$PLUGINSDIR\splash.bmp "${SOURCEPATH}\_installer\splash.BMP"
        advsplash::show 1000 600 400 0x04025C $PLUGINSDIR\splash
        Pop $0 

        Delete $PLUGINSDIR\splash.bmp
FunctionEnd

;--------------------------------

;Pages

  !insertmacro MUI_PAGE_LICENSE "${SOURCEPATH}\License.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"


;--------------------------------

;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------

; The stuff to install

Section ""

!ifdef WINAMP_AUTOINSTALL
  Call MakeSureIGotWinamp
!endif

  SetOutPath $INSTDIR\Plugins\freeform\wacs\ColorEditor
  File "${SOURCEPATH}\*.txt"
  File "${SOURCEPATH}\*.wac"

  SetOutPath $INSTDIR\Plugins\freeform\wacs\ColorEditor\xml
  File "${SOURCEPATH}\xml\*.png" 
  File "${SOURCEPATH}\xml\*.xml" 
  File "${SOURCEPATH}\xml\*.maki" 
; File "${SOURCEPATH}\xml\*.m" 

  SetOutPath $INSTDIR\Plugins\freeform\wacs\ColorEditor\data
  File "${SOURCEPATH}\data\*.txt"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall_ColorEditor.exe"
 
SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall_ColorEditor.exe"

  RMDir /r "$INSTDIR\Plugins\freeform\wacs\ColorEditor\" 

SectionEnd

;--------------------------------

Function .onVerifyInstDir

!ifndef WINAMP_AUTOINSTALL

  ;Check for Winamp installation

  IfFileExists $INSTDIR\Winamp.exe Good
    Abort
  Good:

!endif ; WINAMP_AUTOINSTALL

FunctionEnd


!ifdef WINAMP_AUTOINSTALL

Function GetWinampInstPath

  Push $0
  Push $1
  Push $2
  ReadRegStr $0 HKLM \
     "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \ 
     "UninstallString"
  StrCmp $0 "" fin

    StrCpy $1 $0 1 0 ; get firstchar
    StrCmp $1 '"' "" getparent 
      ; if first char is ", let's remove "'s first.
      StrCpy $0 $0 "" 1
      StrCpy $1 0
      rqloop:
        StrCpy $2 $0 1 $1
        StrCmp $2 '"' rqdone
        StrCmp $2 "" rqdone
        IntOp $1 $1 + 1
        Goto rqloop
      rqdone:
      StrCpy $0 $0 $1
    getparent:
    ; the uninstall string goes to an EXE, let's get the directory.
    StrCpy $1 -1
    gploop:
      StrCpy $2 $0 1 $1
      StrCmp $2 "" gpexit
      StrCmp $2 "\" gpexit
      IntOp $1 $1 - 1
      Goto gploop
    gpexit:
    StrCpy $0 $0 $1

    StrCmp $0 "" fin
    IfFileExists $0\winamp.exe fin
      StrCpy $0 ""
  fin:
  Pop $2
  Pop $1
  Exch $0
  
FunctionEnd

Function MakeSureIGotWinamp

  Call GetWinampInstPath
  
  Pop $0
  StrCmp $0 "" getwinamp
    Return
    
  getwinamp:
  
  Call ConnectInternet ;Make an internet connection (if no connection available)
  
  StrCpy $2 "$TEMP\Winamp Installer.exe"
  NSISdl::download http://download.nullsoft.com/winamp/client/winamp281_lite.exe $2
  Pop $0
  StrCmp $0 success success
    SetDetailsView show
    DetailPrint "download failed: $0"
    Abort
  success:
    ExecWait '"$2" /S'
    Delete $2
    Call GetWinampInstPath
    Pop $0
    StrCmp $0 "" skip
    StrCpy $INSTDIR $0
  skip:
  
FunctionEnd

Function ConnectInternet

  Push $R0
    
    ClearErrors
    Dialer::AttemptConnect
    IfErrors noie3
    
    Pop $R0
    StrCmp $R0 "online" connected
      MessageBox MB_OK|MB_ICONSTOP "Cannot connect to the internet."
      Quit
    
    noie3:
  
    ; IE3 not installed
    MessageBox MB_OK|MB_ICONINFORMATION "Please connect to the internet now."
    
    connected:
  
  Pop $R0
  
FunctionEnd

!endif ; WINAMP_AUTOINSTALL