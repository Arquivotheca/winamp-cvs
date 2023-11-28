; Language-Country: pl-PL
; LangId:		    1045
; CodePage:		    1250
; Last udpdated:    2013.05.05
; Author:		    Paweł Porwisz aka Pepe
; Email:            pawelporwisz@gmail.com
; Website:			http://www.pawelporwisz.pl

; Notes:
; Use ';' or '#' for comments
; Strings must be in double quotes.
; Only edit the strings in quotes:
; Example: ${LangFileString} installFull "Edit This Value Only!"
; Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

;------------------------------------------------------------------------------------------------------------


!insertmacro LANGFILE_EXT "Polish"
 
; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Język instalatora"
${LangFileString} LANGUAGE_DLL_INFO "Proszę wybrać język:"
 
${LangFileString} installFull "Pełna"
${LangFileString} installStandart "Standardowa"
${LangFileString} installLite "Podstawowa"
${LangFileString} installMinimal "Minimalna"
${LangFileString} installPrevious "Poprzednia instalacja"

; BrandingText
${LangFileString} BuiltOn "utworzono:"
${LangFileString} at "-"

${LangFileString} installWinampTop "Zainstalowany zostanie program Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsBundle "Instalator wersji pełnej (pakiet)"
${LangFileString} installerContainsPro "Instalator wersji profesjonalnej"
${LangFileString} installerContainsFull	"Instalator wersji pełnej"
${LangFileString} installerContainsStandard "Instalator wersji standardowej"
${LangFileString} installerContainsLite "Instalator wersji minimalnej"
${LangFileString} licenseTop "Przed instalacją przeczytaj i zaakceptuj warunki poniższej umowy."
${LangFileString} directoryTop "Instalator określił optymalną lokalizację programu $(^NameDA). Jeśli chcesz zmienić ten folder, zrób to teraz."
${LangFileString} verHistHeader "Historia wersji"
${LangFileString} verHistHeaderSub "Historia wersji programu Winamp"
${LangFileString} verHistTop "Przed kontynuacją zapoznaj się ze zmianami w nowej wersji:"
${LangFileString} verHistBottom "Aktualizacje oznaczone na czerwono wymagają specjalnej uwagi."
${LangFileString} verHistButton "Zakończ"

${LangFileString} uninstallPrompt "Program Winamp zostanie odinstalowany. Kontynuować?"

${LangFileString} msgCancelInstall "Anulować instalację?"
${LangFileString} msgReboot "Aby ukończyć instalację, należy ponownie uruchomić komputer.$\r$\nUruchomić ponownie teraz? (Aby uruchomić ponownie później, kliknij przycisk Nie)"
${LangFileString} msgCloseWinamp "Aby kontynuować, musisz zamknąć Winampa.$\r$\n$\r$\nPo zamknięciu programu, naciśnij przycisk Ponów próbę.$\r$\n$\r$\nJeśli chcesz kontynuować instalację mimo to, wybierz przycisk Ignoruj.$\r$\n$\r$\nJeśli chcesz przerwać instalację, naciśnij przycisk Przerwij."
${LangFileString} msgInstallAborted "Instalacja przerwana przez użytkownika"

${LangFileString} secWinamp "Winamp (wymagany)"
${LangFileString} compAgent "Agent Winampa"
${LangFileString} compModernSkin "Obsługa skórek nowoczesnych"
${LangFileString} whatNew "Co nowego"
${LangFileString} uninstallWinamp "Odinstaluj Winampa"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Pobierz i zainstaluj format Windows Media"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Odtwarzanie OGG Vorbis"
${LangFileString} secOGGEnc "Kodowanie OGG Vorbis"
${LangFileString} secAACE "Kodowanie HE-AAC"
${LangFileString} secMP3E "Kodowanie MP3"
${LangFileString} secMP4E "Obsługa formatu MP4"
${LangFileString} secWMAE "Kodowanie WMA"
${LangFileString} msgWMAError "Wystąpiły błędy podczas instalacji składników. Koder WMA nie zostanie zainstalowany. Odwiedź stronę http://www.microsoft.com/windows/windowsmedia/forpros/encoder/default.mspx, pobierz koder i spróbuj ponownie."
${LangFileString} secCDDA "Odtwarzanie i zgrywanie płyt CD" 
${LangFileString} secSonicBurning "Obsługa zgrywania/nagrywania Sonic"
${LangFileString} msgCDError "Wystąpiły błędy podczas instalowania składników. Zgrywanie/nagrywanie płyt CD może działać nieprawidłowo. "
${LangFileString} secCDDB "Baza CDDB do rozpoznawania płyt CD"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"
${LangFileString} secDVD "Odtwarzanie płyt DVD (wymagany zewnętrzny dekoder DVD)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "Ikona eMusic oraz oferta 50 darmowych plików MP3!"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "Ikona AOL Music Now oraz oferta 30-dniowego darmowego użytkowania!"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Pobierz darmową ochronę antywirusową - Active Virus Shield!"
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Pobierz program Active Security Monitor - za DARMO!"
!endif

${LangFileString} secDSP "Studio przetwarzania sygnału"
${LangFileString} secWriteWAV "Narzędzie do zapisu plików w formacie WAV"
${LangFileString} secLineInput "Obsługa wejścia liniowego"
${LangFileString} secDirectSound "Obsługa wyjścia DirectSound"

${LangFileString} secHotKey "Obsługa globalnych klawiszy skrótów"
${LangFileString} secJmp "Obsługa rozszerzonej funkcji Skok do pliku"
${LangFileString} secTray "Kontrola z zasobnika systemowego"

${LangFileString} msgRemoveMJUICE "Usunąć obsługę formatu MJuice z systemu?$\r$\n$\r$\nWybierz opcję TAK, jeśli nie używasz plików MJF w innych programach."
${LangFileString} msgNotAllFiles "Nie wszystkie pliki zostały usunięte.$\r$\nJeśli chcesz je usunąć, zrób to teraz."


${LangFileString} secNSV "Wideo Nullsoft (NSV)"
${LangFileString} secDSHOW "Formaty DirectShow (MPG, M2V)"
${LangFileString} secAVI "Wideo AVI"
${LangFileString} secFLV "Wideo Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "Wideo MPEG-4 (MP4, M4V)"

${LangFileString} secSWF "Protokół mediów Flash (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Zaawansowane Studio Wizualizacji (AVS)"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Biblioteka mediów Winampa"
${LangFileString} secOM "Media online"
${LangFileString} secOrb "Media zdalne"
${LangFileString} secWire "Katalog podcastów"
${LangFileString} secMagic "Miksy MusicIP"
${LangFileString} secPmp "Przenośne odtwarzacze mediów"
${LangFileString} secPmpIpod "Obsługa urządzeń iPod®"
${LangFileString} secPmpCreative "Obsługa odtwarzaczy firmy Creative®"
${LangFileString} secPmpP4S "Obsługa technologii Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Obsługa urządzeń USB"
${LangFileString} secPmpActiveSync "Obsługa programu Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Obsługa urządzeń Android"
${LangFileString} secPmpWifi "Obsługa urządzeń Android Wifi"

${LangFileString} sec_ML_LOCAL "Media lokalne"
${LangFileString} sec_ML_PLAYLISTS "Listy odtwarzania"
${LangFileString} sec_ML_DISC "Zgrywanie i nagrywanie płyt CD"
${LangFileString} sec_ML_BOOKMARKS "Zakładki"
${LangFileString} sec_ML_HISTORY "Historia"
${LangFileString} sec_ML_NOWPLAYING "Teraz odtwarzane"
${LangFileString} sec_ML_RG "Analizator współczynnika wzmocnienia Replay Gain"
${LangFileString} sec_ML_DASH "Pulpit nawigacyjny Winampa"
${LangFileString} sec_ML_TRANSCODE "Narzędzie transkodowania"
${LangFileString} sec_ML_PLG "Generator list odtwarzania"
${LangFileString} sec_ML_IMPEX "Narzędzie importu/eksportu bazy danych"

; ---------------------------------------------------------------------------------------- ver 3.0

${LangFileString} IDS_CAPTION          "Instalator programu $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}"
${LangFileString} IDS_SELECT_LANGUAGE  "Wybierz język instalatora"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Aparat multimedialny"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Wyjście"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Odtwarzanie plików audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Kodery audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Odtwarzanie plików wideo"
${LangFileString} IDS_GRP_VISUALIZATION		"Wizualizacje"
${LangFileString} IDS_GRP_UIEXTENSION		"Rozszerzenia interfejsu użytkownika"
${LangFileString} IDS_GRP_WALIB				"Biblioteka Winampa"
${LangFileString} IDS_GRP_WALIB_CORE		"Główne składniki biblioteki mediów"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Obsługa przenośnych odtwarzaczy mediów"
${LangFileString} IDS_GRP_LANGUAGES 	    "Pakiety językowe"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Wyjście MME/WaveOut"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Kodowanie FLAC"
${LangFileString} IDS_SEC_MILKDROP2     "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Automatyczne dodawanie znaczników"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (alfa)"
; ${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Dodatki Winampa"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Pobierz dodatkowe MP3"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "Wtyczka radia francuskiego"
${LangFileString} IDS_SEC_CLOUD       "Chmura Winampa"

; Installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Konfigurowanie usług online..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Sprawdzanie, czy uruchomione jest inne wystąpienie programu Winamp..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Otwieranie połączenia internetowego..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Sprawdzanie dostępności połączenia internetowego..."
${LangFileString} IDS_RUN_NOINET				"Brak połączenia internetowego"
${LangFileString} IDS_RUN_EXTRACT				"Rozpakowywanie"
${LangFileString} IDS_RUN_DOWNLOAD				"Pobieranie"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Pobieranie zostało ukończone."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Pobieranie nie powiodło się. Powód:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Pobieranie zostało anulowane."
${LangFileString} IDS_RUN_INSTALL				"Instalowanie"
${LangFileString} IDS_RUN_INSTALLFIALED			"Instalacja nie powiodła się."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Nie odnaleziono pliku. Planowanie pobierania."
${LangFileString} IDS_RUN_DONE					"Ukończono."
${LangFileString} IDS_RUN_OPTIMIZING			"Optymalizacja..."

${LangFileString} IDS_DSP_PRESETS 	"ustawień SPS"
${LangFileString} IDS_DEFAULT_SKIN	"domyślnych skórek"
${LangFileString} IDS_AVS_PRESETS	"wizualizacji AVS"
${LangFileString} IDS_MILK_PRESETS	"wizualizacji MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"wizualizacji MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Usuwanie wtyczek..."
${LangFileString} IDS_REMOVE_SKINS		"Usuwanie domyślnych skórek..."

; Download
${LangFileString} IDS_DOWNLOADING	"Pobieranie"
${LangFileString} IDS_CONNECTING	"Łączenie..."
${LangFileString} IDS_SECOND		" (pozostała 1 sekunda)"
${LangFileString} IDS_MINUTE		" (pozostała 1 minuta)"
${LangFileString} IDS_HOUR			" (pozostała 1 godzina)"
${LangFileString} IDS_SECONDS		" (pozostało %u sekund)"
${LangFileString} IDS_MINUTES		" (pozostało %u minut)"
${LangFileString} IDS_HOURS			" (pozostało %u godzin)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) z %skB przy %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler	"Odtwórz"

; ----------------------------------------------------------------------------------------
; Pages
; Finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Instalacja ukończona"
${LangFileString} IDS_PAGE_FINISH_TEXT		"Program $(^NameDA) został zainstalowany na komputerze.$\r$\n$\r$\n\
													Kliknij przycisk Zakończ, aby zamknąć kreatora."
${LangFileString} IDS_PAGE_FINISH_RUN		"Uruchom program $(^NameDA) po zamknięciu instalatora"
${LangFileString} IDS_PAGE_FINISH_LINK		"Kliknij tutaj, aby odwiedzić witrynę Winamp.com"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Dodaj dodatkowe pliki MP3 do listy odtwarzania Winampa"

; Welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Witamy w kreatorze instalacji programu $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) pozwala na zarządzanie biblioteką multimediów oraz słuchanie radia internetowego.  \
											Dzięki usłudze Winamp Cloud możesz słuchać swojej muzyki w domu, pracy, samochodzie lub gdziekolwiek masz ochotę.$\r$\n$\r$\n\
											Korzystaj z poniższych funkcjonalności:$\r$\n$\r$\n  \
												•  Usługa Winamp Cloud unifikuje Twoją bibliotekę muzyki$\r$\n$\r$\n  \
												•  Zarządzaj biblioteką muzyki na wszystkich urządzeniach$\r$\n$\r$\n  \
												•  Synchronizuj bezprzew. media z Winamp for Android$\r$\n$\r$\n  \
												•  Twórz listy odtw. w generatorze list odtwarzania$\r$\n$\r$\n  \
												•  Słuchaj i korzystaj z prawie 30,000 podcastów"
			
; Components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"UWAGA: Aby korzystać z nowych funkcji \
															i skórki Bento (zalecane), należy \
															zaznaczyć wszystkie składniki."

; Start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Wybierz opcje uruchamiania"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Dokonaj wyboru spośród następujących opcji uruchamiania."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Dokonaj wyboru spośród następujących opcji, aby skonfigurować ustawienia uruchamiania Winampa."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Utwórz skrót w menu Start"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Utwórz ikonę szybkiego uruchamiania"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Utwórz ikonę na pulpicie"

; Bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"Wykorzystaj wszystkie możliwości programu $(^NameDA)"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"Wybierz funkcje dodatkowe, aby w pełni wykorzystać możliwości programu $(^NameDA)."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Dodatkowe funkcje rozszerzające możliwości programu $(^NameDA)."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"Rozmiar:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"Już zainstalowano"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"tylko w systemie Windows XP lub nowszym"
${LangFileString} IDS_PAGE_BUNDLE_MB		"MB"
${LangFileString} IDS_PAGE_BUNDLE_KB		"KB"

; Bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote"
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Ciesz się muzyką i plikami wideo w drodze, pobierając je z sieci Web, odtwarzając za pomocą zgodnych telefonów komórkowych i \
;                                                    konsoli do gier, takich jak Nintendo® Wii™, Sony® PlayStation® 3 i Microsoft® Xbox™ 360."
${LangFileString} IDS_BUNDLE2_NAME			"Winamp® Toolbar"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Steruj programem Winamp bezpośrednio z przeglądarki internetowej i uzyskuj natychmiastowy dostęp \
													do kanałów SHOUTcast."
${LangFileString} IDS_BUNDLE21_NAME			"Ustaw AOL® Search moim domyślnym dostawcą wyszukiwania"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Wykorzystaj Winamp® Search do przeszukiwania sieci web w programie Winamp."
${LangFileString} IDS_WINAMP_SEARCH			"Wyszukiwarka Winampa"
													
${LangFileString} IDS_BUNDLE3_NAME			"50 plików MP3 i 1 Audiobook do pobrania z serwisu eMusic"
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Rozpocznij 2-tyg. okres próbnego korzystania z serwisu eMusic i pobierz 50 bezpłatnych plików MP3 \
                                            oraz 1 audiobook. Pliki te można zachować nawet po rezygnacji z korzystania z serwisu."

; Messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Nie można zamknąć Agenta Winampa.$\r$\n\
                                                   Upewnij się, że w systemie Windows nie jest zalogowany inny użytkownik.\
                                                   $\r$\n$\r$\n	Po zamknięciu Agenta Winampa, kliknij przycisk Ponów próbę.\
                                                   $\r$\n$\r$\n	Aby mimo to kontynuować instalację, kliknij przycisk Ignoruj.\
                                                   $\r$\n$\r$\n	Aby przerwać instalację, kliknij przycisk Przerwij."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Ta wersja systemu Windows nie jest już obsługiwana.$\r$\n\
                                                 Do uruchomienia $(^NameDA)a ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} wymagany jest system Windows XP lub nowszy."

${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Wykryto niekompatybilną wtyczkę gen_msn7.dll!$\r$\n$\r$\nWtyczka ta uniemożliwia prawidłowe uruchomienie Winampa 5.57 i nowszych.$\r$\nWtyczka zostanie wyłączona. Kliknij OK, aby kontynuować."
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE 	"Wykryto niekompatybilną wtyczkę gen_lyrics.dll!$\r$\n$\r$\nWtyczka ta uniemożliwia prawidłowe uruchomienie Winampa 5.59 i nowszych.$\r$\nWtyczka zostanie wyłączona. Kliknij OK, aby kontynuować."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING 	"Wykryto wtyczkę gen_lyrics.dll!$\r$\n$\r$\nStarsza wersja tej wtyczki jest niekompatybilna z Winampem w wersji 5.6 i nowszym. Przed kontynuacją upewnij się, że używasz najnowszej wersji wtyczki z http://lyricsplugin.com."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"Wykryto niekompatybilną wtyczkę gen_lyrics_ie.dll!$\r$\n$\r$\nWtyczka ta uniemożliwia prawidłowe działanie Winampa.$\r$\nWtyczka zostanie wyłączona. Kliknij OK, aby kontynuować."

; Winamp3
${LangFileString} msgWA3				"Wykryto zainstalowany program Winamp3..."
${LangFileString} msgWA3_UPGRADE		"Wykryto zainstalowany program Winamp3. Czy chcesz uaktualnić instalację programu Winamp3 do Winampa ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} oraz przenieść skórki do Winampa ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}?$\r$\n(Wybranie opcji Tak rozpocznie aktualizację, co jest zalecane, wybranie zaś opcji Nie pozwoli Ci na zachowanie instalacji programu Winamp3 oraz Winamp 5.x)"
${LangFileString} msgWA3_MIGRATE		"Przenoszenie skórek..."
${LangFileString} msgWA3_REMOVE			"Usuwanie instalacji programu Winamp3..."
${LangFileString} msgWA3_REMOVE2		"Nie wszystkie pliki z katalogu '$0' zostały usunięte.$\r$\nJeśli chcesz usunąć te pliki, możesz to zrobić ręcznie."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Wykryto wersję ${DIRECTXINSTAL_WINVER_LO} lub starszą"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Wykryto wersję ${DIRECTXINSTAL_WINVER_HI} lub nowszą"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Sprawdzanie wersji ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Wymagana jest wersja co najmniej ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Nie udało się pobrać informacji o wersji ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Wykryto wersję ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Nieobsługiwana wersja ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Sprawdzanie, czy $0 istnieje"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Wymagane jest pobranie pliku instalatora"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Sprawdzanie połączenia internetowego"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Najnowsza wersja ${DIRECTXINSTAL_DIRECTXNAME} dostępna jest na:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Pobieranie instalatora ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Znaleziono"
${LangFileString} IDS_DIRECTX_MISSING					"Brak"
${LangFileString} IDS_DIRECTX_SUCCESS					"Sukces"
${LangFileString} IDS_DIRECTX_ABORTED					"Anulowano"
${LangFileString} IDS_DIRECTX_FAILED					"Nie udało się"
${LangFileString} IDS_DIRECTX_DONE						"OK"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Uruchamianie instalatora ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"Aby działać prawidłowo instalator ${DIRECTXINSTAL_WINAMPNAME} wymaga programu w wersji co najmniej ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER}.$\r$\nZainstalować ją teraz?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"Aby działać prawidłowo instalator ${DIRECTXINSTAL_WINAMPNAME} wymaga programu w wersji co najmniej ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER}"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Nie udało się pobrać ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Nie udało się zainstalować ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Na komputerze nie znaleziono składnika ${DIRECTXINSTAL_DIRECTXNAME} wymaganego przez ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Nie udało się pobrać brakującego składnika ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Nie udało się zainstalować brakującego składnika ${DIRECTXINSTAL_DIRECTXNAME}"

; French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Trwa instalacja: $(IDS_SEC_GEN_FRENCHRADIO)..."
; ----------------------------------------------------------------------------------------
; Descriptions
${LangFileString} IDS_SEC_WINAMP_DESC			"Pliki główne programu Winamp (wymagane)"
${LangFileString} IDS_SEC_AGENT_DESC			"Agent Winampa zapewnia szybki dostęp z zasobnika systemowego i zarządza skojarzeniami z typami plików"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Aparat multimedialny (system wejście/wyjście)"
${LangFileString} IDS_SEC_CDDB_DESC				"Obsługa bazy CDDB umożliwia pobieranie tytułów albumów CD z bazy danych Gracenote"
${LangFileString} IDS_SEC_SONIC_DESC			"Biblioteka Sonic, wymagana do zgrywania i nagrywania płyt CD Audio"
${LangFileString} IDS_SEC_DSP_DESC				"Wtyczka DSP umożliwia generowanie efektów (chorus, flanger, zmiana tempa i ster. wysokością dźwięku)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Odtwarzanie plików audio (wtyczki wejściowe: dekodery audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Obsługa odtwarzania plików w formatach MP3, MP2, MP1, AAC (wymagane)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Obsługa odtwarzania plików w formacie WMA (w tym obsługa mechanizmu DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Obsługa odtwarzania plików w formacie MIDI (MID, RMI, KAR, MUS, CMF i innych)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Obsługa odtwarzania plików w formacie Module (MOD, XM, IT, S3M, ULT i innych)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Obsługa odtwarzania plików w formacie Ogg Vorbis (OGG, OGA)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Obsługa odtwarzania plików w formacie MPEG-4 Audio (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Obsługa odtwarzania plików w formacie FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Obsługa odtwarzania płyt CD Audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Obsługa odtwarzania plików w formacie Waveform (WAV, VOC, AU, AIFF i innych)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Odtwarzanie plików wideo (wtyczki wejściowe: dekodery wideo)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Obsługa odtwarzania plików w formacie Windows Media Video (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Obsługa odtwarzania plików wideo Nullsoft (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Obsługa odtwarzania plików wideo MPEG-1/2 oraz innych formatów wideo"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Obsługa odtwarzania plików wideo w formacie AVI"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Obsługa odtwarzania plików wideo w formacie Flash (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Obsługa odtwarzania plików wideo kontenera Matroska (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Obsługa odtwarzania plików wideo MPEG-4 (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Obsługa odtwarzania strumieniowego formatu Adobe Flash (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Obsługa kodowania i transkodowania (zgrywanie płyt CD i konwersja plików z jednego formatu na inny)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Obsługa zgrywania i transkodowania do formatu WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Obsługa zgrywania i transkodowania do formatu WAV"
${LangFileString} IDS_SEC_MP3_ENC_DESC			"Obsługa zgrywania i transkodowania do formatu MP3"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Obsługa zgrywania i transkodowania do formatów M4A i AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Obsługa zgrywania i transkodowania do formatu FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Obsługa zgrywania i transkodowania do formatu Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Wtyczki wyjściowe (sterujące sposobem przetwarzania i wysyłania dźwięku do karty dźwiękowej)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Tradycyjne narzędzie do zapisu plików w formacie WAV/MME (niezalecane, choć niektórzy go używają)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Wyjście DirectSound (wymagane/domyślna wtyczka wyjściowa)"	
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Tradycyjna wtyczka wyjściowa WaveOut (opcjonalna, obecnie nie jest zalecana ani wymagana)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Rozszerzenia interfejsu użytkownika"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Wtyczka globalnych klawiszy skrótów, umożliwia sterowanie Winampem, gdy aktywne są inne aplikacje"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Obsługa rozszerzonej funkcji Skok do pliku, np. do kolejkowania utworów na liście odtwarzania"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Wtyczka kontroli z poziomu zasobnika systemowego (wyświetla w nim ikony sterowania odtwarzaniem)"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Obsługa skórek nowoczesnych (dla skórek o dowolnym wyglądzie, typu Winamp Modern lub Winamp Bento)"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Wtyczki wizualizacji"
${LangFileString} IDS_SEC_NSFS_DESC				"Wtyczka wizualizacji Nullsoft Tiny Fullscreen"
${LangFileString} IDS_SEC_AVS_DESC				"Wtyczka Zaawansowane Studio Wizualizacji (AVS)"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Wtyczka wizualizacji MilkDrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Wtyczka wizualizacji MilkDrop2 (domyślna wtyczka wizualizacji)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Obsługa wejścia liniowego przy użyciu polecenia linein:// (umożliwia wizualizację wejścia mikrofonu/liniowego)"
${LangFileString} IDS_GRP_WALIB_DESC			"Biblioteka Winampa"
${LangFileString} IDS_SEC_ML_DESC				"Biblioteka mediów Winampa (wymagane)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Transkoder, służący do konwertowania pliku z jednego formatu na inny"
${LangFileString} IDS_SEC_ML_RG_DESC			"Narzędzie analizy współczynnika wzmocnienia Replay Gain, służące do regulowania poziomu głośności"
${LangFileString} IDS_SEC_ML_DASH_DESC			"Pulpit nawigacyjny, opcjonalna strona portalu dla biblioteki mediów"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Funkcja automatycznego dodawania znaczników (Gracenote Auto-Tagger), uzupełniająca metadane"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Katalog audycji podcast, umożliwiający ich subskrybowanie i pobieranie"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Usługi Online, w tym strumienie radiowe i telewizyjne SHOUTcast, Radio AOL z Radio CBS, statystki Winampa i wiele więcej"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Generator list odtwarzania (firmy Gracenote), umożliwiający tworzenie dynamicznych list odtwarzania"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Główne składniki biblioteki mediów"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Baza danych mediów lokalnych (rozbudowany system wyszukiwania i niestandardowe widoki złożone)"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Menedżer list odtwarzania (tworzenie, edytowanie i przechowywanie wszystkich list odtwarzania)"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Zgrywanie i nagrywanie płyt CD, interfejs biblioteki mediów zgrywania i nagrywania płyt CD Audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Menedżer zakładek, umożliwiający tworzenie zakładek dla ulubionych strumieni, plików lub folderów"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Historia, zapewniająca dostęp do ostatnio odtwarzanych strumieni i plików, lokalnych lub zdalnych"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Widok Teraz odtwarzane, zawierający informacje o aktualnie odtwarzanym utworze"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Obsługa przenośnych odtwarzaczy mediów"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Główna wtyczka obsługi przenośnych odtwarzaczy mediów (wymagane)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Obsługa urządzeń iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Obsługa urządzeń przenośnych Creative® (zarządzanie odtwarzaczami Nomad™, Zen™ i MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Obsługa urządzeń Microsoft® PlaysForSure® (zarządzanie wszystkimi zgodnymi z tą technologią urządzeniami MTP && P4S)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Obsługa urządzeń USB (zarządzanie zwykłymi pamięciami i odtwarzaczami USB)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Obsługa Microsoft® ActiveSync® (zarządzanie urządzeniami Windows Mobile®, Smartphone i Pocket PC)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Obsługa urządzeń Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Obsługa urządzeń Android Wifi"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Wersja alfa wkrótce dostępnej wtyczki Dropbox. Aby aktywować, użyj skrótu Ctrl-Shift-D"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Wtyczka importu/eksportu bazy danych biblioteki mediów iTunes"
; ${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Winamp Orgler™ umożliwia śledzenie, tworzenie statystyk oraz dzielenie się historią odtwarzanych utworów"
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Wtyczka pozwala na przegląd i dodawanie do Winampa dodatków rozszerzających jego możliwości"
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Pobiera i instaluje darmową muzykę z sekcji Muzyka Winampa"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Odtwarzanie ponad 300 francuskich stacji radiowych, na żywo w programie $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"
${LangFileString} IDS_SEC_CLOUD_DESC			"Usługa chmury, o której wszyscy mówią"

${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Wybierz powiązane programy do odinstalowania."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Odinstalowuje program $(^NameDA) z Twojego komputera."

;${LangFileString} IDS_UNINSTALL_COMPONENTS_HEADER		"Następujące programy powiązane z programem $(^NameDA) zostaną usunięte. Aby to zmienić, odznacz odpowiednią opcję:"
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Ścieżka dezinstalacji:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Odtwarzacz mediów"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Odinstaluj wszystkie składniki odtwarzacza $(^NameDA) (łącznie z wtyczkami innych wydawców rozprowadzanymi z Winampem)."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Preferencje użytkownika"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Odinstaluj wszystkie ustawienia oraz wtyczki programu $(^NameDA)."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"Następujące programy powiązane z programem Winamp zostaną teraz odinstalowane:"
; ${LangFileString} IDS_UNINSTALL_BUNDLES_FOOTER		"Uwaga: Programy te możesz odinstalować w dowolnej chwili, korzystając z apletu Panelu Sterowania Windows."
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Steruj programem $(^NameDA) bezpośrednio z przeglądarki internetowej i uzyskaj szybki dostęp do SHOUTcast."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Ciesz się swoją muzyką i filmami w biegu."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Wtyczki detektora Winampa dla przeglądarek Mozilla Firefox oraz Internet Explorer."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"Pobieranie muzyki, pobieranie plików MP3, pliki MP3 z serwisu eMusic.com"
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"Dezinstalacja: $2..."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Wyślij opinie o programie $(^NameDA)"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Otwórz katalog programu $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nUwaga: Nie wszystkie pliki zostały odinstalowane. Aby je zobaczyć, otwórz katalog Winampa."