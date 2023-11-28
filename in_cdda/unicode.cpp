#include <windows.h>

/* unicode helper functions */

void ListBox_GetTextUnicode(HWND hwnd, int num, wchar_t *text)
{
		SendMessageW(hwnd, LB_GETTEXT, num, (LPARAM)text);
}

void ListBox_AddStringUnicode(HWND hwnd, wchar_t *text)
{
	SendMessageW(hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}