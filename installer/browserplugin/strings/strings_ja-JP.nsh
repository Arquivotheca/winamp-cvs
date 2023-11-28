; Language-Country:	ja-JP
; LangId:			1033
; CodePage:			932
; Revision:			1
; Last udpdated:	06.12.2009
; Author:			T-Matsuo(win32lab.com)
; Email:			tms3@win32lab.com


!insertmacro LANGFILE_EXT "Japanese"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "インストーラ言語"
${LangFileString} IDS_INSTALLER_LANG_INFO "言語を選択してください。"

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "サポートしているブラウザが Winamp 検出とバージョン情報を \
					ローカルコンピュータから取得するためのプラグインをインストールします。 \
					どんな個人情報も共有されません。これらのプラグインはいつでも \
					アンインストールできます。$\r$\n$\r$\n続行するには「次へ」をクリックしてください。"

${LangFileString} IDS_PAGE_DIRECTORY_TOP "セットアップは $(^NameDA) を以下のフォルダーにインストールします。 \
													別のフォルダーにインストールするには、「参照」ボタンをクリックしてください。 \
													$\r$\n$\r$\nNote:  いくつかのプラグインはブラウザフォルダーにインストールしなければなりません。"

${LangFileString} IDS_PAGE_COMPONENT_HEADER "ブラウザプラグイン"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "プラグインをインストールするブラウザを選んでください。"
${LangFileString} IDS_PAGE_COMPONENT_TOP "コンピュータで検出されたサポートしているブラウザのリストからプラグインを選んでください。 \
													続行するには「次へ」ボタンをクリックしてください。" 
${LangFileString} IDS_PAGE_COMPONENT_LIST "インストールするブラウザプラグインを選択:"

${LangFileString} IDS_PAGE_FINISH_TITLE "プラグインインストール完了"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA) セットアップはブラウザプラグインのインストールを完了しました。"
${LangFileString} IDS_PAGE_FINISH_LINK "よりすばらしいもののためにwinamp.comを訪問してください。"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "プラグインをアンインストールするブラウザを選んでください。"
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "アンインストールしたいインストールされたプラグインのリストからプラグインを選んでください。 \
											続行するには「次へ」ボタンをクリックしてください。" 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "選択ブラウザプラグインのアンインストール:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "$(^NameDA) アンインストール完了"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) をコンピュータからアンインストールしました。"
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "プラグインのアンインストール"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA)は、選択されたプラグインのアンインストールを完了しました。"
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "アンインストールを完了するにはブラウザを再起動する必要があります:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "フィードバックの送信"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp アプリケーション検出"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "$(IDS_INSTALLER_NAME) のアンインストール"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Mozilla Firefox ブラウザプラグイン"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Windows Internet Explorer ブラウザプラグイン"

${LangFileString} IDS_MESSAGE_NOWINAMP "セットアップは指定された位置で Winamp の場所を見つけることができませんでした。$\r$\n続行しますか？"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "セットアップは別の場所に前回のインストールを検出しました。$\r$\n\
														先に古いバージョンをアンインストールすることをお勧めします。$\r$\nアンインストールを続けますか？"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "アンインストールに失敗しました！"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "インストール対象がありません。$\r$\n続行しますか？"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "ブラウザプラグインを使用するには以下のブラウザを再起動しなければなりません:"

${LangFileString} IDS_DETAILS_INITIALIZING "初期化中"
${LangFileString} IDS_DETAILS_SKIPPING "省略"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "ショートカットを作成"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "アンインストール情報を書き込み"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Target uninstaller newer or the same"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "プラグイン $0 をインストール中"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "ブラウザを検出できず"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "ブラウザバージョン"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "ソースプラグインのバージョン"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "ターゲットプラグインのバージョン"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "ターゲットプラグインを検出できず"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Target plugin version newer or the same"
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "プラグインのインストール中"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "プラグイン $0 のインストールエラー"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "いくつかのプラグインはアンインストールされませんでした"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "選択されたコンポーネントは削除されました。アンインストーラは後の使用のために保持されます。"
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "プラグイン $0 のアンインストール中"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "アンインストーラの削除"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "書き込みエラー"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "登録エラー"

${LangFileString} IDS_DETAILS "詳細"
${LangFileString} IDS_UNKNOWN "不明"

														