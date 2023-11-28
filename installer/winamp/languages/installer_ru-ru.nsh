; Language-Country:	RU-RU
; LangId:			1049
; CodePage:			1251
; Revision:			4
; Last udpdated:		09.12.2011
; Author:			Alexander Nureyev
; Email:			alexander@aol.ru

; Notes:
; use ';' or '#' for comments
; strings must be in double quotes.
; only edit the strings in quotes:
# example: ${LangFileString} installFull "Edit This Value Only!"
# Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

; History
; 04.10 > barabanger:  added 360 after Microsoft Xbox.
; 05.10 > barabanger:  in IDS_SEC_FREEFORM_DESC Winamp Bento changed to  "Bento"
; 05.10 > djegg: fixed typos in header comments, added extra notes
; 06.10 > barabanger: milkdrop2 strings added
; 27.10 > djegg: removed some trailing spaces
; 01.11 > benski: added in_flv
; 02.11 > djegg: added description for in_flv
; 15.11 > barabanger: added old os message - IDS_MSG_WINDOWS_TOO_OLD
; 14.01 > barabanger: changed winamp remote bundle text (IDS_BUNDLE1_DESCRIPTION).
; 20.03 > barabanger: added toolbar search (IDS_BUNDLE21_XXX).
; 21.03 > barabanger: added winamp search (IDS_WINAMP_SEARCH).
; 26.03 > djegg: removed "(enhanced by Google®)" from IDS_BUNDLE21_DESCRIPTION
; 02.05 > koopa: moved text "(default vis plugin) " from IDS_SEC_AVS_DESC to IDS_SEC_MILKDROP2_DESC
; 20.05 > djegg: added secSWF and IDS_SEC_SWF_DEC_DESC (possibly subject to change)
; 13.06 > barabanger: added IDS_SEC_GEN_DROPBOX & IDS_SEC_GEN_DROPBOX_DESC (subject to change)
; 17.06 > djegg: added missing SEC_ML_PLG item for Playlist Generator
; 03.07 > barabanger: changed emusic bundle text
; 24.11 > djegg: added Winamp3 section for upgrade messages
; 01/01 > djegg: added localized "built on" and "at" strings for branding text
; 15/01 > djegg: added ml_impex entry & description
; 01/02 > djegg: added AutoplayHandler
; 09/02 > djegg: added Language Selection dialog section
; 20/02 > djegg: added MB & KB to Bundle page
; 17.06 > barabanger: added orgler section name and description (IDS_SEC_ML_ORGLER)
; aug 27 2009 > benski: added IDS_RUN_OPTIMIZING
; sep 08 2009 > smontgo: changed Welcome screen per US1145 and Powerpoint deck.
; sep 17 2009 > benski: added sections & descriptions for MP4V & MKV
; sep 21 2009 > djegg: changed sec_ml_impex description
; oct 30 2009 > benski: changed in_dshow desc to IDS_SEC_DSHOW_DEC_DESC, removed AVI from desc
; oct 30 2009 > benski: added IDS_SEC_AVI_DEC_DESC for in_avi
; oct 30 2009 > djegg: changed secDSHOW description (removed AVI)
; oct 30 2009 > djegg: added secAVI
; nov 4 2009 > barabanger: added IDS_SEC_ML_ADDONS & IDS_SEC_ML_ADDONS_DESC, lines 229 & 466 (nov 6 2009 > ychen: modified description)
; nov 13 2009 > barabanger: added IDS_SEC_MUSIC_BUNDLE & DESC, lines 230 & 467. Edited "Downloading" string
; nov 22 2009 > barabanger: updated music bundle related text (see prev rec)
; nov 24 2009 > djegg: updated IDS_SEC_ML_ONLINE_DESC
; nov 30 2009 > smontgo: added IDS_DXDIST for download of DirectX9 web installer (for d3dx9 libs for osd)
; dec 01 2009 > smontgo: added IDS_DIRECTX_INSTALL_ERR to report directx download or install error
; dec 04 2009 > barabanger: removed IDS_DXDIST and IDS_DIRECTX_ISNTALL_ERR
; dec 04 2009 > barabanger: added DirectX Section: IDS_DIRECTX_*
; dec 11 2009 > smontgo: edited IDS_DIRECTX_EMBED_CONNECT_FAILED string (Your computer is missing a)
; jan 22 2010 > djegg: added IDS_CLEANUP_PLUGINS & IDS_REMOVE_SKINS to 'installation strings' (lines 255-256)
; mar 15 2010 > barabanger: new uninstaller strings (lines 469-488) // oct 4 2010 > djegg: edited lines 470+472
; may 26 2010 > djegg: added pmp_android (lines 184 & 460)
; sep 29 2010 > benski: added pmp_wifi (lines 185 & 461)
; nov 08 2010 > barabanger: updated IDS_PAGE_WELCOME_TEXT // nov 12 2010 > added extra line inbetween welcome text and bullet points // nov 19 2010 > updated welcome text
; nov 12 2010 > barabanger: Commented-out Winamp Remote from bundle page (line 322)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_DISABLE for disabling incompatible gen_lyrics plugin (line 350)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_WARNING for warning about incompatible gen_lyrics plugin (line 351)
; jun 23 2011 > djegg: changed AAC/aacPlus to HE-AAC (secAACE, line 123)

!insertmacro LANGFILE_EXT "Russian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Язык установщика"
${LangFileString} LANGUAGE_DLL_INFO "Выберите язык:"
 
${LangFileString} installFull "Полная"
${LangFileString} installStandart "Стандартная"
${LangFileString} installLite "Упрощенная"
${LangFileString} installMinimal "Минимальная"
${LangFileString} installPrevious "Предыдущая установка"

; BrandingText
${LangFileString} BuiltOn "собран"
${LangFileString} at "в"

${LangFileString} installWinampTop "Вы собираетесь установить проигрыватель Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsBundle "Данный установщик содержит полную версию с возможностью загрузки подарочной музыки."
${LangFileString} installerContainsPro "Данный установщик содержит версию Winamp Pro."
${LangFileString} installerContainsFull "Данный установщик содержит полную версию."
${LangFileString} installerContainsStandard "Данный установщик содержит стандартную версию."
${LangFileString} installerContainsLite "Данный установщик содержит упрощенную версию."
${LangFileString} licenseTop "Перед установкой прочтите и примите условия лицензионного соглашения."
${LangFileString} directoryTop "Установщик определил оптимальное размещение для установки $(^NameDA). Если требуется установить в другое место, измените путь в поле ниже."
${LangFileString} verHistHeader "Изменения версии"
${LangFileString} verHistHeaderSub "Изменения текущей версии проигрывателя Winamp."
${LangFileString} verHistTop "Перед продолжением ознакомьтесь с изменениями текущей версии:"
${LangFileString} verHistBottom "Изменения, выделенные красным цветом, требуют особого внимания."
${LangFileString} verHistButton "Завершить"

${LangFileString} uninstallPrompt "Проигрыватель Winamp будет удален. Продолжить?"

${LangFileString} msgCancelInstall "Отменить установку?"
${LangFileString} msgReboot "Для завершения установки требуется перезагрузить компьютер.$\r$\nПерезагрузить компьютер прямо сейчас? (Если нужно перезагрузить позже, нажмите кнопку «Нет»)"
${LangFileString} msgCloseWinamp "Перед продолжением требуется закрыть проигрыватель Winamp.$\r$\n$\r$\nНажмите кнопку «Повтор» после закрытия Winamp.$\r$\n$\r$\nНажмите кнопку «Пропустить», чтобы продолжить операцию в любом случае.$\r$\n$\r$\nНажмите кнопку «Прервать», чтобы прервать операцию."
${LangFileString} msgInstallAborted "Установка прервана пользователем"

${LangFileString} secWinamp "Winamp (обязательно)"
${LangFileString} compAgent "Агент проигрывателя Winamp"
${LangFileString} compModernSkin "Поддержка современных обложек"
${LangFileString} whatNew "История версий"
${LangFileString} uninstallWinamp "Удалить Winamp"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV и ASF)"
${LangFileString} secWMFDist "Загрузить и установить Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD, XM, S3M и IT"
${LangFileString} secOGGPlay "Ogg Vorbis"
${LangFileString} secOGGEnc "Ogg Vorbis"
${LangFileString} secAACE "HE-AAC"
${LangFileString} secMP3E "MP3"
${LangFileString} secMP4E "MP4"
${LangFileString} secWMAE "WMA"
${LangFileString} msgWMAError "При установке некоторых компонентов возникли неполадки. Кодировщик Windows Media Audio не был установлен. Чтобы загрузить этот кодировщик, посетите веб-узел корпорации Майкрософт по адресу http://www.microsoft.com/windows/windowsmedia/9series/encoder и повторите попытку"
${LangFileString} secCDDA "Воспроизведение и копирование компакт-дисков"
${LangFileString} secSonicBurning "Обработчик Sonic для копирования и записи компакт-дисков"
${LangFileString} msgCDError "При установке некоторых компонентов возникли неполадки. Процесс записи и копирования компакт-дисков может выполняться с ошибками"
${LangFileString} secCDDB "CDDB для распознания компакт-дисков"
${LangFileString} secWAV "WAV, VOC, AU и AIFF"
${LangFileString} secDVD "Воспроизведение DVD (требуется сторонний DVD-декодер)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "Получить 50 бесплатных песен в формате MP3 от eMusic"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "Получить 30-дневный бесплатный ознакомительный период от AOL Music"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Получить бесплатную антивирусную защиту Active Virus Shield"
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Получить Active Security Monitor - БЕСПЛАТНО"
!endif

${LangFileString} secDSP "Подключаемый модуль «Студия обработки сигнала»"
${LangFileString} secWriteWAV "Устаревший модуль записи WAV"
${LangFileString} secLineInput "Поддержка линейного входа"
${LangFileString} secDirectSound "Поддержка вывода DirectSound"

${LangFileString} secHotKey "Поддержка глобальных сочетаний клавиш"
${LangFileString} secJmp "Подключаемый модуль «Переход к файлу»"
${LangFileString} secTray "Управление из области уведомлений"

${LangFileString} msgRemoveMJUICE "Удалить из системы поддержку MJuice?$\r$\n$\r$\nНажмите кнопку «Да», если MJF-файлы не используются в других программах, кроме Winamp."
${LangFileString} msgNotAllFiles "Не все файлы были удалены.$\r$\nЕсли нужно удалить оставшиеся файлы, сделайте это вручную."


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "Форматы DirectShow (MPG и M2V)"
${LangFileString} secAVI "AVI"
${LangFileString} secFLV "Видео Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV и MKA)"
${LangFileString} secM4V "MPEG-4 (MP4 и M4V)"

${LangFileString} secSWF "Протокол мультимедиа Flash (SWF и RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "MilkDrop"

${LangFileString} secML "Библиотека проигрывателя Winamp"
${LangFileString} secOM "Интерактивные службы"
${LangFileString} secOrb "Удаленные файлы"
${LangFileString} secWire "Каталог подкастов"
${LangFileString} secMagic "Микшер MusicIP"
${LangFileString} secPmp "Портативные плееры"
${LangFileString} secPmpIpod "Поддержка iPod®"
${LangFileString} secPmpCreative "Поддержка плееров Creative®"
${LangFileString} secPmpP4S "Поддержка Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Поддержка USB-устройств"
${LangFileString} secPmpActiveSync "Поддержка Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Поддержка Android"
${LangFileString} secPmpWifi "Поддержка Wi-Fi для Android"

${LangFileString} sec_ML_LOCAL "Локальные файлы"
${LangFileString} sec_ML_PLAYLISTS "Списки воспроизведения"
${LangFileString} sec_ML_DISC "Копирование и запись дисков"
${LangFileString} sec_ML_BOOKMARKS "Закладки"
${LangFileString} sec_ML_HISTORY "История воспроизведения"
${LangFileString} sec_ML_NOWPLAYING "Проигрывается"
${LangFileString} sec_ML_RG "Средство выравнивания громкости"
${LangFileString} sec_ML_DASH "Панели мониторинга"
${LangFileString} sec_ML_TRANSCODE "Средство перекодировки"
${LangFileString} sec_ML_PLG "Генератор списков воспроизведения"
${LangFileString} sec_ML_IMPEX "Средство импорта и экспорта для iTunes"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Установщик проигрывателя $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Выберите язык установщика"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Обработчики мультимедиа"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Вывод"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Воспроизведение звука"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Звуковые кодировщики"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Воспроизведение видео"
${LangFileString} IDS_GRP_VISUALIZATION		"Зрительные образы"
${LangFileString} IDS_GRP_UIEXTENSION		"Расширения пользовательского интерфейса"
${LangFileString} IDS_GRP_WALIB				"Библиотека проигрывателя Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Основные компоненты библиотеки"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Поддержка портативных плееров"
${LangFileString} IDS_GRP_LANGUAGES 	    "Языки"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Вывод WaveOut и MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC"
${LangFileString} IDS_SEC_MILKDROP2 	"MilkDrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Средство автозаполнения тегов"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (альфа-версия)"
;${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Дополнения Winamp"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Загрузка подарочной музыки в формате MP3"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Настройка интерактивных служб..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Проверка на работу других экземпляров Winamp..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Открытие подключения к Интернету..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Проверка подключения к Интернету..."
${LangFileString} IDS_RUN_NOINET				"Подключение к Интернету отсутствует"
${LangFileString} IDS_RUN_EXTRACT				"Извлечение"
${LangFileString} IDS_RUN_DOWNLOAD				"Загрузка"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Загрузка завершена."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Ошибка загрузки. Причина:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Загрузка отменена."
${LangFileString} IDS_RUN_INSTALL				"Установка"
${LangFileString} IDS_RUN_INSTALLFIALED			"Ошибка установки."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Файл не найден. Планирование загрузки."
${LangFileString} IDS_RUN_DONE					"Готово."
${LangFileString} IDS_RUN_OPTIMIZING			"Оптимизация..."

${LangFileString} IDS_DSP_PRESETS 	"заготовок SPS"
${LangFileString} IDS_DEFAULT_SKIN	"обложек по умолчанию"
${LangFileString} IDS_AVS_PRESETS	"заготовок AVS"
${LangFileString} IDS_MILK_PRESETS	"заготовок MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"заготовок MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Удаление подключаемых модулей..."
${LangFileString} IDS_REMOVE_SKINS		"Удаление обложек по умолчанию..."


; download
${LangFileString} IDS_DOWNLOADING	"Загрузка %s"
${LangFileString} IDS_CONNECTING	"Подключение..."
${LangFileString} IDS_SECOND		" (осталась 1 сек.)"
${LangFileString} IDS_MINUTE		" (осталась 1 мин.)"
${LangFileString} IDS_HOUR			" (остался 1 ч.)"
${LangFileString} IDS_SECONDS		" (осталось %u сек.)"
${LangFileString} IDS_MINUTES		" (осталось %u мин.)"
${LangFileString} IDS_HOURS			" (осталось %u ч.)"
${LangFileString} IDS_PROGRESS		"%s КБ (%d%%) из %s КБ на скорости %u.%01u КБ/сек."


; AutoplayHandler
${LangFileString} AutoplayHandler	"Воспроизводить,"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Установка завершена"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) был установлен на ваш компьютер.$\r$\n$\r$\n\
													Нажмите кнопку «Готово» для выхода из мастера установки."
${LangFileString} IDS_PAGE_FINISH_RUN		"Запустить проигрыватель $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"Щелкните здесь для перехода на веб-узел Winamp.com"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Добавить подарочную музыку в список воспроизведения"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Вас приветствует установщик проигрывателя $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) позволит воспроизводить музыку, видео, подкасты и интернет-радио даже на портативных устройствах.$\r$\n$\r$\n$\r$\nКлючевые особенности:$\r$\n$\r$\n  \
													•  Беспроводная синхронизация с Android$\r$\n  \
													•  Управление воспроизведением через обозреватель$\r$\n      \
													   с помощью Winamp Toolbar$\r$\n  \
													•  Возможность автозаполнения тегов метаданных$\r$\n  \
													•  Генератор списков воспроизведения поможет найти$\r$\n      \
													   песни, похожие на те, которые вам нравятся$\r$\n  \
													•  Доступ к более чем 30 000 подкастов"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"Примечание: для получения всех возможностей и \
															обложки «Bento» (рекомендуется) отметьте \
															все компоненты."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Создание ярлыков"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Укажите, где нужно создать ярлыки."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Укажите, где нужно создать ярлыки для запуска Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Создать ярлык в меню «Пуск»"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Создать ярлык на панели быстрого запуска"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Создать ярлык на рабочем столе"

; bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"Дополнительные возможности $(^NameDA)"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"Выберите дополнительные возможности, чтобы получить от $(^NameDA) больше."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Получите больше впечатлений от $(^NameDA) с помощью дополнительных возможностей."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"Размер загрузки:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"Уже установлено"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"Только для Windows XP и выше"
${LangFileString} IDS_PAGE_BUNDLE_MB		"МБ"
${LangFileString} IDS_PAGE_BUNDLE_KB		"КБ"

; bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote" ; keep for uninstaller
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Воспроизводите музыку и видео со своего домашнего компьютера через Winamp.com, \
;                                            совместимые мобильные устройства, Nintendo® Wii™, Sony® PlayStation® 3 и Microsoft® Xbox™ 360."
${LangFileString} IDS_BUNDLE2_NAME			"Winamp® Toolbar"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Управляйте проигрывателем Winamp прямо из обозревателя и \
													получите прямой доступ к интерактивным службам SHOUTcast."
${LangFileString} IDS_BUNDLE21_NAME			"Сделать Winamp® Search поставщиком поиска по умолчанию"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Используйте веб-узел Winamp Search для поиска в Интернете по умолчанию."
${LangFileString} IDS_WINAMP_SEARCH			"Winamp Search"

${LangFileString} IDS_BUNDLE3_NAME			"Получить бесплатную загрузку с веб-узла eMusic"
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Подпишитесь на 2-недельный пробный период на eMusic и загрузите 50 бесплатных песен и одну аудиокнигу. \
                                                    Они останутся вашими, даже если вы отмените подписку."

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Не удается закрыть агент проигрывателя Winamp.$\r$\n\
                                                   Убедитесь, что нет других активных учетных записей Windows.\
                                                   $\r$\n$\r$\nНажмите кнопку «Повтор» после закрытия агента.\
                                                   $\r$\n$\r$\nНажмите кнопку «Пропустить», чтобы продолжить операцию в любом случае.\
                                                   $\r$\n$\r$\nНажмите кнопку «Прервать», чтобы прервать операцию."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Эта версия Windows больше не поддерживается.$\r$\n\
                                                 Для работы $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} требуется Windows 2000 или новее."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Обнаружен сторонний несовместимый подключаемый модуль gen_msn7.dll.$\r$\n$\r$\nЭтот подключаемый модуль вызывает критическую ошибку в работе Winamp 5.57 и выше.$\r$\nОн будет отключен. Для продолжения нажмите кнопку «ОК»."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Обнаружен сторонний несовместимый подключаемый модуль gen_lyrics.dll.$\r$\n$\r$\nЭтот подключаемый модуль вызывает критическую ошибку в работе Winamp 5.59 и выше.$\r$\nОн будет отключен. Для продолжения нажмите кнопку «ОК»."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Обнаружен сторонний несовместимый подключаемый модуль gen_lyrics.dll.$\r$\n$\r$\nСтарые версии данного подключаемого модуля несовместимы с Winamp 5.6 и новее. Загрузите последнюю версию gen_lyrics.dll с веб-узла по адресу http://lyricsplugin.com."

;Winamp3
${LangFileString} msgWA3			"Обнаружен Winamp3..."
${LangFileString} msgWA3_UPGRADE		"Обнаружен Winamp3. Выполнить обновление с Winamp3 до Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}, включая перенос обложек Winamp3 в папку Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}?$\r$\nНажмите кнопку «Да», чтобы обновить (рекомендуется). Чтобы не выполнять обновление с Winamp3 до Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}, нажмите кнопку «Нет»."
${LangFileString} msgWA3_MIGRATE		"Перенос обложек..."
${LangFileString} msgWA3_REMOVE			"Удаление Winamp3..."
${LangFileString} msgWA3_REMOVE2		"Не все файлы были удалены из папки $0.$\r$\nЕсли нужно удалить оставшиеся файлы, сделайте это вручную."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Обнаружен ${DIRECTXINSTAL_WINVER_LO} или ниже"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Обнаружен ${DIRECTXINSTAL_WINVER_HI} или выше"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Проверка версии ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Требуется версия не ниже ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Не удалось определить версию ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Обнаружена версия ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Неподдерживаемая версия ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Проверка на наличие $0"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Требуется загрузка"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Проверка подключения к Интернету"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Доступна последняя версия ${DIRECTXINSTAL_DIRECTXNAME}:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Загрузка установщика ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Найдено"
${LangFileString} IDS_DIRECTX_MISSING					"Отсутствует"
${LangFileString} IDS_DIRECTX_SUCCESS					"Успешно"
${LangFileString} IDS_DIRECTX_ABORTED					"Прервано"
${LangFileString} IDS_DIRECTX_FAILED					"Ошибка"
${LangFileString} IDS_DIRECTX_DONE						"Готово"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Запуск установщика ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"Для нормальной работы ${DIRECTXINSTAL_WINAMPNAME} требуется ${DIRECTXINSTAL_DIRECTXNAME} не ниже ${DIRECTXINSTALL_DIRECTXMINVER}.$\r$\nУстановить?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"Для нормальной работы ${DIRECTXINSTAL_WINAMPNAME} требуется ${DIRECTXINSTAL_DIRECTXNAME} не ниже ${DIRECTXINSTALL_DIRECTXMINVER}"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Не удалось загрузить ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Не удалось установить ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"На компьютере отсутствует компонент ${DIRECTXINSTAL_DIRECTXNAME}, требуемый ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Не удается загрузить недостающий компонент ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Не удается установить недостающий компонент ${DIRECTXINSTAL_DIRECTXNAME}"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Ядро проигрывателя Winamp (обязательно)."
${LangFileString} IDS_SEC_AGENT_DESC			"Быстрый вызов Winamp через область уведомлений и защита сопоставления типов файлов."
${LangFileString} IDS_GRP_MMEDIA_DESC			"Обработчики мультимедиа (система ввода и вывода)."
${LangFileString} IDS_SEC_CDDB_DESC				"Получение расширенных сведений о компакт-дисках из интерактивной базы данных Gracenote."
${LangFileString} IDS_SEC_SONIC_DESC			"Поддержка копирования и записи звуковых компакт-дисков с использованием библиотеки Sonic."
${LangFileString} IDS_SEC_DSP_DESC				"Модуль DSP для применения эффектов, таких как хор, флэнджер, темп и управление высотой тона."
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Поддержка воспроизведения звука (подключаемые модули ввода: звуковые декодеры)."
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Поддержка воспроизведения форматов MP3, MP2, MP1 и AAC (обязательно)."
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Поддержка воспроизведения формата WMA (включая поддержку DRM)."
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Поддержка воспроизведения форматов MIDI (MID, RMI, KAR, MUS, CMF и другие)."
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Поддержка воспроизведения форматов трекерной музыки (MOD, XM, IT, S3M, ULT и другие)."
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Поддержка воспроизведения формата Ogg Vorbis (OGG)."
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Поддержка воспроизведения звуковых форматов MPEG-4 (MP4 и M4A)."
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Поддержка воспроизведения формата FLAC."
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Поддержка воспроизведения звуковых компакт-дисков."
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Поддержка воспроизведения файлов звукозаписи (WAV, VOC, AU, AIFF и другие)."
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Поддержка воспроизведения видео (подключаемые модули ввода: декодеры видео)."
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Поддержка воспроизведения видео в формате Windows Media Video (WMV и ASF)."
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Поддержка воспроизведения видео в формате Nullsoft Video (NSV и NSA)."
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Поддержка воспроизведения видео в форматах MPEG."
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Поддержка воспроизведения видео в форматах AVI."
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Поддержка воспроизведения видео в формате Flash VP6 (FLV)."
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Поддержка воспроизведения видео в формате Matroska (MKV)."
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Поддержка воспроизведения видео в форматах MPEG-4 (MP4 и M4V)."
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Поддержка воспроизведения потоковой передачи в формате Adobe Flash (SWF и RTMP)."
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Поддержка кодирования и перекодирования (копирование дисков и преобразование форматов)."
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Поддержка копирования дисков и перекодирования в формате WMA."
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Поддержка копирования дисков и перекодирования в формате WAV."
${LangFileString} IDS_SEC_MP3_ENC_DESC			"Поддержка копирования дисков и перекодирования в формате MP3."
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Поддержка копирования дисков и перекодирования в форматах M4A и AAC."
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Поддержка копирования дисков и перекодирования в формате FLAC."
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Поддержка копирования дисков и перекодирования в формате Ogg Vorbis."
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Подключаемые модули вывода (управляют обработкой и отправкой звука на звуковую плату)."
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Запись в WAV и MME (устарел, но некоторыми людьми используется вместо кодировщиков)."
${LangFileString} IDS_SEC_OUT_DS_DESC			"Вывод DirectSound (стандартный и необходимый подключаемый модуль вывода)."
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Устаревший вывод WaveOut (устарел и более не рекомендуется для использования)."
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Расширения пользовательского интерфейса."
${LangFileString} IDS_SEC_HOTKEY_DESC			"Поддержка управления Winamp сочетаниями клавиш через любое активное приложение."
${LangFileString} IDS_SEC_JUMPEX_DESC			"Добавление песен из списка воспроизведения и библиотеки в очередь и т. п."
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Подключаемый модуль для управления Winamp через значки области уведомлений."
${LangFileString} IDS_SEC_FREEFORM_DESC			"Необходимо для обложек (таких как Winamp Modern и Bento), работающих на обработчике Freeform."
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Подключаемые модули зрительных образов."
${LangFileString} IDS_SEC_NSFS_DESC				"Подключаемый модуль зрительных образов Nullsoft Tiny Fullscreen."
${LangFileString} IDS_SEC_AVS_DESC				"Подключаемый модуль зрительных образов Advanced Visualization Studio."
${LangFileString} IDS_SEC_MILKDROP_DESC			"Подключаемый модуль зрительных образов MilkDrop."
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Подключаемый модуль зрительных образов MilkDrop2 (по умолчанию)."
${LangFileString} IDS_SEL_LINEIN_DESC			"Поддержка линейного входа через linein:// (для приема сигнала с микрофона или линейного входа)."
${LangFileString} IDS_GRP_WALIB_DESC			"Библиотека проигрывателя Winamp."
${LangFileString} IDS_SEC_ML_DESC				"Библиотека проигрывателя Winamp (обязательно)."
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Преобразование файлов из одного формата в другой."
${LangFileString} IDS_SEC_ML_RG_DESC			"Выравнивание громкости у звуковых файлов."
${LangFileString} IDS_SEC_ML_DASH_DESC			"Получение доступа к веб-узлу библиотеки Winamp в Интернете."
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Автоматическое заполнение тегов метаданных (на платформе Gracenote)."
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Подписка и загрузка подкастов."
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Получение доступа к интерактивным службам, включая SHOUTcast, AOL Radio, Winamp Charts и др."
${LangFileString} IDS_SEC_ML_PLG_DESC			"Создание динамических списков воспроизведения (на платформе Gracenote)."
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Основные компоненты библиотеки."
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"База данных с развитой системой запросов и автосписками для локальных файлов."
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Создание, изменение и сортировка списков воспроизведения."
${LangFileString} IDS_SEC_ML_DISC_DESC			"Интерфейс в библиотеке для копирования и записи звуковых компакт-дисков."
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Создание закладок на любимые потоки, файлы и папки."
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Прямой доступ к недавно воспроизводимым потокам, локальным и удаленным файлам." 
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Получение сведений о воспроизводимой дорожке."
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Поддержка портативных плееров."
${LangFileString} IDS_SEC_ML_PMP_DESC			"Основной подключаемый модуль для портативных плееров (обязательно)."
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Поддержка iPod®."
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Поддержка портативных плееров Creative® (для Nomad™, Zen™ и MuVo™)."
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Поддержка Microsoft® PlaysForSure® (для всех плееров, совместимых с P4S)."
${LangFileString} IDS_SEC_PMP_USB_DESC			"Поддержка USB-устройств (для USB-накопителей и плееров)."
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Поддержка Microsoft® ActiveSync® (для смартфонов и КПК под управлением Windows Mobile®)."
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Поддержка Android-устройств."
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Поддержка Wi-Fi для Android."
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Альфа-версия подключаемого модуля DropBox. Используйте Сtrl+Shift+D для вызова."
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Подключаемый модуль импорта и экспорта базы данных библиотеки, совместимой с iTunes."
;${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Организация музыкального интернет-чарта на основе истории воспроизведения."
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Поиск новых подключаемых модулей и обложек для Winamp."
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Загрузка и установка бесплатной музыки из службы Winamp Music."

${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Выбор связанных программ для удаления."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Удаление $(^NameDA) из компьютера."

;${LangFileString} IDS_UNINSTALL_COMPONENTS_HEADER		"Следующие связанные программы с $(^NameDA) будут удалены. Чтобы не удалять их, снимите флажки:"
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Удаление из каталога:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Проигрыватель мультимедиа"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Удаление всех компонентов проигрывателя $(^NameDA), включая стандартные подключаемые модули."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Параметры пользователя"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Удаление параметров $(^NameDA) и параметров подключаемых модулей."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"Следующие связанные программы с $(^NameDA) будут удалены:"
; ${LangFileString} IDS_UNINSTALL_BUNDLES_FOOTER		"Примечание: данные программы можно удалить в любое время через панель управления."
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Управление проигрывателем $(^NameDA) через обозреватель и прямой доступ к интерактивным службам SHOUTcast."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Воспроизведение музыки и видео с удаленного компьютера."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Подключаемые модули Winamp Detector для обозревателей Mozilla Firefox и Internet Explorer."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"Загрузка бесплатной музыки с веб-узла eMusic.com."
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"Удаление $2..."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Отправить отзыв о $(^NameDA)"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Открыть папку $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nПримечание: не все файлы были удалены. Чтобы увидеть их, откройте папку Winamp."
