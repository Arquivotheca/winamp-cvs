!include "functions.nsh"

!ifmacrondef ROTATION_MAKE_LENGTH_HHMMSS
	!macro ROTATION_MAKE_LENGTH_HHMMSS
				
		!define ROTATION_LENGTH_TMP		${ROTATION_LENGTH_SEC}
		!define ROTATION_LENGTH_TMP2
		
		!if ${ROTATION_LENGTH_TMP} >= 3600
			!define /math ROTATION_LENGTH_PART_HH	${ROTATION_LENGTH_TMP} / 3600
			!undef ROTATION_LENGTH_TMP
			!define 	${ROTATION_LENGTH_TMP}
			!undef ROTATION_LENGTH_TMP
			!define /math ROTATION_LENGTH_TMP	${ROTATION_LENGTH_TMP2} % 60
		!endif
		
		!if ${ROTATION_LENGTH_TMP} >= 60
			!define /math ROTATION_LENGTH_PART_MM	${ROTATION_LENGTH_TMP} / 60
			!undef ROTATION_LENGTH_TMP2
			!define ROTATION_LENGTH_TMP2	${ROTATION_LENGTH_TMP}
			!undef ROTATION_LENGTH_TMP
			!define /math ROTATION_LENGTH_TMP	${ROTATION_LENGTH_TMP2} % 60
			
			!if ${ROTATION_LENGTH_PART_MM} < 10
				!undef ROTATION_LENGTH_TMP2
				!define ROTATION_LENGTH_TMP2	${ROTATION_LENGTH_PART_MM}
				!undef ROTATION_LENGTH_PART_MM
				!define ROTATION_LENGTH_PART_MM		"0${ROTATION_LENGTH_TMP2}"
			!endif
		!else 
			!define ROTATION_LENGTH_PART_MM	"00"
		!endif
		
		!define ROTATION_LENGTH_PART_SS	${ROTATION_LENGTH_TMP}
		!if ${ROTATION_LENGTH_PART_SS} < 10
			!undef ROTATION_LENGTH_TMP2
			!define ROTATION_LENGTH_TMP2	${ROTATION_LENGTH_PART_SS}
			!undef ROTATION_LENGTH_PART_SS
			!define ROTATION_LENGTH_PART_SS		"0${ROTATION_LENGTH_TMP2}"
		!endif
		
		!ifdef ROTATION_LENGTH_HHMMSS
			!undef ROTATION_LENGTH_HHMMSS
		!endif
		
		!ifdef ROTATION_LENGTH_PART_HH
			!define ROTATION_LENGTH_HHMMSS		"${ROTATION_LENGTH_PART_HH}.${ROTATION_LENGTH_PART_MM}:${ROTATION_LENGTH_PART_SS}"
		!else 
			!define ROTATION_LENGTH_HHMMSS		"${ROTATION_LENGTH_PART_MM}:${ROTATION_LENGTH_PART_SS}"
		!endif

		!ifdef ROTATION_LENGTH_PART_HH
			!undef ROTATION_LENGTH_PART_HH
		!endif
		
		!undef ROTATION_LENGTH_PART_MM
		!undef ROTATION_LENGTH_PART_SS
		
		!ifdef ROTATION_LENGTH_TMP
			!undef ROTATION_LENGTH_TMP
		!endif
		
		!ifdef ROTATION_LENGTH_TMP2
			!undef ROTATION_LENGTH_TMP2
		!endif
		
	!macroend
!endif

!ifmacrondef ROTATION_SECTION_ADD

	!macro ROTATION_SECTION_ADD
		
		!ifdef ROTATION_IDX
			!define /math ROTATION_IDX_TMP	${ROTATION_IDX} + 1
			!undef ROTATION_IDX
			!define ROTATION_IDX ${ROTATION_IDX_TMP}
			!undef ROTATION_IDX_TMP
		!else 
			!define  ROTATION_IDX	1
		!endif
		
		!insertmacro ROTATION_MAKE_LENGTH_HHMMSS
		
		!define ROTATION_SECTION_NAME "${ROTATION_ARTIST} - ${ROTATION_TITLE}   (${ROTATION_LENGTH_HHMMSS})"
		
		Section "${ROTATION_SECTION_NAME}" IDX_SEC_ROTATION${ROTATION_IDX}
			
			${NormalizePathPart} $0 "${ROTATION_ARTIST}"
			${NormalizePathPart} $2 "${ROTATION_ALBUM}"
			${NormalizePathPart} $3 "${ROTATION_TITLE}"
			${GetFileExtension} $1 "${ROTATION_FILE}"
						
			StrCpy $0 "$0\$2"
			StrCpy $1 "$0\$3.$1"
			
			ClearErrors
			${If} ${FileExists} "$INSTDIR\$1"
				DetailPrint "Song already exists ($INSTDIR\$1)"
				goto rotation_skip_file_writing
			${EndIf}
						
			ClearErrors
			CreateDirectory "$INSTDIR\$0"
			${If} ${Errors} 
				DetailPrint "Error: Unable to create folder ($INSTDIR\$0)"
				goto rotation_section_end_with_errror
			${EndIf}
			
		 	!define ROTATION_NEED_DOWNLOAD ""
			!searchparse /ignorecase /noerrors "${ROTATION_FILE}" "http://" ROTATION_NEED_DOWNLOAD
			!if "${ROTATION_NEED_DOWNLOAD}" != ""
				DetailPrint "Downloading song from ${ROTATION_FILE}"
				NSISdl::download /TRANSLATE2 "$(IDS_DOWNLOADING_COMPOSITION) ${ROTATION_ARTIST} - ${ROTATION_TITLE}..."\
					"$(IDS_CONNECTING)" "$(IDS_SECOND)" "$(IDS_MINUTE)" "$(IDS_HOUR)" "$(IDS_SECONDS)" "$(IDS_MINUTES)"\
					"$(IDS_HOURS)" "$(IDS_PROGRESS)" /TIMEOUT=30000 "${ROTATION_FILE}" "${ROTATION_INSTALLER_DLTMP}"
				Pop $3 ; Get the return value
				${If} $3 != "success"
					DetailPrint "Error: Download failed (${ROTATION_FILE})"
					goto rotation_section_end_with_errror
				${EndIf}
				
				ClearErrors
				Rename "${ROTATION_INSTALLER_DLTMP}" "$INSTDIR\$1"
				${If} ${Errors} 
					DetailPrint "Error: Copy file to destination failed ($INSTDIR\$1)"
					goto rotation_section_end_with_errror
				${EndIf}

			!else
				ClearErrors			
				File "/oname=$1" "${ROTATION_FILE}"
				${If} ${Errors} 
					DetailPrint "Error: Copy file to destination failed ($INSTDIR\$1)"
					goto rotation_section_end_with_errror
				${EndIf}

			!endif
			!undef ROTATION_NEED_DOWNLOAD
	
	rotation_skip_file_writing:
			!ifdef ROTATION_ART
				ClearErrors
				${If} ${FileExists} "$0\$2.jpg"
					DetailPrint "Album art aready exists ($0\$2.jpg)"
					goto rotation_skip_art_writing
				${EndIf}
						
				!define ROTATION_NEED_DOWNLOAD ""
				!searchparse /ignorecase /noerrors "${ROTATION_ART}" "http://" ROTATION_NEED_DOWNLOAD
				!if "${ROTATION_NEED_DOWNLOAD}" != ""
					DetailPrint "Downloading album art from ${ROTATION_ART}}"
					NSISdl::download /TRANSLATE2 "$(IDS_DOWNLOADING_ABLUMART) ${ROTATION_ARTIST} - ${ROTATION_TITLE}..."\
						"$(IDS_CONNECTING)" "$(IDS_SECOND)" "$(IDS_MINUTE)" "$(IDS_HOUR)" "$(IDS_SECONDS)" "$(IDS_MINUTES)"\
						"$(IDS_HOURS)" "$(IDS_PROGRESS)" "${ROTATION_ART}" "${ROTATION_INSTALLER_DLTMP}"
					Pop $3 ; Get the return value
					
					${If} $3 == "success"
						ClearErrors
						Rename "${ROTATION_INSTALLER_DLTMP}" "$INSTDIR\$0\$2.jpg"
						${If} ${Errors} 
							DetailPrint "Error: Copy file to destination failed ($INSTDIR\$0\$2.jpg)"
						${EndIf}
					${Else}
						DetailPrint "Error: Download failed (${ROTATION_ART})"
					${EndIf}
					
				!else	
					ClearErrors
					File "/oname=$0\$2.jpg" "${ROTATION_ART}"
					${If} ${Errors} 
						DetailPrint "Error: Copy file to destination failed ($INSTDIR\$0\$2.jpg)"
					${EndIf}
				!endif
				!undef ROTATION_NEED_DOWNLOAD
				
			!endif
	
!ifdef ROTATION_ART
	rotation_skip_art_writing:	
!endif		
			!ifndef SECTIONS_SIZEINFO
				!tempfile SECTIONS_SIZEINFO
			!endif
			!appendfile "${SECTIONS_SIZEINFO}" "SectionGetSize ${IDX_SEC_ROTATION${ROTATION_IDX}} $0$\r$\n"
			!ifdef ROTATION_FILESIZEKB
				!appendfile "${SECTIONS_SIZEINFO}" "IntOp $0 $0 + ${ROTATION_FILESIZEKB}$\r$\n"
			!endif	
			!ifdef ROTATION_ARTSIZEKB
				!appendfile "${SECTIONS_SIZEINFO}" "IntOp $0 $0 + ${ROTATION_ARTSIZEKB}$\r$\n"
			!endif	
			!appendfile "${SECTIONS_SIZEINFO}" "SectionSetSize ${IDX_SEC_ROTATION${ROTATION_IDX}} $0$\r$\n"
			
			FileOpen $3 "${ROTATION_INSTALLER_PLTMP}" a
			FileSeek $3 0 END $4
			IntCmpU $4 0 0 rotation_file_write_data rotation_file_write_data
				FileWriteWord $3 "65279"
				FileWriteUTF16LE $3 "#EXTM3U"
			
	rotation_file_write_data:
			FileWriteUTF16LE $3 "$\r$\n#EXTINF:${ROTATION_LENGTH_SEC},${ROTATION_ARTIST} - ${ROTATION_TITLE}$\r$\n$INSTDIR\$1"
			FileClose $3
			goto rotation_section_end
			
	rotation_section_end_with_errror:
			IntOp $musicbundle.errors.count $musicbundle.errors.count + 1
						
	rotation_section_end:
			Delete "${ROTATION_INSTALLER_DLTMP}"
		SectionEnd
			
				
		!ifndef SECTIONS_HELP
			!tempfile SECTIONS_HELP
		!endif
		!ifndef ROTATION_COMMENT
			!define ROTATION_COMMENT  ""
		!endif
		!appendfile "${SECTIONS_HELP}" "!insertmacro MUI_DESCRIPTION_TEXT ${IDX_SEC_ROTATION${ROTATION_IDX}}		$\"${ROTATION_COMMENT}$\"$\n"
		
		!undef ROTATION_SECTION_NAME
		!undef ROTATION_ARTIST
		!undef ROTATION_ALBUM
		!undef ROTATION_TITLE
		!undef ROTATION_LENGTH_HHMMSS
		!undef ROTATION_LENGTH_SEC
		!undef ROTATION_FILE
		!ifdef ROTATION_ART
			!undef ROTATION_ART
		!endif 
		!ifdef ROTATION_FILESIZEKB
			!undef ROTATION_FILESIZEKB
		!endif
		!ifdef ROTATION_ARTSIZEKB
			!undef ROTATION_ARTSIZEKB
		!endif 
		!undef ROTATION_COMMENT

	!macroend

!endif