!include "config.nsh"
!include "fileFunc.nsh"
!include "wordFunc.nsh"
!include "logicLib.nsh"

SetCompressor /SOLID zlib
SetCompress force

SetDatablockOptimize on
AutoCloseWindow true
ShowInstDetails nevershow
RequestExecutionLevel user
InstProgressFlags smooth
;XPStyle on

ChangeUI all "${EMUSIC_QUALIFIER_UI_PATH}"

OutFile "${EMUSIC_QUALIFIER_OUTPUT_FILE}"
Name ""

Function .onInit
	
${If} ${FileExists} "$INSTDIR\eMusicClient.exe"
${OrIf} ${FileExists} "$PROGRAMFILES\eMusic Download Manager"
	SetErrorLevel 3
${Else}
	SetErrorLevel 0
${EndIf}
Quit
FunctionEnd

Section -Main
SectionEnd