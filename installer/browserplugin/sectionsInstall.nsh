﻿!ifndef WACHK_SECTIONS_INSTALLER_HEADER
!define WACHK_SECTIONS_INSTALLER_HEADER

!include "FileFunc.nsh"
!include ".\functions.nsh"
!include ".\firefox.nsh"
!include ".\explorer.nsh"

!macro WACHK_INSTALLER_COMPONENTS_INIT
	${WACHK_INIT}
	${WACHK_INSTALLER_FIREFOX_INIT}
	${WACHK_INSTALLER_EXPLORER_INIT}
!macroend

!macro WACHK_INSTALLER_COMPONENTS_PRE
	${WACHK_INSTALLER_FIREFOX_PRE}
	${WACHK_INSTALLER_EXPLORER_PRE}
!macroend

!macro WACHK_INSTALLER_COMPONENTS_LEAVE
	StrCpy $0 0
	${WACHK_INSTALLER_FIREFOX_LEAVE} $0
	${WACHK_INSTALLER_EXPLORER_LEAVE} $0
	${If} $0 == 0
		MessageBox MB_YESNO|MB_ICONEXCLAMATION $(IDS_MESSAGE_EMPTYSELECTION) /SD IDYES IDYES wachk_components_leave_exit
			Abort
	${EndIf}		
		
	wachk_components_leave_exit:
!macroend

!macro WACHK_INSTALLER_COMPONENTS_DESC
	${DESCRIPTION} IDX_GROUP_WINAMP_DETECT $(IDS_GROUP_WINAMP_DETECT_DESC)
	${WACHK_INSTALLER_FIREFOX_DESC}
	${WACHK_INSTALLER_EXPLORER_DESC}
!macroend

!macro WACHK_INSTALLER_WRITE_STARTMENU
	${WACHK_GET_STARTMENU_PATH} $0
	${If} $0 != ""
		DetailPrint	"$(IDS_DETAILS_CREATE_SHORTCUTS)..."
		SetDetailsPrint none
		
		CreateDirectory "$0"
		CreateShortCut "$0\$(IDS_UNINSTAL_SHORTCUT).lnk" "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
		
		SetDetailsPrint lastused
	${EndIf}
!macroend

!macro WACHK_INSTALLER_WRITE_INSTALL_INFO
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "DisplayName" "$(IDS_INSTALLER_NAME)"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "DisplayVersion" "${WACHK_INSTALLER_VER}"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "UninstallString" "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "URLInfoAbout" "${WACHK_INSTALLER_HOMELINK}"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "Publisher" "$(IDS_INSTALLER_PUBLISHER)"
	  
	IntOp $0 ${WACHK_INSTALLER_VER_MAJOR} << 16
	IntOp $1 ${WACHK_INSTALLER_VER_MINOR} & 0x0000FFFF
	IntOp $0 $0 | $1
	WriteRegDWORD ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "VersionMajor" "$0"
 
	IntOp $0 ${WACHK_INSTALLER_VER_MINOR2} << 16
	IntOp $1 ${WACHK_INSTALLER_VER_BUILD} & 0x0000FFFF
	IntOp $0 $0 | $1
	WriteRegDWORD ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "VersionMinor" "$0"
	
	WriteRegDWORD ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "NoRepair" "1"
	WriteRegDWORD ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "NoModify" "0"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "ModifyPath" "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
	
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "DisplayIcon" "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
!macroend

!macro WACHK_INSTALLER_WRITE_INSTALL_METRICS
	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "InstallDate" "$2$1$0"
	WriteRegDWORD ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "EstimatedSize" "$wachk.installer.installedSize"
	WriteRegStr ${WACHK_INSTALLER_REGROOT} "${WACHK_UNINSTALLER_REGKEY}" "${WACHK_STARTMENU_REGKEY}" "$wachk.installer.startMenuPath"
	WriteRegStr "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "${WACHK_LANGUAGE_REGKEY}" "$LANGUAGE"
!macroend

!define WACHK_INSTALLER_COMPONENTS_INIT	"!insertmacro 'WACHK_INSTALLER_COMPONENTS_INIT'"
!define WACHK_INSTALLER_COMPONENTS_PRE	"!insertmacro 'WACHK_INSTALLER_COMPONENTS_PRE'"
!define WACHK_INSTALLER_COMPONENTS_LEAVE "!insertmacro 'WACHK_INSTALLER_COMPONENTS_LEAVE'"
!define WACHK_INSTALLER_COMPONENTS_DESC "!insertmacro 'WACHK_INSTALLER_COMPONENTS_DESC'"
!define WACHK_INSTALLER_WRITE_STARTMENU "!insertmacro 'WACHK_INSTALLER_WRITE_STARTMENU'"
!define WACHK_INSTALLER_WRITE_INSTALL_INFO "!insertmacro 'WACHK_INSTALLER_WRITE_INSTALL_INFO'"
!define WACHK_INSTALLER_WRITE_INSTALL_METRICS "!insertmacro 'WACHK_INSTALLER_WRITE_INSTALL_METRICS'"


!endif

!ifdef WACHK_INSERT_SECTIONS

!ifndef WACHK_EMBEDDED_INSTALLER
Section -First
	DetailPrint	"$(IDS_DETAILS_INITIALIZING)..."
	SetDetailsPrint none
	StrCpy $wachk.installer.installedSize 0
	SetDetailsPrint lastused
SectionEnd
!endif

!define WACHK_INSERT_INSTALLER_SECTIONS
!include ".\firefox.nsh"
!include ".\explorer.nsh"
!undef WACHK_INSERT_INSTALLER_SECTIONS

!ifndef WACHK_EMBEDDED_INSTALLER
Section -WriteStartMenu
	${WACHK_INSTALLER_WRITE_STARTMENU} 
SectionEnd
!endif

!ifndef WACHK_EMBEDDED_INSTALLER
Section -WriteUninstall
			
	${If} ${FileExists} "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
		${GetFileVersion} "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}" $0
		${VersionCompare} $0 "${WACHK_INSTALLER_VER}" $1
		${If} $1 == 0
		${OrIf} $1 == 1
			DetailPrint	"$(IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME)."
			DetailPrint	"$(IDS_DETAILS_SKIPPING)..."
			goto wachk_write_uninstaller_skip
		${EndIf}
	${EndIf}
				
	DetailPrint	"$(IDS_DETAILS_WRITE_UNINSTALL)..."
	SetDetailsPrint none
	WriteUninstaller "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
	SetDetailsPrint lastused
		
	${WACHK_INSTALLER_WRITE_INSTALL_INFO}

wachk_write_uninstaller_skip:	
	${WACHK_INSTALLER_WRITE_INSTALL_METRICS}
	
SectionEnd
!endif

!ifndef WACHK_EMBEDDED_INSTALLER
Section -Last
	SetDetailsPrint both
	!ifdef WACHK_DEBUG_MODE
		SetAutoClose false
	!endif 
SectionEnd
!endif

!endif