// dlgCfg.cpp - load/save/edit global Cfg biz
//              and per song cfg via CfgInit/CfgLoad/CfgSave

#include "pcheetah.h"

CfgDef Cfg;

void CfgDef::Init ()                   // default the global settings
{  tran   = 0;                         // tran,ezHop come from .song
   ezHop  = false;                     // trc from App
   cmdKey = MKey (CC("8c"));  
   ntCo   = 0;                         // scale
   barCl  = false;
   updt   = true;
}

void CfgDef::Load ()                   // load global settings
{ StrArr t (CC("cfg"), 80, 80*sizeof(TStr));
  TStr   fn, s, v;
  char  *p;
  ubyt2  i;
TRC("CfgDef::Load");
   Init ();
   App.Path (fn, 'c');   StrAp (fn, CC("/cfg.txt"));   t.Load (fn);
   for (i = 0;  i < t.num;  i++) {
      StrCp (s, t.str [i]);   if (! (p = StrCh (s, '=')))  continue;
      *p = '\0';   StrCp (v, p+1);
      if (StrSt (s, CC("cmdKey")))  Cfg.cmdKey = (ubyte)Str2Int (v);
      if (StrSt (s, CC("ntCo"  )))  Cfg.ntCo   = (ubyte)Str2Int (v);
      if (StrSt (s, CC("barCl" )))  Cfg.barCl  = (*v == 'y') ? true:false;
      if (StrSt (s, CC("updt"  )))  Cfg.updt   = (*v == 'y') ? true:false;
   }
}

void CfgDef::Save ()                   // save global settings
{ BStr buf;
  TStr fn;
  File f;
TRC("CfgDef::Save");
   StrFmt (buf, "cmdKey=`d\n"  "ntCo=`d\n"  "barCl=`s\n"   "updt=`s\n",
                 cmdKey,        ntCo,        barCl?"y":"n", updt?"y":"n");
   App.Path (fn, 'c');   StrAp (fn, CC("/cfg.txt"));
   f.Save (fn, buf, StrLn (buf));
}


//______________________________________________________________________________
/*
void DlgCfg::ReDo ()
{ TStr c, v;
   if      ((ctrl == IDC_REDO) && (evnt == BN_CLICKED))
      {App.SetTrc (false);   Cfg.Init ();   Init ();}
   else if ((ctrl == IDC_QUAN) && (evnt == BN_CLICKED)) 
      emit sgCmd ("quan x");
   else if ((ctrl == IDC_TRAN) && (evnt == EN_CHANGE)) 
      emit sgCmd (StrFmt (c, "tran `s", _tran.Get (v)));
   else if ((ctrl == IDC_EZHOP) && (evnt == EN_CHANGE)) 
      emit sgCmd (StrFmt (c, "ezHop `c", _ezHop.Get () ? 'y' : 'n')));
}
*/

void DlgCfg::Open ()
{ TStr s;
TRC("DlgCfg::Open");
  CtlSpin tr (ui->tran, -12, 12);
  CtlChek e  (ui->ezHop);
  CtlLine k  (ui->cmdKey);
  CtlList c  (ui->ntCo, CC("scale\0" "velocity\0" "track\0"));
  CtlChek b  (ui->barCl), t (ui->trc), u (ui->upd);
   tr.Set (Cfg.tran);
   e.Set  (Cfg.ezHop);
   k.Set  (MKey2Str (s, Cfg.cmdKey));
   c.Set  (Cfg.ntCo);
   b.Set  (Cfg.barCl);   t.Set (App.trc);   u.Set (Cfg.updt);
   show ();   raise ();   activateWindow ();   
}

void DlgCfg::Shut ()                   // set em n save em
{ TStr s; 
TRC("DlgCfg.Shut");
  CtlSpin tr (ui->tran);
  CtlChek e  (ui->ezHop), b (ui->barCl), u (ui->upd), t (ui->trc);
  CtlLine k  (ui->cmdKey);
  CtlList c  (ui->ntCo);
   Cfg.tran  = tr.Get ();
   Cfg.ezHop =  e.Get ();
   StrCp (s, k.Get ());   Cfg.cmdKey = MKey (s);
   if (! Cfg.cmdKey)      Cfg.cmdKey = MKey (CC("8c"));
   Cfg.ntCo  = c.Get ();
   Cfg.barCl = b.Get ();   App.TrcPut (t.Get ());   Cfg.updt = u.Get ();   
   Cfg.Save ();                        // in case we die early :/
   done (true);   lower ();   hide ();
}

void DlgCfg::Init ()  {Cfg.Load ();   Gui.DlgLoad (this, "DlgCfg");}
void DlgCfg::Quit ()  {Cfg.Save ();   Gui.DlgSave (this, "DlgCfg");}
