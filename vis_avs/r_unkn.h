#ifndef _R_UNKN_H_
#define _R_UNKN_H_

#define UNKN_ID 0xffffffff

class C_UnknClass : public C_RBASE {
	protected:
		char *configdata;
		int configdata_len;

	public:
		C_UnknClass();
		virtual ~C_UnknClass();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		virtual char *get_desc();
		virtual void SetID(int d, char *dString);
		int id;
		char idString[33];
		static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
};

#endif // _R_UNKN_H_