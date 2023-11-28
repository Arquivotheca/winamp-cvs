/*************************************************************

  coloreditor.m
  by Leechbite (v1.02)

  - this script is used for the coloreditor extension.
  - fixes by pjn123 (24-Sep-2008)

*************************************************************/

#include <lib/std.mi>

Global group frameGroup;
Global edit rededit, greenedit, blueedit;
Global slider redslider, greenslider, blueslider;
Global checkbox autoapply, hslmode, rgbmode;
Global button apply;
Global boolean localchange = false;
Global string lastR,lastG,lastB;

System.onScriptLoaded() {

  frameGroup = getScriptGroup();

  rededit = frameGroup.findObject("red.edit");
  greenedit = frameGroup.findObject("green.edit");
  blueedit = frameGroup.findObject("blue.edit");

  redslider = frameGroup.findObject("coloreditor.slider.red");
  greenslider = frameGroup.findObject("coloreditor.slider.green");
  blueslider = frameGroup.findObject("coloreditor.slider.blue");

  autoapply = frameGroup.findObject("coloreditor.checkbox.autoapply");
  hslmode = frameGroup.findObject("coloreditor.checkbox.hsl");
  rgbmode = frameGroup.findObject("coloreditor.checkbox.rgb");
  apply = frameGroup.findObject("coloreditor.apply");
  
  redslider.onSetPosition(redslider.getPosition());
  greenslider.onSetPosition(greenslider.getPosition());
  blueslider.onSetPosition(blueslider.getPosition());
}

system.onScriptUnloading() {
  return;
}

hslmode.onToggle(int newstate) {
  redslider.onPostedPosition(redslider.getPosition());
  greenslider.onPostedPosition(greenslider.getPosition());
  blueslider.onPostedPosition(blueslider.getPosition());
}
rgbmode.onToggle(int newstate) {
  redslider.onPostedPosition(redslider.getPosition());
  greenslider.onPostedPosition(greenslider.getPosition());
  blueslider.onPostedPosition(blueslider.getPosition());
}

redslider.onSetPosition(int newp) {

  if (!hslmode.isChecked()) newp = newp-4096;
  
  localchange = true;
  lastR=integertoString(newp);
  rededit.setText(integertoString(newp));
  localchange = false;
}

greenslider.onSetPosition(int newp) {

  if (!hslmode.isChecked()) newp = newp-4096;

  localchange = true;
  lastG=integertoString(newp);
  greenedit.setText(integertoString(newp));
  localchange = false;
}

blueslider.onSetPosition(int newp) {

  if (!hslmode.isChecked()) newp = newp-4096;
  
  localchange = true;
  lastB=integertoString(newp);
  blueedit.setText(integertoString(newp));
  localchange = false;
}

redslider.onPostedPosition(int newp) {
	debugString("ce..red="+integerToSTring(newp),9);
	debugString("hslmode="+integerToSTring(hslmode.isChecked()),9);
  if (!hslmode.isChecked()) newp = newp-4096;
	rededit.setText(integertoString(newp));
}
greenslider.onPostedPosition(int newp) {
  if (!hslmode.isChecked()) newp = newp-4096;
	greenedit.setText(integertoString(newp));
}
blueslider.onPostedPosition(int newp) {
  if (!hslmode.isChecked()) newp = newp-4096;
	blueedit.setText(integertoString(newp));
}

rededit.onEnter() {
  //if (localchange) return;
  string newval = getText();
  if (lastR == newval) return;
  lastR = newval;

  // checks if sliders are disabled.
  int lastp;
  if (redslider.getAlpha()<255) {  
    if (!hslmode.isChecked()) lastp = redslider.getPosition()-4096;
    rededit.setText(integertoString(lastp));
    return;
  }

  if (newval=="") {
    redslider.onSetFinalPosition(redslider.getPosition());
    return;
  }

  redslider.setPosition(stringToInteger(newval)+4096);
  
  if (autoapply.isChecked()) apply.leftclick();

}

greenedit.onEnter() {
  if (localchange) return;
  string newval = getText();
  if (lastG == newval) return;
  lastG = newval;

  // checks if sliders are disabled.
  int lastp;
  if (greenslider.getAlpha()<255) {  
    if (!hslmode.isChecked()) lastp = greenslider.getPosition()-4096;
    greenedit.setText(integertoString(lastp));
    return;
  }

  if (newval=="") {
    greenslider.onSetFinalPosition(greenslider.getPosition());
    return;
  }

  greenslider.setPosition(stringToInteger(newval)+4096);
  
  if (autoapply.isChecked()) apply.leftclick();

}

blueedit.onEnter() {
  if (localchange) return;
  string newval = getText();
  if (lastB == newval) return;
  lastB = newval;

  // checks if sliders are disabled.
  int lastp;
  if (blueslider.getAlpha()<255) {  
    if (!hslmode.isChecked()) lastp = blueslider.getPosition()-4096;
    blueedit.setText(integertoString(lastp));
    return;
  }

  if (newval=="") {
    blueslider.onSetFinalPosition(blueslider.getPosition());
    return;
  }

  blueslider.setPosition(stringToInteger(newval)+4096);
  
  if (autoapply.isChecked()) apply.leftclick();

}