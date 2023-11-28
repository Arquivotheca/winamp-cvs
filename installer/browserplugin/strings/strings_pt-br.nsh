; Language-Country:	PT-BR
; LangId:			1046
; CodePage:			1252
; Revision:			2
; Last udpdated:	07/12/2010
; Author:			Anderson Silva (candiba)
; Email:			candiba@gmail.com


!insertmacro LANGFILE_EXT "PortugueseBR"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Idioma do Instalador"
${LangFileString} IDS_INSTALLER_LANG_INFO "Por favor selecione um idioma."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Este instalador irá adicionar um plugin nos navegadores \
					suportados para permitir aplicações web detectar a versão do seu Winamp. \
					Nenhum dado pessoal será compartilhado e você pode desinstalar \
					o plugin quando quiser.$\r$\n$\r$\nClique em Próximo para continuar."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "O instalador irá instalar $(^NameDA) na seguinte \
					pasta. Para instalar em uma pasta diferente, clique em Procurar e selecione \
					outra pasta.  $\r$\n$\r$\nNota: Alguns plugins serão instalados \
					na pasta do navegador."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Plugins para o Navegador"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Escolha o(s) navegador(es) que deseja instalar o plugin."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Escolha o(s) navegador(s) que o $(^NameDA) \
					terá suporte. Clique em Próximo para continuar." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Selecione o(s) navegador(es) para instalar o plugin:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Instalação completada"
${LangFileString} IDS_PAGE_FINISH_TEXT "Instalação do $(^NameDA) completada com sucesso."
${LangFileString} IDS_PAGE_FINISH_LINK "Visite Winamp.com para mais ferramentas e complementos."

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Desinstalar $(^NameDA) do seu computador."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Escolha o(s) navegador(es) que o $(^NameDA) \
					será desinstalado. Clique em Desinstalar para continuar." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Selecione o(s) navegador(es) para desinstalar o plugin:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "Desinstalação do $(^NameDA) completa"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) foi desinstalado do seu computador."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Plugins desinstalados"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "Desinstalação do $(^NameDA) completa."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "É necessário reiniciar o(s) seguinte(s) \
					navegador(es) para completar a desinstalação:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Enviar Avaliação"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp Detectar Aplicação"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Desinstalar $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Plugin para o navegador Mozilla Firefox"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Plugin para o navegador Windows Internet Explorer"

${LangFileString} IDS_MESSAGE_NOWINAMP "O instalador não encontrou o local especificado para o Winamp.$\r$\nContinuar?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "O instalador encontrou outra instalação em local diferente.$\r$\n\
														É recomendado desinstalar essa outra versão primeiro.$\r$\nConfirmar desinstalação?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "D'Oh, erro ao desinstalar!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nada para instalar.$\r$\nContinuar?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "É necessário reiniciar o(s) seguinte(s) navegador(es) para ativar o plugin:"

${LangFileString} IDS_DETAILS_INITIALIZING "Inicializando"
${LangFileString} IDS_DETAILS_SKIPPING "Pulando"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Criando atalhos"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Escrevendo dados de desinstalação"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Novo ou mesmo objetivo para desinstalador"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Instalando plugin para $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Navegador não encontrado"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Versão do navegador"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Versão da origem do plugin"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Versão do objetivo do plugin"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Objetivo do plugin não encontrado"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Nova ou mesma versão do objetivo do plugin"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Instalando plugin"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Erro ao instalar plugin para $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Alguns plugins não foram desinstalados"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Removido os componentes selecionados. Retido o uso do desinstalador."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Desinstalando plugin do $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Removendo desinstalador"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Impossível escrever"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Impossível registrar"

${LangFileString} IDS_DETAILS "Detalhes"
${LangFileString} IDS_UNKNOWN "Desconhecido"

														