// sRecord.cpp - all the junk for Song::EvRcrd()

#include "song.h"


void Song::Shush (bool tf)
// on bg chans (non lrn, non REC) set vol per tf
// to 0  else "whatever it was before"
{ MidiO *mo;
  bool   got;
  ubyte  t, dt;
  ubyt2  craw;
TRC("Shush `b", tf);
   for (t = 0;  t < _f.trk.Ln;  t++)  if (! TLrn (t)) {
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
            tTr = (ubyte)(tTr * _f.trk.Ln / 65);
         }
      }
      else if (! StrCm (cMod, CC("step"))) {
         if (tOn)  if (--tTr >= _f.trk.Ln)  tTr = _f.trk.Ln-1;
      }
      else
         tTr = (ubyte)(tTr * _f.trk.Ln / 128);
//DBG("trLn=`d ev->valu=`d _eOn=`s _eTrk=`d tOn=`s tTr=`d",
//_f.trk.Ln, ev->valu, _eOn?"t":"f",_eTrk, tOn?"t":"f",tTr);
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
            Info (CC(UCmd [c].cmd));   Cmd (UCmd [c].cmd);
         }
      return true;
   }
   return false;
}


//______________________________________________________________________________
bool Song::DnOK (char nxt, ubyte *trk, MidiEv *ev)
// ONLy ok if ALL hit.   nxt=\0 for current(default) or n[ext]
{ ubyt4 tm;
  ubyte c, n, tr, oct;
  bool  d, ok = true;
  DownRow *dn;
   if (nxt != 'n') {dn = & _dn [_pDn  ];   tm = _pDn ? _dn [_pDn-1].time : 0;}
   else            {if (_pDn+1 >= _dn.Ln)  return false;
                    dn = & _dn [_pDn+1];   tm =        _dn [_pDn  ].time;}
   for (c = 0;  c < dn->nNt;  c++) {
      d = TDrm (tr = dn->nt [c].t);   n = dn->nt [c].nt;
      if (ev && (ev->ctrl == n) && (MDR (ev) == d))  *trk = tr; // where ev is
      if (! nxt)  _lrn.hld [_f.trk [tr].ht - '1'] = n;     // mark so no red dot
//TStr d1,d2,d3;
//DBG("   c=`d/`d dr=`b nt=`s recTm=`s dnTm=`s",
//c, dn->nNt, d, MKey2Str(d1,n), TmSt(d2,_lrn.rec [d][n].tm), TmSt(d3,tm));
      if (_lrn.rec [d?1:0][n].tm <= tm)  ok = false;
   }
TRC("DnOK(`s)=> `b", (nxt=='n')?"next":"curr", ok);
   return ok;
}


void Song::SetMSec (ubyt4 p, MidiEv *ev)
{ ubyt2 tpP, tpR, tpS;
  ubyt4 ms, msR, tk, nm, dn, ne, tm, i;
  ubyte n, t;
  sbyte h;
  char  cl;
  TStr  ts;
   ms = ev->msec;
TRC("SetMSec  p=`d _pDn=`d ms=`d", p, _pDn, ms);
   if (_dn [p].msec)  return;          // already got 1st note

   _dn [p].msec = ms;   _dn [p].tmpo = 0;   if (p)  _dn [p-1].tmpo = 0;
   if ( (p == 0) || (_dn [p-1].msec == 0) || (ms <= _dn [p-1].msec) )
      return;                          // ^ somethin not right msR wise

// msec = tick*625/(tmpo*2)   so tmpo=tick*625/(msec*2)
   msR = ms - _dn [p-1].msec;          // actual ms in recording
   tk  = _dn [p].time - _dn [p-1].time;
   nm  = tk * 625;                     // num,den of rec tempo
   dn  = msR * 2;
   tpR = nm / dn + ( ((nm % dn) > (dn/2)) ? 1 : 0 );  // round it

// clip if beyond +-1/4 of prescribed tempo (learn track tempo)
   tpP = TmpoAt (_dn [p-1].time, 'a');
   if      (tpR < (ubyt4)(tpP-tpP/4))  {tpS = tpP-tpP/4;   cl = 's';}
   else if (tpR > (ubyt4)(tpP+tpP/4))  {tpS = tpP+tpP/4;   cl = 'f';}
   else                                {tpS = tpR;         cl = '\0';}

// store it.  just usin new tpS from now on
   _dn [p-1].tmpo = TmpoSto ((ubyt2)tpS);
   _dn [p-1].clip = cl;
TRC(" msR=`d-`d=`d  tpRecd=`d tpPrescribed=`d clip=`c tpClipped=`d tpStored=`d",
_dn[p-1].msec, ms, msR,  tpR, tpP, cl?cl:' ', tpS, _dn [p-1].tmpo);

// update bug arr
   tm = _dn [p].time;   ne = _f.bug.Ln;
   for (i = 0;  (i < ne) && (_f.bug [i].time < tm-6);  i++)  ;
TStr s1,s2;
TRC(" bug pos=`d/`d  bugTm=`s dnTm=`s",
i, ne, (i<ne)?TmSt(s1,_f.bug [i].time):"-", TmSt(s2, tm));
   if ( (i < ne) && (_f.bug [i].time > tm-6) &&
                    (_f.bug [i].time < tm+6) ) { // got existing bug to upd/del
      h = (sbyte)Str2Int (_f.bug [i].s);
      if (cl)  {if (h < 9)  h++;}   else h--;    // bump hits per clip
TRC("    bug upd `s=>`d", _f.bug [i].s, h);
      if (h > 0)  StrCp (_f.bug [i].s, Int2Str (h, ts));
      else        _f.bug.Del (i);
   }
   else if (cl) {
      TxtIns (tm, CC("1"), & _f.bug);      // got new bug to ins
TRC("    bug ins");
   }
}


//______________________________________________________________________________
void Song::Record (MidiEv *ev)
// filter lame ntUps due to EdCmd n looping leftovers
{ ubyt4  p, x;
  TrkEv *e;
  Arr<TrkEv,MAX_RCRD> *re;
   if (_lrn.POZ)  return;              // poz recording only happens when done

TRC("Record `s", MDR(ev)?"drum":"melo");
   re = MDR(ev) ? (& _recD) : (& _recM);
   if (re->Full ())  return;
   for (p = 0;  p < re->Ln;  p++)  if ((*re) [p].time > ev->time)  break;
   if (ENTUP (ev)) {
      x = p;
      while (x && ((*re) [x-1].time == ev->time)) {
         --x;
         if ((ENTDN (& ((*re) [x]))) && ((*re) [x].ctrl == ev->ctrl))
            {p = x;   break;}
      }
   }
   re->Ins (p);   e = & ((*re) [p]);
   e->time = ev->time;   e->ctrl = (ubyte) ev->ctrl;
   e->valu = ev->valu;   e->val2 = ev->val2;   e->x = 0;
}


//______________________________________________________________________________
void Song::EvRcrd (ubyte dev, MidiEv *ev)
// deal with a midiin device's event
{ ubyte  dr, nt, tr, oct;
  ubyt4  pd, dnTm;
  MidiEv te;
  TStr   cSt, cMod, s1,s2,s3,s4,s5,s6,s7,s8,s9,sa;
TRC("EvRcrd `s.`d `s `s\n"
" ms=`d _pNow=`s _rNow=`s _now=`s tmr=`s\n"
" _pDn=`d dn.time=`s dn+1.time=`s",
(dev<_mi.Ln)?_mi [dev].mi->Name ():"kbd", ev->chan+1, TmSt(s1,ev->time),
(ev->ctrl & 0xFF80)
? StrFmt  (s2, "c=`s v=`d v2=`d", MCtl2Str(s3,ev->ctrl), ev->valu, ev->val2)
: MNt2Str (s4, ev),
ev->msec, TmSt(s5,_pNow),TmSt(s6,_rNow),TmSt(s7,_now),TmSt(s8,_timer->Get ()),
_pDn,(_pDn<_dn.Ln)?TmSt(s9,_dn [_pDn  ].time):"x",
(1+   _pDn<_dn.Ln)?TmSt(sa,_dn [_pDn+1].time):"x"
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
   if (*cSt) {
      if ( (! StrCm (cSt, CC("."))) || CCEd (cSt, cMod, dev, ev) )
         TRC("EvRcrd: filt'd|edit ctl");    // no rec?
   // map cc str to pos in _f.ctl[] and fixup ev.ctrl
      else if (ev->ctrl = CCUpd (cSt)) {    // outa _f.ctl spots?  xit
         _rNow = ev->time;
         Record (ev);                  // ins ev into rec trk
      }
   }
   if (MCTRL (ev) || (! RCRD))  return;

// notes only now:  map drum .din => .drm
   dr = MDR(ev) ? 1 : 0;   nt = (ubyte) ev->ctrl;
   if (dr)  for (tr = 0;  tr < _f.trk.Ln;  tr++)      // might hafta map .din
      if (TLrn (tr) && TDrm (tr) && (_f.trk [tr].din == nt))
         {ev->ctrl = nt = _f.trk [tr].drm;   break;}

// set _lrn.rec, clear on NtUp (and done), set time on NtDn
   if (! MNTDN (ev))
      {_lrn.rec [dr][nt].tm = 0;   Record (ev);   DrawNow ();   return;}

   if (_lrn.nt1)  {                    // kill rec on 1st ntdn of loop
      _lrn.nt1 = false;   _recM.Ln = _recD.Ln = 0;    // recWipe
      for (ulong x = 0;  x < _dn.Ln;  x++) {          // clear _dn's rec stuff
         _dn [x].msec = 0;   _dn [x].tmpo = 0;   _dn [x].clip = '\0';
         MemSet (_dn [x].velo, 0, sizeof (_dn [0].velo));
      }
      Draw ('a');
   }
   _lrn.rec [dr][nt].tm = ev->time + (ev->time ? 0 : 1);
   _lrn.rec [dr][nt].vl = ev->valu & 0x7F;

// check if got all nts;  find lrn trk - look in _pDn, then _pDn+1
   dnTm = _dn [pd = _pDn].time;
   tr = 0x80;                          // default to no matched lrn trk from _dn

   if (ev->time <= dnTm) {
      if (DnOK ('c', & tr, ev)) {                     // ding ding ding
         SetMSec (pd, ev);
         if (_lrn.POZ) {
TRC("   UNPOZ cuz done w down");
            ev->time = dnTm;
            _lrn.POZ = false;   _timer->Set (dnTm);   Shush (false);
            Poz (false);
            for (dr = 0;  dr < 2;  dr++)
                           for (nt = 0;  nt < 128;  nt++)  if (nt != ev->ctrl) {
            // ntup for prev'ly not, ntdn for prev'ly not
               if      (   _lrn.rec [dr][nt].tm && (! _lrn.prec [dr][nt].tm)) {
                  te.chan = dr?9:0;   te.time = dnTm;   te.ctrl = nt;
                  te.valu = 0x80 | _lrn.rec [dr][nt].vl;   te.val2 = 0;
                  Record (& te);       // NtDn
               }
               else if ((! _lrn.rec [dr][nt].tm) &&   _lrn.prec [dr][nt].tm ) {
                  te.chan = dr?9:0;   te.time = dnTm;   te.ctrl = nt;
                  te.valu = te.val2 = 0;
                  Record (& te);       // NtUp
               }
            }
         }
      }
   }
   else if (! _lrn.POZ) {
      pd++;
      if (DnOK ('n', & tr, ev)) {   // ding ding ding
         SetMSec (pd, ev);                             // BOING !!
TRC("   hard BOING forward");
         ev->time = dnTm = _dn [pd].time;
         _timer->SetSig (dnTm);   _timer->Set (dnTm);
      }
   }
TRC("   lrnTrk=`d pDn=`d", tr, pd);

   if (tr != 0x80) {                   // store our velo in _dn[].velo[]
      oct = TDrm (tr) ? 0 : (_f.trk [tr].ht - '0');
      _dn [pd].velo [oct] = ev->valu & 0x7F;     // .ht '1' => 1 (0 for drum)
TRC("   oct=`d", oct);
   }

   _rNow = ev->time;
   Record (ev);                        // ins ev into rec trk
   DrawNow ();
TRC("EvRcrd end");
}


//______________________________________________________________________________
void Song::MIn ()
{ MidiEv e;
   for (ubyte d = 0;  d < _mi.Ln;  d++)
      while (_mi [d].mi->Get (& e))  EvRcrd (d, & e);
   Put ();
}
