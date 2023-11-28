#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

// this will be the directory and APE name displayed in the AVS Editor
#define MOD_NAME "Trans / Channel Shift"

#define C_THISCLASS C_ChannelShiftClass

typedef struct {
	int	mode;
	int onbeat;
} apeconfig;

class C_THISCLASS : public C_RBASE 
{
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat,	int *framebuffer, int *fbout, int w, int h);		
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual char *get_desc();
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		apeconfig config;
		HWND hwndDlg;
};

// global configuration dialog pointer 
static C_THISCLASS *g_ConfigThis; 
static HINSTANCE g_hDllInstance; 

// this is where we deal with the configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ids[] = { IDC_RBG, IDC_BRG, IDC_BGR, IDC_GBR, IDC_GRB, IDC_RGB };
	switch (uMsg)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED) {
				for (int i=0;i<sizeof(ids)/sizeof(ids[0]);i++)
					if (IsDlgButtonChecked(hwndDlg, ids[i]))
						g_ConfigThis->config.mode = ids[i];
				g_ConfigThis->config.onbeat = IsDlgButtonChecked(hwndDlg, IDC_ONBEAT) ? 1 : 0;
			}
			return 1;

		case WM_INITDIALOG:
			g_ConfigThis->hwndDlg = hwndDlg;

			CheckDlgButton(hwndDlg, g_ConfigThis->config.mode, 1);
			if (g_ConfigThis->config.onbeat)
				CheckDlgButton(hwndDlg, IDC_ONBEAT, 1);
			
			return 1;

		case WM_DESTROY:
			KillTimer(hwndDlg, 1);
			return 1;
	}
	return 0;
}

// set up default configuration 
C_THISCLASS::C_THISCLASS() 
{
	memset(&config, 0, sizeof(apeconfig));
	config.mode = IDC_RBG;
	config.onbeat = 1;
}

// virtual destructor
C_THISCLASS::~C_THISCLASS() 
{
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	if (isBeat&0x80000000) return 0;

	int c;
	int modes[] = { IDC_RGB, IDC_RBG, IDC_GBR, IDC_GRB, IDC_BRG, IDC_BGR };

	if (isBeat && config.onbeat) {
		config.mode = modes[rand() % 6];
	}

	c = w*h;

	switch (config.mode) {
		default:
		case IDC_RGB:
			return 0;
		case IDC_RBG:
			__asm {
				mov ebx, framebuffer;
				mov ecx, c;
				lp1:
				sub ecx, 4;

				mov eax, dword ptr [ebx+ecx*4];
				xchg ah, al;
				mov [ebx+ecx*4], eax;

				mov eax, dword ptr [ebx+ecx*4+4];
				xchg ah, al;
				mov [ebx+ecx*4+4], eax;

				mov eax, dword ptr [ebx+ecx*4+8];
				xchg ah, al;
				mov [ebx+ecx*4+8], eax;

				mov eax, dword ptr [ebx+ecx*4+12];
				xchg ah, al;
				mov [ebx+ecx*4+12], eax;

				test ecx, ecx;
				jnz lp1;
			}
			break;
			
		case IDC_BRG:
			__asm {
				mov ebx, framebuffer;
				mov ecx, c;
				lp2:
				sub ecx, 4;
				
				mov eax, dword ptr [ebx+ecx*4];
				mov dl, al;
				shr eax, 8;
				bswap eax;
				mov ah, dl;
				bswap eax;
				mov [ebx+ecx*4], eax;
				
				mov eax, dword ptr [ebx+ecx*4+4];
				mov dl, al;
				shr eax, 8;
				bswap eax;
				mov ah, dl;
				bswap eax;
				mov [ebx+ecx*4+4], eax;

				mov eax, dword ptr [ebx+ecx*4+8];
				mov dl, al;
				shr eax, 8;
				bswap eax;
				mov ah, dl;
				bswap eax;
				mov [ebx+ecx*4+8], eax;

				mov eax, dword ptr [ebx+ecx*4+12];
				mov dl, al;
				shr eax, 8;
				bswap eax;
				mov ah, dl;
				bswap eax;
				mov [ebx+ecx*4+12], eax;

				test ecx, ecx;
				jnz lp2;
			}
			break;

		case IDC_BGR:
			__asm {
				mov ebx, framebuffer;
				mov ecx, c;
				lp3:
				sub ecx, 4;

				mov eax, dword ptr [ebx+ecx*4];
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4], eax;

				mov eax, dword ptr [ebx+ecx*4+4];
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+4], eax;

				mov eax, dword ptr [ebx+ecx*4+8];
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+8], eax;

				mov eax, dword ptr [ebx+ecx*4+12];
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+12], eax;

				test ecx, ecx;
				jnz lp3;
			}
			break;

		case IDC_GBR:
			__asm {
				mov ebx, framebuffer;
				mov ecx, c;
				lp4:
				sub ecx, 4;

				mov eax, dword ptr [ebx+ecx*4];
				mov edx, eax;
				bswap edx;
				shl eax, 8;
				mov al, dh;
				mov [ebx+ecx*4], eax;

				mov eax, dword ptr [ebx+ecx*4+4];
				mov edx, eax;
				bswap edx;
				shl eax, 8;
				mov al, dh;
				mov [ebx+ecx*4+4], eax;

				mov eax, dword ptr [ebx+ecx*4+8];
				mov edx, eax;
				bswap edx;
				shl eax, 8;
				mov al, dh;
				mov [ebx+ecx*4+8], eax;

				mov eax, dword ptr [ebx+ecx*4+12];
				mov edx, eax;
				bswap edx;
				shl eax, 8;
				mov al, dh;
				mov [ebx+ecx*4+12], eax;

				test ecx, ecx;
				jnz lp4;
			}
			break;

		case IDC_GRB:
			__asm {
				mov ebx, framebuffer;
				mov ecx, c;
				lp5:
				sub ecx, 4;

				mov eax, dword ptr [ebx+ecx*4];
				shl eax, 8;
				bswap eax;
				xchg ah, al;
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4], eax;

				mov eax, dword ptr [ebx+ecx*4+4];
				shl eax, 8;
				bswap eax;
				xchg ah, al;
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+4], eax;

				mov eax, dword ptr [ebx+ecx*4+8];
				shl eax, 8;
				bswap eax;
				xchg ah, al;
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+8], eax;

				mov eax, dword ptr [ebx+ecx*4+12];
				shl eax, 8;
				bswap eax;
				xchg ah, al;
				bswap eax;
				shr eax, 8;
				mov [ebx+ecx*4+12], eax;

				test ecx, ecx;
				jnz lp5;
			}
			break;
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) 
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_CHANSHIFT, hwndParent, (DLGPROC)g_DlgProc);
}

char *C_THISCLASS::get_desc(void)
{ 
	static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_CHANNEL_SHIFT,desc,128):desc);
}

void C_THISCLASS::load_config(unsigned char *data, int len) 
{
	srand(time(0));
	if (len <= sizeof(apeconfig))
		memcpy(&this->config, data, len);
}

int  C_THISCLASS::save_config(unsigned char *data) 
{
	memcpy(data, &this->config, sizeof(apeconfig));
	return sizeof(apeconfig);
}

C_RBASE *R_ChannelShift(char *desc) 
{
	if (desc) { 
		strcpy(desc,MOD_NAME); 
		return NULL; 
	}
	return (C_RBASE *) new C_THISCLASS();
}

#endif