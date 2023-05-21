// sDump.cpp

#include "song.h"

char *Song::LrnS ()
{ static TStr s;
   StrCp (s, CC("x"));
   if (Up.lrn    == LHEAR)  StrCp (s, CC("hear"));
   if (Up.lrn    == LHLRN)  StrCp (s, CC("hLrn"));
   if (Up.lrn    == LPRAC)  StrCp (s, CC("prac"));
   if (Up.lrn    == LPLAY)  StrCp (s, CC("play"));
   if (_lrn.pLrn == LHEAR)  StrAp (s, CC(" p=hear"));
   if (_lrn.pLrn == LHLRN)  StrAp (s, CC(" p=hLrn"));
   if (_lrn.pLrn == LPRAC)  StrAp (s, CC(" p=prac"));
   if (_lrn.pLrn == LPLAY)  StrAp (s, CC(" p=play"));
   return s;
}


void DumpZ (char const *t, char *z)
{ char *s;
  ubyt2 c = 0;
   DBG("`s", t);
   for (s = z;  *s;  s = & s [StrLn (s)+1])  DBG("`d: `s", c++, s);
}


void Song::DumpEv (TrkEv *e, ubyte t, ubyt4 p, char *pre)
{ TStr  o, s, ts;
  ubyte v;
   StrFmt (o, "`s t=`d ", pre ? pre : "", t);
   if (p < 1000000)  StrAp (o, StrFmt (ts, "p=`d ", p));
   StrAp (o, TmSt (ts, e->time));   StrAp (o, CC(" "));
   if (ECTRL (e))
      StrFmt (&o[StrLn(o)], "`s(cc`d)=`02x,`02x",
              _f.ctl [e->ctrl & 0x7F].s, e->ctrl & 0x7F, e->valu, e->val2);
   else {
      StrFmt (&o[StrLn(o)], "`s`c`d",
         TDrm (t) ? MDrm2Str (s, e->ctrl) : MKey2Str (s, e->ctrl),
         EUP (e) ? '^' : (EDN (e) ? '_' : '~'),  e->valu & 0x007F);
      if ((v = (e->val2 & 0x1F)))
         {StrAp (o, CC("@"));   StrAp (o, MFing [v-1]);}
   }
   DBG(o);
}


void Song::DumpTrEv (ubyte t)
{  DBG("t=`d ne=`d nb=`d nn=`d",
       t, _f.trk [t].ne, _f.trk [t].nb, _f.trk [t].nn);
  TrkEv *ev = _f.trk [t].e;
   for (ubyt4 e = 0;  e < _f.trk [t].ne;  e++, ev++)  DumpEv (ev, t, e);
}


void Song::DumpRec ()
{  for (ubyte t = Up.rTrk;  t < _f.trk.Ln;  t++) {
     TrkEv *ev = _f.trk [t].e;
      for (ubyt4 e = 0;  e < _f.trk [t].ne;  e++, ev++)  DumpEv (ev, t, e);
   }
}


void Song::Dump (bool e2)
{ ubyte  t;
  ubyt4  s;
  TStr   t1, t2, t3;
  TrkEv *ev;
//DumpX ();   return;
DBG("DUMP");
   DBG(
      "rcrd=`b bEnd=`d tEnd=`s rTrk=`d eTrk=`d eOn=`b pLyr=`d\n"
      "onBt=`b now=`s pDn=`d/`s\n"
      "dn.Ln=`d nEv=`d maxEv=`d\n"
      "Cfg_cmdKey=`d ntCo=`d barCl=`b\n"
      "SnF_tmpo=`d tran=`d `s ez=`b hand=`c\n"
      "hLrn=`b vwNt=`b POZ=`b uPoz=`b",
      _rcrd, _bEnd, TmSt(t1,_tEnd), Up.rTrk, Up.eTrk, _eOn, _pLyr,
      _onBt, TmSt(t2,_now), _pDn, TmSt(t3,_dn[_pDn].time),
      _dn.Ln, _f.nEv, _f.maxEv,
      Cfg.cmdKey, Cfg.ntCo, Cfg.barCl,
      _f.tmpo, _f.tran, LrnS (), _lrn.ez, _lrn.hand?_lrn.hand:' ',
      _lrn.hLrn, _lrn.vwNt, _lrn.POZ, Up.uPoz
   );

// DBG("dsc: `s", _dsc);
   DBG("dev name     type     desc");
   for (t = 0; t < Up.dev.Ln; t++)
      DBG("`>3d `<8s `<8s `s",
         t,
         Up.dev [t].mo ? Up.dev [t].mo->Name () : "(empty)",
         Up.dev [t].mo ? Up.dev [t].mo->Type () : "",
         Up.dev [t].mo ? Up.dev [t].mo->Desc () : ""
      );
   DBG("mi name");
   for (t = 0; t < _mi.Ln; t++) DBG("`02d `s", t, _mi [t].mi->Name ());
   DBG("trk name             "
       "dev chn      snd        e       ne       nn       nb"
       "        p shh lrn ht drm");
   for (t = 0; t < _f.trk.Ln; t++)
      DBG("`>3d `s`<15s "
           "`>3d `>3d `>8d `08x `>8d `>8d `>8d `>8d `b   `b   `c  `02x",
         t,
         _f.trk [t].grp?"+":".",
         _f.trk [t].name,
         _f.trk [t].dev,
         _f.trk [t].chn,
         _f.trk [t].snd,
         _f.trk [t].e,
         _f.trk [t].ne,
         _f.trk [t].nn,
         _f.trk [t].nb,
         _f.trk [t].p,
         _f.trk [t].shh,
         _f.trk [t].lrn,
         _f.trk [t].ht ?_f.trk [t].ht :' ',
         _f.trk [t].drm
      );
   for (t = 0; t < Up.dev.Ln; t++)
      if (Up.dev [t].mo)  Up.dev [t].mo->DumpOns ();

   DBG("ctl name sho");
   for (t = 0; t < _f.ctl.Ln; t++)
      DBG("`>3d `s `b", t,_f.ctl [t].s, _f.ctl [t].sho);
   DBG("cch dev chn ctl      trk valu val2     time");
   for (t = 0; t < _cch.Ln; t++)
      DBG("`>3d `>3d `>3d `<8s `>3d `>4d `>4d `>8d",
         t,
         _cch [t].dev,
         _cch [t].chn,
         _f.ctl [_cch [t].ctl & 0x7F].s,
         _cch [t].trk,
         _cch [t].valu,
         _cch [t].val2,
         _cch [t].time
      );
   DBG("tSg     time  bar num den sub");
   for (s = 0; s < _f.tSg.Ln; s++)
      DBG("`>3d `>8d `>4d `>3d `>3d",
         s,
         _f.tSg [s].time,
         _f.tSg [s].bar,
         _f.tSg [s].num,
         _f.tSg [s].den,
         _f.tSg [s].sub
      );
   DBG("kSg     time key min flt");
   for (s = 0; s < _f.kSg.Ln; s++)
      DBG("`>3d `>8d `<3s `<3b `<3b",
         s,
         _f.kSg [s].time,
         MKeyStr [_f.kSg [s].key],
         _f.kSg [s].min,
         _f.kSg [s].flt
      );
   DBG("lrn: lpBgn=`s lpEnd=`s pg=`d veloSng=`d veloRec=`d",
      TmSt(t1,_lrn.lpBgn), TmSt(t2,_lrn.lpEnd),
      _pg, _lrn.veloSng, _lrn.veloRec
   );
/*
   DBG("mapD shh ht inp ctl vol pan snd");
   for (t = 0; t < _mapD.Ln; t++)
      DBG("`>3d `b `c `s `s `>3d `>3d `d",
         t, _mapD [t].shh, _mapD [t].ht ? _mapD [t].ht : ' ',
         MDrm2Str(t2,_mapD [t].inp), MDrm2Str(t1,_mapD [t].ctl),
         _mapD [t].vol, _mapD [t].pan, _mapD [t].snd);
   for (t = 0; t < _dvt.Ln; t++)  {DBG("dvt=`02d...", t);   _dvt [t].Dump ();}
   DBG("lyr     time  str");
   for (s = 0; s < _f.lyr.Ln; s++)
      DBG("`>3d `s `s", s, TmSt (t1, _f.lyr [s].time), _f.lyr [s].s);
*/
   DBG("chd     time  str");
   for (s = 0; s < _f.chd.Ln; s++)
      DBG("`>3d `s `s", s, TmSt (t1, _f.chd [s].time), _f.chd [s].s);
   DBG("cue time tend str");
   for (s = 0; s < _f.cue.Ln; s++)  if (_f.cue [s].s [0] != '[')
      DBG("`d `s `s `s", s,  TmSt(t1,_f.cue [s].time),
           _f.cue [s].tend ? TmSt(t2,_f.cue [s].tend) : "", _f.cue [s].s);
   DBG("bug time hits");
   for (s = 0; s < _f.bug.Ln; s++)
      DBG("`d `s `s", s, TmSt(t1,_f.bug [s].time), _f.bug [s].s);

/*
TStr d1;
   DBG("_dn p tm msec tmpo tmpoAct  /  nt trk p");
   for (p = 0;  p < _dn.Ln;  p++) {
      DBG("`d `s `d `d `d",
          p, TmSt (d1,_dn [p].time), _dn [p].msec, _dn [p].tmpo,
          TmpoAct (_dn [p].tmpo));
     TStr s;
      for (ubyte c = 0;  c < _dn [p].nNt;  c++)
         DBG("   `s `d `d", MKey2Str(s,_dn [p].nt [c].nt), _dn [p].nt [c].t,
                                                           _dn [p].nt [c].p);
   }
   DBG("_dn end");
   DBG("_f.tmpo=`d/`d", _f.tmpo, FIX1);
   DBG("{ _f.tpo time tmpo tmpoAct");
   for (p = 0;  p < _f.tpo.Ln;  p++)
      DBG("`d `s `d `d",  p, TmSt (d1,_f.tpo [p].time), _f.tpo [p].val,
                                               TmpoAct (_f.tpo [p].val));
   DBG("_f.tpo end");


  ubyt4 ct [2][12];
  ubyte r;
   MemSet (ct, 0, sizeof (ct));
   for (t = 0; t < _f.trk.Ln; t++)  if (! TDrm (t)) {
      r = _f.trk [t].lrn ? 1 : 0;
      ev = _f.trk [t].e;
      for (ubyt4 e = 0;  e < _f.trk [t].ne;  e++, ev++)
         if (ENTDN (ev))  ct [r][ev->ctrl % 12]++;
   }
   for (t = 0; t < 12; t++) ct [2][t] = ct [0][t] + ct [1][t];
   DBG("lrn,non,tot:");
   DBG("`>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d",
      ct[0][0], ct[0][1], ct[0][2], ct[0][3], ct[0][4], ct[0][5],
      ct[0][6], ct[0][7], ct[0][8], ct[0][9], ct[0][10], ct[0][11]);
   DBG("`>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d",
      ct[1][0], ct[1][1], ct[1][2], ct[1][3], ct[1][4], ct[1][5],
      ct[1][6], ct[1][7], ct[1][8], ct[1][9], ct[1][10], ct[1][11]);
   DBG("`>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d `>4d",
      ct[2][0], ct[2][1], ct[2][2], ct[2][3], ct[2][4], ct[2][5],
      ct[2][6], ct[2][7], ct[2][8], ct[2][9], ct[2][10], ct[2][11]);
   DBG("} DUMP");

   if (e2) {
      for (t = 0; t < _f.trk.Ln; t++) {
         DBG("t=`d ne=`d nb=`d nn=`d",
             t, _f.trk [t].ne, _f.trk [t].nb, _f.trk [t].nn);
         ev = _f.trk [t].e;
         for (ubyt4 e = 0;  e < _f.trk [t].ne;  e++, ev++)  DumpEv (ev, t, e);
      }

   }
*/
// PosTM (_syn, MSG_CLOSE+3, 0, 0);
DBG("DUMP end");
}
