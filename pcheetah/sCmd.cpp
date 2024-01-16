// sCmd.cpp - song thread's main Cmd () for gui thread to use

#include "song.h"

UCmdDef UCmd [] = {
// done by gui/pcheetah
   {"exit",      "",    "esc","song",       "quit PianoCheetah"},
   {"song<",     "3c",  "z",  "",           "load prev"},
   {"song>",     "3d",  "x",  "",           "load next"},
   {"songRand",  "3c#", "a",  "",           "load random"},
   {"songKill",  "",    "!",  "",           "DELETE SONG (be CAREful)"},
   {"songRate",  "",    "r",  "",           "rating: none=>a=>b=>c=>"},
// done by thread/song
   {"timePause", "3d#", "spc","time",       "play/pause"},
   {"timeBar1",  "3e",  "1",  "",           "hop to 1st bar"},
   {"timeBar<",  "3f",  "2",  "",           "prev bar"},
   {"timeBar>",  "3f#", "3",  "",           "next bar"},
   {"timeHop<",  "3g",  "lft","",           "prev page/loop/8th bar"},
   {"timeHop>",  "3g#", "rit","",           "next page/loop/8th bar"},
   {"timeBug",   "",    "b",  "",           "hop to loop with most bugs"},
   {"tempoHop",  "3a",  "t",  "tempo",      "tempo: 60%=>80%=>100%=>"},
   {"tempo<",    "3a#", "f02","",           "down"},
   {"tempo>",    "3b",  "f03","",           "up"},
   {"tran<",     "4c",  "f11","transpose",  "down"},
   {"tran>",     "4c#", "f12","",           "up"},
   {"recSave",   "4d",  "s",  "recording",  "save (broke - sorry)"},
   {"recWipe",   "4d#", "w",  "",           "wipe ALL (CAREFUL)"},
   {"learn",     "4e",  "l",  "learn mode", "learn: hear=>play=>practice=>"},
   {"ez",        "4f",  "e",  "ez mode",    "(toggle)"},
   {"hand",      "4f#", "h",  "",           "hand: practice LH=>RH=>HT=>"},
   {"color",     "4g",  "c",  "",           "color: scale=>velocity=>track=>"},
   {"hearLoop",  "4g#", "/",  "loop",       "Hear loop notes to learn"},
   {"hearRec",   "4a",  ".",  "",           "Hear your recording"}
};
ubyte NUCmd = BITS (UCmd);


void Song::Cmd (QString s)
{ TStr  c;
  ubyte i;
   StrCp (c, UnQS (s));
DBGTH("PcSng");                        // first time Song is hit :/
DBG("Cmd='`s'", c);
   for (i = 0;  i < NUCmd;  i++)  if (! StrCm (c, CC(UCmd [i].cmd)))  break;
   if (i < 6)     {emit sgUpd (s);   return;}
   if (i < NUCmd)  switch (i) {
      case  6:  EdTime (0);  break;         // timePause
      case  7:  EdTime (1);  break;         // timeBar1
      case  8:  EdTime (2);  break;         // timeBar<
      case  9:  EdTime (3);  break;         // timeBar>
      case 10:  EdTime (4);  break;         // timeHop<
      case 11:  EdTime (5);  break;         // timeHop>
      case 12:  EdTime (6);  break;         // timeBug

      case 13:  EdTmpo (0);  break;         // tempoHop
      case 14:  EdTmpo (1);  break;         // tempo<
      case 15:  EdTmpo (2);  break;         // tempo>

      case 16:  EdTmpo (3);  break;         // tran<
      case 17:  EdTmpo (4);  break;         // tran>

      case 18:  EdRec  (0);  break;         // recSave
      case 19:  EdRec  (1);  break;         // recWipe

      case 20:  EdLrn  (0);  break;         // learn
      case 21:  EdLrn  (1);  break;         // ez
      case 22:  EdLrn  (2);  break;         // hand
      case 23:  EdLrn  (3);  break;         // color
      case 24:  EdLrn  (4);  break;         // hearLoop
      case 25:  EdLrn  (5);  break;         // hearRec
   }
   else if (! StrCm (c, CC("init")))      Init ();
   else if (! StrCm (c, CC("quit")))      Quit ();
   else if (! StrCm (c, CC("wipe")))      Wipe ();
   else if (! MemCm (c, CC("load "), 5))  Load (& c [5]);
   else if (! StrCm (c, CC("mute")))      EdLrn (6);
   else if (! StrCm (c, CC("prac")))      EdLrn (7);
   else if (! StrCm (c, CC("dump")))      Dump (true);
   else if (! StrCm (c, CC("quan")))      {SetDn ('q');   ReDo ();}
   else if (! MemCm (c, CC("tran "), 5))
      {NotesOff ();   _f.tran  = (sbyte) Str2Int (& c [5]);       DscSave ();}
   else if (! StrCm (c, CC("showAll"))) {
     bool  dr, all = true;             // drums or melo, all shown now?
     ubyte t, nt = Up.rTrk;
      dr = TDrm (Up.eTrk);
      for (t = 0;  t < nt;  t++)  if (! _f.trk [t].lrn) {
         if (dr)  {if (  TDrm (t))  if (_f.trk [t].ht != 'S')
                                       {all = false;   break;}}
         else      if (! TDrm (t))  if (_f.trk [t].ht != 'S')
                                       {all = false;   break;}
      }
      for (t = 0;  t < nt;  t++)  if (! _f.trk [t].lrn) {
         if (dr)  {if (  TDrm (t))  _f.trk [t].ht = all ? '\0' : 'S';}
         else      if (! TDrm (t))  _f.trk [t].ht = all ? '\0' : 'S';
      }
      ReDo ();
   }
   else if (! MemCm (c, CC("htype "), 6))  HType  (& c [6]);
   else if (! MemCm (c, CC("trk "),   4))
                            {StrCp (_f.trk [Up.eTrk].name, & c [4]);   ReDo ();}
   else if (! MemCm (c, CC("grp "),   4))  NewGrp (& c [4]);
   else if (! MemCm (c, CC("snd "),   4))  NewSnd (& c [4]);
   else if (! MemCm (c, CC("dev "),   4))  NewDev (& c [4]);
   else if (! MemCm (c, CC("mix "),   4))  Mix    (& c [4]);
   else if (! MemCm (c, CC("drmap "), 6))  DrMap  (& c [6]);
   else if (! MemCm (c, CC("trkEd "), 6))  TrkEd  (& c [6]);
   else if (! MemCm (c, CC("preTDr"), 6))  PreTDr ();
   else if (! MemCm (c, CC("tDr "),   4))     TDr (& c [4]);
   else if (! MemCm (c, CC("ctl"),    3))  Ctl    ();
   else if (! MemCm (c, CC("cue "),   4))  Cue    (& c [4]);
   else if (! MemCm (c, CC("chd "),   4))  Chd    (& c [4]);
   else if (! MemCm (c, CC("fng "),   4))  Fng    (& c [4]);
   else if (! MemCm (c, CC("setCtl "),7))  SetCtl (& c [7]);
   else if (! StrCm (c, CC("mov")))        Mov    ();
   Put ();
}


//______________________________________________________________________________
ubyte Song::ChkETrk ()                 // be sure _eTrk is still ok
{  if (Up.eTrk >= Up.rTrk) Up.eTrk = (ubyte)(Up.rTrk-1);   return Up.eTrk;  }


//______________________________________________________________________________
void Song::EdTime (char ofs)           // edit song time
{ ubyt2 bar, bt;
  TStr  str;
  char *s;
  bool  tofs = true;
// init str, bar n bt to "now"
   TmStr (str, _timer->Get () + (M_WHOLE/64));   // round time to next bar
   bar = (ubyt2)Str2Int (str, & s);   bt = (ubyt2)Str2Int (s+1);

   switch (ofs) {
   case 0:  NotesOff ();               // toggle timer's pause state
            Up.uPoz = Up.uPoz ? false : true;   Poz (Up.uPoz);
            emit sgUpd ("tbPoz");
            return;

   case 1:  Poz (false);               // timeBar1  ooUHN MO TAHM!
            TmHop ((PRAC || (_lrn.pLrn==LPRAC)) ? _lrn.lpBgn : 0);
            if (Up.uPoz)  Poz (true);   else Put ();       // restart schedulin
            return;

   case 2:  if ((bar > 1) && (bt <= 2))  bar--;     break; // else restart bar
   case 3:                               bar++;     break;

   case 4:  if (PRAC || _lrn.pLrn)  {SetLp ('<');   return;}    // timeHop<
            if (_pg) {
               bt =        _pag [_pg-1].col [0].blk [0].bar;
               if ((_pg == 1) || (bar > bt))  bar = bt;
               else  bar = _pag [_pg-2].col [0].blk [0].bar;
               tofs = false;
               break;
            }
            if (bar <= 8) bar = 1;  else bar -= 8;   break;
   case 5:  if (PRAC || _lrn.pLrn)  {SetLp ('>');   return;}    // timeHop>
            if (_pg) {
               bar = (_pg >= _pag.Ln) ? _bEnd :
                     _pag [_pg].col [0].blk [0].bar;
               tofs = false;
               break;
            }
                                         bar += 8;  break;
   case 6:  if (! PRAC) {              // timeBug
              ubyte t;                 // if no lrn, show message
               for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))  break;
               if (t >= Up.rTrk)  {Cmd ("learn");   return;}

               Up.lrn = LPRAC;
            }
            SetLp ('b');   Cmd ("recWipe");   Cmd ("timeBar1");
            return;
   }
// hop to new bar (minus a teeny bit)
  ubyt4 t = Bar2Tm (bar);
   if (tofs)  {if (t >= (M_WHOLE/64))  t -= (M_WHOLE/64);   else t = 0;}
   TmHop (t);   if (! Up.uPoz)  Put ();
}


//______________________________________________________________________________
void Song::EdTmpo (char ofs)           // tempo
// bump curr tempo and ins/upd event in bar#1 of a drum trak
{ ubyt4 tp;
  ubyt2 tt;
  TStr  ts;
   if      (ofs <= 2) {
      tt = TmpoAt (_timer->Get ());    // straight track tempo (tmpoSto)
      tp = _timer->Tempo ();           // actual current tempo
      switch (ofs) {
      case 0:                          // 60=>80=>100%
         if      (_f.tmpo >= 9*FIX1/10)     // 100=>60
            tp = (60*tt / 100) + (((60*tt % 100) >= 50)?1:0);
         else if (_f.tmpo <= 7*FIX1/10)     // 60=>80
            tp = (80*tt / 100) + (((80*tt % 100) >= 50)?1:0);
         else                               // 80=>100
            tp = tt;
         break;
      case 1:  if (tp >= 2)    tp--;   break;    // bump to wanted actual
      case 2:  if (tp <= 959)  tp++;   break;
      }                                          // calc frac to make sto=>act
      _f.tmpo = (FIX1*tp / tt) + ((FIX1*tp % tt >= (tt/(ubyt4)2)) ? 1 : 0);
DBG("EdTmpo `d  _f.tmpo=`d/`d tmpoSto=`d tmpoAct=`d",
ofs, _f.tmpo, FIX1, tt, tp);
      DscSave ();   PutTp (tt);   ReTrk ();
   }
   else if (ofs <= 4) {                // transpose, actually :/
      NotesOff ();                     // SHUSH !
      if (ofs == 3) {if (_f.tran > (sbyte)-12) _f.tran--;}
      else          {if (_f.tran < (sbyte) 12) _f.tran++;}
      DscSave ();   StrFmt (ts, "transpose=`d", _f.tran);   Hey (ts);
   }
}


//______________________________________________________________________________
void Song::EdRec (char ofs)
{  if (ofs == 0)  {Save ();   ReTrk ();   return;}    // recSave
   for (ubyte t = Up.rTrk;  t < _f.trk.Ln;  t++)  EvDel (t, 0, _f.trk [t].ne);
   ReDo ();                                           // recWipe
}


//______________________________________________________________________________
void Song::EdLrn (char ofs)            // this has gotten pretty hairy :(
// 0=learn  1=hand  2=color  3=hearLoop  4=hearRec  5=shh  6=prac
{ ubyte e = ChkETrk (), t;
  char  c;
   if      (ofs == 0) {                // learn - toggle _f.lrn
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))  break;
      if (t >= Up.rTrk) {
         Up.lrn = LHEAR;
         Hey (CC("First, pick track(s) to practice - "
                 "click left track icon into a green arrow (practice track)"));
      }
      else {
         if      (PLAY)  {Up.lrn = LPRAC;   SetLp ('i');}
         else if (PRAC)   Up.lrn = LHEAR;
         else             Up.lrn = LPLAY;
      }
      ReDo ();
      if (PRAC)  {Cmd ("recWipe");   Cmd ("timeBar1");}
      return;
   }
   else if (ofs == 1)  Up.ez = ! Up.ez;     // ez - toggle
   else if (ofs == 2) {                // hand type - ? LH,RH,'';  \0 show,''
      if (! _lrn.hand)
         {Hey (CC("click tracks' 2nd square into RH,LH"));   return;}
      if (Up.ez)
         {Hey (CC("click tracks' 2nd square into RH,LH"));   return;}
      if      (_lrn.hand == 'B')  c = 'r';       // bump to next b=>r=>l=>b...
      else if (_lrn.hand == 'R')  c = 'l';
      else                        c = 'b';
   // set any non rec *H tracks' .lrn to ? or \0
      for (t = 0;  t < Up.rTrk;  t++) {
         if (_f.trk [t].ht == 'L')  _f.trk [t].lrn = ((c == 'l') || (c == 'b'));
         if (_f.trk [t].ht == 'R')  _f.trk [t].lrn = ((c == 'r') || (c == 'b'));
      }
   }
   else if (ofs == 3) {                // color
      if (++Cfg.ntCo == 3)  Cfg.ntCo = 0;   Cfg.Save ();
     TStr s;
      StrCp (s, CC("color of "));
      switch (Cfg.ntCo) {
         case 0:  StrAp (s, CC("scale"));      break;
         case 1:  StrAp (s, CC("velocity"));   break;
         default: StrAp (s, CC("track"));
      }
      Hey (s);
   }
   else if (ofs == 4) {                // hearLoop (lrn tracks, not rec, no bg)
//    if (! PRAC)
//       {Hey (CC("you need to be in practice mode to hear a loop"));   return;}
      t = Up.lrn;   Cmd ("recWipe");   Cmd ("timeBar1");
      Up.lrn = LHEAR;   _lrn.pLrn = t;   emit sgUpd ("tbLrn");
      if (_lrn.POZ)  {_lrn.POZ = false;   Poz (false);}
      return;                          // unpoz cuz we might be after timeBar1
   }
   else if (ofs == 5) {                // hearRec (rec tracks and bg)
      t = Up.lrn;   Cmd ("timeBar1");   TmpoPik ('r');
      Up.lrn = LHREC;   _lrn.pLrn = t;   emit sgUpd ("tbLrn");
      if (_lrn.POZ)  {_lrn.POZ = false;   Poz (false);}
      return;
   }
   else if (ofs == 6) {                // flip shh (reset lrn,ht on shh true)
      if ((_f.trk [e].shh = ! _f.trk [e].shh))  {_f.trk [e].lrn = false;
                                                 _f.trk [e].ht  = '\0';}
   }
   else if (ofs == 7)                  // flip lrn
      if ((_f.trk [e].lrn = ! _f.trk [e].lrn))  {_f.trk [e].shh = false;
                                                 _f.trk [e].ht  = '\0';}
   ReDo ();
}


//______________________________________________________________________________
void Song::HType (char *s)             // \0=HT,RH,LH,x=flip S
{ ubyte e = ChkETrk ();
  char  c;
  TStr  tn;
DBG("HType `s t=`d", s, e);
   c = *s;
   StrCp (tn, _f.trk [e].name);
   if     ((c == 'L') || (c == 'R')) {
      if      (*tn == '\0')       StrCp (tn, CC("LH"));
      else if (! ((tn [1] == 'H') && ((tn [2] == '\0') ||
                                      (tn [2] == ' '))))
         {StrCp (& tn [3], tn);   MemCp (tn, CC("LH "), 3);}
      *tn = c;
      StrCp (_f.trk [e].name, tn);
   }
   else if (c != 'x') {                // '\0'=HT
      if      (! StrCm (& tn [1], CC("H")))            *tn = '\0';
      else if (! MemCm (& tn [1], CC("H "), 2))  StrCp (tn, & tn [3]);
      StrCp (_f.trk [e].name, tn);
   }
   else                                // flip show
      c = (_f.trk [e].ht == 'S') ? '\0' : 'S';
   _f.trk [e].ht = c;
   ReDo ();
}


void Song::Mix (char *s)
{ char *s2;
  ubyte v, p;
  v = (ubyte)Str2Int (s, & s2);
  p = (ubyte)Str2Int (s2);
TRC("mix v=`d p=`d", v, p);
  ubyte e = ChkETrk ();   CCInit (e, CC("Vol"), v);
                          CCInit (e, CC("Pan"), p);   ReTrk ();
}
