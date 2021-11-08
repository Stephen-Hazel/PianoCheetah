// dlgfl.cpp - PianoCheetah's file list (FL.lst[], FL.pos)
//             and dialog to mess w it

#include "pcheetah.h"

FLstDef FL;

static ubyte NCol;   static char *Col [8];
static ubyt4 NFnd;
static TStr  DirF, DirT;
static File  FFnd;

static bool SongOK (void *ptr, char dfx, char *fn)
// find any a.song files and put em in _tb
{ StrArr *a = (StrArr *) ptr;
  ubyt4   ln = StrLn (fn);
   if ( (dfx == 'f') && (ln > 7) && (! StrCm (& fn [ln-7], CC("/a.song"))) )
      {fn [ln-7] = '\0';   a->Add (fn);}
   return false;
}

static char *SongOK2 (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
{ StrArr *a = (StrArr *) ptr;
  TStr    s;
   (void)pos;   (void)len;
   a->Add (StrFmt (s, "`s/`s", a->x, fn));   return nullptr;
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
   StrFmt (t.x, "`s/4_queue", dr);   StrFmt (fn, "`s/_songcache.txt", t.x);
   if (f.Size (fn))  f.DoText (fn, & t, SongOK2);     // MidImp done w my cache?
   else              t.Add (StrFmt (c, "`s/(caching - come back later)", t.x));
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
      FL.lst.Ins (r);   StrCp (FL.lst [r], t.str [i]);
      FL.lst [r][FL.X] = StrSt (FL.lst [r], CC("4_queue")) ? 'n' : 'y';
   }
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


static char *FLFind (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
// find cached mid/kar/rmi files matchin search Col[NCol] - count,make found.txt
{  (void)pos;   (void)len;   (void)ptr;
   for (ubyte i = 0;  i < NCol;  i++)  if (! StrSt (fn, Col [i]))
                                           return nullptr;
   NFnd++;
DBG("FLFind fn=`s", fn);
   FFnd.Put (fn);   FFnd.Put (CC("\n"));   return nullptr;
}


static char *FLCopy (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
{ ubyte i;
  TStr  fr, to, fnx, ext, cmd;
  File  f;                             // copy and Mid2Song found mids
   (void)pos;   (void)len;   (void)ptr;
   StrCp (fnx, fn);   for (i = 0;  i < StrLn (fnx);  i++)
                         if (fnx [i] == '/')  fnx [i] = '_';
   StrCp (ext, & fnx [StrLn (fnx)-4]);   StrAp (fnx, CC(""), 4);;
   if ((StrLn (cmd) + StrLn (DirT) + StrLn (fnx) + 10) >= sizeof (TStr))
      {DBG("found file too long `s", fnx);   return nullptr;}
   StrFmt (fr, "`s/`s",          DirF, fn);
   StrFmt (to, "`s/`s/path.txt", DirT, fnx);
   f.Save (to, fr, StrLn (fr));
   StrFmt (to, "`s/`s/orig`s",   DirT, fnx, ext);   f.Copy (fr, to);
   StrCp (cmd, CC("mid2song "));   StrAp (cmd, to);   App.Run (cmd);
   return nullptr;
}


bool FLstDef::DoDir (char *dir, char *srch)
{ TStr pc, ts, fnx, fnf;
  File f;
  ubyt4 i, ln;
// if pc dir (win expl play song dir), just scoot to it in list
TRC("FL.DoDir `s", dir);
   App.Path (pc, 'd');
   if (! MemCm (dir, pc, StrLn (pc))) {
      for (ln = StrLn (dir), i = 0;  i < FL.lst.Ln;  i++)
         if (! MemCm (dir, FL.lst [i], ln)) {
            FL.pos = i;
            StrAp (dir, CC("/a.song"));     // if just one song, load it upp
            return f.Size (dir) ? true : false;
         }
   }

// else do a real find...
DBG("checkin _midicache.txt");
   StrFmt (fnx, "`s/_midicache.txt", dir);
   if (! f.Size (fnx)) {               // no cache yet so start makin one
      StrCp (pc, CC("ll midi "));   StrAp (pc, dir);   StrAp (pc, CC(" &"));
      App.Run (pc);
      Gui.Hey ("Making cache.  When it's done, find again please.");
      return false;
   }

// turn srch into Col[],NCol
   StrCp (ts, srch ? srch : CC(""));
  ColSep ss (ts, 8);
   for (NCol = 0;  ss.Col [NCol][0];  NCol++)  Col [NCol] = ss.Col [NCol];

// wipe n recreate 4_queue/found
  FDir d;
   StrAp (pc, CC("/4_queue/found"));   d.Kill (pc);   d.Make (pc);

   NFnd = 0;   StrFmt (fnf, "`s.txt", pc);
   if (! FFnd.Open (fnf, "w"))  return false;
   f.DoText (fnx, nullptr, FLFind);   FFnd.Shut ();
DBG("found `d", NFnd);
   if (NFnd == 0)   {Gui.Hey ("none found");   return false;}
   if (NFnd >= 256) {Gui.Hey ("too many files - see 4_queue/found.txt");
                                               return false;}
   StrCp (DirF, dir);   StrCp (DirT, pc);   f.DoText (fnf, nullptr, FLCopy);
                                            f.Kill   (fnf);
   FL.Load ();                         // relist and move sPos to queue/found
   FL.pos = 0;
   for (ln = StrLn (pc), i = 0;  i < FL.lst.Ln;  i++)
      if (! MemCm (pc, FL.lst [i], ln))  {FL.pos = i;   break;}
   return ((FL.pos > 0) ? true : false);
}


bool FLstDef::DoFN (char *fn)          // add single file to fLst
{ TStr pc, ts, fr, to, fnx;
  File f;
  FDir d;
  ubyt4 ln, i;
TRC("FL.DoFN `s", fn);
   ln = StrLn (fn);   *to = '\0';
   App.Path (pc, 'd');
   if ( ((ln > 4) && (! StrCm (& fn [ln-4], CC(".mid")))) ||
        ((ln > 4) && (! StrCm (& fn [ln-4], CC(".kar")))) ||
        ((ln > 4) && (! StrCm (& fn [ln-4], CC(".rmi")))) ) {
      if (MemCm (fn, pc, StrLn (pc))) {
      // .mid not in pc dir?  copy it (mini find)
         StrAp (pc, CC("/4_queue/found"));   d.Kill (pc);
         StrCp (fr, fn);
         StrCp (to, pc);   StrAp (to, CC("/"));   StrAp (to, FnName (ts, fn));
         FnName (fnx, to);   StrAp (to, CC("/"), 4);   StrAp (to, fnx);
         f.Copy (fr, to);
      }
      else
         StrCp (to, fn);               // inside pc, just plain Mid2Song of it
      StrCp (ts, CC("mid2song "));   StrAp (ts, to);   App.Run (ts);
      FL.Load ();
      Fn2Path (to);
   }
   else if ((ln > 8) && (! StrCm (& fn [ln-8], CC("song.txt")))) {
      StrCp (ts, CC("txt2song "));   StrAp (ts, fn);   App.Run (ts);
      FL.Load ();
      StrCp (to, fn);   Fn2Path (to);
   }
   else if ((ln > 5) && (! StrCm (& fn [ln-5], CC(".song"))))
      {StrCp (to, fn);   Fn2Path (to);}
   if (*to) {                          // set FPos
      for (i = 0;  i < FL.lst.Ln;  i++)  if (! StrCm (to, FL.lst [i]))
                                            {FL.pos = i;   break;}
      return true;
   }
   return false;                       // wtf is THAT?
}


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
      d = StrLn (s2);
      if ( (d >= 4) && ((! StrCm (& s2 [d-4], CC(".mid"))) ||
                        (! StrCm (& s2 [d-4], CC(".kar"))) ||
                        (! StrCm (& s2 [d-4], CC(".rmi")))) )
         StrAp (s2, CC(""), 4);
      if ( (d >= 9) && (! StrCm (& s2 [d-9], CC("_song.txt"))) )
         StrAp (s2, CC(""), 4);
      ro [0] = s1;   ro [1] = s2;   ro [2] = nullptr;
      _t.Put (ro);
   }
   _t.Shut ();   _t.HopTo (FL.pos, 0);
   Pik ();
}

void DlgFL::Find ()
{ TStr srch, dir;
   App.CfgGet (CC("DlgFL_dir"), dir);
   if (*dir == '\0')  StrCp (dir, CC("/"));
   if (! Gui.AskDir (dir, "pick top dir to search for songs in (NOT / please)"))
      return;
   App.CfgPut (CC("DlgFL_dir"), dir);
  CtlLine l (ui->srch);
   StrCp (srch, l.Get ());   FL.DoDir (dir, srch);   ReDo ();
}

void DlgFL::Brow ()
{ TStr d, c;   if (system (StrFmt (c, "dolphin `s", App.Path (d, 'd'))))  {}  }

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


//______________________________________________________________________________
void DlgFL::Open ()  {ReDo ();   show ();   raise ();   activateWindow ();}

void DlgFL::Shut ()  
{  FL.pos = _t.CurRow ();   done (true);   lower ();   hide ();  }

void DlgFL::Init ()
{  Gui.DlgLoad (this, "DlgFL", ui->spl);   FL.Load ();
  CtlTBar tb (this,
      "Search\n"   "Search a big midi file dir for matching search strings"
                   "`:/tbar/flst/0" "`\0"   // edit-find
      "Browse\n"   "Open file browser in PianoCheetah directory\n"
                   "(to delete/rename/etc)"
                   "`:/tbar/flst/1" "`\0"   // object-columns
      "Up\n"       "Scoot song up in the list"
                   "`:/tbar/flst/2" "`\0"   // go-up
      "Down\n"     "Scoot song down in the list"
                   "`:/tbar/flst/3" "`\0"   // go-down
   );
   connect (tb.Act (0), & QAction::triggered, this, & DlgFL::Find);
   connect (tb.Act (1), & QAction::triggered, this, & DlgFL::Brow);
   connect (tb.Act (2), & QAction::triggered, this, & DlgFL::Up);
   connect (tb.Act (3), & QAction::triggered, this, & DlgFL::Dn);

   _t.Init (ui->fLst, "Stage\0Song\0");
   connect (ui->fLst, &QTableWidget::itemClicked,       this, & DlgFL::Pik);
   connect (ui->fLst, &QTableWidget::itemDoubleClicked, this, & DlgFL::Shut);
 
  CtlLine s (ui->srch);
  TStr t;
   App.CfgGet (CC("DlgFL_srch"), t);   if (*t) s.Set (t);
 
   connect (ui->all, &QCheckBox::stateChanged, this, & DlgFL::ReDo);
}

void DlgFL::Quit ()
{ CtlLine s (ui->srch);
  TStr t;
   StrCp (t, s.Get ());   App.CfgPut (CC("DlgFL_srch"), t);
   Gui.DlgSave (this, "DlgFL", ui->spl);   
}
