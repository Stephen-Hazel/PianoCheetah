// sFile.cpp - File I/O and supporting funcs of Song

#include "song.h"


bool Song::DscGet (char *key, char *val)    // key is whatev= or whatev={
// messin w _f.dsc
{ char *p, *e;
   *val = '\0';
   if (! (p = StrSt (_f.dsc, key)))    return false;
   if (key [StrLn (key)-1] == '{') {
      if (! (e = StrSt (p, CC("}\n"))))  return false;
      p += StrLn (key)+1;              // skip \n too
   }
   else {
      if (! (e = StrSt (p, CC("\n"))))   return false;
      p += StrLn (key);
   }
   MemCp (val, p, e-p);   val [e-p] = '\0';
   return true;
}


void Song::DscPut (char *repl)
// update a line of _f.dsc with a name=value thingy
{ char *e, *p, *p2;
  ubyt4 ofs;
  TStr  tag;
//DBG("DscPut repl=`s _f.dsc=...\n`s", repl, _f.dsc);
   e = StrCh (repl, '=');
   MemCp (tag, repl, e-repl+1);   tag [e-repl+1] = '\0';
   if ((p = StrSt (_f.dsc, tag))) {    // get n find 'tag='  replace btw end p2
      ofs = p - _f.dsc;
   // point p2 to char right after what we're replacin (usu on a \n)
      if (! MemCm (& p [StrLn (tag)], CC("{\n"), 2)) {  // multi line value
         if ((p2 = StrSt (& _f.dsc [ofs], CC("}\n"))))  p2++;
         else  p2 = p + StrLn (tag) + 1;      // broke so try to recover :(
      }                                               // single line value
      else {
         if ((p2 = StrSt (& _f.dsc [ofs], CC("\n"))))  ;
         else  {p2 = & _f.dsc [StrLn (_f.dsc)] - 2;   MemCp (p2, CC("\n"), 1);}
      }
      if (StrLn (_f.dsc) + StrLn (repl) - (p2-p) + 1 > (sbyt4)sizeof (_f.dsc))
         DBG("DscPut  upd - outa room `s", repl);
      StrCp (p + StrLn (repl), p2);   MemCp (p, repl, StrLn (repl));
   }
   else {                              // it's new so just append w \n\0
      if (StrLn (_f.dsc) + StrLn (repl) + 3 > sizeof (_f.dsc))
         DBG("DscPut  ins - outa room `s", repl);
      StrAp (_f.dsc, repl);   StrAp (_f.dsc, CC("\n"));
   }
//DBG("DscPut:  _f.dscOut=...\n`s", _f.dsc);
}


void Song::Pract ()
// update _f.dsc w pract line for today:  pract yyyymm .. dd
{ char *p;                             // (too tricky for DscPut)
  TStr  day, mo, chk;
  ubyt4 ln, pLn;
   Now (day);   StrCp (mo, day);   mo [6] = '\0';
                day [5] = ' ';   day [8] = '\0';      // use & day [5] for " dd"
   StrFmt (chk, "pract `s", mo);
   if ((p = StrSt (_f.dsc, chk))) {
   // if got pract yyyymm and it doesn't end w dd, stick (spc)dd in
      if ( (p = StrSt (p, CC("\n"))) && MemCm (p-2, & day [6], 2) ) {
         if (StrLn (_f.dsc)+4 > sizeof (_f.dsc))
            DBG("Pract  _f.dsc outa room");
         StrCp (p+3, p);   MemCp (p, & day [5], 3);
      }
   }
   else {
   // insert new pract yyyymm dd\n
      StrAp (chk, & day [5]);   StrAp (chk, CC("\n"));
   // search through lines (split on \n) looking for pract* and notes*
      ln = 0;   pLn = 0;
      do
         if (! MemCm (& _f.dsc [pLn], CC("pract "), 6))  break;
      while ((pLn = LinePos (_f.dsc, ++ln)));
      if (StrLn (_f.dsc) + StrLn (chk) + 1 > sizeof (_f.dsc))
         DBG("Pract  _f.dsc outa room");
   // right before 1st pract* line;  else ins at start
      StrCp (& _f.dsc [pLn + StrLn (chk)], & _f.dsc [pLn]);
      MemCp (& _f.dsc [pLn], chk, StrLn (chk));
   }
}


void Song::DscInit ()                  // init w defaults
{  _f.tmpo = FIX1;   _f.tran = 0;   Up.lrn = LHEAR;   _f.ezHop = false;  }


void Song::DscLoad ()                  // parse junk outa _dsc plus info={...}
{ sbyt4 i;
  TStr  s;
   DscInit ();
   if (DscGet (CC("tempo="), s))      _f.tmpo = (ubyt4)Str2Int (s);
   if (DscGet (CC("transpose="), s)) {
      i = Str2Int (s);
      if ((i >= -36) && (i <= 36))    _f.tran = (sbyte)i;
   }
   if (DscGet (CC("ezhop="), s))      _f.ezHop = StrCm (s, CC("n"))
                                                                 ? true : false;
   if (DscGet (CC("learn="), s))      Up.lrn   = (ubyte)Str2Int (s);
   Cfg.tran = _f.tran;   Cfg.ezHop = _f.ezHop;   // ...sigh
TRC("DscLoad  tmpo=`d, tran=`d lrn=`d", _f.tmpo, _f.tran, Up.lrn);
}


void Song::DscSave ()                  // put junk back in _dsc
{ ubyte i;
  TStr  pr, ks, ts, tp, s;
  BStr  buf;
   StrFmt (buf, "tempo=`d",     _f.tmpo);                DscPut (buf);
   StrFmt (buf, "transpose=`d", _f.tran);                DscPut (buf);
   StrFmt (buf, "ezhop=`s",     _f.ezHop ? "y" : "n");   DscPut (buf);
   StrFmt (buf, "learn=`d",     Up.lrn);                 DscPut (buf);

// ok, make our summary thingy
// prac   RH LH HT DRUM REC
   *pr = '\0';
   for (i = 0;  i < Up.rTrk;  i++)
      if ((! TDrm (i)) && (_f.trk [i].ht == 'L'))
         {StrAp (pr, CC("LH "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if ((! TDrm (i)) && (_f.trk [i].ht == 'R'))
         {StrAp (pr, CC("RH "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if  (TDrm (i))
         {StrAp (pr, CC("Drum "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if (TLrn (i) && (! TDrm (i)) && (_f.trk [i].ht == 'L'))
         {StrAp (pr, CC("REC_LH "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if (TLrn (i) && (! TDrm (i)) && (_f.trk [i].ht == 'R'))
         {StrAp (pr, CC("REC_RH "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if (TLrn (i) && (! TDrm (i)) && (_f.trk [i].ht != 'R') &&
                                      (_f.trk [i].ht != 'L'))
         {StrAp (pr, CC("REC_HT "));   break;}
   for (i = 0;  i < Up.rTrk;  i++)
      if (TLrn (i) && TDrm (i))
         {StrAp (pr, CC("REC_DRUM "));   break;}
   if (*pr)
      {StrCp (& pr [7], pr);   MemCp (pr, CC("prac   "), 7);
                               StrAp (pr, CC("\n"));}
// ksig   ...
   StrCp (ks, CC("ksig   "));
   for (i = 0;  (i < _f.kSg.Ln) && (i < 3);  i++) {
      if    (! _f.kSg [i].flt)        StrCp (s, MKeyStr  [_f.kSg [i].key]);
      else if (_f.kSg [i].key != 11)  StrCp (s, MKeyStrB [_f.kSg [i].key]);
      else                            StrCp (s, CC("Cb"));      // B/Cb iz WEIRD
      if (_f.kSg [i].min)  StrAp (s, CC("m"));
      *s = CHUP (*s);
      StrAp (ks, s);   StrAp (ks, CC(" "));
   }
   if (_f.kSg.Ln > 3)  StrFmt (& ks [StrLn (ks)], "+`d", _f.kSg.Ln - 3);
   if (_f.kSg.Ln == 0) *ks = '\0';   else StrAp (ks, CC("\n"));

// tsig   ...
   StrCp (ts, CC("tsig   "));
   for (i = 0;  (i < _f.tSg.Ln) && (i < 3); i++) {
      if (_f.tSg [i].sub > 1)
            StrFmt (s, "`d/`d/`d ", _f.tSg [i].num, _f.tSg [i].den,
                                                    _f.tSg [i].sub);
      else  StrFmt (s, "`d/`d ",    _f.tSg [i].num, _f.tSg [i].den);
      StrAp (ts, s);
   }
   if (_f.tSg.Ln > 3)  StrFmt (& ts [StrLn (ts)], "+`d", _f.tSg.Ln - 3);
   if (_f.tSg.Ln == 0) *ts = '\0';   else StrAp (ts, CC("\n"));

// tempo  ...
   StrCp (tp, CC("tempo  "));
   for (i = 0;  (i < _f.tpo.Ln) && (i < 3);  i++)
      StrFmt (& tp [StrLn (tp)], "`d ", _f.tpo [i].val);
   if (_f.tpo.Ln > 3)  StrFmt (& tp [StrLn (tp)], "+`d", _f.tpo.Ln - 3);
   if (_f.tpo.Ln == 0) *tp = '\0';   else StrAp (tp, CC("\n"));
/* loops  23  bug'd=13
   fing   29 / ez
   cue    13 verse=2 chorus=3
   synth  syn rockin88 drum +2
          keys bass guit melo drum   keys: piano organ chromperc synlead
   velo      2   23    0  127    1
   note      '    '    '    '    '
   ctrl      '    '    '
   track     '    '    '
   sound     '    '    '
*/
//
   StrFmt (buf,
      "info={\n"
      "bars   `d\n"
      "`s`s`s`s"
      "lyric  `d\n"
      "}",
      _bEnd, pr, ks, ts, tp, _f.lyr.Ln
   );
   DscPut (buf);
}


//------------------------------------------------------------------------------
// messin w drums n ev sortin

int EvCmp (void *p1, void *p2)         // TrkEv sortin by time,ctrl,up/prs/dn
{ TrkEv *e1 = (TrkEv *)p1, *e2 = (TrkEv *)p2;
  int t;
   if ((t = e1->time - e2->time))  return t;
   if ((t = e1->ctrl - e2->ctrl))  return t;
   if ((t = (e1->valu & 0x80) - (e2->valu & 0x80)))  return t;  // up / pr,dn
   return   (e2->val2 & 0x80) - (e1->val2 & 0x80);              // pr / dn
}

int DTrCmp (void *p1, void *p2)  // ...TrkEv sortin for DrumExp() by DrumNt,
{ TrkEv *e1 = (TrkEv *)p1, *e2 = (TrkEv *)p2;                     // time,etc
  ubyte  c1 = 0, c2 = 0;               // non notes sort w 1st dr nt
  int t;
   if (! (e1->ctrl & 0x80)) {
      for (c1 = 0; c1 < NMDrum; c1++)  if (e1->ctrl == MKey (MDrum [c1].key))
         break;
      if (c1 >= NMDrum)  c1 += e1->ctrl;
      c1++;
   }
   if (! (e2->ctrl & 0x80)) {
      for (c2 = 0; c2 < NMDrum; c2++)  if (e2->ctrl == MKey (MDrum [c2].key))
         break;
      if (c2 >= NMDrum)  c2 += e2->ctrl;
      c2++;
   }
   if ((t = c1       - c2      ))  return t;               // drum nt;ctl=0
   if ((t = e1->time - e2->time))  return t;
   if ((t = (e1->valu & 0x80) - (e2->valu & 0x80)))  return t;
   return   (e2->val2 & 0x80) - (e1->val2 & 0x80);
}


void Song::DrumExp (bool setBnk)
// expand any (per dev) DrumTrack tracks to a track per dev/drum
// use _f.mapD (already set up) to override default drum params
{ ubyte t, nD, d [128], x, i;
  ubyt4 e, neX, neT;
  TStr  s, s2;
  TrkEv *ev;
TRC("DrumExp rTrk=`d setBnk=`b", Up.rTrk, setBnk);
   for (t = 0;  t < Up.rTrk;) {
TRC(" t=`d", t);
      if (! TDrm (t))  t++;
      else {
TRC("  got dr trk - ne=`d", _f.trk [t].ne);
      // split drum events - sort by MDrum pos
         ev = _f.trk [t].e;   neT = _f.trk [t].ne;
         Sort (ev, neT, sizeof (TrkEv), DTrCmp);

      // calc d [] - distinct dr notes
         for (nD = 0, e = 0;  e < neT;  e++) {
            i = ev [e].ctrl;           // got note n no prev or diff?  add it
            if ( (! (i & 0x80)) && ((! nD) || (i != d [nD-1])) )  d [nD++] = i;
         }
for (e=0; e<nD; e++)TRC("  d[`d]=`d=`s",e,d[e],MDrm2Str(s,d[e]));
         if (! nD)                     // nD == 0...  just Kick for it...
              {_f.trk [t].drm  = MDrm (CC("Kick"));   nD = 1;}
         else {
            if (_f.trk.Full (nD-1))  DBG("DrumExp  too many drm trks");
            _f.trk.Ins (t+1, nD-1);      // scoot postdrum tracks down
         // adj eTrk, rTrk
            if (Up.eTrk > t) Up.eTrk += (nD-1);
            if (Up.rTrk > t) Up.rTrk += (nD-1);
         // init new _f.trk for extra drums;  set each .e,.ne n new trk[t].ne
            for (x = nD-1;  x;  x--) {
               _f.trk.Cp (t+x, t);
               for (neX = 0;  neT && (ev [neT-1].ctrl == d [x]);
                    neX++, neT--)  ;
               _f.trk [t+x].e = & ev [neT];  _f.trk [t+x].ne  = neX;
TRC("  t=`d ne=`d", t+x, _f.trk [t+x].ne);
            }
            _f.trk [t].ne = neT;
TRC("  t=`d ne=`d", t, neT);
         // resort 1st trk so ctrls and drum nt sorted by time now
            Sort (_f.trk [t].e, _f.trk [t].ne, sizeof (TrkEv), EvCmp);

            for (x = 0;  x < nD;  x++) {    // set .grp,.drm
               _f.trk [t+x].grp = x ? true : false;
               _f.trk [t+x].drm = d [x];
            }
         }
      // so far .e,ne,dur,drm are set;  set rest;  _mapD overrides defaults now
         for (i = 0;  i < nD;  i++) {
TRC("  top=`d i=`d => set trk `d", t, i, t+i);
         // defaults
            _f.trk [t+i].vol = 127;   _f.trk [t+i].pan = 64;
            _f.trk [t+i].snd = SND_NONE;
            _f.trk [t+i].shh = false;
            _f.trk [t+i].lrn = _f.trk [t+i].ht = '\0';
            _f.trk [t+i].din = _f.trk [t+i].drm;
            for (x = 0;  x < _f.mapD.Ln;  x++)
                                      if (_f.mapD [x].ctl == _f.trk [t+i].drm) {
               _f.trk [t+i].shh = _f.mapD [x].shh;
               _f.trk [t+i].lrn = _f.mapD [x].lrn;
               _f.trk [t+i].ht  = _f.mapD [x].ht;
               _f.trk [t+i].vol = _f.mapD [x].vol;
               _f.trk [t+i].pan = _f.mapD [x].pan;
               _f.trk [t+i].snd = _f.mapD [x].snd;
               _f.trk [t+i].din = _f.mapD [x].inp;
TRC("   set trk `d snd from mapD[`d]=`d", t+i, x, _f.trk [t+i].snd);
               break;                  // on to the next drum
            }                          // uh oh, if no map syn NEEDS .snd :(
            MDrm2Str (_f.trk [t+i].name, _f.trk [t+i].drm);
            if (_f.trk [t+i].drm != _f.trk [t+i].din)
               StrFmt (_f.trk [t+i].name, "`s => `s",
                       MDrm2Str (s,  _f.trk [t+i].din),
                       MDrm2Str (s2, _f.trk [t+i].drm));
         }
TRC("  t=`d nTr=`d dev=`d dvt=`d",
t, Up.rTrk, _f.trk [t].dev, Up.dev [_f.trk [t].dev].dvt);
         if (_f.trk [t].snd == SND_NONE) {
            _f.trk [t].snd =
               Up.dvt [Up.dev [_f.trk [t].dev].dvt].SndID (CC("Drum/Drum"));
TRC("  set trk `d snd to drum/drum=`d", t, _f.trk [t].snd);
         }
//Dump ();
         t += nD;
      }
TRC(" t end");
   }
TRC("t loop done");
   if (setBnk)  SetBnk ();
TRC("DrumExp end");
}


ubyte Song::DrumCon ()
// build _f.mapD for caller n
// join all individual drum tracks back into ONE DrumTrack per dev
{ ubyte nD, x, otr = MAX_TRK;
  ubyt4 t = 0, tt;
TRC("DrumCon");
   _f.mapD.Ln = 0;
   while (t < Up.rTrk)  if (! TDrm ((ubyte)t))  t++;
   else {
      nD = 1;
      while ( (t+nD < Up.rTrk) && (_f.trk [t+nD].dev == _f.trk [t].dev) &&
                                  TDrm ((ubyte)t+nD) )   nD++;
   // write _f.mapD before we kill the info
      _f.mapD.Ln = nD;
      for (x = 0;  x < nD;  x++) {
         _f.mapD [x].ctl = _f.trk [t+x].drm;
         _f.mapD [x].shh = _f.trk [t+x].shh;
         _f.mapD [x].lrn = _f.trk [t+x].lrn;
         _f.mapD [x].ht  = _f.trk [t+x].ht;
         _f.mapD [x].inp = _f.trk [t+x].din;
         _f.mapD [x].vol = 127;
         _f.mapD [x].pan = 64;
         _f.mapD [x].snd = SND_NONE;
      }
   // adj all .trk refs
      if (Up.eTrk > t) Up.eTrk -= (nD-1);
      if (Up.rTrk > t) Up.rTrk -= (nD-1);

   // glue all the track evs back into 1st drum track re-sort events
      for (tt = t+nD-1; tt > t; tt--)  _f.trk [t].ne += _f.trk [tt].ne;
      Sort (_f.trk [t].e, _f.trk [t].ne, sizeof (TrkEv), EvCmp);

   // toss sub trks, clear out trk
      _f.trk.Del (t+1, nD-1);
      StrCp (_f.trk [t].name, CC("DrumTrack"));   _f.trk [t].drm = PRG_NONE;
      otr = (ubyte)t;
      t++;
   }
TRC("DrumCon end");
   return otr;
}


//------------------------------------------------------------------------------
// loadin song junk

void Song::CCClean ()
// kill any dup CCs with same trk/time/cc leaving only the last
{
/*
  ubyte t;
  ubyt4 p, q, k [1024], nk;
  TrkEv *e;
   for (t = 0;  t < Up.rTrk;  t++)
      for (e = _f.trk [t].e, p = 0;  p < _f.trk [t].ne;  p++)
         if (ECTRL (& e [p])) {
            for (nk = 0, q = p+1;     q < _f.trk [t].ne;  q++) {
               if (e [q].time > e [p].time)  break;
               if ( (e [q].ctrl == e [p].ctrl) && (nk < BITS (k)) )
                  k [nk++] = q;
            }
            if (nk) {               // gots killin ta do
DBG("killin `d dup CCs on same track=`d time=`d", nk, t, e [p].time);
               while (--nk) EvDel (t, k [nk-1]); // leave last one be; kill rest
                            EvDel (t, p);        // n kill p;  last at p now
            }
         }
*/
}


char *SongRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr)
{ static ubyte TB;                     // parse a record of the .song file
  static TStr  Msg;
  STable *s = (STable *)ptr;
  ubyte   i, nc;
   (void)len;   (void)ptr;
   if (pos == 0)  TB = TB_DSC;         // always start in descrip section
   if ((*buf == '-') || (*buf == '\0'))  return NULL;      // skip comment/empty
   for (ubyte t = 0;  t < TB_MAX;  t++)     // new section/table ?
      if (! StrCm (buf, s [t].Name ()))  {TB = t;  return NULL;}
   if ((s [TB].NRow () + 1) > s [TB].MaxRow ())
      return  StrFmt (Msg, "line=`d `s too many lines", pos+1, s [TB].Name ());
   if ((nc = s [TB].NCol ()) == 1)  s [TB].Add (buf);
   else {
     ColSep ss (buf, nc-1);
      for (i = 0;  i < nc;  i++)  s [TB].Add (ss.Col [i]);
   }
   return NULL;
}


static int IniTSigCmp (void *p1, void *p2)  // by .bar
{ TSgRow *t1 = (TSgRow *)p1, *t2 = (TSgRow *)p2;
   return (int)(t1->bar) - (int)(t2->bar);
}

void Song::Load (char *fn)
// ok, go thru the BRUTAL HELL to load in a song, etc...
{ TStr  fnt, buf;
  ubyte nt, t, dt, i;
  ubyt4 ln, e, ne, pe, te, tm, mint;
  char *m, *p, ud;
  File  f;
  TrkEv *e2;
  STable st [TB_MAX];
TRC("Load fn=`s", fn);
   App.Path (buf, 'd');   StrAp (buf, CC("/4_queue/"));    // git window title
   if (MemCm (fn, buf, StrLn (buf)))  FnName (fnt, fn);
   else                               StrCp  (fnt, & fn [StrLn (buf)]);
   StrCp (Up.ttl, fnt);   emit sgUpd ("ttl");
   Gui.SetTtl (StrFmt (buf, "`s - PianoCheetah", fnt));
   StrCp (_f.fn, fn);   _f.got = false;

   StrAp (fn, CC("/a.song"));
   if (! f.Size (fn)) {TRC("} Load  file size=0");   return;}

   if (_f.ev)  delete [] _f.ev;        // hope ya init'd it or BOOM :(
   _f.ev = NULL;   _f.nEv = 0;   _f.maxEv = 0;
   st [TB_DSC].Init (CC("Descrip:"), 1, MAX_DSC);
   st [TB_TRK].Init (CC("Track:")  , 4, MAX_TRK);
   st [TB_DRM].Init (CC("DrumMap:"), 7, MAX_DRM);
   st [TB_LYR].Init (CC("Lyric:")  , 2, MAX_LYR);
   st [TB_EVT].Init (CC("Event:")  , 2, MAX_EVT);

   if ((m = f.DoText (fn, & st, SongRec)))
      {TRC("} Load  DoText err=`s", m);   return;}
//st [TB_DSC].Dump (); st [TB_TRK].Dump (); st [TB_DRM].Dump ();
//st [TB_LYR].Dump (); st [TB_EVT].Dump ();

   for (e = 0, ln = st [TB_DSC].NRow ();  e < ln;  e++) {
      StrCp (buf, st [TB_DSC].Get (e, 0));
      if (StrLn (_f.dsc) + StrLn (buf) + 3 > sizeof (_f.dsc))
         TRC ("Load  _f.dsc is too big");
      StrAp (_f.dsc, buf);   StrAp (_f.dsc, CC("\n"));
   }
   DscLoad ();

TRC("init _f.ev, _f.trk[].e, build _f.ctl[].s");
   _f.maxEv = st [TB_EVT].NRow () + MAX_RCRD;    // couple extra but wtf :)
   _f.ev = new TrkEv [_f.maxEv];   _f.nEv = 0;
   if (! _f.ev)  {TRC("} Load  ev alloc fail");   return;}

   nt = (ubyte)(_f.trk.Ln = st [TB_TRK].NRow ());
   ne =                     st [TB_EVT].NRow ();
   _f.ctl.Ln = 3;   StrCp (_f.ctl [0].s, CC("Tmpo"));
                    StrCp (_f.ctl [1].s, CC("TSig"));
                    StrCp (_f.ctl [2].s, CC("KSig"));
   _f.tSg.Ln = 0;
   for (t = 0, pe = e = 0;  e < ne;  e++) {
      if (! StrCm (st [TB_EVT].Get (e, 0), CC("EndTrack"))) {
         if (t >= nt)  {TRC("} Load  EndTrack>nt");   return;}
         _f.trk [t].ne = (e-pe);   pe = e+1;
         _f.trk [t].e = & _f.ev [_f.nEv];   _f.nEv += _f.trk [t].ne;
         t++;
      }
      if ('!' ==       st [TB_EVT].Get (e, 1) [0]) {
         StrCp (buf, & st [TB_EVT].Get (e, 1) [1]);
         if ((m = StrCh (buf, '=')))  *m = '\0';
         else  {TRC("} Load  !ctrl no = at trk=`d ev#=`d", t+1, e);   return;}

         for (i = 0;  i < _f.ctl.Ln;  i++)  if (! StrCm (buf, _f.ctl [i].s))
                                               break;
         if (i >= _f.ctl.Ln) {         // new ctl s
            if (_f.ctl.Ln >= 128)  {TRC("} Load  >128 ctls");   return;}
            _f.ctl.Ln++;   StrCp (_f.ctl [i].s, buf);
         }
         if ((i == 1) && (! _f.tSg.Full ())) {
            StrCp (buf, ++m);   if ((m = StrCh (buf, '/')))  *m++ = '\0';
            te = _f.tSg.Ins ();
            _f.tSg [te].num = (ubyte)Str2Int (buf);
            _f.tSg [te].den = (ubyte)Str2Int (m);
            _f.tSg [te].sub = 0;      // don't need yet.  will refresh l8r
            _f.tSg [te].bar = (ubyt2)Str2Int (st [TB_EVT].Get (e, 0));
         }
      }
   }
   if (t < nt)  {TRC("} Load  EndTrack<ntrk");   return;}
   for (t = 0;  t < _f.ctl.Ln;  t++)  _f.ctl [t].sho = false;

// init initial tSg - sort by bar n calc .time
   Sort (_f.tSg.Ptr (), _f.tSg.Ln, _f.tSg.Siz (), IniTSigCmp);
   if ((_f.tSg.Ln == 0) || (_f.tSg [0].bar > 1))
      {_f.tSg.Ins (0);   _f.tSg [0].num = 4;   _f.tSg [0].den = 4;
                         _f.tSg [0].sub = 0;   _f.tSg [0].bar = 1;}
   for (tm = 0, t = 0;  t < _f.tSg.Ln;  t++) {
      _f.tSg [t].time = tm;
      if ((ubyt4)(t+1) < _f.tSg.Ln)
         tm += (_f.tSg [t+1].bar - _f.tSg [t].bar) * M_WHOLE * _f.tSg [t].num /
                                                               _f.tSg [t].den;
   }

// NOW we can convert TmSt n ctrls (n everything) into _f.trk[].e[]
   for (e2 = _f.trk [t = 0].e, e = 0;  e < ne;  e++) {
      StrCp (buf, st [TB_EVT].Get (e, 0));
      if (! StrCm (buf, CC("EndTrack")))  {e2 = _f.trk [++t].e;   continue;}

      e2->time = Str2Tm (buf);         // ok parse the rec - all start w time

      StrCp (buf, st [TB_EVT].Get (e, 1));
      if (*buf == '!') {               // ctl
         if ((m = StrCh (buf, '=')))  *m++ = '\0';
         else  {TRC("} Load  !ctl no = in trk=`d ev#=`d", t+1, e);   return;}
         for (i = 0;  i < _f.ctl.Ln;  i++)
            if (! StrCm (& buf [1], _f.ctl [i].s))
               {e2->ctrl = (ubyte)(0x80 | i);   break;}
         if      (i == 0) {            // tmpo
           ubyt2 tp;
            tp = (ubyt2)Str2Int (m);
            e2->valu = tp & 0x00FF;   e2->val2 = tp >> 8;
         }
         else if (i == 1) {            // tsig
           ubyte n, d, s, x;
            if (! (p = StrCh (m, '/')))
               {TRC("} Load  TSig missing / trk=`d ev#=`d", t+1, e);   return;}
            n = (ubyte)Str2Int (m);
            d = (ubyte)Str2Int (++p);
            s = 1;
            if ((m = StrCh (p, '/')))  s = (ubyte)Str2Int (++m);
            for (x = 0;  x < 8;  x++)  if ((1 << x) == d)  break;
            if (x >= 8)
               {TRC("} Load  TSig bad denom trk=`d ev#=`d", t+1, e);   return;}
            e2->valu = n;   e2->val2 = x | ((s-1) << 4);
         }
         else if (i == 2) {            // ksig
           char *map = CC("b2#b#b2#b#b2");
            e2->valu = i = MNt (m);
            if (m [StrLn (m)-1] == 'm')  e2->val2 = 1;  // minor
            if (map [i] != '2')
                  {if (map [i] == 'b')  e2->val2 |= 0x80;}  // b not #
            else  {if (m [1]   == 'b')  e2->val2 |= 0x80;}
         }
         else {
            e2->valu = (ubyte)Str2Int (m, & p);
            e2->val2 = (ubyte)(*p ? Str2Int (p) : 0);
         }
      }
      else {                           // note
//TStr db1;DBG("note trk=`d tm=`s buf=`s", t, TmS (db1, e2->time), buf);
         i = MDrm (p = buf);
         if (i == (ubyte)128) i = MKey (buf, & p);   else p += 4;
         ud = *p++;
         e2->ctrl = i;
         e2->valu = (ubyte)Str2Int (p, & p) | ((ud == '_') ? 0x80 : 0);
         e2->val2 = (ud == '~') ? 0x80 : 0;
         if (*p == '@')  for (i = 0;  i < BITS (MFing);  i++)
            if (! StrCm (MFing [i], p+1))  {e2->val2 = ++i;   break;}
      }
      e2++;
   }

// init trk: .p,dev,chn,grp,shh,lrn,ht,name
   for (t = 0;  t < nt;  t++) {
      _f.trk [t].p = 0;   _f.trk [t].dev = _f.trk [t].chn = 0xFF;    // nothin
      StrCp (_f.trk [t].name, st [TB_TRK].Get (t,3));      // track name col

   // set .grp,.shh,.lrn,.ht outa track mode thingy
      StrCp (buf, st [TB_TRK].Get (t,2));                  // track mode col
      _f.trk [t].grp = (*buf ==    '+') ? true : false;
      _f.trk [t].shh = StrCh (buf, '#') ? true : false;
      _f.trk [t].lrn = StrCh (buf, '?') ? true : false;
      if  ((m = StrSt (buf, CC("EZ")))) _f.trk [t].ht = m [2];
      else if  (StrSt (buf, CC("LH")))  _f.trk [t].ht = 'L';
      else if  (StrSt (buf, CC("RH")))  _f.trk [t].ht = 'R';
      else if  (StrSt (buf, CC("SH")))  _f.trk [t].ht = 'S';
      else                              _f.trk [t].ht = '\0';
   }
   _lrn.chd = false;
   ne = st [TB_LYR].NRow ();
   for (e = 0;  e < ne;  e++) {
      tm = Str2Tm (st [TB_LYR].Get (e, 0));
      StrCp (buf,  st [TB_LYR].Get (e, 1));
      while ((m = StrCh (buf, '_')))  *m = ' ';
      if      (*buf == '*') {                                        // * chd
         _lrn.chd = true;
         if (StrCm (buf, CC("*SHOW")))  TxtIns (tm, & buf [1], & _f.chd);
      }
      else if (*buf == '!')  TxtIns (tm, & buf [1], & _f.bug);       // ! bug
      else if (*buf == '?')  TxtIns (tm, & buf [1], & _f.cue, 'c');  // ? cue
      else                   TxtIns (tm,   buf,     & _f.lyr);       // lyr
   }

TRC("map dev/chn/snd");
   Up.rTrk = nt;
   for (t = 0; t < nt; t++)  PickDev (t, st [TB_TRK].Get (t, 1),
                                         st [TB_TRK].Get (t, 0));
TRC("sortin n ins rec trks");
   for (t = 0;  t < nt;  t++)   // sort play events
      Sort (_f.trk [t].e, _f.trk [t].ne, sizeof (TrkEv), EvCmp);
   if (TrkIns (nt++, CC("rec drum")) == MAX_TRK)
      {TRC("} Load  Too many tracks for rec drum");   return;}
   if (TrkIns (nt++, CC("rec melo")) == MAX_TRK)
      {TRC("} Load  Too many tracks for rec melo");   return;}
   CCClean ();
   mint = ReEv ();

TRC("lyr");
   for (e = 0;  e < _f.lyr.Ln;  e++)  if (StrCh (_f.lyr [e].s, '/'))  break;
   _hLyr = (e >= _f.lyr.Ln) ? 1 : 2;

TRC("soundbank init");
   for (dt = MAX_TRK, t = 0;  t < Up.rTrk;  t++)  if (TDrm (t))
      {dt = t;   break;}               // got drum trk ?
//DBG("dtrk=`d", dt);   st [TB_DRM].Dump ();
   _f.mapD.Ln = st [TB_DRM].NRow ();   // load _mapD so DrumExp can sep drm trks
   for (e = 0;  e < _f.mapD.Ln;  e++) {
   // defaults
      _f.mapD [e].ctl = _f.mapD [e].inp = MDrm (st [TB_DRM].Get (e, 0));
      _f.mapD [e].snd = SND_NONE;   _f.mapD [e].vol = 127;
                                    _f.mapD [e].pan = 64;
      _f.mapD [e].shh = _f.mapD [e].lrn = false;   _f.mapD [e].ht = '\0';
   // ok overwrote w drummap
      StrCp (buf, st [TB_DRM].Get (e, 4));
      if      (*buf == '#')  _f.mapD [e].shh = true;
      else if (*buf == '?')  _f.mapD [e].lrn = true;
      StrCp (buf, st [TB_DRM].Get (e, 5));
      if (*buf != '.')       _f.mapD [e].ht  = *buf;
      StrCp (buf, st [TB_DRM].Get (e, 6));
      if (*buf != '.')  if (MDrm (buf) < 128)
                             _f.mapD [e].inp = MDrm (buf);
      StrCp (buf, st [TB_DRM].Get (e, 1));  // sound
      if ((e == 0) && StrCm (buf, CC(".")))
      // can have drum progch in 1st .snd
         _f.mapD [0].snd = Up.dvt [Up.dev [_f.trk [dt].dev].dvt].SndID (buf);
   }
TStr t1, t2;  TRC("mapD shh lrn ht inp ctl vol pan snd");
for (t = 0; t < _f.mapD.Ln; t++)  TRC("`d/`d `b `c `c `s `s `>3d `>3d `d",
t, _f.mapD.Ln, _f.mapD [t].shh, _f.mapD [t].lrn ? _f.mapD [t].lrn : ' ',
                                _f.mapD [t].ht  ? _f.mapD [t].ht  : ' ',
MDrm2Str(t2,_f.mapD [t].inp), MDrm2Str(t1,_f.mapD [t].ctl),
_f.mapD [t].vol, _f.mapD [t].pan, _f.mapD [t].snd);
   DrumExp ();                         // SetBnk happens in DrumExp
   _f.got = true;                      // SetChn happens in TmHop
   ReDo ();   SetLp ('i');
if (App.trc)  Dump ();
   SetSym ();
   if (_rcrd)  Cmd (CC("timeBar1"));   else TmHop (mint);
TRC("Load end !");
}


//------------------------------------------------------------------------------
void Song::Save (bool pracOnly)
// Wipe sets pracOnly to true for autosave of practice a.song
// usually false for RecSave() to save in Recording/yyyymmddEtc/a.song
{ File  f;
  TStr  fnt, fns, fm, s, s2, s3, s4;
  char *m;
  ubyt4 i;
  ubyte d, t, dt, c;
  TrkEv *e;
  bool   prac = StrSt (_f.fn, CC("PianoCheetah")) ? true : false;  // in pc dir?
TRC("Save pracOnly=`b prac=`b _f.fn=`s trkLn=`d",
pracOnly, prac, _f.fn, Up.rTrk);
   if ((! Up.rTrk) || (pracOnly && (! prac)))  return;

TRC("actually savin'");
   StrCp (fm, CC("w"));
   if (pracOnly) {                     // new practice a.song w backup
      Cmd ("recWipe");
      StrCp (fns, _f.fn);   StrAp (fns, CC("/a.song"));   StrCp (fm, CC("wb"));
   }
   else {                              // recorded\songTitle\yyyymmddEtc
     FDir d;
//    TmpoPik ('r');                                         // \a.song into fns
      FnName (fnt, _f.fn);   Fn2Name (fnt);   // kill path
      StrFmt (fns, "`s/recorded/`s/`s",  App.Path (s2, 'd'), fnt, Now (s));
      d.Make (fns);   StrAp (fns, CC("/a.song"));
   }
TRC("save fn=`s", fns);
   dt = DrumCon ();                    // which also sets _f.mapD for us
   if (f.Open (fns, fm)) {
      f.Put (_f.dsc);

      f.Put (CC("Track:\n"));
      for (t = 0;  t < Up.rTrk;  t++) {
         StrCp (s2, TDrm (t) ? CC("Drum/Drum") : SndName (t));
         s3 [1] = *s4 = '\0';
         *s3 = _f.trk [t].shh ? '#' : '\0';
         if ((*s3 == '\0') && TLrn (t))  *s3 = pracOnly ? '?' : '#';
         if      (TEz (t))        StrFmt (s4, "EZ`c", _f.trk [t].ht);
         else if (_f.trk [t].ht)  StrFmt (s4, "`cH",  _f.trk [t].ht);
         f.Put (StrFmt (s, "`s  `s  `c`s`s  `s\n",
            DevName (t), *s2 ? s2 : "Piano\\AcousticGrand",
            _f.trk [t].grp ? '+' : '.', s3, s4, _f.trk [t].name));
      }

      f.Put (CC("DrumMap:\n"));
      for (d = 0;  d < _f.mapD.Ln;  d++)
         f.Put (StrFmt (s, "`s `s `d `d `c `c `s\n",
            MDrm2Str (s3, _f.mapD [d].ctl),
            (_f.mapD [d].snd == SND_NONE) ? "." :
              Up.dvt [Up.dev [_f.trk [dt].dev].dvt].Snd (_f.mapD [d].snd)->name,
            _f.mapD [d].vol, _f.mapD [d].pan,
            _f.mapD [d].shh ? '#' : (_f.mapD [d].lrn ? '?' : '.'),
            _f.mapD [d].ht  ? _f.mapD [d].ht : '.',
            MDrm2Str (s2,     _f.mapD [d].inp)
         ));

   // jam _f.cue,_f.bug,_f.chd into _f.lyr
      for (i = 0;  i < _f.cue.Ln;  i++) {
         StrCp (s2, CC("?"));
         if (_f.cue [i].tend)
            StrFmt (s2, "?/`d", _f.cue [i].tend - _f.cue [i].time);
         TxtIns (_f.cue [i].time, StrAp  (s2,        _f.cue [i].s), & _f.lyr);
      }                                // SHOW chord flag needed?
      if (_lrn.chd && (! _f.chd.Ln))  TxtIns (0, CC("*SHOW"), & _f.lyr);
      for (i = 0;  i < _f.chd.Ln;  i++)
         TxtIns (_f.chd [i].time, StrFmt (s2, "*`s", _f.chd [i].s), & _f.lyr);
      for (i = 0;  i < _f.bug.Ln;  i++)
         TxtIns (_f.bug [i].time, StrFmt (s2, "!`s", _f.bug [i].s), & _f.lyr);

      f.Put (CC("Lyric:\n"));
      for (i = 0;  i < _f.lyr.Ln;  i++) {
         StrCp (s3, _f.lyr [i].s);
         while ((m = StrCh (s3, ' ')))  *m = '_';
         f.Put (StrFmt (s, "`<9s `s\n",  TmSt (s2, _f.lyr [i].time), s3));
      }
      for (i = 0;  i < _f.lyr.Ln;)     // take em BACK on out :/
         {if (StrCh (CC("*?!"), _f.lyr [i].s [0])) _f.lyr.Del (i);   else i++;}

      f.Put (CC("Event:\n"));
      for (t = 0;  t < Up.rTrk;  t++) {
         for (i = 0, e = _f.trk [t].e;  i < _f.trk [t].ne;  i++) {
            f.Put (StrFmt (s, "`<9s ", TmSt (s2, e [i].time)));
            c = e [i].ctrl;
            if (c & 0x0080) {          // ctrl
               StrCp (s, _f.ctl [c & 0x7F].s);
               if      (! StrCm (s,  CC("Tmpo")))  // tmpo,tsig,ksig are special
                  f.Put (StrFmt (s, CC("!Tmpo=`d"),
                                                 e [i].valu | (e [i].val2<<8)));
               else if (! StrCm (s,  CC("TSig"))) {
                  f.Put (StrFmt (s, CC("!TSig=`d/`d"),  e [i].valu,
                                              1 << (e [i].val2 & 0x0F)));
                  if (e [i].val2 >> 4)  f.Put (StrFmt (s, "/`d",
                                               1 + (e [i].val2 >> 4)));
               }
               else if (! StrCm (s, CC("KSig"))) {
                  f.Put (CC("!KSig="));
                  if   (! (e [i].val2 & 0x80)) StrCp (s, MKeyStr  [e [i].valu]);
                  else if (e [i].valu != 11)   StrCp (s, MKeyStrB [e [i].valu]);
                  else                         StrCp (s, CC("Cb"));  // weird :/
                  if (e [i].val2 & 0x01)  StrAp (s, CC("m"));
                  *s = CHUP (*s);
                  f.Put (s);
               }
               else {
                  f.Put (StrFmt (s2, "!`s=`d",           s, e [i].valu));
                  if (e [i].val2)  f.Put (StrFmt (s, " `d", e [i].val2));
               }
            }
            else {                     // note
               StrFmt (s, "`s`c`d",
                  TDrm (t) ? MDrm2Str (s2, c) : MKey2Str (s2, c),
                  EUP (& e [i]) ? '^' : (EDN (& e [i]) ? '_' : '~'),
                  e [i].valu & 0x7F);
               if ((c = (e [i].val2 & 0x1F)))  {StrAp (s, CC("@"));
                                                StrAp (s, MFing [c-1]);}
               f.Put (s);
            }
            f.Put (CC("\n"));
         }
         f.Put (StrFmt (s, "EndTrack `d #ev=`d\n", t+1, _f.trk [t].ne));
      }
      f.Shut ();
      if (fm [1] == 'b')  App.Run (StrFmt (s, "delsame `s", fns));
   }
// if (! pracOnly)  TmpoPik ('o');     // restore tempo from having set to rec
   DrumExp (false);
TRC("Save done");
}
