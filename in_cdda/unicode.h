#ifndef NULLSOFT_IN_CDDA_UNICODE_H
#define NULLSOFT_IN_CDDA_UNICODE_H

#include <windows.h>

void ListBox_AddStringUnicode(HWND hwnd, wchar_t *text);
void ListBox_GetTextUnicode(HWND hwnd, int num, wchar_t *text);
/*
bool SetupUnicode();

void GetDlgItemTextUnicode(HWND hwndDlg, int id, wchar_t *dest, int length);

void SetDlgItemText(HWND hwndDlg, int id, wchar_t *text);

void SetDlgItemTextUnicode(HWND hwndDlg, int id, wchar_t *text);
*/
#endif