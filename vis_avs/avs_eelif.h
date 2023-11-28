#ifndef _AVS_EEL_IF_H_
#define _AVS_EEL_IF_H_

#include "../ns-eel/ns-eel.h"
#ifdef AVS_MEGABUF_SUPPORT
#include "../ns-eel/megabuf.h"
#endif

void AVS_EEL_IF_init();
void AVS_EEL_IF_quit();

int AVS_EEL_IF_Compile(int context, char *code);
void AVS_EEL_IF_Execute(void *handle, char visdata[2][2][576]);
void AVS_EEL_IF_resetvars(NSEEL_VMCTX ctx);
void AVS_EEL_IF_VM_free(NSEEL_VMCTX ctx);
extern char last_error_string[1024];
extern int g_log_errors;
extern CRITICAL_SECTION g_eval_cs;

// our old-style interface
#define compileCode(exp) AVS_EEL_IF_Compile(AVS_EEL_CONTEXTNAME,(exp))
#define executeCode(x,y) AVS_EEL_IF_Execute((void*)(x),(y))
#define freeCode(h) NSEEL_code_free((NSEEL_CODEHANDLE)(h))
#define resetVars(x) FIXME+++++++++
#define registerVar(x) NSEEL_VM_regvar((NSEEL_VMCTX)AVS_EEL_CONTEXTNAME,(x))
#define clearVars() AVS_EEL_IF_resetvars((NSEEL_VMCTX)AVS_EEL_CONTEXTNAME)

#define AVS_EEL_INITINST() AVS_EEL_CONTEXTNAME=(int)NSEEL_VM_alloc()

#define AVS_EEL_QUITINST() AVS_EEL_IF_VM_free((NSEEL_VMCTX)AVS_EEL_CONTEXTNAME)


#endif//_AVS_EEL_IF_H_