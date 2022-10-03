// sRecord.cpp - all the junk for Song::EvRcrd()

#include "song.h"


void Song::Shush (bool tf)
// on bg chans (non lrn, non REC) set vol per tf
// to 0  else "whatever it was before"
{ MidiO *mo;
  bool   got;
  ubyte  t, dt;
  ubyt2  craw;
DBG("Shush `b", tf);
   for (t = 0;  t < Up.rTrk;  t++)  if (! TLrn (t)) {
      mo = Up.dev [_f.trk [t].dev].mo;
      got = false;                     // already did w my dev,chn?
      for (dt = 0;  dt < t;  dt++)  if ((_f.trk [t].dev == _f.trk [dt].dev) &&
                                        (_f.trk [t].chn == _f.trk [dt].chn))
         {got = true;  break;}
      if (! got) {
         dt = Up.dev [_f.trk [t].dev].dvt;
         if ((craw = Up.dvt [dt].CCID (CC("Vol"))))
            mo->Put (_f.trk [t].chn,
                     craw, tf ? 0 : CCValAt (_now, t, CC("Vol")), 0);
//DBG("   t=`d vol=`d", t, tf ? 0 : CCValAt (_now, t, CC("Vol")));
      }
   }
   if (! tf)  _lrn.POZ = false;
}


void Song::CCMap (char *cSt, char *cMod, ubyte dev, MidiEv *ev)
// if it's a ctrl, cook it (via ccin.txt, ccmap.txt)
{ ubyte c;
   *cSt = *cMod = '\0';                // default to notta ctrl (note)
   if (MNOTE (ev) || (dev >= _mi.Ln))  return;   //  notta ctrl so outa herez
// raw to cooked per ccin.txt  (else default cooking w MCtl2Str)
   for (c = 0;  c < _mi [dev].cc.Ln;  c++)
      if (ev->ctrl == _mi [dev].cc [c].raw) {
         StrCp (cSt,  _mi [dev].cc [c].map);
         if (! StrCm (cSt, CC(".")))
            {TRC(" ccIn.txt  sez . so FILTER");   return;}
         break;
      }
   if (c >= _mi [dev].cc.Ln)  MCtl2Str (cSt, ev->ctrl);
//TRC(" ccIn.txt=> `s", cSt);

// translate it if we've got a ccMap.txt
   for (c = 0; c < _mi [dev].mp.Ln; c++)
      if (! StrCm (cSt, _mi [dev].mp [c].cci)) {
         StrCp    (cSt, _mi [dev].mp [c].cco);
         if (! StrCm (cSt, CC(".")))
            {TRC(" ccMap.txt sez . so FILTER");   return;}

      // get cMod  (flip, step, etc)  can't do step till later...:/
         StrCp (cMod,   _mi [dev].mp [c].mod);
         if (StrCm (cMod, CC("flip")) == 0)  ev->valu = (ubyte)127 - ev->valu;
         break;
      }
TRC("MapCtl=> `s cMod=`s", cSt, cMod);
}


bool Song::CCEd (char *cSt, char *cMod, ubyte dev, MidiEv *ev)
// EDIT ctrl/key?  set tOn/tTr=0-nTrk-1;  to set _eOn/_eTrk n hit gui
{ bool  tOn;
  ubyte tTr, c;
   if (! StrCm (cSt, CC("edit"))) {
      tOn = (tTr = ev->valu) != 0;
      if      (! StrCm (cMod, CC("bend"))) {
         if (tTr > 64) {tOn = false;   tTr = 0;}
         else {
            tOn = (tTr = 64 - tTr) != 0;
            tTr = (ubyte)(tTr * Up.rTrk / 65);
         }
      }
      else if (! StrCm (cMod, CC("step"))) {
         if (tOn)  if (--tTr >= Up.rTrk)  tTr = Up.rTrk-1;
      }
      else
         tTr = (ubyte)(tTr * Up.rTrk / 128);
//DBG("rTrk=`d ev->valu=`d _eOn=`s _eTrk=`d tOn=`s tTr=`d",
//Up.rTrk, ev->valu, _eOn?"t":"f",_eTrk, tOn?"t":"f",tTr);
      if (tOn != _eOn)  _eOn = tOn;//{_c->dHlp->Show (_eOn = tOn);}
      if (tTr != Up.eTrk)  {Up.eTrk = tTr;   ReTrk ();}
      return true;
   }
   else if (! MemCm (cSt, CC("do_"), 3)) {
      for (c = 0;  c < _cDo.Ln;  c++)
         if ((_cDo [c].dev == dev) &&
             (_cDo [c].chn == ev->chan) && (_cDo [c].ctl == ev->ctrl))  break;
      if (c >= _cDo.Ln) {
         if (_cDo.Full ())  return true;
         _cDo.Ins ();
         _cDo [c].dev = dev;                _cDo [c].chn  = ev->chan;
         _cDo [c].ctl = (ubyte) ev->ctrl;   _cDo [c].valu = 0;
      }
      if ((_cDo [c].valu == 0) && ev->valu)  Cmd (cSt+3);
      _cDo [c].valu = ev->valu;
      return true;
   }
   if ((dev == 0) && (ev->ctrl == Cfg.cmdKey))
      {_eOn = (ev->valu & 0x80 ? true : false);   return true;}
   // {_c->dHlp->Show (_eOn = (ev->valu & 0x80 ? true : false));   return true;}

// cmd "shift" on and key down matching cmd on dev 0 ?
   if ((dev == 0) && _eOn) {
      if (ev->valu & 0x80)             // cmds only on key down
         for (c = 0; c < NUCmd; c++)  if (ev->ctrl == MKey (CC(UCmd [c].nt))) {
TRC("Edit edit key!");
            Hey (CC(UCmd [c].cmd));   Cmd (UCmd [c].cmd);
         }
      return true;
   }
   return false;
}


void Song::CCInit (ubyte t, char *cc, ubyte val)
// for special bar#1 events, upd else ins event at time=0 w valu
{ ubyte cid;
  ubyt4 p;
  bool  got;
  TrkRow *tr;
TRC("CCInit tr=`d cc=`s val=`d", t, cc, val);
   if (TDrm (t))  return;
   if (! (cid = CCUpd (cc, t))) {      // map cc str to pos in _f.ctl[]
TRC("CCInit: new cc outa IDs");
      return;
   }
   if (_f.nEv >= _f.maxEv) {
TRC("CCInit: hit maxEv");
      return;                          // no room - scram
   }
   while (t && _f.trk [t].grp)  t--;   // get 1st trk of grp
   tr = & _f.trk [t];                  // look for a time=0 ctrl=this to upd
   for (got = false, p = 0; (p < tr->ne) && (tr->e [p].time < M_WHOLE); p++)
      {if (tr->e [p].ctrl == cid)  {got = true;  break;}}
   if      (got)
      tr->e [p].valu = val;
   else if (EvIns (t, 0)) {
      tr->e [0].time = 0;
      tr->e [0].ctrl = cid;
      tr->e [0].valu = val;
      tr->e [0].val2 = tr->e [0].x = 0;
      p = 0;
   }
   PutCC (t, & tr->e [p]);             // put it out "live"
TRC("CCInit: got=`b p=`d tr=`d tm=`d", got, p, t, tr->e [p].time);
}


//------------------------------------------------------------------------------
char Song::DnOK (char nxt)             // \0=current(default) or n[ext]
{ ubyte c, d, n;
  ubyt4 pt, ms, mn, mx;
  char  ok, in;
  DownRow *dn;
//DBG("{ DnOK `c", nxt);
   dn = & _dn [_pDn];   pt = _pDn ? _dn [_pDn-1].time : 0;
   if (nxt)  {if (_pDn+1 >= _dn.Ln)  return 'n';
              dn = & _dn [_pDn+1];   pt = _dn [_pDn].time;}
// ONLy ok if ALL hit n ms range<=100
   for (in = 'n', ok = 'y', c = 0;  c < dn->nNt;  c++) {
      d = TDrm (dn->nt [c].t) ? 1 : 0;   n = dn->nt [c].nt;
//TStr d1,d2,d3;  DBG("c=`d/`d dr=`d nt=`s tm=`s pt=`s",
//c, dn->nNt, d, MKey2Str(d1,n), TmSt(d2,_lrn.rec [d][n].tm), TmSt(d3,pt));
      if ( _lrn.rec [d][n].tm <= pt)  {ok = 'n';   break;}
      ms = _lrn.rec [d][n].ms;
      if (in != 'y')  {in = 'y';   mn = mx = ms;}
      else            {if (ms < mn)  mn = ms;
                       if (ms > mx)  mx = ms;}
   }
// even if all ok, if some nts not hit together, back to not ok-ish
   if ((ok == 'y') && (in == 'y') && ((mx-mn) > 100))  ok = 'a';
//DBG("} DnOK `c", ok);
   return ok;                          // y/n/a[gain]
}


void Song::RecDvCh (MidiEv *ev, ubyte *d, ubyte *c, ubyte *dL, ubyte *cL)
// given mEv n _lrn, get out dev,chn (and LH dev,chn) from lrn trks
{ ubyte t;
   *d = *dL = 0;   *c = *cL = 16;
   if (ev->chan == 9) {                // drums are just a dev to lookup
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t) && TDrm (t))
               {*d  = _f.trk [t].dev;   *c  = 9;   break;}
DBG("RecDvCh dv=`d ch=`d dL=`d cL=`d", *d, *c+1, *dL, *cL+1);
      return;
   }

// no echoing of ctrls in ez/rHop
   if (_lrn.ez && MCTRL (ev))  return;

   for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t) && (! TDrm (t))) {
      if (_lrn.ez) {
         if (_f.trk [t].ht == (ev->ctrl / 12 - 1 + '0'))
               {*d  = _f.trk [t].dev;   *c  = _f.trk [t].chn;}
      }
      else {
         if (_f.trk [t].ht == 'L')
               {*dL = _f.trk [t].dev;   *cL = _f.trk [t].chn;}
         else  {*d  = _f.trk [t].dev;   *c  = _f.trk [t].chn;}
      }
   }
DBG("RecDvCh dv=`d ch=`d dL=`d cL=`d", *d, *c+1, *dL, *cL+1);
}


void Song::RecDvCh (ubyte ti,
                    TrkEv *ev, ubyte *d, ubyte *c, ubyte *dL, ubyte *cL)
// given trk,tEv n _lrn here
{ ubyte t;
   *d = *dL = 0;   *c = *cL = 16;
   if (ti == Up.rTrk) {                // drums are just a dev to lookup
      for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t) && TDrm (t))
               {*d  = _f.trk [t].dev;   *c  = 9;   break;}
      return;
   }

// no echoing of ctrls in ez
   if (_lrn.ez && MCTRL (ev))  return;

   for (t = 0;  t < Up.rTrk;  t++)  if (TLrn (t) && (! TDrm (t))) {
      if (_lrn.ez) {
         if (_f.trk [t].ht == (ev->ctrl / 12 - 1 + '0'))
               {*d  = _f.trk [t].dev;   *c  = _f.trk [t].chn;}
      }
      else {
         if (_f.trk [t].ht == 'L')
               {*dL = _f.trk [t].dev;   *cL = _f.trk [t].chn;}
         else  {*d  = _f.trk [t].dev;   *c  = _f.trk [t].chn;}
      }
   }
}


//------------------------------------------------------------------------------
void Song::PozIns ()
// insert buffered toRec note/ctrl events into respective trks
{ ubyte c, d, t;
  ubyt4 p, ne;
  TrkRow *tr;
  TrkEv  *ep;
   for (t = Up.rTrk;  t < _f.trk.Ln;  t++) {
   // count ne per trk
      d = (t == Up.rTrk) ? 1 : 0;
      for (ne = 0, c = 0;  c < 128;  c++) {
         if (_lrn.toRec [d][       c] & 0x007F)  ne++;     // notes
         if (_lrn.toRec [d][0x80 | c] & 0x0080)  ne++;     // ctrls
      }
      tr = & _f.trk [t];
   // find ins spot in time
      for (p = 0;  p < tr->ne;  p++)  if (tr->e [p].time > _now)  break;
TStr d1;
TRC("PozIns EvIns tr=`d drm=`d ne=`d p=`d", t, d, ne, p);
      if (EvIns (t, p, ne)) {          // got room?
      // ins new spots;  bump following _f.trk[].e's
         ep = & tr->e [p];
         for (c = 0;  c < 128;  c++) {
            if ( ( _lrn.toRec [d][c] & 0x007F) &&
                 ((_lrn.toRec [d][c] & 0x0080) == 0) ) {   // ntUps 1st !!
               ep->time = _now;   ep->ctrl = c;   ep->val2 = ep->x = 0;
               ep->valu = (ubyte)(_lrn.toRec [d][c] & 0x00FF);
               _f.trk [t].nb--;
TRC("`s^`d t=`d nn=`d nb=`d", MKey2Str (d1, c), ep->valu & 0x007F,
t, _f.trk [t].nn, _f.trk [t].nb);
               ep++;
            }
            if (   _lrn.toRec [d][0x80|c] & 0x0080   ) {   // ctrls
               ep->time = _now;   ep->ctrl = 0x80|c;   ep->val2 = ep->x = 0;
               ep->valu = (ubyte)(_lrn.toRec [d][0x80|c] & 0x007F);
TRC("`s $`02x", _f.ctl [c].s, ep->valu);
               ep++;
            }
         }
         for (c = 0;  c < 128;  c++) {
            if ( ( _lrn.toRec [d][c] & 0x007F) &&
                 ( _lrn.toRec [d][c] & 0x0080)       ) {   // ntDns after Ups
               ep->time = _now;   ep->ctrl = c;   ep->x = 0;
               ep->valu = (ubyte)(_lrn.toRec [d][c] & 0x00FF);
               ep->val2 = (ubyte)(_lrn.toRec [d][c] >> 8);
               _f.trk [t].nn++;   _f.trk [t].nb++;
TRC("`s_`d t=`d nn=`d nb=`d", MKey2Str (d1, c), ep->valu & 0x007F,
t, _f.trk [t].nn, _f.trk [t].nb);
               ep++;
            }
         }
      }
      tr->p += ne;                     // already echoed em
   }
}


void Song::SetMSec (ubyt4 p, MidiEv *ev)
{ ubyt2 tp;
  ubyt4 ne, tm, nm, dn, ms, msA, tk, i;     // num, den, ev, actual msec, tick
  ubyte n, t, ent, edr, nt, dr;
  sbyte h;
  bool  cl = false;
  TStr  ts;
  NtDef *na;
  TrkEv *e, te;
   ms = ev->msec;
TRC("SetMSec  p=`d _pDn=`d ms=`d", p, _pDn, ms);
   if (_dn [p].msec)  return;          // already got 1st note

TRC(" settin msec");
   _dn [p].msec = ms;   _dn [p].tmpo = 0;   if (p)  _dn [p-1].tmpo = 0;
   if ( (p == 0) || (_dn [p-1].msec == 0) || (ms <= _dn [p-1].msec) )
      return;                          // ^ somethin not right msA wise

TRC(" settin tmpo");
   msA = ms - _dn [p-1].msec;          // actual ms in recording

// prescribed msec = tick*625/tmpo*2
   tk = _dn [p].time - _dn [p-1].time;
   nm = tk * 625;   dn = msA * 2;      // played (actual) tempo
   nm = nm / dn + ( ((nm % dn) > (dn/2)) ? 1 : 0 );   // round it
   tp = TmpoAt (_dn [p-1].time, 'a');

// clip if beyond +-1/4 of lrn tempo
   if      (nm < (ubyt4)(tp-tp/4))  {nm = tp-tp/4;   cl = true;}
   else if (nm > (ubyt4)(tp+tp/4))  {nm = tp+tp/4;   cl = true;}

// store it
   _dn [p-1].tmpo = TmpoSto ((ubyt2)nm);   _dn [p-1].clip = cl;
   tp = (ubyt2)nm;                     // just usin new rec'd tp now

// update bug arr
   tm = _dn [p-1].time;   ne = _f.bug.Ln;
   for (i = 0;  (i < ne) && (_f.bug [i].time < tm-6);  i++)  ;
//TStr s1,s2;
//DBG("bug i=`d/`d=`s tm=`s", i, ne, TmSt(s1,_f.bug [i].time), TmSt(s2, tm));
   if ( (i < ne) && (_f.bug [i].time > tm-6) &&
                    (_f.bug [i].time < tm+6) ) { // got existing bug to upd/del
      h = (sbyte)Str2Int (_f.bug [i].s);
      if (cl)  {if (h < 9)  h++;}   else h--;    // bump hits per clip
      if (h > 0)  StrCp (_f.bug [i].s, Int2Str (h, ts));
      else        _f.bug.Del (i);
   }
   else if (cl)  TxtIns (tm, CC("1"), & _f.bug);      // got new bug to ins

// adj .time given tempo n .msec, for other notes
   ent = (ubyte)ev->ctrl;   edr = (ev->chan == 9) ? 1 : 0;
   for (na = _dn [p].nt,  n = 0;  n < _dn [p].nNt;  n++)
               if ( (na [n].nt != ent) || ((TDrm (na [n].t) ? 1 : 0) != edr) ) {
      nt =           na [n].nt;   dr =      TDrm (na [n].t) ? 1 : 0;
      t = Up.rTrk+1 - dr;
      tm = _lrn.rec [dr][nt].tm;
TStr d1,d2,d3;TRC(" hop nt=`s t=`d tm=`s", MKey2Str(d1,nt), t, TmSt(d3, tm));
   // ok, get it, kill it, reIns it w new time
      for (e = _f.trk [t].e, ne = _f.trk [t].ne;  ne;) {
         ne--;
//DBG(" ne=`d nt=`s tm=`s valu=`d",
//ne, MKey2Str (d1, e[ne].ctrl), TmSt(d2, e [ne].time), e [ne].valu);
         if (e [ne].time <= _dn [p-1].time)  break;
         if ((e [ne].ctrl == nt) && ENTDN (& e [ne]) && (e [ne].time <= tm)) {
            MemCp (& te, & e [ne], sizeof (te));
            EvDel (t, ne);
            msA = ms - _lrn.rec [0][nt].ms;
         // tick = msec*tmpo*2/625
            nm = msA*tp*2;   dn = 625;
            nm = nm / dn + ( ((nm % dn) > (dn/2)) ? 1 : 0 );
            te.time = _dn [p].time - nm;
            if (te.time < _pNow)  _pNow = te.time;
TRC(" upd ntDn=`s tm=`s", MKey2Str (d1,te.ctrl), TmSt (d2,te.time));
            EvInsT (t, & te);
         // argh - gotta restore .nn,.nb cuzu prev EvDel :/
            _f.trk [t].nn--;   _f.trk [t].nb--;
            break;               // only need last one
         }
      }
      for (e = _f.trk [t].e, ne = _f.trk [t].ne;  ne;) {
         ne--;
//DBG(" ne=`d nt=`s tm=`s valu=`d",
//ne, MKey2Str (d1, e[ne].ctrl), TmSt(d2, e [ne].time), e [ne].valu);
         if (e [ne].time < tm)  break;
         if ((e [ne].ctrl == nt) && ENTUP (& e [ne]) && (e [ne].time == tm)) {
            MemCp (& te, & e [ne], sizeof (te));
            EvDel (t, ne);
            msA = ms - _lrn.rec [0][nt].ms;
         // tick = msec*tmpo*2/625
            nm = msA*tp*2;   dn = 625;
            nm = nm / dn + ( ((nm % dn) > (dn/2)) ? 1 : 0 );
            te.time = _dn [p].time - nm;
            if (te.time < _pNow)  _pNow = te.time;
TRC(" upd ntUp=`s tm=`s", MKey2Str (d1,te.ctrl), TmSt (d2,te.time));
            EvInsT (t, & te);
         // argh - gotta restore .nn,.nb cuzu prev EvDel :/
            _f.trk [t].nb++;
            break;               // only need last one
         }
      }
   }
}


void Song::PozBuf (MidiEv *ev, char *cSt)
// during pause, WE echo events cuz time has stopped so Put() ain't goin
// then evs are buffered for recording last state of note/ctrl on unpoz
// upon POZ, toRec init'd to 0 except notes w _lrn.rec[][].tm == 0 - set to 0080
{ ubyte t, dv, ch, dL, cL, dr, nt, c, t1, n;
  ubyt2 craw;
  bool  ntDn = false, lh;
   dr = (ev->chan == 9) ? 1 : 0;
   t  = Up.rTrk;   if (! dr)  t++;          // to rec trk (drum or melo)
   RecDvCh (ev, & dv, & ch, &dL, &cL);      // get rec out dev,chn from lrn trks

// echo dat ev on out
   craw = nt = (ubyte)ev->ctrl;
   if (*cSt) {                         // tmpo,tsig,prog won't come in midi
      if (_lrn.ez)  return;            // ez/rHop - no ctrls yet
//TStr s1,s2;
//DBG("PozCC `s.`d `s=`d,`d  rTrk=`d  r2=`b  tm=`s tmr=`s",
//Up.dev [dv].mo->Name (), ch+1, cSt, ev->valu, ev->val2, t, r2,
//TmSt(s1,ev->time),TmSt(s2,_timer->Get ()));
      c = (ubyte)(ev->ctrl) & 0x7F;    // might be filtered?
      if ((craw = Up.dvt [Up.dev [dL].dvt].CCMap [c])) {
         if (ch != 16)  Up.dev [dv].mo->Put (ch, craw, ev->valu, ev->val2);
         if (cL != 16)  Up.dev [dL].mo->Put (cL, craw, ev->valu, ev->val2);
      }                                // LH might be diff than RH,HT
   }
   else {                              // note
      if (ch != 9)  craw += Cfg.tran;
      lh = (ev->val2 & 0x40) ? true : false;
TStr s1,s2;
TRC("PozNt echo to `s.`d  rTrk=`d  tm=`s tmr=`s",
Up.dev [lh?dL:dv].mo->Name (), (lh?cL:ch)+1, t,
TmSt(s1,ev->time), TmSt(s2,_timer->Get ()));
      if (lh)  Up.dev [dL].mo->Put (cL, craw, ev->valu, ev->val2);
      else     Up.dev [dv].mo->Put (ch, craw, ev->valu, ev->val2);
   }

// track what notes/ctrls to rec later when we UNpause
// for ctrls, bit 7 means "record it" (last val we get)
// for notes, bit 7 sez Dn/Up (set=Dn)
//            bits0-6 being non0 means "record it"
   if (*cSt)  _lrn.toRec [dr][(ubyte)0x80 | c] = 0x0080 | ev->valu;
   else {
      _lrn.toRec [dr][nt] &= 0x0080;   // reset to just flag of dn/up
      if ((ev->valu & 0x80) == _lrn.toRec [dr][nt])   // not flippin dn/up??
         {if (! (t1 = ev->valu & 0x7F))  t1 = 0x01;   // replace velo,val2
          _lrn.toRec [dr][nt] |= ((ev->val2 << 8) | t1);}  // velo always non0
      if (EDOWN (ev))  {if (! EPRSS (ev))  ntDn = true;}
      else  // ntUp in _dn[].nt[], rec it n buf any following ntDn
         for (n = 0;  n < _dn [_pDn].nNt;  n++)
            if ( (nt ==        _dn [_pDn].nt [n].nt) &&
                 (dr == (TDrm (_dn [_pDn].nt [n].t) ? 1 : 0)) ) {
TStr s1;
TRC("PozBuf ntUp rec, reflag ntDn t=`d nt=`s", t, MKey2Str (s1,nt));
               EvInsT (t, ev);   _f.trk [t].p++;
               _lrn.toRec [dr][nt] = (ev->val2 << 8) | 0x0080;
            }
   }
   if (ntDn && (DnOK () == 'y')) {     // only ntDn can unpause us
TRC("PozBuf  UNPOZ !");
      PozIns ();   SetMSec (_pDn, ev);      // buf rec trks, adj .time per .msec
      _lrn.POZ = false;   _timer->Set (_now);   Shush (false);   Poz (false);
   }
}


//------------------------------------------------------------------------------
void Song::NtGet (MidiEv *ev)
// map if drum note n got .din
// NtUp - clears _lrn.rec n uses prev 0x40 LH for rec nt
// NtDn - sets   _lrn.rec
//   find lrn trk - look in _dn[_pDn], then _pDn+1, else _lm
//   set velo[] if ez;  set veloRec/veloSng if hard
//   ht in hard mode only - store in ev->val2 bit 0x40 for LH
//   and does some weird sync junk :/
{ ubyte dr, nt, tr, n, v, t;
  ubyt4 i;
  char  kind;
  TStr  s;
  DownRow *dn;
// only messin w ntDn/Up in prac/play mode
   if ( MCTRL (ev) || (! (PRAC || PLAY)) )  return;

// map raw drum .din => .drm
   dr = (ev->chan == 9) ? 1 : 0;   nt = (ubyte) ev->ctrl;
   if (dr)  for (t = 0;  t < Up.rTrk;  t++)      // might hafta map
      if (TLrn (t) && TDrm (t) && (_f.trk [t].din == nt))
         {ev->ctrl = nt = _f.trk [t].drm;   break;}

// if non NtDn, clear _lrn.rec n scram
   if (! MNTDN (ev)) {
      _lrn.rec [dr][nt].tm = _lrn.rec [dr][nt].ms = 0;
      ev->val2 = _lrn.recLH [nt];
DBG("NtGet NtUp val2=$`02x", ev->val2);
      return;
   }

// NtDn - set _lrn.rec
   _lrn.rec [dr][nt].tm = ev->time + (ev->time ? 0 : 1);
   _lrn.rec [dr][nt].ms = ev->msec;

// try to match ev to _dn[_pDn].nt[] n get lrn trk
// kind for debuggin - wrong/curr/next
   dn = & _dn [_pDn];
   tr = 0x80;                          // default to no matched lrn trk from _dn
   kind = 'w';                         // default to wrong

// check ez first;  else is fer hard
//TODO why the weird tLate condition?
   if (_lrn.ez && (! dr)) {
     ubyt4 t, p, ne;
     TrkEv *e, te;
TStr d1,d2,d3;   // debug this when testin ez mode
DBG("   wtf tLate=`s tSoon=`s ev->time=`s",
TmSt(d1,_tLate), TmSt(d2,_tSoon), TmSt(d3,ev->time));
      if ((ev->time >= _tLate) && (ev->time <= _tLate))  return;
      if (ev->time < _tLate) {
         t = ev->time;
         for (n = 0;  n < dn->nNt;  n++)
            if ( (! TDrm (dn->nt [n].t)) &&
                 (nt ==   dn->nt [n].nt) ) {     // ding ding ding
               tr = dn->nt [n].t;   kind = 'c';  // curr pDn
               break;
            }
      }
      else if ((ev->time > _tSoon) && (_pDn+1 < _dn.Ln)) {
         dn++;                         // has to BE a next
         t = dn->time;
         for (n = 0;  n < dn->nNt;  n++)
            if ( (! TDrm (dn->nt [n].t)) &&
                 (nt ==   dn->nt [n].nt) ) {     // ding ding ding
               tr = dn->nt [n].t;   kind = 'n';  // next pDn
               break;
            }
      }
TRC("NtGet ez lrnTrk=`d `s",
tr, (kind=='c')?"current":((kind=='n')?"next":"wrong"));

   // PutNt/CC till now on trk if < tLate;  till pDn+1 time if < tSoon
      if (tr < (ubyte)0x80) {
         _lrn.velo [_f.trk [tr].lrn - '1'] = ev->valu & 0x7F;
TRC("bump time a t=`s p=`d", TmSt(s,t), _f.trk [tr].p);
         for (e = _f.trk [tr].e,  ne = _f.trk [tr].ne,  p = _f.trk [tr].p;
              (p < ne) && (e [p].time <= t);  p++) {
            if (ECTRL (& e [p]))  PutCC (tr, & e [p]);
            else {
               MemCp (& te, & e [p], sizeof (TrkEv));
               if (ENTDN (e))  te.valu = ev->valu;
               PutNt (tr, & te);
            }
         }
TRC("bump time b p=`d", p);
         _f.trk [tr].p = p;
      }
      return;
   }

// just hard from here on
   if (ev->time <= dn->time) {         // only lookin to get tr if curr dn
      for (n = 0;  n < dn->nNt;  n++)
         if ( (dr == (TDrm (dn->nt [n].t) ? 1 : 0)) &&
              (nt ==        dn->nt [n].nt) ) {   // ding ding ding
            tr = dn->nt [n].t;   kind = 'c';     // curr pDn
            break;
         }
   }
// else we're seein bout _pDn+1 (to bump time, unpoz, hop forward IFF rHop)
   else if (_pDn+1 < _dn.Ln) {         // has to BE a next
      dn++;
      for (n = 0;  n < dn->nNt;  n++)
         if ( (dr == (TDrm (dn->nt [n].t) ? 1 : 0)) &&
              (nt ==        dn->nt [n].nt) ) {   // ding ding ding
            tr = dn->nt [n].t;   kind = 'n';     // next pDn
            if (_lrn.rHop && (DnOK ('n') == 'y')) {   // HOP ok?
               SetMSec (_pDn+1, ev);                  // BOING !!
               if (! _lrn.POZ) {                      // else PozBuf/Ins unpozs
TRC("NtGet hard BOING");
                  for (ubyte d = 0;  d < _mi.Ln;  d++)
                     _mi [d].mi->BufAdj ((sbyt4)dn->time-(sbyt4)ev->time);
                  ev->time = dn->time;
                  for (n = 0;  n < dn->nNt;  n++) {
                     dr = TDrm (dn->nt [n].t) ? 1 : 0;
                     nt =       dn->nt [n].nt;
                     _lrn.rec [dr][nt].tm = dn->time;
                  }
                  _timer->SetSig (dn->time);   _timer->Set (dn->time);
               }
            }
            break;
         }
   }
// matched to lrn trk ev?  stamp rec ev velo versus lrn trk velo
   if (tr < (ubyte)0x80) {
     TrkEv *e = _f.trk [tr].e;
     ubyt4  p = dn->nt [n].p;
      if ((v = e [p].valu & 0x7F))
         {_lrn.veloRec = ev->valu & 0x7F;   _lrn.veloSng = v;}
      ev->val2 = (e [p].val2 & 0x7F) | ((_f.trk [tr].ht == 'L') ? 0x40 : 0);
   }
   else if ((! dr) && _lm.Ln) {        // for wrong melo nts by LH max split
      for (i = 0;  i < _lm.Ln;  i++)  if (ev->time <= _lm [i].tm)  break;
      if (i >= _lm.Ln)  i--;
      ev->val2 = (nt <= _lm [i].nt) ? 0x40 : 0;
TRC("wrong nt LH=`b", (ev->val2 & 0x40) ? true : false);
   }
   _lrn.recLH [nt] = ev->val2 & 0x40;
TRC("NtGet hard lrnTrk=`d `s ht=`s",
tr, (kind=='c')?"current":((kind=='n')?"next":"wrong"),
(ev->val2 & 0x40)?"LH":"RH/HT");
}


//------------------------------------------------------------------------------
void Song::Record (MidiEv *ev)
// filter lame ntUps due to EdCmd n looping leftovers
{ ubyte t;
  ubyt4 ne, p;
  TStr  s1, s2;
  TrkEv *e;
   t = Up.rTrk;   if (ev->chan != 9)  t++;  // to rec trk (drum or melo)
TRC("Record t=`d", t);
   if (MNTUP (ev)) {
      for (e = _f.trk [t].e, ne = _f.trk [t].ne,
           p = 0;  p < ne;  p++)  if (e [p].time > ev->time)  break;
      if (GetDn (e, p, (ubyte) ev->ctrl) == p) {
TRC("Record - toss lame ntUp tr=`d `s now=`s p=`d",
t, MNt2Str (s1, ev), TmSt (s2, _now), p);
         return;
      }
   }
   EvInsT (t, ev);
}


//------------------------------------------------------------------------------
void Song::EvRcrd (ubyte dev, MidiEv *ev)
// deal with a midiin device's event
{ ubyte t;
  TStr  cSt, cMod, s1,s2,s3,s4,s5,s6,s7,s8,s9,sa;
DBG("EvRcrd `s.`d `s `s",
// ms=`d\n"
//"_pNow=`s _rNow=`s _now=`s tmr=`s\n"
//"_pDn=`d dn.time=`s dn+1.time=`s",
(dev<_mi.Ln)?_mi [dev].mi->Name ():"kbd", ev->chan+1, TmSt(s1,ev->time),
(ev->ctrl & 0xFF80)
? StrFmt  (s2, "c=`s v=`d v2=`d", MCtl2Str(s3,ev->ctrl), ev->valu, ev->val2)
: MNt2Str (s4, ev)
//,
//ev->msec, TmSt(s5,_pNow),TmSt(s6,_rNow),TmSt(s7,_now),TmSt(s8,_timer->Get ()),
//_pDn,(_pDn<_dn.Ln)?TmSt(s9,_dn [_pDn].time):"x",
//(1+   _pDn<_dn.Ln)?TmSt(sa,_dn [_pDn+1].time):"x"
);
   if (! _f.trk.Ln) {                  // no song? - do DlgFL input
      if (MNTDN (ev)) {
         if      (ev->ctrl == MKey (CC("3b")))  emit sgUpd ("FLex");
         else if (ev->ctrl == MKey (CC("4c")))  emit sgUpd ("FLgo");
         else if (ev->ctrl == MKey (CC("4d")))  emit sgUpd ("FLdn");
         else if (ev->ctrl == MKey (CC("4e")))  emit sgUpd ("FLup");
      }
      return;
   }

// get str,mod if ctrl  (cSt="\0" fer nt)
   CCMap (cSt, cMod, dev, ev);
   if ( (! StrCm (cSt, CC("."))) || CCEd (cSt, cMod, dev, ev) )
      {TRC("EvRcrd: filt'd|edit ctl");   return;}     // no rec?

// hack ez rec notes' octave(1-7) into white note(C-B) of octave 4
   if (_lrn.ez && StrCm (cSt, CC(".")) && (ev->chan != 9)) {
      t = ev->ctrl / 12;   if (t >= 2)  t -= 2;  else t = 0;
      StrCp (s1, CC("4x"));   s1 [1] = "cdefgab" [t];   ev->ctrl = MKey (s1);
   }
   NtGet (ev);                         // set _lrn.rec, check ev in Dn[]
   t = Up.rTrk;   if (ev->chan != 9)  t++;  // to rec trk (drum or melo)

// gotta recWipe on 1st ntDn?  (in prac loop)
   if (_lrn.dWip) {
      if (! StrCm (cSt, CC("hold")))  _lrn.hVal = ev->valu;     // buf hold valu
      else if (*cSt == '\0') {         // GO!
         _lrn.dWip = false;
         RecWipeQ ();                       // ok we're clean
        MidiEv et;                          // gotta ins buf'd hold val
         MemCp (& et, ev, sizeof (et));     // ...messy
         et.ctrl = CCUpd (CC("hold"), t);   et.valu = _lrn.hVal;   et.val2 = 0;
         if (_lrn.POZ)  PozBuf (& et, CC("hold"));
         else           Record (& et);
         Draw ();                      // and redraw
      }
   }
// map cc str to pos in _f.ctl[] and fixup ev.ctrl
   if (*cSt)  if (! (ev->ctrl = CCUpd (cSt, t)))      // outa _f.ctl spots?  xit
                                       {TRC("EvRcrd: a");   return;}
   if (_lrn.rHop && _lrn.POZ) {
      PozBuf (ev, cSt);                // in rHop, on poz, buff/echo evs
      DrawNow ();
TRC("EvRcrd: b");
      return;
   }
   if (! _lrn.POZ)  _rNow = ev->time;
   if (PosInZZ (cSt, CC("PBnR\0Vol\0Pan\0"))) {
      CCInit (t, cSt, ev->valu);       // if special bar#1 ctl, upd/ins@tm=0
      DrawNow ();
TRC("EvRcrd: c");
      return;
   }
   Record (ev);                        // ins ev into rec trk
   DrawNow ();

// stamp song as practiced?
   if (_rcrd && (! _prac) && (_f.trk [t].ne >= 60))  {_prac = true;   Pract ();}
TRC("EvRcrd end");
}
