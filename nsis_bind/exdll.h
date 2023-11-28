#ifndef _EXDLL_H_
#define _EXDLL_H_

#include <tchar.h>
typedef struct _stack_t {
  struct _stack_t *next;
  TCHAR text[1]; // this should be the length of string_size
} stack_t;

/*
typedef struct {
  exec_flags_type *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
} extra_parameters;
*/



#endif//_EXDLL_H_
