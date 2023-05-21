// sCmd.cpp - song thread's main Cmd () for gui thread to use

#include "song.h"

UCmdDef UCmd [] = {
// done by gui/pcheetah
   {"exit",      "",    "esc","quit",       "quit PianoCheetah"},
   {"song<",     "3c",  "z",  "song",       "load prev"},
   {"song>",     "3d",  "x",  "",           "load next"},
   {"songRand",  "3c#", "a",  "",           "load random"},
   {"songKill",  "",    "!",  "",           "DELETE (be CAREful)"},
   {"songRate",  "",    "r",  "",           "rating: none=>a=>b=>c=>"},
// done by thread/song
   {"timePause", "3d#", "spc","time",       "play/pause"},
   {"timeBar1",  "3e",  "1",  "",           "hop to 1st bar"},
   {"timeBar<",  "3f",  "2",  "",           "prev bar"},
   {"timeBar>",  "3f#", "3",  "",           "next bar"},
   {"timeHop<",  "3g",  "lft","",           "prev page/loop/8th bar"},
   {"timeHop>",  "3g#", "rit","",           "next page/loop/8th bar"},
   {"tempoHop",  "3a",  "t",  "tempo",      "tempo: 60%=>80%=>100%=>"},
   {"tempo<",    "3a#", "f02","",           "down"},
   {"tempo>",    "3b",  "f03","",           "up"},
   {"tran<",     "4c",  "f11","transpose",  "down"},
   {"tran>",     "4c#", "f12","",           "up"},
   {"recSave",   "4d",  "s",  "recording",  "save"},
   {"recWipe",   "4d#", "w",  "",           "wipe ALL (CAREFUL)"},
   {"learn",     "4e",  "l",  "song modes", "learn: hear=>play=>practice=>"},
   {"hand",      "4f",  "h",  "",           "hand: practice LH=>RH=>HT=>"},
   {"color",     "4f#", "c",  "",           "color: scale=>velocity=>track=>"},
   {"hearLoop",  "4g",  "/",  "loop",       "Hear loop then back to Prac"},
   {"focus",     "4g#", "f",  "",          "focus loop around bugs / restore"},
   {"edTrk<",    "4a",  "up", "pick track", "prev"},
   {"edTrk>",    "4a#", "dn", "",           "next"},
   {"mute",      "5c",  "m",  "track mode", "toggle mute on edit track"},
   {"prac",      "5d",  "p",  "",           "toggle practice on edit track"},
   {"snd<",      "5e",  "[",  "pick sound", "prev sound on edit track"},
   {"snd>",      "5f",  "]",  "",           "next sound on edit track"}
};
ubyte NUCmd = BITS (UCmd);


void Song::Cmd (QString s)
{ TStr  c;
  ubyte i;
   StrCp (c, UnQS (s));
DBG("Cmd='`s'", c);
   for (i = 0;  i < NUCmd;  i++)  if (! StrCm (c, CC(UCmd [i].cmd)))  break;
   if (i < 6)     {emit sgUpd (s);   return;}
   if (i < NUCmd)  switch (i) {
      case  6:  EdTime (0);  break;       // timePause
      case  7:  EdTime (1);  break;       // timeBar1
      case  8:  EdTime (2);  break;       // timeBar<
      case  9:  EdTime (3);  break;       // timeBar>
      case 10:  EdTime (4);  break;       // timeHop<
      case 11:  EdTime (5);  break;       // timeHop>

      case 12:  EdTmpo (0);  break;       // tempoHop
      case 13:  EdTmpo (1);  break;       // tempo<
      case 14:  EdTmpo (2);  break;       // tempo>

      case 15:  EdTmpo (3);  break;       // tran<
      case 16:  EdTmpo (4);  break;       // tran>

      case 17:  EdRec  (0);  break;       // recSave
      case 18:  EdRec  (1);  break;       // recWipe

      case 19:  EdLrn  (0);  break;       // learn
      case 20:  EdLrn  (1);  break;       // hand
      case 21:  EdLrn  (2);  break;       // color
      case 22:  EdLrn  (3);  break;       // hearLoop
      case 23:  EdLrn  (4);  break;       // focus

      case 24:  EdTrak (0);  break;       // edTrk<
      case 25:  EdTrak (1);  break;       // edTrk>

      case 26:  EdLrn  (5);  break;       // mute
      case 27:  EdLrn  (6);  break;       // prac

      case 28:  NewSnd ((char)0);  break; // snd<
      case 29:  NewSnd (1);  break;       // snd>
   }
   else if (! StrCm (c, CC("init")))        Init ();
   else if (! StrCm (c, CC("quit")))        Quit ();
   else if (! StrCm (c, CC("wipe")))        Wipe ();
   else if (! MemCm (c, CC("load "),   5))  Load (& c [5]);
   else if (! StrCm (c, CC("dump")))        Dump (true);
   else if (! MemCm (c, CC("tran "),   5))
      {NotesOff ();   _f.tran  = (sbyte) Str2Int (& c [5]);       DscSave ();}
   else if (! MemCm (c, CC("ezHop "),  6))
      {NotesOff ();   _f.ezHop = (c [6] == 'y') ? true : false;   DscSave ();}
   else if (! MemCm (c, CC("quan x"),  6))  {SetDn ('q');   ReDo ();}
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
   else if (! StrCm (c, CC("cue")))        Cue    ();
   else if (! MemCm (c, CC("chd "),   4))  Chd    (& c [4]);
   else if (! MemCm (c, CC("fng "),   4))  Fng    (& c [4]);
   else if (! MemCm (c, CC("setCtl "),7))  SetCtl (& c [7]);
   else if (! StrCm (c, CC("mov")))        Mov    ();
   Put ();
}


//______________________________________________________________________________
ubyte Song::ChkETrk ()                 // be sure _eTrk is still ok
{  if (Up.eTrk >= Up.rTrk) Up.eTrk = (ubyte)(Up.rTrk-1);   return Up.eTrk;  }


void Song::EdTrak (char ofs)
{ ubyte t = ChkETrk ();
   if (ofs)  {if (++t == Up.rTrk)  t = 0;}
   else      {if (! t--)  t = (ubyte)(Up.rTrk-1);}
   Up.eTrk = t;   ReTrk ();
}


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

   case 1:  RecWipeQ ();               // might as well :)
            Poz (false);               // timeBar1  ooUHN MO TAHM!
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
   }                                   // hop to new bar (minus a teeny bit)
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
      if (ofs == 3) {if (_f.tran > -12) _f.tran--;}
      else          {if (_f.tran <  12) _f.tran++;}
      DscSave ();   StrFmt (ts, "transpose=`d", _f.tran);   Hey (ts);
   }
}


//______________________________________________________________________________
void Song::RecWipeQ ()                 // quicker recWipe eoloop
{ ubyte t;
   TmpoPik ('o');                      // original tempo changes;  kill evs
   for (t = Up.rTrk;  t < _f.trk.Ln;  t++)  EvDel (t, 0, _f.trk [t].ne);
   ReDo ();
}

void Song::EdRec (char ofs)
{  if      (ofs == 0) {Save ();   ReTrk ();}     // recSave
   else if (ofs == 1) {RecWipeQ ();   ReDo ();}  // recWipe
}


//______________________________________________________________________________
void Song::EdLrn (char ofs)            // this has gotten pretty hairy :(
// 0=learn  1=hand  2=color  3=hearLoop  4=focus  5=shh  6=prac
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
   else if (ofs == 1) {                // hand type - ? LH,RH,'';  \0 show,''
      if (! _lrn.hand)
         {Hey (CC("click tracks' 2nd square into RH,LH"));   return;}
      if (_lrn.ez)
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
   else if (ofs == 2) {                // color
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
   else if (ofs == 3) {                // hearLoop
      if (! PRAC) {
         Hey (CC("you need to be in practice mode to hear a loop"));
         return;
      }
      Cmd ("recWipe");   Cmd ("timeBar1");
      Up.lrn = LHLRN;   emit sgUpd ("tbLrn");
      _lrn.pLrn = LPRAC;   _lrn.hLrn = true;     // hear the ? tracks, not rec
      if (_lrn.POZ)  {_lrn.POZ = false;   Poz (false);}
      return;                          // unpoz cuz we might be after timeBar1
   }
   else if (ofs == 4)  SetLp ('f');    // focus
   else if (ofs == 5) {                // flip shh (reset lrn,ht on shh true)
      if ((_f.trk [e].shh = ! _f.trk [e].shh))  {_f.trk [e].lrn = false;
                                                 _f.trk [e].ht  = '\0';}
   }
   else if (ofs == 6)                  // flip lrn
      if ((_f.trk [e].lrn = ! _f.trk [e].lrn))  {_f.trk [e].shh = false;
                                                 _f.trk [e].ht  = '\0';}
   ReDo ();
}


//______________________________________________________________________________
void Song::HType (char *s)             // HT,RH,LH,ez1-ez7
{ ubyte t, e = ChkETrk ();
  char  c;
DBG("HType `s r=`d", s, e);
   c = *s;
   if (c == 'e')  c = s [2];
   if (c == 'x')  c = (_f.trk [e].ht == 'S') ? '\0' : 'S';
   _f.trk [e].ht = c;
   if (TEz (e)) {                      // set one track ez, they all go
      for (e = 0;  e < Up.rTrk;  e++)
                                  if (TLrn (e) && (! TDrm (e)) && (! TEz (e))) {
         for (c = '1';  c <= '7';  c++) {
            for (t = 0;  t < Up.rTrk;  t++)  if (_f.trk [t].ht == c) break;
            if (t >= Up.rTrk)  break;
         }
         if (c == '8')  c = '7';
         _f.trk [e].ht = c;
      }
   }
   else if (TLrn (e))                  // set one nonEZ, all other ez go HT
      for (e = 0;  e < Up.rTrk;  e++)  if (TLrn (e) && (! TDrm (e)) && TEz (e))
                                          _f.trk [e].ht = '\0';
   ReDo ();
Dump ();
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
