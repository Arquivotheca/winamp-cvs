!ifndef WACHK_EXPLORER
	!define WACHK_EXPLORER  "Explorer"
!endif

!ifndef WACHK_SECTION_${WACHK_EXPLORER}_HEADER
!define WACHK_SECTION_${WACHK_EXPLORER}_HEADER
	
	!include ".\config.nsh"
	!include ".\variables.nsh"
	!include ".\functions.nsh"

	Var wachk.explorer.version
	
	!define WACHK_${WACHK_EXPLORER}_PLUGIN				"iewachk.dll"
	!define WACHK_${WACHK_EXPLORER}_PLUGIN_PATH			"$INSTDIR"
	!define WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH		"${WACHK_${WACHK_EXPLORER}_PLUGIN_PATH}\${WACHK_${WACHK_EXPLORER}_PLUGIN}"
	!define WACHK_${WACHK_EXPLORER}_PLUGIN_SOURCE		"${WACHK_SRC_PATH}\winamp\browserplugins\ie\${WACHK_${WACHK_EXPLORER}_PLUGIN}"
	!define WACHK_${WACHK_EXPLORER}_PROCESSNAME			"iexplore.exe"
	
	!define WACHK_DETECT_${WACHK_EXPLORER}				"!insertmacro 'WACHK_DETECT_${WACHK_EXPLORER}'"
	!define WACHK_INSTALLER_${WACHK_EXPLORER}_INIT		"!insertmacro 'WACHK_INSTALLER_${WACHK_EXPLORER}_INIT'"
	!define WACHK_INSTALLER_${WACHK_EXPLORER}_PRE		"!insertmacro 'WACHK_INSTALLER_${WACHK_EXPLORER}_PRE'"
	!define WACHK_INSTALLER_${WACHK_EXPLORER}_LEAVE		"!insertmacro 'WACHK_INSTALLER_${WACHK_EXPLORER}_LEAVE'"
	!define WACHK_INSTALLER_${WACHK_EXPLORER}_DESC		"!insertmacro 'WACHK_INSTALLER_${WACHK_EXPLORER}_DESC'"
	!define WACHK_UNINSTALLER_${WACHK_EXPLORER}_INIT	"!insertmacro 'WACHK_UNINSTALLER_${WACHK_EXPLORER}_INIT'"
	!define WACHK_UNINSTALLER_${WACHK_EXPLORER}_PRE		"!insertmacro 'WACHK_UNINSTALLER_${WACHK_EXPLORER}_PRE'"
	!define WACHK_UNINSTALLER_${WACHK_EXPLORER}_GET_REMOVED "!insertmacro 'WACHK_UNINSTALLER_${WACHK_EXPLORER}_GET_REMOVED'"
	
	!macro WACHK_DETECT_${WACHK_EXPLORER}
		StrCpy $wachk.explorer.version ""
		ReadRegStr $0 HKLM "Software\Microsoft\Internet Explorer" "Version"
		${If} $0 != ""
			StrCpy $wachk.explorer.version $0
		${EndIf}
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_EXPLORER}_INIT
		${WACHK_SECTION_SIZE_BEGIN} ${IDX_SECTION_${WACHK_EXPLORER}}
		${WACHK_SECTION_SIZE_ADD_FILE} '${WACHK_${WACHK_EXPLORER}_PLUGIN_SOURCE}'
		${WACHK_SECTION_SIZE_END} 
		
		${WACHK_DETECT_${WACHK_EXPLORER}}
		${WACHK_SECTION_REMOVE} ${IDX_SECTION_${WACHK_EXPLORER}} $wachk.explorer.version "" ""
	!macroend

	!macro WACHK_INSTALLER_${WACHK_EXPLORER}_PRE
		${WACHK_SECTION_UPDATESELECT} ${IDX_SECTION_${WACHK_EXPLORER}} "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}" "${WACHK_${WACHK_EXPLORER}_PLUGIN_SOURCE}"
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_EXPLORER}_LEAVE __incrementVar
		${If} ${SectionIsSelected} ${IDX_SECTION_${WACHK_EXPLORER}} 
			IntOp '${__incrementVar}' '${__incrementVar}' + 1
		${EndIf}
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_EXPLORER}_DESC
		${DESCRIPTION} IDX_SECTION_${WACHK_EXPLORER}	$(IDS_SECTION_${WACHK_EXPLORER}_DESC)
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_EXPLORER}_INIT
		${WACHK_DETECT_${WACHK_EXPLORER}}
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_EXPLORER}_GET_REMOVED	__sectionRemoved
		${IfNot} ${FileExists} "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
			StrCpy	${__sectionRemoved}	"true"
		${Else}
			StrCpy	${__sectionRemoved}	"false"
		${EndIf}
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_EXPLORER}_PRE
		Push $0
		${WACHK_UNINSTALLER_${WACHK_EXPLORER}_GET_REMOVED} $0
		${If} $0 == "true"
			${WACHK_SECTION_HIDE} ${IDX_SECTION_UN_${WACHK_EXPLORER}}
		${EndIf}
		Pop $0
	!macroend
	
!endif
	
!ifdef WACHK_INSERT_INSTALLER_SECTIONS

${WACHK_SECTION_INSTALL} "${WACHK_EXPLORER}"
	${WACHK_INSTALL_BROWSER_PLUGIN} "$(IDS_SECTION_${WACHK_EXPLORER})" $wachk.explorer.version \
			'${WACHK_${WACHK_EXPLORER}_PLUGIN_SOURCE}' '${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}'\
			WinampCheck_${WACHK_EXPLORER}InstallCallback "${WACHK_${WACHK_EXPLORER}_PROCESSNAME}" "${IDX_SECTION_${WACHK_EXPLORER}}"
			
${WACHK_SECTION_END}

Function WinampCheck_ExplorerInstallCallback
	Push $0
	StrCpy $0 "success"
	
	${If} ${FileExists} "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
		UnRegDLL "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
		Delete "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
	${EndIf}
	
	SetOutPath "${WACHK_${WACHK_EXPLORER}_PLUGIN_PATH}"
	
	ClearErrors
	File "${WACHK_${WACHK_EXPLORER}_PLUGIN_SOURCE}"
	${If} ${Errors}
		StrCpy $0 "$(IDS_ERROR_UNABLE_TO_WRITE) '${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}'."
		goto wachk_install_error
	${EndIf}

	ClearErrors
	RegDLL "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
	${If} ${Errors}
		StrCpy $0 "$(IDS_ERROR_UNABLE_TO_REGISTER) '${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}'."
		goto wachk_install_error
	${EndIf}
	
wachk_install_error:
	Exch $0
FunctionEnd
!endif

!ifdef WACHK_INSERT_UNINSTALLER_SECTIONS
Section 'un.$(IDS_SECTION_${WACHK_EXPLORER})' 'IDX_SECTION_UN_${WACHK_EXPLORER}'

	StrCpy $0 "$(IDS_SECTION_${WACHK_EXPLORER})"
	DetailPrint "$(IDS_DETAILS_REMOVE_PLUGIN)..."
	;SetDetailsPrint none
	
	UnRegDLL "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
	Delete /REBOOTOK "${WACHK_${WACHK_EXPLORER}_PLUGIN_FULLPATH}"
	${WACHK_PROCESS_RESTART_REQUIRED} "${WACHK_${WACHK_EXPLORER}_PROCESSNAME}" "$(IDS_SECTION_${WACHK_EXPLORER})"
	
	SetDetailsPrint lastused
SectionEnd
!endif
