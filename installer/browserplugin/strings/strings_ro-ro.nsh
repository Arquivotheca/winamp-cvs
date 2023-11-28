; Language-Country:	RO-RO
; LangId:			1048
; CodePage:			1250 - UTF8
; Revision:			1.1
; Last udpdated:	25.06.2010
; Author:			Cătălin ZAMFIRESCU
; Email:			x10@mail.com


!insertmacro LANGFILE_EXT "Romanian"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Limbă instalare"
${LangFileString} IDS_INSTALLER_LANG_INFO "Vă rugăm, selectaţi o limbă."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Acest asistent instalează extensii pentru navigatoarele \
					Dvs. pentru a permite aplicaţiilor web să detecteze prezenţa Winamp şi versiunea sa \
					pe acest computer.  Nici o informaţie personală nu va fi publicată şi veţi putea \
					dezinstala oricând aceaste extensii.$\r$\n$\r$\nApăsaţi Înainte pentru a continua."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "Asistentul va instala $(^NameDA) în următorul dosar \
					destinaţie. Pentru a schimba locaţie de instalare, apăsaţi Răsfoire şi selectaţi \
					dosarul dorit.$\r$\n$\r$\nNotă: Unele extensii vor fi instalate în dosarul de instalare \
					al navigatorlului."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Extensii navigator"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Alegere navigator pentru care se vor instala extensiile."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Alegeţi navigatorul pentru care va funţiona $(^NameDA). \
					Apăsaţi Înainte pentru a continua." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Selectare extensii:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Instalare finalizată"
${LangFileString} IDS_PAGE_FINISH_TEXT "Instalarea $(^NameDA) s-a încheia cu succes."
${LangFileString} IDS_PAGE_FINISH_LINK "Visitaţi Winamp.com pentru a găsi mai multe opţiuni."

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Dezinstalare $(^NameDA) de pe calculator."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Alegeţi navigatorul pentru care se va dezinstala $(^NameDA) \
					Apăsaţi Înainte pentru a continua." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Selectare extensii:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "Dezinstalare finalizată"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "Dezinstalarea $(^NameDA) s-a încheia cu succes."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Extensii dezinstalate"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA) a dezinstalat complet extensiile selectate."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Se recomandă repornirea următoarelor navigatoare \
					pentru ca dezinstalarea să fie finalizată:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Trimiteţi părerile Dvs."

!endif

${LangFileString} IDS_INSTALLER_NAME "Detector Winamp"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Dezinstalare $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Extensie Mozilla Firefox"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Extensie Windows Internet Explorer"

${LangFileString} IDS_MESSAGE_NOWINAMP "Asistentul nu a putut localiza Winamp în locaţia specificată.$\r$\nContinuaţi oricum?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "Asistentul a detectat o instalare anterioară într-o locaţie diferită.$\r$\n\
													Se recomandă dezinstalarea celeilalte versiuni mai întâi .$\r$\nContinuaţi fără dezinstalare?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "Naso', dezinstalarea a eşuat!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nimic de instalat.$\r$\nContinuaţi oricum?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Următoarele navigatoare trebuie repornite pentru ca extensiile să poată fi folosite:"

${LangFileString} IDS_DETAILS_INITIALIZING "Iniţializare"
${LangFileString} IDS_DETAILS_SKIPPING "Ignorare"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Creare comenzi rapide"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Înregistrare informaţii dezinstalare"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Program de dezinstalare  mai nou sau identic"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Instalare extensie pentru $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Navigator negăsit"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Versiune vavigator"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Versiune extensie sursă"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Versiune extensie destinaţie"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Extensie destinaţie negăsită"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Versiune extensie destinaţie mai nouă sau identică"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Instalare extensie"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Eroare instalare extensie pentru $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Unele extensii nu for fi dezinstalare"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Componentele selectate au fost eliminate. Programul de dezinstalare a fost menţinut pentru o utilizare ulterioară."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Dezinstalare extensii pentru $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Eliminare program de dezinstalare"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Imposibil de scris"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Imposibil de înregistrat"

${LangFileString} IDS_DETAILS "Detalii"
${LangFileString} IDS_UNKNOWN "Necunoscut"

														