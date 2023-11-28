; Language-Country:	HU-HU
; LangId:			1038
; CodePage:			1252
; Revision:			1
; Last udpdated:	2012-05-06
; Author:			László Gárdonyi aka gLes
; Email:			gles@pro.hu


!insertmacro LANGFILE_EXT "Hungarian"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Telepítő nyelve"
${LangFileString} IDS_INSTALLER_LANG_INFO "Kérem válasszon nyelvet."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Ez a program bővítményeket telepít a támogatott \
					böngészőkhöz, amelyek segítségével a webes alkalmazások felismerik a \
					Winampot és a verzióját ezen a számítógépen. Semmilyen személyes adat \
					nem kerül megosztásra és bármikor eltávolíthatja ezeket a bővítményeket. \
					$\r$\n$\r$\nKattintson a Továbbra a folytatáshoz."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "A telepítő a $(^NameDA) alkalmazást az alábbi \
					célkönyvtárba fogja telepíteni. A célkönyvtár megváltoztatásához \
					kattintson a Tallózás gombra és válasszon egy másik könyvtárat. \
					$\r$\n$\r$\nMegjegyzés: Néhány bővítmény a böngészők saját \
					könyvtárába fog települni."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Böngésző bővítmények"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Válassza ki a böngészőket, amelyekhez telepíteni szeretné a bővítményeket."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Válassza ki, hogy melyik böngészőket támogassa a $(^NameDA). \
					A folytatásoz kattintson a Tovább gombra."
${LangFileString} IDS_PAGE_COMPONENT_LIST "Válassza ki a telepítendő böngésző bővítményeket:"

${LangFileString} IDS_PAGE_FINISH_TITLE "A telepítés kész"
${LangFileString} IDS_PAGE_FINISH_TEXT "A $(^NameDA) telepítése sikeresen elkészült."
${LangFileString} IDS_PAGE_FINISH_LINK "Látogassa meg a Winamp.com-ot további eszközökért és bővítményekért."

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "A $(^NameDA) eltávolítása a számítógépről."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Válassza ki, melyik böngészőkből távolítsa el a $(^NameDA) a bővítményeket. \
					A folytatáshoz kattintson az Eltávolítás gombra."
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Válassza ki az eltávolítandó bölngészp bővítményeket:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "A $(^NameDA) eltávolítása kész"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "A $(^NameDA) eltávolítása sikeresen befejeződött."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Eltávolított bővítmények"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "A $(^NameDA) sikeresen eltávolította a kiválasztott bővítményeket."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Azt javasoljuk, hogy az alábbi böngészőket indítsa újra az eltávolítás befejezéséhez:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Visszajelzés küldése"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp Detector bővítmény"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "$(IDS_INSTALLER_NAME) eltávolítása"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Mozilla Firefox böngésző bővítmény"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Windows Internet Explorer böngésző bővítmény"

${LangFileString} IDS_MESSAGE_NOWINAMP "A telepítő nem találta a Winampot a megadott helyen.$\r$\nÍgy is folytatja?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "A telepítő talált egy előző telepítést egy másik helyen.$\r$\n\
														Javasolt először eltávolítani a másik verziót.$\r$\nFolytatja az eltávolítással?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "D'oh, az eltávolítás nem sikerült!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nincs mit telepíteni.$\r$\nÍgy is folytatja?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Az alábbi böngészőket újra kell indítani mielőtt a bővítményt használhatná:"

${LangFileString} IDS_DETAILS_INITIALIZING "Inicializálás"
${LangFileString} IDS_DETAILS_SKIPPING "Kihagyás"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Parancsikonok létrehozása"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Eltávolítási adatok írása"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "A cél eltávolító újabb vagy megegyezik"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "$0 bővítmény telepítése"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "A böngésző nem található"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Böngésző verzió"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Forrás bővítmény verzió"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Cél bővítmény verzió"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Cél bővítmény nem található"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Cél bővítmény verziója újabb vagy megegyezik"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Bővítmény telepítése"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Hiba a(z) $0 bővítmény telepítésekor"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Néhány bővítmény még nincs telepítve"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Kiválasztott összetevők elváolítva. Az eltávolító megmarad későbbi használatra."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "$0 bővítmény eltávolítása"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Eltávolító törlése"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Nem sikerült az írás"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Nem sikerült a regisztráció"

${LangFileString} IDS_DETAILS "Részletek"
${LangFileString} IDS_UNKNOWN "Ismeretlen"

