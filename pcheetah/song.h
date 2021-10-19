// song.h - thread dealin w song data, midi, timer - da guts

#ifndef SONG_H
#define SONG_H

#include "glo.h"
#include "../../stv/timer.h"

#define HOVAL(v)  (((v) & 0x40) ? 1 : 0)

// _f.lrn modes:  a=hear all, b=hear Lrn, c=prac(learn), d=play(lead)
#define LHEAR  'a'
#define LHLRN  'b'
#define LPRAC  'c'
#define LPLAY  'd'
#define HEAR  (Up.lrn == 'a')
#define HLRN  (Up.lrn == 'b')
#define PRAC  (Up.lrn == 'c')
#define PLAY  (Up.lrn == 'd')

extern void   CInit ();
extern QColor CMap (ubyte n);

extern char  KeyCol [13];              // in sEdit.cpp
extern ubyte WXOfs  [12];


//______________________________________________________________________________
struct UCmdDef {const char *cmd, *nt, *ky, *grp, *desc;};
extern UCmdDef UCmd [];                // in sCmd.cpp
extern ubyte  NUCmd;


//______________________________________________________________________________
// note cache stuph
struct NtDef   {ubyte t, nt;   ubyt4 p;};   // evTrk, melo/drum nt, evPos

struct DownRow {ubyt4 time;  ubyte w, nNt;  NtDef nt [14]; // prac/play defs
                ubyt4 msec;  ubyt2 tmpo;  bool clip;};     // tmpo rec'd at
                                            // w used by TrkEZ,  any nt missed
struct TrkNt   {ubyt4 dn, up, tm, te;   ubyte nt;   bool ov;};

// drum map cache - use: into drumCon, outa drumExp (Load,Save) n temp in SetBnk
struct MapDRow {ubyte ctl, inp, vol, pan;   ubyt4 snd;   bool shh, lrn;
                                                         char ht;};

//______________________________________________________________________________
struct TrkRow  {ubyte  dev, chn,   din, drm,   vol, pan;
                bool   grp, shh, lrn;       // ^these 2 JUST for syn drum chans
                char   ht;
                TStr   name;
                ubyt4  snd;
                TrkEv *e;
                TrkNt *n;
                ubyt4 ne, nn, nb, p;};           // #e, #n, #broke, pos
struct CtlRow  {WStr s;   bool sho;};            // song's ctl map
struct CChRow  {ubyte dev, chn, ctl, trk, valu, val2;  ubyt4 time;};
struct CDoRow  {ubyte dev, chn, ctl, valu;};
struct TxtRow  {ubyt4 time, tend;  TStr s;};     // lyr,sct,chd,cue,bug
struct TpoRow  {ubyt4 time;  ubyt2 val;};                       // tempo (orig)
struct TSgRow  {ubyt4 time;  ubyte num, den, sub;  ubyt2 bar;}; // timesig
struct KSgRow  {ubyt4 time;  ubyte key, min, flt;};             // keysig


//______________________________________________________________________________
extern int EvCmp (void *p1, void *p2);      // ..._TrkEv[] sortin
//extern KSgRow CSig;

const ubyte TB_DSC = 0;                // my .song table format stuph
const ubyte TB_TRK = 1;                // Track:    dev snd trkName
const ubyte TB_DRM = 2;                // DrumMap:  note snd
const ubyte TB_LYR = 3;                // Lyric:    time name
const ubyte TB_EVT = 4;                // Event:    placeholder fake STable...:/
const ubyte TB_MAX = 5;                // number of em

class SongFile {                       // all the data from a .song file
public:
   bool  got;                          // got somethin loaded?
   TStr  fn;                           // filename

   char                  dsc [65536];
   Arr<TrkRow, MAX_TRK>  trk;
   Arr<MapDRow,MAX_DRM>  mapD;
   Arr<TxtRow, MAX_LYR>  lyr, sct, chd, cue, bug;
   Arr<CtlRow, 128    >  ctl;
   TrkEv *ev;
   ubyt4 nEv, maxEv;

// song settings from _dsc section.  DscSave()/DscLoad() preserve/restore these
   ubyt4 tmpo;                         // tempo per-thousand-age
   sbyte tran;                         // transpose halfsteps
   bool  ezHop;                        // even in ez mode, do hop forward,etc

// special ctls
   Arr<TpoRow,MAX_SIG>  tpo;
   Arr<TSgRow,MAX_SIG>  tSg;
   Arr<KSgRow,MAX_SIG>  kSg;
};


//______________________________________________________________________________
struct RecDef  {ubyt4 tm, ms;};        // map o all rec evs

struct LrnDef {
   bool  vwNt, ez, rHop;               // any TSho() tracks;  ez mode;  sync2rec
   char  pLrn, hand;                   // prev lrn mode;  hand: \0=none/L/R/B
   bool  hLrn,                         // hear ?ez instead of rec?  set in HopTo
         dWip;                         // wipe on ntDn?
   ubyte hVal;                         //    if so, hold val pre ntDn
   ubyt4 lpBgn, lpEnd;                 // current loop's bgn,end times
   bool  POZ,                          // paused?
         chd;                          // w of cue area
   ubyte veloSng, veloRec, velo [7];   // velo scale for ? vs rec; n ez tracks
   ubyt2 toRec [2][256];               // buffer vals to rcrd per ctl post poz
   RecDef  rec [2][128];               // notes cur down from EvRcrd
   ubyte    nt    [128];               // track nondrum non? held notes
};


//______________________________________________________________________________
// notation stuph
struct BlkDef  {ubyt2 bar, y, h;   ubyt4 tMn, tMx;   ubyte sb;};
                                                 // bar's tm block, y scale
struct SymDef  {ubyte tr;   ubyt4 nt;
                bool top, bot;   ubyt2 x, y, w, h;};  // note's symbols
struct ColDef  {BlkDef *blk;   SymDef *sym;   ubyt4 nBlk,  nSym;
                ubyte nMn, nMx, nDrm, dMap [128];   ubyt2 x, w, h;};
struct PagDef  {ColDef *col;   ubyt4 nCol;   ubyt2 w, h;};

struct LHMxRow {ubyt4 tm;  ubyte nt;};      // for LH shading


//______________________________________________________________________________
class Song: public QObject {
   Q_OBJECT

public:
   Song ()                             // prep for 1st Wipe()
   {  _eOn = false;   _f.ev = nullptr;   _nt = nullptr;   }

private:
// sTime.cpp
   TSgRow *TSig  (ubyt4 tm);
   KSgRow *KSig  (ubyt4 tm);
   char *TmStr   (char *str, ubyt4 tm, ubyt4 *tL8r = NULL, ubyte *subt = NULL);
   char *TmSt    (char *str, ubyt4 tm);
   ubyt2 Tm2Bar  (ubyt4 tm);
   bool  Poz     (bool tf, ubyt4 msx = 0);
   ubyt4 Bar2Tm  (ubyt2 b, ubyte bt = 1);
   ubyt4 Str2Tm  (char *ts);
   void  SetPDn  (ubyt4 p);
   void  TmHop   (ubyt4 tm);
   ubyte CCValAt (ubyt4 tm, ubyte trk, char *cc);
   ubyt2 TmpoAct (ubyt2 val);
   ubyt2 TmpoSto (ubyt2 act);
   ubyt2 TmpoAt  (ubyt4 tm, char act = '\0');    // return stored unless 'a'

// sDevice.cpp  (n sDevTyp.cpp)
   void OpenMIn (), ShutMIn ();

   char *DevName (ubyte t)
   {  if (!  Up.dev [_f.trk [t].dev].mo)  return CC("");
      return Up.dev [_f.trk [t].dev].mo->Name ();
   }
   char *SndName (ubyte t)
   {  if (_f.trk [t].snd == SND_NONE)  return CC("");
      return Up.dvt [Up.dev [_f.trk [t].dev].dvt].Snd (_f.trk [t].snd)->name;
   }
   void  NotesOff ();                  // notes+hold OFF
   void  PickDev (ubyte tr, char *sndName, char *devName = NULL);
   ubyte OpenDev (char *nm);
   void  ShutDev (ubyte d);
   void  SetBnk ();
   void  SetChn (), SetChn (ubyte t);

// sFile.cpp
   bool  DscGet  (char *key, char *val);
   void  DscPut  (char *repl);
   void  Pract   ();
   void  DscInit (), DscLoad (), DscSave ();
   ubyte DrumCon ();                   // outputs _mapD
   void  DrumExp (bool setBnk = true);    // usin _mapD
   void  CCClean ();
   void  Load    (char *fn);
   void  Save    (bool prac = false);  // true for prac;  false/omit for rec

// sDump.cpp
   void DumpEv   (TrkEv *e, ubyte t, ubyt4 p = 1000000, char *pre = NULL);
   void DumpTrEv (ubyte t);
   void DumpRec  ();
   void Dump     (bool ev2 = false);

// sRecord.cpp
   void  Shush   (bool tf);            // flip by volume cc (only) on/off
   char  DnOK    (char n = '\0');
   void  CCMap   (char *cSt, char *cMod, ubyte dev, MidiEv *ev);
   bool  CCEd    (char *cSt, char *cMod, ubyte dev, MidiEv *ev);
   void  CCInit  (ubyte tr, char *cc, ubyte val);
   void  NtGet   (MidiEv *ev);
   void  SetMSec (ubyt4 p, MidiEv *ev);
   void  RecDvCh (MidiEv *ev, ubyte *d, ubyte *c, ubyte *dL, ubyte *cL);
   void  RecDvCh (ubyte ti,
                  TrkEv *ev, ubyte *d, ubyte *c, ubyte *dL, ubyte *cL);
   void  PozBuf  (MidiEv *ev, char *cSt);
   void  PozIns  ();
   void  Record  (MidiEv *ev);
   void  EvRcrd  (ubyte dev, MidiEv *ev);

// sChrd.cpp
   void  DoChrd (char *arg);
   void  PopChd (ubyt4 tm);
   void  ChdArr (ubyt4 tm);

// sEdit.cpp
   ubyt4 GetDn   (TrkEv *e, ubyt4 p, ubyte nt);
   ubyt4 GetUp   (TrkEv *e, ubyt4 p, ubyt4 ne, ubyte nt);
   ubyt2 Nt2X    (ubyte n, ColDef *co, char gr = '\0');
   ubyt2 Dr2X    (ubyte d, ColDef *co);
   ubyt2 CtlX    (ColDef *co);
   ubyt2 Tm2Y    (ubyt4 t, ColDef *co, BlkDef **bl = NULL);
   ubyt4 Y2Tm    (ubyt2 y, ColDef *co);
   ubyt4 SilPrv  (ubyt4 tm), SilNxt  (ubyt4 tm);
   ubyt4 NtDnPrv (ubyt4 tm), NtDnNxt (ubyt4 tm);
   void  SetLp   (char dir);
   void  ReCtl   ();                   // map all DevTyps w song's _ctl[]s
   void  CtlClean();                   // redo _ctl[] to just used ones sorted
   ubyt4 ReEv    ();                   // redo _tpo,_tSg,_kSg,_tEnd,etccccc
   ubyte CCUpd   (char *cSt, ubyte t); // get _ctl pos; upd _cch,_ctl,dvt.CCMap
   void  EvDel   (ubyte t, ubyt4 p, ubyt4 ne = 1);
   bool  EvIns   (ubyte t, ubyt4 p, ubyt4 ne = 1);
   void  EvInsT  (ubyte t, TrkEv  *ev);
   void  EvInsT  (ubyte t, MidiEv *ev);
   void  NtIns   (ubyte t, ubyt4 tm, ubyt4 te, ubyte c, ubyte v = 100);
   ubyt4 NtHit   (ubyte t, ubyt4 tmn, ubyt4 tmx, ubyte nmn, ubyte nmx);
   void  NtUpd   (char *tnf);
   void  NtHop   (char *arg);
   void  NtDur   (char *arg);
   void  RcMov   (char *arg);
   void  Split   ();
   void  setCtl  (char *arg);
   bool  TxtIns  (ubyt4 tm, char *s, Arr<TxtRow,MAX_LYR> *at, char cue = '\0');
   void  TrkSnd  (ubyt4 snew);
   void  NewSnd  (char ofs);
   void  NewGrp  (char *grp),  NewSnd  (char *snd);
   void  NewDev  (char *dev);
   void  TrkSplt ();
   void  TrkDel  (ubyte t);
   ubyte TrkIns  (ubyte t = MAX_TRK, char *name = NULL, char *snd = NULL);
   void  DrMap   (char *d);
   void  TrkEd   (char *op);
   ubyte GetSct  (TxtRow *sct);        // pull sections outa _f.lyr[] - sct[64]!
   void  TrkDr   (char *d);
   void  TmpoPik (char o_r);           // stamp cur tmpo w orig or rec'd vals

// sCmd.cpp
   ubyte ChkETrk ();
   void  Msg (char *s), LoopInit (), RecWipeQ (),
         EdSong (char ofs), EdTrak (char ofs),
         EdTime (char ofs), EdTmpo (char ofs),
         EdRec  (char ofs), EdLrn  (char ofs),
         HType  (char *s),  Mix    (char *s);

// sReDo.cpp
   bool  TSho    (ubyte t);
   bool  TLrn    (ubyte t);
   bool  TEz     (ubyte t);
   bool  TDrm    (ubyte t);
   void  ReTrk   ();                   // give gui _trk info ta draw
   void  BarH    (ubyt2 *h, ubyte *sb, ubyt2 b);
   void  SetDn   (char q = '\0'),      // default to no quantize
         SetNt   (),
         SetSym  ();
   void  ReDo    ();

// sNote.cpp
   ubyte DrawRec (bool all, ubyt4 pp);
   void  DrawFng (ubyt2 x, ubyt2 , ubyte f, char tc = '\0');
   void  DrawSym (SymDef *s, ColDef *co);
   void  DrawPg  (ubyt4 pp);
   void  DrawNow ();
   void  Draw    ();                   // da top o notation drawin

// song.cpp
   void  PutTp   (ubyt2 tp);
   void  PutTs   (ubyte n, ubyte d, ubyte sb);
   void  PutLy   ();
   void  PutCC   (ubyte t, TrkEv *e);
   void  PutNt   (ubyte t, TrkEv *e, bool bg = false);

   void  Init (), Quit ();
   void  Wipe ();
   void  Hey  (char *msg);
   void  Die  (char *msg);

   Timer               *_timer;
   Arr<MInDef,MAX_DEVI> _mi;
   SongFile             _f;

   bool   _onBt, _rcrd, _prac;         // tL8r is still on beat?  recording?
   ubyt4  _tEnd;                       // last time       DID some practice evs?
   ubyt2  _bEnd;                       // last bar
// _now is curr time if poz'd, else next time Put happens (usually in future)
// _pNow to _rNow is timeframe for DrawNowP to update
//    Put, EvRcrd update _rNow;  _pNow follows after  (tSoon/Late window pDn tm)
   ubyt4  _now, _pNow, _rNow, _tSoon, _tLate;
   LrnDef _lrn;

   bool            _eOn;               // piano key editin'
   Arr<CDoRow,128> _cDo;

   ubyt4 _pLyr, _hLyr, _pChd;

   Arr<CChRow,MAX_CCH>  _cch;          // ctl chasing
   TrkNt                *_nt;          // size is always same as _f.nEv
   Arr<DownRow,128*1024> _dn;          // prac/play defs
   ubyt4                _pDn;

   ubyt4  _pg;   ubyte _tr;   QRect _rc;    // ...notation junk
   Arr<PagDef,200   >  _pag;
   Arr<ColDef,500   >  _col;
   Arr<BlkDef,9999  >  _blk;
   Arr<SymDef,131584>  _sym;
   Arr<LHMxRow, 16*1024> _lm;          // for LH shading
// DWORD _syn, _lzr;                   // other .EXEs (IPC)

public slots:
   void Cmd (QString s);               // gui listenin
   void ReSz ()                        // note widget resize/repixmap
   {  TRC("Song::ReSz");   _pg = _tr = 0;   SetSym ();   Draw ();  }

   void Put ();                        // pcheetah's heartbeat
   void MIn ();                        // top o da recordin' biz

signals:
   void sgCmd (QString cmd);
   void sgUpd (QString upd);           // gui talkin
};


#endif // SONG_H
