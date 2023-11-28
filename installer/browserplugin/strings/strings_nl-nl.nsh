; Winamp Detect Translation file.
;
; Languge-Culture:	NL-NL
; LangId:		1043
; CodePage:		1252
; Author:		Updates by Paul van Garderen
; Email:		paul@winampnederlands.nl
;
; History:
; 30.11.09	* First version
; 13.12.09	* Up-to-date with 23 nov 2009 en-us version
;

!insertmacro LANGFILE_EXT "Dutch"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE 			"Installatietaal"
${LangFileString} IDS_INSTALLER_LANG_INFO 			"Selecteer een taal."

${LangFileString} IDS_PAGE_WELCOME_TITLE 			"$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  			"Dit programma installeerd plugins voor bepaalde browsers zodat web applicaties versie informatie van de geinstalleerde Winamp installatie kunnen opvragen. Er word geen persoonlijke data overgezonden en u kunt deze plugins op elk moment verwijderen$\r$\n$\r$\nDruk op Volgende om verder te gaan."

${LangFileString} IDS_PAGE_DIRECTORY_TOP 			"De setup zal $(^NameDA) in de volgende map installeren. \
									Om in een andere map te installeren, klik op Bladeren en kies een map. \
									$\r$\n$\r$\nN.B.: Sommige plugins moeten in de browsermap geinstalleerd worden."

${LangFileString} IDS_PAGE_COMPONENT_HEADER 			"Browser Plugins"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER 			"Kies de browsers waarvoor u de plugins wilt installeren."
${LangFileString} IDS_PAGE_COMPONENT_TOP			"Kies plugins voor de gevonden browsers. Druk op Volgende om verder te gaan." 
${LangFileString} IDS_PAGE_COMPONENT_LIST 			"Selecteer browser plugins om te installeren:"

${LangFileString} IDS_PAGE_FINISH_TITLE 			"Plugins installatie voltooid"
${LangFileString} IDS_PAGE_FINISH_TEXT 				"$(^NameDA) setup is klaar met de installatie van browser plugins."
${LangFileString} IDS_PAGE_FINISH_LINK 				"Ga naar winamp.com voor meer informatie"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER 		"Voor welke browser wilt u de plugins verwijderen."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP 			"Kies uit de lijst welke plugins u wilt verwijderen. \
									Druk op Volgende om door te gaan." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST			"Selecteer browser plugins om te verwijderen:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE 			"$(^NameDA) deinstallatie voltooid"
${LangFileString} IDS_UNPAGE_FINISH_TEXT 			"$(^NameDA) is van uw computer verwijderd."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL 		"Plugins verwijderd"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL 		"$(^NameDA) is klaar met het verwijderen van de geselecteerde plugins."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART 		"Deze browsers moeten opnieuw worden gestart om de deinstallatie te voltooien:"
${LangFileString} IDS_UNPAGE_FINISH_LINK			"Feedback Opsturen"

!endif

${LangFileString} IDS_INSTALLER_NAME 				"Winamp Applicatie Detect"
${LangFileString} IDS_INSTALLER_PUBLISHER 			"Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT 			"Verwijder $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT			"$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC			"$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX				"Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC			"Mozilla Firefox browser plugin"
${LangFileString} IDS_SECTION_EXPLORER				"Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC			"Windows Internet Explorer browser plugin"

${LangFileString} IDS_MESSAGE_NOWINAMP 				"De setup was niet instaat om Winamp in de opgegeven locatie te vindenn.$\r$\nToch doorgaan?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL			"De setup heeft een vorige installtie gevonden op een andere locatie.$\r$\n\
									Het is aan te raden de andere versie eerste te verwijderen.$\r$\nDoorgaan met verwijderen?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED 			"Oeps. Deinstallatie heeft een fout ondervonden!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION 			"Er is niets om te installeren.$\r$\nToch doorgaan?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART			"De volgende browsers moeten opnieuw worden gestart voordat de plugins gebruikt kunnen worden:"

${LangFileString} IDS_DETAILS_INITIALIZING			"Initialiseren"
${LangFileString} IDS_DETAILS_SKIPPING				"Overslaan"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS			"Snelkoppelingen maken"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL			"Deinstallatie data wegschrijven"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME		"Deinstallatie is even oud of nieuwer"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN		"Plugin installeren voor $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND			"Browser niet gevonden"
${LangFileString} IDS_DETAILS_BROWSER_VERSION			"Browser versie"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION		"Bronplugin versie"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION		"Doelplugin versie"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND		"Doelplugin niet gevonden"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME	"Target plugin version newer or the same"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN			"Plugin installeren"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR		"Fout tijdens installatie van plugin voor $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT		"Een aaantal plugins zijn niet verwijderd"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER		"Geselecteerde componenten zijn verwijderd. Deinstaller behouden voor later gebruik."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN			"Plugin voor $0 verwijderen"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER		"Deinstallater verwijderen"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE			"Kan niet schrijven"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER			"Kan niet registreren"

${LangFileString} IDS_DETAILS					"Details"
${LangFileString} IDS_UNKNOWN					"Onbekend"

														