#ifndef NULLSOFT_NONSILENTDRMWINDOWH
#define NULLSOFT_NONSILENTDRMWINDOWH
HWND LaunchNonSilentDRMWindow(const wchar_t *url, void *postData = NULL, size_t postSize = 0, WMHandler *callback=0);
void KillWindow();


#endif 