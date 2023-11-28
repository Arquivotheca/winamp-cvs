!include "config.nsh"
!include "fileFunc.nsh"
!include "wordFunc.nsh"
!include "languages.nsh"

!define PROMOERROR_CALLBACK		Promotion_OnError
!include "errors.nsh"

Var promotion.data.path

SetCompressor /SOLID zlib
SetCompress force

SetDatablockOptimize on
AutoCloseWindow true
ShowInstDetails nevershow
RequestExecutionLevel user
InstProgressFlags smooth
XPStyle on

Icon "${PROMOTION_ICON_PATH}"
ChangeUI all "${PROMOTION_UI_PATH}"

OutFile "${PROMOTION_OUTPUT_FILE}"
Name "$(IDS_INSTALLER_NAME)"


!macro SetWindowText __hwnd __text
	SendMessage "${__hwnd}" ${WM_SETTEXT} 0 "STR:${__text}"
!macroend 
!define WM_SETTEXT 0x000C 
!define SetWindowText "!insertmacro 'SetWindowText'"

!macro SetOperationText __text
	SetDetailsPrint textonly
	DetailPrint  "${__text}"
	SetDetailsPrint none
!macroend
!define SetOperationText "!insertmacro 'SetOperationText'"


Function Promotion_OnError
	StrCmp $promotion.data.path "" +2 0
		RMDir /r "$promotion.data.path"
FunctionEnd

Function .onInit

	StrCpy $promotion.data.path ""
	
	${GetParameters} $1
	${GetOptions} "$1" "/remove" $0
	IfErrors installer_continue_init
	
	SetSilent silent
	${StrFilter} "$0" "" "" " =$\"" $1
	StrCpy $0 0
	
	ClearErrors
	SetOutPath $TEMP
	SetFileAttributes "$1" NORMAL
	
	installer_attempt_delete:
		${SetOperationText}	"$(IDS_DETAILS_DELETE_TEMP_FOLDER)"
				
		ClearErrors
		
		RMDir /r "$1"
		IfErrors 0 installer_exit
		
		IntOp $0 $0 + 1
		IntCmp $0 100 installer_exit 0 installer_exit
		
		Sleep 100
		Goto installer_attempt_delete
	
	installer_exit:
		Quit
		
	installer_continue_init:
	
FunctionEnd


Section -Main
	${SetWindowText} $HWNDPARENT "$(^NameDA)"
	
	${SetOperationText} "$(IDS_DETAILS_CREATE_TEMP_FOLDER)"
	GetTempFileName $promotion.data.path
	Delete "$promotion.data.path"
	
	ClearErrors
	CreateDirectory "$promotion.data.path"
	${IfErrorsExit} ${PROMOTION_ERROR_CREATE_FOLDER}

	${SetOperationText} "$(IDS_DETAILS_EXTRACT_CONFIG)"
	ClearErrors
	File "/oname=$promotion.data.path\promotion.ini" "${PROMOTION_CONFIG_PATH}"
	WriteINIStr "$promotion.data.path\promotion.ini" "Installer" "invokeComplete" "$EXEPATH /remove=$\"$promotion.data.path$\""
	${IfErrorsExit} ${PROMOTION_ERROR_CONFIG_EXTRACT}
		
		
	!define PROMOTION_NEED_DOWNLOAD ""
	!searchparse /ignorecase /noerrors "${PROMOTION_INSTALLER_PATH}" "http://" PROMOTION_NEED_DOWNLOAD
	!if "${PROMOTION_NEED_DOWNLOAD}" != ""
		${SetOperationText} "$(IDS_DETAILS_DOWNLOAD_INSTALLER)"
		ClearErrors
		NSISdl::download_quiet /TIMEOUT=30000 "${PROMOTION_INSTALLER_PATH}" "$promotion.data.path\wasetup.exe"
		Pop $1 ; Get the return value
		StrCmp $1 "success" +2 0
			SetErrors
		${IfErrorsExit} ${PROMOTION_ERROR_INSTALLER_DOWNLOAD}
	!else
		${SetOperationText} "$(IDS_DETAILS_EXTRACT_INSTALLER)"
		ClearErrors
		File "/oname=$promotion.data.path\wasetup.exe" "${PROMOTION_INSTALLER_PATH}"
		${IfErrorsExit} ${PROMOTION_ERROR_INSTALLER_EXTRACT}
	!endif
	
	${SetOperationText} "$(IDS_DETAILS_START_INSTALLER)"
	ClearErrors
	ExecShell "open" "$promotion.data.path\wasetup.exe" "/promotion=$\"$promotion.data.path\promotion.ini$\""
	${IfErrorsExit} ${PROMOTION_ERROR_INSTALLER_START} 
	
SectionEnd