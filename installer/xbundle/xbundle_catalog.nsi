!include "fileFunc.nsh"
!include "wordFunc.nsh"
!include "logicLib.nsh"

!include "..\shared\scripts\locale.nsh"

;SetCompressor /SOLID lzma
SetCompress off

SetDatablockOptimize on
AutoCloseWindow true
ShowInstDetails nevershow
RequestExecutionLevel user
InstProgressFlags smooth

OutFile "xbundle_catalog.exe"
Name ""

Var xbundle.catalog.url
Var xbundle.catalog.path
Var xbundle.output.path
Var xbundle.offer.path
Var xbundle.journal.path
	
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

Function ProcessOffer
	Exch $0 ; offer_name
	Exch
	Exch $1 ; offer_index
	Push $2 
	Push $3 
	
	WriteINIStr "$xbundle.output.path" "$0" "offer_name" $0 
	WriteINIStr "$xbundle.output.path" "$0" "offer_index" $1
	
	ReadINIStr $2 "$xbundle.catalog.path" "$0" "url"
	${If} $2 == ""
		DeleteINISec "$xbundle.output.path" "$0" 
		StrCpy $0 "Error_BadOfferURL"
	${Else}
		WriteINIStr "$xbundle.output.path" "$0" "offer_url" $2
		
		ReadINIStr $2 "$xbundle.catalog.path" "$0" "selected"
		${If} $2 == ""
			StrCpy $2 "no"
		${EndIf}
		WriteINIStr "$xbundle.output.path" "$0" "selected_by_default" $2
		
		ReadINIStr $2 "$xbundle.catalog.path" "$0" "silent_mode_install"
		${If} $2 == ""
			StrCpy $2 "no"
		${EndIf}
		WriteINIStr "$xbundle.output.path" "$0" "silent_mode_install" $2
		
		WriteINIStr "$xbundle.output.path" "$0" "status" "advertised"
		WriteINIStr "$xbundle.output.path" "$0" "error" "OK"
		
		WriteINIStr "$xbundle.journal.path" "offer_$1" "name" $0
	
		StrCpy $3 ""
		${If} ${Silent}
			StrCpy $3 "/S $3"
		${EndIf}
		StrCpy $3 "$3 --language-id=$LANGUAGE"
		StrCpy $3 "$3 --offer-name=$0"
		StrCpy $3 "$3 --catalog-path=$\"$xbundle.output.path$\""
		ExecDos::exec /NOUNLOAD /ASYNC /TIMEOUT=5000 '"$xbundle.offer.path" $3' "" ""
		Pop $3
		${If} $3 <> 0
			WriteINIStr "$xbundle.journal.path" "offer_$1" "process_handle" $3
			StrCpy $0 "OK"
		${Else}
			WriteINIStr "$xbundle.output.path" "$0" "error" "Error_UnableToStartOfferCheck"
			StrCpy $0 "Error_UnableToStartOfferCheck"
		${EndIf}
	${EndIf}
	
	Pop $3
	Pop $2
	Pop $1
	Exch $0
FunctionEnd
!macro ProcessOffer __name __index __outputVar
	Push "${__index}"
	Push "${__name}"
	Call ProcessOffer
	Pop "${__outputVar}"
!macroend
!define ProcessOffer "!insertmacro 'ProcessOffer'"

Section -Main

	${GetParameters} $0
	
	${GetOptions} "$0" "--language-id=" $1
	${If} $1 != ""
		StrCpy $LANGUAGE $1
	${EndIf}
	
	${GetOptions} "$0" "--catalog-url=" $xbundle.catalog.url
	${GetOptions} "$0" "--output-path=" $xbundle.output.path

	${If} $xbundle.catalog.url == ""
		${ReportErrorAndQuit} 3 "Missing required parameter: catalog-url"
	${ElseIf} $xbundle.output.path == ""
		${ReportErrorAndQuit} 3 "Missing required parameter: output-path"
	${EndIf}
	
	FileOpen $0 "$xbundle.output.path" w
	FileWriteWord $0 "65279";"65534"
	FileClose $0

	InitPluginsDir
	
	StrCpy $xbundle.catalog.path "$PLUGINSDIR\xbundle.catalog"
	StrCpy $xbundle.journal.path "$PLUGINSDIR\xbundle.journal"
	
	!system '$\"${NSISDIR}\makensis.exe$\" /V2 /NOCD $\"xbundle_offer.nsi$\"' = 0
	
	StrCpy $xbundle.offer.path "$PLUGINSDIR\xbundle_offer.exe"
	File /oname=$xbundle.offer.path ".\xbundle_offer.exe"
	
	inetc::get /SILENT "$xbundle.catalog.url" "$xbundle.catalog.path" /end
	Pop $0
	
	${If} $0 != "OK"
		${ReportErrorAndQuit} 4 "Unable to download catalog (error=$0 url=$xbundle.catalog.url)"
	${EndIf}
		
	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	StrCpy $0 "$1/$0/$2 $4:$5:$6"
	WriteINIStr "$xbundle.output.path" "catalog" "create_started" "$0"
	WriteINIStr "$xbundle.output.path" "catalog" "create_finished" "$0"
	
	ReadINIStr $0 "$xbundle.catalog.path" "catalog" "display_count"
	WriteINIStr "$xbundle.output.path" "catalog" "display_count" "$0"
	
	${Locale_MakeLocaleID} $LANGUAGE 0 $0
	${Locale_GetLanguageName} $0 $1
	${Locale_GetCountryName} $0 $2
	WriteINIStr "$xbundle.output.path" "catalog" "language" "$1-$2"
	
	StrCpy $2 0 ;successfully processed offers 
	
	ReadINIStr $0 "$xbundle.catalog.path" "catalog" "offer_list"
 	StrLen $3 $0 ;length
	StrCpy $4 0  ;block
	
	${ForEach} $5 0 $3 + 1 ; cursor
		StrCpy $6 $0 1 $5 ; test char
		${If} $6 == " "
		${OrIf} $6 == ","
		${OrIf} $6 == ";"
		${OrIf} $6 == ""
			IntOp $7 $5 - $4 ; block len
			${If} $7 > 0
				StrCpy $6 $0 $7 $4 ; copy block in $6
				IntOp $2 $2 + 1
				${ProcessOffer} "$6" $2 $7 ; process results in $7
				${If} $7 != "OK"
					IntOp $2 $2 - 1
				${EndIf}
			${EndIf}
			IntOp $4 $5 + 1 ; set next block begin
		${EndIf}
	${Next}
	
	StrCpy $5 ""
	${ForEach} $0 1 $2 + 1
		ReadINIStr $1 "$xbundle.journal.path" "offer_$0" "process_handle"
		${If} $1 != ""
			ExecDos::wait /NOUNLOAD $1
			Pop $3 ; error level
			
			ReadINIStr $1 "$xbundle.journal.path" "offer_$0" "name"
			ReadINIStr $4 "$xbundle.output.path" "$1" "error"
			${If} $4 == "OK"
				${If} $3 <> 0
					WriteINIStr "$xbundle.output.path" "$1" "error" "Error_OfferDiscoveryFailed"
				${Else}
					WriteINIStr "$xbundle.output.path" "$1" "status" "ready"
					StrCpy $5 "$5$1 "
					WriteINIStr "$xbundle.output.path" "catalog" "offer_list" "$5"
				${EndIf}
			${EndIf}
		${EndIf}
	;	MessageBox MB_OK "finished checking index $0 of $2"
	${Next}
	WriteINIStr "$xbundle.output.path" "catalog" "offer_list" "$5"

	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	StrCpy $0 "$1/$0/$2 $4:$5:$6"
	WriteINIStr "$xbundle.output.path" "catalog" "create_finished" "$0"
	FlushINI "$xbundle.output.path"
SectionEnd