#include "precomp.h"
#include "xuisetlist.h"
#include "xuiinstance.h"
#include <api/api.h>
#include <api/wnd/popup.h>
#include <api/service/svc_enum.h>

// -----------------------------------------------------------------------
wchar_t ColorEditorSetListXuiObjectStr[] = L"ColorEditor:SetList"; // This is the xml tag
char ColorEditorSetListXuiSvcName[] = "ColorEditor:SetList XuiObject Service";


// -----------------------------------------------------------------------
ColorEditorSetList::ColorEditorSetList() {
  	setFontSize(15);
  editor = NULL;
  setVirtual(0);
  setPreventMultipleSelection(1);
  setAutoSort(1);
  getScriptObject()->vcpu_setInterface(COLOREDIT_SETLIST_GUID, this);
}

// -----------------------------------------------------------------------
ColorEditorSetList::~ColorEditorSetList() {
}

// -----------------------------------------------------------------------
int ColorEditorSetList::onInit() {
  SETLIST_PARENT::onInit();
  addColumn(L"Theme",250);
  SkinBitmap *tmp = new SkinBitmap(L"coloredit.lock");
  setIconHeight(tmp->getHeight());
  setIconWidth(tmp->getWidth());
  loadThemes();
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorSetList::loadThemes() {
  setShowIcons(1);
  deleteAllItems();
  for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++) {
    StringW set = WASABI_API_SKIN->colortheme_enumColorSet(i);
    StringW x = set;
    x.trunc(11);
    if (!x.iscaseequal(L"{coloredit}")) {
      int pos = addItem(set, i);
      setItemIcon(pos, L"coloredit.lock");
    } else {
      addItem(set.getValue()+11, i);
    }
  }
}

// -----------------------------------------------------------------------
int ColorEditorSetList::onResize() {
  SETLIST_PARENT::onResize();
  RECT r;
  getClientRect(&r);
  ListColumn *col = getColumn(0);
  if (col) col->setWidth(r.right-r.left-4);
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorSetList::selectCurrentSet() {
  const wchar_t *curset = WASABI_API_SKIN->colortheme_getColorSet();
  if (!curset) return;
  int unlocked = 0;
  if (!wcsncmp(curset, L"{coloredit}", 11)) { curset += 11; unlocked = 1; }
  if (curset != NULL) {
    wchar_t set[256]=L"";
    for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++) {
      getItemLabel(i, 0, set, 255); set[255]=0;
      if (set && !_wcsicmp(curset, set) && (unlocked ? getItemIcon(i) == NULL : getItemIcon(i) != NULL)) {
        setSelected(i, 1);
        ensureItemVisible(i);
        return;
      }
    }
  }
}

// -----------------------------------------------------------------------
void ColorEditorSetList::selectSet(const wchar_t *daset) {
  StringW curset = daset;
  int unlocked = 0;
  if (!wcsncmp(daset, L"{coloredit}", 11)) { curset = curset.getValue()+11; unlocked = 1; }
  if (curset != NULL) {
    wchar_t set[256]=L"";
    for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++) {
      getItemLabel(i, 0, set, 255); set[255]=0;
      if (set && !_wcsicmp(curset, set) && (unlocked ? getItemIcon(i) == NULL : getItemIcon(i) != NULL)) {
        setSelected(i, 1);
        ensureItemVisible(i);
        return;
      }
    }
  }
}

// -----------------------------------------------------------------------
void ColorEditorSetList::onItemSelection(int itemnum, int selected) {
  SETLIST_PARENT::onItemSelection(itemnum, selected);
  if (selected) {
    if (editor != NULL) {
      wchar_t str[256];
      getItemLabel(itemnum, 0, str, 255);
      str[255]=0;
      StringW setname = str;
      if (!getItemIcon(itemnum)) 
				setname.prepend(L"{coloredit}");
      editor->setCallback(setname);
    }
  }
}

// -----------------------------------------------------------------------
void ColorEditorSetList::onDoubleClick(int itemnum) {
  SETLIST_PARENT::onDoubleClick(itemnum);
  coloreditor_switch();
}

// -----------------------------------------------------------------------
int ColorEditorSetList::onRightClick(int itemnum) {
  SETLIST_PARENT::onRightClick(itemnum);
  PopupMenu *pm = new PopupMenu(this);
  pm->addCommand(L"Switch to", 1);
  pm->addSeparator();
  pm->addCommand(L"Rename", 2);
  pm->addCommand(L"Clone", 3);
  pm->addCommand(L"Delete", 4);
  int locked = getItemIcon(itemnum) != NULL;
  if (locked) pm->disableCommand(2, 1);
  if (locked) pm->disableCommand(4, 1);
  int r = pm->popAtMouse();
  if (r == 1) {
    coloreditor_switch();
  } else if (r == 2) {
    if (editor) sendAction(editor, L"RENAME");
  } else if (r == 3) {
    if (editor) sendAction(editor, L"CLONE");
  } else if (r == 4) {
    if (editor) sendAction(editor, L"DELETE");
  }
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorSetList::coloreditor_switch() {
  int sel = getFirstItemSelected();
  if (sel >= 0) {
    wchar_t set[256]=L"";
    getItemLabel(sel, 0, set, 255); set[255] = 0;
    StringW setname = set;
    if (!getItemIcon(sel)) 
			setname.prepend(L"{coloredit}");
    WASABI_API_SKIN->colortheme_setColorSet(setname);
  }
}

// -----------------------------------------------------------------------
int ColorEditorSetList::getTextBold(LPARAM lParam) 
{
  if (WCSCASEEQLSAFE(WASABI_API_SKIN->colortheme_enumColorSet(lParam), WASABI_API_SKIN->colortheme_getColorSet())) 
		return 1;
  return SETLIST_PARENT::getTextBold(lParam);
}


