/* This file was generated by Maki Compiler, do not edit manually */

#ifndef __C_TREEITEM_H
#define __C_TREEITEM_H

#include "c_rootobj.h"

#define C_TREEITEM_PARENT C_RootObject

class C_TreeItem : public C_TREEITEM_PARENT {
  public:

  C_TreeItem(ScriptObject *object);
  C_TreeItem();
  virtual ~C_TreeItem();

  virtual void C_hook(ScriptObject *o);

  ScriptObject *getScriptObject();

  virtual int getNumChildren();
  virtual void setLabel(const wchar_t *label);
  virtual const wchar_t *getLabel();
  virtual void ensureVisible();
  virtual ScriptObject *getNthChild(int nth);
  virtual ScriptObject *getChild();
  virtual ScriptObject *getChildSibling(ScriptObject *_item);
  virtual ScriptObject *getSibling();
  virtual ScriptObject *getParent();
  virtual void editLabel();
  virtual int hasSubItems();
  virtual void setSorted(int issorted);
  virtual void setChildTab(int haschildtab);
  virtual int isSorted();
  virtual int isCollapsed();
  virtual int isExpanded();
  virtual void invalidate();
  virtual int isSelected();
  virtual int isHilited();
  virtual void setHilited(int ishilited);
  virtual int collapse();
  virtual int expand();
  virtual ScriptObject *getTree();
  virtual void onTreeAdd();
  virtual void onTreeRemove();
  virtual void onSelect();
  virtual void onDeselect();
  virtual int onLeftDoubleClick();
  virtual int onRightDoubleClick();
  virtual int onChar(int key);
  virtual void onExpand();
  virtual void onCollapse();
  virtual int onBeginLabelEdit();
  virtual int onEndLabelEdit(const wchar_t *newlabel);
  virtual int onContextMenu(int x, int y);

  private:

  ScriptObject *obj;
  int inited;
  static int loaded;
  static int getnumchildren_id;
  static int setlabel_id;
  static int getlabel_id;
  static int ensurevisible_id;
  static int getnthchild_id;
  static int getchild_id;
  static int getchildsibling_id;
  static int getsibling_id;
  static int getparent_id;
  static int editlabel_id;
  static int hassubitems_id;
  static int setsorted_id;
  static int setchildtab_id;
  static int issorted_id;
  static int iscollapsed_id;
  static int isexpanded_id;
  static int invalidate_id;
  static int isselected_id;
  static int ishilited_id;
  static int sethilited_id;
  static int collapse_id;
  static int expand_id;
  static int gettree_id;
  static int ontreeadd_id;
  static int ontreeremove_id;
  static int onselect_id;
  static int ondeselect_id;
  static int onleftdoubleclick_id;
  static int onrightdoubleclick_id;
  static int onchar_id;
  static int onexpand_id;
  static int oncollapse_id;
  static int onbeginlabeledit_id;
  static int onendlabeledit_id;
  static int oncontextmenu_id;
};

#endif
