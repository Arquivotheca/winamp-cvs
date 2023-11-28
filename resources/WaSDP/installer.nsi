; Color Editor Installer

;--------------------------------
;Include Modern UI

!include "MUI.nsh"

;--------------------------------

; Uncomment the next line to enable auto Winamp download
; !define WINAMP_AUTOINSTALL

; The name of the installer
Name "Winamp Skin Development Pack"

; The file to write
OutFile "WaSDP_1.13.exe"

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

SetCompressor /SOLID lzma

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "modern-header.BMP"
  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

;--------------------------------

Function .onInit
        # the plugins dir is automatically deleted when the installer exits
        InitPluginsDir
        File /oname=$PLUGINSDIR\splash.bmp "splash.BMP"
        advsplash::show 1000 600 400 0x04025C $PLUGINSDIR\splash
        Pop $0 

        Delete $PLUGINSDIR\splash.bmp
FunctionEnd

;--------------------------------

;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install

Section ""

!ifdef WINAMP_AUTOINSTALL
  Call MakeSureIGotWinamp
!endif
  
SectionEnd

Section "Maki Compiler" SecCompiler

  DetailPrint "Installing Maki Compiler..."
  SetOutPath "$INSTDIR\"
  File "..\..\Wasabi\mc.exe"

SectionEnd

Section "Maki Standard Libraries" SecLibs

  DetailPrint "Installing Maki Standard Libraries..."
  SetOutPath "$INSTDIR\lib"
  File /x "private.mi" "..\..\Wasabi\lib\*.m*"

SectionEnd

Section "Maki Community Scripts" SecLibsCom

  DetailPrint "Installing Maki Community Scripts..."
  SetOutPath "$INSTDIR\lib\com"
  File /x "private.mi" "..\..\Wasabi\lib\com\*.m*"

SectionEnd

Section "Winamp Bento Source" SecSkinBento

  DetailPrint "Installing Winamp Bento Source Code..."
  SetOutPath "$INSTDIR\Skins\Big Bento"
  File /r /x "about.m" /x "nibbles.m" "..\skins\Big Bento\*.m"
  SetOutPath "$INSTDIR\Skins\Bento"
  File /r "..\skins\Bento\*.m"

SectionEnd

Section "Winamp Modern Source" SecSkinModern

  DetailPrint "Installing Winamp Modern Source Code..."
  SetOutPath "$INSTDIR\Skins\Winamp Modern"
  File /r "..\skins\Winamp Modern\*.m"

SectionEnd

Section "Wasabi Debugger" SecDebugger

  DetailPrint "Installing Wasabi Debugger..."
  SetOutPath "$INSTDIR\system"
  File "ConsoleFile.w5s"

SectionEnd

Section "Edit Plus Syntax Libs" SecEditplus

  DetailPrint "Installing Edit Plus Syntax..."
  SetOutPath "$INSTDIR"
  File "Maki.*"

SectionEnd

Section ""

  SetOutPath "$INSTDIR"
  File "wasdp_readme.txt"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall_WaSDP.exe"

  ExecShell "open" "$INSTDIR\wasdp_readme.txt"
 
SectionEnd
;--------------------------------

;Descriptions

  ;Language strings

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCompiler} "This tool (mc.exe) is needed to compile *.m files to *.maki files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLibs} "Standard Maki Libraries."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLibsCom} "Some maki scripts done by the Winamp community."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinModern} "Install Winamp Modern Skin Maki source code."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinBento} "Install Winamp Bento Maki source code."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDebugger} "Wasabi Debugger will print debug strings to c:\wasabi.log"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEditplus} "This will install Edit Plus Syntax Libs. For more info see readme.txt"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\mc.exe"
  Delete "$INSTDIR\Maki.*"
  Delete "$INSTDIR\wasdp_readme.txt"
  Delete "$INSTDIR\system\ConsoleFile.w5s"
  Delete "$INSTDIR\Skins\Winamp Modern\scripts\*.m"
  Delete "$INSTDIR\Skins\Big Bento\scripts\*.m"
  Delete "$INSTDIR\Skins\Bento\scripts\*.m"
  Delete "$INSTDIR\Skins\Big Bento\about\*.m"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR\Skins\Bento\scripts\mcvcore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\mcvcore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\lib"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\suicore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\attribs"
  Delete "$INSTDIR\Uninstall_WaSDP.exe"

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