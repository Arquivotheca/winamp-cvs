#include "precomp.h"
#include "xuipreviewrect.h"
#include "xuiinstance.h"
#include <api/wnd/canvas.h>
#include <api/api.h>
#include <api/service/svc_enum.h>

// -----------------------------------------------------------------------
wchar_t ColorEditorPreviewRectXuiObjectStr[] = L"ColorEditor:PreviewRect"; // This is the xml tag
char ColorEditorPreviewRectXuiSvcName[] = "ColorEditor:PreviewRect XuiObject Service";


// -----------------------------------------------------------------------
ColorEditorPreviewRect::ColorEditorPreviewRect() {
  getScriptObject()->vcpu_setInterface(COLOREDIT_PREVIEWRECT_GUID, this);
  editor = NULL;
}

// -----------------------------------------------------------------------
ColorEditorPreviewRect::~ColorEditorPreviewRect() {
}

// -----------------------------------------------------------------------
int ColorEditorPreviewRect::onInit() {
  PREVIEWRECT_PARENT::onInit();
  return 1;
}

// -----------------------------------------------------------------------
int ColorEditorPreviewRect::onPaint(Canvas *canvas) {
  int rt = PREVIEWRECT_PARENT::onPaint(canvas);

  if (editor != NULL) {

    int _r = editor->getRedAndHueSlider();
    int _g = editor->getGreenAndSaturationSlider();
    int _b = editor->getBlueAndLuminositySlider();

    int _h = _r;
    int _s = _g;
    int _l = _b;

    int ishsl = (editor->getRGBHSL() == 1);

    int gray = editor->getGray();
    int boost = editor->getBoost();

    int channel = editor->getChannel(this);

    COLORREF input = editor->getInputColor();

    RECT cr;
    getClientRect(&cr);

    int w = cr.right - cr.left;
    float c = 8192.0f/w;

    switch (channel) {
      case CHANNEL_RED: {
        int i=0;
        if (ishsl) {
          int s=_s, l=_l;
          int r, g, b;
          for (float f=0;f<8192;f+=c,i++) {
            editor->hslToRgb((int)(f-4096), _s, _l, &r, &g, &b);
            COLORREF fc = editor->filterColor(input, r, g, b, gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        } else {
          for (float f=0;f<8192;f+=c,i++) {
            COLORREF fc = editor->filterColor(input, (int)(f-4096), _g, _b, gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        }
        break;
      }
      case CHANNEL_GREEN: {
        int i=0;
        if (ishsl) {
          int s=_s, l=_l;
          int r, g, b;
          for (float f=0;f<8192;f+=c,i++) {
            editor->hslToRgb(_h, (int)(f-4096), _l, &r, &g, &b);
            COLORREF fc = editor->filterColor(input, r, g, b, gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        } else {
          for (float f=0;f<8192;f+=c,i++) {
            COLORREF fc = editor->filterColor(input, _r, (int)(f-4096), _b, gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        }
        break;
      }
      case CHANNEL_BLUE: {
        int i=0;
        if (ishsl) {
          int s=_s, l=_l;
          int r, g, b;
          for (float f=0;f<8192;f+=c,i++) {
            editor->hslToRgb(_h, _s, (int)(f-4096), &r, &g, &b);
            COLORREF fc = editor->filterColor(input, r, g, b, gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        } else {
          for (float f=0;f<8192;f+=c,i++) {
            COLORREF fc = editor->filterColor(input, _r, _g, (int)(f-4096), gray, boost);
            canvas->pushPen(fc);
            canvas->moveTo(cr.left+i,cr.top);
            canvas->lineTo(cr.left+i,cr.bottom);
            canvas->popPen();
          }
        }
        break;
      }
    }
  }

  return rt;
}
