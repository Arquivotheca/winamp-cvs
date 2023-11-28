; Language-Country:	pt-BR
; LangId:			1046
; CodePage:			1252
; Revision:			34
; Last updated:	    04 de maio 2013
; Author:			Anderson Silva aka candiba
; Email:			contato.winamp.br@gmail.com

; Notes:
; use ';' or '#' for comments
; strings must be in double quotes.
; only edit the strings in quotes:
; example: ${LangFileString} installFull "Edit This Value Only!"
; Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

;------------------------------------------------------------------------------------------------------------

!insertmacro LANGFILE_EXT "PortugueseBR"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Idioma do Instalador"
${LangFileString} LANGUAGE_DLL_INFO "Selecione o idioma do Instalador:"
 
${LangFileString} installFull "Completa"
${LangFileString} installStandart "Padrão"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "Mínima"
${LangFileString} installPrevious "Instalação Anterior"

; BrandingText
${LangFileString} BuiltOn "compilado em"
${LangFileString} at "às"

${LangFileString} installWinampTop "Você irá instalar o Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsBundle "Este instalador contém o pacote para instalação Bundle."
${LangFileString} installerContainsPro "Este instalador contém a instalação da versão Pro."
${LangFileString} installerContainsFull "Este instalador contém a instalação da versão Completa."
${LangFileString} installerContainsStandard "Este instalador contém a instalação da versão Padrão."
${LangFileString} installerContainsLite "Este instalador contém a instalação da versão Lite."
${LangFileString} licenseTop "Por favor, leia e aceite os termos de licença abaixo antes de instalar."
${LangFileString} directoryTop "O instalador indica a pasta destino abaixo para o $(^NameDA). Se deseja alterar, clique em Procurar e selecione outra pasta."
${LangFileString} verHistHeader "Histórico"
${LangFileString} verHistHeaderSub "Histórico das versões do Winamp"
${LangFileString} verHistTop "Por favor veja as mudanças da versão antes de continuar:"
${LangFileString} verHistBottom "Atualizações em vermelho necessita de sua atenção em especial."
${LangFileString} verHistButton "Finalizar"

${LangFileString} uninstallPrompt "Você irá desinstalar o Winamp. Continuar?"

${LangFileString} msgCancelInstall "Cancelar instalação?"
${LangFileString} msgReboot "É preciso reiniciar para completar a instalação.$\r$\nReiniciar agora? (Para reiniciar depois, clique em Não)"
${LangFileString} msgCloseWinamp "Você deve fechar o Winamp antes de continuar.$\r$\n$\r$\n	Depois que você fechar o Winamp, clique em Tentar novamente.$\r$\n$\r$\n	Se desejar instalar de qualquer maneira, clique em Ignorar.$\r$\n$\r$\n	Se desejar cancelar a instalação, clique em Cancelar."
${LangFileString} msgInstallAborted "Instalação cancelada pelo usuário"

${LangFileString} secWinamp "Winamp (necessário)"
${LangFileString} compAgent "Agente do Winamp"
${LangFileString} compModernSkin "Suporte Skin Moderna"
${LangFileString} whatNew "O Que há de Novo"
${LangFileString} uninstallWinamp "Desinstalar Winamp"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Baixar e Instalar Formato do Windows Media"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Reprodução OGG Vorbis"
${LangFileString} secOGGEnc "Codificação OGG Vorbis"
${LangFileString} secAACE "Codificação HE-AAC"
${LangFileString} secMP3E "Codificação MP3"
${LangFileString} secMP4E "Suporte MP4"
${LangFileString} secWMAE "Codificação WMA"
${LangFileString} msgWMAError "Há um problema nos componentes instalados. Codificador WMA não está instalado. Por favor visite http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , baixe o codificador e tente novamente."
${LangFileString} secCDDA "Reprodução e extração do CD"
${LangFileString} secSonicBurning "Suporte à Gravar/Ripar via Sonic"
${LangFileString} msgCDError "Há um problema nos componentes instalados. Gravar/Ripar CD pode não funcionar corretamente."
${LangFileString} secCDDB "CDDB para identificar CDs"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"
${LangFileString} secDVD "Reprodução de DVD (necessário decodificador de DVD de terceiros)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "Ícone eMusic e Oferta de 50 MP3s Grátis!"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "Ícone AOL Music Now e Oferta de 30 dias grátis (trial)!"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Adquirir grátis proteção AV com Proteção Ativa contra Vírus!"
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Adquirir Monitor de Segurança Ativa - é GRÁTIS!"
!endif

${LangFileString} secDSP "Plugin Estúdio de Processamento de Sinal"
${LangFileString} secWriteWAV "Clássico Gravador de WAV"
${LangFileString} secLineInput "Suporte Entrada de Linha"
${LangFileString} secDirectSound "Suporte Saída DirectSound"

${LangFileString} secHotKey "Suporte Teclas de Atalho Global"
${LangFileString} secJmp "Suporte Estendido ao Buscar Arquivo"
${LangFileString} secTray "Nullsoft Controle pela Bandeja"

${LangFileString} msgRemoveMJUICE "Remover suporte MJuice do seu sistema?$\r$\n$\r$\nSelecione SIM a menos que você use arquivos MJF em outros programas além do Winamp."
${LangFileString} msgNotAllFiles "Nem todos os arquivos foram removidos.$\r$\nPara removê-los, apague-os manualmente da pasta."


${LangFileString} secNSV "Nullsoft Vídeo (NSV)"
${LangFileString} secDSHOW "Formatos DirectShow (MPG, M2V)"
${LangFileString} secAVI "Vídeo AVI"
${LangFileString} secFLV "Vídeo Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "Vídeo MPEG-4 (MP4, M4V)"

${LangFileString} secSWF "Protocolo de Mídia Flash (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Estúdio de Visualização Avançada"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Biblioteca de Mídia do Winamp"
${LangFileString} secOM "Mídia Online"
${LangFileString} secOrb "Mídia Remota"
${LangFileString} secWire "Diretório de Podcast"
${LangFileString} secMagic "MusicIP Mix"
${LangFileString} secPmp "Players Portáteis de Mídias"
${LangFileString} secPmpIpod "Suporte iPod®"
${LangFileString} secPmpCreative "Suporte players Creative®"
${LangFileString} secPmpP4S "Suporte Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Suporte Dispositivos USB"
${LangFileString} secPmpActiveSync "Suporte Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Suporte Dispositivos Android"
${LangFileString} secPmpWifi "Suporte Wi-fi Android"

${LangFileString} sec_ML_LOCAL "Mídia Local"
${LangFileString} sec_ML_PLAYLISTS "Playlists"
${LangFileString} sec_ML_DISC "Ripar & Gravar CD"
${LangFileString} sec_ML_BOOKMARKS "Favoritos"
${LangFileString} sec_ML_HISTORY "Histórico"
${LangFileString} sec_ML_NOWPLAYING "Reproduzindo Agora"
${LangFileString} sec_ML_RG "Analisador de Volume"
${LangFileString} sec_ML_DASH "Winamp Dashboard"
${LangFileString} sec_ML_TRANSCODE "Conversor de Mídia"
${LangFileString} sec_ML_PLG "Gerador de Playlist"
${LangFileString} sec_ML_IMPEX "Importar/Exportar Banco de Dados"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Instalador do $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}"
${LangFileString} IDS_SELECT_LANGUAGE  "Selecione o idioma do instalador"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Motor Multimídia"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Plugins de Saída"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Reprodução de Áudio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Codificadores de Áudio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Reprodução de Vídeo"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualização"
${LangFileString} IDS_GRP_UIEXTENSION		"Extensões da Interface do Usuário"
${LangFileString} IDS_GRP_WALIB				"Biblioteca do Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Componentes da Biblioteca de Mídia"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Suporte à Player Portátil de Mídia"
${LangFileString} IDS_GRP_LANGUAGES 	    "Pacotes de Idiomas"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Saída WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC		"Codificação WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Codificador FLAC"
${LangFileString} IDS_SEC_MILKDROP2     "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Auto-Tag"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (alpha)"
;${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Winamp Complementos"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Download Pacote de MP3"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "Plugin de Rádio Francesa"
${LangFileString} IDS_SEC_CLOUD       "Winamp Cloud"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configurando serviços online..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Checando se há outra instância do Winamp em execução..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Abrindo conexão com à internet..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Verificando se há internet disponível..."
${LangFileString} IDS_RUN_NOINET				"Sem conexão com à internet"
${LangFileString} IDS_RUN_EXTRACT				"Extraindo"
${LangFileString} IDS_RUN_DOWNLOAD				"Baixando"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Download completo."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Erro no download. Razão:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Download cancelado."
${LangFileString} IDS_RUN_INSTALL				"Instalando"
${LangFileString} IDS_RUN_INSTALLFIALED			"Erro na instalação."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Arquivo não encontrado. Download agendado."
${LangFileString} IDS_RUN_DONE					"Pronto."
${LangFileString} IDS_RUN_OPTIMIZING			"Otimizando..."

${LangFileString} IDS_DSP_PRESETS 	"Presets SPS"
${LangFileString} IDS_DEFAULT_SKIN	"skins padrão"
${LangFileString} IDS_AVS_PRESETS	"Presets AVS"
${LangFileString} IDS_MILK_PRESETS	"Presets MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"Presets MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Limpando plugins..."
${LangFileString} IDS_REMOVE_SKINS		"Removendo skins padrão..."


; download
${LangFileString} IDS_DOWNLOADING	"Baixando"
${LangFileString} IDS_CONNECTING	"Conectando ..."
${LangFileString} IDS_SECOND		" (1 segundo restante)"
${LangFileString} IDS_MINUTE		" (1 minuto restante)"
${LangFileString} IDS_HOUR			" (1 hora restante)"
${LangFileString} IDS_SECONDS		" (%u segundos restantes)"
${LangFileString} IDS_MINUTES		" (%u minutos restantes)"
${LangFileString} IDS_HOURS			" (%u horas restantes)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) de %skB @ %u.%01ukB/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Reproduzir"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Instalação Completa"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) está instalado em seu computador.$\r$\n$\r$\n\
													Clique em Terminar para fechar o instalador."
${LangFileString} IDS_PAGE_FINISH_RUN		"Abrir $(^NameDA) após fechar instalador."
${LangFileString} IDS_PAGE_FINISH_LINK		"Clique aqui para visitar Winamp.com"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Adicionar pacote de MP3 na playlist do Winamp"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Bem-vindo ao Instalador do $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) permite gerenciar sua biblioteca de mídia e ouvir rádios online.  \
											Agora com Winamp Cloud você pode ouvir músicas em casa, no trabalho, no carro ou em qualquer lugar.$\r$\n$\r$\n\
											Recursos inclusos:$\r$\n$\r$\n  \
												•  Winamp Cloud facilmente unifica sua biblioteca de música$\r$\n$\r$\n  \
												•  Gerenciar sua biblioteca de música em seus dispositivos$\r$\n$\r$\n  \
												•  Sincronizar sem fio as músicas com Winamp for Android$\r$\n$\r$\n  \
												•  Criar playlists com o Gerador Automático de Playlist$\r$\n$\r$\n  \
												•  Ouvir e inscrever-se em mais de 30 mil podcasts"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTA: Para desfrutar das novas opções e \
															design da skin Bento (recomendado), todas \
															opções devem estar marcadas."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Escolha as Opções do Iniciar"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Selecione as seguintes opções do iniciar."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Escolha as seguintes opções para configurar seu Winamp nas opções do iniciar."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Criar entrada no menu Iniciar"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Criar atalho na Inicialização Rápida"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Criar atalho no Área de Trabalho"

; bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"Obtenha o Máximo do $(^NameDA)"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"Escolha as opções adicionais abaixo e obtenha o máximo do $(^NameDA)."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Tenha uma melhor experiência no $(^NameDA) com estas opções adicionais."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"Tamanho:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"já instalado"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"somente Windows XP ou superior"
${LangFileString} IDS_PAGE_BUNDLE_MB		"MB"
${LangFileString} IDS_PAGE_BUNDLE_KB		"KB"

; bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote"
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Desfrute de suas músicas e vídeos em qualquer lugar. Transmita sua música de casa pelo Winamp.com, \
                                            compatibilidade com dispositivos móveis, Nintendo® Wii™, Sony® PlayStation® 3 e Microsoft® Xbox™ 360."
${LangFileString} IDS_BUNDLE2_NAME			"Winamp® Toolbar"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Controle Winamp direto de seu navegador com acesso imediato \
													ao SHOUTcast."
${LangFileString} IDS_BUNDLE21_NAME			"Winamp® Search como sistema padrão de busca"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Deixe o Winamp lhe ajudar a buscar na web via Winamp Search."
${LangFileString} IDS_WINAMP_SEARCH			"Winamp Search"

${LangFileString} IDS_BUNDLE3_NAME			"50 Downloads de MP3 grátis +1 Audiolivro grátis no eMusic"
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Tenha 2 semanas trial grátis no eMusic e faça até 50 downloads de MP3 e 1 audiolivro \
                                             grátis. As músicas baixadas e o audiolivro serão seus, mesmo se você cancelar."

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Não foi possível fechar o Agente do Winamp.$\r$\n\
                                                   Verifique se há outro usuário logado no Windows.\
                                                   $\r$\n$\r$\n	Depois de fechado o Agente do Winamp, clique em Repetir.\
                                                   $\r$\n$\r$\n	Se desejar instalar de qualquer maneira, clique em Ignorar.\
                                                   $\r$\n$\r$\n	Se desejar cancelar a instalação, clique em Anular"

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Esta versão do Windows não é mais suportada.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} requer no mínimo o Windows XP ou superior."
                                                 
; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Incompatibilidade com o plugin de terceiros gen_msn7.dll detectada!$\r$\n$\r$\nEste plugin trava o Winamp 5.57 ou anterior ao executá-lo.$\r$\nPlugin será desativado agora. Clique em OK para confirmar."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Incompatibilidade com o plugin de terceiros gen_lyrics.dll detectada!$\r$\n$\r$\nEste plugin trava o Winamp 5.59 ou anterior ao executá-lo.$\r$\nPlugin será desativado agora. Clique em OK para confirmar."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Plugin de terceiros gen_lyrics detectado!$\r$\n$\r$\nVersões antigas deste plugin são incompativeis com Winamp 5.6 e superior. Certifique-se de que você tem a versão mais recente de http://lyricsplugin.com antes de continuar."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"Incompatibilidade com o plugin de terceiros gen_lyrics_ie.dll detectada!$\r$\n$\r$\nEste plugin faz com que o Winamp trave.$\r$\nPlugin será desativado agora. Clique em OK para confirmar."

;Winamp3
${LangFileString} msgWA3			"Winamp3 detectado..."
${LangFileString} msgWA3_UPGRADE		"Winamp3 detectado. Atualizar o Winamp3 para Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} e migrar as skins para Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}?$\r$\n(Selecionando Sim irá atualizar e é recomendado, selecionando Não irá manter separadas as instalações do Winamp3 e Winamp 5.x)"
${LangFileString} msgWA3_MIGRATE		"Migrando Skins..."
${LangFileString} msgWA3_REMOVE			"Removendo instalação do Winamp3..."
${LangFileString} msgWA3_REMOVE2		"Os arquivos da '$0' não foram removidos.$\r$\nSe desejar remover os arquivos, apague-os manualmente da pasta."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Detectado ${DIRECTXINSTAL_WINVER_LO} ou inferior"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Detectado ${DIRECTXINSTAL_WINVER_HI} ou superior"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Checando versão ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Versão mínima requerida é ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Impossível detectar versão ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Versão detectada ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Versão sem suporte ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Checando se $0 esta presente"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Download requerido"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Verificando conexão com a internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Última versão do ${DIRECTXINSTAL_DIRECTXNAME} disponível em:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Baixando instalador ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Encontrado"
${LangFileString} IDS_DIRECTX_MISSING					"Faltando"
${LangFileString} IDS_DIRECTX_SUCCESS					"Sucesso"
${LangFileString} IDS_DIRECTX_ABORTED					"Cancelado"
${LangFileString} IDS_DIRECTX_FAILED					"Erro"
${LangFileString} IDS_DIRECTX_DONE						"Pronto"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Executando instalador ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} requer a última versão ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} para trabalhar perfeitamente.$\r$\nInstale agora?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} requer a última versão ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} para trabalhar perfeitamente"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Impossível baixar ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Impossível instalar ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"No seu computador está faltando ${DIRECTXINSTAL_DIRECTXNAME}, componente requerido por ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Impossível baixar ${DIRECTXINSTAL_DIRECTXNAME} componente faltando"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Impossível instalar ${DIRECTXINSTAL_DIRECTXNAME} componente faltando"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Instalando $(IDS_SEC_GEN_FRENCHRADIO)..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp (necessário)"
${LangFileString} IDS_SEC_AGENT_DESC			"Agente do Winamp, para acesso rápido pela bandeja do sistema e manutenção das associações de arquivos"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Motor Multimídia (sistema de Entrada/Saída)"
${LangFileString} IDS_SEC_CDDB_DESC				"Suporte CDDB, para buscar automaticamente títulos de CDs no banco de dados online da Gracenote"
${LangFileString} IDS_SEC_SONIC_DESC			"Biblioteca Sonic, necessário para Ripar && Gravar CDs de Áudio"
${LangFileString} IDS_SEC_DSP_DESC				"Plugin DSP para aplicar efeitos extras como: coro, flanger, tempo e pitch control"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Suporte a Reprodução de Áudio (Plugins de Entrada: Decodificadores de Áudio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Suporte para reprodução dos formatos MP3, MP2, MP1, AAC (necessário)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Suporte para reprodução do formato WMA (incluindo suporte à DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Suporte para reprodução dos formatos MIDI (MID, RMI, KAR, MUS, CMF e mais)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Suporte para reprodução dos formatos de Módulo (MOD, XM, IT, S3M, ULT e mais)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Suporte para reprodução do formato Ogg Vorbis (OGG, OGA)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Suporte para reprodução dos formatos Áudio MPEG-4 (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Suporte para reprodução do formato FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Suporte para reprodução a CDs de Áudio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Suporte para reprodução dos formatos Waveform (WAV, VOC, AU, AIFF e mais)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Suporte a Reprodução de Vídeo (Plugins de Entrada: Decodificadores de Vídeo)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Suporte para reprodução dos formatos Windows Media Video (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Suporte para reprodução do formato Nullsoft Vídeo (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Suporte para reprodução do formato MPEG-1/2 e outros formatos de vídeos"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Suporte para reprodução de Vídeo em AVI"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Suporte para reprodução de Vídeo em Flash (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Suporte para reprodução de Vídeo Matroska (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Suporte para reprodução de Vídeo em MPEG-4 (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Suporte para reprodução do formato de transmissão Adobe Flash (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Suporte a Codificar e Converter (necessário para ripar CD e converter um formato de arquivo para outro)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Suporte para ripar e converter para formato WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Suporte para ripar e converter para formato WAV"
${LangFileString} IDS_SEC_MP3_ENC_DESC			"Suporte para ripar e converter para formato MP3"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Suporte para ripar e converter para formatos M4A e AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Suporte para ripar e converter para formato FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Suporte para ripar e converter para formato Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Plugins de Saída (controla como o áudio é processado e enviado a sua placa de som)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Gravador Clássico WAV/MME (desaprovado, mas há usuários que preferem usá-lo ao invés de decodificadores)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Saída DirectSound (necessário / plugin padrão de Saída)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Saída WaveOut clássica (opcional, e não muito recomendado ou necessário)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Extensões da Interface do Usuário"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Plugin Teclas de Atalho Global, para controlar o Winamp com teclado multimídia sobre qualquer aplicação"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Suporte Estendido ao Buscar Arquivo, para filas de músicas na playlist e muito mais"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin para adicionar ícones de controle da reprodução pela bandeja do sistema"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Suporte a Skin Moderna, necessário para usar skins de formas livre como Winamp Modern e Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Plugins de Visualização"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin de Visualização Tiny Fullscreen"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin Estúdio de Visualização Avançada"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin de Visualização MilkDrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Plugin de Visualização MilkDrop2 (plugin padrão)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Suporte a Linha Entrada usando comando linein://  (aplica visualizador a entrada de mic/line)"
${LangFileString} IDS_GRP_WALIB_DESC			"Biblioteca do Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Biblioteca de Mídia do Winamp (necessário)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Ferramenta de Conversão, usada para converter o arquivo de um formato para outro"
${LangFileString} IDS_SEC_ML_RG_DESC			"Ferramenta de Análise de Volume, usada para normalização do volume"
${LangFileString} IDS_SEC_ML_DASH_DESC			"Dashboard, um portal opcional para a Biblioteca de Mídia"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-Tag para arquivos com Tag inválida/faltando. (por Gracenote)"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Diretório de Podcast, para subscrever e baixar podcasts"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Serviços Online, incluindo Rádio SHOUTcast && TV, Rádio AOL pela Rádio CBS, Winamp Charts, etc"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Gerador de Playlist do Winamp (por Gracenote), para criar playlists dinâmicas"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Componentes Essenciais da Biblioteca de Mídia"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Banco de dados de Mídia Local, com poderoso sistema de consulta e personalização de exibições inteligentes"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Gerenciador de Playlists, para criar, editar e armazenar todas suas playlists"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Ripar && Gravar CD, interface da biblioteca de mídia para ripar && gravar CDs de áudio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Gerenciador de Favoritos, para armazenar suas transmissões favoritas, arquivos, pastas, etc"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Histórico, para acesso imediato à tudo recentemente reproduzido local e ou remoto e transmissões"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Reproduzindo Agora, para exibir informação sobre a faixa atual em reprodução"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Suporte Player Portátil de Mídia"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plugin Essencial de Suporte à Player Portátil de Mídia (necessário)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Suporte iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Suporte para portatéis Creative® (gerenciar players Nomad™, Zen™ e MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Suporte Microsoft® PlaysForSure® (gerenciar todos players P4S compatíveis)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Suporte Dispositivo USB (gerenciar pendrives e players usb)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Suporte para Microsoft® ActiveSync® (gerenciar Windows Mobile®, Smartphone && dispositivos Pocket PC)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Suporte para dispositivos Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Suporte Wi-fi Android"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Versão alpha do próximo plugin dropbox. Usar ctrl+shift+d para ativar"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Plugin importar/exportar banco de dados da Biblioteca de Mídia com compatibilidade com iTunes"
;${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Winamp Orgler™ Permite rastrear, mapear e compartilhar o que você escuta"
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Descubra Complementos e adicione as extensões ao Winamp media player"
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Baixe e reproduza música grátis do Winamp Music"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Ouça mais de 300 estações de rádio francesas, ao vivo no $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"
${LangFileString} IDS_SEC_CLOUD_DESC			"Winamp nessa Nuvem que todo mundo está falando"


${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Escolha os programas relacionados para desinstalar."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Remover $(^NameDA) do seu computador."

;${LangFileString} IDS_UNINSTALL_COMPONENTS_HEADER		"O seguinte programa relacionado, o $(^NameDA) será removido. Para cancelar, desmarque a opção correspondente:"
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Pasta Destino da Desinstalação:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Media Player"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Remove todos os componentes do $(^NameDA) Media Player incluindo os plugins de terceiros."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Preferências do Usuário"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Remove todas as preferências e plugins do $(^NameDA)."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"Selecione abaixo o(s) programa(s) relacionado(s) ao Winamp para ser(em) removido(s):"
; ${LangFileString} IDS_UNINSTALL_BUNDLES_FOOTER		"Nota: É possível desinstalar estes programas também via Painel de Controle do Windows."
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Controle $(^NameDA) diretamente de seu browser e tenha acesso imediato ao SHOUTcast."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Desfrute de suas músicas e vídeos em qualquer lugar."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Plugin Detectar Winamp para Mozilla Firefox e Internet Explorer."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"Downloads de Músicas, Audiobooks em MP3 via eMusic.com"
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"Desinstalando $2..."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Ajude o $(^NameDA) por meio de envio de feedback"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Abrir pasta do $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNota:  Nem todos os arquivos foram removidos. Para removê-los, abra a pasta do Winamp."
