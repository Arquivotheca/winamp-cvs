#include "precomp.h"
#include "coloreditor.h"
#include "xuipreviewrect.h"
#include "xuisetlist.h"
#include "xuigrouplist.h"
#include "xuiinstance.h"
#include <tataki/export.h>

#include <api/service/svc_enum.h>
#include <api/service/svcs/svc_wndcreate.h> // WndCreateCreatorSingle
#include <api/wnd/bucketitem.h> // BucketItem
template <class T, class BUCKETCLASS=BucketItem>
class CreateBucketItem : public svc_windowCreateI {
public:
  static const char *getServiceName() { return T::getWindowTypeName(); }
  CreateBucketItem() : issued(0) { }
  virtual int testType(const wchar_t *windowtype) { return WCSEQLSAFE(windowtype, L"buck"); }
  virtual ifc_window *createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n) {
    if (!testType(windowtype)) return NULL;
    if (n == 0) {
      BucketItem *bucketitem = new BUCKETCLASS(T::getWindowTypeGuid(), T::getWindowTypeNameW());
      T::setIconBitmaps(bucketitem);
      issued++;
      return bucketitem;
    }
    return NULL;
  }
  virtual int destroyWindow(ifc_window *w) {
    BUCKETCLASS *wnd = static_cast<BUCKETCLASS *>(w);
    delete wnd;
    issued--;
    return 1;
  }
  virtual int refcount() {
    return issued;
  }
private:
  int issued;
};

template <class T, bool allow_multiple=true>
class CreateWndByGuid : public svc_windowCreateI {
public:
  CreateWndByGuid() : issued(0) { }
  virtual ~CreateWndByGuid() {
    ASSERTPR(issued == 0, "not all created windows were destroyed");
  }
  static const char *getServiceName() { return T::getWindowTypeName(); }
  virtual int testGuid(GUID g) { return (g == T::getWindowTypeGuid()); }
  virtual ifc_window *createWindowByGuid(GUID g, ifc_window *parent) {
    if (!testGuid(g)) return NULL;
    if (!allow_multiple && issued > 0) return NULL;
    ifc_window *ret = WASABI_API_SKIN->group_create(L"wasabi.coloreditor.main");
    issued++;
    return ret;
  }
  virtual int destroyWindow(ifc_window *w) {
    ASSERT(issued >= 1);
    ifc_window *wnd = static_cast<ifc_window*>(w);
		WASABI_API_SKIN->group_destroy(wnd);
    issued--;
    return 1;
  }
  virtual int refcount() {
    return issued;
  }
private:
  int issued;
};

static WACColorEditor wac;
WACPARENT *the = &wac;
WACColorEditor *coloreditorwac = &wac;
HINSTANCE hInstance;
// {EC398636-8736-46b7-B4D1-581A91EFC1A1}
static const GUID coloreditor_guid = 
{ 0xec398636, 0x8736, 0x46b7, { 0xb4, 0xd1, 0x58, 0x1a, 0x91, 0xef, 0xc1, 0xa1 } };

class AutoTataki  : public LoadableResource
{
public:
	virtual void onRegisterServices() { Tataki::Init(WASABI_API_SVC); }
	virtual void onDeregisterServices() { Tataki::Quit(); }
};



// -----------------------------------------------------------------------
WACColorEditor::WACColorEditor() : WAComponentClient(L"Color themes") 
{
	registerResource(new AutoTataki);
  registerService(new XuiObjectCreator<ColorEditorSetListXuiSvc>);
  registerService(new XuiObjectCreator<ColorEditorGroupListXuiSvc>);
  registerService(new XuiObjectCreator<ColorEditorPreviewRectXuiSvc>);
  registerService(new XuiObjectCreator<ColorEditorInstanceXuiSvc>);
  registerSkinFile(L"xml/coloreditor.xml");
	registerService(new WndCreateCreatorSingle< CreateBucketItem<ColorEditorInstance> >);
	registerService(new WndCreateCreatorSingle< CreateWndByGuid<ColorEditorInstance> >);
	hInstance = the->gethInstance();	
}

// -----------------------------------------------------------------------
WACColorEditor::~WACColorEditor() {
}

// -----------------------------------------------------------------------
const wchar_t *WACColorEditor::getName() 
{
  return L"Color Editor";
}

// -----------------------------------------------------------------------
GUID WACColorEditor::getGUID() {
  return coloreditor_guid;
}

// -----------------------------------------------------------------------
void WACColorEditor::onCreate() {
  WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI *>(this));
  WASABI_API_WNDMGR->autopopup_registerGroupId(L"wasabi.coloreditor.main", L"Color Editor", L"resizable_nostatus");
}

// -----------------------------------------------------------------------
void WACColorEditor::onDestroy() {
  WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI *>(this));
  WACPARENT::onDestroy();
}

// -----------------------------------------------------------------------
int WACColorEditor::skincb_onGuiLoaded() {
	StringW oldfile = StringPrintfW(L"%sdata/%s.xml", getComponentPath(), WASABI_API_SKIN->getSkinName()); // Backwards comp
	WASABI_API_SKIN->loadSkinFile(oldfile);
	StringW file = StringPathCombine(WASABI_API_APP->path_getUserSettingsPath(), L"Plugins"); // we use Settings dir for saving, so we should read from it as well.
	file.AppendPath(L"freeform");
	file.AppendPath(L"wacs");
	file.AppendPath(L"coloreditor");
	file.AppendPath(L"data");
	file.AppendPath(WASABI_API_SKIN->getSkinName());
	file.cat(L".xml");
	if (WCSICMP(file, oldfile))
		WASABI_API_SKIN->loadSkinFile(file);
	const wchar_t *s = WASABI_API_SKIN->colortheme_getColorSet();
	if (s != NULL)
		WASABI_API_SKIN->colortheme_setColorSet(s); //reset
	return 0;
}
