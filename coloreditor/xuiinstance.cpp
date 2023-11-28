#include "precomp.h"
#include "xuiinstance.h"
#include "xuipreviewrect.h"
#include <api/api.h>
#include <api/wndmgr/msgbox.h>
#include <api/script/objects/c_script/c_slider.h>
#include <api/script/objects/c_script/c_checkbox.h>
#include <api/script/objects/c_script/c_edit.h>
#include <api/script/objects/c_script/c_guiobject.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/objects/c_script/c_container.h>
#include <api/script/objects/c_script/c_layout.h>
#include <api/script/objects/c_script/c_button.h>
#include "coloreditor.h"
#include <bfc/parse/paramparser.h>
#include "../nu/AutoChar.h"
#include <wasabicfg.h>
#ifdef WIN32
#include <commdlg.h>
#endif

#include <api/service/svc_enum.h>

#define VERSION L"2.2.1"

// -----------------------------------------------------------------------
wchar_t ColorEditorInstanceXuiObjectStr[] = L"ColorEditor:Instance"; // This is the xml tag
char ColorEditorInstanceXuiSvcName[] = "ColorEditor:Instance XuiObject Service";


XMLParamPair ColorEditorInstance::params[] =
{
	COLOREDITOR_SETREDSLIDER, L"redslider", 
  COLOREDITOR_SETGREENSLIDER, L"greenslider", 
  COLOREDITOR_SETBLUESLIDER, L"blueslider", 
  COLOREDITOR_SETGRAYCHECK, L"graycheck", 
  COLOREDITOR_SETGRAY2CHECK, L"gray2check", 
  COLOREDITOR_SETBOOSTCHECK, L"boostcheck", 
  COLOREDITOR_SETREDSOURCE, L"redsource", 
  COLOREDITOR_SETGREENSOURCE, L"greensource", 
  COLOREDITOR_SETBLUESOURCE, L"bluesource", 
  COLOREDITOR_SETSOURCERECT, L"sourcerect", 
  COLOREDITOR_SETSETLIST, L"setlist", 
  COLOREDITOR_SETGROUPLIST, L"grouplist", 
  COLOREDITOR_SETREDPREVIEW, L"redpreview", 
  COLOREDITOR_SETGREENPREVIEW, L"greenpreview", 
  COLOREDITOR_SETBLUEPREVIEW, L"bluepreview", 
  COLOREDITOR_SETDISABLECTL, L"disablecontrols", 
  COLOREDITOR_SETRGBCHECK, L"rgbcheck", 
  COLOREDITOR_SETHSLCHECK, L"hslcheck", 
  COLOREDITOR_SETAUTOAPPLYCHECK, L"autoapplycheck", 
};


// -----------------------------------------------------------------------
ColorEditorInstance::ColorEditorInstance() 
{
  redslider = greenslider = blueslider = graycheck = gray2check = boostcheck = NULL;
  redsource = greensource = bluesource = sourcerect = NULL;
  setlist = NULL;
  grouplist = NULL;

  h_red = h_green = h_blue = NULL;
  h_gray = h_gray2 = h_boost = NULL;
  h_sourcered = h_sourcegreen = h_sourceblue = NULL;
  h_rectobject = NULL;

  h_hsl = h_rgb = NULL;

  h_autoapply = NULL;

  redpreview = greenpreview = bluepreview = NULL;
  copy_red = copy_green = copy_blue = copy_gray = copy_boost = UNDEFINED;

  posted = 0;
  _red = _green = _blue = 127;
  for (int i=0;i<16;i++) 
    colormem[i] = WASABI_API_CONFIG->getIntPrivate(StringPrintfW(L"Color Editor/custom_color%d", i), RGB(255,255,255));

  rgbhsl = WASABI_API_CONFIG->getIntPrivate(L"Color Editor/color_mode", 0); // default to rgb
  autoapply = WASABI_API_CONFIG->getIntPrivate(L"Color Editor/auto_apply", 0); // default to no
  
  skipeditcallback = 0;
  inapply = 0;
  mysetting = 0;

  Std::createDirectory(StringPathCombine(WASABI_API_APP->path_getUserSettingsPath(), L"Plugins\\Freeform\\wacs\\coloreditor\\data"));

  xuihandle = newXuiHandle();
		int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ColorEditorInstance::~ColorEditorInstance() {
  delete h_red;
  delete h_green;
  delete h_blue;
  delete h_gray;
  delete h_gray2;
  delete h_boost;
  delete h_sourcered;
  delete h_sourcegreen;
  delete h_sourceblue;
  delete h_rectobject;
  delete h_rgb;
  delete h_hsl;
  delete h_autoapply;
  groups.deleteAll();
}

// -----------------------------------------------------------------------
int ColorEditorInstance::onInit() {
  int rt = COLOREDITORINSTANCE_PARENT::onInit();
  return rt;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::onDeferredCallback(intptr_t p1, intptr_t p2) {
  if (p1 == DCB_INIT) {
    attachControls();
  } else
    return COLOREDITORINSTANCE_PARENT::onDeferredCallback(p1, p2);
  return 1;
}


// -----------------------------------------------------------------------
int ColorEditorInstance::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *value) 
{
  if (_xuihandle != xuihandle) return COLOREDITORINSTANCE_PARENT::setXuiParam(_xuihandle, attrid, name, value);
  switch (attrid) {
    case COLOREDITOR_SETREDSLIDER:
      str_redslider = value;
      attachControls();
      break;
    case COLOREDITOR_SETGREENSLIDER:
      str_greenslider = value;
      attachControls();
      break;
    case COLOREDITOR_SETBLUESLIDER:
      str_blueslider = value;
      attachControls();
      break;
    case COLOREDITOR_SETGRAYCHECK:
      str_graycheck = value;
      attachControls();
      break;
    case COLOREDITOR_SETGRAY2CHECK:
      str_gray2check = value;
      attachControls();
      break;
    case COLOREDITOR_SETBOOSTCHECK:
      str_boostcheck = value;
      attachControls();
      break;
    case COLOREDITOR_SETREDSOURCE:
      str_redsource = value;
      attachControls();
      break;
    case COLOREDITOR_SETGREENSOURCE:
      str_greensource = value;
      attachControls();
      break;
    case COLOREDITOR_SETBLUESOURCE:
      str_bluesource = value;
      attachControls();
      break;
    case COLOREDITOR_SETSOURCERECT:
      str_sourcerect = value;
      attachControls();
      break;
    case COLOREDITOR_SETSETLIST:
      str_setlist = value;
      attachControls();
      break;
    case COLOREDITOR_SETGROUPLIST:
      str_grouplist = value;
      attachControls();
      break;
    case COLOREDITOR_SETREDPREVIEW:
      str_redpreview = value;
      attachControls();
      break;
    case COLOREDITOR_SETGREENPREVIEW:
      str_greenpreview = value;
      attachControls();
      break;
    case COLOREDITOR_SETBLUEPREVIEW:
      str_bluepreview = value;
      attachControls();
      break;
    case COLOREDITOR_SETDISABLECTL:
      disablectl = value;
      attachControls();
      break;
    case COLOREDITOR_SETRGBCHECK:
      str_rgbcheck = value;
      attachControls();
      break;
    case COLOREDITOR_SETHSLCHECK:
      str_hslcheck = value;
      attachControls();
      break;
    case COLOREDITOR_SETAUTOAPPLYCHECK:
      str_autoapplycheck = value;
      attachControls();
      break;
    default: return 0;
  }
  return 1;
}


// -----------------------------------------------------------------------
void ColorEditorInstance::attachControls() {
 
  if (!isPostOnInit()) {
    posted = 1;
    postDeferredCallback(DCB_INIT, 0);
    return;
  }  

  GuiObject *o = findObject(str_setlist);

  if (o != NULL) {
    ScriptObject *so = o->guiobject_getScriptObject();
    if (so != NULL) {
      setlist = static_cast<ColorEditorSetList *>(so->vcpu_getInterface(COLOREDIT_SETLIST_GUID));
      if (setlist != NULL)
        setlist->setInstanceCallback(this);
    }
  }
  if (setlist == NULL) return;
  
  redslider = findObject(str_redslider);
  greenslider = findObject(str_greenslider);
  blueslider = findObject(str_blueslider);
  graycheck = findObject(str_graycheck);
  gray2check = findObject(str_gray2check);
  boostcheck = findObject(str_boostcheck);
  redsource = findObject(str_redsource);
  greensource = findObject(str_greensource);
  bluesource = findObject(str_bluesource);
  sourcerect = findObject(str_sourcerect);
  rgbcheck = findObject(str_rgbcheck);
  hslcheck = findObject(str_hslcheck);
  autoapplycheck = findObject(str_autoapplycheck);
  
  o = findObject(str_grouplist);
  if (o != NULL) {
    ScriptObject *so = o->guiobject_getScriptObject();
    if (so != NULL) {
      grouplist = static_cast<ColorEditorGroupList *>(so->vcpu_getInterface(COLOREDIT_GROUPLIST_GUID));
      if (grouplist != NULL) grouplist->setInstanceCallback(this);
    }
  }

  o = findObject(str_redpreview);
  if (o != NULL) {
    ScriptObject *so = o->guiobject_getScriptObject();
    if (so != NULL) {
      redpreview = static_cast<ColorEditorPreviewRect *>(so->vcpu_getInterface(COLOREDIT_PREVIEWRECT_GUID));
      if (redpreview != NULL) redpreview->setInstanceCallback(this);
    }
  }
  o = findObject(str_greenpreview);
  if (o != NULL) {
    ScriptObject *so = o->guiobject_getScriptObject();
    if (so != NULL) {
      greenpreview = static_cast<ColorEditorPreviewRect *>(so->vcpu_getInterface(COLOREDIT_PREVIEWRECT_GUID));
      if (greenpreview != NULL) greenpreview->setInstanceCallback(this);
    }
  }
  o = findObject(str_bluepreview);
  if (o != NULL) {
    ScriptObject *so = o->guiobject_getScriptObject();
    if (so != NULL) {
      bluepreview = static_cast<ColorEditorPreviewRect *>(so->vcpu_getInterface(COLOREDIT_PREVIEWRECT_GUID));
      if (bluepreview != NULL) bluepreview->setInstanceCallback(this);
    }
  }

  delete h_red; h_red = NULL;
  delete h_green; h_green = NULL;
  delete h_blue; h_blue = NULL;

  if (redslider != NULL)
    h_red = new H_ColorEditorSlider(this, redslider->guiobject_getScriptObject());
  if (greenslider != NULL)
    h_green = new H_ColorEditorSlider(this, greenslider->guiobject_getScriptObject());
  if (blueslider != NULL)
    h_blue = new H_ColorEditorSlider(this, blueslider->guiobject_getScriptObject());

  delete h_gray; h_gray = NULL;
  delete h_gray2; h_gray2 = NULL;
  delete h_boost; h_boost = NULL;

  if (graycheck != NULL) 
    h_gray = new H_ColorEditorCheckBox(this, graycheck->guiobject_getScriptObject());
  if (gray2check != NULL) 
    h_gray2 = new H_ColorEditorCheckBox(this, gray2check->guiobject_getScriptObject());
  if (boostcheck != NULL) 
    h_boost = new H_ColorEditorCheckBox(this, boostcheck->guiobject_getScriptObject());

  delete h_sourcered; h_sourcered = NULL;
  delete h_sourcegreen; h_sourcegreen = NULL;
  delete h_sourceblue; h_sourceblue = NULL;

  C_Edit c(redsource->guiobject_getScriptObject());
  c.setText(L"127");
  C_Edit c2(greensource->guiobject_getScriptObject());
  c2.setText(L"127");
  C_Edit c3(bluesource->guiobject_getScriptObject());
  c3.setText(L"127");

  if (redsource != NULL) 
    h_sourcered = new H_ColorEditorEdit(this, redsource->guiobject_getScriptObject());
  if (greensource != NULL) 
    h_sourcegreen = new H_ColorEditorEdit(this, greensource->guiobject_getScriptObject());
  if (bluesource != NULL) 
    h_sourceblue = new H_ColorEditorEdit(this, bluesource->guiobject_getScriptObject());

  delete h_rectobject; h_rectobject = NULL;
  if (sourcerect != NULL) 
    h_rectobject = new H_ColorEditorGuiObject(this, sourcerect->guiobject_getScriptObject());

  groups.deleteAll();
  int ns = WASABI_API_SKIN->colortheme_getNumColorSets();
  for (int i=0;i<ns;i++) {
    const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
    int ng = WASABI_API_SKIN->colortheme_getNumColorGroups(set);
    for (int j=0;j<ng;j++) {
      groups.addItem(new ColorEditorGroup(set, WASABI_API_SKIN->colortheme_enumColorGroup(i, j)));
    }
  }

  delete h_rgb; h_rgb = NULL;
  if (rgbcheck != NULL) h_rgb = new H_ColorEditorCheckBox(this, rgbcheck->guiobject_getScriptObject());
  delete h_hsl; h_hsl = NULL;
  if (hslcheck != NULL) h_hsl = new H_ColorEditorCheckBox(this, hslcheck->guiobject_getScriptObject());
  delete h_autoapply; h_autoapply = NULL;
  if (autoapplycheck != NULL) h_autoapply = new H_ColorEditorCheckBox(this, autoapplycheck->guiobject_getScriptObject());

  setlist->selectCurrentSet(); 
  updateControls();
  updateSourceRect();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::sliderCallback(int newpos, H_ColorEditorSlider *slider, int final) {
  if (mysetting) return;
  C_Slider *firstslider=NULL, *secondslider=NULL, *thirdslider=NULL;
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    firstslider = new C_Slider(redslider->guiobject_getScriptObject());
    secondslider = new C_Slider(greenslider->guiobject_getScriptObject());
    thirdslider = new C_Slider(blueslider->guiobject_getScriptObject());
    if (rgbhsl) {
      int h = getRedAndHueSlider();
      int s = getGreenAndSaturationSlider();
      int l = getBlueAndLuminositySlider();
      int _r, _g, _b;
      hslToRgb(h, s, l, &_r, &_g, &_b);
      g->setRed(_r);
      g->setGreen(_g);
      g->setBlue(_b);
    } else {
      g->setRed(firstslider->getPosition()-4096);
      g->setGreen(secondslider->getPosition()-4096);
      g->setBlue(thirdslider->getPosition()-4096);
    }
    delete firstslider;
    delete secondslider;
    delete thirdslider;
  }
  
  if (redpreview) redpreview->invalidate();
  if (greenpreview) greenpreview->invalidate();
  if (bluepreview) bluepreview->invalidate();
  if (final && autoapply) apply();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::checkboxCallback(int newpos, H_ColorEditorCheckBox *checkbox) {
  if (checkbox == h_gray) {
    ColorEditorGroup *g = getCurGroup();
    if (g != NULL) {
      C_CheckBox c(checkbox->getHookedObject());
      g->setGray(c.isChecked() ? 1 : 0);
      if (c.isChecked() && h_gray2) {
        C_CheckBox c2(h_gray2->getHookedObject());
        c2.setChecked(0);
      }
    }
    if (autoapply) apply();
  } else if (checkbox == h_gray2) {
    ColorEditorGroup *g = getCurGroup();
    if (g != NULL) {
      C_CheckBox c(checkbox->getHookedObject());
      g->setGray(c.isChecked() ? 2 : 0);
      if (c.isChecked() && h_gray) {
        C_CheckBox c2(h_gray->getHookedObject());
        c2.setChecked(0);
      }
    }
    if (autoapply) apply();
  } else if (checkbox == h_boost) {
    ColorEditorGroup *g = getCurGroup();
    if (g != NULL) {
      C_CheckBox c(checkbox->getHookedObject());
      int goingboost = c.isChecked();
      g->setBoost(goingboost);
      if (redsource && greensource && bluesource) {
        if (goingboost) {
          C_Edit c(redsource->guiobject_getScriptObject());
          c.setText(StringPrintfW(L"%d", WTOI(c.getText())+127));
          C_Edit c2(greensource->guiobject_getScriptObject());
          c2.setText(StringPrintfW(L"%d", WTOI(c2.getText())+127));
          C_Edit c3(bluesource->guiobject_getScriptObject());
          c3.setText(StringPrintfW(L"%d", WTOI(c3.getText())+127));
        } else {
          C_Edit c(redsource->guiobject_getScriptObject());
          c.setText(StringPrintfW(L"%d", WTOI(c.getText())-127));
          C_Edit c2(greensource->guiobject_getScriptObject());
          c2.setText(StringPrintfW(L"%d", WTOI(c2.getText())-127));
          C_Edit c3(bluesource->guiobject_getScriptObject());
          c3.setText(StringPrintfW(L"%d", WTOI(c3.getText())-127));
        }
      }
    }
    if (autoapply) apply();
  } else if (checkbox == h_rgb) {
    C_CheckBox c(checkbox->getHookedObject());
    goRGB();
  } else if (checkbox == h_hsl) {
    C_CheckBox c(checkbox->getHookedObject());
    goHSL();
  } else if (checkbox == h_autoapply) {
    C_CheckBox c(checkbox->getHookedObject());
    autoapply = c.isChecked();
  }
  if (redpreview) redpreview->invalidate();
  if (greenpreview) greenpreview->invalidate();
  if (bluepreview) bluepreview->invalidate();

  WASABI_API_CONFIG->setIntPrivate(L"Color Editor/color_mode", rgbhsl);
  WASABI_API_CONFIG->setIntPrivate(L"Color Editor/auto_apply", autoapply);
}

int ColorEditorInstance::getRGBHSL() {
  return rgbhsl;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::editCallback(H_ColorEditorEdit *editor) {
  updateSourceRect();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::updateSourceRect() {
  if (skipeditcallback) return;
  if (redsource != NULL) {
    C_Edit e(redsource->guiobject_getScriptObject());
    _red = WTOI(e.getText());
    if (_red > 255) { _red = 255; e.setText(L"255"); }
    if (_red < 0) { _red = 0; e.setText(L"0"); }
  }
  if (greensource != NULL) {
    C_Edit e(greensource->guiobject_getScriptObject());
    _green = WTOI(e.getText());
    if (_green > 255) { _green = 255; e.setText(L"255"); }
    if (_green < 0) { _green = 0; e.setText(L"0"); }
  }
  if (bluesource != NULL) {
    C_Edit e(bluesource->guiobject_getScriptObject());
    _blue = WTOI(e.getText());
    if (_blue > 255) { _blue = 255; e.setText(L"255"); }
    if (_blue < 0) { _blue = 0; e.setText(L"0"); }
  }
  if (sourcerect) sourcerect->guiobject_setXmlParam(L"color", StringPrintfW(L"%d,%d,%d", _red, _green, _blue));
  if (redpreview) redpreview->invalidate();
  if (greenpreview) greenpreview->invalidate();
  if (bluepreview) bluepreview->invalidate();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::updateSource() {
  skipeditcallback = 1;
  if (redsource != NULL) {
    C_Edit e(redsource->guiobject_getScriptObject());
    e.setText(StringPrintfW(L"%d", _red));
  }
  if (greensource != NULL) {
    C_Edit e(greensource->guiobject_getScriptObject());
    e.setText(StringPrintfW(L"%d", _green));
  }
  if (bluesource != NULL) {
    C_Edit e(bluesource->guiobject_getScriptObject());
    e.setText(StringPrintfW(L"%d", _blue));
  }
  if (sourcerect) sourcerect->guiobject_setXmlParam(L"color", StringPrintfW(L"%d,%d,%d", _red, _green, _blue));
  if (redpreview) redpreview->invalidate();
  if (greenpreview) greenpreview->invalidate();
  if (bluepreview) bluepreview->invalidate();
  skipeditcallback = 0;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::clickCallback(H_ColorEditorGuiObject *object) {
  if (object == h_rectobject /*&& !Std::keyDown(VK_SHIFT)*/) {
#ifdef WIN32
    CHOOSECOLOR cc;
    MEMZERO(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = gethWnd();
    cc.rgbResult = RGB(_red,_green,_blue);
    cc.lpCustColors = colormem;
    cc.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
    if (ChooseColor(&cc) != 0) {
      _blue = (cc.rgbResult & 0xFF0000) >> 16;
      _green = (cc.rgbResult & 0xFF00) >> 8;
      _red = cc.rgbResult & 0xFF;
      updateSource();
      for (int i=0;i<16;i++) 
        WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"Color Editor/custom_color%d", i), colormem[i]);
    }
#endif
  }
}

/*// -----------------------------------------------------------------------
void ColorEditorInstance::mouseMoveCallback(H_ColorEditorGuiObject *object, int x, int y) {
  static int ga=0;
  DebugString("got mouse move %d\n", ga++);
  if (object == h_rectobject && Std::keyDown(VK_SHIFT)) {
    ScriptObject *o = object->getHookedObject();
    if (o) {
      GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
      if (go) {
        RootWnd *tw = go->guiobject_getRootWnd();
        if (tw && tw->isVirtual()) {
          int _x = x, _y = y;
          tw->clientToScreen(&_x, &_y);
          POINT pt = {_x, _y};
          RootWnd *w = WASABI_API_WND->rootWndFromPoint(&pt);
          if (w) {
            screenToClient(&_x, &_y);
            RootWnd *rp = w->getRootParent();
            if (rp) {
              BltCanvas *canvas = static_cast<BltCanvas*>(rp->getFrameBuffer());
              if (canvas) {
                COLORREF *bits = (COLORREF*)canvas->getBits();
                if (bits) {
                  int w, h;
                  canvas->getDim(&w, &h, NULL);
                  if (w > _y && h > _x) {
                    bits += w * _y + _x;
                    setPreviewRed((*bits & 0xFF0000) >> 16);
                    setPreviewGreen((*bits & 0xFF00) >> 8);
                    setPreviewBlue((*bits & 0xFF));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}*/

// -----------------------------------------------------------------------
void ColorEditorInstance::setCallback(const wchar_t *set) {
  cur_set = set;
  WASABI_API_SKIN->colortheme_updateColorSet(set);

  int n = WASABI_API_SKIN->colortheme_getNumColorGroups(set);
  for (int i=0;i<n;i++) {
    if (!findGroup(set, WASABI_API_SKIN->colortheme_enumColorGroupName(set, i))) {
      groups.addItem(new ColorEditorGroup(set, WASABI_API_SKIN->colortheme_getColorGroup(set, WASABI_API_SKIN->colortheme_enumColorGroupName(set, i))));
    }
  }

  if (grouplist != NULL) {
    grouplist->setSet(cur_set, 1);
  }
  if (!_wcsnicmp(cur_set, L"{coloredit}", 11)) {
    enableEdition(1);
  } else {
    enableEdition(0);
  }
}

// -----------------------------------------------------------------------
ColorEditorGroup *ColorEditorInstance::getCurGroup() {
  for (int i=0;i<groups.getNumItems();i++) {  
    ColorEditorGroup *g = groups.enumItem(i);
    if (WCSCASEEQLSAFE(cur_set, g->getColorSet()) && WCSCASEEQLSAFE(cur_group, g->getName()))
      return g;
  }
  return NULL;
}

// -----------------------------------------------------------------------
ColorEditorGroup *ColorEditorInstance::findGroup(const wchar_t *set, const wchar_t *group) {
  for (int i=0;i<groups.getNumItems();i++) {  
    ColorEditorGroup *g = groups.enumItem(i);
    if (WCSCASEEQLSAFE(set, g->getColorSet()) && WCSCASEEQLSAFE(group, g->getName()))
      return g;
  }
  return NULL;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::groupCallback(const wchar_t *group) {
  cur_group = group;
  updateControls();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::updateRGBHSLControls() {
  if (rgbhsl == 0) {
    if (h_hsl) {
      C_CheckBox c(h_hsl->getHookedObject());
      if (c.isChecked()) c.setChecked(0);
    }
    if (h_rgb) {
      C_CheckBox c2(h_rgb->getHookedObject());
      if (!c2.isChecked()) c2.setChecked(1);
    }
  } else if (rgbhsl == 1) {
    if (h_rgb) {
      C_CheckBox c(h_rgb->getHookedObject());
      if (c.isChecked()) c.setChecked(0);
    }
    if (h_hsl) {
      C_CheckBox c2(h_hsl->getHookedObject());
      if (!c2.isChecked()) c2.setChecked(1);
    }
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::updateControls() {
  if (inapply) return;
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    int _r = g->getRed();
    int _g = g->getGreen();
    int _b = g->getBlue();
    if (rgbhsl) {
      int h, s, l;
      rgbToHsl(_r, _g, _b, &h, &s, &l);
      _r = h;
      _g = s;
      _b = l;
    }
    mysetting = 1;
    if (h_red != NULL) {
      C_Slider s(h_red->getHookedObject());
      s.setXmlParam(L"low", L"0");
      s.setXmlParam(L"high", L"8192");
      s.setXmlParam(L"hotpos", L"4096");
      s.setXmlParam(L"hotrange", L"128");
      s.setPosition(_r+4096);
    }
    if (h_green != NULL) {
      C_Slider s(h_green->getHookedObject());
      s.setXmlParam(L"low", L"0");
      if (rgbhsl) {
        s.setXmlParam(L"high", L"16384");
        s.setXmlParam(L"hotpos", L"8192");
        s.setXmlParam(L"hotrange", L"256");
      } else {
        s.setXmlParam(L"high", L"8192");
        s.setXmlParam(L"hotpos", L"4096");
        s.setXmlParam(L"hotrange", L"128");
      }
      s.setPosition(_g+4096);
    }
    if (h_blue != NULL) {
      C_Slider s(h_blue->getHookedObject());
      s.setXmlParam(L"low", L"0");
      s.setXmlParam(L"high", L"8192");
      s.setXmlParam(L"hotpos", L"4096");
      s.setXmlParam(L"hotrange", L"128");
      s.setPosition(_b+4096);
    }
    mysetting = 0;
    if (redpreview) redpreview->invalidate();
    if (greenpreview) greenpreview->invalidate();
    if (bluepreview) bluepreview->invalidate();
    if (h_gray != NULL) {
      C_CheckBox c(h_gray->getHookedObject());
      c.setChecked(g->getGray() == 1);
    }
    if (h_gray2 != NULL) {
      C_CheckBox c(h_gray2->getHookedObject());
      c.setChecked(g->getGray() == 2);
    }
    if (h_boost!= NULL) {
      C_CheckBox c(h_boost->getHookedObject());
      c.setChecked(g->getBoost());
    }
    updateRGBHSLControls();
    if (h_autoapply) {
      C_CheckBox c(h_autoapply->getHookedObject());
      c.setChecked(autoapply);  
    }
  }
}

// -----------------------------------------------------------------------
int ColorEditorInstance::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) 
{
  if (!_wcsicmp(action, L"APPLY")) 
    apply();
  else if (!_wcsicmp(action, L"REVERT")) 
    revert();
  else if (!_wcsicmp(action, L"SAVE"))
    save();
  else if (!_wcsicmp(action, L"NEW"))
    newtheme();
  else if (!_wcsicmp(action, L"COPY"))
    copy();
  else if (!_wcsicmp(action, L"PASTE"))
    paste();
  else if (!_wcsicmp(action, L"RENAME"))
    rename();
  else if (!_wcsicmp(action, L"CLONE"))
    clone();
  else if (!_wcsicmp(action, L"DELETE"))
    deletetheme();
  else if (!_wcsicmp(action, L"EXPORT"))
    export();
  else if (!_wcsicmp(action, L"ABOUT"))
    about();
  else if (!_wcsicmp(action, L"FAQ"))
    faq();
  else
    return COLOREDITORINSTANCE_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
  return 1;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::apply() {
  if (inapply) return;
  inapply = 1;
  if (_wcsnicmp(cur_set, L"{coloredit}", 11)) {
    int r = WASABI_API_WNDMGR->messageBox(L"This theme cannot be edited because it is an integral part of the skin, would you like to clone it ?", L"Locked theme", MSGBOX_YES|MSGBOX_NO, NULL, this);
    if (r == MSGBOX_YES) {
      clone();
    }
    return;
  }
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    g->apply();
    WASABI_API_SKIN->colortheme_setColorSet(cur_set);
    updateControls();
  }
  inapply = 0;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::revert() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    g->cancel();
    WASABI_API_SKIN->colortheme_setColorSet(cur_set);
    updateControls();
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::save() 
{
  int r = WASABI_API_WNDMGR->messageBox(L"This will save all your theme settings, click OK to continue.", L"Save", MSGBOX_OK|MSGBOX_CANCEL, L"coloreditorsave", this);
  if (r == MSGBOX_OK) 
    saveAll();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::saveAll(const wchar_t *filename) 
{
  int formyself = filename == NULL;
	StringW file;
	if (formyself)
	{
		file = StringPathCombine(WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
		file.AppendPath(L"freeform");
		file.AppendPath(L"wacs");
		file.AppendPath(L"coloreditor");
		file.AppendPath(L"data");
		file.AppendPath(WASABI_API_SKIN->getSkinName());
		file.cat(L".xml");
	}
	else
		file=filename;

  FILE *out = _wfopen(file, L"wt");
  if (out == NULL) 
	{
    WASABI_API_WNDMGR->messageBox(StringPrintfW(L"Could not open %s", file), L"Error", 0, NULL, this);
    return;
  }
  int ns = WASABI_API_SKIN->colortheme_getNumColorSets();
  for (int i=0;i<ns;i++) {
    if (i == 0) {
      fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n\n");

      if (formyself) {
        fprintf(out, "<!-- This file was generated by Wasabi Color Theme Editor v2.0.\n\n");
        
        fprintf(out, "  This is NOT the file you should distribute. To distribute your custom themes, you should\n");
        fprintf(out, "  use the export button on the editor and then create an NSIS package that installs the exported\n");
        fprintf(out, "  file in the plugins\\ColorThemes\\name_of_the_skin directory of Winamp5, under a filename\n");
        fprintf(out, "  that is unique to your color theme pack. -->\n\n");
      } else {
        fprintf(out, "<!-- This file was generated by Wasabi Color Theme Editor v2.0 -->\n\n");
      }

      fprintf(out, "<WasabiXML version=\"1.0\">\n\n");
    }
    const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);

    if (!_wcsnicmp(set, L"{coloredit}", 11)) 
		{
      fprintf(out, StringPrintf("<gammaset id=\"%s\">\n", formyself ? (char *)AutoChar(set, CP_UTF8) : (char *)AutoChar(set+11, CP_UTF8)));

      int ng = WASABI_API_SKIN->colortheme_getNumColorGroups(set);
      for (int j=0;j<ng;j++) 
			{
        ColorEditorGroup *g = findGroup(set, WASABI_API_SKIN->colortheme_enumColorGroupName(set, j));
        if (g != NULL) {
  //        if (STRNICMP(g->getName(), "#
          fprintf(out, StringPrintf("  <gammagroup id=\"%s\" value=\"%d,%d,%d\" gray=\"%d\" boost=\"%d\" />\n", (char *)AutoChar(g->getName(), CP_UTF8), g->getRed(), g->getGreen(), g->getBlue(), g->getGray(), g->getBoost()));
        }
      }
      fprintf(out, "</gammaset>\n\n");
    }
  }
  if (ftell(out) != 0)
    fprintf(out, "</WasabiXML>\n");
  fclose(out);
}

// -----------------------------------------------------------------------
void ColorEditorInstance::newtheme() 
{
  StringW new_set_name = inputBox(L"New Theme");
  if (new_set_name == NULL) return;
  if (gotTheme(new_set_name)) {
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  new_set_name.prepend(L"{coloredit}");
  if (gotTheme(new_set_name)) {
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  WASABI_API_SKIN->colortheme_newColorSet(new_set_name);
  if (setlist) {
    setlist->loadThemes();
    setlist->selectSet(new_set_name);
    int ns = WASABI_API_SKIN->colortheme_getNumColorSets();
    for (int i=0;i<ns;i++) {                  
      const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
      if (!WCSCASEEQLSAFE(set, new_set_name)) continue;
      int ng = WASABI_API_SKIN->colortheme_getNumColorGroups(set);
      for (int j=0;j<ng;j++) {
        groups.addItem(new ColorEditorGroup(set, WASABI_API_SKIN->colortheme_enumColorGroup(i, j)));
      }
    }
    setlist->selectSet(new_set_name);
    setlist->coloreditor_switch();
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::clone() 
{
  StringW clonedset = cur_set;
  if (clonedset.isempty()) return;
  StringW new_set_name = cur_set;
  if (!_wcsnicmp(new_set_name, L"{coloredit}", 11)) 
	 new_set_name.lSplit(10);
  new_set_name.cat(L" copy");
  new_set_name = inputBox(L"Cloned Theme Name", new_set_name);
  if (new_set_name == NULL) return;
  if (gotTheme(new_set_name)) 
	{
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  new_set_name.prepend(L"{coloredit}");
  if (gotTheme(new_set_name)) {
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  WASABI_API_SKIN->colortheme_newColorSet(new_set_name);
  if (setlist) {
    setlist->loadThemes();
    setlist->selectSet(new_set_name);
    int ns = WASABI_API_SKIN->colortheme_getNumColorSets();
    for (int i=0;i<ns;i++) {
      const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
      if (!WCSCASEEQLSAFE(set, new_set_name)) continue;
      int ng = WASABI_API_SKIN->colortheme_getNumColorGroups(set);
      for (int j=0;j<ng;j++) {
        ColorEditorGroup *newgrp = new ColorEditorGroup(set, WASABI_API_SKIN->colortheme_enumColorGroup(i, j));
        newgrp->copy(findGroup(clonedset, newgrp->getName()));
        groups.addItem(newgrp);
      }
    }
  }
  WASABI_API_SKIN->colortheme_setColorSet(new_set_name);
}

// -----------------------------------------------------------------------
void ColorEditorInstance::onInputEnter(H_InputEdit *edit) {
  C_GuiObject g(edit->getHookedObject());
  C_Layout(g.getParentLayout()).endModal(1);
}

// -----------------------------------------------------------------------
void ColorEditorInstance::onInputEscape(H_InputEdit *edit) {
  C_GuiObject g(edit->getHookedObject());
  C_Layout(g.getParentLayout()).endModal(-1);
}

// -----------------------------------------------------------------------
const wchar_t *ColorEditorInstance::inputBox(const wchar_t *title, const wchar_t* defaultText) {
  C_Container(C_Layout(C_GuiObject(getGuiObject()->guiobject_getScriptObject()).getParentLayout()).getContainer()).setXmlParam(L"canclose", L"0");
  inputbox = new SkinWnd(L"wasabi.coloreditor.newthemebox", L"static", 0, NULL, 0, 1);
  C_Container c(inputbox->getContainer());
  c.setName(title);
  GuiObject *gedit = inputbox->findObject(L"wasabi.edit.box");
  if (gedit) {
    H_InputEdit iedit(this, gedit->guiobject_getScriptObject());
	if (defaultText)
	{
	  GuiObject *o = inputbox->findObject(L"coloredit.newtheme.edit");
      if (o != NULL) {
        C_Edit e(o->guiobject_getScriptObject());
        e.setText(defaultText);
	  }
	}
    int r = inputbox->runModal(1);
    C_Container(C_Layout(C_GuiObject(getGuiObject()->guiobject_getScriptObject()).getParentLayout()).getContainer()).setXmlParam(L"canclose", L"1");
    if (r == 1) {
      GuiObject *o = inputbox->findObject(L"coloredit.newtheme.edit");
      if (o != NULL) {
        C_Edit e(o->guiobject_getScriptObject());
        retval = e.getText();
        inputbox->destroy();
        getDesktopParent()->setFocus();
        delete inputbox; inputbox = NULL;
        if (retval.isempty()) return NULL;
        if (!_wcsnicmp(retval, L"{coloredit}", 11)) {
          WASABI_API_WNDMGR->messageBox(L"Invalid name, your set name cannot start with \"{coloredit}\"\nAnd why would you want it to anyway ?", L"Error", MSGBOX_OK, NULL, this);
          return NULL;
        }
        return retval;
      }
    }
    inputbox->destroy();
    getDesktopParent()->setFocus();
    delete inputbox; inputbox = NULL;
  }
  return NULL;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::copy() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    copy_red = g->getRed();
    copy_green = g->getGreen();
    copy_blue = g->getBlue();
    copy_gray = g->getGray();
    copy_boost = g->getBoost();
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::paste() {
  if (copy_red == UNDEFINED) {
#ifdef WIN32
    MessageBeep(-1);
#endif
    return;
  }
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    g->setRed(copy_red );
    g->setGreen(copy_green);
    g->setBlue(copy_blue);
    g->setGray(copy_gray);
    g->setBoost(copy_boost);
    updateControls();
  }
  if (autoapply) apply();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::deletetheme() {
  WASABI_API_SKIN->colortheme_deleteColorSet(cur_set);
  attachControls();
  WASABI_API_SKIN->colortheme_setColorSet(WASABI_API_SKIN->colortheme_enumColorSet(0));
  setlist->loadThemes();
}

// -----------------------------------------------------------------------
int ColorEditorInstance::gotTheme(const wchar_t *name) {
  int n = WASABI_API_SKIN->colortheme_getNumColorSets();
  for (int i=0;i<n;i++) {
    if (WCSCASEEQLSAFE(name, WASABI_API_SKIN->colortheme_enumColorSet(i))) return 1;
  }
  return 0;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::rename() {
  StringW oldsetname = cur_set;
  StringW new_set_name = cur_set;
  new_set_name.lSplit(10);
  new_set_name = inputBox(L"Rename Name", new_set_name);
  if (new_set_name == NULL) {
    return;
  }
  if (gotTheme(new_set_name)) {
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  new_set_name.prepend(L"{coloredit}");
  if (gotTheme(new_set_name)) {
    WASABI_API_WNDMGR->messageBox(L"There is already a theme with that name, please chose another.", L"Duplicate name",MSGBOX_OK, NULL, this);
    return;
  }
  WASABI_API_SKIN->colortheme_renameColorSet(cur_set, new_set_name);
  setlist->loadThemes();
  setlist->selectSet(new_set_name);
  foreach(groups)
    if (!_wcsicmp(groups.getfor()->getColorSet(), oldsetname)) groups.getfor()->setColorSet(new_set_name);
  endfor;
  cur_set = new_set_name;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getRedAndHueSlider() {
  if (redslider) {
    C_Slider s(redslider->guiobject_getScriptObject());
    return s.getPosition()-4096;
  }
  return 0;
}

int ColorEditorInstance::getGreenAndSaturationSlider() {
  if (greenslider) {
    C_Slider s(greenslider->guiobject_getScriptObject());
    return s.getPosition()-4096;
  }
  return 0;
}

int ColorEditorInstance::getBlueAndLuminositySlider() {
  if (blueslider) {
    C_Slider s(blueslider->guiobject_getScriptObject());
    return s.getPosition()-4096;
  }
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::checkSave() {
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getRed() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) 
    return g->getRed();
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getGreen() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) 
    return g->getGreen();
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getBlue() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) 
    return g->getBlue();
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getHue() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    int _r = g->getRed();
    int _g = g->getGreen();
    int _b = g->getBlue();
    int h, s, l;
    rgbToHsl(_r, _g, _b, &h, &s, &l);
    return h;
  }
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getSaturation() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    int _r = g->getRed();
    int _g = g->getGreen();
    int _b = g->getBlue();
    int h, s, l;
    rgbToHsl(_r, _g, _b, &h, &s, &l);
    return s;
  }
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getLuminosity() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) {
    int _r = g->getRed();
    int _g = g->getGreen();
    int _b = g->getBlue();
    int h, s, l;
    rgbToHsl(_r, _g, _b, &h, &s, &l);
    return l;
  }
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getGray() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) 
    return g->getGray();
  return 0;
}

// -----------------------------------------------------------------------
int ColorEditorInstance::getBoost() {
  ColorEditorGroup *g = getCurGroup();
  if (g != NULL) 
    return g->getBoost();
  return 0;
}

// -----------------------------------------------------------------------
COLORREF ColorEditorInstance::filterColor(COLORREF color, int r, int g, int b, int gray, int boost) {

  COLORREF c;

  if (gray == 1) {
    int r,g,b;
    r = (color&0xff0000)>>16;
    g = (color&0xff00)>>8;
		b = (color&0xff);
    c = MAX(MAX(r,g),b);
    c = (c << 16) | (c << 8) | c;
    color = (color & 0xff000000) | c;
  }

  if (gray == 2) {
    int r,g,b;
    r = (color&0xff0000)>>16;
    g = (color&0xff00)>>8;
		b = (color&0xff);
    c = (r+g+b)/3;
    c = (c << 16) | (c << 8) | c;
    color = (color & 0xff000000) | c;
  }

  if (boost) {
    int r,g,b;
    r = ((color&0xff0000)>>17)+0x80;
    g = ((color&0xff00)>>9)+0x80;
	  b = ((color&0xff)>>1)+0x80;
    color = (color&0xff000000)|(r<<16)|(g<<8)|b;
  }

	int bm=65536+(r<<4);
	int gm=65536+(g<<4);
	int rm=65536+(b<<4);

  r = ((((color&0xff0000)>>16)*rm))&0xffff0000;
	c = max(0, min(r, 0xFF0000));
  r = ((((color&0xff00)>>8)*gm)>>8)&0xffff00;
	c |= max(0, min(r, 0xFF00));
  r = (((color&0xff)*bm)>>16)&0xffff;
	c |= max(0, min(r, 0xFF));
	c = (c & 0xFFFFFF) | (color & 0xFF000000);
  return c;
}


// -----------------------------------------------------------------------
int ColorEditorInstance::getChannel(ColorEditorPreviewRect *rect) {
  if (rect == redpreview) return CHANNEL_RED;  
  if (rect == greenpreview) return CHANNEL_GREEN;  
  if (rect == bluepreview) return CHANNEL_BLUE;  
  return 0;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::setPreviewRed(int r) {
  if (redsource) {
    C_Edit c(redsource->guiobject_getScriptObject());
    c.setText(StringPrintfW(L"%d", r));
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::setPreviewGreen(int g) {
  if (greensource) {
    C_Edit c(greensource->guiobject_getScriptObject());
    c.setText(StringPrintfW(L"%d", g));
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::setPreviewBlue(int b) {
  if (bluesource) {
    C_Edit c(bluesource->guiobject_getScriptObject());
    c.setText(StringPrintfW(L"%d", b));
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::enableEdition(int en) {
  if (redslider) redslider->guiobject_setEnabled(en);
  if (greenslider) greenslider->guiobject_setEnabled(en);
  if (blueslider) blueslider->guiobject_setEnabled(en);
  if (graycheck) graycheck->guiobject_setEnabled(en);
  if (gray2check) gray2check->guiobject_setEnabled(en);
  if (boostcheck) boostcheck->guiobject_setEnabled(en);
  ParamParser pp(disablectl);
  for (int i=0;i<pp.getNumItems();i++) {
    GuiObject *obj = findObject(pp.enumItem(i));
    if (obj) obj->guiobject_setEnabled(en);
  }
}

// -----------------------------------------------------------------------
void ColorEditorInstance::export() {
  wchar_t temp[MAX_PATH]=L"";
  OPENFILENAMEW l={sizeof(l),0};
  wchar_t buf2[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH,buf2);
  l.hwndOwner = gethWnd();
  l.lpstrFilter = L"Wasabi XML Color Themes\0*.xml\0All files\0*.*\0";
  l.lpstrFile = temp;
  l.nMaxFile = MAX_PATH-1;
  l.lpstrTitle = L"Save Color Themes";
  l.lpstrDefExt = L"XML";
  l.lpstrInitialDir = NULL;
  l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_OVERWRITEPROMPT; 	        
  if (GetSaveFileNameW(&l)) 
	{
    saveAll(temp);
  }
  SetCurrentDirectoryW(buf2);
}

// -----------------------------------------------------------------------
void ColorEditorInstance::about() 
{
  C_Container(C_Layout(C_GuiObject(getGuiObject()->guiobject_getScriptObject()).getParentLayout()).getContainer()).setXmlParam(L"canclose", L"0");
  SkinWnd w(L"wasabi.coloreditor.about", L"static", 0, NULL, 0, 1);
  GuiObject *go = w.findObject(L"coloreditor.version");
  C_Text(go ? go->guiobject_getScriptObject() : NULL).setText(StringPrintfW(L"v%s", VERSION));
  go = w.findObject(L"coloreditor.copyright");
  C_Text(go ? go->guiobject_getScriptObject() : NULL).setText(L"Copyright © 2003-2009 Nullsoft");
  w.runModal(1);       
  w.destroy();
  C_Container(C_Layout(C_GuiObject(getGuiObject()->guiobject_getScriptObject()).getParentLayout()).getContainer()).setXmlParam(L"canclose", L"1");
  getDesktopParent()->setFocus();
}

// -----------------------------------------------------------------------
void ColorEditorInstance::goRGB() {
  if (rgbhsl == 0) return;
  rgbhsl = 0;
  updateControls();
/*  int h = getRedAndHueSlider();
  int s = getGreenAndSaturationSlider();
  int l = getBlueAndLuminositySlider();
  int r, g, b;
  hslToRgb(h, s, l, &r, &g, &b);
  mysetting = 1;
  if (redslider) C_Slider(redslider->guiobject_getScriptObject()).setPosition(r+4096);
  if (greenslider) C_Slider(greenslider->guiobject_getScriptObject()).setPosition(g+4096);
  if (blueslider) C_Slider(blueslider->guiobject_getScriptObject()).setPosition(b+4096);
  mysetting = 0;
  updateRGBHSLControls();*/
}

// -----------------------------------------------------------------------
void ColorEditorInstance::goHSL() 
{
  if (rgbhsl == 1) return;
  rgbhsl = 1;
  updateControls();
/*  int r = getRedAndHueSlider();
  int g = getGreenAndSaturationSlider();
  int b = getBlueAndLuminositySlider();
  int h, s, l;
  rgbToHsl(r, g, b, &h, &s, &l);
  mysetting = 1;
  if (redslider) C_Slider(redslider->guiobject_getScriptObject()).setPosition(h+4096);
  if (greenslider) C_Slider(greenslider->guiobject_getScriptObject()).setPosition(s+8192);
  if (blueslider) C_Slider(blueslider->guiobject_getScriptObject()).setPosition(l+4096);
  mysetting = 0;
  updateRGBHSLControls();*/
}

// -----------------------------------------------------------------------
void ColorEditorInstance::rgbToHsl(int r, int g, int b, int *h, int *s, int *l) {
  double _r=r, _g=g, _b=b;
  double _h, _s, _l;

  _r /= 4096.0;
  _g /= 4096.0;
  _b /= 4096.0;
  
  double cmax = MAX(_r, MAX(_g, _b));
  double cmin = MIN(_r, MIN(_g, _b));

  _l = (cmax + MAX(cmin, 0.0)) / 2.0;
  if (cmax == cmin) {
    _s = 0.0;
    _h = 0.0;
  } else {
    double diff = cmax - cmin;
    double diff2 = cmax - MAX(cmin, 0.0);
    if (_l < 0.5) { _s = diff / (cmax + MAX(cmin, 0.0)); } else { _s = diff / (2.0 - cmax-MAX(cmin, 0.0)); }
    if (_r == cmax) {
      if (_g < 0 && _b < 0) 
        _h = (_g - _b) / diff; 
      else
        _h = (MAX(_g, 0.0) - MAX(_b, 0.0)) / diff2; 
    } else if (_g == cmax) {
      if (_b < 0 && _r < 0) 
        _h = 2.0 + (_b - _r) / diff; 
      else
        _h = 2.0 + (MAX(_b, 0.0) - MAX(_r,0.0)) / diff2; 
    } else { 
      if (_r < 0 && _g < 0)
        _h = 4.0 + (_r - _g) / diff; 
      else 
        _h = 4.0 + (MAX(_r, 0.0) - MAX(_g, 0.0)) / diff2; 
    }
    _h = _h / 6.0;
  }	

  *h = _h * 4096;
  *s = _s * 8192;
  *l = _l * 4096;
}

#define M_PI 3.1415926536

// -----------------------------------------------------------------------
double deg_to_rad(double deg) {
  return (M_PI*deg)/180.0;
}

// -----------------------------------------------------------------------
void ColorEditorInstance::hslToRgb(int h, int s, int l, int *r, int *g, int *b) 
{
  double _h=h, _s=s, _l=l;
  double _r, _g, _b;
  
  double m1, m2;
  _h /= 4096.0; 
  _s /= 4096.0;
  _l /= 4096.0;

  if (_l >= 0) {
    if (_l <= 0.5) {
      m2 = _l * (1.0 + _s);
    } else {
      m2 = _l + _s - (_l * _s);
    }
  } else {
    if (_l >= -0.5) {
      m2 = _l * (1.0 + _s);
    } else {
      m2 = _l - _s - (_l * _s);
    }
  }

  m1 = 2.0 * _l - m2;

  double _h2 = _h + 1.0 / 3.0;				

  if (_h2 < 0) { _h2 = _h2 + 1.0; }
  if (_h2 > 1) { _h2 = _h2 - 1.0; }

  if (6.0 * _h2 < 1) {
    _r = (m1 + (m2 - m1) * _h2 * 6.0);
  } else {
    if (2.0 * _h2 < 1) {
      _r = m2;
    } else {
      if (3.0 * _h2 < 2.0) { _r = (m1 + (m2 - m1) * ((2.0 / 3.0) - _h2) * 6.0); } else { _r = m1; }
    } 
  }

  if (_h < 0) { _h = _h + 1.0; }
  if (_h > 1) { _h = _h - 1.0; }

  if (6.0 * _h < 1) {
    _g = (m1 + (m2 - m1) * _h * 6.0);
  } else {
    if (2.0 * _h < 1) {
      _g = m2;
    } else {
      if (3.0 * _h < 2.0) { _g = (m1 + (m2 - m1) * ((2.0 / 3.0) - _h) * 6.0);} else { _g = m1; }
    } 
  }

  _h2 = _h - 1.0 / 3.0;				
  
  if (_h2 < 0) { _h2 = _h2 + 1.0; }
  if (_h2 > 1) { _h2 = _h2 - 1.0; }
  
  if (6.0 * _h2 < 1) {
    _b = (m1 + (m2 - m1) * _h2 * 6.0); 
  } else {
    if (2.0 * _h2 < 1) {
      _b = m2;
    } else {
      if (3.0 * _h2 < 2.0) { _b = (m1 + (m2 - m1) * ((2.0 / 3.0) - _h2) * 6.0); } else { _b = m1; }
    } 
  }

  *r = MAX(_r * 4096, -4096.0);
  *g = MAX(_g * 4096, -4096.0);
  *b = MAX(_b * 4096, -4096.0);

}

void ColorEditorInstance::faq() 
{
  Std::shellExec(L"http://dev.winamp.com/w/index.php?title=Color_Editor");
}



#include <api/wnd/wndclass/buttwnd.h>
#include "resource.h"
void ColorEditorInstance::setIconBitmaps(ButtonWnd *button)
{
  extern WAComponentClient *the;
  button->setBitmaps(the->gethInstance(), IDB_TAB_NORMAL, NULL, IDB_TAB_HILITED, IDB_TAB_SELECTED);
}
