/* This file was generated by Maki Compiler, do not edit manually */

#include <precomp.h>
#include <api/api.h>
#include "c_menubutton.h"
#include <api/script/objcontroller.h>

C_MenuButton::C_MenuButton(ScriptObject *object) : C_GuiObject(object) {
  inited = 0;
  C_hook(object);
}

C_MenuButton::C_MenuButton() {
  inited = 0;
}

void C_MenuButton::C_hook(ScriptObject *object) {
  ASSERT(!inited);
  ScriptObjectController *controller = object->vcpu_getController();
  obj = controller->cast(object, menuButtonGuid);
  if (obj != object && obj != NULL)
    controller = obj->vcpu_getController();
  else
    obj = NULL;

  int iter = WASABI_API_APP->app_getInitCount();
  if (!loaded || loaded != iter) {
    loaded = iter;
    onopenmenu_id = WASABI_API_MAKI->maki_addDlfRef(controller, L"onOpenMenu", this);
    onclosemenu_id = WASABI_API_MAKI->maki_addDlfRef(controller, L"onCloseMenu", this);
    onselectitem_id = WASABI_API_MAKI->maki_addDlfRef(controller, L"onSelectItem", this);
    openmenu_id = WASABI_API_MAKI->maki_addDlfRef(controller, L"openMenu", this);
    closemenu_id = WASABI_API_MAKI->maki_addDlfRef(controller, L"closeMenu", this);
  }
  inited = 1;
}

C_MenuButton::~C_MenuButton() {
}

ScriptObject *C_MenuButton::getScriptObject() {
  if (obj != NULL) return obj;
  return C_MENUBUTTON_PARENT::getScriptObject();
}

void C_MenuButton::onOpenMenu() {
  ASSERT(inited);
  WASABI_API_MAKI->maki_callFunction(getScriptObject(), onopenmenu_id, NULL);
}

void C_MenuButton::onCloseMenu() {
  ASSERT(inited);
  WASABI_API_MAKI->maki_callFunction(getScriptObject(), onclosemenu_id, NULL);
}

void C_MenuButton::onSelectItem(const wchar_t *item) {
  ASSERT(inited);
  scriptVar a = MAKE_SCRIPT_STRING(item);
  scriptVar *params[1] = {&a};
  WASABI_API_MAKI->maki_callFunction(getScriptObject(), onselectitem_id, params);
}

void C_MenuButton::openMenu() {
  ASSERT(inited);
  WASABI_API_MAKI->maki_callFunction(getScriptObject(), openmenu_id, NULL);
}

void C_MenuButton::closeMenu() {
  ASSERT(inited);
  WASABI_API_MAKI->maki_callFunction(getScriptObject(), closemenu_id, NULL);
}

int C_MenuButton::loaded=0;
int C_MenuButton::onopenmenu_id=0;
int C_MenuButton::onclosemenu_id=0;
int C_MenuButton::onselectitem_id=0;
int C_MenuButton::openmenu_id=0;
int C_MenuButton::closemenu_id=0;
