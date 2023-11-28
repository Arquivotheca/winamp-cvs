#ifndef __GROUPLIST_H
#define __GROUPLIST_H

#include <api/wnd/wndclass/listwnd.h>

#define GROUPLIST_PARENT ListWnd

// {85C6A47A-FFFF-4dae-89C0-0C49E12B658C}
static const GUID COLOREDIT_GROUPLIST_GUID = 
{ 0x85c6a47a, 0xffff, 0x4dae, { 0x89, 0xc0, 0xc, 0x49, 0xe1, 0x2b, 0x65, 0x8c } };

class ColorEditorInstance;

// -----------------------------------------------------------------------
class ColorEditorGroupList : public GROUPLIST_PARENT {
  
  public:

    ColorEditorGroupList();
    virtual ~ColorEditorGroupList();

    virtual int onInit();
    virtual void onLeftClick(int itemnum);	// left-click

    virtual int getColumsHeight() { return 0; }
    virtual int onResize();

    void setInstanceCallback(ColorEditorInstance *i) { editor = i; }
    void setSet(const wchar_t *set, int force=0);
    virtual void onItemSelection(int itemnum, int selected);

    int onRightClick(int itemnum);
    virtual int wantHScroll() { return 0; }

  private:
    
    void coloreditor_switch();
    void loadGroups();

    StringW set_name;
    ColorEditorInstance *editor;
};

// -----------------------------------------------------------------------
extern wchar_t ColorEditorGroupListXuiObjectStr[];
extern char ColorEditorGroupListXuiSvcName[];
class ColorEditorGroupListXuiSvc : public XuiObjectSvc<ColorEditorGroupList, ColorEditorGroupListXuiObjectStr, ColorEditorGroupListXuiSvcName> {};

#endif
