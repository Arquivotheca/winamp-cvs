/* This file was generated by Maki Compiler, do not edit manually */

#ifndef __HOOK_TEXT_H
#define __HOOK_TEXT_H

#include "h_guiobject.h"

#define H_TEXT_PARENT H_GuiObject

class H_Text : public H_TEXT_PARENT {

public:

  H_Text(ScriptObject *o);
  H_Text();
  virtual ~H_Text();
  virtual void H_hook(ScriptObject *o);
  ScriptObject *getHookedObject();

  virtual int eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams);
  virtual void hook_onTextChanged(const wchar_t *newtxt) {  }

  private:

  ScriptObject *obj;
  int inited;
  static int loaded;
  static int ontextchanged_id;
};

#endif
