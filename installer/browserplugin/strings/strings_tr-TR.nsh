; Language-Country:	TR-TR
; LangId:			1055
; CodePage:			1252
; Revision:			1
; Last udpdated:	Nov 24 2009
; Author:			Ali Sarıoğlu
; Email:			alsau@mynet.com

; History
; 2009.11.24 > Initial version

!insertmacro LANGFILE_EXT "Turkish"

!ifndef WACHK_EMBEDDED_INSTALLER

${LangFileString} IDS_INSTALLER_LANG_TITLE "Kurulum dili"
${LangFileString} IDS_INSTALLER_LANG_INFO "Lütfen bir dil seçin."

${LangFileString} IDS_PAGE_WELCOME_TITLE "$(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT  "Bu program, Winamp sunum ve sürüm bilgilerinin sorgulanması için yerel bilgisayar web sorgularına izin veren eklentileri, desteklenen web tarayıcıları için yükler. Kişisel veriler asla sorgulanmaz ve paylaşılmaz. Eklentileri istediğiniz zaman kaldırabilirsiniz.$\r$\n$\r$\nDevam etmek için İleri'yi tıklatın."

${LangFileString} IDS_PAGE_DIRECTORY_TOP "Kur, $(^NameDA) programını aşağıdaki klasöre kuracak. \
													Farklı bir klasöre kurmak isterseniz, Gözat'ı tıklatın ve klasörü seçin. \
													$\r$\n$\r$\nNot: Bazı eklentiler tarayıcının bulunduğu klasöre kurulması gerekir."

${LangFileString} IDS_PAGE_COMPONENT_HEADER "Eklenti Tarayıcısı"
${LangFileString} IDS_PAGE_COMPONENT_SUBHEADER "Eklentiler yüklenecek tarayıcıları seçin."
${LangFileString} IDS_PAGE_COMPONENT_TOP "Bilgisayarınızda bulunan web tarayıcıları tarafından desteklenen eklentileri \
													aşağıdaki listeden seçin. Devam etmek için İleri'yi tıklatın."
${LangFileString} IDS_PAGE_COMPONENT_LIST "Yüklenecek tarayıcı eklentilerini seçin:"

${LangFileString} IDS_PAGE_FINISH_TITLE "Eklentiler yüklendi"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA), tarayıcı eklentilerini yüklemeyi tamamladı."
${LangFileString} IDS_PAGE_FINISH_LINK "Daha fazlası için winamp.com'u ziyaret edin"

${LangFileString} IDS_UNPAGE_COMPONENT_SUBHEADER "Eklentileri kaldırılacak tarayıcıları seçin."
${LangFileString} IDS_UNPAGE_COMPONENT_TOP "Kaldırmak istediğiniz yüklü eklentileri aşağıdaki listeden seçin. \
											Devam etmek için İleri'yi tıklatın." 
${LangFileString} IDS_UNPAGE_COMPONENT_LIST "Kaldırılacak tarayıcı eklentilerini seçin:"

${LangFileString} IDS_UNPAGE_FINISH_TITLE "$(^NameDA) başarıyla kaldırıldı"
${LangFileString} IDS_UNPAGE_FINISH_TEXT "$(^NameDA) bilgisayarınızdan kaldırıldı."
${LangFileString} IDS_UNPAGE_FINISH_TITLE_PARTIAL "Eklentiler kaldırıldı"
${LangFileString} IDS_UNPAGE_FINISH_TEXT_PARTIAL "$(^NameDA), seçili eklentileri kaldırdı."
${LangFileString} IDS_UNPAGE_FINISH_BROWSERRESTART "Kaldırma işleminin tamamlanabilmesi için aşağıdaki tarayıcıların yeniden başlatılması gerekir:"
${LangFileString} IDS_UNPAGE_FINISH_LINK "Geri Bildirim İletisi Gönder"

!endif

${LangFileString} IDS_INSTALLER_NAME "Winamp Algılayıcı"
${LangFileString} IDS_INSTALLER_PUBLISHER "Nullsoft, Inc"

${LangFileString} IDS_UNINSTAL_SHORTCUT "$(IDS_INSTALLER_NAME) Kaldır"


${LangFileString} IDS_GROUP_WINAMP_DETECT "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_GROUP_WINAMP_DETECT_DESC "$(IDS_INSTALLER_NAME)"
${LangFileString} IDS_SECTION_FIREFOX "Mozilla Firefox"
${LangFileString} IDS_SECTION_FIREFOX_DESC "Mozilla Firefox tarayıcı eklentisi"
${LangFileString} IDS_SECTION_EXPLORER "Windows Internet Explorer"
${LangFileString} IDS_SECTION_EXPLORER_DESC "Windows Internet Explorer tarayıcı eklentisi"

${LangFileString} IDS_MESSAGE_NOWINAMP "Kur, belirtilen konumda Winamp'ı bulamadı.$\r$\nDevam etmek istiyor musunuz?"
${LangFileString} IDS_MESSAGE_SUGGEST_UNINSTAL "Kur, önceki kurulumun farklı bir konumda olduğunu algıladı.$\r$\n\
														Devam etmeden önce önceki sürümü kaldırmanız önerilir.$\r$\nKaldırılsın mı?"
${LangFileString} IDS_MESSAGE_UNINSTALL_FAILED "Hoop, Kaldırma hatası!"
${LangFileString} IDS_MESSAGE_EMPTYSELECTION "Hiçbir şey yüklenmedi.$\r$\nDevam etmek istiyor musunuz?"
${LangFileString} IDS_MESSAGE_BROWSERRESTART "Aşağıdaki tarayıcılar, tarayıcı eklentilerini kullanabilmeleri için yeniden başlatılmaları gerekir:"

${LangFileString} IDS_DETAILS_INITIALIZING "Başlatılıyor"
${LangFileString} IDS_DETAILS_SKIPPING "Atlanılıyor"
${LangFileString} IDS_DETAILS_CREATE_SHORTCUTS "Kısayollar oluşturuluyor"
${LangFileString} IDS_DETAILS_WRITE_UNINSTALL "Kaldırma verisi yazılıyor"
${LangFileString} IDS_DETAILS_UNINSTALLER_NEWER_OR_SAME "Hedef kaldırıcı daha yeni veya aynı"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_BEGIN "$0 için eklenti yükleniyor"
${LangFileString} IDS_DETAILS_BROWSER_NOT_FOUND "Tarayıcı bulunamadı"
${LangFileString} IDS_DETAILS_BROWSER_VERSION "Tarayıcı sürümü"
${LangFileString} IDS_DETAILS_SOURCE_PLUGIN_VERSION "Kaynak eklenti sürümü"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_VERSION "Hedef eklenti sürümü"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NOT_FOUND "Hedef eklenti bulunamadı"
${LangFileString} IDS_DETAILS_TARGET_PLUGIN_NEWER_OR_SAME "Hedef eklentinin sürümü daha yeni ya da aynı."
${LangFileString} IDS_DETAILS_INSTALLING_PLUGIN "Eklenti yükleniyor"
${LangFileString} IDS_DETAILS_PLUGIN_INSTALL_ERROR "$0 için eklenti yüklenirken hata oluştu"
${LangFileString} IDS_DETAILS_PLUGINS_STILL_PRESENT "Bazı eklentiler kaldırılmadı"
${LangFileString} IDS_DETAILS_SKIP_CLEAR_UNINSTALLER "Seçili eklentiler kaldırıldı. Kaldırıcı, sonraki kullanımlar için bırakıldı."
${LangFileString} IDS_DETAILS_REMOVE_PLUGIN "$0 için eklenti kaldırılıyor"
${LangFileString} IDS_DETAILS_REMOVE_UNINSTALLER "Kaldırıcı kaldırılıyor"


${LangFileString} IDS_ERROR_UNABLE_TO_WRITE "Yazılamıyor"
${LangFileString} IDS_ERROR_UNABLE_TO_REGISTER "Kayıt defteri girdilerine erişilemiyor"

${LangFileString} IDS_DETAILS "Ayrıntılar"
${LangFileString} IDS_UNKNOWN "Bilinmeyen"

														