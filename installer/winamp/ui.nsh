!ifndef NULLSOFT_WINAMP_INSTALLER_UI_HEADER
!define NULLSOFT_WINAMP_INSTALLER_UI_HEADER

!include ".\mui2.nsh"
!include ".\waui.nsh"
!include ".\sections\xbundle.nsh"
!include ".\sections\openCandy.nsh"

ReserveFile "${HEADER_IMAGE_PATH}"
ReserveFile "${WELCOMEFINISH_IMAGE_PATH}"
ReserveFile "${UNINSTALLER_WELCOMEFINISH_IMAGE_PATH}"

!define MUI_ICON 								".\res\install.ico"
!define MUI_UNICON 								".\res\uninstall.ico"
!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP 					"${HEADER_IMAGE_PATH}"

!define MUI_WELCOMEFINISHPAGE_BITMAP 			"${WELCOMEFINISH_IMAGE_PATH}"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP 			"${UNINSTALLER_WELCOMEFINISH_IMAGE_PATH}"

!define MUI_CUSTOMFUNCTION_GUIINIT 				UI_OnInit

; Welcome Page
!define MUI_WELCOMEPAGE_TITLE 					"$(IDS_PAGE_WELCOME_TITLE)"
!define MUI_WELCOMEPAGE_TEXT 					"$(IDS_PAGE_WELCOME_TEXT)"
!insertmacro MUI_PAGE_WELCOME

; License Page
!define MUI_LICENSEPAGE_TEXT_TOP 				"$(licenseTop)"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW  			UI_OnLicensePageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE 			UI_OnLicensePageLeave
!insertmacro MUI_PAGE_LICENSE "${LICENSE_PATH}"
	
; Directory Page
!define MUI_DIRECTORYPAGE_TEXT_TOP 				"$(directoryTop)"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW  			UI_OnDirectoryPageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE 			UI_OnDirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

; Components Page
!define MUI_COMPONENTSPAGE_TEXT_TOP 			"$(installWinampTop)${INSTALLER_TYPE_DESCRIPTION}"
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST 		"$(IDS_PAGE_COMPONENTS_COMPLIST)"
!define MUI_COMPONENTSPAGE_SMALLDESC 
!define MUI_CUSTOMFUNCTION_ONMOUSEOVERSECTION 	UI_OnMouseOverSection
!insertmacro MUI_PAGE_COMPONENTS

; Components Page Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; StartMenu Page
!define STARTMENUPAGE_CHECK_NEXT_BUTTON
!insertmacro WAUI_PAGE_STARTMENU

; XBundle Page
${XBundle_InsertPage}

; OpenCandy Page
${OPENCANDY_PAGE}

; InstallProgress Page
!insertmacro MUI_PAGE_INSTFILES

; WhatsNew Page (only in Beta)
!ifdef BETA
  !ifdef MUI_LICENSEPAGE_TEXT_TOP
    !undef MUI_LICENSEPAGE_TEXT_TOP
  !endif
  !ifdef MUI_LICENSEPAGE_TEXT_BOTTOM
    !undef MUI_LICENSEPAGE_TEXT_BOTTOM
  !endif
  !define MUI_PAGE_HEADER_TEXT 					"$(verHistHeader)"
  !define MUI_PAGE_HEADER_SUBTEXT 				"$(verHistHeaderSub)"
  !define MUI_LICENSEPAGE_TEXT_TOP 				"$(verHistTop)"
  !define MUI_LICENSEPAGE_TEXT_BOTTOM 			"$(verHistBottom)"
  !define MUI_LICENSEPAGE_BUTTON 				"$(^NextBtn)"
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW  		UI_OnWhatsNewPageShow
  !insertmacro MUI_PAGE_LICENSE 				"..\..\resources\data\whatsnew_beta.rtf"
!endif

; Finish Page
!ifdef _DEBUG
	!define MUI_FINISHPAGE_NOAUTOCLOSE
!endif
!define MUI_FINISHPAGE_TEXT_LARGE
!define MUI_FINISHPAGE_TITLE          			"$(IDS_PAGE_FINISH_TITLE)"
!define MUI_FINISHPAGE_TITLE_3LINES 			; For compatibility with installer translations
!define MUI_FINISHPAGE_TEXT          			"$(IDS_PAGE_FINISH_TEXT)"
!ifdef BUNDLE
	!define MUI_FINISHPAGE_RUN
	!define MUI_FINISHPAGE_RUN_TEXT		 		"$(IDS_PAGE_FINISH_SETMUSICBUNDLE)"
	!define MUI_FINISHPAGE_RUN_FUNCTION			UI_OnFinishPageRun
!endif
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT		 	"$(IDS_PAGE_FINISH_RUN)"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION		UI_OnFinishPageReadMe
!define MUI_FINISHPAGE_LINK            			"$(IDS_PAGE_FINISH_LINK)"
!define MUI_FINISHPAGE_LINK_LOCATION   			"http://www.winamp.com/"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW			UI_OnFinsihPageShow
!insertmacro  MUI_PAGE_FINISH

!endif ;NULLSOFT_WINAMP_INSTALLER_UI_HEADER