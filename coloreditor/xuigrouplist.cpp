#include "precomp.h"
#include "xuigrouplist.h"
#include "xuiinstance.h"
#include <api/api.h>
#include <api/wnd/popup.h>
#include <api/service/svc_enum.h>

// -----------------------------------------------------------------------
wchar_t ColorEditorGroupListXuiObjectStr[] = L"ColorEditor:GroupList"; // This is the xml tag
char ColorEditorGroupListXuiSvcName[] = "ColorEditor:GroupList XuiObject Service";


// -----------------------------------------------------------------------
ColorEditorGroupList::ColorEditorGroupList() {
  setFontSize(15);
  getScriptObject()->vcpu_setInterface(COLOREDIT_GROUPLIST_GUID, this);
  editor = NULL;
  setVirtual(0);
  setPreventMultipleSelection(1);
  setAutoSort(1);
}

// -----------------------------------------------------------------------
ColorEditorGroupList::~ColorEditorGroupList() {
}

// -----------------------------------------------------------------------
int ColorEditorGroupList::onInit() {
  GROUPLIST_PARENT::onInit();
  addColumn(L"Group",250);
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorGroupList::onLeftClick(int itemnum) {
}

// -----------------------------------------------------------------------
void ColorEditorGroupList::loadGroups() 
{
  for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorGroups(set_name);i++) 
	{
    const wchar_t *group = WASABI_API_SKIN->colortheme_enumColorGroupName(set_name, i);
    addItem(group, 0);
  }
}

// -----------------------------------------------------------------------
int ColorEditorGroupList::onResize() {
  GROUPLIST_PARENT::onResize();
  RECT r;
  getClientRect(&r);
  ListColumn *col = getColumn(0);
  if (col) col->setWidth(r.right-r.left-4);
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorGroupList::setSet(const wchar_t *set, int force) {
  if (!force && WCSCASEEQLSAFE(set, set_name)) return;
  set_name = set;
  deleteAllItems();
  loadGroups();
  selectFirstEntry();
  ensureItemVisible(0);
}

// -----------------------------------------------------------------------
void ColorEditorGroupList::onItemSelection(int itemnum, int selected) {
  GROUPLIST_PARENT::onItemSelection(itemnum, selected); 
  if (selected) {
    if (editor != NULL) {
      wchar_t str[256];
      getItemLabel(itemnum, 0, str, 255);
      str[255]=0;
      editor->groupCallback(str);
    }
  }
}

// -----------------------------------------------------------------------
int ColorEditorGroupList::onRightClick(int itemnum) {
  SETLIST_PARENT::onRightClick(itemnum);
  PopupMenu *pm = new PopupMenu(this);
  pm->addCommand(L"Copy", 1);
  pm->addCommand(L"Paste", 2);
  pm->addSeparator();
  pm->addCommand(L"Revert", 3);
  int r = pm->popAtMouse();
  if (r == 1) {
    if (editor) sendAction(editor, L"COPY");
  } else if (r == 2) {
    if (editor) sendAction(editor, L"PASTE");
  } else if (r == 3) {
    if (editor) sendAction(editor, L"REVERT");
  }
  return 1;
}



