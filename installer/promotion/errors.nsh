!ifndef PROMOTION_ERRORS
!define PROMOTION_ERRORS

!define PROMOTION_ERROR_CREATE_FOLDER 		-1
!define PROMOTION_ERROR_CONFIG_EXTRACT 		-2
!define PROMOTION_ERROR_INSTALLER_EXTRACT 	-3
!define PROMOTION_ERROR_INSTALLER_DOWNLOAD 	-4
!define PROMOTION_ERROR_INSTALLER_START 	-5


!macro ExitWithError_Code2Str __errorCode __errorText
	IntCmp $0 ${__errorCode} 0 +3 +3
		StrCpy $1 "$(${__errorText})"
		goto promoerror_displaymessage
!macroend
!define ExitWithError_Code2Str "!insertmacro 'ExitWithError_Code2Str'"

Function ExitWithError
	Pop $0
	${ExitWithError_Code2Str} ${PROMOTION_ERROR_CREATE_FOLDER} IDS_MESSAGE_CREATE_TEMP_FAILED
	${ExitWithError_Code2Str} ${PROMOTION_ERROR_CONFIG_EXTRACT} IDS_MESSAGE_CONFIG_EXTRACT_FAILED
	${ExitWithError_Code2Str} ${PROMOTION_ERROR_INSTALLER_EXTRACT} IDS_MESSAGE_INSTALLER_EXTRACT_FAILED
	${ExitWithError_Code2Str} ${PROMOTION_ERROR_INSTALLER_DOWNLOAD} IDS_MESSAGE_INSTALLER_DOWNLOAD_FAILED
	${ExitWithError_Code2Str} ${PROMOTION_ERROR_INSTALLER_START} IDS_MESSAGE_INSTALLER_START_FAILED
	
promoerror_displaymessage:
	
	!ifdef PROMOERROR_CALLBACK
		Call ${PROMOERROR_CALLBACK}
	!endif
	
	SetDetailsPrint textonly
	DetailPrint  "$(IDS_DETAILS_FAILED)"
	SetDetailsPrint lastused
	MessageBox MB_OK|MB_ICONSTOP "$1" /SD IDOK
	SetErrorLevel $0
	Quit
FunctionEnd


!macro ExitWithError __errorCode
	Push "${__errorCode}"
	Call ExitWithError
!macroend
!define ExitWithError "!insertmacro 'ExitWithError'"

!macro IfErrorsExit	__errorCode
	!ifndef ERREXIT_JUMP_INDEX
		!define ERREXIT_JUMP_INDEX 0
	!else 
		!define ERREXIT_JUMP_INDEX_TMP ${ERREXIT_JUMP_INDEX}
		!undef ERREXIT_JUMP_INDEX
		!define /math ERREXIT_JUMP_INDEX ${ERREXIT_JUMP_INDEX_TMP} + 1
		!undef ERREXIT_JUMP_INDEX_TMP
	!endif
	IfErrors 0 errexit_jump_${ERREXIT_JUMP_INDEX}
		${ExitWithError} ${__errorCode}
	errexit_jump_${ERREXIT_JUMP_INDEX}:
!macroend
!define IfErrorsExit "!insertmacro 'IfErrorsExit'"

!endif

