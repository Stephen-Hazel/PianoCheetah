// Song2Mid.cpp - convert .song+.trak => .mid  (NOT pretty)

#include <os.h>
#include <MidiIO.h>
#include "SongLoad.cpp"

ubyte MHdr [] = {
   'M','T','h','d',
   0,0,0,6,        // len is always 6
   0,1,            // midi file format 1 (multiple parallel tracks)
   0,0,            // #tracks - set in SaveMid
   0,192           // resolution
};

ubyte MTrk [] = {
   'M','T','r','k',
   0,0,0,0         // len - set in SaveMid per track
};

ulong PTime;       // running absolute time and status
ubyte PStat;

ubyte Buf [1024*1024];
ulong Len;         // buffer for output .mid file

ubyte KyNum [2][2][12] = {{{9,7,2,9,4,9,6,1,9,3,9,5},      // M#
                           {0,5,9,3,9,1,6,9,4,9,2,7}},     // Mb
                          {{9,4,9,6,1,9,3,9,5,9,7,2},      // m#
                           {3,9,1,6,9,4,9,2,7,0,5,9}}};    // mb
//                          C Db D Eb E F Gb G Ab A Bb B - 9 means none

char SBuf [128*1024];

//------------------------------------------------------------------------------
// stuff scrounged from ditty's Ditty.h and SongFile.cpp, then stripped down
struct TrkDef {
   ubyte  dev,  chn;
   ulong ne, p, snd;
   TrkEv *e;
};

class Song {
public:
   Song ()
   {
TRC("{ Song::Song");
      _nTrk = 0;   _nCtl = 0;   _nLyr = _pLyr = 0;
      _nEv = 0;   _ev = NULL;   _nDev = 0;
      MCCInit ();
TRC("} Song::Song");
   }

  ~Song ()
   {
TRC("{ Song::~Song");
      if (_ev)  delete [] _ev;
TRC("} Song::~Song");
   }

   void Load    (char *fn);

   void BufTrk  (ubyte t);             // the workhorse
   void SaveMid (char *fn);

   STable  _t   [TB_MAX];              // holds the whole song file :)
   TStr    _dev [MAX_DEV];   ubyte _nDev;
   TrkDef  _trk [MAX_TRK];   ubyte _nTrk;
   ubyte   _nCtl;
   ulong   _nLyr, _pLyr;
   TrkEv  *_ev;
   ulong  _nEv;
};


//------------------------------------------------------------------------------
int TrkCmp (void *p1, void *p2)  // ...TrkEv sortin
{ TrkEv *e1 = (TrkEv *)p1, *e2 = (TrkEv *)p2;
  int t;
   return ( (t = e1->time - e2->time) ? t : (e1->ctrl - e2->ctrl) );
}


void Song::Load (char *fn)
// ok, go thru the BRUTAL HELL to load in a song, etc...
{ TStr   dTry;
  char   cmap [17];
  ubyte  t, d, c, tt, nt [128];
  ulong  ln, maxt = 0;
  bool   got;
TRC("{ Song::Load");

// load .song/.trak
   if (! SongLoad (fn, & _ev, & _nEv, 0, _t))
      Die ("Song::Load  SongLoad died");
   _nTrk = (ubyte) _t [TB_TRK].NRow ();   _nCtl = (ubyte) _t [TB_CTL].NRow ();
   _nLyr = (ulong) _t [TB_LYR].NRow ();
// _t [TB_TRK].Dump (); _t [TB_CTL].Dump ();
// _t [TB_LYR].Dump (); _t [TB_DRM].Dump ();

// pick _trk [].dev n open if new;  .chn based on +;  Drum*;  next avail chn
   for (ln = 0, t = 0; t < _nTrk; t++) {
TRC("pickin dev,chn trk=`d",t);
   // set .e, ne from table
      _trk [t].e = & _ev [ln];
      ln += (_trk [t].ne = Str2Int (_t [TB_TRK].Get (t, 3)));

   // init .p,
      _trk [t].p = 0;

      if (t && (_t [TB_TRK].Get (t, 2) [0] == '+'))
         {d = _trk [t-1].dev;  c = _trk [t-1].chn;}
      else {
      // to pick dev, try non . devName, then 01..99
         StrCp (dTry, _t [TB_TRK].Get (t, 0));
         if (StrCm (dTry, ".") == 0)  StrCp (dTry, "01");

      // step try devs till one has chans
         for (got = false; ! got;) {
            for (d = 0; d < _nDev; d++) if (StrCm (_dev [d], dTry) == 0)  break;
//DBG("_nDev=`d d=`d dTry=`s",_nDev,d,dTry);
            if (d >= _nDev) {          // new one
               if (d >= MAX_DEV)  Die ("Song::Load  too many devices");
               StrCp (_dev [_nDev++], dTry);
            }

            if (! MemCm (_t [TB_TRK].Get (t, 1), "Drum", 4)) {
TRC(" Drum");
               c = 9;  got = true;  // default to got it but look for dup
/*
               for (tt = 0; tt < t; tt++)
                  if ((_trk [tt].dev == d) && (_trk [tt].chn == 9))
                     {got = false;  break;}
*/
            }
            else {
TRC(" Melo");
               StrCp (cmap, ".........d......");
               for (tt = 0; tt < t; tt++)  if (_trk [tt].dev == d)
                  cmap [_trk [tt].chn] = 'x';
               for (c = 0; c < 16; c++)  if (cmap [c] == '.')  break;
               if (c < 16)  got = true;
            }
            if (! got) {  // no dice - gotta blow...:(
               if ((d = (ubyte) Str2Int (dTry) + 1) == 100)
                  Die ("Song::Load  tooo many devices needed");
               StrFmt (dTry, "`02d", d);
            }
         }
      }
      _trk [t].dev = d;
      _trk [t].chn = c;
TRC(" dev=`d=`s chn=`d", d, _dev [d], c);
   }

TRC("sortin' evs");
   MemSet (nt, 0, sizeof (nt));
   for (t = 0;  t < _nTrk;  t++) {
//DBG("trk=`d  ne=`d",t,_trk [t].ne);
   // sort play events;
      Sort (_trk [t].e, _trk [t].ne, sizeof (TrkEv), TrkCmp);
   // calc maxt
      if (_trk [t].ne && (_trk [t].e [_trk [t].ne-1].time > maxt))
         maxt =           _trk [t].e [_trk [t].ne-1].time;
   }
   for (t = 0; t < 128; t++)  if (nt [t] && (t >= _nCtl))
      Die ("Song::Load  unmapped controls in .trak - edit w PianoCheetah");
TRC("} Song::Load");
}


//------------------------------------------------------------------------------
void PutByt (ubyte b)
{  if (Len >= sizeof (Buf))  Die ("PutByt  Buf too small - bug Steve...:(");
   Buf [Len++] = b;
}

void PutVar (ulong v)
// Write oneuh those wierd variable length numbers...
{ ubyte b;
  char  bgn = 'n';
   for (ubyte p = 0; p < 5; p++) {
      b = (ubyte)((v >> (7*(4-p))) & 0x7F);
      v &= (0x0FFFFFFF >> (7*p));
      if (bgn == 'n')  if (b || (p == 4))  bgn = 'y';
      if (bgn == 'y')  PutByt (b | ((p == 4) ? 0 : 0x80));
   }
}

void PutTime (ulong t)           {PutVar (t - PTime);  PTime = t;}

void PutStat (ubyte s)           {if (s != PStat)  PutByt (PStat = s);}

void PutMeta (ubyte m, ulong l)  {PutByt (0xFF);  PutByt (m);  PutVar (l);
                                  PStat = 0;}

void PutStr (char *b)            {while (*b)  PutByt (*b++);}


void Song::BufTrk (ubyte t)
{ ubyte  st, chn, c, prog, prog1, prog2;
  bool               doBank, got;
  ulong  tm = 0, tm1;
  uword  i, ln, cr;
  TStr   buf, cs, ts;
  char  *p;
  TrkEv *e;
  ubyte  mvol [8] = {0xF0,0x7F,0x7F,0x04,0x01,0,0,0xF7},
         mbal [8] = {0xF0,0x7F,0x7F,0x04,0x02,0,0,0xF7};
//       mtun [8] = {0xF0,0x7F,0x7F,0x04,0x04,0,0,0xF7};
                                       // 3,4 are +-cent/8192,cent
TRC("{ Song::BufTrk");
//DBG("`d/`d -------------------", t, _nTrk);
   Len = 0;   MemSet (Buf, 0, sizeof (Buf));
   PStat = 0;  PTime = 0;

   chn = _trk [t].chn;

// do track 0 (header) stuph
   if (t == 0) {
   // jam DSC,DRM,CTL stuff into a ditty midi header thingy
      SBuf [0] = '\0';                 // first, top stuff
      for (i = 0, ln = (uword) _t [TB_DSC].NRow ();  i < ln;  i++) {
         StrCp (buf, _t [TB_DSC].Get (i, 0));
      // Copyright gets a sep spot
         if      (p = StrSt (buf, "Copyright=")) {
            p += 10;                   // skip to after Copyright=
            PutTime (0);  PutMeta (0x02, StrLn (p));  PutStr (p);
         }
         else if (p = StrSt (buf, "(c)")) {
            p += 3;                    // skip to after (c)
            PutTime (0);  PutMeta (0x02, StrLn (p));  PutStr (p);
         }
         else  {StrAp (SBuf, buf);   StrAp (SBuf, "\r\n");}
      }
      if (ln = (uword) _t [TB_DRM].NRow ()) {
         StrAp (SBuf, "DrumMap:\r\n");
         for (i = 0; i < ln; i++) {
            StrFmt (buf, "`s `s `s `s `s `s `s\r\n",
               _t [TB_DRM].Get (i, 0),  _t [TB_DRM].Get (i, 1),
               _t [TB_DRM].Get (i, 2),  _t [TB_DRM].Get (i, 3),
               _t [TB_DRM].Get (i, 4),  _t [TB_DRM].Get (i, 5),
                                        _t [TB_DRM].Get (i, 6));
            StrAp (SBuf, buf);
         }
      }
      if (ln = (uword) _t [TB_CTL].NRow ()) {    // preserve the +s
         StrAp (SBuf, "Control:\r\n");
         for (i = 0; i < ln; i++) {
            StrFmt (buf, "`s\r\n", _t [TB_CTL].Get (i, 0));
            StrAp (SBuf, buf);
         }
      }
   // pee on it so Mid2Song knows it came from ME
      PutTime (0);   PutMeta (0x7F, 9 + StrLn (SBuf) + 1);
      PutStr ("ditty_top");   PutStr (SBuf);   PutByt (0);
   }

// DevName - device(port)name
   p = _dev [_trk [t].dev];
   PutTime (0);  PutMeta (0x09, StrLn (p));  PutStr (p);

// SndName - program(patch)name
   p = _t [TB_TRK].Get (t, 1);
   PutTime (0);  PutMeta (0x08, StrLn (p));  PutStr (p);

// TrkName - sequence/trackName
   p = _t [TB_TRK].Get (t, 2);
   PutTime (0);  PutMeta (0x03, StrLn (p));  PutStr (p);

// try to calc a prog (GM progchange) and prog1,prog2 (bankselect hi/lo)
   StrCp (buf, _t [TB_TRK].Get (t, 1));
   ln = (uword) StrLn (buf);
   if ((ln > 6) && (buf [ln-7] == '.')) {
        prog2 = (ubyte) Str2Int (& buf [ln-3]);   buf [ln-3] = '\0';
        prog1 = (ubyte) Str2Int (& buf [ln-6]);   buf [ln-7] = '\0';
   }
   else prog1 = prog2 = 0;
   for (i = 0; i < 128; i++)  if (StrCm (buf, MProg [i]) == 0)  break;
   if (i < 128)  {prog = (ubyte) i;}
   else {
   // dang - try a "top level dir" only match  :(
      for (i = 0; i < 128; i += 8) {
         for (ln = 0; ln < StrLn (MProg [i]); ln++)
            if (MProg [i][ln] == '\\')  break;             // always has one
         if (MemCm (MProg [i], buf, ln+1) == 0)  break;
      }
      if (i < BITS (MProg [i]))  {prog = (ubyte) i;}
      else                       {prog = (ubyte) 0;}       // DANG!  piano ??
   }

// see if we need to bother with bankselect
   doBank = false;
   for (i = 0; i < _nTrk; i++) {
      ln = (uword) StrLn (_t [TB_TRK].Get (t, 1));
      if ((ln > 6) && (_t [TB_TRK].Get (t, 1) [ln-7] == '.'))  break;
   }
   if (i < _nTrk)  doBank = true;

// if non drum AND not a + track, write initial programchange/bankselect
   if ((chn != 9) && (_t [TB_TRK].Get (t, 2)[0] != '+')) {
      if (doBank) {                              // cc#0=bankselect
         PutTime (0);  PutStat (M_CTRL|chn);  PutByt (M_BANK);  PutByt (prog1);
         PutTime (0);  PutStat (M_CTRL|chn);  PutByt (M_BNKL);  PutByt (prog2);
      }
      PutTime    (0);  PutStat (M_PROG|chn);                    PutByt (prog);
   }

// dump the events...
   for (ulong p = 0; p < _trk [t].ne; p++) {
      e = & _trk [t].e [p];   tm = e->time;
//DBG("p=`d/`d tm=`d", p, _trk [t].ne, tm);

   // for track 0, put any lyric events before this ev
      if (t == 0)  do {
         got = false;
         if (_pLyr < _nLyr) {
            tm1 = (ulong) Str2Int (_t [TB_LYR].Get (_pLyr, 0));
            if (tm1 <= tm) {
               got = true;
               PutTime (tm1);   PutMeta (0x05,
                                         StrLn (_t [TB_LYR].Get (_pLyr, 1)));
                                PutStr  (       _t [TB_LYR].Get (_pLyr, 1));
               _pLyr++;
            }
         }
      } while (got);

   // put the delta time
      PutTime (tm);

   // gotta note ?
      if (((c = e->ctrl) & 0x80) == 0) {
         if      ((e->valu & 0x80) == 0)  st = M_NOFF;
         else if ((e->val2 & 0x80) == 0)  st = M_NOTE;
         else                             st = M_NPRS;
//TStr ts1;
//DBG("note=`s", MKey2Str (ts1, c));
         PutStat (st|chn);
         PutByt (c);
         PutByt (e->valu & 0x7F);
         if ((st == M_NOTE) && e->val2) {   // got fingering?
            StrFmt (buf, "ditty_fing=`s,`s",
                     MFing [e->val2-1], MKey2Str (ts, e->ctrl));
            PutTime (tm);   PutMeta (0x7F, StrLn (buf));   PutStr (buf);
         }
      }
      else {                           // gotta controller
         c &= 0x7F;
         StrCp (cs, _t [TB_CTL].Get (c, 0));
         if (*cs == '+')  StrCp (cs, & cs [1]);
         for (c = 0; c < NMCC; c++)  if (! StrCm (MCC [c].s, cs))
            {cr = MCC [c].raw;   break;}
         if (c >= NMCC)  cr = MCtl2Int (cs);
         if      (! StrCm (cs, "Tmpo")) {
//TRC("tmpo");
           ulong bpm = (e->val2 << 8) | e->valu;
           ulong out = 60000000 / bpm;
            if       ((60000000 % bpm) >= (bpm / 2))  out++;
            PutMeta (0x51, 3);  PutByt ((ubyte)((out>>16) & 0x00FF));
                                PutByt ((ubyte)((out>> 8) & 0x00FF));
                                PutByt ((ubyte)((out>> 0) & 0x00FF));
         }
         else if (! StrCm (cs, "TSig")) {
           ubyte den, sub;
            den = (e->val2 & 0x0F);
            sub = (e->val2 >> 4);
//TRC("tsig");
            PutMeta (0x58, 4);  PutByt (e->valu);          PutByt (den);
                                PutByt (96/(1<<den));      PutByt (8);
            StrFmt (buf, "ditty_subbeat=`c,`d,`d", '0'+sub, e->valu, den);
            PutTime (tm);   PutMeta (0x7F, StrLn (buf));   PutStr (buf);
         }
         else if (! StrCm (cs, "KSig")) {
           ubyte key, min, flt;
            key =  e->valu;
            min =  e->val2 & 0x01;
            flt = (e->val2 & 0x80) ? 0x01 : 0x00;
            if (KyNum [min][flt][key] == 9)  flt = (flt?0x00:0x01);
            key = KyNum [min][flt][key];
//TRC("ksig");
            PutMeta (0x59, 2);   PutByt ((ubyte)(flt ? -key : key));
                                 PutByt (min);
         }
         else if (cr == MC_PROG) {     // big 3 midi evs
//TRC("prog");
            if (chn != 9) {
               if (doBank) {
                  PutStat (M_CTRL|chn);  PutByt (M_BANK);  PutByt (prog1);
                  PutTime (tm);
                  PutStat (M_CTRL|chn);  PutByt (M_BNKL);  PutByt (prog2);
                  PutTime (tm);
               }
               PutStat    (M_PROG|chn);                    PutByt (prog);
            }
         }
         else if (cr == MC_PRSS) {
//TRC("prss");
            PutStat (M_PRSS|chn);  PutByt (e->valu);
         }
         else if (cr == MC_PBND) {
//TRC("pbnd");
            PutStat (M_PBND|chn);  PutByt (e->val2);     PutByt (e->valu);
         }
         else if (cr < MC_US) {        // cc
//TRC("ctrl");
            PutStat (M_CTRL|chn);  PutByt (cr - MC_CC);  PutByt (e->valu);
//DBG("`02x `02x `02x", M_CTRL|chn, cr-MC_CC, e->valu);
         }
         else if (cr < MC_RP) {        // univ sysex
            if      (c == MC_MVOL)  {
//TRC("mvol");
               mvol [6] = e->valu;  mvol [5] = e->valu ? 0x7F : 0;
               PutByt (M_SYSX|chn);  PStat = 0;  PutVar (8);
               for (c = 0; c < BITS (mvol); c++) PutByt (mvol [c]);
            }
            else if (c == MC_MBAL)  {
//TRC("mbal");
               mbal [6] = e->valu;
               PutByt (M_SYSX|chn);  PStat = 0;  PutVar (8);
               for (c = 0; c < BITS (mbal); c++) PutByt (mbal [c]);
            }
         }
         else if (cr < MC_NP) {        // rpns
            PutStat (M_CTRL|chn);
            PutByt (M_RPNH);   PutByt (((cr - MC_RP) >> 8) & 0x7F);
            PutTime (tm);
            PutByt (M_RPNL);   PutByt (((cr - MC_RP) >> 1) & 0x7F);
            PutTime (tm);
            PutByt ((cr & 0x0001) ? M_DATL : M_DATH);  PutByt (e->valu);
         }
         else {                        // nrpns
            PutStat (M_CTRL|chn);
            PutByt (M_NRPNH);  PutByt (((cr - MC_NP) >> 8) & 0x7F);
            PutTime (tm);
            PutByt (M_NRPNL);  PutByt (((cr - MC_NP) >> 1) & 0x7F);
            PutTime (tm);
            PutByt ((cr & 0x0001) ? M_DATL : M_DATH);  PutByt (e->valu);
         }
      }
   }

// any lyrics left after max ev time?
   if (t == 0)  while (_pLyr < _nLyr) {
      tm1 = (ulong) Str2Int (_t [TB_LYR].Get (_pLyr, 0));
      PutTime (tm1);   PutMeta (0x05, StrLn (_t [TB_LYR].Get (_pLyr, 1)));
                       PutStr  (             _t [TB_LYR].Get (_pLyr, 1));
      _pLyr++;
   }

// implied last event (required) - EndOfTrack ($2F)
//TRC("EOT");
   PutTime (PTime);  PutMeta (0x2F, 0);
TRC("} Song::BufTrk");
}


void Song::SaveMid (char *fn)
{ File f;
TRC("{ Song::SaveMid");
   if (! f.Open (fn, "wb"))  Die ("SaveMid can't write .mid file", fn);
   MHdr [11] = _nTrk;
   f.Put (MHdr, sizeof (MHdr));
   for (ubyte t = 0; t < _nTrk; t++) {
      BufTrk (t);
      MTrk [4] = (ubyte)(Len>>24);   MTrk [5] = (ubyte)(Len>>16);
      MTrk [6] = (ubyte)(Len>> 8);   MTrk [7] = (ubyte)(Len    );
      f.Put (MTrk, sizeof (MTrk));   f.Put (Buf, Len);
   }
   f.Shut ();
TRC("} Song::SaveMid");
}


//------------------------------------------------------------------------------
int Go ()
{ TStr  fn, tfn, ep;
  char  tid;
TRC("{ Go Song2Mid `s", App.parm);
   tid =        App.parm [0];
   StrCp (fn, & App.parm [2]);
   if (tid == '\0')  Die ("Hey!  Song2Mid t filename.mid");

  Song *s = new Song ();

   App.Path (ep, 'e');   StrFmt (tfn, "`s\\tmp_`c.song", ep, tid);
   s->Load   (tfn);
   s->SaveMid (fn);

   delete s;

TRC("} Go");
   return 0;
}
