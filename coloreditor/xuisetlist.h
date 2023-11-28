#ifndef __SETLIST_H
#define __SETLIST_H

#include <api/wnd/wndclass/listwnd.h>

#define SETLIST_PARENT ListWnd

// {A0B30669-B18E-426c-8EE2-E851D4CE439C}
static const GUID COLOREDIT_SETLIST_GUID = 
{ 0xa0b30669, 0xb18e, 0x426c, { 0x8e, 0xe2, 0xe8, 0x51, 0xd4, 0xce, 0x43, 0x9c } };

class ColorEditorInstance;

// -----------------------------------------------------------------------
class ColorEditorSetList : public SETLIST_PARENT {
  
  public:

    ColorEditorSetList();
    virtual ~ColorEditorSetList();

    virtual int onInit();
    virtual int getColumsHeight() { return 0; }
    virtual int onResize();
    void selectCurrentSet();
    void setInstanceCallback(ColorEditorInstance *i) { editor = i; }
    virtual void onItemSelection(int itemnum, int selected);
    void selectSet(const wchar_t *curset);
    void loadThemes();
    void onDoubleClick(int itemnum);
    int onRightClick(int itemnum);
    const wchar_t *getCurrentSet();
    virtual int getTextBold(LPARAM lParam);
    virtual int wantHScroll() { return 0; }
    void coloreditor_switch();

  private:
    
    ColorEditorInstance *editor;
};

// -----------------------------------------------------------------------
extern wchar_t ColorEditorSetListXuiObjectStr[];
extern char ColorEditorSetListXuiSvcName[];
class ColorEditorSetListXuiSvc : public XuiObjectSvc<ColorEditorSetList, ColorEditorSetListXuiObjectStr, ColorEditorSetListXuiSvcName> {};

#endif
