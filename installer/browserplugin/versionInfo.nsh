!ifndef WACHK_VERSION_INFO_HEADER
!define WACHK_VERSION_INFO_HEADER

!include ".\config.nsh"

VIProductVersion "${WACHK_INSTALLER_VER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" 		"${WACHK_INSTALLER_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" 			"${WACHK_INSTALLER_COMMENTS}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" 		"${WACHK_INSTALLER_COMPANY}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" 	"${WACHK_INSTALLER_TRADEMARKS}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" 		"${WACHK_INSTALLER_COPYRIGHT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" 	"${WACHK_INSTALLER_DESCRIPTION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" 		"${WACHK_INSTALLER_VER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" 		"${WACHK_INSTALLER_VER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" 	"${WACHK_INSTALLER_FILENAME}"

!endif