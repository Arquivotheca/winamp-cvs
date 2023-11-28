#include "r_defs.h"
#include "r_list.h"
#include "r_transition.h"
#include "rlib.h"

void Render_Init(HINSTANCE hDllInstance);
void Render_Quit(HINSTANCE hDllInstance);

extern C_RenderListClass *g_render_effects;
extern C_RenderListClass *g_render_effects2;
extern C_RenderTransitionClass *g_render_transition;
extern C_RLibrary *g_render_library;

extern CRITICAL_SECTION g_render_cs;
