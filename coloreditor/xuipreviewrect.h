#ifndef __PREVIEWRECT_H
#define __PREVIEWRECT_H

#include <api/wnd/api_window.h>
#include <api/wnd/wndclass/guiobjwnd.h>

#define PREVIEWRECT_PARENT GuiObjectWnd

// {12D9C377-A981-4b77-95E0-242AF7226960}
static const GUID COLOREDIT_PREVIEWRECT_GUID = 
{ 0x12d9c377, 0xa981, 0x4b77, { 0x95, 0xe0, 0x24, 0x2a, 0xf7, 0x22, 0x69, 0x60 } };

class ColorEditorInstance;  

// -----------------------------------------------------------------------
class ColorEditorPreviewRect : public PREVIEWRECT_PARENT {
  
  public:

    ColorEditorPreviewRect();
    virtual ~ColorEditorPreviewRect();

    virtual int onInit();
    virtual int onPaint(Canvas *c);

    virtual void setInstanceCallback(ColorEditorInstance *i) { editor = i; }

  private:

    ColorEditorInstance *editor;
    
};

// -----------------------------------------------------------------------
extern wchar_t ColorEditorPreviewRectXuiObjectStr[];
extern char ColorEditorPreviewRectXuiSvcName[];
class ColorEditorPreviewRectXuiSvc : public XuiObjectSvc<ColorEditorPreviewRect, ColorEditorPreviewRectXuiObjectStr, ColorEditorPreviewRectXuiSvcName> {};

#endif
