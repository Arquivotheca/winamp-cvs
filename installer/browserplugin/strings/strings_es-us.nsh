; Language-Country:	ES-US
; LangId:			3082
; CodePage:			1252
; Revision:			1
; Last udpdated:	23-11-09
; Author:			Manuel Fernando Gutiérrez
; Email:			manufco@msn.com


!insertmacro LANGFILE_EXT "SpanishInternational"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Idioma de la instalación"
${LangFileString} IDS_INSTALLER_LANG_INFO "Por favor seleccione su idioma."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Este programa instala plugins para los Navegadores soportados que permitirán a las aplicaciones web consultar si Winamp y está instalado y cual versión, de estarlo. No se comparte información personal y pueden desinstalar los plugins cuando lo desee$\r$\n$\r$\nHaga clic en Siguiente para continuar."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "El asistente de instalación, instalará $(^NameDA) en la siguiente ubicación. \
													Para instalar en una ubicación diferente, haga clic en el botón examinar y seleccionela. \
													$\r$\n$\r$\nNota:  Algunos plugins deben ser instalados en la carpeta del Navegador."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Plugins para el Navegador"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Seleccione los Navegadores a los que se le instalará plugins."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Seleccione de la lista los plugins de la lista de Navegadores soportados encontrados en su computador \
													Haga clic en siguiente para continuar." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Seleccione plugins a instalar para el Navegador: "

${LangFileString} IDS_PAGE_FINISH_TITLE "Instalacion completa"
${LangFileString} IDS_PAGE_FINISH_TEXT "El asistente de instalación de $(^NameDA) ha completado la instalación."
${LangFileString} IDS_PAGE_FINISH_LINK "Visite winamp.com para más contenidos"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Seleccione los Navegadores de los cuales se desinstalarán plugins."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Seleccione los plugins que se desinstalaran, de siguiente lista de plugins instalados. \
											Haga clic en Siguiente para continuar." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Seleccione plugins a desinstalar del navegador:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "La desinstalación de $(^NameDA) se completó."
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) ha sido desinstalado de su computador."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Plugins desinstalados"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA) completó la desinstalación de los plugins seleccionados."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Los siguientes Navegadores deben reiniciarse para completar la desinstalación:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Enviar comentario"

!endif

${LangFileString} IDS_INSTALLER_NAME "Aplicación para detectar Winamp"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Desinstalar $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Plugin para el Navegador Mozilla Firefox"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Plugin para el Navegador Windows Internet Explorer"

${LangFileString} IDS_MESSAGE_NOWINAMP "El asistente de instalación no pudo ubicar Winamp en la ubicación especificada.$\r$\n¿Continuar de todas maneras?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "El asistente de instalación ha detectado una instalación previa en una ubicación diferente.$\r$\n\
														Se recomienda desinstalar la otra versión primero.$\r$\n¿Proceder con la desinstalación?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "D'oh, La desinstalación falló!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nada para instalar.$\r$\nContinuar de todas maneras?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Los siguientes navegadores deben ser reiniciados antes de que los plugins puedan ser usados:"

${LangFileString} IDS_DETAILS_INITIALIZING "Incializando"
${LangFileString} IDS_DETAILS_SKIPPING "Saltando"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Creando Accesos directos"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Escribiendo datos de desinstalación"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Target uninstaller newer or the same"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Instalando plugin para $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Navegador no encontrado"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Versión del navegador"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Versión del plugin"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Versión del plugin objetivo"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Plugin objetivo no encontrado"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "La versión del plugin objetivo es nueva o la misma"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Instalando plugin"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Error instalando el plugin para $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Algunos plugins aún no se desinstalan"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Selected components removed. Uninstaller retained for later use."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Desinstalando plugin para $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Removiendo desinstalador"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Imposible escribir"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Imposible registrar"

${LangFileString} IDS_DETAILS "Detalles"
${LangFileString} IDS_UNKNOWN "Desconocido"