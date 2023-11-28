; Language-Country:	de-DE
; LangId:			1030
; Revision:			1.0
; Last udpdated:	07.12.2009
; Author:			Christoph Grether
; Email:			chris-mail@gmx.de


!insertmacro LANGFILE_EXT "German"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Sprache wählen"
${LangFileString} IDS_INSTALLER_LANG_INFO "Bitte wählen Sie eine Sprache"

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Dieses Programm installiert Plug-ins für unterstützte \
                    Browser, die Web-Anwendungen erlauben Anfragen bezüglich der Präsenz und \
					Versionsinfos von Winamp auf dem lokalen PC abzufragen. Es werden keine \
					persönlichen Daten übertragen und Sie können diese Plug-ins jederzeit deinstallieren$\r$\n$\r$\nKlicken Sie auf Weiter um fortzufahren."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "Setup wird $(^NameDA) in das folgende Verzeichnis installieren. \
				    Um in ein anderem Ordner zu installieren, Klicken Sie auf die Durchsuchen-Schaltfläche \
					und wählen Sie einen anderen Ordner.   $\r$\n$\r$\nHinweis:  Einige Plug-ins müssen ins \
					Verzeichnis des Browsers installiert werden."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Browser Plug-ins"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Wählen Sie einen Browser, für den Sie Plug-ins installieren möchten."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Wählen Sie Plug-ins für die unterstützten Browser, die auf Ihrem System\
													erkannt wurden. Klicken Sie auf Weiter um fortzufahren." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Zu installierende Browser Plug-ins wählen:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Installation der Plug-ins vollständig"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA)-Installer hat Browser Plugins Installation abgeschlossen."
${LangFileString} IDS_PAGE_FINISH_LINK "Besuchen Sie winamp.com für weitere nützliche Dinge"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Wählen Sie einen Browser, für den Sie Plug-ins deinstallieren möchten."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Wählen Sie Plug-ins aus der Liste, die Sie deinstallieren möchten. \
											Klicken Sie auf Weiter um fortzufahren." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Zu deinstallierende Browser Plug-ins wählen:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "$(^NameDA) Deinstallation vollständig"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) wurde von Ihrem System entfernt."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Plugins deinstalliert"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA) vervollständigte Deinstallation der ausgewählten Plug-ins."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Wir empfehlen die folgenden Browser \
                    zur Vervollständigung der Deinstallation neu zu starten:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Rückmeldung senden"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp Erkennungs-Plug-in"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Deinstalliere $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Mozilla Firefox Browser Plug-in"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Windows Internet Explorer Browser Plug-in"

${LangFileString} IDS_MESSAGE_NOWINAMP "Setup war nicht in der Lage Winamp im ausgewählten Verzeichnis zu lokalisieren.$\r$\nTrotzdem fortfahren?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "Setup erkannte eine vorherige Installation an einem anderen Ort.$\r$\n\
														Es wirde empfohlen, die andere Version erst zu deinstallieren.$\r$\nDeinstallation jetzt ausführen?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "Ups, Deinstallation fehlgeschlagen!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nicht zum Installieren.$\r$\nTrotzdem fortfahren?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Die folgenden Browser müssen neu gestartet werden, bevor die Plug-ins genutzt werden können:"

${LangFileString} IDS_DETAILS_INITIALIZING "Initialisiere"
${LangFileString} IDS_DETAILS_SKIPPING "Überspringe"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Erstelle Verknüpfungen"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Schreibe Deinstallationsdaten"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Ziel-Deinstaller neuer oder der gleiche"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Installiere Plug-in für $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Browser wurde nicht gefunden"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Browser Version"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Version des Quell-Plug-ins "
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Version des Ziel-Plug-ins"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Ziel-Plug-in wurde nicht gefunden"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Version des Ziel-Plug-ins neuer oder gleich"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Installiere Plug-in"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Fehler beim Installierend es Plug-ins für $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Einige Plug-ins wurden noch nicht deinstalliert"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Ausgewählte Komponenten wurden entfernt. Deinstaller für spätere Nutzung beibehalten."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Deinstalliere Plug-in für $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Entferne Deinstaller"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Nicht in der Lage zu schreiben"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Nicht in der Lage zu registrieren"

${LangFileString} IDS_DETAILS "Details"
${LangFileString} IDS_UNKNOWN "Unbekannt"

														