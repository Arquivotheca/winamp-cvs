#ifndef _APE_H_
#define _APE_H_


//extended APE stuff

// to use this, you should have:
// APEinfo *g_extinfo;
// void __declspec(dllexport) AVS_APE_SetExtInfo(HINSTANCE hDllInstance, APEinfo *ptr)
// {
//    g_extinfo = ptr;
// }

typedef void *VM_CONTEXT;
typedef void *VM_CODEHANDLE;
typedef struct
{
  int ver; // ver=1 to start
  double *global_registers; // 100 of these

  // lineblendmode: 0xbbccdd
  // bb is line width (minimum 1)
  // dd is blend mode:
     // 0=replace
     // 1=add
     // 2=max
     // 3=avg
     // 4=subtractive (1-2)
     // 5=subtractive (2-1)
     // 6=multiplicative
     // 7=adjustable (cc=blend ratio)
     // 8=xor
     // 9=minimum
  int *lineblendmode; 

  //evallib interface
  VM_CONTEXT (*allocVM)(); // return a handle
  void (*freeVM)(VM_CONTEXT); // free when done with a VM and ALL of its code have been freed, as well

  // you should only use these when no code handles are around (i.e. it's okay to use these before
  // compiling code, or right before you are going to recompile your code. 
  void (*resetVM)(VM_CONTEXT);
  double * (*regVMvariable)(VM_CONTEXT, char *name);

  // compile code to a handle
  VM_CODEHANDLE (*compileVMcode)(VM_CONTEXT, char *code);

  // execute code from a handle
  void (*executeCode)(VM_CODEHANDLE, char visdata[2][2][576]);

  // free a code block
  void (*freeCode)(VM_CODEHANDLE);

  // requires ver >= 2
  void (*doscripthelp)(HWND hwndDlg,char *mytext); // mytext can be NULL for no custom page

  /// requires ver >= 3
  void *(*getNbuffer)(int w, int h, int n, int do_alloc); // do_alloc should be 0 if you dont want it to allocate if empty
                                                          // w and h should be the current width and height
                                                          // n should be 0-7

} APEinfo;



#endif//_APE_H_