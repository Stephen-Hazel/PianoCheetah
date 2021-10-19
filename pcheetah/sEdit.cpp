// sEdit.cpp - gory util funcs o Song

#include "song.h"

char  KeyCol [13] = "wbwbwwbwbwbw";
ubyte WXOfs  [12] = {0, 99, 4, 99, 8, 0, 99, 3, 99, 6, 99, 9};

ubyt2 Song::Nt2X (ubyte n, ColDef *co, char gr)
// nx is W_NT grid start - offset past border, optional chord, cue, whiteBump
// bump by W_NT*(this note-colMin note)
// then normally offset by whiteBump of n unless gr[id] = white tail x
{ ubyte m = co->nMn;
  ubyt2 nx;
   nx = co->x + 4 + (_lrn.chd ? Up.txH : 0) + W_Q +
        ((KeyCol [m % 12] == 'w') ? (WXOfs [m % 12] * W_NT / 12) : 0);
   nx += (W_NT * (n - m));
   if (! gr)  if (KeyCol [n % 12] == 'w')  nx -= (WXOfs [n % 12] * W_NT / 12);
   return nx;
}

ubyt2 Song::Dr2X (ubyte d, ColDef *co)
{ ubyt2 dx;                            // start past any nMx whiteBump
   dx = Nt2X (co->nMx, co) + ((KeyCol [co->nMx % 12] == 'w') ? 24 : W_NT);
   return dx + d * W_NT;               // then offset by drum pos
}

ubyt2 Song::CtlX (ColDef *co)  {return Dr2X (co->nDrm, co);}


ubyt2 Song::Tm2Y (ubyt4 t, ColDef *co, BlkDef **bl)
{ ubyt4 b;
//DBG("Tm2Y t=`d co=`08x bl=`08x", t, co, bl);
   for (b = 0;  (b+1) < co->nBlk;  b++)  if ((t >= co->blk [b].tMn) &&
                                             (t <  co->blk [b].tMx))  break;
   if (bl)  *bl = & co->blk [b];
//DBG("Tm2Y b=`d", b);
   return (ubyt2)(co->blk [b].y + (co->blk [b].h-1) * (t - co->blk [b].tMn) /
                                  (co->blk [b].tMx - 1  -  co->blk [b].tMn));
}


ubyt4 Song::Y2Tm (ubyt2 y, ColDef *co)
// y can be above/below valid Ys for col if dragging cues,chords
{ ubyt4 t;
   if (y < co->blk [0].y) {            // reuse t - dur above blk
      t =                   (co->blk [0].tMx - 1 - co->blk [0].tMn) *
                            (co->blk [0].y - y) / (co->blk [0].h - 1);
      if (t <= co->blk [0].tMn)  return co->blk [0].tMn - t;
      return 0;              // return y timescaled by blk#0;  limited to 0
   }
   for (t = 0;  (t+1) < co->nBlk;  t++)     // end with USEable blk in t
      if ((y >= co->blk [t].y) && (y < co->blk [t].y+co->blk [t].h))  break;
   return co->blk [t].tMn + (co->blk [t].tMx - 1 - co->blk [t].tMn) *
                            (y - co->blk [t].y) / (co->blk [t].h - 1);
}


ubyt4 Song::GetDn (TrkEv *e, ubyt4 p, ubyte nt)
// edcmds and looping can "leak" ntUps
// 8cDn,4f#Dn,8cUp,4f#Up=>recorded 4f#;  looping with some nts Dn;  => :(
// try to prevent that by looking for prev ntDn w/ no prev ntUp (on same nt)
{ ubyt4 end = p;
//TStr d1;
//DBG("GetDn p=`d nt=`s", p, MKey2Str (d1, nt));
   while (p--)  if (e [p].ctrl == nt) {               // got prev ev same nt?
      if      (! (e [p].valu & 0x80))  break;         // got nother (diff) up :(
      else if (! (e [p].val2 & 0x80)) {
//DBG("rc=`d", p);
       return p;      // got dn :)
}
   }
//DBG("rc=end p=`d", p);
   return end;
}


ubyt4 Song::GetUp (TrkEv *e, ubyt4 p, ubyt4 ne, ubyte nt)
// look for next ntUp w/ no prev ntDn (on same nt)
{ ubyt4 end = p++;
   for (;  p < ne;  p++)  if (e [p].ctrl == nt) {     // got latr ev same nt?
      if      (! (e [p].valu & 0x80))  return p;      // got up :)
      else if (! (e [p].val2 & 0x80))  return end;    // got nother (diff) dn :(
   }
   return end;
}


ubyt4 Song::SilPrv (ubyt4 tm)
// scoot q note earlier n look back for silence (no notes coverin time)
// if no silence for 2 bars back, just use tm as is
{ ubyte t;
  bool  got = true;
  ubyt4 tIn, t1, p, bp;
  TrkEv *e;
   if (tm <= (M_WHOLE/4))  return 0;
   tIn = tm;
   tm -= (M_WHOLE/4);   t1 = tm;

// keep checkin for ? notes coverin tm n get new min;  check THAT2
   do
      for (got = false, t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))
         for (e = _f.trk [t].e, p = _f.trk [t].ne;  p;) {
            --p;                    // skip evs till we get a ntUp in p-1
            if (ENTUP (& e [p])) {
               if (e [p].time < tm)  break; // end of the line for this trk
               if ((bp = GetDn (e, p, e [p].ctrl)) < p)
                  if (e [bp].time < tm)  {got = true;   tm = e [bp].time;}
            }
         }
   while (got);

   if (tm == t1) {                     // was in silence already - get 1st NtDn
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))
         for (e = _f.trk [t].e, p = 0;  p < _f.trk [t].ne;  p++)
            if (ENTDN (& e [p]) && (e [p].time >= tm))     // t1 has min so far
               {if (e [p].time <= t1) t1 = e [p].time;   break;}
      if (t1 < tm)  tm = t1;
   }
   if (tm)  tm--;
   if (tm+M_WHOLE*2 < tIn)  tm = tIn;  // don't go further back than 2 bars
   return tm;
}


ubyt4 Song::SilNxt (ubyt4 tm)
// scoot q note later n look forward for silence (no notes coverin time)
// if no silence for 2 bars foreward, just use tm as is
{ ubyte t;
  bool  got = true;
  ubyt4 tIn, t1, tme, ne, p, ep;
  TrkEv *e;
   tme = Bar2Tm (_bEnd);
   tIn = tm;
   tm += (M_WHOLE/4);   if (tm >= tme)  return tme;
                        t1 = tm;
// keep checkin for ? notes coverin tm n get new max;  check THAT2
   do
      for (got = false, t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))
         for (e = _f.trk [t].e, ne = _f.trk [t].ne, p = 0;  p < ne;  p++)
            if (ENTDN (& e [p])) {  // skip evs till we get a ntDn
               if (e [p].time > tm)  break;   // end of the line for this trk
               if ((ep = GetUp (e, p, ne, e [p].ctrl)) > p)
                  if (e [ep].time > tm)  {got = true;   tm = e [ep].time;}
            }
   while (got);

   if (tm == t1) {                     // was in silence already - get end NtUp
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t))
         for (e = _f.trk [t].e, p = _f.trk [t].ne;  p;) {
            p--;
            if (ENTUP (& e [p]) && (e [p].time <= tm))     // t1 has max so far
               {if (e [p].time >= t1) t1 = e [p].time;   break;}
         }
      if (t1 > tm)  tm = t1;
//    tm += ((M_WHOLE/4) - tm % (M_WHOLE/4));
   }
   if (tm > tme)  tm = tme;
   if (tm > tIn+M_WHOLE*2)  tm = tIn;  // don't go further foreward than 2 bars
   return tm;
}


//______________________________________________________________________________
void Song::ReCtl ()                    // rebuild used _f.dvts' CCMaps
{ ubyte d, i;
TRC("ReCtl  rebuild dvt.CCMap");
   for (d = 0;  d < Up.dvt.Ln;  d++)  if (Up.dvt [d].Name () [0]) { // used?
      MemSet (Up.dvt [d].CCMap, 0, sizeof (Up.dvt [d].CCMap));
      for (i = 0;  i < _f.ctl.Ln;  i++)
         Up.dvt [d].CCMap [i] = Up.dvt [d].CCID (_f.ctl [i].s);
   }
}


static int CtlCmp (void *p1, void *p2)  // ..._f.ctl str sortin (by MCC[] pos)
{ char *s1 = (char *)p1, *s2 = (char *)p2;
  ubyte i1, i2;
   if (*s1 == '+')  s1++;   if (*s2 == '+')  s2++;
   for (i1 = 0;  i1 < NMCC;  i1++)  if (! StrCm (s1, MCC [i1].s))  break;
   for (i2 = 0;  i2 < NMCC;  i2++)  if (! StrCm (s2, MCC [i2].s))  break;
   if ((i1 < NMCC) && (i2 < NMCC))  return i1 - i2;
   if (i1 < NMCC)  return -1;
   if (i2 < NMCC)  return  1;
   return StrCm (s1, s2);
}

void Song::CtlClean ()
// compact _f.ctl[] to just used n shown ones.  sorted nice
{ ubyte ncc, c, d, t, tc, cm [128];
  TStr   cc [128];
  ubyt4  e;
  TrkEv *ev;
TRC("CtlClean");
   MemSet (cc, 0, sizeof (cc));        // default to 'don't got it'
   for (t = 0;  t < _f.trk.Ln;  t++)   // find used ctls we DO got
      for (ev = _f.trk [t].e, e = 0;  e < _f.trk [t].ne;  e++)
         if ((tc = ev [e].ctrl) & 0x80)  cc [tc & 0x7F][0] = 'x';
   for (ncc = c = 0;  c < _f.ctl.Ln;  c++)  if ((cc [c][0]) || _f.ctl [c].sho) {
      StrFmt (cc [ncc++], "`s`s", _f.ctl [c].sho ? "+" : "", _f.ctl [c].s);
   }
   Sort (cc, ncc, sizeof (cc [0]), CtlCmp);
// build map of old=>new (_f.ctl[]=>cc[])
   for (c = 0;  c < _f.ctl.Ln;  c++)  for (d = 0;  d < ncc;  d++)
      if (! StrCm (_f.ctl [c].s, (cc [d][0] == '+') ? & cc [d][1] : cc [d]))
         {cm [c] = d;   break;}
   for (t = 0;  t < _f.trk.Ln;  t++)   // restamp ctl evs
      for (ev = _f.trk [t].e, e = 0;  e < _f.trk [t].ne;  e++)
         if ((tc = ev [e].ctrl) & 0x80)  ev [e].ctrl = 0x80 | cm [tc & 0x7F];
   _f.ctl.Ln = ncc;                    // rebuild _f.ctl[]
   for (c = 0;  c < ncc;  c++) {
      if (cc [c][0] == '+')
            {_f.ctl [c].sho = true;    StrCp (_f.ctl [c].s, & cc [c][1]);}
      else  {_f.ctl [c].sho = false;   StrCp (_f.ctl [c].s,   cc [c]);}
   }
   ReCtl ();
TRC("CtlClean end");
}


static int SigCmp (void *p1, void *p2)  // ..._sig sortin (by .time)
{ ubyt4 t1 = *((ubyt4 *)p1), t2 = *((ubyt4 *)p2);
   return t1 - t2;
}

ubyt4 Song::ReEv ()
// redo _f.ctl, _f.tpo,_f.tSg,_f.kSg, _cch given mod'd _f.trk[].e[]
{ ubyte t, d, c, tc;
  ubyt4 e, p, mint = 9999*M_WHOLE, maxt = 0, bard, tm;
  TStr  s;
  TrkEv *ev;
TRC("ReEv  rebuild _f.ctl, _f.tpo,_f.tSg,_f.kSg, _cch");
   CtlClean ();
   _cch.Ln = _f.tSg.Ln = _f.kSg.Ln = _f.tpo.Ln = 0;
   for (t = 0;  t < Up.rTrk;  t++) {
      for (d = _f.trk [t].dev, c = _f.trk [t].chn, ev = _f.trk [t].e,
           e = 0;  e < _f.trk [t].ne;  e++) {
         if ((tc = ev [e].ctrl) & 0x80) {
            StrCp (s, _f.ctl [tc & 0x7F].s);
            if (! StrCm (s, CC("Tmpo")))  if (! _f.tpo.Full ()) {
               p = _f.tpo.Ins ();
               _f.tpo [p].time = ev [e].time;
               _f.tpo [p].val  = ev [e].valu | (ev [e].val2 << 8);
            }
            if (! StrCm (s, CC("TSig")))  if (! _f.tSg.Full ()) {
               p = _f.tSg.Ins ();
               _f.tSg [p].time = ev [e].time;
               _f.tSg [p].num  = ev [e].valu;
               _f.tSg [p].den  = 1 << (ev [e].val2 & 0x0F);
               _f.tSg [p].sub  = 1 +  (ev [e].val2 >> 4);
            }
            if (! StrCm (s, CC("KSig")))  if (! _f.kSg.Full ()) {
               p = _f.kSg.Ins ();
               _f.kSg [p].time = ev [e].time;
               _f.kSg [p].key  = ev [e].valu;
               _f.kSg [p].flt  = ev [e].val2 & 0x80;
               _f.kSg [p].min  = ev [e].val2 & 0x01;
            }
            for (p = 0;  p < _cch.Ln;  p++)
               if ((_cch [p].dev == d) && (_cch [p].chn == c) &&
                                          (_cch [p].ctl == tc))  break;
            if ((p >= _cch.Ln) && (! _cch.Full ())) {
               p = _cch.Ins ();
               _cch [p].dev = d;   _cch [p].chn = c;   _cch [p].ctl = tc;
            }
         }
         else {
            if (ev [e].time <  mint)   mint = ev [e].time;
            if (ev [e].time > _tEnd)  _tEnd = ev [e].time + 1;
         }
      }                                // mint, _tEnd are time range of NOTEs
   // calc maxt
      if (_f.trk [t].ne && (_f.trk [t].e [_f.trk [t].ne-1].time > maxt))
         maxt =             _f.trk [t].e [_f.trk [t].ne-1].time;
   }                                   // maxt of ALL evs (last bar to SHOW)
TRC(" sort tsig,ksig by time");
   Sort (_f.tpo.Ptr (), _f.tpo.Ln, _f.tpo.Siz (), SigCmp);
   Sort (_f.tSg.Ptr (), _f.tSg.Ln, _f.tSg.Siz (), SigCmp);
   Sort (_f.kSg.Ptr (), _f.kSg.Ln, _f.kSg.Siz (), SigCmp);
   if (! _f.tpo.Ln)                    // every song needs tempo to keep edits
      for (t = 0;  t < Up.rTrk;  t++)  if (TDrm (t)) {
         EvIns (t, 0);
         _f.trk [t].e [0].time = 0;
         _f.trk [t].e [0].ctrl = CCUpd (CC("tmpo"), t);
         _f.trk [t].e [0].valu = 120;
         _f.trk [t].e [0].val2 = 0;
         _f.trk [t].e [0].x    = 0;
         break;
      }
// make sure TSig times are on bar boundaries (for safety)
   tm = 0;   bard = M_WHOLE;
   for (p = 0;  p < _f.tSg.Ln;  p++) { // trunc to bar dur
      tm = _f.tSg [p].time = ((_f.tSg [p].time - tm) / bard * bard) + tm;
      bard = M_WHOLE * _f.tSg [p].num / _f.tSg [p].den;
   }

// set _f.tSg[].bar,
   if (_f.tSg.Ln)
      _f.tSg [0].bar = (ubyt2)(1 + _f.tSg [0].time / M_WHOLE);     // 4/4
   for (p = 1;  p < _f.tSg.Ln;  p++)  _f.tSg [p].bar =
      (ubyt2)(_f.tSg [p-1].bar +
      (_f.tSg [p].time - _f.tSg [p-1].time) /
                               (M_WHOLE / _f.tSg [p-1].den * _f.tSg [p-1].num));
// bump maxt w _f.chd,etc;  align maxt up to next bar boundary
   if (_f.lyr.Ln && ((tm = _f.lyr [_f.lyr.Ln-1].time) > maxt))  maxt = tm;
   if (_f.chd.Ln && ((tm = _f.chd [_f.chd.Ln-1].time) > maxt))  maxt = tm;
   if (_f.cue.Ln && ((tm = _f.cue [_f.cue.Ln-1].time) > maxt))  maxt = tm;
   if (_f.cue.Ln && ((tm = _f.cue [_f.cue.Ln-1].tend) > maxt))  maxt = tm;
   _bEnd = Tm2Bar (maxt);   maxt = Bar2Tm (_bEnd+1);
   StrFmt (Up.bars, "`d", _bEnd);   emit sgUpd ("bars");

// now make sure KSig times are on bar boundaries (for safety)
   for (p = 0;  p < _f.kSg.Ln;  p++)  _f.kSg [p].time =
                                              Bar2Tm (Tm2Bar (_f.kSg [p].time));
TRC("ReEv end mint=`d", mint);
   return mint;
}


ubyte Song::CCUpd (char *cSt, ubyte t)
// got a rec ev w cSt so ( without doin a full ReEv() )
// lookup _f.ctl pos given ccStr,t;  update _f.ctl,dvt.CCMap,_cch if needed
{ ubyte c, cc;
  ubyt2 ch;
   for (c = 0;  c < _f.ctl.Ln;  c++)  if (! StrCm (_f.ctl [c].s, cSt))  break;
   if (c >= _f.ctl.Ln) {
TRC("CCUpd  new! cSt=`s t=`d => c=`d", cSt, t, c);
      if (_f.ctl.Full ()) {
         StrCp (cSt, CC("."));
TRC("   out of _f.ctl spots :(");
         return 0;
      }
      _f.ctl.Ln++;
      _f.ctl        [_f.ctl.Ln-1].sho = StrCm (cSt, CC("hold")) ? false : true;
      StrCp (_f.ctl [_f.ctl.Ln-1].s, cSt);
      ReCtl ();
      if (StrCm (cSt, CC("hold")) == 0)  ReDo ();     // ...argh
   }
   cc = 0x80 | c;

// ins into _cch[] if dev/chn/ctl is new
   for (ch = 0;  ch < _cch.Ln;  ch++)
      if ( (_cch [ch].dev == _f.trk [t].dev) &&
           (_cch [ch].chn == _f.trk [t].chn) &&
           (_cch [ch].ctl == cc) )  break;
   if ((ch >= _cch.Ln) && (! _cch.Full ())) {
      _cch.Ins ();                      _cch [ch].dev = _f.trk [t].dev;
      _cch [ch].chn = _f.trk [t].chn;   _cch [ch].ctl = cc;
   }
   return cc;
}


//______________________________________________________________________________
void Song::EvDel (ubyte t, ubyt4 p, ubyt4 ne)
// kill off some events in a track and adj other trk[].e's
// caller NEEDS to verify p..p+ne are w/in trk's e[]
{ ubyt4 pAbs;
   if (! ne)                return;
   pAbs = & _f.trk [t].e [p] - _f.ev;
   RecDel (_f.ev, _f.nEv, sizeof (TrkEv), pAbs, ne);
   for (ubyte i = 0; i < _f.trk.Ln; i++)
      if ((i != t) && (_f.trk [i].e >  _f.trk [t].e))  _f.trk [i].e -= ne;
   _f.trk [t].ne -= ne;   _f.nEv -= ne;
}


bool Song::EvIns (ubyte t, ubyt4 p, ubyt4 ne)
// if room, ins new spots;  bump following _f.trk[].e's
{ ubyt4 pAbs;
   if (! ne)                    return true;
   if (_f.nEv + ne > _f.maxEv)  return false;   // no rooom :(
   pAbs = & _f.trk [t].e [p] - _f.ev;
   RecIns (_f.ev, _f.nEv += ne, sizeof (TrkEv), pAbs, ne);
   for (ubyte i = 0; i < _f.trk.Ln; i++)
      if ((i != t) && (_f.trk [i].e >= _f.trk [t].e))  _f.trk [i].e += ne;
   _f.trk [t].ne += ne;
   return true;
}


void Song::EvInsT (ubyte t, TrkEv *ev)
// insert ev based on it's time;  bump nb/nn too if rec trk
// look for time, scoot ntDn to last of evs w matchin time
{ ubyt4 ne, p, x;
  TrkEv *e;
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  p < ne;  p++)  if (e [p].time > ev->time)  break;
   if (ENTUP (ev)) {
      x = p;
      while (x && (e [x-1].time == ev->time)) {
         --x;
         if ((ENTDN (& e [x])) && (e [x].ctrl == ev->ctrl)) {p = x;   break;}
      }
   }
   if (EvIns (t, p)) {
      MemCp (& e [p], ev, sizeof (TrkEv));
      if ((t >= Up.rTrk) && (! (ev->ctrl & 0x0080))) {
         if (ev->valu & 0x80)  {_f.trk [t].nn++;   _f.trk [t].nb++;}
         else                                      _f.trk [t].nb--;
//DBG("EvInsT1 t=`d nn=`d nb=`d", t, _f.trk [t].nn, _f.trk [t].nb);
      }
   }
}


void Song::EvInsT (ubyte t, MidiEv *ev)
// same as other EvInsT but w MidiEv
{ ubyt4 ne, p, x;
  TrkEv *e;
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  p < ne;  p++)  if (e [p].time > ev->time)  break;
   if (ENTUP (ev)) {
      x = p;
      while (x && (e [x-1].time == ev->time)) {
         --x;
         if ((ENTDN (& e [x])) && (e [x].ctrl == ev->ctrl)) {p = x;   break;}
      }
   }
   if (EvIns (t, p)) {
// TStr  s, s2;
//if (ENOTE (ev))  TRC("EvInsT tr=`d p=`d/`d tm=`s `s",
//t, p, tne+1, TmSt (s, ev->time), MNt2Str (s2, ev));
//else             TRC("EvInsT tr=`d p=`d/`d tm=`s `s $`02x",
//t, p, tne+1, TmSt (s, ev->time), _f.ctl [ev->ctrl & 0x007F].s, ev->valu);
      e [p].time = ev->time;   e [p].ctrl = (ubyte) ev->ctrl;
      e [p].valu = ev->valu;   e [p].val2 = ev->val2;   e [p].x = 0;
      if ((t >= Up.rTrk) && (! (ev->ctrl & 0x0080))) {
         if (ev->valu & 0x80)  {_f.trk [t].nn++;   _f.trk [t].nb++;}
         else                                      _f.trk [t].nb--;
//DBG("EvInsT2 t=`d nn=`d nb=`d", t, _f.trk [t].nn, _f.trk [t].nb);
      }
   }
}


void Song::NtIns (ubyte t, ubyt4 tm, ubyt4 te, ubyte c, ubyte v)
{ TrkEv ed, eu;
   ed.time = tm;   eu.time = te;   ed.ctrl = eu.ctrl = c;
   ed.valu = 0x80 | v;   eu.valu = 64;   ed.val2 = eu.val2 = 0;
   EvInsT (t, & ed);   EvInsT (t, & eu);
}


bool Song::TxtIns (ubyt4 t, char *s, Arr<TxtRow,MAX_LYR> *ta, char cue)
{ ubyt4 p, d;
  char *c;
   if (ta->Full ())  return false;     // no room
   for (p = 0;  p < ta->Ln;  p++)  if ((*ta) [p].time >= t) break;
   ta->Ins (p);
   (*ta) [p].time = t;
   if ((*s == '/') && cue)             // cues have the /dur thingy
         {d = Str2Int (& s [1], & c);
          (*ta) [p].tend = t + d;   StrCp ((*ta) [p].s, c);}
   else  {(*ta) [p].tend = 0;       StrCp ((*ta) [p].s, s);}
   return true;
}


ubyt4 Song::NtHit (ubyte t, ubyt4 tmn, ubyt4 tmx, ubyte nmn, ubyte nmx)
// see if any notes in trk t are w/in tm,nt box, if so return 1st hit's p+1 / 0
{ ubyt4  p, q;
  TrkEv *e;
  ubyt4 ne, tm;
  ubyte nt;
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  p < ne;  p++)  if (ENTDN (& e [p])) {      // all ntDns in t
      nt = e [p].ctrl;   if ((nt < nmn) || (nt > nmx))  continue;
   // find assoc ntUp
      for (q = p+1;  q < ne;  q++)  if (ENTUP (& e [q]) &&
                                                (e [q].ctrl == nt))  break;
      if (q < ne) {
         tm = e [p].time;   if ((tm >= tmn) && (tm <= tmx))  return p+1;
         tm = e [q].time;   if ((tm >= tmn) && (tm <= tmx))  return p+1;
      }
   }
   return 0;
}


//------------------------------------------------------------------------------
void Song::NtUpd (char *tnf)           // set fingering, handswap, or del
{ ubyte t, tc;
  ubyt4 n, p, ne;
  ubyte f;
  char *s, h;
  bool  ok;
  TrkNt *nt;
  TrkEv  dn, up, *e;
   t  = (ubyte) Str2Int (tnf, & s);
   n  =         Str2Int (s,   & s);
   f  = (ubyte) Str2Int (s);
//Dump (true);
   nt = & _f.trk [t].n [n];
   if (nt->dn != NONE)  MemCp (& dn, & _f.trk [t].e [nt->dn], sizeof (dn));
   if (nt->up != NONE)  MemCp (& up, & _f.trk [t].e [nt->up], sizeof (up));
TRC("t=`d n=`d/`d f=`d dn=`d up=`d", t, n, _f.trk [t].nn, f, nt->dn, nt->up);
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
      _f.trk [t].e [p].val2 |= f;
   }
   ReDo ();
}


void Song::NtDur (char *arg)           // set note duration (end time)
{ char *s;
  ubyt4 ap, ac, as, tm, t1, tMx, p, ne;
  sbyt2 y2;
  ubyte t, nt;
  SymDef *sy;
  TrkEv  *e, up;
   ap = Str2Int (arg, & s);   ac =        Str2Int (s, & s);
   as = Str2Int (s,   & s);   y2 = (sbyt2)Str2Int (s, & s);
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


void Song::NtHop (char *arg)           // move note time,dur to new key
{ char *s;
  ubyt4 ap, ac, as, p, ne;
  sbyt2 x2;
  ubyt2 nx;
  ubyte t, dnt;
  SymDef *sy;
  TrkNt  *n;
  TrkEv  *e, dn, up;
  TStr    s1, s2, s3;
   ap = Str2Int (arg, & s);   ac = Str2Int (s, & s);
   as = Str2Int (s,   & s);   x2 = (sbyt2)Str2Int (s, & s);
  PagDef *pg = & _pag [ap];
  ColDef  co;
   MemCp (& co, & pg->col [ac], sizeof (co));
   sy = & co.sym [as];   t = sy->tr;   n = & _f.trk [t].n [sy->nt];
   e = _f.trk [t].e;   ne = _f.trk [t].ne;

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


typedef struct {ubyte t;   ubyt4 p;} TPDef;

int TPCmp (void *a1, void *a2)         // by t,p desc
{ TPDef *p1 = (TPDef *)a1, *p2 = (TPDef *)a2;
  int t;
   if ((t = p2->t - p1->t))   return t;
        t = p2->p - p1->p;    return t;
}

void Song::RcMov (char *arg)           // move rect of notes to RH,LH,bg,kill
{ char *s, *c;                         // if nondrag, insert a new note
  ubyt4 ap, ac, p, p1, tm, t1, t2, tMx, dBt, dSb, ne, nn, i, nDel, nIns;
  sbyt2 x1, y1, x2, y2, tp;
  ubyt2 nx, br;
  ubyte tr, tr1, tD, tR, tL, vDn, vUp, nt, nSb, bt;
  TStr  s1;
  SymDef *it;
  TrkEv  *e, ev, *ins, up, dn;
  TPDef          *del;
  TrkNt  *n;

// parse our dang string args
   ap =         Str2Int (arg, & s);   ac =         Str2Int (s, & s);
   x1 = (sbyt2) Str2Int (s,   & s);   y1 = (sbyt2) Str2Int (s, & s);
   x2 = (sbyt2) Str2Int (s,   & s);   y2 = (sbyt2) Str2Int (s, & s);
   while (*s == ' ')  s++;

  PagDef *pg = & _pag [ap];
  ColDef  co;
   MemCp (& co, & pg->col [ac], sizeof (co));
   nx = Nt2X (co.nMn, & co);
//DBG("ap=`d ac=`d x1=`d y1=`d x2=`d y2=`d s='`s'",ap, ac, x1, y1, x2, y2, s);

   if (*s == '\0') {                   // just gonna ins a note
      if (x1 >= Nt2X (co.nMx+1, & co))
         {Hey (CC("sorry, can't do drums yet"));   return;}

      tm = Y2Tm (y1, & co);                 // y=> time=> round to subbeat
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


//______________________________________________________________________________
void Song::TrkSnd (ubyt4 snew)
{ ubyte d;
   ChkETrk ();
   d = _f.trk [Up.eTrk].dev;
   if (TDrm (Up.eTrk)) {               // change drum sound?
      Hey (CC("Can't edit GM DrumMap :("));
//    _f.trk [_eTrk].snd = snew;
   }
   else {                              // must be melodic sound
      _f.trk [Up.eTrk].snd = snew;   SetChn (Up.eTrk);   ReTrk ();
   }
}


void Song::NewGrp (char *gr)           // slam to a new sound picked by gui
{ TStr t;
TRC("NewGrp `s r=`d", gr, Up.eTrk);
   ChkETrk ();
   StrCp (t, gr);   StrAp (t, CC("/"));
   if (! MemCm (gr, CC("Drum/"), 5))  for (ubyte i = 0; i < NMDrum; i++)
      if (! StrCm (& gr [5], MDGrp [MDrum [i].grp].sym))
         {StrAp (t, MDrum [i].sym);   break;}
   TrkSnd (Up.dvt [Up.dev [_f.trk [Up.eTrk].dev].dvt].SndID (t, true));
}


void Song::NewSnd (char *sn)           // slam to a new sound picked by gui
{ TStr  g;
  char *sl;
TRC("NewSnd `s r=`d", sn, Up.eTrk);
   ChkETrk ();
   StrCp (g, SndName (Up.eTrk));
   if (MemCm (g, CC("Drum/"), 5))  sl = StrCh (  g,     '/');
   else                            sl = StrCh (& g [5], '/');
   if (sl)  sl [1] = '\0';
   StrAp (g, sn);
   TrkSnd (Up.dvt [Up.dev [_f.trk [Up.eTrk].dev].dvt].SndID (g));
}


void Song::NewSnd (char ofs)
// step to new snd via devtyp snd list and offset type
{ ubyte d, t;
  ubyt4 snew;
   ChkETrk ();
   t = Up.dev [d = _f.trk [Up.eTrk].dev].dvt;
   if (TDrm (Up.eTrk))
      {Hey (CC("Can't edit GM DrumMap"));   return;}
   if (! Up.dvt [t].SndNew (& snew, _f.trk [Up.eTrk].snd, ofs))
       Hey (CC("can't change sound"));
}


//______________________________________________________________________________
void Song::NewDev (char *dv)                // slam trk to new dev picked by gui
{ char *p, cmap [17];
  ubyte d, od, och, nch;
  ubyt4 t, to, t1, t2;
  TStr  snm;
  bool  got;
  TrkRow tr;
   ChkETrk ();   to = Up.eTrk;
TRC("NewDev t=`d `s", to, dv);              // set old trk,dev,chn
   if ((p = StrCh (dv, ' ')))  *p = '\0';   // dv is "devName devType" or +
   d = od = _f.trk [to].dev;   och = _f.trk [to].chn;
   if ( (   _f.trk [to].grp  && (! StrCm (dv, CC("+")))) || // no change?  scram
        ((! _f.trk [to].grp) && (! StrCm (dv, DevName ((ubyte)to)))) )  return;
   if      (! StrCm (dv, CC("+"))) {        // to grouped?
      if      (to == 0)
         {Hey (CC("can't + the 1st track"));   return;}
      else if ( (   TDrm ((ubyte)to)  && (! TDrm ((ubyte)to-1))) ||
                ((! TDrm ((ubyte)to)) &&    TDrm ((ubyte)to-1) ) )
         {Hey (CC("drum and melodic sounds can't share a channel"));   return;}
      d = _f.trk [to-1].dev;       nch = _f.trk [to-1].chn;
   }
   else if (TDrm ((ubyte)to))  nch = 9;
   else {                              // all melo chans of new dev in use?
      StrCp (cmap, CC("         x      "));
      for (t = 0;  t < Up.rTrk;  t++)
         if (! StrCm (DevName ((ubyte)t), dv))  cmap [_f.trk [t].chn] = 'x';
      for (nch = 0; nch < 16; nch++)  if (cmap [nch] != 'x')  break;
      if (nch == 16)
         {Hey (CC("That device's channels are all in use"));   return;}
   }

// get prev snd name as str
   if (nch != 9) {
      if (_f.trk [to].snd != SND_NONE)
            StrCp (snm, Up.dvt [Up.dev [d].dvt].Snd (_f.trk [to].snd)->name);
      else  StrCp (snm, CC("Piano/AcousticGrand"));
   }

// t1-t2 covers my group range...
   t1 = t2 = to;
   while (                   _f.trk [t1  ].grp)   t1--;
   while ((t2+1 < Up.rTrk) && (_f.trk [t2+1].grp))  t2++;

// goin to ungrouped?  gotta scoot below old grp
   if (_f.trk [to].grp && StrCm (dv, CC("+"))) {
      if (to != t2) {                  // scoot if not already at bot
         MemCp (& tr,          & _f.trk [to], sizeof (TrkRow));
         MemCp (& _f.trk [to], & _f.trk [t2], sizeof (TrkRow));
         MemCp (& _f.trk [t2], & tr,          sizeof (TrkRow));
         Up.eTrk = (ubyte)(to = t2);
      }
      _f.trk [to].grp = false;
      _f.trk [to].snd = _f.trk [t1].snd;    // ungroup,set snd to orig top trk
      t1 = t2 = to;
   }

// will old dev be gonzo?
   NotesOff ();
   for (got = false, t = 0;  t < Up.rTrk;  t++)  if ((t < t1) || (t > t2))
      if (od == _f.trk [t1].dev)  got = true;
   if (! got)  ShutDev (_f.trk [t1].dev);

   if (StrCm (dv, CC("+")))  d = OpenDev (dv);   // unless +, get new one

// redo cch[]
   for (t = 0;  t < _cch.Ln;  t++)  if ((_cch [t].dev == od) &&
                                        (_cch [t].chn == och))
                                       {_cch [t].dev = d;   _cch [t].chn = nch;}
   _f.trk [to].dev = d;
   _f.trk [to].chn = nch;
   _f.trk [to].grp = (StrCm (dv, CC("+")) ? false : true);

// new sndid since new dev :/
   got = false;                        // drum syn check
   if (nch != 9)  _f.trk [t1].snd = Up.dvt [Up.dev [d].dvt].SndID (snm);
   else
      got = true;                      // hello syn

// setup all trks of grp
   for (t = t1+1; t <= t2; t++)
      {_f.trk [t].dev = _f.trk [t1].dev;   _f.trk [t].chn = _f.trk [t1].chn;
       if (! got)  _f.trk [t].snd = _f.trk [t1].snd;}
// redo drum sounds fer syn (toss the useless _f.mapD n DrumExp recalcs
   if (got)  {DrumCon ();   _f.mapD.Ln = 0;   DrumExp ();}
   else      {SetBnk ();   SetChn ();}      // just in case :/
   ReDo ();
TRC("NewDev end");
}


//______________________________________________________________________________
void Song::TrkSplt ()
// just split the one ? track into 1st stab at LH,RH split on 3e
{ ubyte t, t2;
  ubyt4 p, p2;
  TrkEv *e, ev;
   for (t = 0;  t < Up.rTrk;  t++)  if ((! TDrm (t)) && TLrn (t))  break;
   if (t >= Up.rTrk)  {Hey (CC("pick a track to learn/split"));   return;}

// insert new LH in t+1 and setup all the track junk
   if (TrkIns (t2 = t+1, CC("LH"), NULL) == MAX_TRK)
                      {Hey (CC("couldn't insert LH track :("));   return;}
   _f.trk [t ].ht  = 'R';
   _f.trk [t2].grp = true;
   _f.trk [t2].shh = false;
   _f.trk [t2].lrn = true;
   _f.trk [t2].ht  = 'L';
   _f.trk [t2].dev = _f.trk [t].dev;   _f.trk [t2].chn = _f.trk [t].chn;
   _f.trk [t2].snd = _f.trk [t].snd;   _f.trk [t2].drm = _f.trk [t].drm;
   _f.trk [t2].vol = _f.trk [t].vol;   _f.trk [t2].pan = _f.trk [t].pan;
   for (e = _f.trk [t].e, p = p2 = 0;  p < _f.trk [t].ne;) {
      if (e [p].ctrl <= MKey (CC("3e"))) {
         MemCp (& ev, & e [p], sizeof (ev));
         EvDel (t, p);   EvIns (t2, p2);   MemCp (& _f.trk [t2].e [p2++],
                                                  & ev, sizeof (ev));
      }
      else  p++;
   }
   ReDo ();
}


//______________________________________________________________________________
void Song::TrkDel (ubyte t)
// kill any evs;  maybe close dev;  adjust trk refs
{ ubyte e;
  ubyt4 i;
  bool  got;
   if (t >= Up.rTrk)  return;          // can't :/
TRC("TrkDel");
   NotesOff ();
   if (((ubyte)(t+1) < Up.rTrk) && _f.trk [t+1].grp && (! _f.trk [t].grp)) {
      _f.trk [t+1].grp = false;        // re-start group on next track down
      _f.trk [t+1].dev = _f.trk [t].dev;
      _f.trk [t+1].chn = _f.trk [t].chn;
   }
   EvDel (t, 0, _f.trk [t].ne);
   for (got = false, e = 0;  e < Up.rTrk;  e++)
      if ((e != t) && (_f.trk [e].dev == _f.trk [t].dev))
         {got = true;   break;}
   if (! got) {
      for (i = 0;  i < _cch.Ln;) {     // kill _cch entries for dev
         if (_cch [i].dev == _f.trk [t].dev)  _cch.Del (i);
         else                                 i++;
      }
      ShutDev (_f.trk [t].dev);
   }
   _f.trk.Del (t);
   if (Up.eTrk >  t)        Up.eTrk--;      // adj all .trk refs
   if (Up.eTrk >= Up.rTrk)  Up.eTrk--;
}


ubyte Song::TrkIns (ubyte t, char *name, char *snd)
// Chrd uses t,name,snd
// +    uses t,(NULL,NULL)
{  if (_f.trk.Full ())  {DBG("too many tracks");   return MAX_TRK;}
TRC("TrkIns t=`d/`d nm=`s sn=`s", t, _f.trk.Ln, name?name:"", snd?snd:"");
   if (t >= _f.trk.Ln)  t = (ubyte)(_f.trk.Ln);

// init most _f.trk fields to 0;  set .name, .e
   _f.trk.Ins (t);                     // MemSets _f.trk [t] to 0
   if (name)  StrCp (_f.trk [t].name, name);
   _f.trk [t].e   = t ? (_f.trk [t-1].e + _f.trk [t-1].ne) : _f.ev;
   if (snd)  PickDev (t, *snd ? snd : CC("Piano/AcousticGrand"));
   else {
      _f.trk [t].dev = _f.trk [t+1].dev;   _f.trk [t].chn = _f.trk [t+1].chn;
      _f.trk [t].snd = _f.trk [t+1].snd;   _f.trk [t].drm = _f.trk [t+1].drm;
      _f.trk [t].vol = _f.trk [t+1].vol;   _f.trk [t].pan = _f.trk [t+1].pan;
      _f.trk [t].grp = _f.trk [t+1].grp;   _f.trk [t+1].grp = true;
      _f.trk [t].shh = _f.trk [t+1].shh;   _f.trk [t].lrn = _f.trk [t+1].lrn;
      _f.trk [t].ht  = _f.trk [t+1].ht;
   }
   ReTrk ();
TRC("TrkIns: t=`d", t);
   return t;
}


void Song::TrkEd (char *arg)
// it's kind of a pain to keep groups ok in up/dn
{ ubyte t = ChkETrk (), tb, te, dtr, ntr;
  ubyt4 p, cb, ce, cd, tt, maxt;
  char  op = *arg;
  TrkRow tr [MAX_TRK];
   if ((op == '*') || (op == '-')) {
      cb = ce = cd = maxt = 9999*M_WHOLE;
      for (p = 0;  p < _f.cue.Ln;  p++) {
         if (! StrCm (_f.cue [p].s, CC("{")))
            {if (cb < maxt)  {Hey (CC("multiple { cues"));   return;}
             cb = _f.cue [p].time;}
         if (! StrCm (_f.cue [p].s, CC("}")))
            {if (ce < maxt)  {Hey (CC("multiple } cues"));   return;}
             ce = _f.cue [p].time;}
         if (! StrCm (_f.cue [p].s, CC("^")))
            {if (cd < maxt)  {Hey (CC("multiple ^ cues"));   return;}
             cd = _f.cue [p].time;}
      }
      if (cb == maxt)  {Hey (CC("there's no { cue"));   return;}
      if (ce == maxt)  {Hey (CC("there's no } cue"));   return;}
      if (cd == maxt)  {Hey (CC("there's no ^ cue"));   return;}
TStr d1,d2,d3;
DBG("cb=`s ce=`s cd=`s", TmSt(d1,cb), TmSt(d2,ce), TmSt(d3,cd));
      if (op == '-') {                 // TODO check overlap if cd<cb
      // evs
         for (t = 0;  t < _f.trk.Ln;  t++)
            for (p = 0;  p < _f.trk [t].ne;  p++)
               if (_f.trk [t].e [p].time >= cb)
                  {_f.trk [t].e [p].time += cd;   _f.trk [t].e [p].time -= cb;}
      // markers
         for (p = 0;  p < _f.lyr.Ln;  p++)  if (_f.lyr [p].time >= cb)
            {_f.lyr [p].time += cd;   _f.lyr [p].time -= cb;}
         for (p = 0;  p < _f.chd.Ln;  p++)  if (_f.chd [p].time >= cb)
            {_f.chd [p].time += cd;   _f.chd [p].time -= cb;}
         for (p = 0;  p < _f.cue.Ln;  p++) {
            if (_f.cue [p].time >= cb)
               {_f.cue [p].time += cd;   _f.cue [p].time -= cb;}
            if (_f.cue [p].tend >= cb)
               {_f.cue [p].tend += cd;   _f.cue [p].tend -= cb;}
         }
      }
      else { // '*'                    ...TODO check for overlap if cd>ce
      // evs
         for (t = 0;  t < _f.trk.Ln;  t++)
            for (p = 0;  p < _f.trk [t].ne;  p++)
               if ((tt = _f.trk [t].e [p].time) >= cb)
                  _f.trk [t].e [p].time = (tt < ce) ?
                           (cb + (tt-cb) * (cd-cb) / (ce-cb)) : (tt + cd - ce);
      // markers
         for (p = 0;  p < _f.lyr.Ln;  p++)  if ((tt = _f.lyr [p].time) >= cb)
            _f.lyr [p].time = (tt < ce) ? (cb + (tt-cb) * (cd-cb) / (ce-cb))
                                        : (tt + cd - ce);
         for (p = 0;  p < _f.chd.Ln;  p++)  if ((tt = _f.chd [p].time) >= cb)
            _f.chd [p].time = (tt < ce) ? (cb + (tt-cb) * (cd-cb) / (ce-cb))
                                        : (tt + cd - ce);
         for (p = 0;  p < _f.cue.Ln;  p++) {
            if ((tt = _f.cue [p].time) >= cb)
               _f.cue [p].time = (tt < ce) ? (cb + (tt-cb) * (cd-cb) / (ce-cb))
                                           : (tt + cd - ce);
            if ((tt = _f.cue [p].tend) >= cb)
               _f.cue [p].tend = (tt < ce) ? (cb + (tt-cb) * (cd-cb) / (ce-cb))
                                           : (tt + cd - ce);
         }
      }
   // rebuild it aaaall
      ReDo ();   SetLp ('.');
      return;
   }
   tb = te = t;                        // find group's full range
   while ( tb                         && _f.trk [tb  ].grp)  tb--;
   while (((ubyte)(te+1) < _f.trk.Ln) && _f.trk [te+1].grp)  te++;
   if      (op == 'x')  TrkDel  (t);
   else if (op == '+')  TrkIns  (t);
   else if (op == 's')  TrkSplt ();
   else if (op == 'u') {
      if (t == tb) {                   // no group or top trk of grp
         if (tb == 0)  {Hey (CC("already at the top"));   return;}
         dtr = tb-1;   ntr = 1;   while (_f.trk [dtr].grp) {dtr--;  ntr++;}
//DBG("Ln=`d tb=`d te=`d dtr=`d ntr=`d", _f.trk.Ln, tb, te, dtr, ntr);
         MemCp (tr,                & _f.trk [dtr], sizeof (TrkRow) * ntr);
         MemCp (& _f.trk [dtr],    & _f.trk [tb],  sizeof (TrkRow) * (te-tb+1));
         MemCp (& _f.trk [te-ntr+1], tr,           sizeof (TrkRow) * ntr);
         Up.eTrk -= ntr;
      }
      else {                           // w/in a group - scoot 1:  t<=>t-1
         MemCp (tr,             & _f.trk [t-1], sizeof (TrkRow));
         MemCp (& _f.trk [t-1], & _f.trk [t],   sizeof (TrkRow));
         MemCp (& _f.trk [t],   tr,             sizeof (TrkRow));
         if (t-1 == tb) {              // bump'd it into top of group's trk
            _f.trk [t-1].grp = false;   _f.trk [t].grp = true;
            _f.trk [t  ].snd = _f.trk [t-1].snd; // restore orig top trk snd
            _f.trk [t-1].snd = tr     [0  ].snd;
         }
         Up.eTrk--;
      }
   }
   else if (op == 'd') {
      if (t == tb) {                   // no group or top trk of grp
         if (te == Up.rTrk-1)  {Hey (CC("already at the bottom"));   return;}
         dtr = te+1;   ntr = 1;
         while (((ubyt4)(dtr+ntr) < _f.trk.Ln) && _f.trk [dtr+ntr].grp)  ntr++;
         MemCp (tr,                & _f.trk [dtr], sizeof (TrkRow) * ntr);
         MemCp (& _f.trk [tb+ntr], & _f.trk [tb],  sizeof (TrkRow) * (te-tb+1));
         MemCp (& _f.trk [tb],     tr,             sizeof (TrkRow) * ntr);
         Up.eTrk += ntr;
      }
      else {                           // w/in a group - scoot 1: t<=>t+1
         if (t  == Up.rTrk-1)  {Hey (CC("already at the bottom"));   return;}
         MemCp (tr,             & _f.trk [t+1], sizeof (TrkRow));
         MemCp (& _f.trk [t+1], & _f.trk [t],   sizeof (TrkRow));
         MemCp (& _f.trk [t],   tr,             sizeof (TrkRow));
         Up.eTrk++;
      }
   }
   ReDo ();
}


ubyte Song::GetSct (TxtRow *sct)       // put sections of _f.lyr[] into arr[64]
{ ubyt4 i;
  ubyte ns = 0;
   MemSet (sct, 0, sizeof (TxtRow)*64);
   for (i = 0;  (ns < 64) && (i < _f.cue.Ln);  i++)
      if (_f.cue [i].s [0] == '(')
         MemCp (& sct [ns++], & _f.cue [i], sizeof (TxtRow));
TStr x;
for(i=0;i<ns;i++) TRC("sct[`d].s=`s .time=`d/`s",
i,sct[i].s,sct[i].time,TmSt(x,sct[i].time));
   return ns;
}


extern char *SongRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr);

void Song::TrkDr (char *arg)
{ ubyte nIt, j, k, t;
  TStr   it [64][4], s;
  ubyt4  i, ln, pLn;
  BStr   b;
  char  *p, *e, ud;
  TrkEv *e2;
  ColSep ss (arg, 5);
   NotesOff ();

// set it[nIt][0] w distinct sections
   nIt = 0;
   for (i = 0;  i < _f.cue.Ln;  i++)  if (_f.cue [i].s [0] == '(') {
      for (j = 0;  j < nIt;  j++)
         if (! StrCm (& _f.cue [i].s [1], it [j][0]))  break;
      if (j >= nIt) {
         StrCp (it [nIt][1], CC("(off)"));   StrCp (it [nIt][2], CC("(off)"));
                                             StrCp (it [nIt][3], CC("(off)"));
         StrCp (it [nIt++][0], & _f.cue [i].s [1]);
      }                             // ^ new section
   }

// search thru DSC lines n set a,b,f [1..3] w pata,patb,fill for section
   DscGet (CC("drumpat={"), b);   ln = 0;  pLn = 0;
   do {
      p = & b [pLn];
      if ((e = StrSt (p, CC("\n"))))  for (i = 0;  i < nIt;  i++) {
         StrFmt (s, "`s=", it [i][0]);
         if (! MemCm (p, s, StrLn (s))) {
            p += StrLn (s);
            MemCp (s, p, (e-p));   s [e-p] = '\0';
           ColSep ss (s, 3);
            StrCp (it [i][1], ss.Col [0]);   StrCp (it [i][2], ss.Col [1]);
                                             StrCp (it [i][3], ss.Col [2]);
         }
      }
   } while ((pLn = LinePos (b, ++ln)));

// now set new thingy n DscPut it
   for (j = 0;  j < nIt;  j++)  if (! StrCm (it [j][0], ss.Col [0]))  break;
   k = ss.Col [4][0] - '0';
   StrCp (it [j][k+1], ss.Col [1+k]);
   StrCp (b, CC("drumpat={\n"));
   for (j = 0;  j < nIt;  j++)  StrAp (b, StrFmt (s, "`s=`s `s `s\n",
                                   it [j][0], it [j][1], it [j][2], it [j][3]));
   StrAp (b, CC("}"));
   DscPut (b);

// now do it - make song.txt file with section patterns
  TxtRow m [64];
  bool  co;
  ubyte nm = 0;
  ubyt2 b1 = 1, br, bb, be;            // bars start at 1 not 0 !
  TStr  fn, pt [3];
  FDir  d;
  File  f;
   nm = GetSct (m);
   App.Path (fn, 'd');   StrAp (fn, CC("/4_queue/drumpat"));   d.Make (fn);
   StrAp (fn, CC("/drumpat.txt"));
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
   if (! StrCm (it [0][1], CC("(continue)")))  StrCp (it [0][1], CC("(off)"));
   for (i = 0;  i < nm;) {
   // j = section's pos in it[]
      for (j = 0;  j < nIt;  j++)  if (! StrCm (& m [i].s [1], it [j][0])) {
         for (k = 0;  k < 3;  k++) {   // load pattern in pt[]
            if (StrCm (it [j][k+1], CC("(off)")))
                         StrFmt (pt [k], "#Drum/`s/`s\n",
                                         (k<2) ? "main" : "fill", it [j][k+1]);
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
         for (j = 0;  j < nIt;  j++)  if (! StrCm (& m [i].s [1], it [j][0])) {
            co = StrCm (it [j][1], CC("(continue)")) ? false : true;
            break;
         }
      }
      while (co);

      for (br = bb;  br < be;  br++)
         f.Put (pt ["abababac" [(br - bb) % 8] - 'a']);
   }
   f.Put (CC("EndTrack\n"));
   f.Shut ();

// Txt2Song it, load it, and replace drumtrack
   StrCp (s, CC("txt2song "));   StrAp (s, fn);
   App.Run (s);
   Fn2Path (fn);   StrAp (fn, CC("/a.song"));
  STable st [TB_MAX];
   st [TB_DSC].Init (CC("Descrip:"), 1, MAX_DSC);
   st [TB_TRK].Init (CC("Track:")  , 4, MAX_TRK);
   st [TB_DRM].Init (CC("DrumMap:"), 7, MAX_DRM);
   st [TB_LYR].Init (CC("Lyric:")  , 2, MAX_LYR);
   st [TB_EVT].Init (CC("Event:")  , 2, MAX_EVT);
   if ((p = f.DoText (fn, & st, SongRec)))
      {TRC("  DoText err=`s", p);   return;}

   DrumCon ();
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TDrm (t))  break;
   if (t >= _f.trk.Ln)  return;

   for (i = 0;  i < _f.trk [t].ne;)    // keep ctrls / kill notes
      {if (_f.trk [t].e [i].ctrl & 0xFF80)  i++;   else EvDel (t, i);}

// realloc _f.ev etc
  ubyt4 ne = st [TB_EVT].NRow () - 2;  // omit 1st !TSig=
  TrkEv *olde = _f.ev;                 // old ev buf
   _f.maxEv = ne + _f.nEv+MAX_RCRD;    // new size
   _f.ev = new TrkEv [_f.maxEv];       // new ev buf
   MemCp (_f.ev, olde, _f.nEv * sizeof (TrkEv));
   delete olde;

// rebuild _f.trk[].e's
   for (_f.trk [0].e = _f.ev, i = 1;  i < _f.trk.Ln;  i++)
      _f.trk [i].e = _f.trk [i-1].e + _f.trk [i-1].ne;

// EvIns new notes into drumtrack
   e2 = & _f.trk [t].e [_f.trk [t].ne];
   EvIns (t, _f.trk [t].ne, ne);
   for (i = 0;  i < ne;  i++, e2++) {
      e2->time = Str2Tm (st [TB_EVT].Get (1+i, 0));
      StrCp (s,          st [TB_EVT].Get (1+i, 1));
      e2->ctrl = MDrm (s);
      ud = s [4];
      e2->valu = (ubyte)Str2Int (& s [5]) | ((ud == '_') ? 0x80 : 0);
      e2->val2 = (ud == '~') ? 0x80 : 0;
   }

// cleanup n TmHop
   Fn2Path (fn);   d.Kill (fn);    // comment to keep em
   DrumExp ();   ReDo ();
   for (t = 0;  t < nm;  t++)  if (! StrCm (& m [t].s [1], ss.Col [0]))  break;
   TmHop (m [t].time);
   _rcrd = true;
}


void Song::setCtl (char *arg)
{ char *s, *c;
  ubyte tr, cc, mc;
  ubyt4 tm, p;
  MidiEv e;
  TrkEv  ev;
// parse our dang string args: track, epos, time, ctl, val
   tr = (ubyte)Str2Int (arg, & s);   p = (ubyt4)Str2Int (s, & s);
   tm = (ubyt4)Str2Int (s, & s);
   while (*s == ' ')  s++;
   c = s;
   if ((s = StrCh (s, ' ')))  *s++ = '\0';
   while (*s == ' ')  s++;
TRC("setCtl tr=`d p=`d tm=`d ctl=`s val=`s", tr, p, tm, c, s);
   if (tr < 128) {
      EvDel (tr, p);
      if (! StrCm (c, CC("KILL")))
         {ReEv ();   _prac = true;   Pract ();   ReDo ();   return;}
   }
   else {                              // need SOME track to put it in...
      for (tr = 0;  tr < _f.trk.Ln;  tr++)  if (TLrn (tr))  break;
      if (tr >= _f.trk.Ln)
         {Hey (CC("no track to put control into"));   return;}
   }
   if ( (! StrCm (c, CC("ksig"))) ||       // these HAVE to go in drum trak, etc
        (! StrCm (c, CC("tsig"))) || (! StrCm (c, CC("tmpo"))) ) {
      for (tr = 0;  tr < _f.trk.Ln;  tr++)  if (TDrm (tr))  break;
      if (tr >= _f.trk.Ln)
         {Hey (CC("no drum track to put control into"));   return;}
      if (StrCm (c, CC("tmpo")))           // scoot tsig,ksig time to bar start
         tm = Bar2Tm (Tm2Bar (tm));
   }
   for (mc = 0;  mc < NMCC;  mc++)  if (! StrCm (c, MCC [mc].s))  break;
   if (MCC [mc].typ == 'x')  {CtlX2Val (& ev, c, s);
TRC("new valu=`d val2=`d", ev.valu, ev.val2);
                              e.valu = ev.valu;              e.val2 = ev.val2;}
   else                      {e.valu = (ubyte)Str2Int (s);   e.val2 = 0;}
   for (cc = 0;  cc < _f.ctl.Ln;  cc++)  if (! StrCm (c, _f.ctl [cc].s))  break;
TRC("cc=`d", cc);
   e.time = tm;   e.ctrl = 0x80|cc;
   EvInsT (tr, & e);
   if (MCC [mc].typ == 'x')  ReEv ();
   _prac = true;   Pract ();   ReDo ();
}


void Song::DrMap (char *dr)
// set .din of a drum
{ ubyte t = ChkETrk(), d;
  TStr  s;
  ubyt4 p, ne;
  TrkEv    *e;
TRC("DrMap trk=`d drm=`s", t, dr);
   MemCp (s, & dr [0], 4);   s [4] = '\0';       _f.trk [t].din = MDrm (s);
// for main drum sound, gotta set all the evs
   MemCp (s, & dr [5], 4);   s [4] = '\0';   d = _f.trk [t].drm = MDrm (s);
   e = _f.trk [t].e;   ne = _f.trk [t].ne;
   for (p = 0;  p < ne;  p++)  if (! (e [p].ctrl & 0x80))  e [p].ctrl = d;
   DrumCon ();   DrumExp ();   ReDo ();
}


//______________________________________________________________________________
ubyt4 Song::NtDnNxt (ubyt4 tm)         // find next trk=? noteDn >= tm
{ ubyte t;
  ubyt4 p, ne, tIn = tm;
  bool  got = false;
  TrkEv *e;
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TLrn (t)) {
      for (e = _f.trk [t].e, ne = _f.trk [t].ne, p = 0;  p < ne;  p++)
         if (ENTDN (& e [p]) && (e [p].time >= tIn))  break;
      if (p < ne)  {if ((! got) || (e [p].time < tm))  tm = e [p].time;
                    got = true;}
   }
   return got ? (tm ? (tm-1) : tm) : tIn;
}

ubyt4 Song::NtDnPrv (ubyt4 tm)         // find prev trk=? noteDn+beat <= tm
{ ubyte t;
  ubyt4 p, pEnd, ne, tIn = tm;
  bool  got = false;
  TrkEv *e;
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TLrn (t)) {
      for (p = 0, pEnd = 0xFFFFFFFF, e = _f.trk [t].e, ne = _f.trk [t].ne;
           p < ne;  p++) {
         if (ENTDN (& e [p]) && (e [p].time <  tIn))  pEnd = p;
         if (ENTDN (& e [p]) && (e [p].time >= tIn))  break;
      }
      if (pEnd < ne)  {if ((! got) || (e [pEnd].time > tm))  tm = e [pEnd].time;
                       got = true;}
   }
   if (! got)  return tIn;
  TSgRow *ts = TSig (tm);
   return tm + M_WHOLE / ts->den;
}


static int LpCmp (void *p1, void *p2)  // by .t desc, .b desc, .tm
{ sbyt4 *i1 = (sbyt4 *)p1, *i2 = (sbyt4 *)p2, d;
   if ((d = i2 [2] - i1 [2]))  return d;
   if ((d = i2 [3] - i1 [3]))  return d;
   return   i1 [0] - i2 [0];
}

void Song::SetLp (char dir)
// i=init to worst loop;  .=refresh(but no TmHop)  <,>=hop  f=focus
{ ubyt4 p, x, xt, l, nl, nlB, tMn, tMx, in;
  bool  usex = true;                   // set _lrn.lp* to lp[x] ?
  TStr  ts, t2;
  struct {ubyt4 tm, te, t, b, mx;} lp [4096];
TRC("SetLp dir=`c _now=`s   loops:", dir, TmSt (ts, _now));
// gather loops;  pick start time of loop we're in based on _now
   nl = xt = 0;
   for (p = 0;  p < _f.cue.Ln;  p++)
      if (_f.cue [p].tend && (_f.cue [p].s [0]=='[')) {
         lp [nl].tm = _f.cue [p].time;   lp [nl].te = _f.cue [p].tend;
                                         lp [nl].t = lp [nl].b = lp [nl].mx = 0;
         for (x = 0;  x < _dn.Ln;  x++)  if ((_dn [x].time >= lp [nl].tm) &&
                                             (_dn [x].time <  lp [nl].te))
            lp [nl].mx++;
TRC("   `d `s-`s mx=`d",
nl, TmSt(ts, lp [nl].tm), TmSt(t2, lp [nl].te), lp [nl].mx);
   // usually want 2nd loop of an overlap cuz we're usually sittin at lpBgn
   // so 2nd time thru will set xt again
      if ((_now >= lp [nl].tm) && (_now <= lp [nl].te))  xt = lp [nl].tm;
      nl++;
   }

// cleanup any junky bugs - ones w weird counts or outside loops
   for (p = 0;  p < _f.bug.Ln;) {
      if (Str2Int (_f.bug [p].s) <= 0)  _f.bug.Del (p);
      else {
         for (in = l = 0;  l < nl;  l++)  if ((_f.bug [p].time >= lp [l].tm) &&
                                              (_f.bug [p].time <= lp [l].te))
            {in = 1;   break;}
         if (! in)  _f.bug.Del (p);
         else {
            if (Str2Int (_f.bug [p].s) > 9)  StrCp (_f.bug [p].s, CC("9"));
            p++;
         }
      }
   }
TRC(" xt=`s nl(all)=`d   bugs:", TmSt(ts, xt), nl);

// count up distinct times n bug totals (if hits >= 2)
   for (p = 0;  p < _f.bug.Ln;  p++)  if (Str2Int (_f.bug [p].s) >= 2) {
TRC("   `d tm=`s n=`s", p, TmSt (ts, _f.bug [p].time), _f.bug [p].s);
      for (l = 0;  l < nl;  l++)  if ((_f.bug [p].time >= lp [l].tm) &&
                                      (_f.bug [p].time <= lp [l].te)) {
         lp [l].t++;   lp [l].b += (Str2Int (_f.bug [p].s) - 1);
TRC("      put in lp=`d now t=`d b=`d", l, lp [l].t, lp [l].b);
      }
   }
   Sort (lp, nl, sizeof (lp[0]), LpCmp);    // by #times desc,#bugs desc,time
   nlB = nl;   while (nlB && lp [nlB-1].t == 0)  nlB--;
TRC("lp post sort n trim t=0s");
for (l = 0;  l < nlB;  l++)  TRC("   `d tb=`s te=`s t=`d b=`d",
l, TmSt(ts,lp [l].tm), TmSt(t2,lp [l].te), lp [l].t, lp [l].b);

   if (dir == 'i')  x = 0;             // init loop to worst (top o list)
   else {                              // refind loop havin xt for lpBgn
      for (x = 0;  x < nl;  x++)  if (lp [x].tm == xt)  break;
      if (x >= nl)                     // in rep'd area?
         {x = 0;   if (dir == 'f')  dir = '.';}  // aaand focus/un => .
TRC(" last lp#=`d/`d", x, nl);         // so just default to new worst loop
      if      (dir == '<')  {if (x)         x--;}
      else if (dir == '>')  {if (x+1 < nl)  x++;}
   // focus/un?  plow thru bug times again pickin min/max
      else if (dir == 'f') {
         tMn = M_WHOLE*9999;   tMx = 0;
         for (p = 0;  p < _f.bug.Ln;  p++) {
            xt = _f.bug [p].time;
            if ((xt >= lp [x].tm) && (xt <= lp [x].te) &&
                                     (Str2Int (_f.bug [p].s) >= 2))
               {if (xt < tMn)  tMn = xt;   if (xt > tMx)  tMx = xt;}
         }
         tMn = NtDnNxt ((tMn < M_WHOLE/4) ? 0 : (tMn - M_WHOLE/4));
         tMx = NtDnPrv (                         tMx + M_WHOLE/4 );
         if ((_lrn.lpBgn == lp [x].tm) && (_lrn.lpEnd == lp [x].te)) {
            _lrn.lpBgn = tMn;   _lrn.lpEnd = tMx;     // FOCUS !!
            usex = false;                             // already did
         }                                            // else restore
      }
   }
   if (usex) {                         // gotta set em?  or did focus do it alrd
      if (nl) {_lrn.lpBgn = lp [x].tm;   _lrn.lpEnd = lp [x].te;}
      else    {_lrn.lpBgn = 0;           _lrn.lpEnd = _tEnd;}
   }
   if (dir != '.')  TmHop (_lrn.lpBgn);
   if (x >= nlB)  Hey (CC("(in bugless loop)"));
   else {
      Hey (StrFmt (ts, "on loop #`d of `d with `d spots", x+1, nlB, lp [x].t));
      if (PRAC) {
         in = (lp [x].t * 100) / lp [x].mx;
         if      (in < 25)  _f.tmpo = FIX1;
         else if (in < 50)  _f.tmpo = FIX1 * 80 / 100;
         else               _f.tmpo = FIX1 * 60 / 100;
         p = TmpoAt (_timer->Get ());
DBG(" _f.tmpo=`d TmpoAct=`d",  _f.tmpo, p);
         DscSave ();   PutTp ((ubyt2)p);   ReTrk ();
      }
   }
TRC("SetLp end - bgn=`s end=`s", TmSt(ts,_lrn.lpBgn), TmSt(t2,_lrn.lpEnd));
}


#define MAXBAR  (800)

void Song::LoopInit ()
// find repeat ranges, init loop points, clear bug counts
{ ubyte t, nTr, tr [MAX_TRK];
  ubyt2 b, b2;
  ubyt4 tMin, tMax, tX, p, l, ne, nEv,
        nPat, patBgn [MAXBAR], patEnd [MAXBAR], bar [MAXBAR], barf [MAXBAR];
  TrkEv  *e, *ev;
  TrkRow *trk;
  TStr    ts;
// get trk set from ui
   nTr = 0;   ne = 0;
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TLrn (t))
      {tr [nTr++] = t;   ne += _f.trk [t].ne;}
   if (nTr == 0)        {Hey (CC("pick some tracks to practice"));   return;}
   if (_bEnd > MAXBAR)  {Hey (CC("too many bars (max=800)"));        return;}

// make the "empty" pattern;  init ev,nEv
   patBgn [0] = patEnd [0] = 0;   nPat = 1;
   nEv = 0;   ev = new TrkEv [ne];   MemSet (ev, 0, ne * sizeof (TrkEv));
                                     MemSet (barf, 0, sizeof (barf));
// process (by bar) notes of the tracks
   for (b = 0;  b < _bEnd;  b++) {
      tMin = Bar2Tm (b+1);   tMax = Bar2Tm (b+2);
      patBgn [nPat] = nEv;
      for (t = 0;  t < nTr;  t++) {
         trk = & _f.trk [tr [t]];
         for (p = 0, l = trk->ne, e = trk->e;      // skip evs till >= tMin
                 (p < l) && (e [p].time < tMin);  p++)  ;
      // only ntOns within bar time - no CCs, relative tm, velo=0 for comparing
         for (;  (p < l) && (e [p].time < tMax);  p++)
            if (ENOTE (& e [p]) && EDOWN (& e [p]))
               {ev [nEv  ].ctrl = e [p].ctrl;
                ev [nEv++].time = e [p].time - tMin;}
      }
      patEnd [nPat] = nEv;

      if (patBgn [nPat] == patEnd [nPat])  bar [b] = 0;    // empty bar
      else {
         for (p = 1; p < nPat; p++)
      // got same bar pat?  (same ne, matching evs)
            if ( ((patEnd [p]    - patBgn [p]   ) ==
                  (patEnd [nPat] - patBgn [nPat])) &&
                 (! MemCm ((char *)& ev [patBgn [p]],
                           (char *)& ev [patBgn [nPat]],
                           sizeof (TrkEv) * (patEnd [p] - patBgn [p]), 'x')) )
               break;
         if (p < nPat) {               // matched prev pat!  reset ev[],nEv
              bar [b] = 0;   nEv = patBgn [nPat];     // n find bar stated 1st
              for (b2 = 0;  b2 < b;  b2++)  if (bar [b2] == p)
                                               {barf [b] = b2+1;   break;}
         }
         else bar [b] = nPat++;       // make it a new pat
      }
   }
   delete [] ev;

// kill existing range [ or (repeat bar *
   for (p = 0;  p < _f.cue.Ln;) {
      if (! _f.cue [p].tend)  p++;
      else {
         StrCp (ts, _f.cue [p].s);
         if ( (*ts == '[') || (! MemCm (ts, CC(".repeat bar"), 11)) )
               _f.cue.Del (p);
         else  p++;
      }
   }

// put em in fresh - only skip if >= 3 rep'd bars
   for (b = 0;  b+2 < _bEnd;) {        // first, skip ranges  (repeat bar 9999
      if ( (! bar [b]) && (! bar [b+1]) && (! bar [b+2]) ) {
         b2 = b+2;
         while ((b2+1 < _bEnd) && (! bar [b2+1]))  b2++;
         tMin = Bar2Tm (b+1);  if (tMin) tMin--;
         tMax = Bar2Tm (b2+2) - 1;
         StrFmt (ts, ".repeat bar `d", barf [b]);
      // extra .rep... if from new bar not in sequence
         for (b++;  (b <= b2) && (b < _bEnd);  b++)
            if (barf [b] && (barf [b-1]+1 != barf [b])) {
               tX = Bar2Tm (b+1);
               TxtIns (tMin, ts, & _f.cue, 'c');
               tMin = tX;   StrFmt (ts, ".repeat bar `d", barf [b]);
            }
         TxtIns (tMin, ts, & _f.cue, 'c');
      }
      else  b++;
   }
   for (b = 0;  b < _bEnd;) {          // then, loop ranges [ .. ]
      if (b == 0)  while ((bar [b] == 0) && (b < _bEnd))  b++;
      tMin = Bar2Tm (b+1);      if (b)             tMin -= (M_WHOLE/4);
      for (b2 = b;  b2 < b+3;  b2++)
         if ((b2+3 < _bEnd) && (! bar [b2+1]) &&
             (! bar [b2+2]) && (! bar [b2+3]))  break;
      tMax = Bar2Tm (b2+2)-1;   if (b2 < _bEnd-1)  tMax += (M_WHOLE/4);
      tMin = NtDnNxt (tMin);   tMax = NtDnPrv (tMax);
      TxtIns (tMin, StrFmt (ts, "/`d[", tMax-tMin), & _f.cue, 'c');
      b = b2+1;
      if ((b < _bEnd+2) && (! bar [b]) && (! bar [b+1]) && (! bar [b+2]))
         while ((b < _bEnd) && (! bar [b]))  b++;
   }

   _f.bug.Ln = 0;
   SetLp ('i');   ReDo ();
}


//______________________________________________________________________________
void Song::TmpoPik (char o_r)
// kill active tempo changes and replace fully w [o]rig or [r]ecorded tempos
{ ubyte t, cc;
  ubyt4 p;
  TrkEv *e;
  MidiEv m;
// lookup tmpo track
TRC("TmpoPik origRecorded=`c", o_r);
   for (t = 0;  t < _f.trk.Ln;  t++)  if (TDrm (t))  break;
   if (t >= _f.trk.Ln)  return;        // no tempo track??  nothin ta do

// lookup tmpo cc
   for (cc = 0;  cc < _f.ctl.Ln;  cc++)
      if (! StrCm (_f.ctl [cc].s, CC("tmpo")))  {cc |= 0x80;   break;}
   if (! (cc & 0x80))  return;         // no tempo control??  outa herez
TRC("tempo track=`d cc=`d e=`d ne=`d", t, cc, _f.trk [t].e, _f.trk [t].ne);

// wipe existing
   for (e = _f.trk [t].e, p = 0;  p < _f.trk [t].ne;)
      {if (e [p].ctrl == cc)  EvDel (t, p);   else p++;}
TRC("new ne=`d", _f.trk [t].ne);
   m.ctrl = cc;
   if (o_r == 'o')
      for (p = 0;  p < _f.tpo.Ln;  p++) {
         m.time = _f.tpo [p].time;
         m.valu = (ubyte)(_f.tpo [p].val & 0x00FF);
         m.val2 = (ubyte)(_f.tpo [p].val >> 8);
         EvInsT (t, & m);
      }
   else
      for (p = 0;  p < _dn.Ln;  p++)  if (_dn [p].tmpo) {
         m.time = _dn [p].time;
         m.valu = (ubyte)(_dn [p].tmpo & 0x00FF);
         m.val2 = (ubyte)(_dn [p].tmpo >> 8);
         EvInsT (t, & m);
      }
TRC("TmpoPik done");
}
