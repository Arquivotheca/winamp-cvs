; Color Editor Installer

;Winamp Minimum version (Koopa: define the minimal required Winamp version here)
!define Minimal_Version "5.5.7.2555"

!define PRODUCT_NAME "Winamp 5 Color Editor"
!define PRODUCT_VERSION "v2.2.1"
!define PRODUCT_DESCRIPTION "${PRODUCT_NAME} ${PRODUCT_VERSION}"
!define PRODUCT_WEB_SITE "http://forums.winamp.com"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define /date MyTIMESTAMP "%d.%m.%Y at %H:%M:%S"

; detect winamp path from uninstall string if available
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

; MUI 2.0 compatible (Koopa: using MUI2, instead of the old 1.x version)
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "WordFunc.nsh"

;Interface Configuration
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "instheader.bmp"
  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

;Set compression (Koopa: added lzma based installer compression)
SetCompress force
SetCompressor /solid lzma

;Set styling (Koopa: activated XP+ theme support and added ability to show version on bottom of installer)
XPStyle on
BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION} built on ${MyTIMESTAMP}"

!define MUI_CUSTOMFUNCTION_GUIINIT My_GUIInit

;Version information for Windows Explorer
VIProductVersion "2.2.1.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
VIAddVersionKey "Comments" "Create your own Winamp color themes!"
VIAddVersionKey "LegalCopyright" "Copyright © 2004-2009, Nullsoft Inc."
VIAddVersionKey "Company" "Nullsoft, Inc."
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} ${PRODUCT_VERSION} Installer"
VIAddVersionKey "FileVersion" "2.2.1.0"

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"

RequestExecutionLevel admin
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "wa5_coloreditor_2_2_1.exe"
ShowInstDetails show
ShowUnInstDetails show
 
;Pages
;License page
!define MUI_LICENSEPAGE_CHECKBUTTON
!insertmacro MUI_PAGE_LICENSE "files\license.txt"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

;Uninstaller pages
!define MUI_UNINSTALLER

;Languages 
  !insertmacro MUI_LANGUAGE "English"

;The stuff to install
;Koopa: completely rewrote installer section to get rid of the stupid installation requirement
;forcing an installed ColorEditor to create the installer wasn't ideal

Section "Install"

 SetOutPath "$INSTDIR\Plugins\freeform\wacs\ColorEditor"
 SetOverwrite on
 File "files\changelog.txt"
 File "files\license.txt"
 File "files\coloreditor.wac"
 SetOutPath "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml"
 SetOverwrite on
 File "..\xml\about.png"
 File "..\xml\about.maki"
 File "..\xml\ce_extend.maki"
 File "..\xml\coloreditor.maki"
 File "..\xml\coloreditor.xml"
 File "..\xml\lock.png"
 SetOutPath "$INSTDIR\Plugins\freeform\wacs\ColorEditor\data"
 SetOverwrite on
 File "..\data\readme.txt"

 SetAutoClose false

SectionEnd

Function .onInstSuccess
  MessageBox MB_YESNO \
             '${PRODUCT_NAME} was installed. Do you want to run Winamp now?' \
	 IDNO end
    ExecShell open "$INSTDIR\Winamp.exe"
  end:
FunctionEnd

; Koopa added Section post for better overview and additional registry entries

Section -Post
 WriteUninstaller "$INSTDIR\Uninstall_ColorEditor.exe"
 WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
 WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall_ColorEditor.exe"
 WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
 WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "HelpLink" "http://forums.winamp.com/forumdisplay.php?forumid=8"
 WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\winamp.exe,0"
 SetAutoClose true
SectionEnd

; Uninstaller
; Koopa: Completely rewrote the uninstaller, *.* really isn't ideal ;)

Section "Uninstall"

 ;Delete files

 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\license.txt"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\coloreditor.wac"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\changelog.txt"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\about.png"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\about.maki"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\ce_extend.maki"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\coloreditor.maki"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\coloreditor.xml"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\xml\lock.png"
 Delete "$INSTDIR\Plugins\freeform\wacs\ColorEditor\data\readme.txt"
 RMDir "$INSTDIR\Plugins\freeform\wacs\Color Editor\data"
 RMDir "$INSTDIR\Plugins\freeform\wacs\Color Editor\xml"
 RMDir "$INSTDIR\Plugins\freeform\wacs\Color Editor"

  ;Delete Uninstall info
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

  ;Delete uninstaller
  Delete /REBOOTOK "$INSTDIR\Uninstall_ColorEditor.exe"
  
  SetAutoClose false

SectionEnd

 
Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "${PRODUCT_NAME} was successfully uninstalled"
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you really want to uninstall ${PRODUCT_NAME}?" IDYES +2
  Abort
FunctionEnd

Function GetFileVersion

   !define GetFileVersion `!insertmacro GetFileVersionCall`
   !macro GetFileVersionCall _FILE _RESULT
      Push `${_FILE}`
      Call GetFileVersion
      Pop ${_RESULT}
   !macroend

   Exch $0
   Push $1
   Push $2
   Push $3
   Push $4
   Push $5
   Push $6
   ClearErrors

   GetDllVersion '$0' $1 $2
   IfErrors error
       IntOp $3 $1 / 0x00010000
       IntOp $4 $1 & 0x0000FFFF
       IntOp $5 $2 / 0x00010000
       IntOp $6 $2 & 0x0000FFFF
       StrCpy $0 '$3.$4.$5.$6'
   goto end

   error:
      SetErrors
      StrCpy $0 ''

   end:
   Pop $6
   Pop $5
   Pop $4
   Pop $3
   Pop $2
   Pop $1
   Exch $0

FunctionEnd

Function CheckWinampVersion

	${GetFileVersion} "$INSTDIR\winamp.exe" $R0 ; Get Winamp.exe version information, $R0 = Actual Version
		
	${if} $R0 != "" ; check if Version info is not empty
		${VersionCompare} $R0 ${Minimal_Version} $R1 ; $R1 = Result $R1=0  Versions are equal, $R1=1  Version1 is newer, $R1=2  Version2 is newer
		${if} $R1 = "2"
			MessageBox MB_OK "This version of the ${PRODUCT_NAME} requires at least Winamp ${Minimal_Version} or above.$\r$\nPlease update your Winamp version before you can install this component.$\r$\nInstallation will be aborted."
			Quit
		${EndIf}		
	${Else}
		MessageBox MB_OK "Winamp wasn't detected on this system.$\r$\nPlease install latest Winamp version from Winamp.com,$\r$\nbefore you can install this component.$\r$\nInstallation will be aborted." ; version info is empty, something goes wrong. Display message and exit
		Quit
	${EndIf}
			
FunctionEnd

Function CheckWinampInstallation

	;MessageBox MB_OK "$INSTDIR\winamp.exe"

	${If} ${FileExists} "$INSTDIR\winamp.exe" ;check if Winamp.exe exists
		;
	${Else}
		MessageBox MB_OK "Winamp wasn't detected on this system.$\r$\nPlease install latest Winamp version from Winamp.com,$\r$\nbefore you can install this component.$\r$\nInstallation will be aborted." ;no winamp.exe
		Quit
	${EndIf}

FunctionEnd

Function .onInit
        # the plugins dir is automatically deleted when the installer exits
        InitPluginsDir
        File /oname=$PLUGINSDIR\splash.bmp "splash.BMP"
        advsplash::show 1000 600 400 0x04025C $PLUGINSDIR\splash
        Pop $0 

        Delete $PLUGINSDIR\splash.bmp

;Detect running Winamp instances and close them
!define WINAMP_FILE_EXIT 40001

!ifdef INTERCEPT_MULTIPLE_INSTANCES
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "WinampMbApiSetup") i .r1 ?e'
  Pop $R0

  StrCmp $R0 0 noprevinst
    ReadRegStr $R0 HKCU "${PLUGIN_INSTREGKEY}" "WindowHandle"
    System::Call 'user32::SetForegroundWindow(i $R0) i ?e'
    Abort
	
  noprevinst:
!endif		
		  
  FindWindow $R0 "Winamp v1.x"
  IntCmp $R0 0 ok
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "Please close all instances of Winamp before installing this component" IDYES checkagain IDNO no
    checkagain:
      FindWindow $R0 "Winamp v1.x"
      IntCmp $R0 0 ok
      SendMessage $R0 ${WM_COMMAND} ${WINAMP_FILE_EXIT} 0
      Goto checkagain
    no:
       Abort
  ok:

FunctionEnd

Function My_GUIInit
	Call CheckWinampInstallation
	Call CheckWinampVersion
FunctionEnd