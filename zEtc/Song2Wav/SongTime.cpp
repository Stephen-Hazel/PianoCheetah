// SongTime.cpp - Time funcs of Song

#include "Song2Wav.h"


void Song::TmMap ()
{ uword s;
   if (_nTSg) {                        // get tsig bars
      _tSg [0].bar = (uword)(1 + _tSg [0].time / M_WHOLE);
      for (s = 1; s < _nTSg; s++)
         _tSg [s].bar = (uword)(_tSg [s-1].bar +
                        (_tSg [s].time - _tSg [s-1].time) /
                        (M_WHOLE / _tSg [s-1].den * _tSg [s-1].num));
   }
}


ulong Song::Bar2Tm (uword b)           // get song time of a bar
{ uword s = 0;   while ((s+1 < _nTSg) && (_tSg [s+1].bar <= b))  s++;
   if ((s >= _nTSg) || (_tSg [s].bar > b))
      return (b-1) * M_WHOLE;          // none or none apply yet - use 4/4

   b -= _tSg [s].bar;                  // got tsig so offset from it
   return _tSg [s].time + (b * M_WHOLE * _tSg [s].num / _tSg [s].den);
}


char *Song::TmStr (char *str, ulong tm, ulong *tL8r)
// put song time into a string w bar.beat;  maybe return time of next bt & subbt
{ ubyte sub;
  ulong dBr, dBt, l8r;
  uword    s = 0, br, bt;
   while ((s+1 < _nTSg) && (_tSg [s+1].time <= tm))  s++;
   if ((s >= _nTSg) || (_tSg [s].time > tm)) {
      dBt = M_WHOLE / 4;               // none apply yet - use 4/4/1
      dBr = dBt     * 4;
      sub = 1;
      br  = (uword)(1 + (tm / dBr));
      bt  = (uword)(1 + (tm % dBr) / dBt);
      l8r = (br-1) * dBr + bt * dBt;
   }
   else {
      dBt = M_WHOLE / _tSg [s].den;
      dBr = dBt     * _tSg [s].num;
      sub =           _tSg [s].sub;
      br  = (uword)(_tSg [s].bar + (tm - _tSg [s].time) / dBr);
      bt  = (uword)(1 +           ((tm - _tSg [s].time) % dBr) / dBt);
      l8r = _tSg [s].time + (br - _tSg [s].bar) * dBr + dBt * bt;
   }
   StrFmt (str, "`04d.`d", br, bt);
   if (tL8r) *tL8r = l8r;
   return str;
}


void Song::TmHop (ulong tm, bool put)
{ ulong  p, ne, cc, pn [128];
  uword  tmpo, cw;
  ubyte  c, t, v, v2;
  char  *cs;
  TStr   str;
  TrkEv *e;
DBG("Song::TmHop tm='d put=`b", tm, put);
   for (cc = 0; cc < _nCCh; cc++) {
      _cch [cc].time = 0;
      _cch [cc].valu = 0;
      _cch [cc].val2 = 0;
      _cch [cc].trk  = 0;
      cs = _ctl [_cch [cc].ctl & 0x7F];
//DBG("cc=`d cs=`s", cc, cs);
      if      (! StrCm (cs, "Prog")) { // prog,tmpo,tsig are special
         for (t = 0; t < _nTrk; t++)
            if (_trk [t].chn == _cch [cc].chn)   break;
         if (t < _nTrk) _cch [cc].trk  = t;
      }
      else if (! StrCm (cs, "Tmpo"))   _cch [cc].valu = 120;
      else if (! StrCm (cs, "TSig"))  {_cch [cc].valu = 4;  _cch [cc].val2 = 2;}
      else                             // else lookup in dev/cc.txt
         for (c = 0; c < NMCC; c++) {
//DBG("mcc c=`d/`d s=`s dflt=`d",
//c, NMCC, MCC [c].s, MCC [c].dflt);
            if (! StrCm (MCC [c].s, cs))
               {_cch [cc].valu = (ubyte)MCC [c].dflt;  break;}
         }
//DBG("valu=`d", _cch [cc].valu);
   }
//DBG("chase ctl actual valu in _trk[].e");
   for (t = 0; t < _nTrk; t++) {
      c = (ubyte)_trk [t].chn;
      MemSet (pn, 0xFF, sizeof (pn));
      for (p = 0, ne = _trk [t].ne, e = _trk [t].e;
           (p < ne) && (e->time < tm);  p++, e++) {
         if (e->ctrl & 0x80) {
            for (cc = 0; cc < _nCCh; cc++)
               if ((_cch [cc].chn == c) &&
                   (_cch [cc].ctl == e->ctrl))   break;
            if ((cc < _nCCh) && (_cch [cc].time <= e->time)) {
               _cch [cc].trk  = t;
               _cch [cc].time = e->time;
               _cch [cc].valu = e->valu;
               _cch [cc].val2 = e->val2;
//DBG("t=`d d=`d c=`d cc=`d p=`d time=`d valu=`d",t,d,c,cc,p,e->time,e->valu);
            }
         }
         else pn [e->ctrl] = p;        // track last ev for each nt
      }
      _trk [t].p = p;                  // chase _trk[].p
   }
//DBG("put each ctl");
   for (cc = 0; cc < _nCCh; cc++) {
      c  = _cch [cc].ctl & 0x7F;
      v  = _cch [cc].valu;
      v2 = _cch [cc].val2;
      if      (! StrCm (_ctl [c], "Prog"))  SetChn (_cch [cc].trk);
      else if (! StrCm (_ctl [c], "Tmpo"))
      {  _timer.SetTempo (tmpo = v + (v2 << 8));
         StrFmt (str, "`03d", tmpo);}
      else if (! StrCm (_ctl [c], "TSig"))
      {  StrFmt (str, "`d:`d", v, 1 << (v2 & 0x0F));
         if (v2 >> 4)  StrFmt (& str [StrLn (str)], ":`d", 1 + (v2 >> 4));
      }
      else {
         if (cw = _dvt.CCMap [c])
            _syn->Put (_cch [cc].chn, cw, v, v2);
      }
   }

   _timer.Set (_now = tm);   _onBt = true;
//DBG("} Song::TmHop  timer=`d _now=`d",
//   _timer.Get (), _now);
}
