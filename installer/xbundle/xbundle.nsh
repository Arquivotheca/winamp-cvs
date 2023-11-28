!ifndef NULLSOFT_XBUNDLE_HEADER
!define NULLSOFT_XBUNDLE_HEADER

!ifndef XBUNDLE_DISABLED

!ifdef XBUNDLE_SOURCE_PATH_CD
!addincludedir "${XBUNDLE_SOURCE_PATH_CD}"
!endif


!include "LogicLib.nsh"

Var xbundle.tool.path
Var xbundle.tool.timeout
Var xbundle.tool.downloadHandle
Var xbundle.catalog.url
Var xbundle.catalog.path
Var xbundle.catalog.state
Var xbundle.pagelog.path
Var xbundle.initialized

Function XBundle_ResetCatalog
	${If} $xbundle.tool.downloadHandle <> 0
		ExecDos::wait /NOUNLOAD $xbundle.tool.downloadHandle
		Pop $xbundle.tool.downloadHandle
		StrCpy  $xbundle.tool.downloadHandle 0
	${EndIf}
	${If} $xbundle.catalog.path == ""
		StrCpy $xbundle.catalog.path "$PLUGINSDIR\xbundle_catalog.ini"
	${Else}
		Delete "$xbundle.catalog.path"
	${EndIf}
	
	${If} $xbundle.pagelog.path == ""
		StrCpy $xbundle.pagelog.path "$PLUGINSDIR\xbundle_pagelog.ini"
	${Else}
		Delete "$xbundle.pagelog.path"
	${EndIf}
	StrCpy $xbundle.catalog.state "initialized"
FunctionEnd
!macro XBundle_ResetCatalog
	Call XBundle_ResetCatalog
!macroend
!define XBundle_ResetCatalog "!insertmacro 'XBundle_ResetCatalog'"

!macro XBundle_GetCatalogState __outputVar
	StrCpy ${__outputVar} "$xbundle.catalog.state"
!macroend
!define XBundle_GetCatalogState "!insertmacro 'XBundle_GetCatalogState'"

Function XBundle_Init
	Exch $0
	
	${If} "$0" != "$xbundle.catalog.url"
		StrCpy $xbundle.initialized  "no"
		StrCpy $xbundle.catalog.url "$0"
		${If} "$xbundle.catalog.url" == ""
			SetErrors
		${EndIf}
	${EndIf}
	Pop $0
	
	${If} $xbundle.initialized == "yes"
		Return
	${EndIf}
	
	StrCpy $xbundle.initialized  "yes"
	
	${If} $PLUGINSDIR == ""
	${OrIfNot} ${FileExists} "$PLUGINSDIR"
		InitPluginsDir
	${EndIf}
	
	!ifdef XBUNDLE_SOURCE_PATH_CD
		!cd "${XBUNDLE_SOURCE_PATH_CD}"
	!endif
	
	!system '$\"${NSISDIR}\makensis.exe$\" /V2 /NOCD $\".\xbundle_catalog.nsi$\"' = 0
		
	StrCpy $xbundle.tool.path "$PLUGINSDIR\xbundle_catalog.exe"
	${IfNot} ${FileExists} "$xbundle.tool.path"
		File /oname=$xbundle.tool.path ".\xbundle_catalog.exe"
	${EndIf}
	
	!ifdef XBUNDLE_HOST_PATH_CD
		!cd "${XBUNDLE_HOST_PATH_CD}"
	!endif
	
	StrCpy $xbundle.tool.timeout 5000
	${XBundle_ResetCatalog}
FunctionEnd
!macro XBundle_Init __catalogURL
	Push "${__catalogURL}"
	Call XBundle_Init
!macroend
!define XBundle_Init "!insertmacro 'XBundle_Init'"

Function XBundle_Finish
FunctionEnd
!macro XBundle_Finish
	Call XBundle_Finish
!macroend
!define XBundle_Finish "!insertmacro 'XBundle_Finish'"

Function XBundle_BeginCatalogDownload
	
	${If} $xbundle.catalog.state != "initialized"
		Return
	${EndIf}	

	${If} $xbundle.tool.path == ""
	${OrIf} $xbundle.catalog.path == ""
	${OrIf} $xbundle.tool.downloadHandle <> 0
		SetErrors
		Return
	${EndIf}
	
	Push $0
	StrCpy $0 "/S"
	StrCpy $0 "$0 --language-id=$LANGUAGE"
	StrCpy $0 "$0 --catalog-url=$\"$xbundle.catalog.url$\""
	StrCpy $0 "$0 --output-path=$\"$xbundle.catalog.path$\""
	ExecDos::exec /NOUNLOAD /ASYNC /TIMEOUT=$xbundle.tool.timeout '"$xbundle.tool.path" $0' "" ""
	Pop $xbundle.tool.downloadHandle
	Pop $0
	
	${If} $xbundle.tool.downloadHandle == 0
		SetErrors
		Return
	${EndIf}
	
	StrCpy $xbundle.catalog.state "downloading"
		
FunctionEnd
!macro XBundle_BeginCatalogDownload
	Call XBundle_BeginCatalogDownload
!macroend
!define XBundle_BeginCatalogDownload "!insertmacro 'XBundle_BeginCatalogDownload'"

Function XBundle_FinishCatalogDownload
	${If} $xbundle.tool.path == ""
	${OrIf} $xbundle.catalog.path == ""
		SetErrors
		Return
	${EndIf}
	
	${If} $xbundle.tool.downloadHandle == 0
		Return
	${EndIf}
	
	ExecDos::wait /NOUNLOAD $xbundle.tool.downloadHandle
	Exch $0
	
	StrCpy $xbundle.tool.downloadHandle 0
	${If} $0 <> 0
		SetErrors
	${EndIf}
	
	Pop $0
	StrCpy $xbundle.catalog.state "downloaded"
FunctionEnd
!macro XBundle_FinishCatalogDownload
	Call XBundle_FinishCatalogDownload
!macroend
!define XBundle_FinishCatalogDownload "!insertmacro 'XBundle_FinishCatalogDownload'"

Function XBundle_EnumerateOffers 
	Exch $0 ; callback
	Exch
	Exch $1 ; parameter
	Exch 2
	Exch $2 ; maximum count
	Push $3
	Push $4
	Push $5
	Push $6
	Push $7
	Push $8
	Push $9
	
	${If} $2 == ""
	${OrIf} $2 < 1
		StrCpy $2 1000000000
	${EndIf}
	
	ReadINIStr $3 "$xbundle.catalog.path" "catalog" "offer_list"
 	StrLen $5 $3 ;length
	StrCpy $6 0  ;block
	StrCpy $4 0  ; counter
	
	${ForEach} $7 0 $5 + 1 ; cursor
		StrCpy $8 $3 1 $7 ; test char
		${If} $8 == " "
		${OrIf} $8 == ","
		${OrIf} $8 == ";"
		${OrIf} $8 == ""
			IntOp $9 $7 - $6 ; block len
			${If} $9 > 0
				StrCpy $8 $3 $9 $6 ; copy block in $8
				Push "$1"
				Push "$8"
				Call $0
				Pop $9
				${If} $9 != "OK"
					${If} $9 == "Abort"
						${Break}
					${EndIf}
				${Else}
					IntOp $4 $4 + 1
					${If} $4 U>= $2
						${Break}
					${EndIf}
				${EndIf}
			${EndIf}
			IntOp $6 $7 + 1 ; set next block begin
		${EndIf}
	${Next}
	
	StrCpy $0 $4
	Pop $9
	Pop $8
	Pop $7
	Pop $6
	Pop $5
	Pop $4
	Pop $3
	Pop $2
	Pop $1
	Exch $0
FunctionEnd
!macro XBundle_EnumerateOffers __callback __param __maxCount __outputVar
	Push "${__maxCount}"
	Push "${__param}"
	Push $0
	GetFunctionAddress $0 "${__callback}"
	Exch $0
	Call XBundle_EnumerateOffers
	Pop "${__outputVar}"
!macroend
!define XBundle_EnumerateOffers "!insertmacro 'XBundle_EnumerateOffers'"

!macro XBundle_InstallOffer
Function XBundle_InstallOffer
	Exch $0 ; offer_name
	Exch
	Exch $1 ; param
	Push $2 
	Push $3 
	Push $4
	Push $5

	!ifdef XBUNDLE_OFFER_TITLE
		!undef XBUNDLE_OFFER_TITLE
	!endif
	
	!ifdef XBUNDLE_OFFER_ERROR
		!undef XBUNDLE_OFFER_ERROR
	!endif
		
	ReadINIStr $2 "$xbundle.catalog.path" "$0" "error"
	ReadINIStr $3 "$xbundle.catalog.path" "$0" "status"
	${If} $2 != "OK"
		StrCpy $0 "OfferFailedToPrepare"
	${ElseIf} $3 != "ready"
		StrCpy $0 "OfferInBadState"
	${Else}
		ReadINIStr $2 "$xbundle.catalog.path" "$0" "selected"
		${If} $2 == ""
			ReadINIStr $2 "$xbundle.catalog.path" "$0" "selected_by_default"
		${EndIf}
		
		${If} $2 != "yes"
			StrCpy $0 "OfferNotSelected"
		${Else}
			${If} ${Silent}
				ReadINIStr $2 "$xbundle.catalog.path" "$0" "silent_mode_install"
				${If} $2 != "yes"
					StrCpy $0 "OfferNotInstalledInSilentMode"
				${EndIf}
			${Else}
				ReadINIStr $2 "$xbundle.catalog.path" "$0" "presented"
				${If} $2 != "yes"
					StrCpy $0 "OfferNotPresented"
				${EndIf}
			${EndIf}
		${EndIf}
		
		${If} $2 == "yes"
			ReadINIStr $2 "$xbundle.catalog.path" "$0" "installer_url"
			StrCpy $3 "$PLUGINSDIR\installer_$0.exe"
			WriteINIStr "$xbundle.catalog.path" "$0" "status" "downloading"
			
			ReadINIStr $5 "$xbundle.catalog.path" "$0" "title"
			!define XBUNDLE_OFFER_TITLE "$5"
			!define XBUNDLE_OFFER_ERROR "$4"
			
			DetailPrint "${XBUNDLE_STRING_DOWNLOADING}"
			
		;	inetc::get /NOUNLOAD /CAPTION "${XBUNDLE_STRING_DOWNLOADING}" \
		;			   /TRANSLATE3 "${XBUNDLE_STRING_DOWNLOADING}" \
		;						   "${XBUNDLE_STRING_CONNECTING}" \
		;						   "${XBUNDLE_STRING_ONE_SECOND_REMAINING}" \
		;						   "${XBUNDLE_STRING_ONE_MINUTE_REMAINING}" \
		;						   "${XBUNDLE_STRING_ONE_HOUR_REMAINING}" \
		;						   "${XBUNDLE_STRING_X_SECONDS_REMAINING}" \
		;						   "${XBUNDLE_STRING_X_MINUTES_REMAINING}" \
		;						   "${XBUNDLE_STRING_X_HOURS_REMAINING}" \
		;						   "${XBUNDLE_STRING_DOWNLOAD_PROGRESS_TEMPLATE}" \
		;						   "${XBUNDLE_STRING_DOWNLOAD_PROGRESS_WITHOUT_TOTAL_TEMPLATE}" \
		;							"${XBUNDLE_STRING_SIZE_ZERO_BYTES}" \
		;							"${XBUNDLE_STRING_SIZE_ONE_BYTE}" \
		;							"${XBUNDLE_STRING_SIZE_BYTES}" \
		;							"${XBUNDLE_STRING_SIZE_KB}" \
		;							"${XBUNDLE_STRING_SIZE_MB}" \
		;							"${XBUNDLE_STRING_SIZE_GB}" \
		;							"${XBUNDLE_STRING_SIZE_TB}" \
		;			   "$2" "$3" /END
		
	
			inetc::get /NOUNLOAD /CAPTION "${XBUNDLE_STRING_DOWNLOADING}" \
					   /TRANSLATE2 "${XBUNDLE_STRING_DOWNLOADING}" \
								   "${XBUNDLE_STRING_CONNECTING}" \
								   "${XBUNDLE_STRING_ONE_SECOND_REMAINING}" \
								   "${XBUNDLE_STRING_ONE_MINUTE_REMAINING}" \
								   "${XBUNDLE_STRING_ONE_HOUR_REMAINING}" \
								   "${XBUNDLE_STRING_X_SECONDS_REMAINING}" \
								   "${XBUNDLE_STRING_X_MINUTES_REMAINING}" \
								   "${XBUNDLE_STRING_X_HOURS_REMAINING}" \
								   "${XBUNDLE_STRING_DOWNLOAD_PROGRESS_TEMPLATE}" \
								   "$2" "$3" /END
			Pop $4
			WriteINIStr "$xbundle.catalog.path" "$0" "status" "downloaded"
			${If} $4 == "OK"				
				DetailPrint "${XBUNDLE_STRING_DOWNLOAD_FINISHED}"
			
				ReadINIStr $2 "$xbundle.catalog.path" "$0" "installer_param"
				
				StrCpy $4 "/S "
				StrCpy $4 "$4 --language-id=$LANGUAGE"
				${If} $2 != ""
					StrCpy $4 "$4 $2"
				${EndIf}
				WriteINIStr "$xbundle.catalog.path" "$0" "status" "installing"
				DetailPrint "${XBUNDLE_STRING_INSTALLING}"
				ExecDos::exec /NOUNLOAD '"$3" $4' "" ""
				Pop $4
				WriteINIStr "$xbundle.catalog.path" "$0" "status" "installed"
				${If} $4 == 0
					DetailPrint "${XBUNDLE_STRING_INSTALL_FINISHED}"
					StrCpy $0 "OK"
				${Else}
					${If} $4 == 1
						DetailPrint "${XBUNDLE_STRING_INSTALL_CANCELLED}"
						WriteINIStr "$xbundle.catalog.path" "$0" "error" "Error_InstallerCancelled"
						StrCpy $0 "InstallerCancelled"
					${Else}
						DetailPrint "${XBUNDLE_STRING_INSTALL_FINISHED_WITH_ERROR}"
						WriteINIStr "$xbundle.catalog.path" "$0" "error" "Error_InstallerFailed"
						StrCpy $0 "InstallerFailed"
					${EndIf}
				${EndIf}
			${Else}
				${If} $4 == "cancelled"
					DetailPrint "${XBUNDLE_STRING_DOWNLOAD_CANCELLED}"
					WriteINIStr "$xbundle.catalog.path" "$0" "error" "Error_InstallerDownloadCancelled"
					StrCpy $0 "InstallerDownloadCancelled"
				${Else}
					DetailPrint "${XBUNDLE_STRING_DOWNLOAD_FINISHED_WITH_ERROR}"
					WriteINIStr "$xbundle.catalog.path" "$0" "error" "Error_InstallerDownloadFailed"
					StrCpy $0 "InstallerDownloadFailed"
				${EndIf}
			${EndIf}
		${EndIf}
	${EndIf}
	
	Pop $5
	Pop $4
	Pop $3
	Pop $2
	Pop $1
	Exch $0
	
	!ifdef XBUNDLE_OFFER_TITLE
		!undef XBUNDLE_OFFER_TITLE
	!endif
	
	!ifdef XBUNDLE_OFFER_ERROR
		!undef XBUNDLE_OFFER_ERROR
	!endif
	
FunctionEnd
!macroend
!define XBundle_InstallOffer "!insertmacro 'XBundle_InstallOffer'"

!macro XBundle_InsertSection
	
	${XBundleUI_DefineDefault} XBUNDLE_STRING_ONE_HOUR_REMAINING "1 hour remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_X_HOURS_REMAINING "%u hours remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_ONE_MINUTE_REMAINING "1 minute remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_X_MINUTES_REMAINING "%u minutes remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_ONE_SECOND_REMAINING "1 second remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_X_SECONDS_REMAINING "%u seconds remaining"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_CONNECTING "Connecting ..."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOADING "Downloading '${XBUNDLE_OFFER_TITLE}' ..."
	
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOAD_PROGRESS_TEMPLATE "%s%s of %s (%s/sec)"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOAD_PROGRESS_WITHOUT_TOTAL_TEMPLATE "%s completed (%s/sec)"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_ZERO_BYTES "Zero bytes"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_ONE_BYTE "byte"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_BYTES "bytes"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_KB "KB"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_MB "MB"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_GB "GB"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_SIZE_TB "TB"
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOAD_FINISHED "'${XBUNDLE_OFFER_TITLE}' downloaded successfully."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOAD_CANCELLED "Download of '${XBUNDLE_OFFER_TITLE}' cancelled."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_DOWNLOAD_FINISHED_WITH_ERROR "Unable to download '${XBUNDLE_OFFER_TITLE}' (Error: ${XBUNDLE_OFFER_ERROR})."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_INSTALLING "Installing '${XBUNDLE_OFFER_TITLE}' ..."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_INSTALL_FINISHED "'${XBUNDLE_OFFER_TITLE}' installed successfully."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_INSTALL_CANCELLED "Installation of '${XBUNDLE_OFFER_TITLE}' cancelled."
	${XBundleUI_DefineDefault} XBUNDLE_STRING_INSTALL_FINISHED_WITH_ERROR "Unable to install '${XBUNDLE_OFFER_TITLE}' (Error: ${XBUNDLE_OFFER_ERROR}).."
	
	Section -XBundleSection IDX_XBUNDLE_SECTION
		${XBundle_FinishCatalogDownload}
		Push $0
		Push $1
		ReadINIStr $1 "$xbundle.catalog.path" "catalog" "display_count"
		${XBundle_EnumerateOffers} "XBundle_InstallOffer" 0 $1 $0
		Pop $1
		Pop $0
	SectionEnd
	
	${XBundle_InstallOffer}
!macroend
!define XBundle_InsertSection "!insertmacro 'XBundle_InsertSection'"

Function XBundle_IsOfferAvailable
	Exch $0 ; offer_name
	Exch
	Exch $1 ; param
	Push $2
		
	ReadINIStr $2 "$xbundle.catalog.path" "$0" "error"
	${If} $2 != "OK"
		StrCpy $0 "OfferFailedToPrepare"
	${Else}
		ReadINIStr $2 "$xbundle.catalog.path" "$0" "status"
		${If} $2 != "ready"
			StrCpy $0 "OfferInBadState"
		${Else}
			StrCpy $0 "OK"
		${EndIf}
	${EndIf}
	
	Pop $2
	Pop $1
	Exch $0
FunctionEnd

Function XBundle_IsOffersAvailable
	Push $0
	${XBundle_EnumerateOffers} "XBundle_IsOfferAvailable" 0 1 $0
	${If} $0 > 0
		StrCpy $0 "yes"
	${Else}
		StrCpy $0 "no"
	${EndIf}
	Exch $0
FunctionEnd
!macro XBundle_IsOffersAvailable __outputVar
	Call XBundle_IsOffersAvailable
	Pop "${__outputVar}"
!macroend
!define XBundle_IsOffersAvailable "!insertmacro 'XBundle_IsOffersAvailable'"


!else ; WINAMP_XBUNDLE_INCLUDE
!macro XBundle_Init __param1
!macroend
!define XBundle_Init "!insertmacro 'XBundle_Init'"
!define XBundle_Finish ""
!define XBundle_BeginCatalogDownload ""
!define XBundle_FinishCatalogDownload ""
!define XBundle_InsertSection ""
!define XBundle_IsOffersAvailable ""

!endif 

!include "xbundle_ui.nsh"



!endif ;NULLSOFT_XBUNDLE_HEADER