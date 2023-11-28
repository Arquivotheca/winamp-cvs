/* This file was generated by Maki Compiler, do not edit manually */

#ifndef __C_EDIT_H
#define __C_EDIT_H

#include "c_guiobject.h"

#define C_EDIT_PARENT C_GuiObject

class C_Edit : public C_EDIT_PARENT {
  public:

  C_Edit(ScriptObject *object);
  C_Edit();
  virtual ~C_Edit();

  virtual void C_hook(ScriptObject *o);

  ScriptObject *getScriptObject();

  virtual void onEnter();
  virtual void onAbort();
  virtual void onIdleEditUpdate();
  virtual void onEditUpdate();
  virtual void setText(const wchar_t *txt);
  virtual void setAutoEnter(int onoff);
  virtual int getAutoEnter();
  virtual const wchar_t *getText();
  virtual void selectAll();
  virtual void enter();
  virtual void setIdleEnabled(int onoff);
  virtual int getIdleEnabled();

  private:

  ScriptObject *obj;
  int inited;
  static int loaded;
  static int onenter_id;
  static int onabort_id;
  static int onidleeditupdate_id;
  static int oneditupdate_id;
  static int settext_id;
  static int setautoenter_id;
  static int getautoenter_id;
  static int gettext_id;
  static int selectall_id;
  static int enter_id;
  static int setidleenabled_id;
  static int getidleenabled_id;
};

#endif
