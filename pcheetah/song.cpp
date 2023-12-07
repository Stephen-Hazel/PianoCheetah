// song.cpp - thread dealin w song data, midi, timer - da guts

#include "song.h"

UpdLst Up;                             // what gui needs from meee

void Song::Init ()                     // init that there stuff we needz...
{
DBG("Song::Init bgn");
   _timer = new Timer ();              // boot timer
   connect (_timer, & Timer::TimerEv,   this, & Song::Put);
   connect (_timer, & Timer::TimerMsEv, this,
            [this]()  {if (_lrn.POZ)  Shush (true);});
   Sy.Init ();
   if (Sy.Dead () && StrCm (Sy._snDsc, CC("OFF")))
      Gui.Hey ("Another app owns sound device.\n"
              "So no Syn till you close that app and restart PianoCheetah :(");
   Midi.Load ();
   OpenMIn ();                         // boot MidiI's
   for (ubyte d = 0;  d < _mi.Ln;  d++)
      QObject::connect (_mi [d].mi, & MidiI::MidiIEv, this, & Song::MIn);
   Wipe ();
DBG("Song::Init end");
}


void Song::Quit ()                     // clean up
{
DBG("Song::Quit bgn");
   Wipe ();   ShutMIn ();   Sy.Quit ();   delete _timer;
   if (_f.ev)  delete [] _f.ev;
   emit sgUpd ("bye");
DBG("Song::Quit end");
}


void Song::Wipe ()                     // wipe all data and "empty" display
{
TRC("Wipe");
   _f.got = false;   _pg = _tr = 0;
TRC(" a");
   Save (true);
   *_f.fn = '\0';
TRC(" b");
   for (ubyte d = 0;  d < Up.dev.Ln;  d++)  ShutDev (d);
   Up.dev.Ln = 0;                      // should already be 0 after Shuts
   _rcrd = _prac = false;   _onBt = false;
   _pNow = _rNow = _now = 0;
TRC(" c");
   _f.ctl.Ln = _f.trk.Ln = 0;   Up.rTrk = Up.eTrk = 0;
   _f.lyr.Ln = _f.cue.Ln = _f.chd.Ln = _f.bug.Ln = 0;
   _pLyr = _pChd = 0;   _hLyr = 2;
TRC(" d");
   if (_f.ev)  delete [] _f.ev;
   _f.nEv = 0;   _f.ev = new TrkEv [_f.maxEv = MAX_RCRD];
   if (_nt)  delete [] _nt;   _nt = nullptr;
TRC(" e");
   if (_f.ev == nullptr)  Die (CC("couldn't alloc recording events"));
   _cDo.Ln = _cch.Ln = _f.tSg.Ln = _f.kSg.Ln = _f.tpo.Ln = 0;
   *_f.dsc = '\0';
   MemSet (& _lrn, 0, sizeof (_lrn));
   DscInit ();
   _pag.Ln = _col.Ln = _blk.Ln = _sym.Ln = 0;
TRC(" f");
   _timer->SetSig (0);   _timer->Set (0);   Poz (false);   PutTp (120);
   PutTs (4, 4, 0);   _bEnd = 0;   _tEnd = 0;   StrCp (Up.bars, CC("0"));
   Up.song [0] = '\0';
TRC(" g");
   emit sgUpd ("bars");   *Up.hey = '\0';   PutLy ();   ReTrk ();   Draw ();
TRC("Wipe end");
}


void Song::Hey (char *msg)
{
TRC("hey=`s", msg);
   StrCp (Up.hey, msg);   PutLy ();
}

void Song::Die (char *msg)
{ TStr s;
DBG("die=`s", msg);
   emit sgUpd (StrFmt (s, "die `s", msg));
}


//______________________________________________________________________________
void Song::PutTp (ubyt2 t)
// turn stored tempo to actual n set time n gui
{
//TRC("PutTp t=`d _f.tmpo=`d/`d tmpoAct=`d", t, _f.tmpo, FIX1, TmpoAct (t));
   _timer->SetTempo (t = TmpoAct (t));
   StrFmt (Up.tmpo, "`d", t);   emit sgUpd ("tmpo");
}


void Song::PutTs (ubyte n, ubyte d, ubyte sb)
{ TStr s;
   StrFmt (s, "`d/`d", n, d);
   if (sb)   StrFmt (& s [StrLn (s)], "/`d", sb+1);
   StrCp (Up.tsig, s);   emit sgUpd ("tsig");
}


void Song::PutLy ()
// sorry this code is terrible :(  but kinda necessary :/
// b is hilite start pos.  e is hilite OFF pos (not len)
{ ubyt4 ne, pos, p, b, e, pp, ps, ln, lc;
  char  buf [1000], *pc, nl = 0;
  bool  got = false;
   *Up.lyr = '\0';   Up.lyrHiB = Up.lyrHiE = 0;
   if (*Up.hey)  {StrCp (Up.lyr, Up.hey);   *Up.hey = '\0';
                  emit sgUpd ("lyr");   return;}
// ain't got none so bail
   if (! (ne = _f.lyr.Ln))  return;

// _pLyr is NEXT pos to check so we're actually AT the previous spot
   if ((pos = _pLyr))  {pos--;   got = true;}
//DBG("PutLy _hLyr=`d _pLyr=`d pos=`d ne=`d got=`b _now=`d",
//_hLyr, _pLyr, pos, ne, got, _now);
//for (ubyt4 i = 0;  i < _f.lyr.Ln;  i++) DBG(" `d: `d '`s'",
//i, _f.lyr [i].time, _f.lyr [i].s);
   if (_hLyr > 1) {                    // stanzaz ta do...
      if      (! got)                  // nothin
         {b = e = 0;   *buf = '\0';   p = 0;   got = true;}
      else if (_f.lyr [pos].s [0] == '/')   // pos starts w /
         {b = 0;   StrCp (buf, & _f.lyr [pos].s [1]);   e = StrLn (buf);
          p = pos+1;   got = true;}
      else {                           // from prv pos back to 0 look for /
         for (got = false, p = pos;  p;) {
            p--;                                                   // got /
            if (StrCh (_f.lyr [p].s, '/'))  {got = true;   break;}
         }
         if (p < ne)
             {pc = got ? (StrCh (_f.lyr [p].s, '/')+1) : _f.lyr [p].s;
              StrCp (buf, pc);}
         else *buf = '\0';
         b = e = 0;   p++;   got = false;
      }
      for (;  p < ne;  p++) {
//DBG(" a b=`d e=`d buf='`s' got=`b p=`d pos=`d", b, e, buf, got, p, pos);
         if ((! got) && (p == pos)) {
            b = StrLn (buf);   StrAp (buf, _f.lyr [p].s);
            e = StrLn (buf);   got = true;
         }
         else                  StrAp (buf, _f.lyr [p].s);
//DBG(" b b=`d e=`d buf='`s'", b, e, buf);
         while ((pc = StrCh (buf, '/'))) {
            if (++nl == 2)  {*pc = '\0';   p = ne;   break;}    // DONE !!
            *pc = '\n';
         }
      }
//DBG(" c b=`d e=`d buf='`s'", b, e, buf);
      StrCp (Up.lyr, buf);   Up.lyrHiB = b;   Up.lyrHiE = e;
      emit sgUpd ("lyr");
      return;
   }

   StrCp (buf, CC("                    ")); // 20 spaces
   for (ps = pos, pp = 20;  ps && pp;) {    // copy 20 prev chars
      ps--;
      lc = ln = StrLn (_f.lyr [ps].s);
      if (lc > pp)  lc = pp;
      MemCp (& buf [pp-lc], & _f.lyr [ps].s [ln-lc], lc);
      pp -= lc;
   }                                        // copy next 80 chars
   for (ps = pos, pp = 20;  (ps < ne) && (pp < 100);  ps++) {
      lc = StrLn (_f.lyr [ps].s);
      if (lc > (100-pp))  lc = (100-pp);
      MemCp (& buf [pp], _f.lyr [ps].s, lc);
      buf [pp += lc] = '\0';
   }
   StrCp (Up.lyr, buf);
   Up.lyrHiB = 20;   Up.lyrHiE = 20 + StrLn (_f.lyr [pos].s);
   emit sgUpd ("lyr");
}


void Song::PutCC (ubyte t, TrkEv *e)
{ ubyte c = e->ctrl & 0x7F, dv, ch, dL, cL = 128;
  ubyt2 craw;
   dv = _f.trk[t].dev;   ch = _f.trk[t].chn;
   if (t >= Up.rTrk)  RecDvCh (t, e, & dv, & ch, & dL, & cL);
if (App.trc) {TStr d1,d2;   StrFmt (d1, "PutCC `s.`d tmr=`s",
              Up.dev [dv].mo->Name (), ch, TmSt (d2, _timer->Get ()));
              DumpEv (e, t, _f.trk [t].p, d1);}
   if      (! StrCm (_f.ctl [c].s, CC("Tmpo")))
      PutTp (e->valu + (e->val2 << 8));
   else if (! StrCm (_f.ctl [c].s, CC("TSig")))
      PutTs (e->valu, 1 << (e->val2 & 0x0F), e->val2 >> 4);
   else if (! StrCm (_f.ctl [c].s, CC("KSig")))  ;    // just ignore fer now
   else if (! StrCm (_f.ctl [c].s, CC("Prog")))  SetChn (t);
   else
      if ((craw = Up.dvt [Up.dev [dv].dvt].CCMap [c])) {
         if (ch != 128)  Up.dev [dv].mo->Put (ch, craw, e->valu, e->val2);
         if (cL != 128)  Up.dev [dL].mo->Put (cL, craw, e->valu, e->val2);
      }
}


void Song::PutNt (ubyte t, TrkEv *e, bool bg)
{ ubyte ctrl = e->ctrl, valu = e->valu, i, dv, ch, dL, cL;
  ubyt4 v, d;
   if (! TDrm (t))  ctrl += _f.tran;   // got some live transposin?
// adjust velo if bg trk, learn mode n ntDn
   if (bg && (PRAC || PLAY) && ENTDN (e)) {
      if (_lrn.ez) {
         for (v = d = i = 0;  i < 7;  i++)  if (_dn [_pDn].velo [i])
            {v += _dn [_pDn].velo [i];   d++;}
         if (d)  valu = e->valu = 0x80 |
                                       (v / d + (((v % d) >= (d / 2)) ? 1 : 0));
TRC("   ezbg valu=128+`d", valu & 0x7F);
      }
      else {
         if (_lrn.veloRec && _lrn.veloSng) {
            if      (_lrn.veloRec > _lrn.veloSng) {
               v =  (_lrn.veloRec - _lrn.veloSng) * (128 - (valu & 0x7F));
               d = 128 - _lrn.veloSng;
               if ((v % d) < (d/2)) v /= d;   else {v /= d;  v++;}
               v = (valu & 0x7F) + v;
            }
            else if (_lrn.veloRec < _lrn.veloSng) {
               v =  (_lrn.veloSng - _lrn.veloRec) *        (valu & 0x7F);
               d =       _lrn.veloSng;
               if ((v % d) < (d/2)) v /= d;   else {v /= d;  v++;}
               v = (valu & 0x7F) - v;
            }
            else  v = valu & 0x7F;     // do nothin if exactly =
TRC("   bg valu=`d vRec=`d vSng=`d v=`d",
valu&0x7F,_lrn.veloRec,_lrn.veloSng,v);
            if (v < 1) v = 1;   if (v > 127) v = 127;
            valu = 0x80 | (ubyte)v;
         }
      }
   }
   if ((! bg) && TLrn (t) && (! TDrm (t)) && _lrn.ez && ENTDN (e) && (! HLRN))
      valu = 0x80 | _dn [_pDn].velo [_f.trk [t].ht - '1'];
   if (t >= Up.rTrk)
        {RecDvCh (t, e, & dv, & ch, & dL, & cL);
         if ((cL != 128) && (e->val2 & 0x40))  {dv = dL;   ch = cL;}}
   else {dv = _f.trk [t].dev;   ch = _f.trk [t].chn;}

if (App.trc) {TStr d1,d2;
StrFmt (d1, "PutNt `s.`d velo=`d", Up.dev [dv].mo->Name (), ch+1, valu&0x7F);
DumpEv (e, t, _f.trk [t].p, d1);
DBG("   bg=`b tmr=`s", bg, TmSt (d1, _timer->Get ()));
}
   Up.dev [dv].mo->Put (ch, ctrl, valu, e->val2);
}


//______________________________________________________________________________
void Song::Put ()                      // PianoCheetah's heartbeat
// writes current slice of song (.p .. songtime) to midiouts;  updates screen
// sets when timer next wakes us up
{ ubyte t, c, dr;
  bool  lrn, drm, doPoz = false, draw = false;
  ubyt4 tL8r, tL8r2, p, ne, tm, tend;
  TStr  bar, d1, d2;
  TrkEv *e;
   if (_lrn.POZ || Up.uPoz)  {TRC("Put (naw cuz pozd)");      return;}
                                       // paused or empty?  vamoose...
   if (! _f.got) {
      TmStr (bar, _now, & tL8r);   _timer->SetSig (_now = tL8r);
      StrCp (Up.time, bar);   emit sgUpd ("time");
                              TRC("Put (naw cuz no song)");   return;
   }
TRC("Put");
   tend = _rcrd ? Bar2Tm (_bEnd+1) : _tEnd;
   while (_timer->Get () >= _now) {
TRC(" loopTop tmr=`s now=`s", TmSt(d1,_timer->Get ()), TmSt(d2,_now));
      _rNow = _now;
      if (_f.got && (_now >= tend)) {
TRC(" end o song");
         if (_rcrd)  Cmd ("timeBar1");              // restart
         else       {Cmd ("song>");   return;}      // kick off next song
      }

   // get bar.beat str for now n tL8r (default to wakeup on next subbeat)
      TmStr (bar, _now, & tL8r);
      if (_onBt) {                     // on beat|64th, update time ctrl
TRC(" beat");
         StrFmt (Up.time, "`s`s", _timer->Pause () ? "X" : "", bar);
         emit sgUpd ("time");          // draw bar.beat
        char *bp = 1 + StrCh (bar, '.');
         if (Cfg.barCl && (! StrCm (bp, CC("1")))) {
TRC(" bar");                           // on bar (beat 1) => bar# to clipbd?
            Gui.ClipPut (StrFmt (d1, "`04d ", Str2Int (bar)));
         }
      }

      _onBt = true;

   // wakeup on fractional beats
      tL8r2 = (_now / LRN_BT) * LRN_BT + LRN_BT;
      if (tL8r2 < tL8r)  {draw = true;   _onBt = false;   tL8r = tL8r2;}

   // hoppin from ] back to paired [ if prac or review of prac
      if ((PRAC || (_lrn.pLrn == LPRAC)) && _lrn.lpEnd &&
                                   (_now >= _lrn.lpEnd)) {
TStr s1,s2;
TRC(" eoLoop a `s lpBgn=`s lpEnd=`s", LrnS (), TmSt(s1,_lrn.lpBgn),
                                               TmSt(s2,_lrn.lpEnd));
         Cmd (CC("timeBar1"));   SetLp ('.');
TRC("Put end - eoLoop b `s", LrnS ());
         return;
      }

   // sync _pDn
      while ((_pDn+1 < _dn.Ln) && (_now >= _dn [_pDn+1].time))  ++_pDn;

   // chek da poz !
      if (PRAC || PLAY) {
         if (_now == _dn [_pDn].time)  doPoz = (DnOK () != 'y');
         if (_pag.Ln)  draw = true;    // ^ check if we gots ta poz

         if (doPoz && (! _lrn.POZ)) {
TStr t1,t2,t3;
TRC("   POZ=Y!  _pDn=`d dn.tm=`s _now=`s tmr=`s ms=`d",
_pDn, TmSt(t1,_dn[_pDn].time), TmSt(t2,_now), TmSt(t3,_timer->Get ()),
_timer->MS ());
         // for notes, set bit 7 flag for only notetype (dn/up) to rec w/in poz
            MemSet (_lrn.toRec, 0, sizeof (_lrn.toRec));
            for (dr = 0;  dr < 2;  dr++)  for (c = 0;  c < 128;  c++)
               if (! _lrn.rec [dr][c].tm)  _lrn.toRec [dr][c] = 0x0080;
            _lrn.POZ = true;
            _timer->Set (_now);
            Poz (true, 500);           // GUI shows paused, shush after 1/2 sec
            if (draw)  Draw ();
TRC("Put end - due to poz");
            return;
         }
      }

   // plow thru only rec n lrn trks from .p to _now and dump stuff to midiout
   // no shh,bg tracks till next loop
TRC(" trk lrn,rec loop:");
      for (t = 0;  t < _f.trk.Ln;  t++) {
         lrn = TLrn (t);   if ((! lrn) && (t < Up.rTrk))  continue;
                                       // lrn==true for lrn trk, false for rec
         drm = TDrm (t);
         for (e = _f.trk [t].e,  ne = _f.trk [t].ne,  p = _f.trk [t].p;
              (p < ne) && (e [p].time <= _now);  p++) {
            if (ECTRL (& e [p]))       // ctrl:  if rec or ez or hLrn
               {if ((! lrn) || _lrn.ez || HLRN)
                                             PutCC (t, & e [p]);}
            else if (! lrn)            // note-rec: off if ez n melo
               {if (! (_lrn.ez && (! drm)))  PutNt (t, & e [p]);}
            else {                     // note-lrn: put n mark for hear/prac
               if (HLRN || (_lrn.ez && (! drm)))
                                             PutNt (t, & e [p]);
               else if (! drm)  _lrn.nt [e [p].ctrl] = e [p].valu;
            }
         }
         _f.trk [t].p = p;
         if (p < ne)
            {if ((tm = e [p].time) < tL8r)  {tL8r = tm;   _onBt = false;}}
      }

   // plow thru bg tracks from .p to _now and dump stuff to midiout
TRC(" trk non-lrn,rec loop:");
      for (t = 0;  t < Up.rTrk;  t++) {
         if (TLrn (t))  continue;      // already did

         for (e = _f.trk [t].e,  ne = _f.trk [t].ne,  p = _f.trk [t].p;
              (p < ne) && (e [p].time <= _now);  p++) {
         // ctrls ALWAYS go out
            if      (ECTRL (& e [p]))  PutCC (t, & e [p]);
         // else gotta note - only goes if nonShh and nonHearLrn
            else if ((! _f.trk [t].shh) && (! HLRN))
                                       PutNt (t, & e [p], true);
         }
         _f.trk [t].p = p;
         if (p < ne)
            {if ((tm = e [p].time) < tL8r)  {tL8r = tm;   _onBt = false;}}
      }

   // now plow thru lyrics
      if ((ne = _f.lyr.Ln)) {
         for (p = _pLyr;  (p < ne) && (_f.lyr [p].time <= _now);  p++) ;
         if (p > _pLyr) {
            _pLyr = p;   PutLy ();
            if (p < ne)
               {if ((tm = _f.lyr [p].time) < tL8r)
                   {tL8r = tm;   _onBt = false;}}
         }
      }

      _now = tL8r;
   }
   _timer->SetSig (_now);              // new wakeup
   if (draw)  Draw ();
TRC("Put end - tL8r=_now=`s", TmSt(d1,_now));
}
