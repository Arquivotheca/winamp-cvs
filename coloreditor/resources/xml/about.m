/*---------------------------------------------------
-----------------------------------------------------
Filename:	about.m
Version:	1.0

Type:		maki
Date:		15. Okt. 2006 - 16:30 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>

Global Text txt;
Global Timer tmr;
Global Int count = 0;

System.onScriptLoaded ()
{
	txt = getScriptGroup().FindObject("coloreditor.version2");
	tmr = new Timer;
	tmr.setDelay(4000);
	tmr.start();

}

System.onscriptunloading()
{
	txt.cancelTarget();
	tmr.stop();
	delete tmr;
}

tmr.onTimer ()
{
	if (txt == NULL) return;
	txt.setTargetA(0);
	txt.setTargetSpeed(0.3);
	txt.gotoTarget();
}

txt.onTargetReached ()
{
	if (txt == NULL) return;
	if (getAlpha() == 0)
	{
		count++;
		if (txt == NULL) return;
		if (count == 5) count = 0;
		if (count == 0) txt.setText("Main Development: Ben Allison");
		if (count == 1) txt.setText("Development: Martin Poehlmann");
		if (count == 2) txt.setText("Original Code: Francis Gastellu");
		if (count == 3) txt.setText("Editbox Extension: Jory 'Leechbite' Compendio");
		if (count == 4) txt.setText("XML mods, fixes: Pieter 'pjn123' Nieuwoudt");
		txt.setTargetA(255);
		txt.setTargetSpeed(0.3);
		txt.gotoTarget();
	}
}


