// dlg.cpp - all pcheetah's dang dialogs - a LOT

#include "pcheetah.h"

//______________________________________________________________________________
// DlgCfg - load/save/edit global Cfg biz
//          and per song cfg via CfgInit/CfgLoad/CfgSave

CfgDef Cfg;

void CfgDef::Init ()                   // default the global settings
{  tran   = 0;                         // tran comes from .song
   cmdKey = MKey (CC("8c"));           // trc from App
   ntCo   = 0;                         // scale
   barCl  = false;
   sVol   = 7;
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
      if (StrSt (s, CC("sVol"  )))  Cfg.sVol   = (ubyte)Str2Int (v);
   }
}

void CfgDef::Save ()                   // save global settings
{ BStr buf;
  TStr fn;
  File f;
TRC("CfgDef::Save");
   StrFmt (buf, "cmdKey=`d\n"  "ntCo=`d\n"  "barCl=`s\n"   "sVol=`d\n",
                 cmdKey,        ntCo,        barCl?"y":"n", sVol);
   App.Path (fn, 'c');   StrAp (fn, CC("/cfg.txt"));
   f.Save (fn, buf, StrLn (buf));
}


//______________________________________________________________________________
void DlgCfg::Open ()
{ TStr ts;
TRC("DlgCfg::Open");
  CtlSpin tr (ui->tran, -12, 12);
  CtlLine k  (ui->cmdKey);
  CtlSldr s  (ui->sVol, 0, 100);
  CtlChek b  (ui->barCl), t (ui->trc);
   tr.Set (Cfg.tran);
   k.Set  (MKey2Str (ts, Cfg.cmdKey));
   s.Set  (Cfg.sVol);
   b.Set  (Cfg.barCl);   t.Set (App.trc);
   show ();   raise ();   activateWindow ();
}

void DlgCfg::Shut ()                   // set em n save em
{ TStr ts;
TRC("DlgCfg.Shut");
  CtlSpin tr (ui->tran);
  CtlLine k  (ui->cmdKey);
  CtlSldr s  (ui->sVol);
  CtlChek b  (ui->barCl), t (ui->trc);
   Cfg.tran  = tr.Get ();
   StrCp (ts, k.Get ());   Cfg.cmdKey = MKey (ts);
   if (! Cfg.cmdKey)       Cfg.cmdKey = MKey (CC("8c"));
   Cfg.sVol  = s.Get ();
   Cfg.barCl = b.Get ();   App.TrcPut (t.Get ());
   Cfg.Save ();                        // in case we die early :/
   Sy._vol = (real)Cfg.sVol / 100.0;
   done (true);   lower ();   hide ();
}

void DlgCfg::Init ()  {Cfg.Load ();   Gui.DlgLoad (this, "DlgCfg");
   connect (ui->quan, & QPushButton::clicked, this, [this]()
                                                   {emit sgCmd (CC("quan"));});}
void DlgCfg::Quit ()  {Cfg.Save ();   Gui.DlgSave (this, "DlgCfg");}


//______________________________________________________________________________
// dlgChd - edit chords

static char *IntvMap = CC("1m2m34d5a6m7");


void DlgChd::Cmd (char *s)
{ TStr u;
   emit sgCmd (StrFmt (u, "chd `s", s));
}


void DlgChd::ReDo ()
{ TStr s, s2, chd;
TRC("DlgChd::ReDo");
// pull our chord picks to make chd string
  CtlList root (ui->root), type (ui->type), bass (ui->bass);
   root.GetS (chd);   StrCp (s, CC(MChd [type.Get ()].lbl));   bass.GetS (s2);
   if (! StrCm (chd, CC("(none)")))  *chd = '\0';     // no chord at all?
   else {
      StrAp (chd, s);
      if (StrCm (s2, CC("(none)")))  {StrAp (chd, CC("/"));   StrAp (chd, s2);}
   }
// enable/dis per picks
                 type.Enable (true);    bass.Enable (true);
   if (! *chd)  {type.Enable (false);   bass.Enable (false);
                 type.Set (0);          bass.Set (0);}
// update dat song's _f.chd (ins/del/upd) based on _got,_cp
   Cmd (StrFmt (s, ".`b `d `d `s", _got, _cp, _tm, chd));
   _got = *chd ? true : false;
TRC("DlgChd::ReDo end");
}


void DlgChd::Set (char *s)
{ ubyte i, j;
  TStr  ch;
  char *c;
   StrCp (ch, s);
  CtlList root (ui->root), type (ui->type), bass (ui->bass);
   root.Set (0);   type.Set (0);   bass.Set (0);
   if ((c = StrCh (ch, '/'))) {        // if got bass, chop if off n set ctrl
      *c++ = '\0';
      for (i = 0;  i < 12;  i++)  {if (! StrCm (c, MKeyStr  [i]))  break;
                                   if (! StrCm (c, MKeyStrB [i]))  break;}
      if (i < 12)  bass.Set (1+i);
   }
   if (*ch) {
      i = ((ch [1] == 'b') || (ch [1] == '#')) ? 2 : 1;    // type pos
      c = & ch [i];
      for (j = 0;  j < NMChd;  j++)  if (! StrCm (c, CC(MChd [j].lbl), 'x'))
                                        {type.Set (j);   break;}
      *c = '\0';                       // chop it off
   }
   for (i = 0;  i < 12;  i++)  {if (! StrCm (ch, MKeyStr  [i]))  break;
                                if (! StrCm (ch, MKeyStrB [i]))  break;}
   if (i < 12) root.Set (i+1);
}


void DlgChd::Pop ()
{ TStr s;
  CtlList pop (ui->pop);
   pop.GetS (s);   while (*s == ' ')  StrCp (s, & s [1]);   Set (s);
   ReDo ();   Cmd (StrFmt (s, "@`d", _tm1));
}


void DlgChd::UnDo ()
{ TStr s;
  CtlBttn undo (ui->undo);
   undo.Get (s);   StrCp (s, & s [8]);   Set (s);
   ReDo ();   Cmd (StrFmt (s, "@`d", _tm1));
}


//______________________________________________________________________________
void DlgChd::Init ()
{  Gui.DlgLoad (this, "DlgChd");
  CtlTBar tb (this,
      "guess chords\n"
      "   calculate whole song's chords\n"
      "   using notes of selected practice tracks"
         "`:/tbar/chd/0" "`\0"
      "pop chords\n"
      "   plop some random happy pop chords into this section"
         "`:/tbar/chd/1" "`\0"
      "delete all chords  (be carefulll)"
         "`:/tbar/chd/2" "`\0"
   );
   connect (tb.Act (0), & QAction::triggered, this, [this]() {Cmd (CC("?"));});
   connect (tb.Act (1), & QAction::triggered, this, [this]() {Cmd (CC("+"));});
   connect (tb.Act (2), & QAction::triggered, this, [this]() {Cmd (CC("x"));});
   connect (ui->shhh, & QPushButton::clicked, this, [this]()
                                              {emit sgCmd (CC("timePause"));});
   connect (ui->undo, & QPushButton::clicked, this, [this]() {UnDo ();});
   connect (ui->pop,  QOverload<int>::of(& QComboBox::currentIndexChanged),
                                              this, [this]() {Pop  ();});
}


void DlgChd::Open ()
{ ubyt4 i, j;
  ubyte nbtw;
  TStr  *btw, s, it, t, ch, ch1, ch2;
  char  *c;
TRC("DlgChd::Open");
   show ();   raise ();   activateWindow ();
   _got = Up.pos.got ? true : false;
   _cp = Up.pos.cp;   _tm = Up.pos.tm;   _tm1 = Up.pos.tmBt;
   StrCp (ch1, Up.d [0][0]);   StrCp (ch2, Up.d [0][1]);
   nbtw = ChdBtw (& btw, ch1, ch2);
  CtlList pop  (ui->pop);
  CtlBttn undo (ui->undo);
   pop.ClrLs ();
   for (ubyte i = 0;  i < nbtw;  i++) {
     ColSep c (btw [i], 80);
      pop.InsLs (c.Col [0]);
      for (ubyte j = 1;  c.Col [j][0];  j++)
         {StrCp (it, CC(" "));   StrAp (it, c.Col [j]);   pop.InsLs (it);}
   }
   StrCp (ch, Up.pos.str);   StrCp (s, ch);
   if (! nbtw)  {undo.Enable (false);   pop.InsLs (CC("(none)"), 0);}
   else {        undo.Enable (true);
      if (! Up.pos.got)  StrCp (s, CC("(none)"));
      StrCp (it, CC("restore "));   StrAp (it, s);   undo.Set (it);
   }
   pop.Set (0);

// init lists for chord's root/type/bass
   c = Up.pos.kSg.flt ? CC("(none)\0C\0Db\0D\0Eb\0E\0F\0Gb\0G\0Ab\0A\0Bb\0B\0")
                      : CC("(none)\0C\0C#\0D\0D#\0E\0F\0F#\0G\0G#\0A\0A#\0B\0");
  CtlList root (ui->root, c), type (ui->type), bass (ui->bass, c);
   type.ClrLs ();
   for (i = 0;  i < NMChd;  i++) {
      StrCp (t, CC("............"));
      for (j = 0;  MChd [i].tmp [j] != 'x';  j++)
         t [(ubyte)(MChd [i].tmp [j])] = IntvMap [(ubyte)(MChd [i].tmp [j])];
      StrFmt (s, "`<5s  `s  `s", MChd [i].lbl, t, MChd [i].etc?MChd [i].etc:"");
      type.InsLs (s);
   }

// chord control picks (root, type, bass)  (n disabling)
   Set (ch);   ReDo ();
   Gui.DlgMv (this, Up.gp, "tR");
TRC("} DlgChd::Open");
}


void DlgChd::Shut ()
{ TStr x;
   Cmd (StrFmt (x, "@`d", _tm));
   done (true);   lower ();   hide ();
}


void DlgChd::Quit ()  {Gui.DlgSave (this, "DlgChd");}


//______________________________________________________________________________
// dlgCtl - pick a new drum track per section (patA,patB,fill)

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


//______________________________________________________________________________
// dlgCue - pick a new drum track per section (patA,patB,fill)

void DlgCue::Set (const char *s)
{ TStr s1, s2;
   if (StrCm (CC(s), CC("```")))  StrCp (s1, CC(s));
   else {
     CtlLine l (ui->str);
      StrCp (s1, l.Get ());
   }
   emit sgCmd (StrFmt (s2, "cue `s", s1));   Shut ();
}

void DlgCue::Open ()
{ CtlLine l (ui->str);
   l.Set (Up.pos.str);
   show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "tL");
}

void DlgCue::Shut ()  {done (true);   lower ();   hide ();}

void DlgCue::Init ()
{  Gui.DlgLoad (this, "DlgCue");
  CtlTBar tb (this,
      "redo loops and erase all bug history"
                         "`:/tbar/cue/a0"  "`\0"
      "delete this cue"  "`:/tbar/cue/a1"  "`\0"
      "text / non repeating section"
                         "`:/tbar/cue/b0"  "`\0"
      "verse section"    "`:/tbar/cue/b1"  "`\0"
      "chorus section"   "`:/tbar/cue/b2"  "`\0"
      "break section"    "`:/tbar/cue/b3"  "`\0"
      "crescendo"        "`:/tbar/cue/b4"  "`\0"
      "decrescendo"      "`:/tbar/cue/b5"  "`\0"
      "fermata"          "`:/tbar/cue/c0"  "`\0"
      "tremelo"          "`:/tbar/cue/c1"  "`\0"
      "star"             "`:/tbar/cue/c2"  "`\0"
      "happy"            "`:/tbar/cue/c3"  "`\0"
      "sad"              "`:/tbar/cue/c4"  "`\0"
      "mad"              "`:/tbar/cue/c5"  "`\0"
      "piano x3"         "`*ppp"           "`\0"
      "piano x2"         "`*pp"            "`\0"
      "piano"            "`*p"             "`\0"
      "mezzo piano"      "`*mp"            "`\0"
      "mezzo forte"      "`*mf"            "`\0"
      "forte"            "`*f"             "`\0"
      "forte x2"         "`*ff"            "`\0"
      "forte x3"         "`*fff"           "`\0"
      "forzando"         "`*Fz"            "`\0"
   );
   connect (tb.Act (0),  & QAction::triggered, this, [this]()
                                                           {Set ("loopInit");});
   connect (tb.Act (1),  & QAction::triggered, this, [this]()  {Set ("");});
   connect (tb.Act (2),  & QAction::triggered, this, [this]()  {Set ("```");});
   connect (tb.Act (3),  & QAction::triggered, this, [this]()
                                                           {Set ("(verse");});
   connect (tb.Act (4),  & QAction::triggered, this, [this]()
                                                           {Set ("(chorus");});
   connect (tb.Act (5),  & QAction::triggered, this, [this]()
                                                           {Set ("(break");});
   connect (tb.Act (6),  & QAction::triggered, this, [this]()  {Set ("<");});
   connect (tb.Act (7),  & QAction::triggered, this, [this]()  {Set (">");});
   connect (tb.Act (8),  & QAction::triggered, this, [this]()  {Set ("`fer");});
   connect (tb.Act (9),  & QAction::triggered, this, [this]()  {Set ("`tre");});
   connect (tb.Act (10), & QAction::triggered, this, [this]()  {Set ("`sta");});
   connect (tb.Act (11), & QAction::triggered, this, [this]()  {Set ("`hap");});
   connect (tb.Act (12), & QAction::triggered, this, [this]()  {Set ("`sad");});
   connect (tb.Act (13), & QAction::triggered, this, [this]()  {Set ("`mad");});
   connect (tb.Act (14), & QAction::triggered, this, [this]()  {Set ("ppp");});
   connect (tb.Act (15), & QAction::triggered, this, [this]()  {Set ("pp");});
   connect (tb.Act (16), & QAction::triggered, this, [this]()  {Set ("p");});
   connect (tb.Act (17), & QAction::triggered, this, [this]()  {Set ("mp");});
   connect (tb.Act (18), & QAction::triggered, this, [this]()  {Set ("mf");});
   connect (tb.Act (19), & QAction::triggered, this, [this]()  {Set ("f");});
   connect (tb.Act (20), & QAction::triggered, this, [this]()  {Set ("ff");});
   connect (tb.Act (21), & QAction::triggered, this, [this]()  {Set ("fff");});
   connect (tb.Act (22), & QAction::triggered, this, [this]()  {Set ("Fz");});
}

void DlgCue::Quit ()  {Gui.DlgSave (this, "DlgCue");}


//______________________________________________________________________________
// dlgfl - PianoCheetah's file list (FL.lst[], FL.pos)
//         and dialog to mess w it

FLstDef FL;

static ubyte NCol;   static char *Col [8];
static ubyt4 NFnd;
static TStr  DirF, DirT;
static File  FFnd;

static bool SongOK (void *ptr, char dfx, char *fn)
// find any a.song files and put em in (StrArr)ptr
{ StrArr *a = (StrArr *) ptr;
  ubyt4   ln = StrLn (fn);
   if ( (dfx == 'f') && (ln > 7) && (! StrCm (& fn [ln-7], CC("/a.song"))) )
      {fn [ln-7] = '\0';   a->Add (fn);}
   return false;
}

void FLstDef::Load ()
// load last FLst[];  add new files we got;  del gone files
{ ubyt4  i, j, r, ins1, ins2;
  TStr   dr, fn, c;
  File   f;
  StrArr t (CC("FL.Load"), 65536, 65536*sizeof (TStr)/2);
TRC("FL.Load");
// load cfg/songlist.txt w order of learn,rep songs
   App.Path (fn, 'c');   StrAp (fn, CC("/songlist.txt"));   t.Load (fn);
   FL.pos = 0;   FL.lst.Ln = t.num;
   for (ubyt4 r = 0;  r < t.num;  r++)
      {StrCp (FL.lst [r], t.str [r]);   FL.lst [r][FL.X] = 'n';}
//DBG("songlist.txt:");
//for (i=0;i<FLst.Ln;i++) DBG("`d `s", i, FLst [i]);

// reinit t n list every a.song file in Pianocheetah dir
   t.Init (CC("lstS"), 65536, 65536*sizeof (TStr)/2);
   App.Path (dr, 'd');
   StrFmt (fn,  "`s/1_learning",   dr);   f.DoDir (fn, & t, SongOK);
   StrFmt (fn,  "`s/2_repertoire", dr);   f.DoDir (fn, & t, SongOK);
   StrFmt (fn,  "`s/3_done",       dr);   f.DoDir (fn, & t, SongOK);
   StrFmt (fn,  "`s/4_queue",      dr);   f.DoDir (fn, & t, SongOK);
   t.Sort ();
//DBG("songs:"); t.Dump();

// upd FL / ins at top / del learn/rep songs to keep prev order
   ins1 = 0;
   for (i = 0;  i < FL.lst.Ln;  i++)
      if (StrSt (FL.lst [i], CC("2_repertoire")))  break;
   ins2 = i;
   for (i = 0;  i < t.num;  i++) {
      if (StrSt (t.str [i], CC("3_done")) || StrSt (t.str [i], CC("4_queue")))
                                                                         break;
      for (j = 0;  j < FL.lst.Ln;  j++)     // do an upd ?
         if (! StrCm (t.str [i], FL.lst [j]))
                                              {FL.lst [j][FL.X] = 'y';   break;}
      if (j >= FL.lst.Ln)  {                // gotta ins
         if (StrSt (t.str [i], CC("1_learning"))) {
            FL.lst.Ins (ins1);   StrCp (FL.lst [ins1], t.str [i]);
                                        FL.lst [ins1][FL.X] = 'y';
            ins1++;   ins2++;
         }
         else {
            FL.lst.Ins (ins2);   StrCp (FL.lst [ins2], t.str [i]);
                                        FL.lst [ins2][FL.X] = 'y';
            ins2++;
         }
      }
   }
   for (j = 0;  j < FL.lst.Ln;)        // del gone ones
      {if (FL.lst [j][FL.X] == 'n')  FL.lst.Del (j);   else  j++;}

   r = FL.lst.Ln;                      // append done/queue to FLst[]
   for (;  i < t.num;  i++, r++) {     // FLAG init for rand load
      FL.lst.Ins (r);    StrCp (FL.lst [r], t.str [i]);
      FL.lst [r][FL.X] = StrSt (FL.lst [r], CC("4_queue")) ? 'n' : 'y';
   }
   Save ();
//for (i=0;i<FL.lst.Ln;i++) DBG("`d `c `s", i, FL.lst [i][FL.X], FL.lst [i]);
}


void FLstDef::Save ()
// just dump Learn/Rep (with manual sort) of FLst[] in cfg/songlist.txt
{ TStr fn;
  File f;
   App.Path (fn, 'c');   StrAp (fn, CC("/songlist.txt"));
   if (! f.Open (fn, "w"))  DBG("FL.Save  couldn't write songlist");
   for (ubyt4 r = 0;  r < FL.lst.Ln;  r++) {
      if (StrSt (FL.lst [r], CC("3_done")) || StrSt (FL.lst [r], CC("4_queue")))
                                                                          break;
      f.Put (FL.lst [r]);  f.Put (CC("\n"));
   }
   f.Shut ();
}


bool FLstDef::DoDir (char *dir)
// if pc dir (win expl play song dir), just scoot to it in list
{ ubyt4 i, ln;
  BStr  pc, fn, fne, c;
  File  f;
TRC("FL.DoDir `s", dir);
   FL.pos = 0;
   App.Path (pc, 'd');
   if (MemCm (dir,  pc,  StrLn (pc))) {
      Gui.Hey ("Put new midi files in .../pianocheetah/midi_import");
      return false;
   }
   if (! f.Size (StrFmt (fn, "`s/a.song", dir))) {
      if      (f.Size (StrFmt (fn, "`s/a.txt", dir))) {
         App.Run (StrFmt (c, "txt2song `p", fn));
         StrFmt (fne, "`s/RATS.txt", dir);
         if (f.Size (fne)) {
            f.Load (fne, c, sizeof (c), 'z');
            Gui.Hey (c);
            Gui.Quit ();               // eh, just blow the scene, mannn
         }
      }
      else if (f.Size (StrFmt (fn, "`s/a.mid", dir)))
         App.Run (StrFmt (c, "mid2song `p", fn));
      FL.Load ();
   }
   for (ln = StrLn (dir), i = 0;  i < FL.lst.Ln;  i++)
      if (! MemCm (dir, FL.lst [i], ln))
         {FL.pos = i;   return true;}  // got it?  hop to it n go
   return false;
}


bool FLstDef::DoFN (char *fn)          // just do ma dir
{ TStr dir;   StrCp (dir, fn);   Fn2Path (dir);   return DoDir (dir);  }


//______________________________________________________________________________
void DlgFL::Pik ()
{ sbyt2 p;
  TStr  fn, s, dt;
  BStr  buf, etc;
  ubyt4 r, i, d;
  bool  in = false;
  StrArr tb (CC("FLstEtc"), 16000, 6000*sizeof(TStr));
   if ((p = _t.CurRow ()) >= 0)  FL.pos = p;

// git the .song fn, load for infoz
   StrCp (fn, FL.lst [FL.pos]);   StrAp (fn, CC("/a.song"));
   tb.Load (fn, nullptr, CC("Track:"));

// plow thru top of .song till we hit DrumMap: or Track: and load interestin etc
   *buf = '\0';
   for (d = r = 0; r < tb.NRow (); r++) {
      StrCp (s, tb.Get (r));
      if ((! MemCm (s, CC("DrumMap:"), 8)) ||
          (! MemCm (s, CC("Track:"  ), 6)))  break;
      if  (! MemCm (s, CC("info={"), 6)) {
         d = 1;
         for (i = r+1;  i < tb.NRow ();  i++) {
            if (! MemCm (tb.Get (i), CC("}"), 1))  break;
            StrAp (buf, tb.Get (i));   StrAp (buf, CC("\n"));
         }
      }
   }
   if (d)  StrAp (buf, CC("---------------------------------------\n"));
   for (d = r = 0; r < tb.NRow (); r++) {
      StrCp (s, tb.Get (r));
      if ((! MemCm (s, CC("DrumMap:"), 8)) ||
          (! MemCm (s, CC("Track:"), 6)))  break;
      if  (! MemCm (s, CC("notes "), 6))
         {d = 1;   StrAp (buf, & s [6]);   StrAp (buf, CC("\n"));}
   }
   if (d)  StrAp (buf, CC("---------------------------------------\n"));
   for (r = 0; r < tb.NRow (); r++) {
      StrCp (s, tb.Get (r));
      if ((! MemCm (s, CC("DrumMap:"), 8)) ||
          (! MemCm (s, CC("Track:"), 6)))  break;
      if             (! StrSt (s, CC("={")))     in = true;
      else if (in && (! MemCm (s, CC("}"), 1)))  in = false;
      else if (in || (! MemCm (s, CC("pract "), 6)))  ;
      else  {StrAp (buf, s);   StrAp (buf, CC("\n"));}     // append to etc
   }
   for (d = r = 0; r < tb.NRow (); r++) {
      StrCp (s, tb.Get (r));
      if ((! MemCm (s, CC("DrumMap:"), 8)) ||
          (! MemCm (s, CC("Track:"), 6)))  break;
      if  (! MemCm (s, CC("pract "),   6)) {
         if (d == 0) {
            d = 1;
            StrAp (buf, CC("days practiced...\n"));
            StrAp (buf, CC("                 1111111111222222222233\n"));
            StrAp (buf, CC("yyyymm  1234567890123456789012345678901\n"));
         }
         MemCp  (dt,  & s [6], 6);   dt [6] = '\0';
         MemSet (etc, '.', 31);    etc [31] = '\0';
         for (i = 13;  i < StrLn (s);  i += 3)
            {d = Str2Int (& s [i]);   if (d && (d < 32))  etc [d-1] = '*';}
         StrAp (buf, dt);   StrAp (buf, CC("  "));   StrAp (buf, etc);
                                                     StrAp (buf, CC("\n"));
      }
   }
  CtlText e (ui->etc);   e.Clr ();   e.Add (buf);
}


void DlgFL::ReDo ()                    // FL.lst/FL.pos => gui tbl
{ char *ro [10];
  TStr  ts, s1, s2;
  ubyt4 i, ln, p, d;
  bool  all;
  CtlChek c (ui->all);
   all = c.Get ();
   _t.Open ();
   if (! (ln = FL.lst.Ln))  {_t.Shut ();   return;}

   App.Path (ts, 'd');   p = StrLn (ts) + 1;
   for (i = 0;  i < ln;  i++) {
      StrCp (ts, & FL.lst [i][p]);
      if ((! all) && (*ts >= '3')) {   // not doin all?  done unless sPos sez
         if (FL.pos >= i)  {all = true;  c.Set (true);}
         else              break;
      }
      switch (*ts) {
      // 1_learning/ 2_repertoire/ 3_done/ 4_queue/ etc
         case '1':  StrCp (s1, CC("learn"));   StrCp (s2, & ts [11]);   break;
         case '2':  StrCp (s1, CC("rep"));     StrCp (s2, & ts [13]);   break;
         case '3':  StrCp (s1, CC("done"));    StrCp (s2, & ts [ 7]);   break;
         case '4':  StrCp (s1, CC("queue"));   StrCp (s2, & ts [ 8]);   break;
      }
      if (FnMid (s2))  StrAp (s2, CC(""), 4);
      d = StrLn (s2);
      if ( (d >= 9) && (! StrCm (& s2 [d-9], CC("_song.txt"))) )
         StrAp (s2, CC(""), 4);
      ro [0] = s1;   ro [1] = s2;   ro [2] = nullptr;
      _t.Put (ro);
   }
   _t.Shut ();
DBG("colw=`d", _t.ColW (1));
   if (_t.ColW (1) > 600)  _t.SetColW (1, 600);
   _t.HopTo (FL.pos, 0);
   Pik ();
}


//______________________________________________________________________________
static char *FLFind (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
// find cached mid files matchin search Col[NCol] - count,make found.txt
{                              (void)len; (void)pos; (void)ptr;
   for (ubyte i = 0;  i < NCol;  i++)  if (! StrSt (fn, Col [i]))
                                           return nullptr;
   NFnd++;
DBG("FLFind fn=`s", fn);
   FFnd.Put (fn);   FFnd.Put (CC("\n"));   return nullptr;
}


static char *FLCopy (char *fr, ubyt2 len, ubyt4 pos, void *ptr)
// copy and Mid2Song found mids (limit to 100)
{ ubyte i;                     (void)len;            (void)ptr;
  BStr  to, frP, toP, cmd;
  File  f;
   if (pos >= 100)  return CC("enough, pal!");
   StrCp (to, fr);   StrAp (to, CC(""), 4);   FnFix (to);
   StrFmt (frP, "`s/`s",          DirF, fr);
   StrFmt (toP, "`s/`s/path.txt", DirT, to);   f.Save (toP, frP, StrLn (frP));
   StrFmt (toP, "`s/`s/a.mid",    DirT, to);   f.Copy (frP, toP);
   App.Run (StrFmt (cmd, "mid2song `p", toP));
   return nullptr;
}


void DlgFL::Find ()
{ ubyt4 i, ln;
  TStr  srch, dMid, dFnd, fnC, fnF, c;
  File  f;
  Path  d;
// turn srch into Col[],NCol.  get search dir n save.
  CtlLine l (ui->srch);
   StrCp (srch, l.Get ());
  ColSep ss (srch, 8);
   for (NCol = 0;  ss.Col [NCol][0];  NCol++)  Col [NCol] = ss.Col [NCol];
   App.CfgGet (CC("DlgFL_dir"), dMid);
DBG("a dir=`s", dMid);
   if (*dMid == '\0')  StrCp (dMid, getenv ("HOME"));
DBG("b dir=`s", dMid);
   if (! Gui.AskDir (dMid, "pick dir to search for songs in (NOT / please)"))
      return;

   App.CfgPut (CC("DlgFL_dir"), dMid);

   StrFmt (fnC, "`s/_midicache.txt", dMid);
   if (! f.Size (fnC)) {               // no cache yet so start makin one
DBG("no _midicache.txt for `s", dMid);
      App.Run (StrFmt (c, "ll midi `p &", dMid));
      Gui.Hey ("Making midi cache in that dir...\n"
               "Come back when __midicache.txt turns to _midicache.txt");
      return;
   }

// wipe n recreate 4_queue/found;  start writin 4_queue/found.txt
   StrFmt (dFnd, "`s/4_queue/found", App.Path (c, 'd'));
   d.Kill (dFnd);   d.Make (dFnd);
   NFnd = 0;   if (! FFnd.Open (StrFmt (fnF, "`s.txt", dFnd), "w"))  return;

// FLFind() each cache fn  n shut found.txt
   f.DoText (fnC, nullptr, FLFind);   FFnd.Shut ();
DBG("found `d", NFnd);
   if (NFnd == 0)   {Gui.Hey ("rats!  got nothin");   return;}
   if (NFnd >= 100)  if (Gui.YNo (
                            "I'm only copyin 100 of the midi files, pal.\n"
                            "wanna view all matched filenames?"))
                        App.Open (fnF);
// ok copy em to 4_queue/found
   StrCp (DirF, dMid);   StrCp (DirT, dFnd);   f.DoText (fnF, nullptr, FLCopy);

// relist and move pos to 4_queue/found
  CtlChek a (ui->all);
   a.Set (true);   FL.Load ();   FL.pos = 0;
   for (ln = StrLn (dFnd), i = 0;  i < FL.lst.Ln;  i++)
      if (! MemCm (dFnd, FL.lst [i], ln))  {FL.pos = i;   break;}
   ReDo ();
}


void DlgFL::Up ()
{ ubyt4 p = FL.pos;
   if (p == 0)  return;
   FL.lst.MvUp (p);   FL.pos--;   FL.Save ();   ReDo ();
}

void DlgFL::Dn ()
{ ubyt4 p = FL.pos;
   if (p >= FL.lst.Ln-1)  return;
   FL.lst.MvDn (p);   FL.pos++;   FL.Save ();   ReDo ();
}

void DlgFL::Sfz2Syn ()
{ TStr d;
  BStr c;
   App.CfgGet (CC("DlgFL_sfzdir"), d);
   if (*d == '\0')  StrCp (d, getenv ("HOME"));
   if (Gui.AskDir (d, "pick dir with .sfz files")) {
      App.CfgPut (CC("DlgFL_sfzdir"), d);   // remember it
      App.Run (StrFmt (c, "sfz2syn `p", d));
   }
   App.Run (CC("synsnd"));             // in case dirs del'd etc
}

void DlgFL::Mod2Song ()
{ TStr d;
  BStr c;
   App.CfgGet (CC("DlgFL_moddir"), d);
   if (*d == '\0')  StrCp (d, getenv ("HOME"));
   if (Gui.AskDir (d, "pick dir with .sfz files")) {
      App.CfgPut (CC("DlgFL_moddir"), d);   // remember it
      App.Run (StrFmt (c, "mod2song `p", d));
   }
   App.Run (CC("synsnd"));             // in case dirs del'd etc
}

void DlgFL::Brow ()
{ TStr d, dv;
   App.Open (StrFmt (dv, "`s/device", App.Path (d, 'd')));
}


//______________________________________________________________________________
void DlgFL::Open ()  {ReDo ();   show ();   raise ();   activateWindow ();}

void DlgFL::Shut ()
{  FL.pos = _t.CurRow ();   done (true);   lower ();   hide ();  }

void DlgFL::Init ()
{  Gui.DlgLoad (this, "DlgFL", ui->spl);   FL.Load ();
  CtlTBar tb (this,
      "Up\n"       "Scoot song up in the list"
                   "`:/tbar/flst/0" "`\0"
      "Down\n"     "Scoot song down in the list"
                   "`:/tbar/flst/1" "`\0"
      "Search\n"   "Search a big midi file dir for matching search strings\n"
                   "   fill in the search box below THEN click me"
                   "`:/tbar/flst/2" "`\0"
      "Sfz2Syn\n"  "Pick a dir with .sfz files to add to Syn's sound banks"
                   "`:/tbar/flst/3" "`\0"
      "Mod2Song\n" "Pick a .mod files to convert to a song and\n"
                   "add to Syn's sound banks"
                   "`:/tbar/flst/4" "`\0"
      "Browse\n"   "Open file browser in PianoCheetah/device directory\n"
                   "   to delete/rename/etc"
                   "`:/tbar/flst/5" "`\0"
   );
   connect (tb.Act (0), & QAction::triggered, this, & DlgFL::Up);
   connect (tb.Act (1), & QAction::triggered, this, & DlgFL::Dn);
   connect (tb.Act (2), & QAction::triggered, this, & DlgFL::Find);
   connect (tb.Act (3), & QAction::triggered, this, & DlgFL::Sfz2Syn);
   connect (tb.Act (4), & QAction::triggered, this, & DlgFL::Mod2Song);
   connect (tb.Act (5), & QAction::triggered, this, & DlgFL::Brow);

   _t.Init (ui->fLst, "Stage\0Song\0");
   connect (ui->fLst, &QTableWidget::itemClicked,       this, & DlgFL::Pik);
   connect (ui->fLst, &QTableWidget::itemDoubleClicked, this, & DlgFL::Shut);

  CtlLine s (ui->srch);
  CtlChek a (ui->all);
  TStr t;
   App.CfgGet (CC("DlgFL_srch"), t);   if (*t) s.Set (t);
   App.CfgGet (CC("DlgFL_all"),  t);
   if (*t)  a.Set ((*t=='y')?true:false);   else a.Set (true);

   connect (ui->all, &QCheckBox::stateChanged, this, & DlgFL::ReDo);
}

void DlgFL::Quit ()
{ CtlLine s (ui->srch);
  CtlChek a (ui->all);
  TStr t;
   StrCp (t, s.Get ());               App.CfgPut (CC("DlgFL_srch"), t);
   StrCp (t, CC(a.Get ()?"y":"n"));   App.CfgPut (CC("DlgFL_all"),  t);
   Gui.DlgSave (this, "DlgFL", ui->spl);
}


//______________________________________________________________________________
// dlgFng - fingering picker

void DlgFng::Set (ubyte f)
{  emit sgCmd (StrFmt (Up.pos.str, "`s `d", _s, f));   Shut ();  }

void DlgFng::Open ()
{  StrFmt (_s, "fng `d `d", Up.pos.tr, Up.pos.p);
  CtlLabl l (ui->info);
   l.Set (Up.pos.str);
  CtlBttn qp (ui->qp);
  CtlBttn qn (ui->qn);
  TStr s;
   qp.Set (StrFmt (s, "quantPrev: `s", Up.pos.stp));
   qn.Set (StrFmt (s, "quantNext: `s", Up.pos.stn));
   show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgFng::Shut ()  {done (true);   lower ();   hide ();}

void DlgFng::Init ()
{  Gui.DlgLoad (this, "DlgFng");
  CtlTBar tb (this,
      "swap note to other hand's track" "`:/tbar/fng/0" "`\0"
      "delete this note"                "`:/tbar/fng/1" "`\0"
      "delete =ALL= fingering in the song (BE CAREFUL)"
                                        "`:/tbar/fng/2" "`\0"
      "left hand (drums)"           "`:/tbar/fng/l0" "`\0"
      "right hand (drums)"          "`:/tbar/fng/l1" "`\0"
      "left foot (drums/organ)"     "`:/tbar/fng/l2" "`\0"
      "right foot (drums/organ)"    "`:/tbar/fng/l3" "`\0"
      "accent"                      "`:/tbar/fng/l4" "`\0"
      "staccatissimo (or WHAtever)" "`:/tbar/fng/l5" "`\0"
   );
   connect (ui->qp,   & QPushButton::clicked,  this, [this]()  {Set (95);});
   connect (ui->qn,   & QPushButton::clicked,  this, [this]()  {Set (96);});
   connect (tb.Act (0), & QAction::triggered,  this, [this]()  {Set (97);});
   connect (tb.Act (1), & QAction::triggered,  this, [this]()  {Set (98);});
   connect (tb.Act (2), & QAction::triggered,  this, [this]()  {Set (99);});

   connect (tb.Act (3), & QAction::triggered,  this, [this]()  {Set (26);});
   connect (tb.Act (4), & QAction::triggered,  this, [this]()  {Set (27);});
   connect (tb.Act (5), & QAction::triggered,  this, [this]()  {Set (28);});
   connect (tb.Act (6), & QAction::triggered,  this, [this]()  {Set (29);});
   connect (tb.Act (7), & QAction::triggered,  this, [this]()  {Set (30);});
   connect (tb.Act (8), & QAction::triggered,  this, [this]()  {Set (31);});

   connect (ui->fOff, & QPushButton::clicked,  this, [this]()  {Set (0);});

   connect (ui->f1,   & QPushButton::clicked,  this, [this]()  {Set (1);});
   connect (ui->f2,   & QPushButton::clicked,  this, [this]()  {Set (2);});
   connect (ui->f3,   & QPushButton::clicked,  this, [this]()  {Set (3);});
   connect (ui->f4,   & QPushButton::clicked,  this, [this]()  {Set (4);});
   connect (ui->f5,   & QPushButton::clicked,  this, [this]()  {Set (5);});

   connect (ui->f12,  & QPushButton::clicked,  this, [this]()  {Set (6);});
   connect (ui->f13,  & QPushButton::clicked,  this, [this]()  {Set (7);});
   connect (ui->f14,  & QPushButton::clicked,  this, [this]()  {Set (8);});
   connect (ui->f15,  & QPushButton::clicked,  this, [this]()  {Set (9);});

   connect (ui->f21,  & QPushButton::clicked,  this, [this]()  {Set (10);});
   connect (ui->f23,  & QPushButton::clicked,  this, [this]()  {Set (11);});
   connect (ui->f24,  & QPushButton::clicked,  this, [this]()  {Set (12);});
   connect (ui->f25,  & QPushButton::clicked,  this, [this]()  {Set (13);});

   connect (ui->f31,  & QPushButton::clicked,  this, [this]()  {Set (14);});
   connect (ui->f32,  & QPushButton::clicked,  this, [this]()  {Set (15);});
   connect (ui->f34,  & QPushButton::clicked,  this, [this]()  {Set (16);});
   connect (ui->f35,  & QPushButton::clicked,  this, [this]()  {Set (17);});

   connect (ui->f41,  & QPushButton::clicked,  this, [this]()  {Set (18);});
   connect (ui->f42,  & QPushButton::clicked,  this, [this]()  {Set (19);});
   connect (ui->f43,  & QPushButton::clicked,  this, [this]()  {Set (20);});
   connect (ui->f45,  & QPushButton::clicked,  this, [this]()  {Set (21);});

   connect (ui->f51,  & QPushButton::clicked,  this, [this]()  {Set (22);});
   connect (ui->f52,  & QPushButton::clicked,  this, [this]()  {Set (23);});
   connect (ui->f53,  & QPushButton::clicked,  this, [this]()  {Set (24);});
   connect (ui->f54,  & QPushButton::clicked,  this, [this]()  {Set (25);});
}

void DlgFng::Quit ()  {Gui.DlgSave (this, "DlgFng");}


//______________________________________________________________________________
// dlgHlp - help meee

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


//______________________________________________________________________________
// dlgKSg - edit keysig

static char *Ma = CC(
"C   (0b)\0"                  "Db  (5b  also C# below)\0"
"D   (2#)\0"                  "Eb  (3b)\0"
"E   (4#)\0"                  "F   (1b)\0"
"Gb  (6b  also F# below)\0"   "G   (1#)\0"
"Ab  (4b)\0"                  "A   (3#)\0"
"Bb  (2b)\0"                  "B   (5#  also Cb below)\0"
"F#  (6#  also Gb above)\0"   "C#  (7#  also Db above)\0"
"Cb  (7b  also B  above)\0"),
            *Mi = CC(
"A   (0b)\0"                  "Bb  (5b  also A# below)\0"
"B   (2#)\0"                  "C   (3b)\0"
"C#  (4#)\0"                  "D   (1b)\0"
"Eb  (6b  also D# below)\0"   "E   (1#)\0"
"F   (4b)\0"                  "F#  (3#)\0"
"G   (2b)\0"                  "G#  (5#  also Ab below)\0"
"D#  (6#  also Eb above)\0"   "A#  (7#  also Bb above)\0"
"Ab  (7b  also G# above)\0");

void DlgKSg::Open ()
{  show ();   raise ();   activateWindow ();
   StrCp (_s, Up.pos.etc);
  TStr s;
   StrCp (s, Up.pos.str);
  char *m = & s [StrLn (s)-1];
  CtlList maj (ui->maj, CC("Major\0Minor\0")),
          key (ui->key, *m == 'm' ? Mi : Ma);
   maj.Set (*m == 'm' ? 1 : 0);
   if (*m == 'm')  *m = '\0';
   StrAp (s, CC(" "));   key.SetS (s);
   Gui.DlgMv (this, Up.gp, "tR");
}

void DlgKSg::Shut ()
{ TStr s;
  CtlList key (ui->key), maj (ui->maj);
   key.GetS (s);
   if (s [1] == ' ')  s [1] = '\0';   else s [2] = '\0';
   if (maj.Get ())  StrAp (s, CC("m"));
   StrAp (_s, s);
   emit sgCmd (_s);
   done (true);   lower ();   hide ();
}

void DlgKSg::Init ()
{  Gui.DlgLoad (this, "DlgKSg");
   connect (ui->maj, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
     ubyt2   p =  ui->key->currentIndex ();
     CtlList key (ui->key, ui->maj->currentIndex () ? Mi : Ma);
      key.Set (p);
   });
}

void DlgKSg::Quit ()  {Gui.DlgSave (this, "DlgKSg");}


//______________________________________________________________________________
// dlgMov - what ta do with our rect o notes

void DlgMov::Open ()
{  show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgMov::Shut ()  {done (true);   lower ();   hide ();}

void DlgMov::Init ()
{  Gui.DlgLoad (this, "DlgMov");
   connect (ui->h2r, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC(">"));   Shut ();});
   connect (ui->h2l, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("<"));   Shut ();});
   connect (ui->h2b, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("#"));   Shut ();});
   connect (ui->del, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("x"));   Shut ();});
}

void DlgMov::Quit ()  {Gui.DlgSave (this, "DlgMov");}


//______________________________________________________________________________
// dlgTDr - pick a new drum track per section (patA,patB,fill)

static BStr ALst, BLst, FLst;

static char TDrPop (char *ls, ubyt2 r, ubyte c)
{  (void)r;   ZZCp (ls, (c==1) ? ALst : ((c==2) ? BLst : FLst));  return 'l';}


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
   _t.Init (ui->t, "Section\0_PatA\0_PatB\0_Fill\0", TDrPop);
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


//______________________________________________________________________________
// dlgTpo - edit tempo

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


//______________________________________________________________________________
// dlgTSg - edit timesig

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


//______________________________________________________________________________
