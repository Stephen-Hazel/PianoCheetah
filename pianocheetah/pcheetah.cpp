// pcheetah.cpp - PianoCheetah - Steve's weird midi sequencer

#include "pcheetah.h"

TStr Kick;

void PCheetah::Trak ()
{ QSplitter *sp = ui->spl;
  QList<int> sz = sp->sizes ();
   if (sz [0])                         // table does minimumExpand ta werk
        {Gui.FullSc (true);   sp->setSizes (QList<int>() << 0  <<  width ());}

   else {Gui.FullSc (false);  sp->setSizes (QList<int>() << 90 <<
                                                              (width () - 90));}

}

void PCheetah::GCfg ()  {_dCfg->Open ();}
void PCheetah::MCfg ()  {DBG("gonna midicfg");
                         StrCp (Kick, CC("midicfg"));   Gui.Quit ();}
void PCheetah::LoadGo ()
{ TStr s;   emit sgCmd (StrFmt (s, "load `s", FL.lst [FL.pos]));  }

void PCheetah::Load ()  {emit sgCmd ("wipe");   _dFL->Open ();}


static char *LstLen (char *ln, ubyt2 len, ubyt4 pos, void *ptr)
{  if (MemCm (ln, CC("DIDXX "), 6)) FL.xLen++;   return nullptr;  }

static File  Fw;

static char *LstGet (char *ln, ubyt2 len, ubyt4 pos, void *ptr)
{ TStr o;
  static ubyt4 n;
   StrFmt (o, "`s\n", ln);
   if (pos == 0)  n = 0;
   if (MemCm (o, CC("DIDXX "), 6))
      if (n++ == FL.xPos)  {StrCp (FL.xFn, ln);   StrFmt (o, "DIDXX `s\n", ln);}
   Fw.Put (o);   return nullptr;
}

static void XLoad ()
{ ubyt4 ln, i;
  TStr  dMid, fnC, fnW, dFnd, s;
  File  f;
  Path  d;
   App.CfgGet (CC("DlgFL_dir"), dMid);
   ln = f.Size (StrFmt (fnC,  "`s/_midicache.txt", dMid));

   if (Fw.Open (StrFmt (fnW, "`s/__midicache.txt", dMid), "w"))
      {f.DoText (fnC, nullptr, LstGet);   Fw.Shut ();}
   f.ReNm (fnW, fnC);
//TODO recache if FL.xLen hits 0
   if (--FL.xLen == 0)  {Gui.Hey ("_midicache has 0 left - repick midi dir");
                         return;}
TRC("   xPos=`d xLen=`d xFn=`s dMid=`s", FL.xPos, FL.xLen, FL.xFn, dMid);
// recreate dest dir
   StrFmt (dFnd, "`s/4_queue/found", App.Path (s, 'd'));
   d.Kill (dFnd);   d.Make (dFnd);

// ok copy it to 4_queue/found
  TStr fr, to, frP, toP, cmd;
   StrCp  (to, FL.xFn);   StrAp (to, CC(""), 4);   FnFix (to);
   StrFmt (frP, "`s/`s",          dMid, FL.xFn);
   StrFmt (toP, "`s/`s/path.txt", dFnd, to);   f.Save (toP, frP, StrLn (frP));
   StrFmt (toP, "`s/`s/a.mid",    dFnd, to);   f.Copy (frP, toP);
   App.Run (StrFmt (cmd, "mid2song `p", toP));

// relist and move pos to 4_queue/found
   FL.Load ();   FL.pos = 0;
   for (ln = StrLn (dFnd), i = 0;  i < FL.lst.Ln;  i++)
      if (! MemCm (dFnd, FL.lst [i], ln))  {FL.pos = i;   break;}
}


void PCheetah::SongNxt ()
{  emit sgCmd ("wipe");
   if (FL.ext) {if (FL.xPos+1 < FL.xLen)   FL.xPos++;
                XLoad ();}
   else         if (FL.pos+1 < FL.lst.Ln)  FL.pos++;
   LoadGo ();
}


void PCheetah::SongPrv ()
{  emit sgCmd ("wipe");
   if (FL.ext) {if (FL.xPos)  FL.xPos--;
                XLoad ();}
   else         if (FL.pos)   FL.pos--;
   LoadGo ();
}


void PCheetah::SongRand ()
{ ubyt4 ln;
  TStr  dMid, fnC, c;
  File  f;
   emit sgCmd ("wipe");
   ln = 0;
   App.CfgGet (CC("DlgFL_dir"), dMid);
   if (*dMid)  ln = f.Size (StrFmt (fnC, "`s/_midicache.txt", dMid));
   if ((*dMid == '\0') || (! ln)) {
      if (*dMid == '\0')  StrCp (dMid, getenv ("HOME"));
      if (! Gui.AskDir (dMid, "pick dir to search for midis in (NOT / please)"))
         return;
      App.CfgPut (CC("DlgFL_dir"), dMid);
   }
   StrFmt (fnC, "`s/_midicache.txt", dMid);
   if (! f.Size (fnC)) {               // no cache yet so start makin one
TRC("no _midicache.txt for `s", dMid);
      App.Run (StrFmt (c, "ll midi `p &", dMid));
      Gui.Hey ("Making midi cache in that dir...\n"
               "Come back when __midicache.txt turns to _midicache.txt");
      return;
   }

// count # undid undeld
   FL.ext = true;   FL.xLen = 0;   f.DoText (fnC, nullptr, LstLen);
TRC("   init FL.xLen=`d", FL.xLen);
   FL.xPos = Rand (FL.xLen);
   XLoad ();
   LoadGo ();
}


void PCheetah::SongKill ()
{ ubyt4 p;
  TStr  dr, t;
  Path  d;
   if ((p = FL.pos) >= FL.lst.Ln)  return;
TRC("SongKill `s", FL.lst [FL.pos]);
   StrCp (dr, FL.lst [p]);   App.Path (t, 'd');   StrAp (t, CC("/4_queue/"));
   if (MemCm (dr, t, StrLn (t)))
      {Gui.Hey ("songKill only works in 4_queue dir");   return;}

//TODO gotta path.txt?  rm that .mid too
   FL.lst.Del (p);   d.Kill (dr);   if (p == FL.lst.Ln)  p--;
   FL.pos = p;   LoadGo ();
}


//______________________________________________________________________________
// _tr (ui->tr) stuph...

char TrPop (char *ls, ubyt2 r, ubyte c)
// pop HT, sounddir, sound CtlList
{  *ls = '\0';
TRC("TrPop r=`d c=`d", r, c);
   if ((c == 1) && (Up.trk [r].lrn [0] == 'l') && (! Up.trk [r].drm)) {
      MemCp (ls, CC("4RH\0"
                    "3LH\0"
                    "1LH\0"
                    "2LH\0"
                    "5RH\0"
                    "6RH\0"
                    "7RH\0"), 7*4+1);
      return 'l';
   }
   if (c == 2) {
      if (! Up.trk [r].drm)  return 'e';    // edit melodic track name
     StrArr l (CC("drum.txt"), 256, 128*sizeof(TStr));
TRC("dvt=`d name=`s", Up.trk [r].dvt, Up.dvt [Up.trk [r].dvt].Name ());
     TStr   s, s2;                     // see if devtype has a drum.txt
      l.Load (StrFmt (s, "`s/device/`s/drum.txt",
         App.Path (s2, 'd'), Up.dvt [Up.trk [r].dvt].Name ()));
      if (l.NRow ())  {l.SetZZ (ls);   return 'l';}
   }                                   // else fall thru to readonly
   if ((c == 3) || (c == 4)) {
     ubyte dvt = Up.trk [r].dvt;
      if (Up.trk [r].drm && StrCm (Up.dvt [Up.trk [r].dvt].Name (), CC("syn")))
         return '\0';                  // no sound editin on nonsyn drums
      if (c == 3)  Up.dvt [dvt].SGrp (ls,                 Up.trk [r].drm);
      if (c == 4)  Up.dvt [dvt].SNam (ls, Up.trk [r].grp, Up.trk [r].drm);
      return 'l';
   }
   if (c == 5) {
     ubyte d = 0;
     TStr  n, t, x;
      StrCp (ls, CC("+"));   ls += 2;
      while (Midi.GetPos ('o', d++, n, t, x, x))  if (StrCm (t, CC("OFF")))
         {StrCp (ls, n);   ls += (StrLn (n)+1);}
      *ls = '\0';
      return 'l';
   }
   return '\0';
}

void PCheetah::TrClk ()
{ ubyt2 r = _tr.CurRow ();
  ubyte c = _tr.CurCol ();
TRC("TrClk(L) r=`d c=`d", r, c);
   Up.eTrk = (ubyte)r;
   if      (c == 0)  emit sgCmd ("prac");
   else if (c == 1)  emit sgCmd ("htype x");
}

void PCheetah::TrClkR (const QPoint &pos)
{ ubyt2 r = _tr.CurRow ();   (void)pos;
  ubyte c = _tr.CurCol ();
TRC("TrClkR r=`d c=`d", r, c);
   Up.eTrk = (ubyte)r;
   if      (c == 0)  emit sgCmd ("mute");
   else if (c == 1)  emit sgCmd ("showAll");
}


void PCheetah::TrUpd ()
{ ubyt2 r = _tr.CurRow ();
  ubyte c = _tr.CurCol ();
  TStr  s;
   Up.eTrk = (ubyte)r;
TRC("TrUpd r=`d c=`d", r, c);
   switch (c) {
      case 1:                          // hand type  (\0,1..7,x)
         emit sgCmd (StrFmt (s, "htype `s", _tr.Get (r, 1)));   break;
      case 2:                          // track name/drum in
         if (Up.trk [r].drm)
              {emit sgCmd (StrFmt (s, "drmap `s", _tr.Get (r, 2)));   break;}
         else {emit sgCmd (StrFmt (s, "trk `s",   _tr.Get (r, 2)));   break;}
      case 3:                          // sound group
         emit sgCmd (StrFmt (s, "grp `s",   _tr.Get (r, 3)));   break;
      case 4:                          // sound name
         emit sgCmd (StrFmt (s, "snd `s",   _tr.Get (r, 4)));   break;
      case 5:                          // device.channel
         emit sgCmd (StrFmt (s, "dev `s",   _tr.Get (r, 5)));   break;
   }
}


//______________________________________________________________________________
void PCheetah::Upd (QString upd)
{ TStr  u, s;
  ubyte i;
   StrCp (u, UnQS (upd));
//TRC("Upd `s", u);
   for (i = 0;  i < NUCmd;  i++)  if (! StrCm (u, CC(UCmd [i].cmd)))  break;
   if (i < NUCmd) {
      if (i > 4)  emit sgCmd (upd);
      else  switch (i) {
         case 0:  SongPrv  ();   break;     // song<
         case 1:  SongNxt  ();   break;     // song>
         case 2:  SongRand ();   break;     // songRand
         case 3:  SongKill ();   break;     // songKill
         case 4:  Gui.Quit ();   break;     // exit
      }
      return;
   }

   if (! StrCm (u, CC("ttl")))  Up.tbaTtl->setText (Up.ttl);
//    Gui.SetTtl (StrFmt (s, "`s - PianoCheetah", Up.ttl));

   if (! StrCm (u, CC("FLdn"))) {if (FL.pos < FL.lst.Ln-1)
                                              {++FL.pos;   _dFL->ReDo ();}}
   if (! StrCm (u, CC("FLup"))) {if (FL.pos)  {--FL.pos;   _dFL->ReDo ();}}
   if (! StrCm (u, CC("FLgo"))) {_dFL->Shut ();   LoadGo ();}
   if (! StrCm (u, CC("FLex"))) {_dFL->Shut ();   Gui.Quit ();}

   if (! StrCm (u, CC("nt")))     _nt->update ();
   if (! StrCm (u, CC("ntCur")))  _nt->Cur ();
   if (! StrCm (u, CC("dTDr")))   _dTDr->Open ();
   if (! StrCm (u, CC("dCue")))   _dCue->Open ();
   if (! StrCm (u, CC("dChd")))   _dChd->Open ();
   if (! StrCm (u, CC("dCtl")))   _dCtl->Open ();
   if (! StrCm (u, CC("dTpo")))   _dTpo->Open ();
   if (! StrCm (u, CC("dTSg")))   _dTSg->Open ();
   if (! StrCm (u, CC("dKSg")))   _dKSg->Open ();
   if (! StrCm (u, CC("dQua")))   _dQua->Open ();
   if (! StrCm (u, CC("dMov")))   _dMov->Open ();

   if (! StrCm (u, CC("time"))) {
     CtlLabl l (ui->time);
     TStr s;
      StrCp (s, Up.time);
      if (*s == 'X')
         StrFmt (s, "<span style='color: red;'>`s</span>", & Up.time [1]);
      l.Set (s);
   }
   if (! StrCm (u, CC("bars")))  { CtlLabl l (ui->bars);  l.Set (Up.bars);}
   if (! StrCm (u, CC("tmpo")))  { CtlLabl l (ui->tmpo);  l.Set (Up.tmpo);}
   if (! StrCm (u, CC("tsig")))  { CtlLabl l (ui->tSig);  l.Set (Up.tsig);}
   if (! StrCm (u, CC("tbPoz")))  Up.tbaPoz->setIcon (
                                     *Up.tbiPoz [Up.uPoz ? 1 : 0]);
   if (! StrCm (u, CC("tbLrn")))  Up.tbaLrn->setIcon (
                                     *Up.tbiLrn [PLAY ? 1 : (PRAC ? 2 : 0)]);
   if (! StrCm (u, CC("lyr"))) {
     CtlText t (ui->lyr);
     QColor  fg = t.Fg (), hi = t.Hi ();
      t.Clr ();
      if (! Up.lyrHiE)  {t.SetFg (fg);   t.Add (Up.lyr);}
      else {
        ubyte b = Up.lyrHiB, e = Up.lyrHiE;
        TStr  s;
         StrCp (s, Up.lyr);
//TRC("lyr='`s' b=`d e=`d", Up.lyr, Up.lyrHiB, Up.lyrHiE);
         if (b)  {s [b] = '\0';     t.SetFg (fg);   t.Add (s);}
         StrCp (s, & Up.lyr [b]);   s [e-b] = '\0';
                                    t.SetFg (hi);   t.Add (s);
         StrCp (s, & Up.lyr [e]);   t.SetFg (fg);   t.Add (s);
      }
   }

   if (! StrCm (u, CC("trk"))) {
     char *rp [32];
     BStr  tip;
      _tr.Open ();   rp [6] = nullptr;
      for (ubyte i = 0, tc = 0;  i < Up.trk.Ln;  i++) {
         rp [0] = Up.trk [i].lrn;      rp [1] = Up.trk [i].ht;
         rp [2] = Up.trk [i].name;     rp [3] = Up.trk [i].grp;
         rp [4] = Up.trk [i].snd;      rp [5] = Up.trk [i].dev;
         _tr.Put (rp,                           Up.trk [i].tip);
         if (Cfg.ntCo == 2) {          // color by track (if lrn/show non drum)
            if (((rp [0][0] == 'l') || (rp [1][0] == 's')) &&
                (! Up.trk [i].drm))
               _tr.SetColor (i, CMap (tc++));
         }
         else {
            switch (rp [1][1]) {       // color lh/rh
               case 'L':  tc = 0;   break;
               case 'R':  tc = 1;   break;
               default:   tc = 9;
            }
            if (tc < 2)  _tr.SetColor (i, CTnt [tc]);
         }
      }
      _tr.Shut ();   _tr.HopTo (Up.eTrk, 0);
   }

   if (! MemCm (u, CC("hey "), 4))   Gui.Hey (& u [4]);
   if (! MemCm (u, CC("die "), 4))  {Gui.Hey (& u [4]);   Gui.Quit ();}
}


//______________________________________________________________________________
void PCheetah::keyPressEvent (QKeyEvent *e)
{ KeyMap km;
  ubyte i;
  key   k;
  TStr  s;
//DBG("keyPressEvent raw mod=`d key=`d", e->modifiers (), e->key ());
   k = km.Map (e->modifiers (), e->key ());
   if (! k)  return;
   StrCp (s, km.Str (k));
//DBG("   keystr=`s", s);
   for (i = 0;  i < NUCmd;  i++)  if (! StrCm (s, CC(UCmd [i].ky)))  break;
   if (i < NUCmd)               Upd (UCmd [i].cmd);
   if (! StrCm (s, CC("d")))    emit sgCmd ("dump");
   if (! StrCm (s, CC("f01")))  _dHlp->Open ();
}


void PCheetah::Init ()
{ TStr fn;
  File f;
TRC("Init");
   *Kick = '\0';
   _s    = nullptr;
   _dFL  = nullptr;   _dCfg = nullptr;   _dTDr = nullptr;
   _dCue = nullptr;   _dChd = nullptr;   _dCtl = nullptr;
   _dTpo = nullptr;   _dTSg = nullptr;   _dKSg = nullptr;
   _dQua = nullptr;   _dMov = nullptr;   _dHlp = nullptr;

   App.Path (fn, 'd');   StrAp (fn, CC("/device/device.txt"));
   if (! f.Size (fn))  return MCfg ();      // ain't no point in goin on

TRC(" got device.txt");
   Midi.Load ();
  ubyte i = 0;
  TStr  nm, ty, ds, dv;
   while (Midi.GetPos ('o', i++, nm, ty, ds, dv))
      if (StrCm (ty, CC("OFF")) && (*dv == '?'))
         {Gui.Hey (StrFmt (dv, "Hey! `s: `s (`s)  is off, pal...",
                               nm, ty, ds));   break;}
TRC(" song init");
   _s = new Song;                      // git song worker thread goin

   CInit ();                           // init all them thar colors
   Gui.WinLoad (ui->spl);
   Up.cnv.SetFont ("Noto Sans", 12);   Up.tcnv.SetFont ("Noto Sans", 12);
   Gui.A ()->setStyleSheet ("QToolTip {font: 15pt monospace;}");

   Up.txH   = Up.cnv.FontH ();
   Up.bug   = new QPixmap (":/bug");   // all dem bitmaps
   Up.cue   = new QPixmap (":/cue");
   Up.dot   = new QPixmap (":/dot");
   Up.fade  = new QPixmap (":/fade");
   Up.now   = new QPixmap (":/now");
   Up.oct   = new QPixmap (":/oct");
   Up.pnbg  = new QPixmap (":/pnbg");
   Up.pnbg2 = new QPixmap (":/pnbg2");
   Up.tpm   = new QPixmap ((32+88)*W_NT, M_WHOLE*4);

   _s->moveToThread (& _thrSong);
   connect (this,       & PCheetah::sgCmd,   _s,   & Song::Cmd);
   connect (_s,         & Song::sgUpd,       this, & PCheetah::Upd);
   connect (& _thrSong, & QThread::finished, _s,   & QObject::deleteLater);
   _thrSong.start ();
   emit sgCmd ("init");

   setFocusPolicy (Qt::StrongFocus);   // so we get keyPressEvent()s
TRC(" tbar init");
  CtlTBar tb2 (this,                   // top
      "view fullscreen / track editing" "`:/tbar/0" "`v\0"
      "configure midi devices"          "`:/tbar/1" "`\0"
      "settings and junk"               "`:/tbar/2" "`\0",
      "Config");
   connect (tb2.Act (0), & QAction::triggered, this, & PCheetah::Trak);
   connect (tb2.Act (1), & QAction::triggered, this, & PCheetah::MCfg);
   connect (tb2.Act (2), & QAction::triggered, this, & PCheetah::GCfg);

  CtlTBar tb3 (this,                   // list/prev/next/rand song
      "pick from song list" "`:/tbar/song/0" "`\0"
      "previous song"       "`:/tbar/song/1" "`z\0"
      "next song"           "`:/tbar/song/2" "`x\0"
      "random song"         "`*??"           "`a\0",
      "SongList");
   connect (tb3.Act (0), & QAction::triggered, this, & PCheetah::Load);
   connect (tb3.Act (1), & QAction::triggered, this, & PCheetah::SongPrv);
   connect (tb3.Act (2), & QAction::triggered, this, & PCheetah::SongNxt);
   connect (tb3.Act (3), & QAction::triggered, this, & PCheetah::SongRand);

  CtlTBar tb4 (this,
      "hear / play / practice\n"
         "Click Lrn column of track grid to practice it.\n"
         "Once you have played the song a few times, you can practice loops."
                         "`view-visible" "`l\0",
      "LearnMode");
   Up.tbaLrn = tb4.Act (0);   Up.tbiLrn [0] = new QIcon (":/tbar/lrn/0");
                              Up.tbiLrn [1] = new QIcon (":/tbar/lrn/1");
                              Up.tbiLrn [2] = new QIcon (":/tbar/lrn/2");
   connect (tb4.Act (0), & QAction::triggered,
            this, [this]() {emit sgCmd ("learn");});

  CtlTBar tb5 (this,                   // transport - play/pause/etc
      "restart"            "`:/tbar/time/0" "`1\0"
      "previous loop/page" "`:/tbar/time/1" "`Left\0"
      "previous bar"       "`:/tbar/time/2" "`2\0"
      "play / pause"       "`:/tbar/time/3" "`Space\0"
      "next bar"           "`:/tbar/time/4" "`3\0"
      "next loop/page"     "`:/tbar/time/5" "`Right\0",
      "Time");
   Up.tbaPoz = tb5.Act (3);   Up.tbiPoz [0] = new QIcon (":/tbar/time/3");
                              Up.tbiPoz [1] = new QIcon (":/tbar/time/6");
   connect (tb5.Act (0), & QAction::triggered,
            this, [this]() {emit sgCmd ("timeBar1");});
   connect (tb5.Act (1), & QAction::triggered,
            this, [this]() {emit sgCmd ("time<<");});
   connect (tb5.Act (2), & QAction::triggered,
            this, [this]() {emit sgCmd ("time<");});
   connect (tb5.Act (3), & QAction::triggered,
            this, [this]() {emit sgCmd ("timePoz");});
   connect (tb5.Act (4), & QAction::triggered,
            this, [this]() {emit sgCmd ("time>");});
   connect (tb5.Act (5), & QAction::triggered,
            this, [this]() {emit sgCmd ("time>>");});

  CtlTBar tb6 (this,
      "decrease tempo"           "`:/tbar/tmpo/0" "`F2\0"
      "tempo to 60%=>80%=100%=>" "`:/tbar/tmpo/1" "`t\0"
      "increase tempo"           "`:/tbar/tmpo/2" "`F3\0",
      "Tempo");
   connect (tb6.Act (0), & QAction::triggered,
            this, [this]() {emit sgCmd ("tempo<");});
   connect (tb6.Act (1), & QAction::triggered,
            this, [this]() {emit sgCmd ("tempoHop");});
   connect (tb6.Act (2), & QAction::triggered,
            this, [this]() {emit sgCmd ("tempo>");});

  CtlTBar tb7 (this,
      "`split the learn track (3E and below) into new LH track"
                                    "`:/tbar/trak/0" "`\0"
      "`make drum track from clips" "`:/tbar/trak/1" "`\0"
      "`insert track"               "`:/tbar/trak/2" "`\0"
      "`delete track"               "`:/tbar/trak/3" "`\0"
      "`scoot track up"             "`:/tbar/trak/4" "`\0"
      "`scoot track down"           "`:/tbar/trak/5" "`\0"
      "`time scaling - for { to } => } scales to ^"
                                    "`:/tbar/trak/6" "`\0"
      "`time offsetting - for { to end => { moves to ^"
                                    "`:/tbar/trak/7" "`\0",
      "EditTrack");
   connect (tb7.Act (0), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd sp");});
   connect (tb7.Act (1), & QAction::triggered,
            this, [this]() {emit sgCmd ("preTDr");});
   connect (tb7.Act (2), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd +");});
   connect (tb7.Act (3), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd x");});
   connect (tb7.Act (4), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd u");});
   connect (tb7.Act (5), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd d");});
   connect (tb7.Act (6), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd *");});
   connect (tb7.Act (7), & QAction::triggered,
            this, [this]() {emit sgCmd ("trkEd -");});

  CtlTBar tb (this,
      "(I just show the song file for fullscreen)" "`*..." "`v\0",
      "SongFile");
   connect (tb.Act (0), & QAction::triggered, this, & PCheetah::Trak);
   Up.tbaTtl = tb.Act (0);

TRC(" lyr,tr,nt init");
  CtlText t (ui->lyr);
   _tr.Init (ui->tr,
      "*Lrn\0"
     "*_Hand\0"
      "_Track\0"
      "_SnGrp\0"
      "_Sound\0"
      "_Dev.Ch\0", TrPop);
   _tr.SetRowH (Gui.FontH ()+1);

   ui->tr->setContextMenuPolicy (Qt::CustomContextMenu);
   connect (ui->tr, & QTableWidget::itemClicked, this, & PCheetah::TrClk);
   connect (ui->tr, & QTableWidget::customContextMenuRequested,
                                                 this, & PCheetah::TrClkR);
   connect (ui->tr, & QTableWidget::itemChanged, this, & PCheetah::TrUpd);

   _nt = ui->nt;
   connect (_nt, & CtlNt::sgReSz, _s, & Song::ReSz);
   connect (_nt, & CtlNt::sgMsDn, _s, & Song::MsDn);
   connect (_nt, & CtlNt::sgMsMv, _s, & Song::MsMv);
   connect (_nt, & CtlNt::sgMsUp, _s, & Song::MsUp);
   _nt->Init (ui->nt->width (), ui->nt->height ());

TRC(" dlg init");
   _dFL  = new DlgFL  (this);    _dFL->Init ();
   _dCfg = new DlgCfg (this);   _dCfg->Init ();
   _dTDr = new DlgTDr (this);   _dTDr->Init ();
   _dCue = new DlgCue (this);   _dCue->Init ();
   _dChd = new DlgChd (this);   _dChd->Init ();
   _dCtl = new DlgCtl (this);   _dCtl->Init ();
   _dTpo = new DlgTpo (this);   _dTpo->Init ();
   _dTSg = new DlgTSg (this);   _dTSg->Init ();
   _dKSg = new DlgKSg (this);   _dKSg->Init ();
   _dQua = new DlgQua (this);   _dQua->Init ();
   _dMov = new DlgMov (this);   _dMov->Init ();
   _dHlp = new DlgHlp (this);   _dHlp->Init ();
   connect (_dCfg, & DlgCfg::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dTDr, & DlgTDr::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dCue, & DlgCue::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dChd, & DlgChd::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dTpo, & DlgTpo::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dTSg, & DlgTSg::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dKSg, & DlgKSg::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dQua, & DlgQua::sgCmd, this, [this](char *s)  {emit sgCmd (s);});
   connect (_dFL,  & QDialog::accepted, this, & PCheetah::LoadGo);
   connect (_dFL,  & QDialog::rejected, this, & PCheetah::Quit);
   connect (_dCtl, & QDialog::accepted, this, [this]() {emit sgCmd ("ctl");});
   connect (_dMov, & QDialog::accepted, this, [this]() {emit sgCmd ("mov");});

// parse cmdline arg:  try to load song in dir or turn fn into song to do
  bool ld = false;
  TStr a;
  FDir d;
   StrCp (a, Gui.Arg (0));
TRC(" arg=`s", a);
   if (*a) ld = d.Got (a) ? FL.DoDir (a) : FL.DoFN (a);
   if (ld) LoadGo ();   else Load ();
TRC("Init end");
}


void PCheetah::Quit ()
{
TRC("Quit");
   if (_s != nullptr) {
TRC("  emit quit");
      emit sgCmd (CC("quit"));
   }
   if (_dMov != nullptr) {
TRC("  win,dlg save");
      Gui.WinSave (ui->spl);
      _dFL->Quit ();    _dCfg->Quit ();   _dTDr->Quit ();
      _dCue->Quit ();   _dChd->Quit ();   _dCtl->Quit ();
      _dTpo->Quit ();   _dTSg->Quit ();   _dKSg->Quit ();
      _dQua->Quit ();   _dMov->Quit ();   _dHlp->Quit ();
      delete _dFL;    delete _dCfg;   delete _dTDr;
      delete _dCue;   delete _dChd;   delete _dCtl;
      delete _dTpo;   delete _dTSg;   delete _dKSg;
      delete _dQua;   delete _dMov;   delete _dHlp;
   }
   if (_s != nullptr) {
TRC("  thrEnd");
      _thrSong.quit ();   _thrSong.wait ();
   }
TRC("  kick=`s", Kick);
   if (*Kick)  App.Spinoff (Kick);
TRC("Quit end");
}


int main (int argc, char *argv [])
{  DBGTH ("PcGui");
  QApplication app (argc, argv);
  PCheetah     win;
   App.Init ();                        //TODO scrsaver off.  limit one instance?
   Gui.Init (& app, & win, "PianoCheetah");   win.Init ();   RandInit ();
   qRegisterMetaType<ubyte>("ubyte");
   qRegisterMetaType<sbyt2>("sbyt2");
   qRegisterMetaType<Qt::MouseButton >("Qt::MouseButton" );
   qRegisterMetaType<Qt::MouseButtons>("Qt::MouseButtons");
  int rc = Gui.Loop ();                       win.Quit ();
   return rc;
}
