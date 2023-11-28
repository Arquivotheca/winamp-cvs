!ifndef NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER
!define NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER

!include ".\uninstall\ui.nsh"
!include ".\pages\groupCheckList.nsh"
!include ".\uninstall\programs.nsh"
!include ".\utils\wafuncs.nsh"
!include ".\utils\sectionDescription.nsh"

Function un.UI_OnComponentsPageCreate
	Push $0
	Push $1
	Push $2
	
	${UninstallBundle_InitializeGroup}
	${UninstallBundle_IsGroupEmpty} $0
	${NextButton_SetLastPageMode} $0
	
	StrCpy $0 ${IDX_UNINSTALL_COMPONENTS_GROUP}
	ClearErrors
	SectionGetText $0 $1
	${If} ${Errors}
		StrCpy $1 ""
	${EndIf}
	
	${GetSectionDescription} $0 $2
	!insertmacro MUI_HEADER_TEXT "$1"  "$2"
	
	${GroupCheckList_CreatePage} $0 "" "$(IDS_UNINSTALL_COMPONENTS_FOOTER)" "" "default" ""
	
	Pop $2
	Pop $1
	Pop $0
	
FunctionEnd

Function un.UI_OnProgramsPageCreate
	Push $0
	Push $1
	Push $2
	
	${UninstallBundle_InitializeGroup}
	
	${UninstallBundle_IsGroupEmpty} $0
	${If} $0 == "true"
		Abort
	${EndIf}
		
	StrCpy $0 ${IDX_UNINSTALL_BUNDLES_GROUP}
	ClearErrors
	SectionGetText $0 $1
	${If} ${Errors}
		StrCpy $1 ""
	${EndIf}
	
	${GetSectionDescription} $0 $2
	!insertmacro MUI_HEADER_TEXT "$1"  "$2"
	${UninstallBundle_GetDescriptionProvider} $1
	${GroupCheckList_CreatePage} $0 "$(IDS_UNINSTALL_BUNDLES_HEADER)" "" "" "$1" ""
	
	Pop $2
	Pop $1
	Pop $0
FunctionEnd

Function un.UI_OnFinishPageReadMe
	ExecShell "open" '"http://feedback.aol.com/rs/rs.php?sid=winamp_client_uninstall"'
FunctionEnd

Function un.UI_OnFinishPageRun
	ExecShell "open" '"$INSTDIR"'
FunctionEnd

!ifndef GA_PARENT
	!define GA_PARENT 1
!endif

			
Function un.UI_OnFinsihPageShow

	!ifdef MUI_FINISHPAGE_RUN_VARIABLES
		${OffsetWindowPos} $mui.FinishPage.Run 0 80
	!endif
	
	!ifdef MUI_FINISHPAGE_SHOREADME_VARAIBLES
		${OffsetWindowPos} $mui.FinishPage.ShowReadme 0 70
	!endif
	
	${IncreaseWindowSize} $mui.FinishPage.Text 0 70
	
	Push $0
	
	StrCpy $0 ""
	${If} $winamp.uninstall.checkFolder == "true"
		${DirState} "$INSTDIR" $0
		${If} $0 == 1
			StrCpy $0 "show_explorer"
		${Else}
			StrCpy $0 0
		${EndIf}
	${EndIf}	
	
	${If} $0 == "show_explorer"
		${If} $mui.FinishPage.Text != 0
			${AppendWindowText} $mui.FinishPage.Text "$(IDS_UNINSTALL_FILES_NOT_REMOVED)"
		${EndIf}
	${Else}
		!ifdef MUI_FINISHPAGE_RUN_VARIABLES
			SendMessage $mui.FinishPage.Run ${BM_SETCHECK} 0 0
			ShowWindow $mui.FinishPage.Run ${SW_HIDE}
		!endif
	${EndIf}
	
	Pop $0

FunctionEnd

!endif ; NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER