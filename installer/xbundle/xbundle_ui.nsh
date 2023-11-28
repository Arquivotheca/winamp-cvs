!ifndef NULLSOFT_XBUNDLE_UI_HEADER
!define NULLSOFT_XBUNDLE_UI_HEADER

!ifndef XBUNDLE_DISABLED

!include "LogicLib.nsh"


!ifndef stRECT
	!define stRECT "(i, i, i, i) i"
!endif

!macro XBundleUI_DefineDefault __key __value
  !ifndef "${__key}"
	!define "${__key}_XBUNDLE_DEFAULT" 
    !define "${__key}" "${__value}"
  !endif
!macroend
!define XBundleUI_DefineDefault "!insertmacro 'XBundleUI_DefineDefault'"

!macro XBundleUI_Undefine __key
  !ifdef "${__key}"
    !undef "${__key}"
  !endif
!macroend
!define XBundleUI_Undefine "!insertmacro 'XBundleUI_Undefine'"

!macro XBundleUI_UndefineDefault __key
  !ifdef "${__key}_XBUNDLE_DEFAULT"
    ${XBundleUI_Undefine} "${__key}"
	!undef "${__key}_XBUNDLE_DEFAULT"
  !endif
!macroend
!define XBundleUI_UndefineDefault "!insertmacro 'XBundleUI_UndefineDefault'"

!macro XBundleUI_CallCustomFunction __functionType
  !ifdef XBUNDLE_PAGE_CUSTOMFUNCTION_${__functionType}
    Call "${XBUNDLE_PAGE_CUSTOMFUNCTION_${__functionType}}"
  !endif
!macroend
!define XBundleUI_CallCustomFunction "!insertmacro 'XBundleUI_CallCustomFunction'"

; Check list
${XBundleUI_DefineDefault} XBUNDLE_CLS_VCENTER		0x00000001
${XBundleUI_DefineDefault} XBUNDLE_CLS_BOLDNAME		0x00000002
${XBundleUI_DefineDefault} XBUNDLE_CLS_RADIOBUTTON	0x00000004
${XBundleUI_DefineDefault} XBUNDLE_CLS_COMPACT		0x00000008
${XBundleUI_DefineDefault} XBUNDLE_CLS_READONLY		0x00000010

${XBundleUI_DefineDefault} XBUNDLE_CLIS_DISABLED	0x00000001
${XBundleUI_DefineDefault} XBUNDLE_CLIS_CHECKED		0x00000002

${XBundleUI_DefineDefault} XBUNDLE_NCLM_GETITEMCOUNT	1124

!macro XBundle_InsertPage

	${XBundleUI_DefineDefault} XBUNDLE_PAGE_HEADER_TEXT "Get the Most Out of $(^NameDA)"
	${XBundleUI_DefineDefault} XBUNDLE_PAGE_HEADER_SUBTEXT "Choose the additional features below to get the most ot of $(^NameDA)"
	${XBundleUI_DefineDefault} XBUNDLE_PAGE_TEXT_TOP "Get the best $(^NameDA) experience with these additional features."
	
	PageEx custom
		PageCallbacks XBundle_OnPageCreate XBundle_OnPageLeave
	PageExEnd
	
	!insertmacro XBunndleUI_InsertPageCallbacks XBundle_OnPageCreate XBundle_OnPageLeave
	
	${XBundleUI_Undefine} XBUNDLE_PAGE_HEADER_TEXT
	${XBundleUI_Undefine} XBUNDLE_PAGE_HEADER_SUBTEXT
	${XBundleUI_Undefine} XBUNDLE_PAGE_TEXT_TOP
	${XBundleUI_Undefine} XBUNDLE_PAGE_CUSTOMFUNCTION_PRE
	${XBundleUI_Undefine} XBUNDLE_PAGE_CUSTOMFUNCTION_SHOW
	${XBundleUI_Undefine} XBUNDLE_PAGE_CUSTOMFUNCTION_LEAVE
	
!macroend
!define XBundle_InsertPage "!insertmacro 'XBundle_InsertPage'"

!macro XBundleList_OnCheckChanged
	Exch $0		; hwnd
	Exch 1	
	Exch $1		; item index
	Exch 2
	Exch $2		; checked
	Exch 3
	Exch $3		; item param
	Push $4
	
	ReadINIStr $4 "$xbundle.pagelog.path" "offer_$1" "name"
	${If} $4 != ""
		Push $5
		${If} $2 == 1
			StrCpy $5 "yes"
		${Else}
			StrCpy $5 "no"
		${EndIf}
		WriteINIStr "$xbundle.catalog.path" "$4" "selected" "$5"
		Pop $5
	${EndIf}
	
		
	Pop $4
	Pop $3
	Pop $0
	Pop $1
	Pop $2
!macroend

Function XBundleList_AddOffer
	Exch $0 ; offer_name
	Exch
	Exch $1 ; list handle
	Push $2 
	Push $3 
	Push $4
	Push $5
	Push $6
		
	ReadINIStr $2 "$xbundle.catalog.path" "$0" "error"
	ReadINIStr $3 "$xbundle.catalog.path" "$0" "status"
	${If} $2 != "OK"
		StrCpy $0 "OfferFailedToPrepare"
	${ElseIf} $3 != "ready"
		StrCpy $0 "OfferInBadState"
	${Else}		
		ReadINIStr $2 "$xbundle.catalog.path" "$0" "title"
		ReadINIStr $3 "$xbundle.catalog.path" "$0" "description"
		ReadINIStr $4 "$xbundle.catalog.path" "$0" "installer_size"
		${If} $4 != ""
		${AndIf} $4 > 0
			System::Call "shlwapi::StrFormatByteSizeW(l$4, w.r4, i${NSIS_MAX_STRLEN}) i.r5"
			;inetc::formatSizeInBytes $4
			;Pop $4
			${If} $5 == 0
				StrCpy $4 ""
			${Else}
				StrCpy $4 "($4)"
			${EndIf}
		${EndIf}
		ReadINIStr $5 "$xbundle.catalog.path" "$0" "selected"
		${If} $5 == ""
			ReadINIStr $5 "$xbundle.catalog.path" "$0" "selected_by_default"
			${If} $5 == ""
				StrCpy $5 "no"
				WriteINIStr "$xbundle.catalog.path" "$0" "selected_by_default" "$5"
			${EndIf}
			WriteINIStr "$xbundle.catalog.path" "$0" "selected" "$5"
		${EndIf}
		
		StrCpy $6 0
		${If} $5 == "yes"
			IntOp $6 $6 | ${XBUNDLE_CLIS_CHECKED}
		${EndIf}
		
		nsis_chklist::InsertItem /NOUNLOAD $1 1000000000 "$2" "$4" "$3" $6 0
		Pop $6
		${If} $6 <> -1
			StrCpy $6 "offer_$6"
			WriteINIStr "$xbundle.pagelog.path" "$6" "name" "$0"
			WriteINIStr "$xbundle.catalog.path" "$0" "presented" "yes"
			StrCpy $0 "OK"
		${Else}
			WriteINIStr "$xbundle.catalog.path" "$0" "status" "displaying"
			WriteINIStr "$xbundle.catalog.path" "$0" "error" "Error_UnableToDisplayOffer"
			StrCpy $0 "CheckListInsertFailed"
		${EndIf}
	${EndIf}
	
	Pop $6
	Pop $5
	Pop $4
	Pop $3
	Pop $2
	Pop $1
	Exch $0
FunctionEnd
!macro XBundleList_AddOffer __name __list __outputVar
	Push "${__list}"
	Push "${__name}"
	Call XBundleList_AddOffer
	Pop "${__outputVar}"
!macroend
!define XBundleList_AddOffer "!insertmacro 'XBundleList_AddOffer'"

Function XBundleList_Populate
	Exch $0 ; list control
	Push $1
	Push $2
	
	${XBundle_FinishCatalogDownload}
	
	ReadINIStr $1 "$xbundle.catalog.path" "catalog" "display_count"
	${XBundle_EnumerateOffers} "XBundleList_AddOffer" $0 $1 $2

	Pop $2
	Pop $1
	Pop $0	
FunctionEnd
!macro XBundleList_Populate __list
	Push "${__list}"
	Call XBundleList_Populate
!macroend
!define XBundleList_Populate "!insertmacro 'XBundleList_Populate'"

!macro XBunndleUI_InsertPageCallbacks  __createCallback __leaveCallback
Function "${__createCallback}"

	!define XBUNDLE_DIALOG		$R0
	!define XBUNDLE_LIST		$R1
	!define XBUNDLE_LIST_TOP	$R2
	
	Push $0
	${XBundle_IsOffersAvailable} $0
	${If} $0 == "no"
		Pop $0
		Abort
		Return
	${EndIf}
	Pop $0
	
	${XBundleUI_CallCustomFunction} PRE
	!insertmacro MUI_HEADER_TEXT "${XBUNDLE_PAGE_HEADER_TEXT}"  "${XBUNDLE_PAGE_HEADER_SUBTEXT}"
	
	nsDialogs::Create  /NOUNLOAD 1018
	Exch ${XBUNDLE_DIALOG}
	
	${If} ${XBUNDLE_DIALOG} == error
		Pop ${XBUNDLE_DIALOG}
		Abort
		Return
	${EndIf}

	Push ${XBUNDLE_LIST_TOP}
	Push $0
	Push $1
	
	StrCpy ${XBUNDLE_LIST_TOP} 0
	
	StrCpy $0 "${XBUNDLE_PAGE_TEXT_TOP}"
	${If} $0 != ""
		${NSD_CreateLabel} 0 0 100% 8u "$0"
		Pop $0
		${If} $0 != "error"
			nsis_chklist::AdjustStaticHeight $0 1 3
			Pop $0
			IntOp ${XBUNDLE_LIST_TOP} $0 + 13
		${EndIf}
	${EndIf}
	
	; Calculate check list height
	Push $2
	Push $3
	Push $4
	
	System::Call "*${stRECT} .r0"
	System::Call "User32::GetClientRect(i${XBUNDLE_DIALOG}, i r0)i .r1"
	System::Call "*$0${stRECT} (.r1, .r2, .r3, .r4)"
	System::Free $0

	IntOp $1 $4 - $2
	IntOp $1 $1 - ${XBUNDLE_LIST_TOP}
	IntOp $1 $1 - 4
	${If} $1 < 0
		StrCpy $1 0
	${EndIf}
	Pop $4
	Pop $3
	Pop $2

	; Set check list style
	StrCpy $0 0
	IntOp $0 ${XBUNDLE_CLS_BOLDNAME} | ${XBUNDLE_CLS_VCENTER}
		
	nsis_chklist::CreateControl /NOUNLOAD ${XBUNDLE_DIALOG} $0 0u ${XBUNDLE_LIST_TOP} 100% $1
	Exch ${XBUNDLE_LIST}
	${If} ${XBUNDLE_LIST} != "error"
	
		GetLabelAddress $0 ".XBundleList_OnCheckChanged"
		nsis_chklist::RegisterCallback /NOUNLOAD ${XBUNDLE_LIST} $0
		
		SendMessage ${XBUNDLE_LIST} ${WM_SETREDRAW} 0 0
		
		CreateFont $0 "Tahoma" "8" "400" 
		SendMessage ${XBUNDLE_LIST} ${WM_SETFONT} $0 0
		
		${XBundleList_Populate} ${XBUNDLE_LIST}
		
		SendMessage ${XBUNDLE_LIST} ${WM_SETREDRAW} 1 0
		
	${EndIf}
	
	${XBundleUI_CallCustomFunction} SHOW
	nsDialogs::Show ; not return until the user clicks Next, Back or Cancel.
	
	${If} ${XBUNDLE_LIST} != ""
		SendMessage ${XBUNDLE_LIST} ${WM_GETFONT} 0 0 $0
		System::Call 'gdi32::DeleteObject(i $0) i.s'
		Pop $0
	${EndIf}
	
	Pop $1
	Pop $0
	Pop ${XBUNDLE_LIST_TOP}
	Pop ${XBUNDLE_LIST}
	Pop ${XBUNDLE_DIALOG}
	
	!undef XBUNDLE_LIST
	!undef XBUNDLE_DIALOG
	
	Return
	
	!ifndef XBUNDLE_LIST_ONCHECKCHNAGED_DEFINED
		!define XBUNDLE_LIST_ONCHECKCHNAGED_DEFINED
		.XBundleList_OnCheckChanged:
			!insertmacro 'XBundleList_OnCheckChanged'
			Return
	!endif
FunctionEnd

Function "${__leaveCallback}"
	${XBundleUI_CallCustomFunction} LEAVE
FunctionEnd
!macroend


!else ; WINAMP_XBUNDLE_INCLUDE

!define XBundle_InsertPage ""
!endif 

!endif ;NULLSOFT_WINAMP_XBUNDLE_UI_HEADER