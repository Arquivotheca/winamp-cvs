!include ".\config.nsh"
!include ".\variables.nsh"

!define WACHK_PLUGIN_OVERWRITE_NEWER

!ifdef WACHK_DEBUG_MODE
	SetCompress off
	ShowInstDetails show
	ShowUninstDetails show
	AutoCloseWindow false
!else
	SetCompressor /FINAL /SOLID lzma
	SetDatablockOptimize on
	FileBufSize 64
	ShowInstDetails nevershow
	ShowUninstDetails nevershow
	AutoCloseWindow true
!endif

RequestExecutionLevel admin
InstProgressFlags smooth
XPStyle on

Name "$(IDS_INSTALLER_NAME)"
OutFile "${WACHK_INSTALLER_FILENAME}"
InstallDir "${WACHK_INSTALLER_DEFPATH}"

!define WACHK_INSERT_SECTIONS
!include ".\sectionsInstall.nsh"
!include ".\sectionsUninstall.nsh"
!undef WACHK_INSERT_SECTIONS

!include ".\mui2.nsh"

!include ".\pagesInstall.nsh"
!include ".\pagesUninstall.nsh"
!include ".\languages.nsh"
!include ".\versionInfo.nsh"

ReserveFile "${NSISDIR}\Plugins\System.dll"
ReserveFile "${NSISDIR}\Plugins\LangDLL.dll"


Function .onInit
	
	InitPluginsDir
	
	!ifdef LANGID
		WriteRegStr "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "${WACHK_LANGUAGE_REGKEY}" ${LANGID}
	!endif ; LANGID

	!insertmacro MUI_LANGDLL_DISPLAY
  
	${WACHK_INSTALLER_COMPONENTS_INIT}
FunctionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	${WACHK_UNINSTALLER_COMPONENTS_INIT}
FunctionEnd

