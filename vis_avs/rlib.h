#ifndef _RLIB_H_
#define _RLIB_H_

#define DLLRENDERBASE 16384

class C_RLibrary {
	protected:
		typedef struct
		{
			C_RBASE *(*rf)(char *desc/*=NULL*/);
			int is_r2;
		} rfStruct;
		rfStruct *RetrFuncs;

		int NumRetrFuncs;

		typedef struct 
		{
			HINSTANCE hDllInstance;
			char *idstring;
			C_RBASE *(*createfunc)(char *desc);
			int is_r2;
		} DLLInfo; 

		DLLInfo *DLLFuncs;
		int	NumDLLFuncs;

		void add_dofx(void *rf, int has_r2);
		void initfx(void);
		void initdll(void);
		void initbuiltinape(void);
		void _add_dll(HINSTANCE,class C_RBASE *(__cdecl *)(char *),char *, int);
	public:
		C_RLibrary();
		~C_RLibrary();
		C_RBASE *CreateRenderer(int *which, int *has_r2);
		HINSTANCE GetRendererInstance(int which,HINSTANCE hThisInstance);
		int GetRendererDesc(int which, char *str); 
		// if which is >= DLLRENDERBASE
		// returns "id" of DLL. which is used to enumerate. str is desc
		// otherwise, returns 1 on success, 0 on error
};

#endif // _RLIB_H_