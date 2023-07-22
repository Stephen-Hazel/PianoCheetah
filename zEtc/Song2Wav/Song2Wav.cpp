// Song2Wav.cpp  .song => .wav via Syn but directly...

#include "Song2Wav.h"

#include "SongLoad.cpp"                // common load stuph

CtlEdit *Msg;

int TrkCmp (void *p1, void *p2)  // ...TrkEv sortin by time,ctrl,up/prs/dn
{ TrkEv *e1 = (TrkEv *)p1, *e2 = (TrkEv *)p2;
  int t;
   if (t = e1->time - e2->time)  return t;
   if (t = e1->ctrl - e2->ctrl)  return t;
   if (t = (e1->valu & 0x80) - (e2->valu & 0x80))  return t;  // up / pr,dn
   return  (e2->val2 & 0x80) - (e1->val2 & 0x80);             // pr / dn
}

int SigCmp (void *p1, void *p2)  // ..._sig sortin (time signatures)
{ ulong t1 = *((ulong *)p1), t2 = *((ulong *)p2);
   return t1 - t2;
}

//------------------------------------------------------------------------------
void Song::Dump (bool ev)
{ ubyte t;
  uword s;
   DBG("{ DUMP");
   if (ev) {
     TStr db2;  char *p;
      DBG("evTrk.pos time     ctrl valu val2 x");
      for (t = 0;  t < _nTrk;  t++)
         if (_trk [t].name [0] && (! MemCm (& _trk [t].name [1], "REC", 3))) {
           TrkEv *ev = _trk [t].e;
            for (ulong e = 0;  e < _trk [t].ne;  e++, ev++) {
               if (ev->ctrl & 0x0080) p = Int2Str  (ev->ctrl & 0x7F,db2);
               else                   p = MKey2Str (db2,ev->ctrl);
               DBG("`02d.`06d `08d `<4s `02x   `02x   `02x",
               t, e, ev->time, p, ev->valu, ev->val2, ev->x);
            }
         }
      DBG("DUMP }");
      return;
   }
   DBG("now=`d nEv=`d", _now, _nEv);
   DBG("trk name                      chn      snd        e       ne"
       "        p      dur mute");
   for (t = 0;  t < _nTrk;  t++)
      DBG("`>3d `<25s `>3d `>8d `08x `>8d `>8d `>8d    `c",
         t,
         (char *)_trk [t].name,
         _trk [t].chn,
         _trk [t].snd,
         _trk [t].e,
         _trk [t].ne,
         _trk [t].p,
         _trk [t].dur,
         _trk [t].mute?_trk [t].mute:' '
      );

   DBG("ctl name");
   for (t = 0;  t < _nCtl;  t++)  DBG("`>3d `s", t, (char *)_ctl [t]);
   DBG("cch chn ctl      trk valu val2     time");
   for (t = 0;  t < _nCCh;  t++)
      DBG("`>3d `>3d `<8s `>3d `>4d `>4d `>8d",
         t,
         _cch [t].chn,
         (char *)_ctl [_cch [t].ctl & 0x7F],
         _cch [t].trk,
         _cch [t].valu,
         _cch [t].val2,
         _cch [t].time
      );
   DBG("sig     time  bar num den");
   for (s = 0;  s < _nSig;  s++)
      DBG("`>3d `>8d `>4d `>3d `>3d",
         s,
         _sig [s].time,
         _sig [s].bar,
         _sig [s].num,
         _sig [s].den
      );
   DBG("} DUMP");
}


//------------------------------------------------------------------------------
void Song::Wipe ()  // wipe all data and "empty" display
{ uword i;
   for (i = 0;  i < 16;  i++)  _syn->Put ((ubyte)i, MC_CC|M_ASOFF, 0, 0);
   _now = 0;
   for (i = 0;  i < _nTrk;  i++)  _trk [i].name [0] = '\0';  _nTrk = 0;
   for (i = 0;  i < _nCtl;  i++)  _ctl [i][0]       = '\0';  _nCtl = 0;
   if (_ev)  delete [] _ev;
   _nEv = 0;   _ev = NULL;
   _nCCh = _nSig = 0;
   _nSig = 0;
   _timer.SetTempo (120);
}


//------------------------------------------------------------------------------
void Song::SetChn (ubyte t)
// send BankLo/Bank/ProgCh on trk's dev/chn/snd
{ ubyte p;
  uword c = _trk [t].chn;
  ulong s = _trk [t].snd;
   if ((s == SND_NONE) || (c == 9))  return;
   for (p = 0;  p < _nSyM;  p++)  if (_syM [p] == s)  break;
   _syn->Put ((ubyte)c, MC_PROG, p, 0);
}


//------------------------------------------------------------------------------
void Song::Load (char *fn)
// ok, go thru the BRUTAL HELL to load in a song, etc...
{ TStr  fnt, buf;
  ubyte t, c, tc, x, nt [128];
  uword s;
  ulong e, cc, maxt = 0;
  TrkEv *ev;
  uword nxtChn;
  File  f;
  ubyte d;
  ulong ts;
DBG("{ Song::Load");
   { File f;   if (! f.Size (fn))  Die ("can't read file", fn);  }

DBG("loading .song");
  STable st [TB_MAX];
   if (! SongLoad (fn, & _ev, & _nEv, 0, st))  return;

DBG("transfer tables to song structs");
   _nTrk = (ubyte) st [TB_TRK].NRow ();   _nCtl = (ubyte) st [TB_CTL].NRow ();
st [TB_TRK].Dump (); st [TB_CTL].Dump ();
   for (e = 0;  e < _nCtl;  e++) {
      StrCp (_ctl [e], st [TB_CTL].Get (e, 0));
      if (_ctl [e][0] == '+')  StrCp (_ctl [e], & _ctl [e][1]);
   }
DBG("init _trk[] dev/chn/snd calcs");
   nxtChn = 0;
   for (_nEv = 0, t = 0;  t < _nTrk;  t++) {
   // set .name, e, ne from table
      StrCp (_trk [t].name, st [TB_TRK].Get (t,4));
      _trk [t].e = & _ev [_nEv];
      _nEv += (_trk [t].ne = Str2Int (st [TB_TRK].Get (t,2)));

   // init .p, set .mute later
      _trk [t].p = 0;

   // pick .chn based on +, drum
      if      (t && (_trk [t].name [0] == '+'))
         c = (ubyte)_trk [t-1].chn;
      else if (! MemCm (st [TB_TRK].Get (t, 1), "Drum\\", 5))
         c = 9;
      else {
         if (nxtChn == 255)  Die ("Hit 256 channels :(");
         c = (ubyte)nxtChn++;   if (c == 9) c = (ubyte)nxtChn++;
      }
      _trk [t].chn  = c;
      _trk [t].mute = _trk [t].name [1];
      if (StrCh ("#?", _trk [t].mute) == NULL)  _trk [t].mute = '\0';

   // map sndName to an id;
      _trk [t].snd = _dvt.SndID (st [TB_TRK].Get (t, 1));
      _trk [t].drm = PRG_NONE;
   }

   MemSet (nt, 0, sizeof (nt));
   for (t = 0;  t < _nTrk;  t++) {
   // sort play events;  build _sig[] n _cch[]
      Sort (_trk [t].e, _trk [t].ne, sizeof (TrkEv), TrkCmp);
      for (c = (ubyte)_trk [t].chn, ev = _trk [t].e,
           e = 0;  e < _trk [t].ne;  e++) {
         if ((tc = ev [e].ctrl) & 0x80) {
            if (! StrCm (_ctl [tc & 0x7F], "TSig")) {
               if (_nSig >= BITS (_sig))  Die ("Song::Load  too many timesigs");
               _sig [_nSig].time = ev [e].time;
               _sig [_nSig].num  = ev [e].valu;
               _sig [_nSig].den  = 1 << (ev [e].val2 & 0x0F);
               _sig [_nSig].sub  = 1 +  (ev [e].val2 >> 4);
               _nSig++;
            }
            for (cc = 0;  cc < _nCCh;  cc++)
               if ((_cch [cc].chn == c) && (_cch [cc].ctl == tc))  break;
            if ((cc >= _nCCh) && (_nCCh < BITS (_cch))) {
               _cch [_nCCh].chn = c;
               _cch [_nCCh].ctl = tc;  _nCCh++;
               nt [tc & 0x7F] = 1;
            }
         }
      }
   // calc maxt
      if (_trk [t].ne && (_trk [t].e [_trk [t].ne-1].time > maxt))
         maxt =           _trk [t].e [_trk [t].ne-1].time;
   }
   for (t = 0;  t < 128;  t++)  if (nt [t] && (t >= _nCtl))
      Die ("Song::Load  unmapped controls in .trak - edit with PianoCheetah");
   _dvt.SetCCMap (_ctl, _nCtl);

DBG("tsig/lyr");
// sort timesignatures n recording info
   Sort (_sig, _nSig, sizeof (_sig [0]), SigCmp);
// align maxt up to next bar boundary
   if (_nSig == 0)  {
      maxt /= M_WHOLE;  maxt++;  maxt *= M_WHOLE;}
   else {
      s = 0;   while ((s+1 < _nSig) && (maxt >= _sig [s+1].time))  s++;
      maxt -= _sig [s].time;
      maxt /= (M_WHOLE * _sig [s].num / _sig [s].den);
      maxt++;
      maxt *= (M_WHOLE * _sig [s].num / _sig [s].den);
      maxt += _sig [s].time;
   }
   for (t = 0;  t < _nTrk;  t++) _trk [t].dur = maxt;

   TmMap ();

DBG("DrumMap/used drums -> _syD");
   _nSyD = _nSyM = 0;
   for (x = 0;  x < (ubyte) st [TB_DRM].NRow ();  x++) {
// ctl  sndName/.  vol  pan  mute/.  ht/.  inpctl
      _syD [_nSyD].ctl  = MDrm2Int   (     st [TB_DRM].Get (x, 0));
      _syD [_nSyD].snd  = _dvt.SndID (     st [TB_DRM].Get (x, 1));
      _syD [_nSyD].vol  = (ubyte) Str2Int (st [TB_DRM].Get (x, 2));
      _syD [_nSyD].pan  = (ubyte) Str2Int (st [TB_DRM].Get (x, 3));
      _syD [_nSyD].mute =                  st [TB_DRM].Get (x, 4) [0];
      _nSyD++;                         // .snd could still be SND_NONE
   }

// add any used drum notes in tracks to _syD[]
   MemSet (nt, 0, sizeof (nt));
   for (t = 0;  t < _nTrk;  t++)  if (_trk [t].chn == 9)
      for (e = 0;  e < _trk [t].ne;  e++)   // set nt [used drum ctls]
         if ((_trk [t].e [e].ctrl & 0x80) == 0)  nt [_trk [t].e [e].ctrl] = 1;
   for (c = 0;  c < 128;  c++)  if (nt [c]) {
      for (e = 0;  e < _nSyD;  e++)  if (_syD [e].ctl == c)  break;
      if (e >= _nSyD) {
         _syD [_nSyD].ctl  = c;
         _syD [_nSyD].snd  = _dvt.SndID (MDrm2StG (buf, c));
         _syD [_nSyD].vol  = 127;
         _syD [_nSyD].pan  =  64;
         _syD [_nSyD].mute = '\0';
         _nSyD++;
      }
      else
         {if (_syD [e].snd == SND_NONE)
              _syD [e].snd = _dvt.SndID (MDrm2StG (buf, c));}
   }

// add melodic snds to _syM[]
   for (t = 0;  t < _nTrk;  t++)  if (_trk [t].chn != 9) {
      ts = _trk [t].snd;   if (ts == SND_NONE) continue;
      for (s = 0;  s < _nSyM;  s++)  if (_syM [s] == ts)  break;
      if (s >= _nSyM) {
         if (_nSyM >= 128)  Die ("Song::SetBnk  too many sounds:(");
         _syM [_nSyM++] = ts;
      }
   }

// write melo,drum snds to soundbank.txt for syn
   App.Path (fnt, 'd');   StrAp (fnt, "\\device\\syn\\SoundBank.txt");
DBG("write SoundBank.txt");
   if (! f.Open (fnt, "w"))  Die ("Song::Load  couldn't write file", fn);
   for (d = 0;  d < _nSyM;  d++) {
      f.Put (StrFmt (buf, "`s\r\n", (char *)_dvt.Snd (_syM [d])->name));
DBG("   melo=`s", buf);
   }
   if (_nSyD)  f.Put ("Drum\r\n");
   for (d = 0;  d < _nSyD;  d++) {     // drum ctrl name n sound name
      MDrm2Str (buf, _syD [d].ctl);   StrAp (buf, " ");
      StrAp (buf, (char *)_dvt.Snd (_syD [d].snd)->name);
      StrAp (buf, "\r\n");
      f.Put (buf);
DBG("   drum=`s", buf);
   }
   f.Shut ();
   _syn->LoadSound ();

// syn.h just STOMPED ON App.trc

// do each drum chn's vol,pan;  each melodic chn's init progch
   for (t = 0;  t < _nTrk;  t++) {
      if (_trk [t].chn == 9) {
         for (x = 0;  x < _nSyD;  x++)  if (_trk [t].drm == _syD [x].ctl)
            {_syn->Put (0x80 | _syD [x].ctl, MC_CC|M_VOL, _syD [x].vol, 0);
             _syn->Put (0x80 | _syD [x].ctl, MC_CC|M_PAN, _syD [x].pan, 0);
             break;}
      }
      else
         if (_trk [t].name [0] != '+')  SetChn (t);
   }
   Dump ();   _dvt.Dump ();
DBG("} Song::Load");
}


void Song::PutCC (ubyte t, TrkEv *e)
{ ubyte c = e->ctrl & 0x7F;
   if      (! StrCm (_ctl [c], "Tmpo"))
   { ulong tp = e->valu + (e->val2 << 8);   _timer.SetTempo (tp);}
   else if (! StrCm (_ctl [c], "TSig"))  ;
   else if (! StrCm (_ctl [c], "Prog"))  SetChn (t);
   else {
     uword craw = _dvt.CCMap [c];
      if (craw) _syn->Put ((ubyte)_trk [t].chn, craw, e->valu, e->val2);
   }
}


void Song::PutNt (ubyte t, TrkEv *e)
{ //TStr n;
// DBG("  PutNt chan=`d ctrl=`s valu=`02x val2=`02x",
//     _trk [t].chn+1, MKey2Str (n, e->ctrl),  e->valu, e->val2);
   _syn->Put ((ubyte)_trk [t].chn, e->ctrl,  e->valu, e->val2);
}


sword SmpBuf [2*1024];                 // 1024 stereo sword samples


ulong Song::Put (File *f, Waiter *w)
// the main dude - writes current slice of song (.p .. songtime) to midiouts
{ ubyte  t, ctl;
  ulong  tL8r, p, ne, tm, s, b, totSmp = 0;
  TStr   bar, end;
  TrkEv *e;
DBG("{ Song::Put");
   TmStr (bar, _trk [0].dur);   StrFmt (end, "`04d", Str2Int (bar)-1);
   for (_now = 0;  _now < _trk [0].dur;) {
      TmStr (bar, _now, & tL8r);   StrAp (bar, " / ");   StrAp (bar, end);
      w->Set ((ubyte)(Str2Int (bar) * 100 / Str2Int (end)), bar);
      if (w->die)  return 0;
DBG("bar=`s _now=`d tl8r=`d max=`d",bar,_now,tL8r,_trk[0].dur);
   // plow thru tracks from .p to tNow and write events
      for (t = 0;  t < _nTrk;  t++) {
         for (e = _trk [t].e,  ne = _trk [t].ne,  p = _trk [t].p;
              (p < ne) && (e [p].time <= _now);   p++) {
            if      ((ctl = e [p].ctrl) & 0x80)  PutCC (t, & e [p]);
            else if ( _trk [t].mute == '\0')     PutNt (t, & e [p]);
         }
         _trk [t].p = p;
         if (p < ne)  if ((tm = e [p].time) < tL8r)  tL8r = tm;
      }

   // when do we write events next?  render samples till then
      _now = tL8r;
      totSmp += (s = _timer.Set (_now));
      while (s) {
         b = (s > 1024) ? 1024 : s;   s -= b;
         _syn->PutWav (b, SmpBuf);   f->Put (SmpBuf, b*4);
      }
   }
DBG("} Song::Put totSmp=`d", totSmp);
   return totSmp;
}


//------------------------------------------------------------------------------
void WavHdr (File *f, ulong totLen = 0)
// rewind and update header with lengths
{ ulong ln1, ln2, ln3;
  WAVEFORMATEX wf;
   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = 2;
   wf.wBitsPerSample  = 16;
   wf.nSamplesPerSec  = 44100;
   wf.nBlockAlign     = wf.nChannels   * wf.wBitsPerSample / 8;
   wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
   ln3 = totLen*4;                     // stereo * sizeof (sword)
   ln2 = 16;
   ln1 = 12 + ln2 + 8 + ln3;
   f->Seek (0, "<");
   f->Put ("RIFF"    );  f->Put (& ln1, 4);
   f->Put ("WAVEfmt ");  f->Put (& ln2, 4);  f->Put (& wf, ln2);
   f->Put ("data"    );  f->Put (& ln3, 4);
}


void Cvt (Waiter *w)
{ TStr  fn;
  File  f;
  Song *sng = new Song ();
  ulong totLen;
DBG("{ Cvt");
   StrCp (fn, App.parm);   sng->Wipe ();   sng->Load (fn);

// open wav and write placeholder header
   StrAp (fn, ".wav", 4);
   if (! f.Open (fn, "w"))  Die ("can't write file", fn);
   WavHdr (& f);

   totLen = sng->Put (& f, w);

// update header with REAL length and close file
   WavHdr (& f, totLen);
   f.Shut ();

DBG("gonna wipe");
   sng->Wipe ();
   ::Sleep (50);
DBG("gonna delete sng");
   delete sng;
DBG("} Cvt");
}


//------------------------------------------------------------------------------
class ThrSong2Wav: public Thread {
public:
   ThrSong2Wav (Waiter *w): _w (w), Thread (w->wnd)  {}
private:
   Waiter *_w;

   int End ()  {PostDadW (MSG_CLOSE, 0, 0);   return 0;}

   DWORD Go ()
   { MSG  msg;
     TStr fnt;
DBG("{ ThrSong2Wav::Go");
   // hey wihnders!  gimme a message queue.
      ::PeekMessage (& msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
      msg.message = 0;
      Msg = _msg;
      App.SetTitle ("Song2Wav", App.parm);
   // refresh sound.txt in case of new sounds like pc does
      App.Path (fnt);   StrAp (fnt, "\\SynSound.exe");
      RunWait  (fnt);        // refresh sound.txt
      Cvt (_w);
DBG("cvt came back");
      PostDad (MSG_CLOSE, 0, 0);
DBG("} ThrSong2Wav::Go");
      return End ();
   }
   CtlEdit *_msg;
};


//------------------------------------------------------------------------------
class Song2Wav: public Dialog {
public:
   Song2Wav (): Dialog (IDD_APP, IDI_APP), _t (NULL) {}
  ~Song2Wav ()  {delete _t;}
private:
   ThrSong2Wav *_t;
   Waiter      _w;
   void Open ()  {_w.Init (Wndo ());   _t = new ThrSong2Wav (& _w);}
   void Done ()  {_w.Quit ();   _t->PostKid (WM_CLOSE, 0, 0);}
};

int Go ()
{ Song2Wav dlg;   dlg.Ok (NULL);   return 0; }


// need the stupid crt startup due to reals :(
int WINAPI WinMain (HINSTANCE inst, HINSTANCE pInst, LPSTR cmdLn, int nShowCmd)
{  InitCom ();
  int rc = AppBoot ();
   QuitCom ();
   return rc;
}
