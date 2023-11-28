!ifndef MUSICBUNDLE_HEADER
!define MUSICBUNDLE_HEADER
	!ifdef BUNDLE
		!include "..\musicrotation\config.nsh"
		;!define MUSICBUNDLE_LOCATION	"http://dl.dropbox.com/u/1994752/Rotation%20Test/free_mp3s.exe"
		!define MUSICBUNDLE_LOCATION 	"http://download.nullsoft.com/winamp/client/bundles/free_mp3s.exe"
	
		Var musicbundle.install.result


		Function MusicBundle_Init
		  StrCpy $musicbundle.install.result "skipped"
		FunctionEnd

		Function MusicBundle_GetPlaylistPath
			Push $1
			${If} $musicbundle.install.result == "success"
				ReadRegStr $1 ${ROTATION_INSTALLPATH_REGROOT} "${ROTATION_INSTALLPATH_REGKEY}" "${ROTATION_INSTALLPATH_REGVALUE}"
				${If} $1 == ""
					StrCpy $1 "${ROTATION_DEFAULTPATH}"
				${EndIf}
			
				${IfNot} ${FileExists} "$1\${ROTATION_PLAYLIST}"
					StrCpy $1 ""
				${EndIf}
			${Else}
				StrCpy $1 ""
			${EndIf}	
			Exch $1
		FunctionEnd
		
		!macro MusicBundle_GetPlaylistPath __outputVar
			Call MusicBundle_GetPlaylistPath
			Pop "${__outputVar}"
		!macroend
		!define MusicBundle_GetPlaylistPath "!insertmacro 'MusicBundle_GetPlaylistPath'"
		
		Function MusicBundle_UpdateCheckbox
			Exch $0
			Push $1
						
			${MusicBundle_GetPlaylistPath} $1
			
			${If} $1 != ""
				ShowWindow $0 ${SW_SHOW}
				SendMessage $0 ${BM_SETCHECK} ${BST_CHECKED} 0
			${Else}
				ShowWindow $0 ${SW_HIDE}
				SendMessage $0 ${BM_SETCHECK} ${BST_UNCHECKED} 0
			${EndIf}	
			
			Pop $1
			Exch $0
		FunctionEnd

		Function MusicBundle_SetActivePlaylist
			Push $0
			Push $1
			Push $2
						
			${MusicBundle_GetPlaylistPath} $1
			${If} $1 != ""
				FindFirst $0 $2 "$1\*.m3u8"
				FindClose $0
				${If} $2 != ""
					ClearErrors
					CopyFiles /SILENT /FILESONLY "$1\$2" "$PLUGINSDIR"
					${IfNot} ${Errors}
						Delete "$WINAMPM3U"
						ClearErrors
						Rename "$PLUGINSDIR\$2" "$WINAMPM3U"
					${EndIf}
					
				${EndIf}	
			${EndIf}
			
			Pop $2
			Pop $1
			Pop $0
		FunctionEnd

		!macro MUSIC_BUNDLE_INIT_MACRO	
			Call MusicBundle_Init
		!macroend

		!macro MUSIC_BUNDLE_SET_ACTIVE_PLAYLIST_MACRO
			Call MusicBundle_SetActivePlaylist
		!macroend

		!macro MUSIC_BUNDLE_UPDATE_CHECKBOX_MACRO	checkboxWindow
			Push "${checkboxWindow}"
			Call MusicBundle_UpdateCheckbox
		!macroend

		!macro MUSIC_BUNDLE_SECTION_MACRO
			${WinampSection} "musicBundle" $(IDS_SEC_MUSIC_BUNDLE) IDX_SEC_MUSIC_BUNDLE
				${SECTIONIN_STD}
		
				NSISdl::download /TRANSLATE2 "$(IDS_RUN_DOWNLOAD) $(IDS_SEC_MUSIC_BUNDLE)..."\
					"$(IDS_CONNECTING)" "$(IDS_SECOND)" "$(IDS_MINUTE)" "$(IDS_HOUR)" "$(IDS_SECONDS)" "$(IDS_MINUTES)"\
					"$(IDS_HOURS)" "$(IDS_PROGRESS)" /TIMEOUT=30000 "${MUSICBUNDLE_LOCATION}" "$PLUGINSDIR\musicbundle.exe"
					
				Pop $0 ; Get the return value
				${If} $0 == "success"
					DetailPrint "$(IDS_RUN_INSTALL) $(IDS_SEC_MUSIC_BUNDLE)..."
					SetDetailsPrint none
					
					SetPluginUnload alwaysoff
					System::Call "*${stRECT} .r1"
					System::Call 'User32::GetWindowRect(i, i) i ($HWNDPARENT, r1) .r2'
					; Get left/top/right/bottom
					System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
					System::Free $1
					SetPluginUnload manual
					
					ClearErrors
					ExecWait '"$PLUGINSDIR\musicbundle.exe" /WINAMP_PATH="$INSTDIR\${WINAMPEXE}" /EMBEDDED /LANGID=$LANGUAGE /CENTERRECT=$2,$3,$4,$5' $0
					Delete "$PLUGINSDIR\musicbundle.exe"
					
					SetDetailsPrint lastused
					${If} $0 <> 0
					${OrIf} ${Errors}
						StrCpy $musicbundle.install.result "error"
						DetailPrint "$(IDS_RUN_INSTALLFIALED)"
					${Else}
						StrCpy $musicbundle.install.result "success"
						DetailPrint "$(IDS_RUN_DONE)"
					${EndIf}
				${ElseIf} $0 == "cancel"
					StrCpy $musicbundle.install.result "cancel"
					DetailPrint "$(IDS_RUN_DOWNLOADCANCELLED)"
				${Else}
					StrCpy $musicbundle.install.result "error"
					DetailPrint "$(IDS_RUN_DOWNLOADFAILED) $(IDS_SEC_MUSIC_BUNDLE)"
				${EndIf}
			${WinampSectionEnd}
		!macroend

		!define MUSIC_BUNDLE_INIT					"!insertmacro MUSIC_BUNDLE_INIT_MACRO"
		!define MUSIC_BUNDLE_SET_ACTIVE_PLAYLIST	"!insertmacro MUSIC_BUNDLE_SET_ACTIVE_PLAYLIST_MACRO"
		!define MUSIC_BUNDLE_UPDATE_CHECKBOX		"!insertmacro MUSIC_BUNDLE_UPDATE_CHECKBOX_MACRO"
		!define MUSIC_BUNDLE_INSERT_SECTION				"!insertmacro MUSIC_BUNDLE_SECTION_MACRO"

	!else
		!define MUSIC_BUNDLE_INIT					
		!define MUSIC_BUNDLE_SET_ACTIVE_PLAYLIST	
		!define MUSIC_BUNDLE_UPDATE_CHECKBOX		
		!define MUSIC_BUNDLE_INSERT_SECTION				
	!endif
!endif
