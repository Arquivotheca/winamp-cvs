﻿!ifndef WACHK_CONFIG_FILE
!define WACHK_CONFIG_FILE

;	!define WACHK_DEBUG_MODE
	
	!ifndef WACHK_SRC_PATH
		!define WACHK_SRC_PATH "..\..\output"
	!endif
		
	!define WACHK_INSTALLER_NAME		"Winamp Detect"
	!define WACHK_INSTALLER_FILENAME	"installWaDetect.exe"
	!define WACHK_INSTALLER_DEFPATH		"$PROGRAMFILES\${WACHK_INSTALLER_NAME}\"
	!define WACHK_INSTALLER_HOMELINK	"http://www.winamp.com"
	!define WACHK_INSTALLER_FEEDBACKLINK "http://www.aolfeedback-clarabridge.com/se.ashx?s=04BD76CC65A7BB3C&tid=71&c=en-US"
	!define WACHK_INSTALLER_REGROOT		"HKCU"
	!define WACHK_INSTALLER_VER_MAJOR	1
	!define WACHK_INSTALLER_VER_MINOR	0
	!define WACHK_INSTALLER_VER_MINOR2	0
	!define WACHK_INSTALLER_VER_BUILD	1
	!define WACHK_INSTALLER_VER			"${WACHK_INSTALLER_VER_MAJOR}.${WACHK_INSTALLER_VER_MINOR}.${WACHK_INSTALLER_VER_MINOR2}.${WACHK_INSTALLER_VER_BUILD}"
	!define WACHK_INSTALLER_COMMENTS	"Visit ${WACHK_INSTALLER_HOMELINK} for updates"
	!define WACHK_INSTALLER_COMPANY		"Nullsoft, Inc."
	!define WACHK_INSTALLER_TRADEMARKS	"Nullsoft and Winamp are trademarks of ${WACHK_INSTALLER_COMPANY}"
	!define WACHK_INSTALLER_COPYRIGHT	"Copyright © 1997-2009, ${WACHK_INSTALLER_COMPANY}"
	!define WACHK_INSTALLER_DESCRIPTION	"${WACHK_INSTALLER_NAME} Installer"
	
	!define WACHK_STARTMENU_PAGEID		"WainampCheck_StartMenuPageId"
	!define WACHK_STARTMENU_DEFFOLDER	"$(IDS_INSTALLER_NAME)"
	!define WACHK_STARTMENU_REGKEY		"StartMenu"
	
	!define WACHK_LANGUAGE_REGKEY		"LangId"
		
	!define WACHK_UNINSTALLER_FILENAME	"UninstWaDetect.exe"
	!define WACHK_UNINSTALLER_REGKEY	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${WACHK_INSTALLER_NAME}"
	
!endif