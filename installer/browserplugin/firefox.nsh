!ifndef WACHK_FIREFOX
	!define WACHK_FIREFOX  "FireFox"
!endif

!ifndef WACHK_SECTION_${WACHK_FIREFOX}_HEADER
!define WACHK_SECTION_${WACHK_FIREFOX}_HEADER

	!include ".\config.nsh"
	!include ".\variables.nsh"
	!include ".\functions.nsh"

	Var wachk.firefox.version
	Var wachk.firefox.path
	Var wachk.firefox.plugins.path
	Var wachk.firefox.components.path
	!define WACHK_${WACHK_FIREFOX}_PROCESSNAME			"firefox.exe"
	!define WACHK_${WACHK_FIREFOX}_PLUGIN				"npwachk.dll"
	!define WACHK_${WACHK_FIREFOX}_PLUGIN_PATH			"$wachk.firefox.plugins.path"
	!define WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH		"${WACHK_${WACHK_FIREFOX}_PLUGIN_PATH}\${WACHK_${WACHK_FIREFOX}_PLUGIN}"
	!define WACHK_${WACHK_FIREFOX}_PLUGIN_SOURCE		"${WACHK_SRC_PATH}\winamp\browserplugins\mozilla\plugins\${WACHK_${WACHK_FIREFOX}_PLUGIN}"

	!define WACHK_${WACHK_FIREFOX}_COMPONENT			"npwachk.xpt"
	!define WACHK_${WACHK_FIREFOX}_COMPONENT_PATH		"$wachk.firefox.components.path"
	!define WACHK_${WACHK_FIREFOX}_COMPONENT_FULLPATH	"${WACHK_${WACHK_FIREFOX}_COMPONENT_PATH}\${WACHK_${WACHK_FIREFOX}_COMPONENT}"
	;!define WACHK_${WACHK_FIREFOX}_COMPONENT_SOURCE		"${WACHK_SRC_PATH}\winamp\browserplugins\mozilla\components\${WACHK_${WACHK_FIREFOX}_COMPONENT}"
	
	!define WACHK_DETECT_${WACHK_FIREFOX}			"!insertmacro 'WACHK_DETECT_${WACHK_FIREFOX}'"
	!define WACHK_DETECT_ON_ROOT_${WACHK_FIREFOX}	"!insertmacro 'WACHK_DETECT_ON_ROOT_${WACHK_FIREFOX}'"
	!define WACHK_INSTALLER_${WACHK_FIREFOX}_INIT	"!insertmacro 'WACHK_INSTALLER_${WACHK_FIREFOX}_INIT'"
	!define WACHK_INSTALLER_${WACHK_FIREFOX}_PRE	"!insertmacro 'WACHK_INSTALLER_${WACHK_FIREFOX}_PRE'"
	!define WACHK_INSTALLER_${WACHK_FIREFOX}_LEAVE	"!insertmacro 'WACHK_INSTALLER_${WACHK_FIREFOX}_LEAVE'"
	!define WACHK_INSTALLER_${WACHK_FIREFOX}_DESC	"!insertmacro 'WACHK_INSTALLER_${WACHK_FIREFOX}_DESC'"
	!define WACHK_UNINSTALLER_${WACHK_FIREFOX}_INIT	"!insertmacro 'WACHK_UNINSTALLER_${WACHK_FIREFOX}_INIT'"
	!define WACHK_UNINSTALLER_${WACHK_FIREFOX}_PRE	"!insertmacro 'WACHK_UNINSTALLER_${WACHK_FIREFOX}_PRE'"
	!define WACHK_UNINSTALLER_${WACHK_FIREFOX}_GET_REMOVED "!insertmacro 'WACHK_UNINSTALLER_${WACHK_FIREFOX}_GET_REMOVED'"
	
	!macro WACHK_DETECT_ON_ROOT_${WACHK_FIREFOX} __registryRoot
		StrCpy $wachk.firefox.version 			""
		StrCpy $wachk.firefox.path 				""
		StrCpy $wachk.firefox.plugins.path 		""
		StrCpy $wachk.firefox.components.path 	""
		ReadRegStr $0 ${__registryRoot} "Software\Mozilla\Mozilla Firefox" "CurrentVersion"
		${If} $0 != ""
			ReadRegStr $1 ${__registryRoot} "Software\Mozilla\Mozilla Firefox\$0\Main" "Install Directory"
			${If} $1 != ""
				${If} ${FileExists} "$1\firefox.exe"
					${WordFind} $0 " (" "+1" $wachk.firefox.version
					StrCpy $wachk.firefox.path $1
					ReadRegStr $1 ${__registryRoot} "Software\Mozilla\Mozilla Firefox $wachk.firefox.version\extensions" "Plugins"
					${If} $1 != ""
						StrCpy $wachk.firefox.plugins.path $1
					${Else}
						StrCpy $wachk.firefox.plugins.path "$wachk.firefox.path\plugins"
					${EndIf}
					ReadRegStr $1 ${__registryRoot} "Software\Mozilla\Mozilla Firefox $wachk.firefox.version\extensions" "Components"
					${If} $1 != ""
						StrCpy $wachk.firefox.components.path $1
					${Else}
						StrCpy $wachk.firefox.components.path "$wachk.firefox.path\components"
					${EndIf}
				${EndIf}
			${EndIf}
		${EndIf}
	!macroend
	
	!macro WACHK_DETECT_${WACHK_FIREFOX}
		${WACHK_DETECT_ON_ROOT_${WACHK_FIREFOX}} HKCU
		${If} $wachk.firefox.version == ""
			${WACHK_DETECT_ON_ROOT_${WACHK_FIREFOX}} HKLM
		${EndIf}
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_FIREFOX}_INIT
		${WACHK_SECTION_SIZE_BEGIN} ${IDX_SECTION_${WACHK_FIREFOX}}
		${WACHK_SECTION_SIZE_ADD_FILE}  '${WACHK_${WACHK_FIREFOX}_PLUGIN_SOURCE}'
		;${WACHK_SECTION_SIZE_ADD_FILE}  '${WACHK_${WACHK_FIREFOX}_COMPONENT_SOURCE}'
		${WACHK_SECTION_SIZE_END}
		
		${WACHK_DETECT_${WACHK_FIREFOX}}
		${WACHK_SECTION_REMOVE} ${IDX_SECTION_${WACHK_FIREFOX}} $wachk.firefox.version "" ""
	!macroend

	!macro WACHK_INSTALLER_${WACHK_FIREFOX}_PRE
		${WACHK_SECTION_UPDATESELECT} ${IDX_SECTION_${WACHK_FIREFOX}} "${WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH}" "${WACHK_${WACHK_FIREFOX}_PLUGIN_SOURCE}"
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_FIREFOX}_LEAVE __incrementVar
	
		${If} ${SectionIsSelected} ${IDX_SECTION_${WACHK_FIREFOX}} 
			IntOp '${__incrementVar}' '${__incrementVar}' + 1
		${EndIf}
	!macroend
	
	!macro WACHK_INSTALLER_${WACHK_FIREFOX}_DESC
		${DESCRIPTION} IDX_SECTION_${WACHK_FIREFOX}	$(IDS_SECTION_${WACHK_FIREFOX}_DESC)
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_FIREFOX}_INIT
		${WACHK_DETECT_${WACHK_FIREFOX}}
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_FIREFOX}_GET_REMOVED	__sectionRemoved
		${IfNot} ${FileExists} "${WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH}"
		${AndIfNot} ${FileExists} "${WACHK_${WACHK_FIREFOX}_COMPONENT_FULLPATH}"
			StrCpy	${__sectionRemoved}	"true"
		${Else}
			StrCpy	${__sectionRemoved}	"false"
		${EndIf}
	!macroend
	
	!macro WACHK_UNINSTALLER_${WACHK_FIREFOX}_PRE
		Push $0
		${WACHK_UNINSTALLER_${WACHK_FIREFOX}_GET_REMOVED} $0
		${If} $0 == "true"
			${WACHK_SECTION_HIDE} ${IDX_SECTION_UN_${WACHK_FIREFOX}}
		${EndIf}
		Pop $0
	!macroend
	
!endif
	


!ifdef WACHK_INSERT_INSTALLER_SECTIONS

${WACHK_SECTION_INSTALL} "${WACHK_FIREFOX}"
	${WACHK_INSTALL_BROWSER_PLUGIN} "$(IDS_SECTION_${WACHK_FIREFOX})" $wachk.firefox.version \
			'${WACHK_${WACHK_FIREFOX}_PLUGIN_SOURCE}' '${WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH}'\
			WinampCheck_${WACHK_FIREFOX}InstallCallback "${WACHK_${WACHK_FIREFOX}_PROCESSNAME}" "${IDX_SECTION_${WACHK_FIREFOX}}"
	
${WACHK_SECTION_END}

Function WinampCheck_FirefoxInstallCallback
	Push $0
	StrCpy $0 "success"

	SetOutPath "${WACHK_${WACHK_FIREFOX}_PLUGIN_PATH}"
	ClearErrors
	File "${WACHK_${WACHK_FIREFOX}_PLUGIN_SOURCE}"
	${If} ${Errors}
		StrCpy $0 "$(IDS_ERROR_UNABLE_TO_WRITE) '${WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH}'."
		goto wachk_install_error
	${EndIf}
	
	Delete /REBOOTOK "${WACHK_${WACHK_FIREFOX}_COMPONENT_FULLPATH}"
		
wachk_install_error:
	Exch $0
FunctionEnd
!endif

!ifdef WACHK_INSERT_UNINSTALLER_SECTIONS
Section 'un.$(IDS_SECTION_${WACHK_FIREFOX})' 'IDX_SECTION_UN_${WACHK_FIREFOX}'
	StrCpy $0 "$(IDS_SECTION_${WACHK_FIREFOX})"
	DetailPrint "$(IDS_DETAILS_REMOVE_PLUGIN)..."
	SetDetailsPrint none
	
	Delete "${WACHK_${WACHK_FIREFOX}_PLUGIN_FULLPATH}"
	Delete "${WACHK_${WACHK_FIREFOX}_COMPONENT_FULLPATH}"
	${WACHK_PROCESS_RESTART_REQUIRED} "${WACHK_${WACHK_FIREFOX}_PROCESSNAME}" "$(IDS_SECTION_${WACHK_FIREFOX})"
	
	SetDetailsPrint lastused
SectionEnd
!endif


