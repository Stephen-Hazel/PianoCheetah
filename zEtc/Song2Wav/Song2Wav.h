// Song2Wav.h  mid => wav via Syn (but directly)

#include "rc\resource.h"
#include "ui.h"
#include "MidiIO.h"
#include "Syn.h"

const ubyte MAX_DEVO = 16;             // max midiout devices  for song
const uword MAX_CCH  = 512;            //     controllers to chase
const uword MAX_SIG  = 4096;           //     timesigs

const ubyte PRG_NONE = 255;            // prg value for "no program"

const uword SND_MAX  = 30*1024;        // max sounds per DevTyp
const ulong SND_NONE = 0xFFFFFFFF;     // no sound


//------------------------------------------------------------------------------
class SmpTmr {
public:
   SmpTmr (ulong outFrq = 44100, ulong tempo = 120)
   {_outFrq = outFrq;   _tempo = tempo;   _time = 0;   _sampOfs = 0.;}

// stuff to actually use
   void  SetTempo (ulong tempo)   {_tempo = tempo;}

   ulong GetTempo ()              {return  _tempo;}

   ulong Set (ulong stime)  // advance to stime and return #samples to write
   { ulong tOfs = stime - _time, s;
     real  samp;
      _time = stime;
      samp = (real)(tOfs * _outFrq * 5) / (16. * _tempo) + _sampOfs;
      s = (ulong)samp;
      samp -= (real)s;
      if (samp >= 0.5) {s++;   samp -= 1.;}
      _sampOfs = samp;
      return s;
   }

   ulong Get ()  {return _time;}

   ulong _outFrq, _tempo, _time;
   real  _sampOfs;
};


//------------------------------------------------------------------------------
struct SnDef {TStr name;   ubyte prog, bank, bnkL;};
struct CcDef {WStr map;   TStr dsc;   uword raw;};

// sounds and ctls used in each midiout devtype
class DevTyp {
public:
         DevTyp ();
   void  Open (char *devTyp);          // always "syn"
   void  Shut ();
   char *Name ()  {return _name;}

   uword CCID (char *name)
   {  for (ubyte c = 0; c < _nCc; c++)
         if (! StrCm (name, _cc [c].map))  return _cc [c].raw;
      return 0;
   }

   void SetCCMap (WStr *lst, ubyte len)
   { ubyte i;
      MemSet (CCMap, 0, sizeof (CCMap));
      for (i = 0; i < len; i++)  CCMap [i] = CCID (lst [i]);
   }
   uword CCMap [128];

   ulong  SndID  (char *name);
   SnDef *Snd    (ulong id)  {return & _sn [id];}
   void   Dump   ();
private:
   static char *SnRec (char *buf, uword len, ulong pos, void *ptr);
   static char *CcRec (char *buf, uword len, ulong pos, void *ptr);
   TStr  _name;
   SnDef _sn [SND_MAX];   ulong _nSn, _nDr;
   CcDef _cc [128];       ubyte _nCc;
};


//------------------------------------------------------------------------------
struct TrkDef {TStr   name;
               uword  chn;
               ubyte  drm;
               ulong  snd;
               TrkEv *e;
               ulong ne, p, dur;   char mute;};

struct CChDef {ubyte chn, ctl, trk, valu, val2;  ulong time;};
struct SigDef {ulong time;  ubyte num, den, sub;  uword bar;};  // traktime bar
struct SynDDef {ubyte ctl, vol, pan;
                ulong snd;   char  mute;};  // syn drum bank entry
extern int TrkCmp (void *p1, void *p2);     // ..._trk[] sortin
extern int RecCmp (void *p1, void *p2);     // ..._rec[] sortin

class Song {
public:
   Song ()
   {  App.Path (_aPath);   App.Path (_dPath, 'd');
      _nTrk = _nCtl = 0;   _dvt.Open ("syn");
      _syn = new Syn (& _timer._tempo);
      _ev = NULL;
   }
  ~Song ()
   {  //DBG("gonna delete _syn");
      delete _syn;
      //DBG("gonna shut _dvt");
      _dvt.Shut ();
      //DBG("~Song done");
   }

// Song
   void  Dump   (bool ev = false);
   void  Wipe   ();
   void  PutCC  (ubyte t, TrkEv *e);
   void  PutNt  (ubyte t, TrkEv *e);
   ulong Put    (File *f, Waiter *w);
   char *SndName (ubyte t)
   {  return _dvt.Snd (_trk [t].snd)->name;  }

// SongTime
   void  TmMap  ();
   ulong Bar2Tm (uword b);
   char *TmStr  (char *str, ulong sTm, ulong *ttL8r = NULL);
   void  TmHop  (ulong tm, bool killRec = false);
// SongFile
   void  Load   (char *fn);
private:
   void  SetChn (ubyte trk);
   static char *SongRec (char *buf, uword len, ulong pos, void *ptr);
public:
   ulong   _now, _reRo;                 // current song time, reroll bar?
   SmpTmr  _timer;
   Syn    *_syn;
   DevTyp  _dvt;                        // always syn
   TrkDef  _trk [MAX_TRK];   ubyte _nTrk;
   SigDef  _tSg [MAX_SIG];   ubyte _nTSg;
   SynDDef _syD [128];       ubyte _nSyD;
   ulong   _syM [128];       ubyte _nSyM;
private:
   bool    _onBt;
   WStr    _ctl [MAX_CTL];   ubyte _nCtl;
   CChDef  _cch [MAX_CCH];   uword _nCCh;
   SigDef  _sig [MAX_SIG];   uword _nSig;
   TStr     _dPath, _aPath;
   TrkEv   *_ev;   ulong _nEv;
};
