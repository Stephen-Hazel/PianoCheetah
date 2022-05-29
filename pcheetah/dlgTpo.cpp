// dlgTpo.cpp - edit tempo

#include "pcheetah.h"

void DlgTpo::Open ()
{  show ();   raise ();   activateWindow ();
   StrCp (_s, Up.pos.etc);
  CtlSpin sp (ui->num, 1, 960);
DBG("HEY in=`s", Up.pos.str);
DBG("etc=`s", _s);
   sp.Set (Str2Int (Up.pos.str));
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgTpo::Shut ()
{ TStr s;
   StrAp (_s, Int2Str (ui->num->value (), s));
DBG("HEY done=`s", _s);
   emit sgCmd (_s);
   done (true);   lower ();   hide ();
}

void DlgTpo::Init ()  {Gui.DlgLoad (this, "DlgTpo");}
void DlgTpo::Quit ()  {Gui.DlgSave (this, "DlgTpo");}
