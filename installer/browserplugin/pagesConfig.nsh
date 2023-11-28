!ifndef WACHK_PAGES_CONFIG_HEADER
!define WACHK_PAGES_CONFIG_HEADER

!include ".\config.nsh"

!define MUI_LANGDLL_REGISTRY_ROOT 		"${WACHK_INSTALLER_REGROOT}"
!define MUI_LANGDLL_REGISTRY_KEY 		"${WACHK_UNINSTALLER_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME 	"${WACHK_LANGUAGE_REGKEY}"

!define MUI_LANGDLL_WINDOWTITLE $(IDS_INSTALLER_LANG_TITLE)
!define MUI_LANGDLL_INFO 		$(IDS_INSTALLER_LANG_INFO)

!define MUI_HEADERIMAGE

!define MUI_ICON 						".\resources\install.ico"
!define MUI_HEADERIMAGE_BITMAP 			".\resources\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP 	".\resources\welcome.bmp"

!ifndef WACHK_DEBUG_MODE
	!define MUI_ABORTWARNING
!else
	!define MUI_FINISHPAGE_NOAUTOCLOSE	
!endif

!define MUI_UNICON 							".\resources\uninstall.ico"
!define MUI_HEADERIMAGE_UNBITMAP 			".\resources\header.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP 		".\resources\welcome.bmp"

!ifndef WACHK_DEBUG_MODE
	!define MUI_UNABORTWARNING
!else
	!define MUI_UNFINISHPAGE_NOAUTOCLOSE	
!endif

!endif
