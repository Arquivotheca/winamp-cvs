; Language-Country:	pl-PL
; LangId:			1045
; CodePage:			1250
; Revision:			1
; Last udpdated:	27 stycznia 2010
; Author:			Paweł Porwisz aka Pepe
; Email:			pawelporwisz@gmail.com


; Notes:
; Use ';' or '#' for comments
; Strings must be in double quotes.
; Only edit the strings in quotes:
; Example: ${LangFileString} installFull "Edit This Value Only!"
; Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

; ----------------------------------------------------------------------------------------

!insertmacro LANGFILE_EXT "Polish"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE	"Język instalatora"
${LangFileString} IDS_INSTALLER_LANG_INFO	"Proszę wybrać język:"

${LangFileString} IDS_PAGE_WELCOME_TITLE	"$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"Kreator ten pomoże Ci zainstalować wtyczki dla obsługiwanych przeglądarek. Dzięki nim, aplikacje sieciowe będę miały możliwość wykrycia Winampa oraz informacji o jego wersji. Nie są zbierane żadne dane użytkownika, w każdej chwili możesz je także usunąć$\r$\n$\r$\nKliknij Dalej, aby kontynuować."

${LangFileString} IDS_PAGE_DIRECTORY_TOP	"Instalator zainstaluje program $(^NameDA) w następującym katalogu. \
											Aby zainstalować go w innej lokalizacji, kliknij przycisk Przeglądaj i wybierz inny katalog. \
											$\r$\n$\r$\nUwaga:  Niektóre wtyczki muszą być zainstalowane w katalogu przeglądarki."

${LangFileString} IDS_PAGE_COMPONENT_HEADER		"Wtyczki dla przeglądarek"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER	"Wybierz przeglądarkę, dla której chcesz zainstalować wtyczkę."
${LangFileString} IDS_PAGE_COMPONENT_TOP		"Wybierz przeglądarki, które będą obsługiwane przez $(^NameDA). \
												Kliknij Dalej, aby kontynuować." 
${LangFileString} IDS_PAGE_COMPONENT_LIST		"Wybierz wtyczki przeglądarki do zainstalowania:"

${LangFileString} IDS_PAGE_FINISH_TITLE		"Instalacja zakończona"
${LangFileString} IDS_PAGE_FINISH_TEXT		"Instalator $(^NameDA) pomyślnie zakończył instalację."
${LangFileString} IDS_PAGE_FINISH_LINK		"Więcej znajdziesz na stronie Winamp.com"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER	"Odinstaluj program $(^NameDA) z Twojego komputera."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP			"Wybierz wtyczki przeglądarek, które zostaną odinstalowane przez $(^NameDA). \
													Kliknij Odinstaluj, aby kontynuować." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST			"Wybierz wtyczki przeglądarki do deinstalacji:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE			"Zakończono odinstalowanie programu $(^NameDA)"
${LangFileString} IDS_UNPAGE_FINISH_TEXT			"Program $(^NameDA) został pomyślnie odinstalowany z Twojego komputera."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL	"Wtyczki zostały odinstalowane"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL	"$(^NameDA) zakończył deinstalację wybranych wtyczek."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART	"Aby zakończyć deinstalację, następujace przeglądarki muszą być uruchomione ponownie:"
${LangFileString} IDS_UNPAGE_FINISH_LINK			"Wyślij informacje zwrotne"

!endif

${LangFileString} IDS_INSTALLER_NAME		"Detektor Winampa"
${LangFileString} IDS_INSTALLER_PUBLISHER	"Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT		"Odinstaluj $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT		"$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC	"$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX			"Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC		"Wtyczka dla przeglądarki Mozilla Firefox"
${LangFileString} IDS_SECTION_EXPLORER			"Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC		"Wtyczka dla przeglądarki Windows Internet Explorer"
		
${LangFileString} IDS_MESSAGE_NOWINAMP			"Instalator nie znalazł Winampa w określonej lokalizacji.$\r$\nKontynuować mimo to?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL	"Instalator wykrył poprzednią instalację w innej lokalizacji.$\r$\n\
												Zaleca się najpierw odinstalować poprzednią wersję.$\r$\nCzy chcesz ją odinstalować?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED	"Deinstalacja nie powiodła się!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION	"Nie wybrano elementów do instalacji.$\r$\nKontynuować mimo to?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART	"Aby móc używać zainstalowane wtyczki, musisz uruchomić ponownie następujące przeglądarki:"

${LangFileString} IDS_DETAILS_INITIALIZING					"Inicjalizacja"
${LangFileString} IDS_DETAILS_SKIPPING						"Pomijam"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS				"Tworzenie skrótów"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL				"Zapisywanie danych deinstalacji"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME		"Docelowy deinstalator jest nowszy lub taki sam"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN			"Instalacja wtyczki dla programu $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND				"Przeglądarka nie została znaleziona"
${LangFileString} IDS_DETAILS_BROWSER_VERSION				"Wersja przeglądarki"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION			"Wersja wtyczki źródłowej"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION			"Wersja wtyczki docelowej"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND		"Wtyczka docelowa nie została znaleziona"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME 	"Wersja wtyczki docelowej jest nowsza lub taka sama"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN				"Instalacja wtyczki"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR			"Błąd instalacji wtyczki dla programu $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT 		"Niektóre wtyczki nie zostały odinstalowane"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER 		"Zaznaczone elementy zostały usunięte. Deinstalator nie został usunięty."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN 				"Deinstalacja wtyczki dla programu $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER 			"Usuwanie deinstalatora"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE		"Błąd zapisu"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER	"Błąd rejestracji"

${LangFileString} IDS_DETAILS	"Szczegóły"
${LangFileString} IDS_UNKNOWN	"Nieznany"