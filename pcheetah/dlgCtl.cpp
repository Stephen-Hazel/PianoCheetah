// dlgCtl.cpp - pick a new drum track per section (patA,patB,fill)

#include "pcheetah.h"


void DlgCtl::Upd ()
{ ubyt2 r = _t.CurRow ();
  TStr  s;
   StrCp (s, _t.Get (r, 1));   _t.Set (r, 1, CC((*s == 'y') ? "no" : "yep"));
}


void DlgCtl::Init ()
{  Gui.DlgLoad (this, "DlgCtl");
   _t.Init (ui->t, "control\0show\0");
   connect (ui->t, & QTableWidget::itemClicked, this, & DlgCtl::Upd);
}


void DlgCtl::Open ()
{ ubyte r, c;
  char *ro [3];
   show ();   raise ();   activateWindow ();
   ro [2] = nullptr;
   _t.Open ();
   for (r = 0;  r < Up.nR;  r++) {
      for (c = 0;  c < 2;  c++)  ro [c] = Up.d [r][c];
      _t.Put (ro);
   }
   _t.Shut ();
   Gui.DlgMv (this, Up.gp, "tc");
}


void DlgCtl::Shut ()  
{  Up.nR = _t.NRow ();
   for (ubyte r = 0;  r < Up.nR;  r++)
      for (ubyte c = 0;  c < 2;  c++)
         StrCp (Up.d [r][c], _t.Get (r, c));
   done (true);   lower ();   hide ();
}
 

void DlgCtl::Quit ()  {Gui.DlgSave (this, "DlgCtl");}
