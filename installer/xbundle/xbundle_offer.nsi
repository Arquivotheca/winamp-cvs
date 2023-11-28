!include "fileFunc.nsh"
!include "wordFunc.nsh"
!include "logicLib.nsh"
!include "..\shared\scripts\locale.nsh"

;SetCompressor /SOLID zlib
SetCompress off

SetDatablockOptimize on
AutoCloseWindow true
ShowInstDetails nevershow
RequestExecutionLevel user
InstProgressFlags smooth

OutFile "xbundle_offer.exe"
Name ""

; Qualifier errors
!define ERROR_OFFER_ALREADY_INSTALLED	3
!define ERROR_OFFER_NOT_APPLICABLE		4

Var xbundle.catalog.path
Var xbundle.offer.name
Var xbundle.offer.path
Var xbundle.installer.languageName
Var xbundle.installer.countryName

Function ReportError
	Exch $0
	Exch
	Exch $1
	MessageBox MB_OK|MB_ICONSTOP "Error ($0):$\r$\n$1" /SD IDOK
	SetErrorLevel "{__code}"
	Pop $1
	Pop $0
FunctionEnd
!macro ReportError __code __description
	Push "${__description}"
	Push "${__code}"
	Call ReportError
!macroend
!define ReportError "!insertmacro 'ReportError'"

!macro ReportErrorAndQuit __code __description
	${ReportError} "${__code}" "${__description}"
	${IF} "$PLUGINSDIR" != ""
		InetC::get
		ExecDos::exec
		RMDir /r "$PLUGINSDIR"
	${EndIf}
	Quit
!macroend
!define ReportErrorAndQuit "!insertmacro 'ReportErrorAndQuit'"

!macro ReadOfferStr __outputVar __key
	ReadINIStr "${__outputVar}" "$xbundle.catalog.path" "$xbundle.offer.name" "${__key}" 
!macroend
!define ReadOfferStr "!insertmacro 'ReadOfferStr'"

!macro WriteOfferStr __key __value
	WriteINIStr "$xbundle.catalog.path" "$xbundle.offer.name" "${__key}" "${__value}"
!macroend
!define WriteOfferStr "!insertmacro 'WriteOfferStr'"

!macro WriteOfferError __value
	${WriteOfferStr} "error" "${__value}"
!macroend
!define WriteOfferError "!insertmacro 'WriteOfferError'"

!macro WriteOfferStatus __value
	${WriteOfferStr} "status" "${__value}"
!macroend
!define WriteOfferStatus "!insertmacro 'WriteOfferStatus'"

Function ReadStrFromOfferFile
	Exch $0 ; key name
	Push $1	
	Push $2
	
	${ForEach} $1 0 2 + 1
		${Switch} $1
			${Case} 0
				StrCpy $2 "$xbundle.installer.languageName-$xbundle.installer.countryName"
				${Break}
			${Case} 1
				StrCpy $2 "$xbundle.installer.languageName"
				${Break}
			${Case} 2
				StrCpy $2 "offer"
				${Break}
		${EndSwitch}
		ReadINIStr $2 "$xbundle.offer.path" "$2" "$0"
		${If} $2 != ""
			${Break}
		${EndIf}
	${Next}
	
	StrCpy $0 $2
	Pop $2
	Pop $1
	Exch $0
FunctionEnd
!macro ReadStrFromOfferFile __outputVar __key
	Push "${__key}"
	Call ReadStrFromOfferFile
	Pop "${__outputVar}"
!macroend
!define ReadStrFromOfferFile "!insertmacro 'ReadStrFromOfferFile'"


Section -Main

	${GetParameters} $0
	
	${GetOptions} "$0" "--language-id=" $1
	${If} $1 != ""
		StrCpy $LANGUAGE $1
	${EndIf}
	
	${GetOptions} "$0" "--catalog-path=" $xbundle.catalog.path
	${GetOptions} "$0" "--offer-name=" $xbundle.offer.name

	${If} $xbundle.catalog.path == ""
		${ReportErrorAndQuit} 3 "Missing required parameter: catalog-path"
	${ElseIf} $xbundle.offer.name == ""
		${ReportErrorAndQuit} 3 "Missing required parameter: offer-name"
	${EndIf}
	
	InitPluginsDir

	${Locale_MakeLocaleID} $LANGUAGE 0 $0
	${Locale_GetLanguageName} $0 $xbundle.installer.languageName
	${Locale_GetCountryName} $0 $xbundle.installer.countryName
	
	${WriteOfferStatus} "fetching"

	StrCpy $xbundle.offer.path "$PLUGINSDIR\xbundle.offer"
	${ReadOfferStr} $0 "offer_url"
	inetc::get /NOUNLOAD /SILENT "$0" "$xbundle.offer.path" /end
	Pop $1
	${If} $1 != "OK"
		${WriteOfferError} "Error_UnableToDonwloadOffer"
		${ReportErrorAndQuit} 4 "Unable to download offer (error='$1' url=$0)"
	${EndIf}
	
	${WriteOfferStatus} "qualifying"
	${ReadStrFromOfferFile} $0 "qualifier_url"
	${WriteOfferStr} "qualifier_url" "$0"
	${If} $0 != ""
		StrCpy $1 "$PLUGINSDIR\xbundle_qualifier.exe"
		
		inetc::get /NOUNLOAD /SILENT "$0" "$1" /end
		Pop $2
		${If} $2 != "OK"
			${WriteOfferError} "Error_QualifierDownloadFailed"
			${ReportErrorAndQuit} 5 "Unable to download qualifier (error='$2' url=$1)"
		${EndIf}
		
		${ReadStrFromOfferFile} $0 "qualifier_param"
		${WriteOfferStr} "qualifier_param" "$\"$0$\""
		StrCpy $0 "--language-id=$LANGUAGE $0"
		${If} ${Silent}
			StrCpy $0 "/S $0"
		${EndIf}
		ExecDos::exec /TIMEOUT=2000 '"$1" $0' "" ""
		Pop $2
		${If} $2 <> 0
			${If} $2 == ${ERROR_OFFER_ALREADY_INSTALLED}
				StrCpy $0 "Error_OfferAlreadyInstalled"
			${ElseIf} $2 == ${ERROR_OFFER_NOT_APPLICABLE}
				StrCpy $0 "Error_OfferNotApplicable"
			${Else}
				StrCpy $0 "Error_QualifierCheckFailed"
			${EndIf}
			
			${WriteOfferError} "$0"
			${ReportErrorAndQuit} 6 "Qualifier check finished with errorcode $2"
		${EndIf}
	${EndIf}
	
	${ReadStrFromOfferFile} $0 "installer_url"
	${WriteOfferStr} "installer_url" "$0"
	
	${If} $0 == ""
		${WriteOfferError} "Error_OfferMissingInstaller"
		${ReportErrorAndQuit} 7 "Missing installer url"
	${EndIf}
	
	${WriteOfferStatus} "fetching"
	
	${ReadStrFromOfferFile} $0 "installer_param"
	${WriteOfferStr} "installer_param" "$\"$0$\""
	
	${ReadStrFromOfferFile} $0 "installer_size"
	${If} $0 != ""
		${WriteOfferStr} "installer_size" "$0"
	${EndIf}
	
	${ReadStrFromOfferFile} $0 "title"
	${WriteOfferStr} "title" "$\"$0$\""
	
	${ReadStrFromOfferFile} $0 "description"
	${WriteOfferStr} "description" "$\"$0$\""
	
	${ReadStrFromOfferFile} $0 "help_url"
	${If} $0 != ""
		${WriteOfferStr} "help_url" "$0"
	${EndIf}
	
	; get 'better' installer-size
	${ReadOfferStr} $0 "installer_url"
	StrCpy $1 "$PLUGINSDIR\header.txt"
	inetc::head /SILENT /CONNECTTIMEOUT 1 /RECEIVETIMEOUT 1 "$0" "$1" /end
	Pop $2
	${If} $2 == "OK"
		FileOpen $2 "$1" r
		${Do}
			ClearErrors
			FileReadUTF16LE $2 $1
			${If} ${Errors}
				${Break}
			${EndIf}
			StrLen $3 $1
			${If} $3 > 16
				StrCpy $0 $1 16 0
				${If} $0 == "Content-Length: "
					
					IntOp $3 $3 - 1
					StrCpy $0 $1 1 $3
					${If} $0 != "$\n"
						IntOp $3 $3 + 1
					${EndIf}
					
					IntOp $3 $3 - 1
					StrCpy $0 $1 1 $3
					${If} $0 != "$\r"
						IntOp $3 $3 + 1
					${EndIf}
					
					IntOp $3 $3 - 16
					StrCpy $0 $1 $3 16
					
					${WriteOfferStr} "installer_size" "$0"
					${Break}
				${EndIf}
			${EndIf}
		${Loop}
		FileClose $2
	${EndIf}
	${WriteOfferStatus} "ready"	
SectionEnd