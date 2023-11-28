; Languge-Culture:		FR-FR
; LangId:			1036
; CodePage:			1252
; Revision:			5.59
; Last updated:			17.10.2010
; Author:			Todae
; Email:			veekee@todae.fr


!insertmacro LANGFILE_EXT "French"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Langue de l'installation"
${LangFileString} IDS_INSTALLER_LANG_INFO "Veuillez sélectionner la langue utilisée par le programme d'installation."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Ce programme installe les plugins des navigateurs supportés \
													pour permettre aux applications web de vérifier la présence de Winamp \
													ainsi que sa version. Aucune donnée personnelle n'est transmise et vous pouvez désinstaller ces plugins à n'importe quel moment.$\r$\n$\r$\nCliquez sur Suivant pour continue."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "L'assistant installera $(^NameDA) dans le dossier suivant. \
													Pour utiliser un autre répertoire, cliquez sur le bouton Parcourir pour en sélectionner un autre. \
													$\r$\n$\r$\nNote : Certains plugins seront installés dans le dossier des navigateurs."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Plugins pour navigateurs"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Sélectionnez les navigateurs supportés par $(^NameDA)."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Choisissez les plugins dans la liste des navigateurs détectés sur votre ordinateur. \
													Cliquez sur Suivant pour continuer." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Sélectionnez les navigateurs concernés :"

${LangFileString} IDS_PAGE_FINISH_TITLE "Installation terminée"
${LangFileString} IDS_PAGE_FINISH_TEXT "L'assistant de $(^NameDA) a terminé l'installation des plugins pour les navigateurs sélectionnés."
${LangFileString} IDS_PAGE_FINISH_LINK "Allez sur Winamp.com pour faire un plein de fonctionnalités"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Supprime $(^NameDA) de votre ordinateur."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Choisissez les navugateur pour lesquels supprimer le support de $(^NameDA) \
													. Cliquez sur Suivant pour continuer." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Sélectionnez les navigateurs concernés :"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "Désinstallation de $(^NameDA) terminée"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) a été désinstallé de votre ordinateur."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Plugins désinstallés"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "L'assistant de $(^NameDA) a terminé la désinstallation des plugins sélectionnés."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Nous recommandons que vous redémarrions ces navigateurs pour finaliser la désinstallation :"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Envoyez votre avis"

!endif

${LangFileString} IDS_INSTALLER_NAME "Détection de l'application Winamp"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Désinstaller $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Plugin pour le navigateur Mozilla Firefox"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Plugin pour le navigateur Windows Internet Explorer"

${LangFileString} IDS_MESSAGE_NOWINAMP "L'assistant n'a pu trouver Winamp dans le dossier spécifié.$\r$\nVoulez-vous tout de même continuer ?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "L'assistant a été détecté une installation précédente dans un dossier différent.$\r$\n\
														Il est recommandé de préalablement désinstaller la précédente version.$\r$\nVoulez-vous la désinstaller ?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "T'oh, la désinstallation ne marche pas !"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Vous n'avez rien sélectionné.$\r$\nVoulez-vous tout de même continuer ?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Les navigateurs suivants doivent être redémarrés pour que les plugins soient utilisables :"

${LangFileString} IDS_DETAILS_INITIALIZING "Initialisation"
${LangFileString} IDS_DETAILS_SKIPPING "Ignore"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Création des raccourcis"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Ecriture des informations de désinstallation"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "La désinstallation cible est d'une version plus récente ou équivalente"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Installation du plugin pour $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Navigateur non trouvé"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Version du navigateur"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Version originale du plugin"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Version cible du plugin"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Plugin cible non trouvé"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "La version du plugin cble est plus récente ou équivalente"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Installation du plugin"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Erreur lors de l'installation du plugin pour $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Certains plugins ne sont toujours pas désinstallés"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Les plugins sélectionnés ont été supprimés. Le programme de désinstallation a été laissé."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Désinstallation du plugin pour $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Suppression de la désinstallation"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Impossible d'écrire"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Impossible d'enregistrer"

${LangFileString} IDS_DETAILS "Détails"
${LangFileString} IDS_UNKNOWN "Inconnu"

														