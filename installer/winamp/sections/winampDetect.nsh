!ifndef WINAMPDETECT_HEADER
!define WINAMPDETECT_HEADER

!ifdef FULL
	!define WINAMPDETECT_INCLUDE
!endif

!ifdef WINAMPDETECT_INCLUDE
	!define WACHK_EMBEDDED_INSTALLER
	!define WACHK_PLUGIN_OVERWRITE_NEWER

	!cd "..\browserplugin"
	!include ".\config.nsh"
	!include ".\variables.nsh"
	!include ".\sectionsInstall.nsh"
	!cd "..\winamp"

	var winampdetect.instal.dir
	var winampdetect.winamp.instal.dir

	!define WINAMPDETECT_INIT "!insertmacro 'WINAMPDETECT_INIT'"
	!macro WINAMPDETECT_INIT
		StrCpy $winampdetect.winamp.instal.dir "$INSTDIR"
		StrCpy $winampdetect.instal.dir "${WACHK_INSTALLER_DEFPATH}"
		
		!cd "..\browserplugin"
		${WACHK_INSTALLER_COMPONENTS_INIT}
		!cd "..\winamp"
		
		StrCpy $winampdetect.instal.dir "$INSTDIR"
		StrCpy $INSTDIR "$winampdetect.winamp.instal.dir"
	!macroend

	!define WINAMPDETECT_COMPONENTS_PRE "!insertmacro 'WINAMPDETECT_COMPONENTS_PRE'"
	!macro WINAMPDETECT_COMPONENTS_PRE
		!cd "..\browserplugin"
		
		ReadRegStr $winampdetect.instal.dir "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "InstallLocation"
		${If} $winampdetect.instal.dir == ""
			${GetParent} "$INSTDIR" $winampdetect.instal.dir
			${If} $winampdetect.instal.dir != ""
				StrCpy $winampdetect.instal.dir "$winampdetect.instal.dir\${WACHK_INSTALLER_NAME}"
			${Else}
				StrCpy $winampdetect.instal.dir "${WACHK_INSTALLER_DEFPATH}"
			${EndIf}
		${EndIf}
			
		StrCpy $winampdetect.winamp.instal.dir "$INSTDIR"
		StrCpy $INSTDIR "$winampdetect.instal.dir"
		
		${WACHK_INSTALLER_COMPONENTS_PRE}
		StrCpy $INSTDIR "$winampdetect.winamp.instal.dir"
		
		!cd "..\winamp"
	!macroend

	!define WINAMPDETECT_INSERT_SECTION "!insertmacro 'WINAMPDETECT_INSERT_SECTION'"
	!macro WINAMPDETECT_INSERT_SECTION
		Section -WinampDetectFirst
			StrCpy $winampdetect.winamp.instal.dir "$INSTDIR"
			StrCpy $INSTDIR $winampdetect.instal.dir
			; CreateDirectory "$INSTDIR"
		SectionEnd
		
		SectionGroup  $(IDS_GROUP_WINAMP_DETECT) IDX_GROUP_WINAMP_DETECT
			!define WACHK_INSERT_SECTIONS
				!cd "..\browserplugin"
				!include ".\sectionsInstall.nsh"
				!cd "..\winamp"
			!undef WACHK_INSERT_SECTIONS
		SectionGroupEnd
		
		Section -WinampDetectLast
			!cd "..\browserplugin"
			!system '$\"${NSISDIR}\makensis.exe$\" /V2 /NOCD /DLANG_USE_ALL $\".\createUninstall.nsi$\"'
			!system '.\createUninstall.exe'
			!cd "..\winamp"
			
			${If} $wachk.installer.installedSize == 0
				goto winampdetect_last_section_end
			${EndIf}
			
			${WACHK_INSTALLER_WRITE_STARTMENU}
			
			${If} ${FileExists} "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}"
				${GetFileVersion} "$INSTDIR\${WACHK_UNINSTALLER_FILENAME}" $0
				${VersionCompare} $0 "${WACHK_INSTALLER_VER}" $1
				${If} $1 == 0
				${OrIf} $1 == 1
					DetailPrint	"$(IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME)."
					DetailPrint	"$(IDS_DETAILS_SKIPPING)..."
					goto winampdetect_last_section_end_with_metrics
				${EndIf}
			${EndIf}
			
			DetailPrint	"$(IDS_DETAILS_WRITE_UNINSTALL)..."
			SetDetailsPrint none
			
			StrCpy $0 $OUTDIR
			SetOutPath $winampdetect.instal.dir
			File "..\browserplugin\${WACHK_UNINSTALLER_FILENAME}"
			SetOutPath $0
			
			SetDetailsPrint lastused
			
			${WACHK_INSTALLER_WRITE_INSTALL_INFO}
		
		winampdetect_last_section_end_with_metrics:
			${WACHK_INSTALLER_WRITE_INSTALL_METRICS}
			
		winampdetect_last_section_end:
			StrCpy $INSTDIR "$winampdetect.winamp.instal.dir"
		SectionEnd
	!macroend

	!define WINAMPDETECT_INSERT_LANG "!insertmacro 'WINAMPDETECT_INSERT_LANG'"
	!macro WINAMPDETECT_INSERT_LANG
		!cd "..\browserplugin"
		!include ".\languages.nsh"
		!cd "..\winamp"
	!macroend
	
	!define WINAMPDETECT_SHOW_REQUESTRESTART "!insertmacro 'WINAMPDETECT_SHOW_REQUESTRESTART'"
	!macro WINAMPDETECT_SHOW_REQUESTRESTART
		; as requested by Ben London user notification disabled (double phee)
		;${If} $wachk.installer.restartRequest != ""
		;		MessageBox MB_OK "$(IDS_MESSAGE_BROWSERRESTART)$\r$\n$\r$\n$wachk.installer.restartRequest" /SD IDOK
		;${EndIf}
	!macroend
	
	!define WINAMPDETECT_INSERT_DESCRIPTIONS "!insertmacro 'WINAMPDETECT_INSERT_DESCRIPTIONS'"
	!macro WINAMPDETECT_INSERT_DESCRIPTIONS
		${WACHK_INSTALLER_COMPONENTS_DESC}
	!macroend
	
!else
	!define WINAMPDETECT_INIT ""
	!define WINAMPDETECT_COMPONENTS_PRE ""
	!define WINAMPDETECT_INSERT_SECTION ""
	!define WINAMPDETECT_INSERT_LANG ""
	!define WINAMPDETECT_SHOW_REQUESTRESTART ""
	!define WINAMPDETECT_INSERT_DESCRIPTIONS ""
!endif 

!endif


