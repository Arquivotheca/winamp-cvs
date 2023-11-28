#include <precomp.h>
#include "spanbar.h"
#include <api/api.h>
#include <api/core/api_core.h>

const wchar_t panBarXuiStr[] = L"PanBar"; // This is the xml tag
char panBarXuiSvcName[] = "PanBar xui object"; // this is the name of the xuiservice


#define SPANBAR_NOTIFY_MSG NUM_NOTIFY_MESSAGES+0x1000

SPanBar::SPanBar() {
	setDrawOnBorders(TRUE);
  setEnable(TRUE);
  setHotPosition(127);
  locked = 0;
}

SPanBar::~SPanBar() {
  WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SPanBar::onInit() {
  SPANBAR_PARENT::onInit();

  corecb_onPanChange(WASABI_API_MEDIACORE->core_getPan(0));
  WASABI_API_MEDIACORE->core_addCallback(0, this);
  
  return 1;
}

int SPanBar::onSetPosition() {
  SPANBAR_PARENT::onSetPosition();
  int pos = getSliderPosition();	// get slider pos
  int p=pos-127;
  WASABI_API_MEDIACORE->core_setPan(0,p);
  /*{
    String b;
    if(!p) b="Center";
    else if(p>0) b.printf("%i%% Right",p*100/127);
    else b.printf("%i%% Left",(-p)*100/127);
    WASABI_API_MEDIACORE->status_setText(StringPrintf("Balance: %s",b.getValue()));
  }*/
  return 1;
}

int SPanBar::corecb_onPanChange(int newpan) {
  if (getSeekStatus()) return 0;
  int pos = newpan+127;
  setPosition(pos,0);
  onPostedPosition(pos);
  return 0;
}

void SPanBar::lock() {
  if (locked) return;
  locked = 1;
  WASABI_API_MEDIACORE->core_delCallback(0, this);
}

void SPanBar::unlock() {
  if (!locked) return;
  locked = 0;
  WASABI_API_MEDIACORE->core_addCallback(0, this);
}