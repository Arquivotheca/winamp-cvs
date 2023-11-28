!ifndef WACHK_PAGES_UNINSTALL_HEADER
!define WACHK_PAGES_UNINSTALL_HEADER

!include ".\pagesConfig.nsh"
!include ".\functions.nsh"
!include ".\sectionsUninstall.nsh"

!ifdef MUI_COMPONENTSPAGE_NODESC
!undef MUI_COMPONENTSPAGE_NODESC
!endif


!insertmacro MUI_UNPAGE_CONFIRM

!define MUI_COMPONENTSPAGE_NODESC
!define MUI_PAGE_HEADER_TEXT				$(IDS_PAGE_COMPONENT_HEADER)
!define MUI_PAGE_HEADER_SUBTEXT				$(IDS_UNPAGE_COMPONENT_SUBHEADER)
!define MUI_COMPONENTSPAGE_TEXT_TOP			$(IDS_UNPAGE_COMPONENT_TOP)
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST	$(IDS_UNPAGE_COMPONENT_LIST)
!define MUI_PAGE_CUSTOMFUNCTION_PRE			un.WinampCheck_OnComponentsPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW		un.WinampCheck_OnComponentsShow
!insertmacro MUI_UNPAGE_COMPONENTS

!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE				$(IDS_UNPAGE_FINISH_TITLE)
!define MUI_FINISHPAGE_TEXT_LARGE
!define MUI_FINISHPAGE_TEXT 				$(IDS_UNPAGE_FINISH_TEXT)
!define MUI_FINISHPAGE_LINK  				$(IDS_UNPAGE_FINISH_LINK)
!define MUI_FINISHPAGE_LINK_LOCATION 		"${WACHK_INSTALLER_FEEDBACKLINK}"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_PAGE_CUSTOMFUNCTION_SHOW		un.WinampCheck_OnFinishShow
!insertmacro MUI_UNPAGE_FINISH

Function un.WinampCheck_OnComponentsPre
	${WACHK_UNINSTALLER_COMPONENTS_PRE}
FunctionEnd

Function un.WinampCheck_OnComponentsShow
	ShowWindow $mui.ComponentsPage.SpaceRequired ${SW_HIDE}
FunctionEnd

Function un.WinampCheck_OnFinishShow
	${WACHK_UNINSTALLER_IS_COMPLETE} $0
	${If} $0 != "true"
		StrCpy $1 "$(IDS_UNPAGE_FINISH_TITLE_PARTIAL)"
		StrCpy $2 "$(IDS_UNPAGE_FINISH_TEXT_PARTIAL)"
	${Else}
		StrCpy $1 "$(IDS_UNPAGE_FINISH_TITLE)"
		StrCpy $2 "$(IDS_UNPAGE_FINISH_TEXT)"
	${EndIf}

	${If} $wachk.installer.restartRequest != ""
		StrCpy $2 "$2$\r$\n$\r$\n$\r$\n$\r$\n$\r$\n$(IDS_MESSAGE_BROWSERRESTART)$\r$\n$\r$\n$wachk.installer.restartRequest"
	${EndIf}
		
	SendMessage $mui.FinishPage.Title ${WM_SETTEXT} 0 "STR:$1"
	SendMessage $mui.FinishPage.Text ${WM_SETTEXT} 0 "STR:$2"
FunctionEnd

!endif