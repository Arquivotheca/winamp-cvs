!ifndef NULLSOFT_WINAMP_INSTALLER_HELPER_FUNCTIONS_HEADER
!define NULLSOFT_WINAMP_INSTALLER_HELPER_FUNCTIONS_HEADER


!macro CloseProgram UNIQUEBS CLASSNAME
  Push $R0
  Push $R1
  StrCpy $R1 0
  ${UNIQUEBS}loop:
    FindWindow $R0 "${CLASSNAME}"
    IntCmp $R0 0 ${UNIQUEBS}done
      SendMessage $R0 ${WM_CLOSE} 0 0
      Sleep 250
      IsWindow $R0 "" ${UNIQUEBS}loop
      SendMessage $R0 ${WM_DESTROY} 0 0
      Sleep 250
      IntOp $R1 $R1 + 1
    IntCmp $R1 20 "" ${UNIQUEBS}loop 

  ${UNIQUEBS}done: 
  Pop $R1
  Pop $R0 
!macroend

!macro PromptCloseProgram UNIQUEBS CLASSNAME
  Push $R0
  ${UNIQUEBS}loop:
    FindWindow $R0 "${CLASSNAME}"
    IntCmp $R0 0 ${UNIQUEBS}done
      MessageBox MB_DEFBUTTON2|MB_ABORTRETRYIGNORE $(msgCloseWinamp) IDIGNORE ${UNIQUEBS}done IDRETRY ${UNIQUEBS}skipabort
       Abort $(msgInstallAborted)
    ${UNIQUEBS}skipabort:
    Goto ${UNIQUEBS}loop 
  ${UNIQUEBS}done: 
  
  Pop $R0 
!macroend

!macro OutputDebugString STRING
!ifdef _DEBUG
System::Call "Kernel32::OutputDebugStringW(w s)" `${string}`
!endif
!macroend
!define OutputDebugString "!insertmacro OutputDebugString"
;${OutputDebugString} "com2_show1"

!define stRECT "(i, i, i, i) i"

Function SetVisPluginDir
  Push $R0
  SetOutPath $INSTDIR\Plugins
  ReadINIStr $R0 "$WINAMPINI" "Winamp" "VISDIR"
  StrCmp $R0 "" NoVisDirCh
    SetOutPath $R0
  NoVisDirCh:
  Pop $R0
FunctionEnd

Function GetSkinDir
  Push $R0
  ReadINIStr $R0 "$WINAMPINI" "Winamp" "SKINDIR"
  StrCmp $R0 "" "" NoSkinCh
    StrCpy $R0 $INSTDIR\Skins
  NoSkinCh:
  Exch $R0
FunctionEnd

# return "online" if success
Function ConnectInternet
  ClearErrors
  Dialer::AttemptConnect
  IfErrors 0 +2
    Push "noexplorer"
  ClearErrors
FunctionEnd

# return "online" if success
Function IsInternetAvailable
  nsExec::Exec "ping -n 1 -w 400 www.google.com"
  Pop $0 # $0 is zero if ping was successful
  ${If} $0 <> 0
    nsExec::Exec "ping -n 1 -w 400 www.yahoo.com"
    Pop $0 
  ${EndIf}
  
  ${If} $0 == 0
    StrCpy $0 "online"
  ${Else}
    StrCpy $0 "no connection"
  ${EndIf}
  Push $0
FunctionEnd

Function SetupWinampDirectories
  ${If} $PREVINSTINI != "$INSTDIR\install.ini"
     StrCpy $PREVINSTINI  "$INSTDIR\install.ini"
     ${If} ${FileExists} "$PREVINSTINI"
       CopyFiles /SILENT "$PREVINSTINI" "$INSTINI"
       InstTypeSetText $IDX_INSTTYPE_PREVIUOS $(installPrevious)
       Call ReadSections
       SetCurInstType $IDX_INSTTYPE_PREVIUOS
     ${Else}
       Delete "$INSTINI"
       InstTypeSetText $IDX_INSTTYPE_PREVIUOS ""
       SetCurInstType $IDX_INSTTYPE_FULL
    ${EndIf}
    Call .onSelChange
  ${EndIf}
  
  SetPluginUnload alwaysoff
  ${If} ${FileExists} "$INSTDIR\paths.ini"
    Push $INSTDIR
    nsis_winamp::ReadSettings
    nsis_winamp::GetIniPath
    Pop $SETTINGSDIR
    nsis_winamp::GetIniFile
    Pop $WINAMPINI
    nsis_winamp::GetM3uPath
    Pop $WINAMPM3U
    nsis_winamp::GetM3uBase
    Pop $M3UBASEDIR

    ${If} $WINAMPINI == ""
      StrCpy $WINAMPINI "winamp.ini"
    ${EndIf}

  ${Else}
    StrCpy $SETTINGSDIR ""
    StrCpy $WINAMPINI "winamp.ini"
  ${EndIf}

  ${If} $SETTINGSDIR == ""
    ${If} ${FileExists} "$INSTDIR\${WINAMPEXE}"
      StrCpy $SETTINGSDIR $INSTDIR
    ${Else}
      SetShellVarContext current
      Call GetWinampFolder
      Pop $R0
      StrCpy $SETTINGSDIR "$APPDATA\$R0"
    ${EndIf}
  ${EndIf}
  
  ${If} $WINAMPM3U == ""
    StrCpy $WINAMPM3U "$SETTINGSDIR"
  ${EndIf}
  
  ${If} $M3UBASEDIR == ""
    StrCpy $M3UBASEDIR "$WINAMPM3U"
  ${EndIf}

  Push $SETTINGSDIR
  Push $WINAMPINI
  nsis_winamp::GetFullPath
  Pop $WINAMPINI
  
  Push $WINAMPM3U
  Push "winamp.m3u8"
  nsis_winamp::GetFullPath
  Pop $WINAMPM3U

  SetPluginUnload manual
FunctionEnd

!macro FileIfExist	__destPath __sourcePath
	!ifdef FILECHKTMP
		!undef FILECHKTMP
	!endif	
	!tempfile FILECHKTMP
	!system 'if exist "${__sourcePath}" (echo !define FILECHK_SOURCE_FILE_FOUND > "${FILECHKTMP}") \
			else (echo !define FILECHK_SOURCE_FILE_NOT_FOUND > "${FILECHKTMP}")'
	!include "${FILECHKTMP}"
	!ifdef FILECHK_SOURCE_FILE_FOUND
		!undef FILECHK_SOURCE_FILE_FOUND
		File "/oname=${__destPath}" "${__sourcePath}"
	!endif
	
	!ifdef FILECHK_SOURCE_FILE_NOT_FOUND
		!undef FILECHK_SOURCE_FILE_NOT_FOUND
	!endif
	!delfile "${FILECHKTMP}"
	!undef FILECHKTMP
!macroend
!define FileIfExist "!insertmacro 'FileIfExist'"

!macro SetWindowText __hwnd __text
	SendMessage "${__hwnd}" ${WM_SETTEXT} 0 "STR:${__text}"
!macroend 
!define SetWindowText "!insertmacro 'SetWindowText'"

!macro AppendWindowText __hwnd __text
	Push $0
	System::Call "user32::GetWindowText(i${__hwnd}, w.r0, i${NSIS_MAX_STRLEN})"
	StrCpy $0 "$0${__text}"
	SendMessage "${__hwnd}" ${WM_SETTEXT} 0 "STR:$0"
	Pop $0
!macroend 
!define AppendWindowText "!insertmacro 'AppendWindowText'"

!macro NextButton_SetLastPageModeInternal
	Exch $0
	Push $1
	${If} $0 != "true"
		StrCpy $1 $(^NextBtn)
	${Else}
		!ifndef __UNINSTALL__
			StrCpy $1 $(^InstallBtn)
		!else
			StrCpy $1 $(^UninstallBtn)
		!endif
	${EndIf}

	GetDlgItem $0 $HWNDPARENT 1
	${SetWindowText} $0 $1
	Pop $1
	Pop $0
!macroend

!macro NextButton_SetLastPageMode __enableMode
	Push ${__enableMode}
	${CallArtificialFunction} NextButton_SetLastPageModeInternal
!macroend

!macro NextButton_SetNextPageMode __enableMode
	${If} "${__enableMode}" == "true"
		Push "false"
	${Else}
		Push "true"
	${EndIf}
	${CallArtificialFunction} NextButton_SetLastPageModeInternal
!macroend

!define NextButton_SetLastPageMode "!insertmacro 'NextButton_SetLastPageMode'"
!define NextButton_SetNextPageMode "!insertmacro 'NextButton_SetNextPageMode'"

!macro Path_RemoveArgsInternal
	Exch $0
	Push $1
	Push $2
	Push $3
	
	StrCpy $3 ""
	StrCpy $1 0
	${Do}
		StrCpy $2 $0 1 $1
		${If} $1 == ""
			StrLen $1 $0
			${Break}
		${ElseIf} $2 == "$\""
			${If} $1 == 0
				StrCpy $3 "inQuote"
			${Else}
				IntOp $1 $1 + 1
				${Break}
			${EndIf}
		${ElseIf} $2 == " "
			${If} $3 != "inQuote"
				${Break}
			${EndIf}
		${EndIf}
		IntOp $1 $1 + 1
	${Loop}
	
	StrCpy $0 $0 $1 0
	
	Pop $3
	Pop $2
	Pop $1
	Exch $0
!macroend
!macro Path_RemoveArgs __path __outputVar
	Push "${__path}"
	${CallArtificialFunction} Path_RemoveArgsInternal
	Pop "${__outputVar}"
!macroend
!define Path_RemoveArgs "!insertmacro 'Path_RemoveArgs'"

!macro Path_UnquoteSpacesInternal
	Exch $0
	Push $1
	Push $2
	
	StrLen $1 $0
	${If} $1 > 1
		IntOp $1 $1 - 1
		StrCpy $2 $0 1 $1
		${If} $2 == "$\""
		${OrIf} $2 == "'"
			StrCpy $2 $1
		${Else}
			IntOp $2 $1 + 1
		${EndIf}
	${Else}
		StrCpy $2 $1
	${EndIf}
				
	StrCpy $1 $0 1 0
	${If} $1 == "$\""
	${OrIf} $1 == "'"
		StrCpy $1 1
	${Else}
		StrCpy $1 0
	${EndIf}
	
	IntOp $2 $2 - $1
	StrCpy $0 $0 $2 $1
	
	Pop $2
	Pop $1
	Exch $0
!macroend
!macro Path_UnquoteSpaces __path __outputVar
	Push "${__path}"
	${CallArtificialFunction} Path_UnquoteSpacesInternal
	Pop "${__outputVar}"
!macroend
!define Path_UnquoteSpaces "!insertmacro 'Path_UnquoteSpaces'"

!macro Path_RemoveBlanksInternal
	Exch $0
	Push $1
	Push $2
		
	StrLen $1 $0
	${If} $1 > 1
		${Do}
			IntOp $1 $1 - 1
			StrCpy $2 $0 1 $1
			${If} $2 != " "
				IntOp $1 $1 + 1
				StrCpy $2 $1
				${Break}
			${OrIf}	$1 == 0
				StrCpy $2 0
				${Break}
			${EndIf}
		${Loop}	
	${Else}
		StrCpy $2 $1
	${EndIf}
			
	${If} $2 > 0
		Push $3
		StrCpy $3 0
		${Do}
			StrCpy $1 $0 1 $3
			${If} $1 != " "
				StrCpy $1 $3
				${Break}
			${EndIf}
			IntOp $3 $3 + 1
		${Loop}
		Pop $3
	${Else}
		StrCpy $1 0
	${EndIf}	
	
	IntOp $2 $2 - $1
	StrCpy $0 $0 $2 $1
		
	Pop $2
	Pop $1
	Exch $0
!macroend
!macro Path_RemoveBlanks __path __outputVar
	Push "${__path}"
	${CallArtificialFunction} Path_RemoveBlanksInternal
	Pop "${__outputVar}"
!macroend
!define Path_RemoveBlanks "!insertmacro 'Path_RemoveBlanks'"

!macro OffsetWindowPos __hwnd __offset_x __offset_y
	Push $R0
	Push $R1
	Push $R2
	Push $R3
	Push $R4
	
	System::Call "*${stRECT} .R4"
	System::Call "User32::GetWindowRect(i, i) i (${__hwnd}, R4) .R0"
	System::Call "User32::GetAncestor(i, i) i (${__hwnd}, ${GA_PARENT}) .R0"
	${If} $R0 != 0
		System::Call 'User32::MapWindowPoints(i, i, i, i) (0, R0, R4, 2)'
	${EndIf}
	System::Call "*$R4${stRECT} (.R0, .R1, .R2, .R3)"
	System::Free $R4
	
	IntOp $R0 $R0 + ${__offset_x}
	IntOp $R1 $R1 + ${__offset_y}

	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b (${__hwnd}, 0,  $R0, $R1, 0, 0, ${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOSIZE})"
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0
!macroend
!define OffsetWindowPos "!insertmacro 'OffsetWindowPos'"

!macro IncreaseWindowSize __hwnd __delta_cx __delta_cy
	Push $R0
	Push $R1
	Push $R2
	Push $R3
	Push $R4
	
	System::Call "*${stRECT} .R4"
	System::Call "User32::GetWindowRect(i, i) i (${__hwnd}, R4) .R0"
	System::Call "*$R4${stRECT} (.R0, .R1, .R2, .R3)"
	System::Free $R4

	IntOp $R0 $R2 - $R0
	IntOp $R1 $R3 - $R1
	IntOp $R0 $R0 + ${__delta_cx}
	IntOp $R1 $R1 + ${__delta_cy}

	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b (${__hwnd}, 0, 0, 0, $R0, $R1, ${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOMOVE})"
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0
!macroend
!define IncreaseWindowSize "!insertmacro 'IncreaseWindowSize'"

!endif ;NULLSOFT_WINAMP_INSTALLER_HELPER_FUNCTIONS_HEADER
