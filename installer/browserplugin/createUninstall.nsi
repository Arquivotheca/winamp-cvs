!define WACHK_DEBUG_MODE
!define WACHK_UNINSTALLER_CODE_ONLY
!include ".\config.nsh"
!include ".\variables.nsh"




OutFile "createUninstall.exe"
Name "$(IDS_INSTALLER_NAME)"

SilentInstall silent
RequestExecutionLevel admin

Section
	WriteUninstaller "$EXEDIR\${WACHK_UNINSTALLER_FILENAME}"
SectionEnd

!define WACHK_INSERT_SECTIONS
!include ".\sectionsUninstall.nsh"
!undef WACHK_INSERT_SECTIONS

!include ".\mui2.nsh"
!include ".\pagesUninstall.nsh"
!include ".\languages.nsh"
!include ".\versionInfo.nsh"

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	${WACHK_UNINSTALLER_COMPONENTS_INIT}
FunctionEnd
