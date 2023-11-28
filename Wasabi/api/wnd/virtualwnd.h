#ifndef _VIRTUALWND_H
#define _VIRTUALWND_H

#include <api/wnd/basewnd.h>

#define VIRTUALWND_PARENT BaseWnd
#define AUTOWH 0xFFFE
#define NOCHANGE 0xFFFD

class NOVTABLE VirtualWnd : public VIRTUALWND_PARENT {
protected:
  VirtualWnd();
  virtual ~VirtualWnd();
public:
  virtual int init(ifc_window *parent, int nochild=FALSE);
  virtual int init(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild=FALSE);

  virtual void bringToFront();
  virtual void bringToBack();
  virtual void bringAbove(BaseWnd *w);
  virtual void bringBelow(BaseWnd *w);

  //NONPORTABLE--avoid prolonged use
  virtual OSWINDOWHANDLE getOsWindowHandle();
  virtual OSMODULEHANDLE getOsModuleHandle();

public:
  virtual void resize(int x, int y, int w, int h, int wantcb=1);
  virtual void resize(RECT *r, int wantcb=1);
  virtual void move(int x, int y);
  virtual void invalidate();
  virtual void invalidateRect(RECT *r);
  virtual void invalidateRgn(api_region *reg);
  virtual void validate();
  virtual void validateRect(RECT *r);
  virtual void validateRgn(api_region *reg);
  virtual void getClientRect(RECT *);
  virtual void getNonClientRect(RECT *);
  virtual void getWindowRect(RECT *);
  virtual int beginCapture();
  virtual int endCapture();
  virtual int getCapture();
  virtual void setVirtualChildCapture(BaseWnd *child);
  virtual void repaint();
/*  virtual int focusNextSibbling(int dochild);
  virtual int focusNextVirtualChild(BaseWnd *child);*/
  virtual int cascadeRepaint(int pack=1); 
  virtual int cascadeRepaintRect(RECT *r, int pack=1); 
  virtual int cascadeRepaintRgn(api_region *r, int pack=1); 
  virtual ifc_window *rootWndFromPoint(POINT *pt);
  virtual double getRenderRatio();
  virtual int reparent(ifc_window *newparent);
  virtual int setVirtual(int i);
  virtual ifc_window *getRootParent();
  virtual int gotFocus();
  virtual int onGetFocus();
  virtual int onKillFocus();
  virtual void setFocus();
  virtual int onActivate();
  virtual int onDeactivate();
  virtual void setVirtualChildFocus(ifc_window *child);
  virtual int wantFocus() { return 0; }
  virtual void setAllowDeactivation(int allow);
  virtual int allowDeactivation();

public:
  virtual int isVirtual() { return !bypassvirtual; }

protected:
  int virtualX,virtualY,virtualH,virtualW;
  int bypassvirtual;
  int focus;
  int resizecount;
  double lastratio;
};

#endif
