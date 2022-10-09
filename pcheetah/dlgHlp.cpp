// dlgHlp.cpp - help meee

#include "pcheetah.h"

void DlgHlp::Open ()
{  show ();   raise ();   /* don't steal focus activateWindow (); */  }

void DlgHlp::Shut ()
{  Gui.DlgSave (this, "DlgHlp");
   done (true);   lower ();   hide ();
}

void DlgHlp::Init ()
{ char *ro [10];
   ro [4] = nullptr;
   setAttribute (Qt::WA_ShowWithoutActivating, true);
   setWindowFlags (windowFlags () |
      Qt::Tool |
      Qt::WindowStaysOnTopHint |
   // Qt::WindowTransparentForInput |
   //   Qt::FramelessWindowHint |
   //   Qt::NoDropShadowWindowHint |
   //   Qt::X11BypassWindowManagerHint |
      Qt::WindowDoesNotAcceptFocus);
   Gui.DlgLoad (this, "DlgHlp");   
   _t.Init (ui->t, "Group\0Note\0Key\0Command Description\0");
   _t.Open ();
   for (ubyte i = 0;  i < NUCmd;  i++) {
      ro [0] = CC(UCmd [i].grp);   ro [1] = CC(UCmd [i].nt);
      ro [2] = CC(UCmd [i].ky);    ro [3] = CC(UCmd [i].desc);
      _t.Put (ro);
   }
   _t.Shut ();
}

void DlgHlp::Quit ()  {}
