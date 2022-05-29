// sReDo.cpp - init alll the data structs for learn mode tracks

#include "song.h"


// TDrm says whether drum or melodic
// TLrn says if ?
// TEz says if ez mode is on, melodic, and ? track
//    ez can only be on for all melodic or off for everything hard mode
//       ? drums are always hard mode
//       rHop says if we're waiting (versus syncd to pc clock)
// TSho says if track is drawn in notation
bool Song::TDrm (ubyte t)  {return (_f.trk [t].chn == 9)  ? true : false;}
bool Song::TLrn (ubyte t)  {return   _f.trk [t].lrn;}
bool Song::TEz  (ubyte t)  {return ((_f.trk [t].ht>='1') &&
                                    (_f.trk [t].ht<='7')) ? true : false;}
bool Song::TSho (ubyte t)  {return (TLrn (t) || (_f.trk [t].ht == 'S'))
                                                          ? true : false;}

void Song::ReTrk ()
// give gui what it needs in Up.tr
{ ubyte r;
  char  c;
  TStr  s, g;
  char *sl;
   Up.trk.Ln = 0;
   for (r = 0;  r < Up.rTrk;  r++) {
      Up.trk.Ins (r);
      Up.trk [r].dvt = Up.dev [_f.trk [r].dev].dvt;
      Up.trk [r].drm = TDrm (r);
      StrCp (Up.trk [r].lrn, _f.trk [r].shh ? CC("mute")
                          : (_f.trk [r].lrn ? CC("lrn") : CC("")));
      c = *s = _f.trk [r].ht;   s [1] = '\0';
      if ((c >= '1') && (c <= '7'))  {StrCp (s, CC("*ez9"));   s [3] = c;}
      StrCp (Up.trk [r].ez, s);
      StrCp (Up.trk [r].name, _f.trk [r].name);
      StrCp (s, SndName (r));
      *g = '\0';
      if (*s) {
         StrCp (g, s);
         if (MemCm (g, CC("Drum/"), 5))  sl = StrCh (g,       '/');
         else                            sl = StrCh (& g [5], '/');
         if (sl)  {*sl = '\0';   StrCp (s, & s [StrLn (g)+1]);}
         else                          *s = '\0';
      }
      StrCp (Up.trk [r].grp, g);       // SndGrp
      StrCp (Up.trk [r].snd, s);       // SndName
      if (_f.trk [r].grp)  StrCp  (s, CC("+"));
      else                 StrFmt (s, "`s.`d", DevName (r), _f.trk [r].chn+1);
      StrCp (Up.trk [r].dev, s);       // + / dev.chn
      if (_f.trk [r].nb)
           StrFmt (s, "(`d)`d", _f.trk [r].nb, _f.trk [r].nn);
      else StrFmt (s,     "`d",                _f.trk [r].nn);
      StrCp (Up.trk [r].notes, s);
      StrFmt (s, "`d", _f.trk [r].ne - _f.trk [r].nb -
                      (_f.trk [r].nn - _f.trk [r].nb)*2);
      StrCp (Up.trk [r].ctrls, s);
   }
TRC("ReTrk eTrk=`d ln=`d", Up.eTrk, r);
   emit sgUpd ("trk");
}


void Song::SetDn (char qu)             // DlgCfg quantize button ONLY allows it
// calc notesets (by time, all the ntDns across tracks)   trk.e[] => _dn[]
{ ubyte t, c, d, nn, x;
  ubyt4 p, q, tpos [128], tm, ptm;
  bool  got, qd [MAX_TRK], didq;
  TrkEv *e;
  ubyt4 ne;
  struct {ubyte t;   ubyt4 p;}  on [2][128], pon [2][128];
   _pDn = _dn.Ln = 0;
TRC("SetDn qu=`c", qu);
   _dn.Ln = 0;   MemSet (tpos, 0, sizeof (tpos));
                                  MemSet (qd,   0, sizeof (qd));
   MemSet (on, 0, sizeof (on));   MemSet (pon,  0, sizeof (pon));
   for (didq = false, got = true;  got; ) {
      got = false;
   // get tm - min time of a NtDn across all ? trks
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))
         for (p = tpos [t], e = _f.trk [t].e, ne = _f.trk [t].ne;  p < ne;  p++)
            if (ENTDN (& e [p])) {
               if      (! got)           {tm = e [p].time;   got = true;}
               else if (e [p].time < tm)  tm = e [p].time;
               break;
            }
      if (got) {
      // build on[] noteset from noteDn evs @ 1st time
         MemSet (on, 0, sizeof (on));  // start fresh - notes all off
         for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t)) {
            if (qu == 'q') {           // ARE mini quantizing to 64th note
               for (p = tpos [t], e = _f.trk [t].e, ne = _f.trk [t].ne;
                    (p < ne) && (e [p].time < tm + 12);  p++)   // 12 ticks=64th
                  if (ENTDN (& e [p])) {
                     ptm = tm;
                     if (e [p].time != tm) {
//if (! didq) {
//   didq = true;
//   for (ubyte tx = 0;  tx < Up.rTrk;  tx++)  if (TLrn (tx))  DumpTrEv (tx);
//}
TStr s1, s2;
DBG(" SetDn quant trk=`d p=`d/`d tmFr=`s tmTo=`s",
t, p, ne, TmSt(s1,e [p].time), TmSt(s2,tm));
                        qd [t] = true;   ptm = e [p].time;
                     }
                     e [p].time = tm;   d = TDrm (t) ? 1 : 0;
                     on [d][e [p].ctrl].t = t+1;
                     on [d][e [p].ctrl].p = p;

                  // need to scoot a possible NtUp earlier cuzu me?
                     if (ptm != tm)  for (q = p;  q;  q--)
                        if ((e [q-1].ctrl == e [p].ctrl) && ENTUP (& e [q-1]) &&
                            (e [q-1].time >= tm) && (e [q-1].time <= ptm))
                           {e [q-1].time = tm ? tm-1 : 0;   break;}
                  }
            }
            else {                     // NO quantizing
               for (p = tpos [t], e = _f.trk [t].e, ne = _f.trk [t].ne;
                    (p < ne) && (e [p].time <= tm);  p++)
                  if (ENTDN (& e [p])) {
                     d = TDrm (t) ? 1 : 0;
                     on [d][e [p].ctrl].t = t+1;
                     on [d][e [p].ctrl].p = p;
                  }
            }
            tpos [t] = p;
         }

      // stamp a Dn if room with all on melo,drum ntDns (drum ctrl => trk's din)
         if (_dn.Full ())  DBG("SetDn  outa room in Dn");
         _dn [_dn.Ln].time = tm;
         _dn [_dn.Ln].msec = 0;
         _dn [_dn.Ln].tmpo = 0;
         _dn [_dn.Ln].clip = false;
         nn = 0;
         if (_lrn.ez) {                // ez - max o one per trk
            d = 0;
            for (c = 0;  c < 128;  c++)
               if (on [d][c].t && (nn < BITS (_dn [0].nt))) {
                  for (x = 0;  x < nn;  x++)  if (on [d][c].t-1 ==
                                                  _dn [_dn.Ln].nt [x].t)  break;
                  if (x >= nn) {
                     x = on [d][c].t - 1;
                    TStr k;
                     StrCp (k, CC("4x"));
                     k [1] = "cdefgab" [_f.trk [x].ht-'1'];
                     _dn [_dn.Ln].nt [nn].nt = MKey (k);
                     _dn [_dn.Ln].nt [nn].t  = x;
                     _dn [_dn.Ln].nt [nn].p  = 0;
                     nn++;
                  }
                  MemCp (& pon [d][c], & on [d][c], sizeof (on [0][0]));
               }
            d = 1;                     // drums hard never ez
            for (c = 0;  c < 128;  c++)
               if (on [d][c].t && (nn < BITS (_dn [0].nt))) {
                  _dn [_dn.Ln].nt [nn].nt = c;
                  _dn [_dn.Ln].nt [nn].t  = on [d][c].t - 1;
                  _dn [_dn.Ln].nt [nn].p  = on [d][c].p;
                  nn++;
                  MemCp (& pon [d][c], & on [d][c], sizeof (on [0][0]));
               }
         }
         else                          // all real notes
            for (d = 0;  d < 2;  d++)  for (c = 0;  c < 128;  c++)
               if (on [d][c].t && (nn < BITS (_dn [0].nt))) {
                  _dn [_dn.Ln].nt [nn].nt = c;
                  _dn [_dn.Ln].nt [nn].t  = on [d][c].t - 1;
                  _dn [_dn.Ln].nt [nn].p  = on [d][c].p;
                  nn++;
                  MemCp (& pon [d][c], & on [d][c], sizeof (on [0][0]));
               }
         _dn [_dn.Ln++].nNt = nn;
      }
   }
//if (didq)  for (ubyte tx = 0;  tx < Up.rTrk;  tx++)  if (TLrn (tx))
//                                                      DumpTrEv (tx);
// always need dn[pdn].time >= _now so add a dummy at time=0 if none yet
   if ( (! _dn.Ln) || _dn [0].time )
      {_dn.Ins (0);   _dn [0].time = 0;   _dn [0].nNt = 0;}

// re-sort (ta be safe) if quant happened
   if (didq) {
      for (t = 0;  t < Up.rTrk;  t++)  if (qd [t])
         Sort (_f.trk [t].e, _f.trk [t].ne, sizeof (TrkEv), EvCmp);
//    for (ubyte tx = 0;  tx < Up.rTrk;  tx++)  if (TLrn (tx))  DumpTrEv (tx);
   }
TRC("SetDn end");
}


void Song::SetNt ()                    // turn trk[].e into trk[].n
{ bool  DN;                            // n build .nb, .nn too
  ubyte t, nt;
  ubyt4 ne, nn, n1, p, q, r, n, ov [128][2];
  TStr  s1, s2;
  TrkEv *e;
TRC("SetNt");
   if (_nt)  delete [] _nt;
   ne = 0;

// will never use a full ne, but whatev
   for (t = 0;  t < Up.rTrk;  t++)
      {_f.trk [t].nn = _f.trk [t].nb = 0;   ne += _f.trk [t].ne;}
   _nt = new TrkNt [ne];   MemSet (_nt, 0, sizeof (TrkNt) * ne);

   for (nn = 0, t = 0;  t < Up.rTrk;  t++) {
      _f.trk [t].n = & _nt [n1 = nn];
//DBG("trk=`d nn=`d", t, nn);
      for (nt = 0;  nt < 128;  nt++)  ov [nt][0] = ov [nt][1] = NONE;

   // find evs for each note;  one layer overlap allowed
      for (e = _f.trk [t].e, ne = _f.trk [t].ne, p = 0;  p < ne;  p++) {
         nt = e [p].ctrl;
         if (ENOTE (& e [p])) {        // note?
            if      (EUP (& e [p])) {  // ntUp - is it still unpaired?
               for (n = nn;  n > n1;  n--)  if (_nt [n-1].up == p)  break;
               if (n <= n1) {                    // yep, save it as broke
DBG(" no ntDn for ntUp at `s `s trk=`d",
TmSt (s1, e [p].time), MKey2Str (s2, e [p].ctrl), t);
                  _nt [nn].dn = NONE;   _nt [nn].up = p;
                  _nt [nn].tm = (e [p].time > (M_WHOLE/32-1)) ?
                                (e [p].time - (M_WHOLE/32-1)) : 0;
                  _nt [nn].te =  e [p].time;
                  _nt [nn].nt =  e [p].ctrl;
                  if ((_nt [nn].dn == NONE) || (_nt [nn].up == NONE))
                          _f.trk [t].nb++;
                  nn++;   _f.trk [t].nn++;
               }
            }
            else if (EDN (& e [p])) {  // ntDn - hunt for ntUp
               for (q = p+1;  q < ne;  q++)  if (e [q].ctrl == nt) {
                  if      (EUP (& e [q])) { // regular ntUp
                     _nt [nn].dn = p;   _nt [nn].up = q;   // n see if overlap
                     if ((ov [nt][0] != NONE) &&
                         (e [p].time >= e [ov [nt][0]].time) &&
                         (e [q].time <= e [ov [nt][1]].time))
                        _nt [nn].ov = true;
                  }                    // ntDn aGAIN - scan fer overlap or broke
                  else if (EDN (& e [q])) {
                     for (DN = true, r = q+1;  r < ne;  r++)
                        if (e [r].ctrl == nt) {
                           if      (EUP (& e [r]))    // up
                              {if (  DN) DN = false;   else            break; }
                                                           // yay  :)  2nd up
                           else if (EDN (& e [r]))    // dn
                              {if (! DN) DN = true;    else {r = ne;   break;}}
                        }                                  // rats :(  3rd down
                     if (r < ne) {_nt [nn].dn = ov [nt][0] = p;
                                  _nt [nn].up = ov [nt][1] = r;}
                     else        {_nt [nn].dn = p;   _nt [nn].up = NONE;
DBG(" no ntUp for ntDn at `s `s trk=`d",
TmSt (s1, e [p].time), MKey2Str (s2, e [p].ctrl), t);
                                 }
                  }
                  break;     // done on findin' very next ev on this nt
               }
               if (q >= ne)  {_nt [nn].dn = p;   _nt [nn].up = NONE;
DBG(" no ntUp for ntDn at `s `s trk=`d",
TmSt (s1, e [p].time), MKey2Str (s2, e [p].ctrl), t);
                             }
               _nt [nn].tm = e [p].time;   _nt [nn].nt = e [p].ctrl;
               _nt [nn].te = (_nt [nn].up != NONE) ? e [_nt [nn].up].time :
                                       (M_WHOLE/32-1+e [_nt [nn].dn].time);
               if ((_nt [nn].dn == NONE) || (_nt [nn].up == NONE))
                       _f.trk [t].nb++;
               nn++;   _f.trk [t].nn++;
            }
         }
      }
   }

// ok, if both hands get LH max from lrn,ht=L trks to show split point btw LH,RH
  ubyt4 m, m1, m2;
  ubyte unt;
  bool  un;
//TStr  db1,db2,db3,db4,db5,db6,db7;
   _lm.Ln = 1;   _lm [0].tm = 0;   _lm [0].nt = 0;
   if (_lrn.hand != 'B')  _lm.Ln = 0;
   else
      for (t = 0;  t < Up.rTrk;  t++)
         if (TLrn (t) && (_f.trk [t].ht == 'L'))
                                        for (n = 0;  n < _f.trk [t].nn;  n++) {
     TrkNt *tn = & _f.trk [t].n [n];
      if (tn->ov)  continue;           // underneath one takes care of me
//DBG("nt=`s tm=`s te=`s",
//MKey2Str(db1,tn->nt), TmSt(db2,tn->tm), TmSt(db3,tn->te));
//DBG("{ _lm[] so far");
//for (m = 0;  m < _lm.Ln;  m++)
//DBG("_lm[`d].tm=`s nt=`s", m, TmSt(db1,_lm[m].tm), MKey2Str(db2,_lm[m].nt));
//DBG("}");
   // m2 = pos in _lm of max nt at or before tn->te
      for (m = 0;  m < _lm.Ln;  m++)  {if (_lm [m].tm <= tn->te)  m2 = m;
                                       else                       break;}
      unt = _lm [m2].nt;   un = false;
//DBG("  m2=`d uTm=`s uNt=`s", m2, TmSt(db2,_lm[m2].tm), MKey2Str(db1,unt));

   // m1 = pos in _lm of where we ins/upd for ntDn
      for (m = 0;  m < _lm.Ln;  m++)   if (_lm [m].tm >= tn->tm)  break;
      m1 = m;
//DBG("  m1=`d", m1);

   // do 1st dn (if max-er)
      if ((m1 >= _lm.Ln) || ((tn->tm != _lm [m1].tm) &&
                             (tn->nt >  _lm [m1-1].nt))) {
//DBG("  ins new max - m1=`d", m1);
         _lm.Ins (m1);                 // ins max-er
         _lm [m1].tm = tn->tm;         _lm [m1].nt = tn->nt;   un = true;
      }                                // upd max-er
      else if ((tn->nt > _lm [m1].nt) && (tn->te >= _lm [m1].tm))
                                      {_lm [m1].nt = tn->nt;   un = true;}

   // and on the way down to te, upd max nt if max
      for (m = m1+1;  (m < _lm.Ln) && (_lm [m].tm < tn->te);  m++)
         if (tn->nt > _lm [m].nt)     {_lm [m ].nt = tn->nt;   un = true;}
// will need to RecDelhere sometimes when 2 same _lm in a row

   // restore min nt at te
      if (un) {
         if ((m >= _lm.Ln) || (tn->te != _lm [m].tm)) {
//DBG("  ins restore min - m=`d", m);
            _lm.Ins (m);
            _lm [m].tm = tn->te;   _lm [m].nt = unt;
         }
         else _lm [m].nt = unt;
      }
   }
//DBG("_lm[] pre-smooth");
//for (m = 0;  m < _lm.Ln;  m++)
//DBG("_lm[`d].tm=`s nt=`s", m, TmSt(db1,_lm[m].tm), MKey2Str(db2,_lm[m].nt));
// smooth it
   for (m = 0;  m < _lm.Ln;) {
      if ( (m+1 < _lm.Ln) && ((_lm [m+1].tm - _lm [m].tm) < (M_WHOLE/8)) &&
           (_lm [m].nt == 0) )
            _lm.Del (m);
      else  m++;
   }
//DBG("_lm[] post-smooth");
//for (m = 0;  m < _lm.Ln;  m++)
//DBG("_lm[`d].tm=`s nt=`s", m, TmSt(db1,_lm[m].tm), MKey2Str(db2,_lm[m].nt));
// DBG("tr# nt# note  dnTime upTime  dnVelo upVelo  dnEv upEv");
// for (t = 0;  t < Up.rTrk;  t++)  for (n = 0;  n < _f.trk [t].nn;  n++) {
//    StrCp (db4, "");   StrFmt (db5, "NONE");
//    StrCp (db6, "");   StrFmt (db7, "NONE");
//    if (_f.trk [t].n [n].dn != NONE)
//       {StrFmt (db4, "`d", _f.trk [t].e [_f.trk [t].n [n].dn].valu & 0x7F);
//        StrFmt (db6, "`d",               _f.trk [t].n [n].dn);}
//    if (_f.trk [t].n [n].up != NONE)
//       {StrFmt (db5, "`d", _f.trk [t].e [_f.trk [t].n [n].up].valu & 0x7F);
//        StrFmt (db7, "`d",               _f.trk [t].n [n].up);}
//    DBG("`d `d `s  `s `s  `s `s  `s `s",
//         t, n, MKey2Str (db1, _f.trk [t].n [n].nt),
//         TmSt (db2, _f.trk [t].n [n].tm), TmSt (db3, _f.trk [t].n [n].te),
//         db4, db5, db6, db7);
// }
TRC("SetNt end");
}


void Song::BarH (ubyt2 *h, ubyte *sb, ubyt2 bar)
{ bool  mt, got [9];
  ubyt4 md, tb, te, nn, p, btdur, sbdur, t1, st [9];
  ubyte sub, t, s;
  TSgRow *ts;
  TrkNt  *n;
   tb = Bar2Tm (bar);   te = Bar2Tm (bar+1);   ts = TSig (tb);
   if (ts->sub > 1) {                  // gonna check for ntDns outside bt spots
      sbdur = (btdur = M_WHOLE / ts->den) / ts->sub;
      MemSet (got, 0, sizeof (got));
      for (t = 0;  t < ts->sub;  t++)  st [t] = (t * btdur / ts->sub) + sbdur/2;
   }
   mt = true;   md = te - tb;
   for (t = 0;  t < Up.rTrk;  t++)  if (TSho (t))
      for (n = _f.trk [t].n, nn = _f.trk [t].nn, p = 0;  p < nn;  p++)
      // any note hittin bar makes it non empty
         if ((n [p].te >= tb) && (n [p].tm < te)) {
            mt = false;
         // find min dur of notes completely within
            if ( (n [p].tm >= tb) && (n [p].te <= te) &&
                ((n [p].te - n [p].tm + 1) < md) )
               md = n [p].te - n [p].tm + 1;
         // mark subbt if tsig has, note starts in bar
            if ( (ts->sub > 1) && (n [p].tm >= tb) && (n [p].tm < te) ) {
            // get offset from beat in ticks
               t1 = n [p].tm - tb;   while (t1 >= btdur)  t1 -= btdur;
               for (s = 0;  s < ts->sub;  s++)  if (t1 < st [s])  break;
               if (s && (s < ts->sub))  got [s] = true;
            }                          // don't care bout "on beat" subbeats
         }
   sub = 1;
   if (ts->sub > 1) {                  // see if subbt can be lessened
      for (s = 0;  s < ts->sub;  s++)  if (got [s])  {sub = ts->sub;   break;}
      switch (sub) {                   // see bout a lower div if got empty stuf
         case 4:                       // 1 . x .  makes sb=2 work
            if ((! got [1]) && (! got [3]))  sub = 2;
            break;
         case 9:                       // b . . x . . x . . => 3
            if ((! got [1]) && (! got [2]) && (! got [4]) && (! got [5]) &&
                (! got [7]) && (! got [8]))  sub = 3;
            break;
         case 8:                       // b . x . x . x . => 4
                                       // b . . . x . . . => 2
            if ((! got [1]) && (! got [3]) && (! got [5]) && (! got [7])) {
               if (got [2] || got [6])  sub = 4;
               else                     sub = 2;
            }
            break;
         case 6:                       // b . . x . . => 2
                                       // b . x . x . => 3
            if ((! got [1]) && (! got [5])) {
               if  (! got [3])                  sub = 3;
               if ((! got [2]) && (! got [4]))  sub = 2;
            }
            break;
      }
   }
//DBG("BarH bar=`d mt=`b sub=`d md=`d", bar, mt, sub, md);
   *sb = sub;                          // store subbeat
// default to MAX barh - max magnif is barDur*5/16
   *h = (ubyt2)((te - tb)*5/16);
// usually, noteh = ntDur * barh / barDur;  calc (less) barh if md's h >14
   if ((md*5/16) > 14)                 // then shrink so h of mindur is 14
      *h = (ubyt2)(14 * (te - tb) / md);
// got subbt>1 - expand h if needed
   if ((sub > 1) && (*h < (ts->num * ts->sub * 8)))
      *h =                (ts->num * ts->sub * 8);
// absolute min bar h
   if (mt)  *h = ts->num * 10;
//DBG("   final h=`d sub=`d", *h, *sb);
}


void Song::SetSym ()
// ok, now paginate it all up given our CtlNt's w,h and all them thar notes
// Draw calls us, not ReDo;  depends on SetDn(_dn[]) n SetNt(trk[].n) tho
{ ubyte nw, ww, nMn, nMx, nd, dmap [128], dpos, td, t, st, bl, sb, i, nt;
  ubyt2 W, H, x, xo, w, th, ch, b, dx, cw, y1, y2, h;
  ubyt4 np, nc, nb, ns, p, q, nn, pc1, cb1, cs1, tb, te, ntb, nte, sn, d;
  bool  drm, mt;
  TrkNt   *n;
  DownRow *dr;
//TStr ts1, ts2;
   W = Up.w;   H = Up.h;
TRC("SetSym w=`d h=`d", W, H);
   nw = W_NT;   ww = 24;   th = Up.txH;     // dem consts
   for (cw = 0, p = 0;  p < _f.ctl.Ln;  p++)  if (_f.ctl [p].sho)  cw += th;
   _pag.Ln = _col.Ln = _blk.Ln = _sym.Ln = 0;
   for (b = 1;  b <= _bEnd;) {
      if (_pag.Full ()) {
TRC("SetSym end - w,h too small");
         return;                       // prob screen too small - give up
      }
      np = _pag.Ins ();                // init pag[np]
      _pag [np].col = & _col [pc1 = _col.Ln];
//DBG("b=`d/`d np=`d pc1=`d W=`d H=`d", b, _bEnd, np, pc1, W, H);
      _pag [np].nCol = 0;
      _pag [np].w = _pag [np].h = 0;
      do {
         if (_col.Full ()) {
TRC("_col full prob cuz w,h");
            break;
         }
         nc = _col.Ins ();             // init col[nc]
         _col [nc].blk = & _blk [cb1 = _blk.Ln];   _col [nc].nBlk = 0;
         _col [nc].sym = & _sym [cs1 = _sym.Ln];   _col [nc].nSym = 0;
//DBG(" nc=`d cb1=`d cs1=`d", nc, cb1, cs1);
         _col [nc].x   = _pag [np].w;  // remem 1st blk,sym of col
         ch = H_KB;

      // build blk[]s for col from b while blks h (per b's tm,tSc) fit
         BarH (& h, & sb, b);
         nb = _blk.Ins ();     _col [nc].nBlk++;
         _blk [nb].bar = b;    _blk [nb].tMn = Bar2Tm (b);
                               _blk [nb].tMx = Bar2Tm (b+1);
         _blk [nb].y   = ch;   _blk [nb].h   = h;   ch += h;
         _blk [nb].sb  = sb;
//DBG("  nb(1)=`d bar=`d ch=`d tMn=`d tMx=`d y=`d h=`d",
//nb,b,ch,_blk[nb].tMn, _blk[nb].tMx, _blk[nb].y, _blk[nb].h);
         while (b+1 <= _bEnd) {        // there has to BE another bar
            BarH (& h, & sb, b+1);
            if ((ch + h) > H)  break;  // won't fit, col complete, outa herez

            nb = _blk.Ins ();      _col [nc].nBlk++;  // add block, extend col
            _blk [nb].bar = ++b;   _blk [nb].tMn = Bar2Tm (b);
                                   _blk [nb].tMx = Bar2Tm (b+1);
            _blk [nb].y   = ch;    _blk [nb].h   = h;   ch += h;
            _blk [nb].sb  = sb;
//DBG("  nb=`d bar=`d ch=`d tMn=`d tMx=`d y=`d h=`d",
//nb,b,ch,_blk[nb].tMn, _blk[nb].tMx, _blk[nb].y, _blk[nb].h);
         }
         b++;                          // on to the next bar

      // init width parts of col based on sym[]s we'll have, ez, n ntPoss
         nd = nMx = 0;   nMn = 127;
         tb = Bar2Tm (_blk [cb1].bar);   te = Bar2Tm (_blk [nb].bar+1);

         for (t = 0;  t < Up.rTrk;  t++)  if (TSho (t)) {
            if      (TDrm (t)) {
               for (n = _f.trk [t].n, nn = _f.trk [t].nn, p = 0;  p < nn;  p++)
                  if ((n [p].te >= tb) && (n [p].tm < te))
                     {dmap [nd++] = t;   break;}      // one'll do it
            }
            else if (_lrn.ez) {
               for (dr = & _dn [0], d = 0;  d < _dn.Ln;  d++, dr++) {
                  ntb = nte = dr->time;   nte += (M_WHOLE/8*3/4);
                  for (i = 0;  i < dr->nNt;  i++)  if (dr->nt [i].t == t)
                     if ((nte >= tb) && (ntb < te)) {
                        if (dr->nt [i].nt < nMn)  nMn = dr->nt [i].nt;
                        if (dr->nt [i].nt > nMx)  nMx = dr->nt [i].nt;
                        break;                        // one'll do it
                     }
               }
            }
            else
               for (n = _f.trk [t].n, nn = _f.trk [t].nn, p = 0;  p < nn;  p++)
                  if ((n [p].te >= tb) && (n [p].tm < te)) {
                     if (n [p].nt < nMn)  nMn = n [p].nt;
                     if (n [p].nt > nMx)  nMx = n [p].nt;
                  }
         }

      // always want SOME notes in col (if none, use 4c,c#,d;  if 1, use nMn+2)
         if      (! nMx)       {nMn = MKey (CC("4c"));   nMx = MKey (CC("4d"));}
         else if (nMx-nMn < 2)  nMx = nMn + 2;
         dx = (nMx-nMn+1) * nw;        // melo nt w - used by drum sym .x later

      // if nMn or nMx are white, gotta give extra w sigh
         xo = 0;
         if (KeyCol [nMn%12] == 'w')  dx += (xo = WXOfs [nMn%12] * W_NT / 12);
         if (KeyCol [nMx%12] == 'w')  dx += (24 - W_NT -
                                                  WXOfs [nMx%12] * W_NT / 12);
         _col [nc].nMn  = nMn;
         _col [nc].nMx  = nMx;
         _col [nc].nDrm = nd;
         MemCp (_col [nc].dMap, dmap, 128);
         _col [nc].w    = 8 + (_lrn.chd?th:0) + W_Q + dx + nd*nw + cw;
         _col [nc].h    = ch;          // borders,chd,cue,meloNt,drumNt,ctrl
//DBG("  nc=`d nMn=`s nMx=`s nDrm=`d w=`d h=`d",
//nc,MKey2Str(ts1,nMn),MKey2Str(ts2,nMx),nd,_col[nc].w,_col[nc].h);

      // build sym[]s for col calcin w=b,w,clip n nt,trk overlaps
      // sym.x is an offset from col's nx
      // order by reversed track so lead overdraws bass,
      // white then black so black overdraws,etc
         dpos = nd;
         for (t = (ubyte)Up.rTrk;  t--;)  if (TSho (t)) {
            drm = TDrm (t);
            if (drm) {
               for (mt = true, td = 0;  td < nd;  td++) if (dmap [td] == t)
                                                           {mt = false;  break;}
               if (mt)  continue;      // empty of notes - neeext
               dpos--;                 // bump dpos
            }
            if (_lrn.ez && (! TDrm (t))) {
               for (dr = & _dn [0], d = 0;  d < _dn.Ln;  d++, dr++) {
                  ntb = nte = dr->time;   nte += (M_WHOLE/8*3/4);
                  for (i = 0;  i < dr->nNt;  i++)  if (dr->nt [i].t == t) {
                     if ((nte < tb) || (ntb >= te))  continue;
                     nt = dr->nt [i].nt;
                     ns = _sym.Ins ();
                     _sym [ns].tr = t;   _sym [ns].nt = nt;
                     _sym [ns].top = _sym [ns].bot = true;
                     if      (ntb < ((tb < M_WHOLE/32) ? 0 : (tb-M_WHOLE/32)))
                        {_sym [ns].top = false;   _sym [ns].y = H_KB;}
                     else if (ntb < tb) {
                     // top stickin into piano area
                        q = cb1;
                        _sym [ns].y = (ubyt2)(_blk [q].y - _blk [q].h *
                                              (_blk [q].tMn - ntb) /
                                              (_blk [q].tMx - _blk [q].tMn));
                     }
                     else {
                        for (q = cb1;  q < nb;  q++)
                           if ((ntb >= _blk [q].tMn) &&
                               (ntb <  _blk [q].tMx))  break;
                        _sym [ns].y = (ubyt2)(_blk [q].y + _blk [q].h *
                                              (ntb          - _blk [q].tMn) /
                                              (_blk [q].tMx - _blk [q].tMn));
                     }
                     if (nte >= te)
                        {_sym [ns].bot = false;
                         _sym [ns].h = ch - _sym [ns].y;}
                     else {
                        for (q = cb1;  q < nb;  q++)
                           if ((nte >= _blk [q].tMn) &&
                               (nte <  _blk [q].tMx))  break;
                        _sym [ns].h = (ubyt2)(_blk [q].y + _blk [q].h *
                                              (nte          - _blk [q].tMn) /
                                              (_blk [q].tMx - _blk [q].tMn) -
                                              _sym [ns].y + 1);
                     }
                     if (_sym [ns].h < 4)  _sym [ns].h = 4;     // min we can do
                  // always white
                     x = xo + (nt - nMn) * nw;   w = ww;
                     x -= (WXOfs [nt % 12] * nw / 12);
                     _sym [ns].x = x;   _sym [ns].w = w;
//DBG("   ns=`d tr=`d nt=`s x=`d y=`d w=`d h=`d top=`b bot=`b",
//ns,_sym[ns].tr, MKey2Str(ts1,nt),
//_sym[ns].x,_sym[ns].y, _sym[ns].w,_sym[ns].h, _sym[ns].top,_sym[ns].bot);
                     _col [nc].nSym++;
                  }
               }
            }
            else
               for (bl = 0;  bl < (drm ? 1 : 2);  bl++)
                  for (n = _f.trk [t].n, nn = _f.trk [t].nn,
                       p = 0;  p < nn;  p++) {
                     ntb = n [p].tm;   nte = n [p].te;
                     if ((nte < tb) || (ntb >= te))  continue;
                     if ((! drm) && (KeyCol [n [p].nt % 12] ==
                                     (bl ? 'w' : 'b')))        continue;
                     ns = _sym.Ins ();
                     _sym [ns].tr = t;   _sym [ns].nt = p;
                     _sym [ns].top = (n [p].dn != NONE) ? true : false;
                     _sym [ns].bot = (n [p].up != NONE) ? true : false;
                     if      (ntb < ((tb < M_WHOLE/32) ? 0 : (tb-M_WHOLE/32)))
                        {_sym [ns].top = false;   _sym [ns].y = H_KB;}
                     else if (ntb < tb) {
                     // top stickin into piano area
                        q = cb1;
                        _sym [ns].y = (ubyt2)(_blk [q].y - _blk [q].h *
                                              (_blk [q].tMn - ntb) /
                                              (_blk [q].tMx - _blk [q].tMn));
                     }
                     else {
                        for (q = cb1;  q < nb;  q++)
                           if ((ntb >= _blk [q].tMn) &&
                               (ntb <  _blk [q].tMx))  break;
                        _sym [ns].y = (ubyt2)(_blk [q].y + _blk [q].h *
                                              (ntb          - _blk [q].tMn) /
                                              (_blk [q].tMx - _blk [q].tMn));
                     }
                     if (nte >= te)
                        {_sym [ns].bot = false;
                         _sym [ns].h = ch - _sym [ns].y;}
                     else {
                        for (q = cb1;  q < nb;  q++)
                           if ((nte >= _blk [q].tMn) &&
                               (nte <  _blk [q].tMx))  break;
                        _sym [ns].h = (ubyt2)(_blk [q].y + _blk [q].h *
                                              (nte          - _blk [q].tMn) /
                                              (_blk [q].tMx - _blk [q].tMn) -
                                              _sym [ns].y + 1);
                     }
                     if (_sym [ns].h < 4)  _sym [ns].h = 4;     // min we can do
                     _sym [ns].x = xo + (n [p].nt - nMn) * nw;  // default x,w
                     _sym [ns].w = nw;
                     if (drm)  _sym [ns].x = dx + dpos*nw; // weird x for drums
                     else {
                        if (! bl) {    // white keys - special,clipped x,w
                           x = _sym [ns].x;   w = ww;
                           x -= (WXOfs [n [p].nt % 12] * nw / 12);
                           _sym [ns].x = x;   _sym [ns].w = w;
                        }

                     // check for sym on dif melo trk, same nt, time overlap
                        y1 = _sym [ns].y;   y2 = y1 + _sym [ns].h - 1;
                        for (q = cs1;  q < ns;  q++) {
                           st = _sym [q].tr;   sn = _sym [q].nt;
                           if ( (t != st) && (! TDrm (st)) &&
                                (_f.trk [st].n [sn].nt == n [p].nt) &&
                                (_sym [q].y+_sym [q].h-1 > y1)      && // NOT =
                                (_sym [q].y              < y2) ) {
                              _sym [ns].x = _sym [q].x;
                              _sym [ns].w = _sym [q].w;
                              if (_sym [ns].w >= 9)
                                 {_sym [ns].x += 3;   _sym [ns].w -= 3;}
                           }
                        }
                        if (n [p].ov && (_sym [ns].w >= 9))
                           {_sym [ns].x += 3;   _sym [ns].w -= 3;}
                     }
//DBG("   ns=`d tr=`d nt=`s x=`d y=`d w=`d h=`d top=`b bot=`b",
//ns,_sym[ns].tr, MKey2Str(ts1,n [_sym[ns].nt].nt),
//_sym[ns].x,_sym[ns].y, _sym[ns].w,_sym[ns].h, _sym[ns].top,_sym[ns].bot);
                     _col [nc].nSym++;
                  }
         }

      // complete out col, bumping pag - take last barline into account :/
         if (_col [nc].h + 2 > _pag [np].h)  _pag [np].h = _col [nc].h + 2;
         _pag [np].w += _col [nc].w;   _pag [np].nCol++;   nc++;

      } while ((b <= _bEnd) && (_pag [np].w < W));
//DBG(" lpEnd b=`d nc=`d", b, nc);

   // last col will usually go over, but always keep one of em
      if ((_pag [np].nCol > 1) && (_pag [np].w > W)) {     // reset to lop off
         _pag [np].nCol--;   nc--;                         // last col
         _pag [np].w = _pag [np].h = 0;
         for (p = pc1;  p < nc;  p++) {
            _pag [np].w += _col [p].w;
            if (_col [p].h + 2 > _pag [np].h)  _pag [np].h = _col [p].h + 2;
         }
         b        = _col [nc].blk [0].bar;
         _blk.Ln -= _col [nc].nBlk;
         _sym.Ln -= _col [nc].nSym;
         _col.Ln--;
//DBG("b=`d/`d bLn=`d sLn=`d cLn=`d", b, _bEnd, _blk.Ln, _sym.Ln, _col.Ln);
      }
   }
TRC("SetSym end");
}


//------------------------------------------------------------------------------
void Song::ReDo ()
{ ubyte t;
  char  ch;
TRC("ReDo");
   if (_lrn.POZ)                       Shush (false);
   NotesOff ();
   if ((! Up.uPoz) && _timer->Pause ())  Poz (false);
TRC(" clear stuph");
   MemSet (_lrn.rec,   0, sizeof (_lrn.rec));
   MemSet (_lrn.toRec, 0, sizeof (_lrn.toRec));
TRC(" redo vwNt/ez/rHop");
   _lrn.vwNt = _lrn.ez = _lrn.rHop = false;
   for (t = 0; t < Up.rTrk; t++)  {if (TSho (t))  _lrn.vwNt = true;
                                   if (TEz  (t))  _lrn.ez   = true;}
   if ( (PLAY || PRAC || _lrn.pLrn) && ((! _lrn.ez) || _f.ezHop) )
                                                _lrn.rHop = true;
// where are our hands - always ht(\0) unless got BOTH r,l
   _lrn.hand = '\0';
   for (ch = '\0', t = 0; (ch != 'b') && (t < Up.rTrk); t++) {
      if (_f.trk [t].ht == 'L')  ch = (ch == 'r') ? 'b' : 'l';
      if (_f.trk [t].ht == 'R')  ch = (ch == 'l') ? 'b' : 'r';
   }                                   // set Cfg.hand based on what's ?d
   if (ch == 'b')  for (t = 0; (_lrn.hand != 'B') && (t < Up.rTrk); t++)
      if (TLrn (t)) {
         if (_f.trk [t].ht == 'L')  _lrn.hand = (_lrn.hand == 'R') ? 'B' : 'L';
         if (_f.trk [t].ht == 'R')  _lrn.hand = (_lrn.hand == 'L') ? 'B' : 'R';
      }
TRC(" vwNt=`b ez=`b rHop=`b hand=`c", _lrn.vwNt, _lrn.ez, _lrn.rHop, _lrn.hand);
TRC(" set icos");
   emit sgUpd ("tbPoz");   emit sgUpd ("tbLrn");
TRC(" ReEv; SetDn; SetNt; TmHop");
   ReEv ();   SetDn ();   SetNt ();   TmHop (_now);
   _pg = _tr = 0;   SetSym ();   Draw ();   ReTrk ();   DscSave ();
TRC("ReDo end");
}
