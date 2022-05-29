// dlgTDr.cpp - pick a new drum track per section (patA,patB,fill)

#include "pcheetah.h"

static BStr ALst, BLst, FLst;

static void TDrPop (char *ls, ubyt2 r, ubyte c)    
{  (void)r;   ZZCp (ls, (c==1) ? ALst : ((c==2) ? BLst : FLst));  }


void DlgTDr::Cmd ()
{ ubyt2 r = _t.CurRow ();
  ubyte c = _t.CurCol ();
  TStr  s;
DBG("DlgTDr::Upd r=`d c=`d s=`s", r, c, _t.Get (r, c));
   emit sgCmd (StrFmt (s, "tDr `d `d `s", r, c, _t.Get (r, c)));
}


//______________________________________________________________________________
void DlgTDr::Init ()  
{ TStr s, fn;
  BStr b;
  StrArr sa;
   Gui.DlgLoad (this, "DlgTDr");
   StrFmt (fn, "`s/clip/drum/main", App.Path (s, 'd'));
   sa.GetDir (fn, 'f', 1024, 'x');   
   sa.SetZZ (b);
   *ALst = *BLst = *FLst = '\0';
   ZZAp (ALst, CC("(off)\0(continue)\0"));   ZZAp (ALst, b);
   ZZAp (BLst, CC("(off)\0"));               ZZAp (BLst, b);
   StrAp  (fn,           CC("fill"), 4);
   sa.GetDir (fn, 'f', 1024, 'x');   sa.SetZZ (b);
   ZZAp (FLst, CC("(off)\0"));               ZZAp (FLst, b);
   _t.Init (ui->t, "Section\0^PatA\0^PatB\0^Fill\0", TDrPop);
   connect (ui->t, & QTableWidget::itemChanged, this, & DlgTDr::Cmd);
}


void DlgTDr::Open ()  
{ ubyte r, c;
  char *ro [5];
   if (Up.nR == 0) {
      Gui.Hey ("...oops!  Add some Cues!\n" "(for verse/chorus/etc)");
      return;
   }
   show ();   raise ();   activateWindow ();    
   ro [4] = nullptr;
   _t.Open ();
   for (r = 0;  r < Up.nR;  r++) {
      for (c = 0;  c < 4;  c++)  ro [c] = Up.d [r][c];
      _t.Put (ro);
   }
   _t.Shut ();
}


void DlgTDr::Shut ()  {done (true);   lower ();   hide ();}
void DlgTDr::Quit ()  {Gui.DlgSave (this, "DlgTDr");}
