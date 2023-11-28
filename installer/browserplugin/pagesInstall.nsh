!ifndef WACHK_PAGES_INSTALL_HEADER
!define WACHK_PAGES_INSTALL_HEADER

!include ".\pagesConfig.nsh"
!include ".\functions.nsh"
!include ".\sectionsInstall.nsh"

!ifdef MUI_COMPONENTSPAGE_NODESC
!undef MUI_COMPONENTSPAGE_NODESC
!endif


!define MUI_WELCOMEPAGE_TITLE		 	$(IDS_PAGE_WELCOME_TITLE)
!define MUI_WELCOMEPAGE_TEXT			$(IDS_PAGE_WELCOME_TEXT) 
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE 			".\resources\license.rtf"

!define MUI_DIRECTORYPAGE_TEXT_TOP		$(IDS_PAGE_DIRECTORY_TOP) 
!define MUI_PAGE_CUSTOMFUNCTION_SHOW	WinampCheck_OnDirectoryShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE	WinampCheck_OnDirectoryLeave
!insertmacro MUI_PAGE_DIRECTORY



!define MUI_COMPONENTSPAGE_NODESC
!define MUI_PAGE_HEADER_TEXT				$(IDS_PAGE_COMPONENT_HEADER)
!define MUI_PAGE_HEADER_SUBTEXT				$(IDS_PAGE_COMPONENT_SUBHEADER)
!define MUI_COMPONENTSPAGE_TEXT_TOP			$(IDS_PAGE_COMPONENT_TOP) 
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST	$(IDS_PAGE_COMPONENT_LIST)
!define MUI_PAGE_CUSTOMFUNCTION_PRE			WinampCheck_OnComponentsPre
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE		WinampCheck_OnComponentsLeave
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_STARTMENUPAGE_DEFAULTFOLDER 		"${WACHK_STARTMENU_DEFFOLDER}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT 		"${WACHK_INSTALLER_REGROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY 			"${WACHK_UNINSTALLER_REGKEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME 	"${WACHK_STARTMENU_REGKEY}"
!insertmacro MUI_PAGE_STARTMENU ${WACHK_STARTMENU_PAGEID} $wachk.installer.startMenuPath

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE				$(IDS_PAGE_FINISH_TITLE)
!define MUI_FINISHPAGE_TEXT_LARGE
!define MUI_FINISHPAGE_TEXT 				$(IDS_PAGE_FINISH_TEXT)
!define MUI_FINISHPAGE_LINK  				$(IDS_PAGE_FINISH_LINK)
!define MUI_FINISHPAGE_LINK_LOCATION 		"${WACHK_INSTALLER_HOMELINK}"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_PAGE_CUSTOMFUNCTION_SHOW		WinampCheck_OnFinishShow
!insertmacro MUI_PAGE_FINISH

Function WinampCheck_OnDirectoryShow
	ShowWindow $mui.DirectoryPage.SpaceRequired	${SW_HIDE}
FunctionEnd

Function WinampCheck_OnDirectoryLeave
	ReadRegStr $0 "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "InstallLocation"
	${If} $0 != ""
		${If} $0 != $INSTDIR
			ReadRegStr $1 "${WACHK_INSTALLER_REGROOT}" "${WACHK_UNINSTALLER_REGKEY}" "UninstallString"
			${If} $1 != ""
			${AndIf} ${FileExists} $1
				MessageBox MB_YESNO|MB_ICONEXCLAMATION $(IDS_MESSAGE_SUGGEST_UNINSTAL) /SD IDYES IDNO wachk_ignore_uninstall_and_continue
				ExecWait '"$1" /S _?=$0' $1
				${If} $1 == 0
					RMDir /r "$0"
				${Else}
					MessageBox MB_OK|MB_ICONSTOP $(IDS_MESSAGE_UNINSTALL_FAILED) /SD IDOK
					Abort
				${EndIf}
			${EndIf} 
		${EndIf}
	${EndIf}
wachk_ignore_uninstall_and_continue:
FunctionEnd

Function WinampCheck_OnComponentsPre
	${If} $wachk.winamp.version == ""
		MessageBox MB_YESNO|MB_ICONEXCLAMATION $(IDS_MESSAGE_NOWINAMP) /SD IDNO IDYES wachk_no_winamp_continue_init
		Quit	
	${EndIf}
	wachk_no_winamp_continue_init:
	${WACHK_INSTALLER_COMPONENTS_PRE}
FunctionEnd

Function WinampCheck_OnComponentsLeave
	${WACHK_INSTALLER_COMPONENTS_LEAVE}
FunctionEnd

Function WinampCheck_OnFinishShow
	${If} $wachk.installer.restartRequest != ""
		System::Call "user32::GetWindowText(i$mui.FinishPage.Text, w.r0, i${NSIS_MAX_STRLEN})"
		StrCpy $0 "$0$\r$\n$\r$\n$\r$\n$\r$\n$\r$\n$(IDS_MESSAGE_BROWSERRESTART)$\r$\n$\r$\n$wachk.installer.restartRequest"
		SendMessage $mui.FinishPage.Text ${WM_SETTEXT} 0 "STR:$0"
	${EndIf}
FunctionEnd
!endif