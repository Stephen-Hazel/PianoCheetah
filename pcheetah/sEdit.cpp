// sEdit.cpp - gory edit funcs o Song tied to mouse,dialogs,etc

#include "song.h"

void Song::PreTDr (bool kick)
// load Up.nR/.d[] w song's section cues n drum rhy picks
{ ubyte n, j;
  ubyt4 i, ln, pLn;
  BStr  b;
  TStr  s;
  char *p, *e;
   n = Up.nR = 0;
   for (i = 0;  i < _f.cue.Ln;  i++)  if (_f.cue [i].s [0] == '(') {
      for (j = 0;  j < n;  j++)
         if (! StrCm (& _f.cue [i].s [1], Up.d [j][0]))  break;
      if (j >= n) {
         StrCp (s, CC("(off)"));
         StrCp (Up.d [n][1], s);   StrCp (Up.d [n][2], s);
                                   StrCp (Up.d [n][3], s);
         StrCp (Up.d [n++][0], & _f.cue [i].s [1]);
      }                                // ^ new section
   }
   DscGet (CC("drumpat={"), b);
// search thru song's drumpat desc lines n set pata,patb,fill per section
   ln = 0;  pLn = 0;
   do {
      p = & b [pLn];
      if ((e = StrCh (p, '\n')))  for (i = 0;  i < n;  i++) {
         StrFmt (s, "`s=", Up.d [i][0]);
         if (! MemCm (p, s, StrLn (s))) {
            p += StrLn (s);
            MemCp (s, p, (e-p));   s [e-p] = '\0';
           ColSep ss (s, 3);
            for (j = 0;  j < 3;  j++)  StrCp (Up.d [i][j+1], ss.Col [j]);
         }
      }
   } while ((pLn = LinePos (b, ++ln)));
   Up.nR = n;
   if (kick)  emit sgUpd ("dTDr");
}


void Song::TDr (char *arg)
{ ubyte j, k, t, r, c;
  char *a;
  TStr  s;
  ubyt4 i;
  BStr  b;
  char *p, ud;
  TrkEv *e2;
   NotesOff ();
   a = arg;   r = Str2Int (a, & a);
              c = Str2Int (a, & a);   while (*a == ' ')  a++;
// set new dsc
   PreTDr (false);   StrCp (Up.d [r][c], a);
DBG("TDr r=`d c=`d s=`d", r, c, a);
   StrCp (b, CC("drumpat={\n"));
   for (j = 0;  j < Up.nR;  j++)
      StrAp (b, StrFmt (s, "`s=`s `s `s\n",
             Up.d [j][0], Up.d [j][1], Up.d [j][2], Up.d [j][3]));
   StrAp (b, CC("}"));
   DscPut (b);

// unique sections from _f.cue w time
  TxtRow m [64];
  ubyte nm;
   nm = GetSct (m);

// make a.txt song file with section patterns
  bool  co;
  ubyt2 b1 = 1, br, bb, be;            // bars start at 1 not 0 !
  TStr  fn, pt [3];
  Path  d;
  File  f;
   App.Path (fn, 'd');   StrAp (fn, CC("/4_queue/drumpat"));   d.Make (fn);
   StrAp (fn, CC("/a.txt"));
   if (! f.Open (fn, "w"))  Die (StrFmt (s, "can't write song file `s", fn));
   f.Put (CC("-- drumpat.txt\n"
             "!name=drum\n"));
  TSgRow *ts = TSig (0);
   f.Put (StrFmt (s, "!TSig=`d/`d\n", ts->num, ts->den));

   if (m [0].time) {                   // oops gotta pad in start bars
      b1 = Tm2Bar (m [0].time);
      for (j = 1;  j < b1;  j++)  f.Put (CC("w\n"));
      f.Put (CC("--\n"));
   }                                   // sigh, watch out for weirdness
   if (! StrCm (Up.d [0][1], CC("(continue)")))
      StrCp (   Up.d [0][1], CC("(off)"));
   for (i = 0;  i < nm;) {
   // j = section's pos in tDr[]
      for (j = 0;  j < Up.nR;  j++)
         if (! StrCm (& m [i].s [1], Up.d [j][0])) {
            for (k = 0;  k < 3;  k++) {     // load pattern in pt[]
               if (StrCm (Up.d [j][k+1], CC("(off)")))
                            StrFmt (pt [k], "#drum/`s/`s\n",
                                    (k<2) ? "main" : "fill", Up.d [j][k+1]);
               else if (k)  StrCp  (pt [k], pt [k-1]);
               else         StrCp  (pt [k], CC("w\n"));
         }
         break;
      }
      bb =    Tm2Bar (m [i].time);
      do {
         ++i;
         if (i >= nm)  {be = _bEnd+1;   break;}

         be = Tm2Bar (m [i].time);
         for (j = 0;  j < Up.nR;  j++)
            if (! StrCm (& m [i].s [1], Up.d [j][0])) {
               co = StrCm (Up.d [j][1], CC("(continue)")) ? false : true;
            break;
         }
      }
      while (co);

      for (br = bb;  br < be;  br++)
         f.Put (pt ["abababac" [(br - bb) % 8] - 'a']);
   }
   f.Put (CC("NextTrack\n"));
   f.Shut ();

// Txt2Song it, load it, and replace drumtrack
   App.Run (StrFmt (s, "txt2song `p", fn));
   Fn2Path (fn);   StrAp (fn, CC("/a.song"));
TRC("a");
  STable st [TB_MAX];
   st [TB_DSC].Init (CC("Descrip:"), 1, MAX_DSC);
   st [TB_TRK].Init (CC("Track:")  , 4, MAX_TRK);
   st [TB_DRM].Init (CC("DrumMap:"), 7, MAX_DRM);
   st [TB_LYR].Init (CC("Lyric:")  , 2, MAX_LYR);
   st [TB_EVT].Init (CC("Event:")  , 2, MAX_EVT);
   if ((p = f.DoText (fn, & st, SongRec)))
      {TRC("  DoText err=`s", p);   return;}
TRC("b");

   DrumCon ();
TRC("c");
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TDrm (t))  break;
   if (t >= _f.trk.Ln)  return;
TRC("d");

   for (i = 0;  i < _f.trk [t].ne;)    // keep ctrls / kill notes
      {if (_f.trk [t].e [i].ctrl & 0xFF80)  i++;   else EvDel (t, i);}
TRC("e");

// realloc _f.ev etc
  ubyt4 ne = st [TB_EVT].NRow () - 2;  // omit 1st !TSig=
TRC("f");
  TrkEv *olde = _f.ev;                 // old ev buf
TRC("g");
   _f.maxEv = ne + _f.nEv+MAX_RCRD;    // new size
TRC("h");
   _f.ev = new TrkEv [_f.maxEv];       // new ev buf
   MemCp (_f.ev, olde, _f.nEv * sizeof (TrkEv));
   delete olde;
TRC("i");

// rebuild _f.trk[].e's
   for (_f.trk [0].e = _f.ev, i = 1;  i < _f.trk.Ln;  i++)
      _f.trk [i].e = _f.trk [i-1].e + _f.trk [i-1].ne;
TRC("j");

// EvIns new notes into drumtrack
   e2 = & _f.trk [t].e [_f.trk [t].ne];
TRC("k");
   EvIns (t, _f.trk [t].ne, ne);
TRC("l");
   for (i = 0;  i < ne;  i++, e2++) {
TRC("m");
      e2->time = Str2Tm (st [TB_EVT].Get (1+i, 0));
      StrCp (s,          st [TB_EVT].Get (1+i, 1));
      e2->ctrl = MDrm (s);
TRC("n");
      ud = s [4];
      e2->valu = (ubyte)Str2Int (& s [5]) | ((ud == '_') ? 0x80 : 0);
      e2->val2 = (ud == '~') ? 0x80 : 0;
   }
TRC("o");

// cleanup n TmHop
TRC("p");
   Fn2Path (fn);   d.Kill (fn);
TRC("q");
   if (c == 2)  c = 1;   else if (c == 3)  c = 7;  else c = 0;
   _rcrd = true;   DrumExp ();   ReDo ();   TmHop (m [r].time + M_WHOLE*c);
}


//______________________________________________________________________________
void Song::PreCtl ()
{ ubyte i, j;
  TStr  s;
   Up.nR = 1+_f.ctl.Ln;
   StrCp (Up.d [0][0], CC("chords"));
   StrCp (Up.d [0][1], CC(_lrn.chd ? "yep" : "no"));
   for (i = 0;  i < _f.ctl.Ln;  i++) {
      StrCp (Up.d [i+1][0],    _f.ctl [i].s);
      StrCp (Up.d [i+1][1], CC(_f.ctl [i].sho ? "yep" : "no"));
   }
   StrCp (s, CC("no"));
   for (i = 0;  i < NMCC;  i++) {
      for (j = 0;  j < _f.ctl.Ln;  j++)
         if (! StrCm (_f.ctl [j].s, MCC [i].s))  break;
      if (j >= _f.ctl.Ln) {
         StrCp (Up.d [Up.nR  ][0], MCC [i].s);
         StrCp (Up.d [Up.nR++][1], s);
      }
   }
   emit sgUpd ("dCtl");
}


void Song::Ctl ()
{ ubyte i, ln;
   _lrn.chd = (Up.d [0][1][0] == 'y') ? true : false;
   ln = _f.ctl.Ln;
   for (i = 0;  i < ln;  i++)
      _f.ctl [i].sho = (Up.d [i+1][1][0] == 'y') ? true : false;
   for (i = ln+1;  i < Up.nR;  i++)  if (Up.d [i][1][0] == 'y') {
      _f.ctl.Ln++;
      StrCp (_f.ctl [ln].s, Up.d [i][0]);   _f.ctl [ln++].sho = true;
   }
   ReDo ();
}


void Song::ShoCtl (char *ctl, bool sho)
{  for (ubyte i = 0;  i < _f.ctl.Ln;  i++)
      if (! StrCm (_f.ctl [i].s, ctl))  _f.ctl [i].sho = sho;
}


void Song::SetCtl (char *arg)
{ char *s, *c;
  ubyte tr, cc, mc;
  ubyt4 tm, p;
  TStr  ts;
  MidiEv e;
  TrkEv  ev;
// parse our dang string args: track, epos, time, ctl, val
   tr = (ubyte)Str2Int (arg, & s);   p = (ubyt4)Str2Int (s, & s);
   tm = (ubyt4)Str2Int (s, & s);
   while (*s == ' ')  s++;
   c = s;
   if ((s = StrCh (s, ' ')))  *s++ = '\0';  else s = c;
   while (*s == ' ')  s++;
TRC("setCtl tr=`d p=`d tm=`d ctl=`s val=`s", tr, p, tm, c, s);
   if (tr < 128) {
      EvDel (tr, p);
      if (! StrCm (c, CC("KILL")))
         {_prac = true;   Pract ();   ReDo ();   return;}
   }
   else {                              // need SOME track to put it in...
      if ( (! StrCm (c, CC("ksig"))) ||     // these HAVE to go in drum trak
           (! StrCm (c, CC("tsig"))) || (! StrCm (c, CC("tmpo"))) ) {
         for (tr = 0;  tr < _f.trk.Ln;  tr++)  if (TDrm (tr))  break;
         if (tr >= _f.trk.Ln)
            {Hey (CC("no drum track to put control into"));   return;}
      }
      else {                           // find a learn track
         for (tr = 0;  tr < _f.trk.Ln;  tr++)  if (TLrn (tr))  break;
         if (tr >= _f.trk.Ln)
            {Hey (CC("no learn track to put control into"));   return;}
      }
   }
// scoot tsig,ksig time to bar start
   if ( (! StrCm (c, CC("tsig"))) || (! StrCm (c, CC("tsig"))) )
      tm = Bar2Tm (Tm2Bar (tm));
   for (mc = 0;  mc < NMCC;  mc++)  if (! StrCm (c, MCC [mc].s))  break;
   if (MCC [mc].typ == 'x')  {CtlX2Val (& ev, c, s);
                              e.valu = ev.valu;              e.val2 = ev.val2;}
   else                      {e.valu = (ubyte)Str2Int (s);   e.val2 = 0;}
   for (cc = 0;  cc < _f.ctl.Ln;  cc++)  if (! StrCm (c, _f.ctl [cc].s))  break;
TRC("set time=`s tr=`d ctrl=`d valu=`d val2=`d",
TmSt (ts, tm), tr+1, cc, e.valu, e.val2);
   e.time = tm;   e.ctrl = 0x80|cc;
   EvInsT (tr, & e);
   if (MCC [mc].typ == 'x')
   _prac = true;   Pract ();   ReDo ();
}


//______________________________________________________________________________
void Song::Cue (char *s)
{ TStr s1;
DBG("Cue '`s'", s);
   if (! StrCm (s, CC("loopInit")))  return LoopInit ();
   if (*s == '[')  return;             // can't del loops

  ubyt4 tm = Up.pos.tm, te = 0;
   if (Up.pos.got)
      {tm = _f.cue [Up.pos.p].time;
       te = _f.cue [Up.pos.p].tend;  _f.cue.Del (Up.pos.p);}
   if (*s) {
      StrCp (s1, s);
      if (StrCh (CC("<>"), *s)) {      // need dur?
         if (! Up.pos.got)  te = tm + M_WHOLE;
         StrFmt (s1, "/`d`s", te-tm, s);
      }
      if (*s == '(')                   // chop tm at bar for sections
         tm = Bar2Tm (Tm2Bar (tm));
      TxtIns (tm, s1, & _f.cue, 'c');
   }
   ReDo ();
}


//______________________________________________________________________________
void Song::PreFng ()
{
DBG("PreFng");
  PagDef *pg = & _pag [Up.pos.pg];
  ColDef  co;
   MemCp (& co, & pg->col [Up.pos.co], sizeof (co)); // load column
  SymDef *it = & co.sym [Up.pos.sy];
  ubyte   t  = it->tr;
  TrkNt *nt = & _f.trk [t].n [it->nt];
  TStr   st, s1, s2;
   if (nt->dn == NONE)  StrCp (st, CC("ntDn: NONE\n"));
   else {
      TmSt (s1, _f.trk [t].e [nt->dn].time);
      StrFmt (st,                "ntDn: `s velo=`d\n",
                s1, _f.trk [t].e [nt->dn].valu & 0x7F);
   }
   if (nt->up == NONE)  StrAp (st, CC("ntUp: NONE"));
   else {
      TmSt (s2, _f.trk [t].e [nt->up].time);
      StrFmt (& st [StrLn (st)], "ntUp: `s velo=`d",
                s2, _f.trk [t].e [nt->up].valu & 0x7F);
   }
// pass st, t, it->nt;  dlg sends on t,nt,fng
   StrCp (Up.pos.str, st);   Up.pos.tr = t;   Up.pos.p = it->nt;
   emit sgUpd ("dFng");
}


void Song::Fng (char *tnf)           // set fingering, handswap, or del
{ ubyte t, tc;
  ubyt4 n, p, ne;
  ubyte f;
  char *s, h;
  bool  ok;
  TrkNt *nt;
  TrkEv  dn, up, *e;
DBG("Fng `s", tnf);
   t  = (ubyte) Str2Int (tnf, & s);
   n  =         Str2Int (s,   & s);
   f  = (ubyte) Str2Int (s);
DBG("Fng `d `d `d", t, n, f);
//Dump (true);
   nt = & _f.trk [t].n [n];
   if (nt->dn != NONE)  MemCp (& dn, & _f.trk [t].e [nt->dn], sizeof (dn));
   if (nt->up != NONE)  MemCp (& up, & _f.trk [t].e [nt->up], sizeof (up));
DBG("t=`d n=`d/`d f=`d dn=`d up=`d", t, n, _f.trk [t].nn, f, nt->dn, nt->up);
   if (nt->dn == NONE)
      {MemCp (& dn, & up, sizeof (dn));   dn.time = (up.time >(M_WHOLE/32-1)) ?
                                                    (up.time -(M_WHOLE/32-1)):0;
                                          dn.valu = 100;
TRC("made dn");
                                          }
   if (nt->up == NONE)
      {MemCp (& up, & dn, sizeof (dn));   up.time = dn.time + (M_WHOLE/32-1);
                                          up.valu = 64;
TRC("made up");
                                          }
   if (f == 99) {
      for (t = 0;  t < _f.trk.Ln;  t++)
         for (e = _f.trk [t].e, ne = _f.trk [t].ne, p = 0;  p < ne;  p++)
            if (! (e [p].ctrl & 0x80))  e [p].val2 = e [p].val2 & 0x80;
      ReDo ();
      return;
   }
   if (f == 97) {                      // gotta find dst trk for hand swap
      for (ok = false, h = _f.trk [t].ht, tc = 0;  tc < _f.trk.Ln;  tc++) {
         if ((h == 'R') && (_f.trk [tc].ht == 'L'))  {ok = true;   break;}
         if ((h == 'L') && (_f.trk [tc].ht == 'R'))  {ok = true;   break;}
      }
      if (! ok)  {Hey (CC("can't find track for other hand"));   return;}
TRC("dst trk=`d", tc);
   }
   if (f >= 97) {                      // delete or swap track
      if (nt->up != NONE)  EvDel (t, nt->up);    // del up 1st so dn stays
      if (nt->dn != NONE)  EvDel (t, nt->dn);    // in same pos
      if (f == 98)  {ReDo ();   return;}         // that's it for del
//TODO wtf is tc ?? (dbg to remove compiler warning)
      e = _f.trk [tc].e;   ne = _f.trk [tc].ne;
      for (p = 0;  (p < ne) && (dn.time >= e [p].time);  p++)  ;
//TStr d1;
//DBG("dn ins at p=`d/`d tm=`s", p, ne, TmSt(d1,dn.time));
      EvIns (tc, p);   MemCp (& e [p], & dn, sizeof (dn));

      e = _f.trk [tc].e;   ne = _f.trk [tc].ne;  // reset after 1st ins :/
      for (;       (p < ne) && (up.time >= e [p].time);  p++)  ;
//DBG("up ins at p=`d/`d tm=`s", p, ne, TmSt(d1,up.time));
      EvIns (tc, p);   MemCp (& e [p], & up, sizeof (up));
//Dump (true);
   }
   else {                              // fng update (but gotta ins if dn broke)
      if ((p = nt->dn) == NONE) {
         e = _f.trk [t].e;   ne = _f.trk [t].ne;
         for (p = 0;  (p < ne) && (dn.time >= e [p].time);  p++)  ;
         EvIns (t, p);   MemCp (& e [p], & dn, sizeof (dn));
      }
      _f.trk [t].e [p].val2 &= 0xC0;
DBG("fng=`d", f);
      _f.trk [t].e [p].val2 |= f;
   }
   ReDo ();
}


void Song::NtDur ()                    // set note duration (end time)
{ char *s;
  ubyt4 ap, ac, as, tm, t1, tMx, p, ne;
  sbyt2 y2;
  ubyte t, nt;
  SymDef *sy;
  TrkEv  *e, up;
   ap =  Up.pos.pg;   ac = Up.pos.co;   as = Up.pos.sy;   y2 = Up.pos.y2;
  PagDef *pg = & _pag [ap];
  ColDef  co;
   MemCp (& co, & pg->col [ac], sizeof (co));
   sy = & co.sym [as];
   t = sy->tr;
//DBG("pg=`d co=`d sy=`d y2=`d", ap, ac, as, y2);
   if (y2 <= sy->y)  return;           // just quit if dur is lame

   tm = Y2Tm (y2, & co);               // y=> time
   nt = _f.trk [t].n [sy->nt].nt;
   t1 = _f.trk [t].n [sy->nt].tm;
//TStr d1,d2,d3;
//DBG("tm=`s nt=`s t1=`s", TmSt(d1,tm), MKey2Str (d2,nt), TmSt(d3,t1));
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  p < ne;  p++)          // find next ntDn of nt to limit dur
      if ((e [p].ctrl == nt ) &&    (e [p].time > t1) &&
          (e [p].valu & 0x80) && (! (e [p].val2 & 0x80)))  break;
   tMx = (p < ne) ? e [p].time : (t1 + 4*M_WHOLE);
//DBG("tMx=`s", TmSt(d1,tMx));
   if (tm >= tMx)  tm = tMx-1;

// ok, let's do this thing...
   if ((p = _f.trk [t].n [sy->nt].up) != NONE) {
      MemCp (& up, & _f.trk [t].e [p], sizeof (up));
      EvDel (t, p);
   }
   else {
      MemCp (& up, & _f.trk [t].e [_f.trk [t].n [sy->nt].dn], sizeof (up));
      up.valu = 64;
   }
   up.time = tm;
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  (p < ne) && (e [p].time <= up.time);  p++)  ;
   EvIns (t, p);   MemCp (& e [p], & up, sizeof (up));
   ReDo ();
}


void Song::NtHop ()                    // move note time,dur to new key
{ ubyt4 ap, ac, as, p, ne;
  sbyt2 x2;
  ubyt2 nx;
  ubyte t, dnt;
  SymDef *sy;
  TrkNt  *n;
  TrkEv  *e, dn, up;
  TStr    s1, s2, s3;
   ap = Up.pos.pg;   ac = Up.pos.co;
   as = Up.pos.sy;   x2 = Up.pos.x1 + W_NT/2;
  PagDef *pg = & _pag [ap];
  ColDef  co;
   MemCp (& co, & pg->col [ac], sizeof (co));
   sy = & co.sym [as];   t = sy->tr;   n = & _f.trk [t].n [sy->nt];
   e = _f.trk [t].e;

DBG("NtHop pg=`d co=`d sy=`d x2=`d tr=`d", ap, ac, as, x2, t);
   nx = Nt2X (co.nMn, & co);
   if ((x2 < nx) || (x2 >= CtlX (& co)))  return;     // lame dest nt pos

// quit if overlap
   dnt = co.nMn + (x2 - nx) / W_NT;    // note to move it to
   if ((p = NtHit (t, n->tm, n->te, dnt, dnt))) {
      StrFmt (s1, "can't move - overlap at note=`s time=`s",
         MKey2Str (s2, dnt), TmSt (s3, e [p-1].time));
      Hey (s1);
      return;
   }

// ok, let's do this thing...  set dn,up; kill old; ins new
   if ((p = n->up) != NONE)  MemCp (& up, & e [p],     sizeof (up));
   else                     {MemCp (& up, & e [n->dn], sizeof (up));
                             up.time += (M_WHOLE/32-1);   up.valu = 64;}
   if ((p = n->dn) != NONE)  MemCp (& dn, & e [p],     sizeof (dn));
   else                     {MemCp (& dn, & e [n->up], sizeof (dn));
                             dn.time -= (M_WHOLE/32-1);   dn.valu = 100;}
   if ((p = n->up) != NONE)  EvDel (t, p);
   if ((p = n->dn) != NONE)  EvDel (t, p);
   dn.ctrl = up.ctrl = dnt;

   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  (p < ne) && (e [p].time <= dn.time);  p++)  ;
   EvIns (t, p);   MemCp (& e [p], & dn, sizeof (dn));

   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  (p < ne) && (e [p].time <= up.time);  p++)  ;
   EvIns (t, p);   MemCp (& e [p], & up, sizeof (up));

   ReDo ();
}


//______________________________________________________________________________
typedef struct {ubyte t;   ubyt4 p;} TPDef;

int TPCmp (void *a1, void *a2)         // by t,p desc
{ TPDef *p1 = (TPDef *)a1, *p2 = (TPDef *)a2;
  int t;
   if ((t = p2->t - p1->t))   return t;
        t = p2->p - p1->p;    return t;
}

void Song::Mov ()
// move rect of notes to RH,LH,bg,kill
// if nondrag, insert a new note
{ char *s, *c;
  ubyt4 ap, ac, p, p1, tm, t1, t2, tMx, dBt, dSb, ne, nn, i, nDel, nIns;
  sbyt2 x1, y1, x2, y2, tp;
  ubyt2 nx, br;
  ubyte tr, tr1, tD, tR, tL, vDn, vUp, nt, nSb, bt;
  TStr  s1;
  SymDef *it;
  TrkEv  *e, ev, *ins, up, dn;
  TPDef          *del;
  TrkNt  *n;
   ap = Up.pos.pg;   ac = Up.pos.co;   x1 = Up.pos.x1;   y1 = Up.pos.y1;
                                       x2 = Up.pos.x2;   y2 = Up.pos.y2;
   s = Up.pos.str;
DBG("Mov ap=`d ac=`d x1=`d y1=`d x2=`d y2=`d s='`s'",
ap, ac, x1, y1, x2, y2, s);
  PagDef *pg = & _pag [ap];
  ColDef  co;
   MemCp (& co, & pg->col [ac], sizeof (co));
   nx = Nt2X (co.nMn, & co);
   if (*s == '\0') {                   // just gonna ins a note
      if (x1 >= Nt2X (co.nMx+1, & co))
         {Hey (CC("sorry, can't do drums yet"));   return;}

      tm = Y2Tm (y1, & co);            // y=> time=> round to subbeat
      TmStr (s1, tm, & t2, & nSb);   br = (ubyt2)Str2Int (s1, & c);
                                     bt = (ubyte)Str2Int (c+1);
      t1 = Bar2Tm (br, bt);   dBt = t2 - t1;   dSb = dBt / nSb;
      t1 += (((tm - t1) / dSb) * dSb);           // t1 now has trunc'd subbt
      if ((tm - t1) >= (dSb / 2))  t1 += dSb;    // now rounded subbt

      nt = co.nMn + (x1 - nx) / W_NT;  // note is easy

   // find nearest (unbroke) note on any ? trk (by time, then by frq)
   // use THAT dude's trk, dur, n velos for ins
      p1 = NONE;   tr1 = 0;
      for (tr = 0;  tr < Up.rTrk;  tr++)  if (TLrn (tr))
         for (n = _f.trk [tr].n, nn = _f.trk [tr].nn, p = 0;  p < nn;  p++)
                                 if ((n [p].dn != NONE) && (n [p].up != NONE)) {
//DBG("p=`d/`d  t1=`d nt=`d  tmDn=`d tmUp=`d nt=`d  p1=`d",
//p, nn,  t1, nt,  e [n [p].dn].time, e [n [p].up].time, n [p].nt,  p1);
            if      (p1 == NONE)
                  {p1 = p;   tr1 = tr;}
            else if (ABSL((sbyt4)n [p].tm - (sbyt4)t1) <
                                ABSL((sbyt4)_f.trk [tr1].n [p1].tm - (sbyt4)t1))
                  {p1 = p;   tr1 = tr;}
            else if (ABSL((sbyt4)n [p].tm - (sbyt4)t1) ==
                                ABSL((sbyt4)_f.trk [tr1].n [p1].tm - (sbyt4)t1))
               {if  (ABSL((sbyt4)n [p].nt - (sbyt4)nt) <=
                                ABSL((sbyt4)_f.trk [tr1].n [p1].nt - (sbyt4)nt))
                  {p1 = p;   tr1 = tr;}}
            else if (n [p].tm > t1)  break;      // past min
         }
      if (p1 != NONE) {
         vDn =     _f.trk [tr1].e [_f.trk [tr1].n [p1].dn].valu & 0x7F;
         vUp =     _f.trk [tr1].e [_f.trk [tr1].n [p1].up].valu & 0x7F;
         t2 = t1 + _f.trk [tr1].e [_f.trk [tr1].n [p1].up].time -
                   _f.trk [tr1].e [_f.trk [tr1].n [p1].dn].time;
//DBG("GOT p1=`d tmDn=`d tmUp=`d  t1=`d t2=`d vDn=`d vUp=`d",
//p1, e [n [p1].dn].time, e [n [p].up].time,  t1, t2, vDn, vUp);
      }
      else {                           // default trk, velos, n dur
         vDn = 100;   vUp = 64;   t2 = t1 + dBt - 1;
         for (tr = 0;  tr < Up.rTrk;  tr++)  if (TLrn (tr))  break;
         if (tr >= Up.rTrk)
            {Hey (CC("you need a practice track to add notes on"));   return;}
         tr1 = tr;
      }

   // limit t2 at next ntDn time on same nt
      e = _f.trk [tr1].e;   ne = _f.trk [tr1].ne;
      for (p = 0;  p < ne;  p++)         // find next ntDn of nt to limit dur
         if ((e [p].ctrl == nt ) &&    (e [p].time > t1) &&
             (e [p].valu & 0x80) && (! (e [p].val2 & 0x80)))  break;
      tMx = (p < ne) ? e [p].time : (t1 + 4*M_WHOLE);
      if (t2 >= tMx)  t2 = tMx-1;

   // pop in our dn,up evs
      ev.time = t1;   ev.ctrl = nt;   ev.valu = vDn | 0x80;   ev.val2 = 0;
      for (p = 0;  (p < ne) && (ev.time >= e [p].time);  p++)  ;
      EvIns (tr1, p);   MemCp (& e [p], & ev, sizeof (ev));

      ev.time = t2;                   ev.valu = vUp;
      e = _f.trk [tr1].e;   ne = _f.trk [tr1].ne;
      for (p = 0;  (p < ne) && (ev.time >= e [p].time);  p++)  ;
      EvIns (tr1, p);   MemCp (& e [p], & ev, sizeof (ev));

      ReDo ();
      return;
   }

// ok, hunt em down n kill/move each dude
   if (x2 < x1)  {tp = x1;   x1 = x2;   x2 = tp;}
   if (y2 < y1)  {tp = y1;   y1 = y2;   y2 = tp;}
  ubyt2 dx = Nt2X (co.nMx+1, & co);
   if (x1 >= dx)  {Hey (CC("sorry, can't do drums yet :("));   return;}

   if (x2 >= dx)  x2 = dx-1;
   if (*s != 'x') {                    // find dest track if not doin del
     bool got [3];
      MemSet (got, 0, sizeof (got));
      for (i = 0;  i < co.nSym;  i++)
         if ((((nx+co.sym [i].x                    >= x1) &&
               (nx+co.sym [i].x                    <= x2)) ||
              ((nx+co.sym [i].x + co.sym [i].w - 1 >= x1) &&
               (nx+co.sym [i].x + co.sym [i].w - 1 <= x2))) &&
             (((   co.sym [i].y                    >= y1) &&
               (   co.sym [i].y                    <= y2)) ||
              ((   co.sym [i].y + co.sym [i].h - 1 >= y1) &&
               (   co.sym [i].y + co.sym [i].h - 1 <= y2))))
            switch (_f.trk [co.sym [i].tr].ht) {
               case 'R': got [0] = true;   break;
               case 'L': got [1] = true;   break;
               case 'S': got [2] = true;   break;
            }
      tR = tL = 255;
      for (tr = 0;  tr < Up.rTrk;  tr++) {
         if ((tR == 255) && (_f.trk [tr].ht == 'R')) tR = tr;
         if ((tL == 255) && (_f.trk [tr].ht == 'L')) tL = tr;
      }
      if (*s != '#') {
         if ((tR == 255) || (tL == 255))
            {Hey (CC("you need to have RH and LH tracks"));   return;}
      }
      else {
         if ( got [0] && ((tR == 255) || ((ubyt4)(tR+1) >= Up.rTrk) ||
                          (! _f.trk [tR+1].grp) || TLrn (tR+1)) )
            {Hey (CC("add a RH backing track (+ track) to move notes to"));
             return;}
         if ( got [1] && ((tL == 255) || ((ubyt4)(tL+1) >= Up.eTrk) ||
                          (! _f.trk [tL+1].grp) || TLrn (tL+1)) )
            {Hey (CC("add a LH backing track (+ track) to move notes to"));
             return;}
         tR++;   tL++;
      }
   }
   ins = new TrkEv [co.nSym * 2];   del = new TPDef [co.nSym * 2];
   nIns = nDel = 0;                    // may not need ins, but whatever..
   for (i = 0;  i < co.nSym;  i++)
      if ((((nx+co.sym [i].x                    >= x1) &&
            (nx+co.sym [i].x                    <= x2)) ||
           ((nx+co.sym [i].x + co.sym [i].w - 1 >= x1) &&
            (nx+co.sym [i].x + co.sym [i].w - 1 <= x2))) &&
          (((   co.sym [i].y                    >= y1) &&
            (   co.sym [i].y                    <= y2)) ||
           ((   co.sym [i].y + co.sym [i].h - 1 >= y1) &&
            (   co.sym [i].y + co.sym [i].h - 1 <= y2)))) {
         it = & co.sym [i];
      // already ok?
         if ( ((*s == '#') && (_f.trk [it->tr].ht == 'S')) ||
              ((*s == '>') && (it->tr == tR)) ||
              ((*s == '<') && (it->tr == tL)) )  continue;
      // first, we kill
         if ((p = _f.trk [it->tr].n [it->nt].dn) != NONE)
            {del [nDel].t = it->tr;   del [nDel].p = p;   nDel++;}
         if ((p = _f.trk [it->tr].n [it->nt].up) != NONE)
            {del [nDel].t = it->tr;   del [nDel].p = p;   nDel++;}

      // next, gotta ins em incl making fake 2nd (unless doin del)
         if (*s != 'x') {
//TStr db1, db2;
//DBG("move `s `s from tr=`d to tR=`d tL=`d",
//TmSt    (db1,_f.trk[it->tr].n [it->nt].tm),
//MKey2Str(db2,_f.trk[it->tr].n [it->nt].nt), it->tr, tR, tL);
            if ((p = _f.trk [it->tr].n [it->nt].dn) != NONE)
               MemCp (& dn, & _f.trk [it->tr].e [p], sizeof (dn));
            else {
               MemCp (& dn, & up, sizeof (up));
               dn.time = (up.time > (M_WHOLE/32-1)) ?
                         (up.time - (M_WHOLE/32-1)) : 0;
               dn.valu = 100;
            }
            if ((p = _f.trk [it->tr].n [it->nt].up) != NONE)
               MemCp (& up, & _f.trk [it->tr].e [p], sizeof (up));
            else {
               MemCp (& up, & dn, sizeof (dn));
               up.time =  dn.time + (M_WHOLE/32-1);
               up.valu = 64;
            }
            if      (*s == '>')  dn.x = up.x = tR;
            else if (*s == '<')  dn.x = up.x = tL;
            else if (_f.trk [it->tr].ht == 'R')  dn.x = up.x = tR;
            else                               dn.x = up.x = tL;
            MemCp (& ins [nIns++], & dn, sizeof (dn));
            MemCp (& ins [nIns++], & up, sizeof (up));
         }
      }
   Sort (del, nDel, sizeof (TPDef), TPCmp);      // by tr,p desc so this works:
   for (i = 0;  i < nDel;  i++)  EvDel (del [i].t, del [i].p);
   for (i = 0;  i < nIns;  i++) {
      tD = ins [i].x;   e = _f.trk [tD].e;   ne = _f.trk [tD].ne;
      ins [i].x = 0;
      for (p = 0;  (p < ne) && (e [p].time <= ins [i].time);  p++)  ;
      EvIns (tD, p);
      MemCp (& e [p], & ins [i], sizeof (TrkEv));
   }
   delete [] ins;   delete [] del;
   ReDo ();
}
