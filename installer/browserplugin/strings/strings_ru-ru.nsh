; Language-Country:	RU-RU
; LangId:			1049
; CodePage:			1251
; Revision:			4
; Last udpdated:	11.10.2010
; Author:			Alexander Nureyev
; Email:			alexander@aol.ru


!insertmacro LANGFILE_EXT "Russian"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Язык установщика"
${LangFileString} IDS_INSTALLER_LANG_INFO "Выберите язык:"

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Эта программа установит подключаемые модули для поддерживаемых \
					обозревателей, чтобы позволить веб-приложениям находить Winamp и \
					определять его версию. Никакие конфиденциальные данные передаваться \
					не будут. Вы можете в любое время удалить эти подключаемые модули.$\r$\n$\r$\nДля продолжения нажмите кнопку «Далее»."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "Программа установки установит «$(^NameDA)» \
					в следующую папку. Для того, чтобы изменить путь установки, нажмите кнопку \
					«Обзор».  $\r$\n$\r$\nПримечание: некоторые подключаемые модули будут \
					установлены в папку обозревателя."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Подключаемые модули для обозревателей"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Выберите обозреватели, для которых следует установить подключаемые модули."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Выберите обозреватели, для которых следует установить «$(^NameDA)». \
					Для продолжения нажмите кнопку «Далее»." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Обозреватели, для которых нужно установить:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Установка завершена"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA) был успешно установлен."
${LangFileString} IDS_PAGE_FINISH_LINK "Перейти на Winamp.com для получения дополнений"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Удаление «$(^NameDA)» из компьютера."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Выберите обозреватели, у которых следует удалить «$(^NameDA)». \
					Для продолжения нажмите кнопку «Удалить»." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Обозреватели, у которых нужно удалить:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "$(^NameDA) удален"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) успешно был удален."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Подключаемые модули удалены"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA) завершил удаление выбранных подключаемых модулей."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Для завершения процедуры удаления \
					рекомендуется перезапустить следующие обозреватели:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Отзывы и предложения"

!endif

${LangFileString} IDS_INSTALLER_NAME "Подключаемый модуль Winamp Detector"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Удаление «$(IDS_INSTALLER_NAME)»"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Подключаемый модуль для обозревателя Mozilla Firefox."
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Подключаемый модуль для обозревателя Windows Internet Explorer."

${LangFileString} IDS_MESSAGE_NOWINAMP "Программа установки не смогла найти Winamp в указанной папке.$\r$\nПродолжить в любом случае?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "Программа установки обнаружила в другой папке раннее установленный экземпляр.$\r$\n\
														Перед установкой рекомендуется сначала удалить установленный экземпляр.$\r$\nВыполнить удаление?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "К сожалению, произошла ошибка при удалении."
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Ничего не выбрано.$\r$\nПродолжить в любом случае?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Для работы подключаемого модуля необходимо перезапустить следующие обозреватели:"

${LangFileString} IDS_DETAILS_INITIALIZING "Инициализация"
${LangFileString} IDS_DETAILS_SKIPPING "Пропускается"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Создание ярлыков"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Запись данных для процедуры удаления"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Версия программы удаления новее или такая же"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Установка подключаемого модуля для $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Обозреватель не найден"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Версия обозревателя"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Версия исходного подключаемого модуля"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Версия конечного подключаемого модуля"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Конечный подключаемый модуль не найден"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Версия конечного подключаемого модуля новее или такая же"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Установка подключаемого модуля"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Ошибка установки подключаемого модуля для $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Некоторые подключаемые модули не были удалены"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Выбранные компоненты были удалены. Программа удаления оставлена для последующего использования."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Удаление подключаемого модуля для $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Удаление программы удаления"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Не удалось выполнить запись"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Не удалось выполнить регистрацию"

${LangFileString} IDS_DETAILS "Подробности"
${LangFileString} IDS_UNKNOWN "Неизвестно"

														