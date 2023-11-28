!ifndef WACHK_FUNCTIONS_HEADER
!define WACHK_FUNCTIONS_HEADER

!include ".\logicLib.nsh"
!include ".\wordFunc.nsh"
!include ".\fileFunc.nsh"


!define WACHK_INIT	"!insertmacro 'WACHK_INIT'"
!macro WACHK_INIT
	
	
	StrCpy $wachk.installer.installedSize 0
	StrCpy $wachk.installer.startMenuPath ""
	
	
	ReadRegStr $0 "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "InstallLocation"
	${If} $0 != ""
		StrCpy $INSTDIR "$0"
	${EndIf}
		
	StrCpy $wachk.winamp.version ""
	ReadRegStr $0 HKCU "Software\Winamp" ""
	${If} $0 != ""
		ClearErrors
		${GetFileVersion} "$0\winamp.exe" $wachk.winamp.version
		${If} ${Errors}
			StrCpy $wachk.winamp.version ""
		${EndIf}
	${EndIf}

!macroend

!define WACHK_GET_STARTMENU_PATH	"!insertmacro 'WACHK_GET_STARTMENU_PATH'"
!macro WACHK_GET_STARTMENU_PATH	__pathResult
	${If} $wachk.installer.startMenuPath == ""
		ReadRegStr $wachk.installer.startMenuPath "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "${WACHK_STARTMENU_REGKEY}"
		${If} $wachk.installer.startMenuPath == ""
			StrCpy $wachk.installer.startMenuPath	"${WACHK_STARTMENU_DEFFOLDER}"
		${EndIf}
	${EndIf}
	
	${If} $wachk.installer.startMenuPath == ""
		StrCpy "${__pathResult}" ""
	${Else}
		StrCpy "${__pathResult}" $wachk.installer.startMenuPath 1
		${If} "${__pathResult}" == ">"
			StrCpy "${__pathResult}" ""
		${Else}
			SetShellVarContext current
			StrCpy "${__pathResult}" "$SMPROGRAMS\$wachk.installer.startMenuPath"
		${EndIf}
	${EndIf}
!macroend

!define WACHK_SECTION_HIDE	"!insertmacro 'WACHK_SECTION_HIDE'"
!macro WACHK_SECTION_HIDE	__sectionIndex
	Push $0
	Push $1
	
	SectionGetText ${__sectionIndex} $0
	SectionGetFlags	'${__sectionIndex}' $0
	IntOp $1 ${SF_SELECTED} ~
	IntOp $0 $0	& $1
	SectionSetFlags	'${__sectionIndex}' $0
	SectionSetText '${__sectionIndex}' ""

	Pop $1
	Pop $0
!macroend

!define WACHK_SECTION_REMOVE	"!insertmacro 'WACHK_SECTION_REMOVE'"
!macro WACHK_SECTION_REMOVE	__sectionIndex __discoveredVersion __minVersion __maxVersion
	Push $0
	Push $1
	StrCpy $1 '${__discoveredVersion}'
	${If} '${__minVersion}' != ""
		${VersionCompare} "${__discoveredVersion}" "${__minVersion}" $0
		${If} $0 == "2"
			StrCpy $1 ""
		${EndIf}
	${EndIf}
	
	${If} '${__maxVersion}' != ""
		${VersionCompare} "${__discoveredVersion}" "${__maxVersion}" $0
		${If} $0 == "1"
			StrCpy $1 ""
		${EndIf}
	${EndIf}
	
	${If} '${__discoveredVersion}' == ""
		${WACHK_SECTION_HIDE} '${__sectionIndex}' 
	${EndIf}
	
	Pop $1
	Pop $0	
!macroend

!define WACHK_SECTION_UPDATESELECT	"!insertmacro 'WACHK_SECTION_UPDATESELECT'"
!macro WACHK_SECTION_UPDATESELECT	__sectionIndex __targetDll __localDll
	!define WACHK_UPDATESECTION_CHECK		wachk_updatesection_check'${__sectionIndex}'
	!define WACHK_UPDATESECTION_UNCHECK		wachk_updatesection_uncheck'${__sectionIndex}'
	!define WACHK_UPDATESECTION_END			wachk_updatesection_end'${__sectionIndex}'
	
	Push $0
	Push $1
	Push $2
	Push $3
	
	SectionGetText '${__sectionIndex}' $0
	${If} $0 == ""
		goto ${WACHK_UPDATESECTION_UNCHECK}
	${EndIf}
	
	IfFileExists '${__targetDll}' 0 ${WACHK_UPDATESECTION_CHECK}
	
	GetDLLVersion '${__targetDll}' $0 $1
		${If} $0 != ""
	${AndIf} $1 != ""
		GetDLLVersionLocal '${__localDll}' $2 $3
		${If} $2 != ""
		${AndIf} $3 != ""
			${If} $0 > $2
				goto ${WACHK_UPDATESECTION_UNCHECK}
			${Else}
				${If} $0 == $2
				${AndIf} $1 >= $3
					goto ${WACHK_UPDATESECTION_UNCHECK}
				${EndIf}
			${EndIf}	
		${EndIf}
	${EndIf}
	
	goto ${WACHK_UPDATESECTION_CHECK}

${WACHK_UPDATESECTION_UNCHECK}:
	SectionGetFlags	'${__sectionIndex}' $0
	IntOp $1 ${SF_SELECTED} ~
	IntOp $0 $0	& $1
	SectionSetFlags	'${__sectionIndex}' $0
	goto ${WACHK_UPDATESECTION_END}

${WACHK_UPDATESECTION_CHECK}:
	SectionGetFlags	'${__sectionIndex}' $0
	IntOp $0 $0	| ${SF_SELECTED}
	SectionSetFlags	'${__sectionIndex}' $0
	goto ${WACHK_UPDATESECTION_END}

${WACHK_UPDATESECTION_END}:	
	Pop $3
	Pop $2
	Pop $1
	Pop $0

	!undef WACHK_UPDATESECTION_CHECK
	!undef WACHK_UPDATESECTION_UNCHECK
	!undef WACHK_UPDATESECTION_END
!macroend


!macro WACHK_SECTION_INSTALL __sectionName
	!ifdef WinampSection
		${WinampSection} "winampDetect${__sectionName}" '$(IDS_SECTION_${__sectionName})' 'IDX_SECTION_${__sectionName}'
	!else
		Section '$(IDS_SECTION_${__sectionName})' 'IDX_SECTION_${__sectionName}'
	!endif
!macroend
!define WACHK_SECTION_INSTALL "!insertmacro 'WACHK_SECTION_INSTALL'"

!macro WACHK_SECTION_UNINSTALL __sectionName
	Section 'un.$(IDS_SECTION_${__sectionName})' 'IDX_SECTION_UN_${__sectionName}'
!macroend
!define WACHK_SECTION_UNINSTALL "!insertmacro 'WACHK_SECTION_UNINSTALL'"

!macro WACHK_SECTION_END
	!ifdef WinampSection
		${WinampSectionEnd}
	!else
		SectionEnd
	!endif
!macroend
!define WACHK_SECTION_END "!insertmacro 'WACHK_SECTION_END'"

!define WACHK_INSTALL_BROWSER_PLUGIN "!insertmacro 'WACHK_INSTALL_BROWSER_PLUGIN'"
!macro WACHK_INSTALL_BROWSER_PLUGIN	__browserName __browserVersion __sourceDll __targetDll __payloadCallback __processName __sectionIndex
	Push $0
	Push $1
	Push '${__sectionIndex}'
	Push '${__processName}'
	GetDLLVersionLocal '${__sourceDll}' $0 $1
	Push $1
	Push $0
	GetFunctionAddress $0 '${__payloadCallback}'
	Push $0
	Push '${__targetDll}'
	Push '${__browserVersion}'
	Push '${__browserName}'	
	Call WinampCheck_InstallBrowserPlugin
	Pop $1
	Pop $0
!macroend

!define WACHK_PROCESS_RESTART_REQUIRED "!insertmacro 'WACHK_PROCESS_RESTART_REQUIRED'"
!macro WACHK_PROCESS_RESTART_REQUIRED __processName __printName
	Push $0
	FindProc $0 "${__processName}"
	${If} $0 == 1
		Pop $0
		StrCpy $wachk.installer.restartRequest "$wachk.installer.restartRequest$\t- ${__printName}$\r$\n"
	${Else}
		Pop $0
	${EndIf}
	
	
!macroend

!ifndef WACHK_UNINSTALLER_CODE_ONLY
Function WinampCheck_InstallBrowserPlugin
	Exch $0	; browser name
	Exch 1
	Exch $1	; browser version
	Exch 2
	Exch $2	; target dll
	Exch 3
	Exch $3  ; payload callback
	Exch 4
	Exch $4  ; source dll version high
	Exch 5
	Exch $5  ; source dll version low
	Exch 6
	Exch $8  ; process name 
	Exch 7
	Exch $9  ; section index
	Push $6
	Push $7
	Push $R0
	Push $R1
	Push $R2
	Push $R3
	
	DetailPrint "$(IDS_DETAILS_PLUGIN_INSTALL_BEGIN)..."
	${If} $1 == ""
		DetailPrint "  $(IDS_DETAILS_BROWSER_NOT_FOUND)."
		goto wachk_install_browser_plugin_end
	${EndIf}
	ClearErrors 
	DetailPrint "  $(IDS_DETAILS_BROWSER_VERSION): $1"
		
	${If} $4 != ""
	${AndIf} $5 != ""
		IntOp $R0 $4 / 0x00010000
		IntOp $R1 $4 & 0x0000FFFF
		IntOp $R2 $5 / 0x00010000
		IntOp $R3 $5 & 0x0000FFFF
		DetailPrint "  $(IDS_DETAILS_SOURCE_PLUGIN_VERSION): $R0.$R1.$R2.$R3"
	${EndIf}
	
	${If} ${FileExists} '$2'
		GetDLLVersion '$2' $6 $7
		${If} $6 != ""
		${AndIf} $7 != ""
			IntOp $R0 $6 / 0x00010000
			IntOp $R1 $6 & 0x0000FFFF
			IntOp $R2 $7 / 0x00010000
			IntOp $R3 $7 & 0x0000FFFF
			DetailPrint "  $(IDS_DETAILS_TARGET_PLUGIN_VERSION): $R0.$R1.$R2.$R3"
						
			${If} $6 > $4
				DetailPrint "  $(IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME)."
				!ifndef WACHK_PLUGIN_OVERWRITE_NEWER
					goto wachk_install_browser_plugin_add_section_size_and_end
				!endif	
			${Else}
				${If} $6 == $4
				${AndIf} $7 >= $5
					DetailPrint "  $(IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME)."
					!ifndef WACHK_PLUGIN_OVERWRITE_NEWER
						goto wachk_install_browser_plugin_add_section_size_and_end
					!endif
				${EndIf}
			${EndIf}
		${Else}
			DetailPrint "  $(IDS_DETAILS_TARGET_PLUGIN_VERSION): $(IDS_UNKNOWN)."
		${EndIf}
	${Else}
		DetailPrint "  $(IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND)."
	${EndIf}

	
	DetailPrint "  $(IDS_DETAILS_INSTALLING_PLUGIN)..."
	SetDetailsPrint none
	Call $3
	Pop $R0

	${If} $R0 == "success"
		SectionGetSize $9 $R0
		IntOp $wachk.installer.installedSize $wachk.installer.installedSize + $R0
		${WACHK_PROCESS_RESTART_REQUIRED} "$8" "$0"
	${Else}
		SetDetailsPrint lastused
		DetailPrint	"  Error: $R0"
		SetDetailsPrint none
		MessageBox MB_OK|MB_ICONSTOP "$(IDS_DETAILS_PLUGIN_INSTALL_ERROR)$\r$\n$\r$\n$(IDS_DETAILS): $R0" /SD IDOK
	${EndIf}
	
	SetDetailsPrint lastused
	goto wachk_install_browser_plugin_end

!ifndef WACHK_PLUGIN_OVERWRITE_NEWER
wachk_install_browser_plugin_add_section_size_and_end:	
	SectionGetSize $9 $R0
	IntOp $wachk.installer.installedSize $wachk.installer.installedSize + $R0
	goto wachk_install_browser_plugin_end
!endif
	
wachk_install_browser_plugin_end:	
	DetailPrint "  Done."
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0
	Push $7
	Push $6
	Push $9
	Push $8
	Push $5
	Push $4
	Push $3
	Push $2
	Push $1
	Push $0
FunctionEnd
!endif

!define WACHK_SECTION_SIZE_ADD_FILE "!insertmacro 'WACHK_SECTION_SIZE_ADD_FILE'"
!define WACHK_SECTION_SIZE_BEGIN "!insertmacro 'WACHK_SECTION_SIZE_BEGIN'"
!define WACHK_SECTION_SIZE_END "!insertmacro 'WACHK_SECTION_SIZE_END'"

!macro WACHK_SECTION_SIZE_ADD_FILE __localFile 
	!ifndef WACHK_FILESIZELOCAL_GENERATED
	!define WACHK_FILESIZELOCAL_GENERATED
		!system '$\"${NSISDIR}\makensis.exe$\" /V2 /NOCD $\".\fileSizeLocal.nsi$\"'
	!endif
	
	!system '.\fileSizeLocal.exe /source=$\"${__localFile}$\" /output=$\".\fileSizeLocal.nsh$\"'
	!include ".\fileSizeLocal.nsh"
	SectionGetSize '${WACHK_SECTION_SIZE_INDEX}' $0
	
	IntOp $0 $0 + ${FILE_SIZE_LOCAL}
	SectionSetSize '${WACHK_SECTION_SIZE_INDEX}' $0
	!undef FILE_SIZE_LOCAL
	!delfile ".\fileSizeLocal.nsh"
!macroend

!macro WACHK_SECTION_SIZE_BEGIN __sectionIndex
	!ifdef WACHK_SECTION_SIZE_INDEX
		!undef WACHK_SECTION_SIZE_INDEX
	!endif
	!define WACHK_SECTION_SIZE_INDEX '${__sectionIndex}' 
	SectionSetSize '${__sectionIndex}' $0
!macroend
	
!macro WACHK_SECTION_SIZE_END
	SectionGetSize '${WACHK_SECTION_SIZE_INDEX}' $0
	IntOp $0 $0 / 1024
	SectionSetSize '${WACHK_SECTION_SIZE_INDEX}' $0
	!ifdef WACHK_SECTION_SIZE_INDEX
		!undef WACHK_SECTION_SIZE_INDEX
	!endif
!macroend


!endif