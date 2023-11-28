#ifndef __COLOREDITORINSTANCE_H
#define __COLOREDITORINSTANCE_H

#include <api/skin/nakedobject.h>
#include <api/skin/colorthemes.h>
#include "xuisetlist.h"
#include "xuigrouplist.h"
#include <api/script/objects/c_script/h_slider.h>
#include <api/script/objects/c_script/h_checkbox.h>
#include <api/script/objects/c_script/h_edit.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/wndmgr/skinwnd.h>

#define COLOREDITORINSTANCE_PARENT NakedObject

class H_ColorEditorSlider;
class H_ColorEditorCheckBox;
class H_ColorEditorEdit;
class H_ColorEditorGuiObject;
class ColorEditorPreviewRect;
class H_InputEdit;

#define DCB_INIT                0xF00D
#define UNDEFINED               0xDEADBEEF

#define CHANNEL_RED   1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE  4

// -----------------------------------------------------------------------
class ColorEditorGroup {
  public:
    ColorEditorGroup(const wchar_t *_set, ColorThemeGroup *grp) : set(_set), group(grp) {
      red = original_red = group->getRed();
      green = original_green = group->getGreen();
      blue = original_blue = group->getBlue();
      gray = original_gray = group->getGray();
      boost = original_boost = group->getBoost();
    }
    virtual ~ColorEditorGroup() {}

    int getRed() { return red; }
    int getGreen() { return green; }
    int getBlue() { return blue; }
    int getGray() { return gray; }
    int getBoost() { return boost; }
    const wchar_t *getName() { return group->getName(); }
    const wchar_t *getColorSet() { return set; }
    void setColorSet(const wchar_t *s) { set = s; }
    
    void setRed(int r) { red = r; }
    void setGreen(int g) { green = g; }
    void setBlue(int b) { blue = b; }
    void setGray(int g) { gray = g; }
    void setBoost(int b) { boost = b; }

    void cancel() { red = original_red;
                    green = original_green;
                    blue = original_blue;
                    gray = original_gray;
                    boost = original_boost; 
                    apply(); }
    void apply() { group->setRed(red);
                  group->setGreen(green);
                  group->setBlue(blue);
                  group->setGray(gray);
                  group->setBoost(boost); }
    void save() { group->setRed(red);
                  group->setGreen(green);
                  group->setBlue(blue);
                  group->setGray(gray);
                  group->setBoost(boost); 
                  original_red = red;
                  original_green = green;
                  original_blue = blue;
                  original_gray = gray;
                  original_boost = boost; }
    void copy(ColorEditorGroup *g) {
                  if (g == NULL) return;
                  red = g->red;
                  green = g->green;
                  blue = g->blue;
                  gray = g->gray;
                  boost = g->boost;
                  original_red = red;
                  original_green = green;
                  original_blue = blue;
                  original_gray = gray;
                  original_boost = boost;
                  group->setRed(red);
                  group->setGreen(green);
                  group->setBlue(blue);
                  group->setGray(gray);
                  group->setBoost(boost);
                }

  private:
    ColorThemeGroup *group;
    StringW set;
    int red, green, blue, gray, boost;
    int original_red, original_green, original_blue;
    int original_gray, original_boost;
};

class ButtonWnd;
// -----------------------------------------------------------------------
class ColorEditorInstance : public COLOREDITORINSTANCE_PARENT {
  
  public:
		  static const wchar_t *getWindowTypeNameW() { return L"Color Editor"; }
		static const char *getWindowTypeName() { return "Color Editor"; }
		static GUID getWindowTypeGuid() {
// {FC77225E-D52A-4df2-B9DC-48A18A3FD594}
    const GUID ret = 
{ 0xfc77225e, 0xd52a, 0x4df2, { 0xb9, 0xdc, 0x48, 0xa1, 0x8a, 0x3f, 0xd5, 0x94 } };
    return ret;
		}
		static void setIconBitmaps(ButtonWnd *button);
		
    ColorEditorInstance();
    virtual ~ColorEditorInstance();

    virtual int onInit();

    virtual int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *value);

    void attachControls();

    void sliderCallback(int newpos, H_ColorEditorSlider *slider, int final);
    void checkboxCallback(int newpos, H_ColorEditorCheckBox *checkbox);
    void editCallback(H_ColorEditorEdit *editor);
    void clickCallback(H_ColorEditorGuiObject *object);
//    void mouseMoveCallback(H_ColorEditorGuiObject *object, int x, int y);
    void groupCallback(const wchar_t *group);
    void setCallback(const wchar_t *set);
    void onInputEnter(H_InputEdit *edit);
    void onInputEscape(H_InputEdit *edit);

    const wchar_t *inputBox(const wchar_t *title, const wchar_t* defaultText = 0);

    static void rgbToHsl(int r, int g, int b, int *h, int *s, int *l);
    static void hslToRgb(int h, int s, int l, int *r, int *g, int *b);

    int getRed();
    int getGreen();
    int getBlue();

    int getHue();
    int getSaturation();
    int getLuminosity();

    int getRedAndHueSlider();
    int getGreenAndSaturationSlider();
    int getBlueAndLuminositySlider();

    int getGray();
    int getBoost();

    int getRGBHSL(); // 0 = rgb, 1 = hsl
    
    void goRGB();
    void goHSL();

    void setPreviewRed(int r);
    void setPreviewGreen(int g);
    void setPreviewBlue(int b);

    COLORREF filterColor(COLORREF color, int r, int g, int b, int gray, int boost);
    COLORREF getInputColor() { return RGB(_red,_green,_blue); }

    ColorEditorGroup *getCurGroup();
    ColorEditorGroup *findGroup(const wchar_t *set, const wchar_t *group);

    int getChannel(ColorEditorPreviewRect *rect);

    void apply();
    void revert();
    void save();
    void newtheme();
    void copy();
    void paste();
    int checkSave();
    void saveAll(const wchar_t *file=NULL);
    void rename();
    void clone();
    void deletetheme();
    void export();
    void about();
    void faq();
    
    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

    virtual int onDeferredCallback(int p1, int p2);

    enum {
      COLOREDITOR_SETREDSLIDER=0,
      COLOREDITOR_SETGREENSLIDER,
      COLOREDITOR_SETBLUESLIDER,
      COLOREDITOR_SETGRAYCHECK,
      COLOREDITOR_SETGRAY2CHECK,
      COLOREDITOR_SETBOOSTCHECK,
      COLOREDITOR_SETREDSOURCE,
      COLOREDITOR_SETGREENSOURCE,
      COLOREDITOR_SETBLUESOURCE,
      COLOREDITOR_SETSOURCERECT,
      COLOREDITOR_SETSETLIST,
      COLOREDITOR_SETGROUPLIST,
      COLOREDITOR_SETREDPREVIEW,
      COLOREDITOR_SETGREENPREVIEW,
      COLOREDITOR_SETBLUEPREVIEW,
      COLOREDITOR_SETDISABLECTL,
      COLOREDITOR_SETRGBCHECK,
      COLOREDITOR_SETHSLCHECK,
      COLOREDITOR_SETAUTOAPPLYCHECK,
    };

  private:
		static XMLParamPair params[];
    void updateSet(const wchar_t *set);
    void updateRGBHSLControls();
    void updateControls();
    void updateSourceRect();
    void updateSource();
    int xuihandle;
    int gotTheme(const wchar_t *name);
    void enableEdition(int en);
    StringW str_redslider, str_greenslider, str_blueslider, str_graycheck, str_gray2check, str_boostcheck,
           str_redsource, str_greensource, str_bluesource, str_sourcerect, str_setlist, 
           str_grouplist, str_redpreview, str_greenpreview, str_bluepreview, str_rgbcheck, str_hslcheck, str_autoapplycheck;

    GuiObject *redslider, *greenslider, *blueslider, *graycheck, *gray2check, *boostcheck;
    GuiObject *rgbcheck, *hslcheck, *autoapplycheck;
    GuiObject *redsource, *greensource, *bluesource, *sourcerect;
    ColorEditorSetList *setlist;
    ColorEditorGroupList *grouplist;
    ColorEditorPreviewRect *redpreview, *greenpreview, *bluepreview;

    H_ColorEditorSlider *h_red, *h_green, *h_blue;
    H_ColorEditorCheckBox *h_gray, *h_gray2, *h_boost, *h_rgb, *h_hsl, *h_autoapply;
    H_ColorEditorEdit *h_sourcered, *h_sourcegreen, *h_sourceblue;
    H_ColorEditorGuiObject *h_rectobject;

    StringW cur_set;
    StringW cur_group;

    int rgbhsl;
    int autoapply;
    int inapply;
    int mysetting;

    PtrList<ColorEditorGroup> groups;
    int posted;

    int copy_red, copy_green, copy_blue, copy_gray, copy_boost;
    int _red, _green, _blue;
    int skipeditcallback;
    COLORREF colormem[16];
    StringW retval;
    SkinWnd *inputbox;
    StringW disablectl;
};

// -----------------------------------------------------------------------
class H_ColorEditorSlider: public H_Slider {
  public:

    H_ColorEditorSlider(ColorEditorInstance *cb, ScriptObject *o) : H_Slider(o), callback(cb) {}
    H_ColorEditorSlider(ColorEditorInstance *cb) : H_Slider(), callback(cb) {};
    virtual ~H_ColorEditorSlider() {}

    virtual void hook_onSetPosition(int newpos) { callback->sliderCallback(newpos, this, 0); }
    virtual void hook_onSetFinalPosition(int newpos) { callback->sliderCallback(newpos, this, 1); }

  private:
    ColorEditorInstance *callback;
};

// -----------------------------------------------------------------------
class H_ColorEditorCheckBox: public H_CheckBox {
  public:

    H_ColorEditorCheckBox(ColorEditorInstance *cb, ScriptObject *o) : H_CheckBox(o), callback(cb) {}
    H_ColorEditorCheckBox(ColorEditorInstance *cb) : H_CheckBox(), callback(cb) {};
    virtual ~H_ColorEditorCheckBox() {}

    virtual void hook_onToggle(int newstate) { callback->checkboxCallback(newstate, this); }

  private:
    ColorEditorInstance *callback;
};

// -----------------------------------------------------------------------
class H_ColorEditorEdit : public H_Edit {
  public:

    H_ColorEditorEdit(ColorEditorInstance *cb, ScriptObject *o) : H_Edit(o), callback(cb) {}
    H_ColorEditorEdit(ColorEditorInstance *cb) : H_Edit(), callback(cb) {};
    virtual ~H_ColorEditorEdit() {}

    virtual void hook_onEditUpdate() { callback->editCallback(this); }

  private:
    ColorEditorInstance *callback;
};

// -----------------------------------------------------------------------
class H_ColorEditorGuiObject : public H_GuiObject {
  public:

    H_ColorEditorGuiObject(ColorEditorInstance *cb, ScriptObject *o) : H_GuiObject(o), callback(cb) {}
    H_ColorEditorGuiObject(ColorEditorInstance *cb) : H_GuiObject(), callback(cb) {};
    virtual ~H_ColorEditorGuiObject() {}

    virtual void hook_onLeftButtonDown(int x, int y) { callback->clickCallback(this); }
//    virtual void hook_onMouseMove(int x, int y) { callback->mouseMoveCallback(this, x, y); }

  private:
    ColorEditorInstance *callback;
};

// -----------------------------------------------------------------------
class H_InputEdit : public H_Edit {
  public:

    H_InputEdit(ColorEditorInstance *cb, ScriptObject *o) : H_Edit(o), callback(cb) {}
    H_InputEdit(ColorEditorInstance *cb) : H_Edit(), callback(cb) {};
    virtual ~H_InputEdit() {}

    virtual void hook_onEnter() { callback->onInputEnter(this); }
    virtual void hook_onAbort() { callback->onInputEscape(this); }

  private:
    ColorEditorInstance *callback;
};

// -----------------------------------------------------------------------
extern wchar_t ColorEditorInstanceXuiObjectStr[];
extern char ColorEditorInstanceXuiSvcName[];
class ColorEditorInstanceXuiSvc : public XuiObjectSvc<ColorEditorInstance, ColorEditorInstanceXuiObjectStr, ColorEditorInstanceXuiSvcName> {};

#endif
