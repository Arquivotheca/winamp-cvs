!ifndef WINAMP_PAGES_INSTALL_HEADER
!define WINAMP_PAGES_INSTALL_HEADER

!include ".\ui.nsh"
!include ".\sections\musicbundle.nsh"
!include ".\sections\winampDetect.nsh"
!include ".\sections\xbundle.nsh"

Var waui.button.advanced

Function UI_OnInit
	Push $R0
	StrCpy $R0 0
	StrCpy $IDX_INSTTYPE_FULL  $R0
	!ifdef FULL
		InstTypeSetText $R0 $(installFull)
		IntOp $R0 $R0 + 1
	!endif ; FULL

	StrCpy $IDX_INSTTYPE_STANDART  $R0
	!ifdef FULL | STD
		InstTypeSetText $R0 $(installStandart)
		IntOp $R0 $R0 + 1
	!endif ; FULL / STD
 
	InstTypeSetText $R0 $(installLite)
	IntOp $R0 $R0 + 1
	InstTypeSetText $R0 $(installMinimal)

	StrCpy $IDX_INSTTYPE_PREVIUOS  $R0
FunctionEnd

Function .onGUIEnd
	${OPENCANDY_GUIEND}
FunctionEnd

Function UI_OnDirectoryPageShow
	FindWindow $1 "#32770" "" $HWNDPARENT
	GetDlgItem $0 $1 0x3FF
	ShowWindow $0 ${SW_HIDE}
  
	GetDlgItem $0 $HWNDPARENT 0x02
	System::Call "*${stRECT} .r1"
	System::Call 'User32::GetWindowRect(i, i) i ($0, r1) .r2'
	System::Call 'User32::MapWindowPoints(i, i, i, i) (0, $HWNDPARENT, r1, 2)'
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
  
	IntOp $4 $4 - $2 
	IntOp $5 $5 - $3

	System::Call `user32::CreateWindowExW(i 0, w "BUTTON", w "Advanced...", i ${WS_TABSTOP}|${WS_CHILD}, i 12, i r3, i r4 , i r5, i $HWNDPARENT, i 0, i 0, i 0) i.s`
	Pop $waui.button.advanced

	SendMessage $HWNDPARENT ${WM_GETFONT} 0 0 $0
	SendMessage $waui.button.advanced ${WM_SETFONT} $0 0 
FunctionEnd

Function UI_OnDirectoryPageLeave
	System::Call "user32::DestroyWindow(i) i ($waui.button.advanced) ."
	Call SetupWinampDirectories
  	${WINAMPDETECT_COMPONENTS_PRE}
FunctionEnd

Function UI_OnLicensePageShow
	System::Call "user32::DestroyWindow(i) i ($waui.button.advanced) ."
FunctionEnd

Function UI_OnLicensePageLeave
	${Xbundle_BeginCatalogDownload}
	${OPENCANDY_START}
FunctionEnd

Function UI_OnMouseOverSection
	Push $0
	Call GetSectionDescription
	Exch $0
	EnableWindow $mui.ComponentsPage.DescriptionText 1
    SendMessage $mui.ComponentsPage.DescriptionText ${WM_SETTEXT} 0 "STR:$0"
	Pop $0
FunctionEnd

Function UI_OnFinsihPageShow
!ifdef BUNDLE
	${MUSIC_BUNDLE_UPDATE_CHECKBOX} $mui.FinishPage.Run
!endif 
	${WINAMPDETECT_SHOW_REQUESTRESTART}
FunctionEnd

Function UI_OnFinishPageReadMe
	SetPluginUnload alwaysoff
	; Find Window info for the window we're displaying
	System::Call "*${stRECT} .r1"
	System::Call 'User32::GetWindowRect(i, i) i ($HWNDPARENT, r1) .r2'
	; Get left/top/right/bottom
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	WriteINIStr "$WINAMPINI" "SETUP" "left" $2
	WriteINIStr "$WINAMPINI" "SETUP" "top" $3
	WriteINIStr "$WINAMPINI" "SETUP" "right" $4
	WriteINIStr "$WINAMPINI" "SETUP" "bottom" $5

	;SetPluginUnload manual
	HideWindow
  
	Push $1
	StrCpy $1 1
	File "/oname=$PLUGINSDIR\ShellDispatch.dll" "${NSISDIR}\Plugins\ShellDispatch.dll"
	${If} ${FileExists} "$PLUGINSDIR\ShellDispatch.dll"
	${AndIf} ${FileExists} "$INSTDIR\${WINAMPEXE}"
		Push $0
		StrCpy $0 ""
		ClearErrors
		GetFullPathName /SHORT $0 "$PLUGINSDIR\ShellDispatch.dll"
		${IfNot} ${Errors}
		${AndIf} $0 != ""
			ExecWait 'rundll32.exe $0,RunDll_ShellExecute "open" "$INSTDIR\${WINAMPEXE}" "/NEW /REG=S" "$INSTDIR" 1' $1
			${If} ${Errors}
				StrCpy $1 1
			${EndIf}
		${EndIf}
		Pop $0
	${EndIf}
	
	${If} $1 != 0
		Exec "$INSTDIR\${WINAMPEXE} /NEW /REG=S"
	${EndIf}
	Pop $1
	
	
	Sleep 500
FunctionEnd

!ifdef BUNDLE
	Function UI_OnFinishPageRun
		${MUSIC_BUNDLE_SET_ACTIVE_PLAYLIST}
	FunctionEnd
!endif 

!ifdef BETA
	Function UI_OnWhatsNewPageShow
		Push $0
		GetDlgItem $0 $HWNDPARENT 2
		EnableWindow $0 0
		Pop $0
	FunctionEnd
!endif

!endif