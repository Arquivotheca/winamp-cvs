; Language-Country: pl-PL
; LangId:		    1045
; CodePage:		    1250
; Revision:		    1
; Last udpdated:    27 stycznia 2010
; Author:		    Paweł Porwisz aka Pepe
; Email:            pawelporwisz@gmail.com

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

${LangFileString} IDS_INSTALLER_TITLE "Winamp MP3 Bundle"

${LangFileString} IDS_INSTALLER_LANG_TITLE "Język instalatora"
${LangFileString} IDS_INSTALLER_LANG_INFO "Proszę wybrać język:"

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT "Kreator instalacji zainstaluje program $(^NameDA) - kolekcję darmowych plików mp3, które zostały wybrane specjalnie dla Ciebie.$\r$\n$\r$\nKliknij Dalej, aby kontynuować."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Wybierz utwory"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Wybierz utwory z listy, które chcesz skopiować."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Zaznacz utwory, które chcesz skopiować i odznacz te, których nie chcesz." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Wybierz utwory do skopiowania"
${LangFileString} IDS_PAGE_COMPONENT_DESC_TITLE "Informacje"
${LangFileString} IDS_PAGE_COMPONENT_DESC_INFO "Przesuń kursor myszy nad utwór, aby zobaczyć jego opis"

${LangFileString} IDS_PAGE_DIRECTORY_HEADER "Wybierz katalog docelowy"
${LangFileString} IDS_PAGE_DIRECTORY_SUBHEADER "Wybierz katalog, do którego chcesz skopiować utwory."
${LangFileString} IDS_PAGE_DIRECTORY_TOP "Wybrane utwory zostaną skopiowane do poniższego katalogu. Aby skopiować je do innego katalogu, kliknij przycisk Przeglądaj i wybierz inny katalog." 

${LangFileString} IDS_PAGE_FINISH_TITLE "Program $(^NameDA) został zainstalowany"
${LangFileString} IDS_PAGE_FINISH_TEXT "Gratulacje! Darmowa muzyka Winampa została skopiowana na Twój komputer.$\r$\nWłącz i słuchaj!"
${LangFileString} IDS_PAGE_FINISH_PLAYLIST_ERROR "$\r$\nUwaga:$\r$\n    Utworzenie listy odtwarzania nie powiodło się."
${LangFileString} IDS_PAGE_FINISH_PLAYROTATION "Włącz odtwarzanie"
${LangFileString} IDS_PAGE_FINISH_CREATESHORTCUT "Utwórz skrót do listy odtwarzania na pulpicie"
${LangFileString} IDS_PAGE_FINISH_DISCOVERLINK "Odwiedź stronę z muzyką Winampa i pobierz więcej"

${LangFileString} IDS_DOWNLOADING_COMPOSITION "Pobieranie utworu"
${LangFileString} IDS_DOWNLOADING_ABLUMART "Pobieranie okładki albumu"
${LangFileString} IDS_CONNECTING "Łączenie..."
${LangFileString} IDS_SECOND " (pozostała 1 sekunda)"
${LangFileString} IDS_MINUTE " (pozostała 1 minuta)"
${LangFileString} IDS_HOUR " (pozostała 1 godzina)"
${LangFileString} IDS_SECONDS " (pozostało %u sekund)"
${LangFileString} IDS_MINUTES " (pozostało %u minut)"
${LangFileString} IDS_HOURS " (pozostało %u godzin)"
${LangFileString} IDS_PROGRESS "%skB (%d%%) z %skB przy %u.%01ukB/s"

${LangFileString} IDS_SHORTCUT_NAME "$(^NameDA)"

${LangFileString} IDS_DETAILS_CREATE_PLAYLIST_ERROR "Nie można utworzyć listy odtwarzania"

${LangFileString} IDS_WINAMP_PLAYLIST_NAME "$(^NameDA)"