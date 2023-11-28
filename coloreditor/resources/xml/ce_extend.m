/*************************************************************

  ce_extend.m
  by pjn123 (v1.00)
  28 September 2008
  www.SkinConsortium.com

  - this script enables you to move the sliders together if you press Ctrl or Shift (Only on RGB mode)

*************************************************************/

#include <lib/std.mi>

Global Group myGroup;
Global Slider r_slider, g_slider, b_slider;
Global boolean key_shift = false;
Global boolean md_r, md_g, md_b;
Global int r_pos, g_pos, b_pos;
Global checkbox hslmode;

System.onScriptLoaded() {
	myGroup = getScriptGroup();
	r_slider = myGroup.findObject("coloreditor.slider.red");
	g_slider = myGroup.findObject("coloreditor.slider.green");
	b_slider = myGroup.findObject("coloreditor.slider.blue");
	hslmode = myGroup.findObject("coloreditor.checkbox.hsl");
	
	md_r=false;
	md_g=false;
	md_b=false;
}

r_slider.onSetPosition(int newpos){
	if(md_r && !hslmode.isChecked()){
		if(isKeyDown(VK_CONTROL)){
			if(g_slider.getPosition()!=newpos) g_slider.setPosition(newpos);
			if(b_slider.getPosition()!=newpos) b_slider.setPosition(newpos);
		}
		else if(isKeyDown(VK_SHIFT)){
			if(g_slider.getPosition()!=newpos-(r_pos-g_pos)) g_slider.setPosition(newpos-(r_pos-g_pos));
			if(b_slider.getPosition()!=newpos-(r_pos-b_pos)) b_slider.setPosition(newpos-(r_pos-b_pos));
		}
	}
}
g_slider.onSetPosition(int newpos){
	if(md_g && !hslmode.isChecked()){
		if(isKeyDown(VK_CONTROL)){
			if(r_slider.getPosition()!=newpos) r_slider.setPosition(newpos);
			if(b_slider.getPosition()!=newpos) b_slider.setPosition(newpos);
		}
		else if(isKeyDown(VK_SHIFT)){
			if(r_slider.getPosition()!=newpos-(g_pos-r_pos)) r_slider.setPosition(newpos-(g_pos-r_pos));
			if(b_slider.getPosition()!=newpos-(g_pos-b_pos)) b_slider.setPosition(newpos-(g_pos-b_pos));
		}
	}
}
b_slider.onSetPosition(int newpos){
	if(md_b && !hslmode.isChecked()){
		if(isKeyDown(VK_CONTROL)){
			if(r_slider.getPosition()!=newpos) r_slider.setPosition(newpos);
			if(g_slider.getPosition()!=newpos) g_slider.setPosition(newpos);
		}
		else if(isKeyDown(VK_SHIFT)){
			if(r_slider.getPosition()!=newpos-(b_pos-r_pos)) r_slider.setPosition(newpos-(b_pos-r_pos));
			if(g_slider.getPosition()!=newpos-(b_pos-g_pos)) g_slider.setPosition(newpos-(b_pos-g_pos));
		}
	}
}

System.onKeyDown(String key){
	if(key=="shift"){
		if(!key_shift){
			r_pos = r_slider.getPosition();
			g_pos = g_slider.getPosition();
			b_pos = b_slider.getPosition();
			key_shift=true;
		}
	}
	else key_shift=false;
}

r_slider.onLeftButtonDown(int x, int y){md_r=true;}
r_slider.onLeftButtonUp(int x, int y){md_r=false;}

g_slider.onLeftButtonDown(int x, int y){md_g=true;}
g_slider.onLeftButtonUp(int x, int y){md_g=false;}

b_slider.onLeftButtonDown(int x, int y){md_b=true;}
b_slider.onLeftButtonUp(int x, int y){md_b=false;}