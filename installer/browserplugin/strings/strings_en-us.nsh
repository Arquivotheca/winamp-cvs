; Language-Country:	EN-US
; LangId:			1033
; CodePage:			1252
; Revision:			
; Last udpdated:	
; Author:			Nullsoft
; Email:


!insertmacro LANGFILE_EXT "English"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Installer Language"
${LangFileString} IDS_INSTALLER_LANG_INFO "Please select a language."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "This program installs plug-ins for your supported \
					browsers to allow web applications  to detect Winamp and its version on \
					this computer.  No personal data is shared and you can uninstall these \
					plug-ins at anytime.$\r$\n$\r$\nClick Next to continue."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "Setup will install the $(^NameDA) to the following \
					destination folder. To change install location, click Browse and select \
					another folder.  $\r$\n$\r$\nNote:  Some plug-ins will be installed into \
					the browser folder."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Browser Plug-ins"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Choose browsers to install plug-ins for."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Choose which browsers the $(^NameDA) will support. \
					Click Next to continue." 
${LangFileString} IDS_PAGE_COMPONENT_LIST "Select browser plug-ins to install:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Setup complete"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA) setup has successfully completed."
${LangFileString} IDS_PAGE_FINISH_LINK "Visit Winamp.com for more tools and add-ons."

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Remove $(^NameDA) from your computer."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Choose which browsers the $(^NameDA) will uninstall \
					for.  Click Uninstall to continue." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Select browser plug-ins to uninstall:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "$(^NameDA) Uninstall Completed"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) has successfully uninstalled."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Plug-ins unistalled"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA) completed uninstalling selected plug-ins."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "We recommend that following browsers be \
					restarted for the uninstall to take place:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Send Feedback"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp Detector Plug-in"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "Uninstall $(IDS_INSTALLER_NAME)"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Mozilla Firefox browser plugin"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Windows Internet Explorer browser plugin"

${LangFileString} IDS_MESSAGE_NOWINAMP "Setup was unable to locate Winamp at the specified location.$\r$\nContinue anyway?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "Setup detected a previous installation at a different location.$\r$\n\
														It is recommended to uninstall the other version first.$\r$\nProceed with uninstall?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "D'oh, Uninstall failed!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Nothing to install.$\r$\nContinue anyway?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "The following browsers must be restarted before the plug-in can be used:"

${LangFileString} IDS_DETAILS_INITIALIZING "Initializing"
${LangFileString} IDS_DETAILS_SKIPPING "Skipping"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Creating shortcuts"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Writing uninstall data"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Target uninstaller newer or the same"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "Installing plugin for $0"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Browser not found"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Browser version"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Source plugin version"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Target plugin version"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Target plugin not found"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Target plugin version newer or the same"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Installing plugin"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "Error installing plugin for $0"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Some plugins still not uninstalled"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Selected components removed. Uninstaller retained for later use."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "Uninstalling plugin for $0"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Removing uninstaller"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Unable to write"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Unable to register"

${LangFileString} IDS_DETAILS "Details"
${LangFileString} IDS_UNKNOWN "Unknown"

														