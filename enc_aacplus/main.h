#ifndef NULLSOFT_ENC_AACPLUS_MAINH
#define NULLSOFT_ENC_AACPLUS_MAINH

#include <windows.h>
#include "../aacPlus/aacplusenc.h"

BOOL CALLBACK AACConfigurationDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

struct AACplusConfig
{
  int sampleRate;
  aacPlusEncChannelMode channelMode;
  int bitRate;
	char section_name[64];
	aacPlusEncSbrMode aacMode;
	bitstreamFormat format;
	sbrSignallingMode signallingMode;
	bool speech;
	bool extraOptions;
	bool pns;
};

class ConfigWnd
{
public:
  AACplusConfig cfg;
  char *configfile;
	aacPlusEncHandle configEncoder;
};

void readconfig(char *configfile, char *section, AACplusConfig *cfg);
extern HWND winampwnd;

#endif