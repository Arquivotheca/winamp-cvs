#ifndef NULLSOFT_ENC_AACPLUS_CONFIGH
#define NULLSOFT_ENC_AACPLUS_CONFIGH
#include <windows.h>
#include "../aacPlus/aacPlusEnc.h"
#include "main.h"

int GetBitRate(HWND hwndDlg);
aacPlusEncChannelMode GetChannelMode(HWND hwndDlg);
bool SelectBitRate(HWND hwndDlg, int item, int bitRate);
bool HasBitRate(HWND hwndDlg, int item, int bitRate);
bool SelectChannelMode(HWND hwndDlg, int item, int mode);
void writeconfig(char *configfile, AACplusConfig *cfg);
void FillBitrates(HWND hwndDlg, int item, aacPlusEncFormatList *formatList);
void BuildChannelComboBox(HWND hwndDlg, int item);
void BuildBitRates(HWND hwndDlg, int item);
#endif