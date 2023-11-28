!include "MUI2.nsh"
!include ".\fileFunc.nsh"
!include ".\wordFunc.nsh"
!include ".\logicLib.nsh"

!define ROTATION_INSTALLER_CONFIG
!include "config.nsh"

Var musicbundle.embedded.mode
Var musicbundle.playlist.created
Var musicbundle.errors.count
Var musicbundle.winamp.path


!ifdef ROTATION_DEBUG_MODE
	SetCompress off
	ShowInstDetails show
	AutoCloseWindow false
!else
	SetCompressor /FINAL /SOLID lzma
	SetDatablockOptimize on
	FileBufSize 64
	ShowInstDetails nevershow
	AutoCloseWindow true
!endif

RequestExecutionLevel user
InstProgressFlags smooth
XPStyle on
InstallDirRegKey ${ROTATION_INSTALLPATH_REGROOT} "${ROTATION_INSTALLPATH_REGKEY}" "${ROTATION_INSTALLPATH_REGVALUE}"


Name 	"$(IDS_INSTALLER_TITLE)"
OutFile "${ROTATION_INSTALLER_NAME}"
InstallDir "${ROTATION_DEFAULTPATH}"

Caption "$(^NameDA)"

!define MUI_LANGDLL_REGISTRY_ROOT 					${ROTATION_LANGUAGE_REGROOT}
!define MUI_LANGDLL_REGISTRY_KEY 					"${ROTATION_LANGUAGE_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME 				"${ROTATION_LANGUAGE_REGVALUE}"

!define MUI_CUSTOMFUNCTION_GUIINIT 					MusicBundle_OnGuiInit
!define MUI_CUSTOMFUNCTION_ABORT					MusicBundle_OnAbort
!define MUI_ICON 									".\resources\install.ico"
!define MUI_UNICON 									".\resources\uninstall.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP 						".\resources\header.bmp"


!define MUI_WELCOMEPAGE_TITLE		 				$(IDS_PAGE_WELCOME_TITLE) 
!define MUI_WELCOMEPAGE_TEXT						$(IDS_PAGE_WELCOME_TEXT) 
!define MUI_WELCOMEFINISHPAGE_BITMAP 				".\resources\welcome.bmp"
!define MUI_PAGE_CUSTOMFUNCTION_PRE					MusicBundle_OnWelcomePagePre
!insertmacro MUI_PAGE_WELCOME

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_PAGE_HEADER_TEXT						$(IDS_PAGE_COMPONENT_HEADER)
!define MUI_PAGE_HEADER_SUBTEXT						$(IDS_PAGE_COMPONENT_SUBHEADER)
!define MUI_COMPONENTSPAGE_TEXT_TOP					$(IDS_PAGE_COMPONENT_TOP) 
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST			$(IDS_PAGE_COMPONENT_LIST)
!define MUI_COMPONENTSPAGE_TEXT_DESCRIPTION_TITLE	$(IDS_PAGE_COMPONENT_DESC_TITLE)
!define MUI_COMPONENTSPAGE_TEXT_DESCRIPTION_INFO	$(IDS_PAGE_COMPONENT_DESC_INFO)
!define MUI_PAGE_CUSTOMFUNCTION_PRE					MusicBundle_OnComponentsPagePre
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_HEADER_TEXT						$(IDS_PAGE_DIRECTORY_HEADER)
!define MUI_PAGE_HEADER_SUBTEXT						$(IDS_PAGE_DIRECTORY_SUBHEADER)
!define MUI_DIRECTORYPAGE_TEXT_TOP					$(IDS_PAGE_DIRECTORY_TOP) 
!define MUI_PAGE_CUSTOMFUNCTION_PRE					MusicBundle_OnDirectoryPagePre
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!ifdef ROTATION_DEBUG_MODE
	!define MUI_FINISHPAGE_NOAUTOCLOSE
!endif

!define MUI_FINISHPAGE_TITLE						$(IDS_PAGE_FINISH_TITLE)
!define MUI_FINISHPAGE_TEXT_LARGE
!define MUI_FINISHPAGE_TEXT 						$(IDS_PAGE_FINISH_TEXT)
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_RUN_TEXT        				$(IDS_PAGE_FINISH_CREATESHORTCUT)
!define MUI_FINISHPAGE_RUN_FUNCTION     			FinishPage_CreateShortcut
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT		 		$(IDS_PAGE_FINISH_PLAYROTATION)
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION			FinishPage_PlayRotation
!define MUI_FINISHPAGE_LINK  						$(IDS_PAGE_FINISH_DISCOVERLINK)
!define MUI_FINISHPAGE_LINK_LOCATION 				"${ROTATION_INSTALLER_LINK}"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_PAGE_CUSTOMFUNCTION_PRE					MusicBundle_OnFinishPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW				MusicBundle_OnFinishPageShow
!insertmacro MUI_PAGE_FINISH

!include ".\languages.nsh"
ReserveFile "${NSISDIR}\Plugins\LangDLL.dll"


!define MUI_LANGDLL_WINDOWTITLE 					$(IDS_INSTALLER_LANG_TITLE)
!define MUI_LANGDLL_INFO 							$(IDS_INSTALLER_LANG_INFO)


!macro IfPlaylistCreatedOk
	${If} $musicbundle.playlist.created == "success"
	${AndIf} ${FileExists} "$INSTDIR\${ROTATION_PLAYLIST}"
!macroend 
!define IfPlaylistCreatedOk "!insertmacro 'IfPlaylistCreatedOk'"

!macro HideCheckbox __hwndCheckbox
	ShowWindow "${__hwndCheckbox}" ${SW_HIDE}
	SendMessage "${__hwndCheckbox}" ${BM_SETCHECK} ${BST_UNCHECKED} 0
!macroend 
!define HideCheckbox "!insertmacro 'HideCheckbox'"

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

!macro MUSICBUNDLE_EMBEDDED_MODE_SKIP
	StrCmp $musicbundle.embedded.mode "true" 0 +2
		Abort
!macroend
!define MUSICBUNDLE_EMBEDDED_MODE_SKIP "!insertmacro 'MUSICBUNDLE_EMBEDDED_MODE_SKIP'"
  
Section -BeginMusicCopy
	SetOutPath $INSTDIR
	SearchPath $0 "${ROTATION_INSTALLER_PLTMP}"
	Delete "$0"
	
	SetShellVarContext current
SectionEnd

!include "rotation.nsh"


Section -FinishMusicCopy
	
	Delete "$INSTDIR\${ROTATION_PLAYLIST}"
	SearchPath $0 "${ROTATION_INSTALLER_PLTMP}"
	
	${If} ${FileExists} "$0"
		unicode::FileUnicode2UTF8 "$0" "$INSTDIR\${ROTATION_PLAYLIST}" "AUTO"
		Pop $1
	${Else}
		StrCpy $1 "6"
	${EndIf}
	
	${If} $1 == "0"
		StrCpy $musicbundle.playlist.created "success"
	${Else}
		DetailPrint "$(IDS_DETAILS_CREATE_PLAYLIST_ERROR)"
	${EndIf}
	
	Delete "$0"
	
	${IfPlaylistCreatedOk}
		${If} $$musicbundle.winamp.path != ""
		${AndIf} ${FileExists} "$musicbundle.winamp.path"
			ClearErrors
			ExecWait '"$musicbundle.winamp.path" /CREATEPLAYLIST "$(IDS_WINAMP_PLAYLIST_NAME)" ${ROTATION_WINAMP_PLAYLIST_GUID}'
			${IfNot} ${Errors}
				ExecWait '"$musicbundle.winamp.path" /APPENDPLAYLIST ${ROTATION_WINAMP_PLAYLIST_GUID} "$INSTDIR\${ROTATION_PLAYLIST}"'
			${EndIf}
		${EndIf}
	${EndIf}
	
	ClearErrors
	ReadRegStr $0 ${ROTATION_INSTALLPATH_REGROOT} "${ROTATION_INSTALLPATH_REGKEY}" ""
	${IfNot} ${Errors}
		WriteRegStr ${ROTATION_INSTALLPATH_REGROOT} "${ROTATION_INSTALLPATH_REGKEY}" "${ROTATION_INSTALLPATH_REGVALUE}"	"$INSTDIR"
	${EndIf}
	
SectionEnd

!ifdef SECTIONS_HELP
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!include "${SECTIONS_HELP}"
	!insertmacro MUI_FUNCTION_DESCRIPTION_END
	!delfile "${SECTIONS_HELP}"
!endif 

Function .onInit
	InitPluginsDir
	
	StrCpy $musicbundle.playlist.created ""
	StrCpy $musicbundle.errors.count 0
	
	
	!ifdef SECTIONS_SIZEINFO
		!include "${SECTIONS_SIZEINFO}"
		!delfile "${SECTIONS_SIZEINFO}"
	!endif 
	
	
	${GetParameters} $0
	
	ClearErrors
	${GetOptions} "$0" "/EMBEDDED"  $1
	${If} ${Errors}
		StrCpy $musicbundle.embedded.mode "false"
	${Else}
		StrCpy $musicbundle.embedded.mode "true"
	${EndIf}
	
	ClearErrors
	${GetOptions} "$0" "/LANGID="  $1
	${IfNot} ${Errors}
	${AndIf} $1 != ""
		WriteRegStr ${ROTATION_LANGUAGE_REGROOT} "${ROTATION_LANGUAGE_REGKEY}" "${ROTATION_LANGUAGE_REGVALUE}" "$1"
		StrCpy $LANGUAGE $1
	${Else}
		ReadRegStr $1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" "LangId"
		${If} $1 != ""
			WriteRegStr ${ROTATION_LANGUAGE_REGROOT} "${ROTATION_LANGUAGE_REGKEY}" "${ROTATION_LANGUAGE_REGVALUE}" "$1"
		${EndIf}
		!insertmacro MUI_LANGDLL_DISPLAY
	${EndIf}
	
	ClearErrors
	${GetOptions} "$0" "/WINAMP_PATH=" $musicbundle.winamp.path
	${If} ${Errors}
		StrCpy $musicbundle.winamp.path ""
	${EndIf}	
			
	${If} $musicbundle.winamp.path == ""
		ClearErrors
		ReadRegStr $musicbundle.winamp.path HKCU "Software\Winamp" ""	
		${If} $musicbundle.winamp.path != ""
			StrCpy $musicbundle.winamp.path "$musicbundle.winamp.path\winamp.exe"
		${EndIf}
	${EndIf}
	
FunctionEnd

Function .onInstSuccess
	DeleteRegKey ${ROTATION_LANGUAGE_REGROOT} "${ROTATION_LANGUAGE_REGKEY}"
FunctionEnd

Function .onInstFailed
	DeleteRegKey ${ROTATION_LANGUAGE_REGROOT} "${ROTATION_LANGUAGE_REGKEY}"
FunctionEnd

Function MusicBundle_OnAbort
	DeleteRegKey ${ROTATION_LANGUAGE_REGROOT} "${ROTATION_LANGUAGE_REGKEY}"
FunctionEnd

!define SWP_NOSIZE          0x0001
!define SWP_NOZORDER        0x0004
!define DM_REPOSITION		0x402	; WM_USER + 2

Function MusicBundle_OnGuiInit
	${GetParameters} $0
	
	ClearErrors
	${GetOptions} "$0" "/CENTERRECT="  $1
	${IfNot} ${Errors}
		${WordFind} $1 "," "E+1" $2
		${WordFind} $1 "," "E+2" $3
		${WordFind} $1 "," "E+3" $4
		${WordFind} $1 "," "E+4" $5
		${IfNot} ${Errors}
			SetPluginUnload alwaysoff
			System::Call '*(i, i, i, i) i.s'
			Pop $1
			${If} $1 <> 0
				System::Call 'user32::GetWindowRect(i$HWNDPARENT, ir1)'
				System::Call '*$1(i, i, i, i) (.r6, .r7, .r8, .r9)'
				System::Free $1
				
				IntOp $4 $4 - $2
				IntOp $8 $8 - $6
				IntOp $4 $4 - $8
				IntOp $4 $4 / 2
				IntOp $2 $2 + $4
				
				IntOp $5 $5 - $3
				IntOp $9 $9 - $7
				IntOp $5 $5 - $9
				IntOp $5 $5 / 2
				IntOp $3 $3 + $5
				
				System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b ($HWNDPARENT, 0, $2, $3, 0, 0, ${SWP_NOZORDER}|${SWP_NOSIZE})"
				SendMessage $HWNDPARENT ${DM_REPOSITION} 0 0
				
			${EndIf}
			SetPluginUnload manual
		${EndIf}
	${EndIf}
FunctionEnd

Function FinishPage_PlayRotation
 
	ExecShell "open" "$INSTDIR\${ROTATION_PLAYLIST}"

FunctionEnd

Function FinishPage_CreateShortcut
	SetShellVarContext current
	SetOutPath "$INSTDIR"
	CreateShortCut "$DESKTOP\$(IDS_SHORTCUT_NAME).lnk" "$INSTDIR\${ROTATION_PLAYLIST}"
FunctionEnd


Function MusicBundle_OnWelcomePagePre
	${MUSICBUNDLE_EMBEDDED_MODE_SKIP}
FunctionEnd

Function MusicBundle_OnComponentsPagePre
	${MUSICBUNDLE_EMBEDDED_MODE_SKIP}
FunctionEnd

Function MusicBundle_OnDirectoryPagePre
	${MUSICBUNDLE_EMBEDDED_MODE_SKIP}
FunctionEnd

Function MusicBundle_OnFinishPagePre
	${MUSICBUNDLE_EMBEDDED_MODE_SKIP}
FunctionEnd

Function MusicBundle_OnFinishPageShow
	${IfPlaylistCreatedOk}
		
	${Else}
		${AppendWindowText} $mui.FinishPage.Text "$\r$\n$\r$\n$(IDS_PAGE_FINISH_PLAYLIST_ERROR)"
		${HideCheckbox}	$mui.FinishPage.Run
		${HideCheckbox}	$mui.FinishPage.ShowReadme
	${EndIf}

FunctionEnd