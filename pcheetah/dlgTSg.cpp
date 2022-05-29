// dlgTSg.cpp - edit timesig

#include "pcheetah.h"

void DlgTSg::Open ()
{  show ();   raise ();   activateWindow ();
   StrCp (_s, Up.pos.etc);
DBG("HEY in=`s", Up.pos.str);
DBG("etc=`s", _s);
  TStr  n;
  char *d, *s;
  ubyte i;
  BStr  ld, ls;
   *ld = *ls = '\0';
   for (i = 0;  i < 16;  i++)  ZZAp (ld, StrFmt (n, "`d", 1 << i));
   for (i = 0;  i <  9;  i++)  ZZAp (ls, StrFmt (n, "`d", i +  1));
   StrCp (n, Up.pos.str);
   d = StrCh (n, '/');   *d++ = '\0';
   s = StrCh (d, '/');   if (s)  *s++ = '\0';  else s = CC("1");
  CtlSpin num (ui->num, 1, 255);
  CtlList den (ui->den, ld),
          sub (ui->sub, ls);
   num.Set (Str2Int (n));   den.SetS (d);   sub.SetS (s);
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgTSg::Shut ()
{ TStr s;
  CtlSpin num (ui->num);
  CtlList den (ui->den), sub (ui->sub);
   StrFmt (_s, "`d/`d", num.Get (), den.Get ());
   if (sub.Get ())  StrAp (_s, StrFmt (s, "/`d", sub.Get ()));
   emit sgCmd (_s);
   done (true);   lower ();   hide ();
}

void DlgTSg::Init ()  {Gui.DlgLoad (this, "DlgTSg");}
void DlgTSg::Quit ()  {Gui.DlgSave (this, "DlgTSg");}
