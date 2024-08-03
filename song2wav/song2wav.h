// song2wav.h - song => wav  via syn
// this is a clone of pianocheetah with ONLY
// song load, syn write.  but to .WAV, not live to soundcard.

#include "stv/os.h"
#include "stv/midi.h"
#include "stv/syn.h"

const ubyt2 MAX_CTL = 512;             // controls to map
const ubyt2 MAX_SIG = 4096;            // timesigs
const ubyt4 MAX_EV  = 100*1024;        // whatev

const ubyte TB_DSC = 0;                // my .song table format stuph
const ubyte TB_TRK = 1;                // Track:    dev snd trkName
const ubyte TB_DRM = 2;                // DrumMap:  note snd
const ubyte TB_LYR = 3;                // Lyric:    time name
const ubyte TB_EVT = 4;                // Event:    placeholder fake STable...:/
const ubyte TB_MAX = 5;                // (num of em)


class SmpTmr {
public:
   SmpTmr ()  {_outFrq = 44100;   _tempo = 120;   _time = 0;   _sampOfs = 0.;}

// stuff to actually use
   void  SetTempo (ubyt4 tempo)   {_tempo = tempo;}

   ubyt4 Set (ubyt4 stime)   // advance to stime and return #samples to write
   { ubyt4 tOfs = stime - _time, s;
     real  samp;
      _time = stime;
      samp = (real)(tOfs * _outFrq * 5) / (16. * _tempo) + _sampOfs;
      s = (ubyt4)samp;
      samp -= (real)s;
      if (samp >= 0.5) {s++;   samp -= 1.;}
      _sampOfs = samp;
      return s;
   }

   ubyt4 _outFrq, _tempo, _time;
   real  _sampOfs;
};


//------------------------------------------------------------------------------
struct CcDef {TStr map;   TStr dsc;   ubyt2 raw;};
#include "devtyp.h"

struct TSgRow {ubyt4 time;  ubyte num, den;  ubyt2 bar;};

struct TrkRow {TStr   name, snd;
               ubyt2  chn;
               bool   grp, shh;
               TrkEv *e;
               ubyt4 ne, p;};

struct DrmRow {ubyte ctl;   TStr snd;   bool shh;};  // syn drum bank entry

class Song {
public:
   void Init ()  {_trk.Ln = _ctl.Ln = 0;   _dvt.Init ();}

// Song
   void  DumpEv (TrkEv *e, ubyte t, ubyt4 p);
   void  Dump   ();
   void  Wipe   ();
   void  SetChn (ubyte trk);
   void  PutCC  (ubyte t, TrkEv *e);
   void  PutNt  (ubyte t, TrkEv *e);
   ubyt4 Put    (File *f);

   ubyt4 Bar2Tm (ubyt2 b, ubyte bt);   // sTime.cpp
   ubyt4 Str2Tm (char *s);
   char *TmStr  (char *str, ubyt4 sTm, ubyt4 *ttL8r = NULL);

   void  Load   (char *fn);            // sLoad.cpp
private:
public:
   ubyt4   _now, _tEnd;                // current song time, end time
   SmpTmr  _timer;
   DevTyp  _dvt;                       // always syn
   TStr    _bnk [256];
   Arr<TrkRow,MAX_TRK> _trk;
   Arr<DrmRow,128>     _drm;
private:
   Arr<TStr,  MAX_CTL> _ctl;
   Arr<TSgRow,MAX_SIG> _tSg;
   TrkEv               _ev [MAX_EV];
   ubyt4              _nEv;
};

extern Song Sg;
