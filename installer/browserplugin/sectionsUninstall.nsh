!ifndef WACHK_SECTIONS_UNINSTALLER_HEADER
!define WACHK_SECTIONS_UNINSTALLER_HEADER

!include ".\functions.nsh"
!include ".\firefox.nsh"
!include ".\explorer.nsh"

!define WACHK_UNINSTALLER_COMPONENTS_INIT	"!insertmacro 'WACHK_UNINSTALLER_COMPONENTS_INIT'"
!define WACHK_UNINSTALLER_COMPONENTS_PRE	"!insertmacro 'WACHK_UNINSTALLER_COMPONENTS_PRE'"
!define WACHK_UNINSTALLER_IS_COMPLETE	"!insertmacro 'WACHK_UNINSTALLER_IS_COMPLETE'"


!macro WACHK_UNINSTALLER_COMPONENTS_INIT
	${WACHK_INIT}
	${WACHK_UNINSTALLER_FIREFOX_INIT}
	${WACHK_UNINSTALLER_EXPLORER_INIT}
!macroend

!macro WACHK_UNINSTALLER_COMPONENTS_PRE
	${WACHK_UNINSTALLER_FIREFOX_PRE}
	${WACHK_UNINSTALLER_EXPLORER_PRE}
!macroend

!macro WACHK_UNINSTALLER_IS_COMPLETE __completeUnistall
	${WACHK_UNINSTALLER_${WACHK_FIREFOX}_GET_REMOVED}	${__completeUnistall}
	${If} ${__completeUnistall} == "true"
		${WACHK_UNINSTALLER_${WACHK_EXPLORER}_GET_REMOVED}	${__completeUnistall}
	${EndIf}
!macroend

!endif

!ifdef WACHK_INSERT_SECTIONS

!define WACHK_INSERT_UNINSTALLER_SECTIONS
!include ".\firefox.nsh"
!include ".\explorer.nsh"
!undef WACHK_INSERT_UNINSTALLER_SECTIONS

Section -un.ClearInstaller
	
	${WACHK_UNINSTALLER_IS_COMPLETE} $0
	${If} $0 == "true"
		DetailPrint "$(IDS_DETAILS_REMOVE_UNINSTALLER)..."
		SetDetailsPrint none
		
		Delete "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
		RMDir /REBOOTOK "$INSTDIR"

		${WACHK_GET_STARTMENU_PATH} $0
		${If} $0 != ""
			RMDir /r /REBOOTOK "$0"
			DeleteRegKey "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}"
		${EndIf}
		
		SetDetailsPrint lastused 
	${Else}	
		DetailPrint "$(IDS_DETAILS_PLUGINS_STILL_PRESENT)"
		DetailPrint "$(IDS_DETAILS_SKIP_CLEAR_UNINSTALLER)"
	${EndIf}
	
SectionEnd

Section -un.Last
	!ifdef WACHK_DEBUG_MODE
		SetAutoClose false
	!endif 
SectionEnd
!endif

