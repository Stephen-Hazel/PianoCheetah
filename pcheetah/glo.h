// glo.h - global vars sigh

#ifndef GLO_H
#define GLO_H

#include "../../stv/ui.h"
#include "../../stv/uiKey.h"
#include "../../stv/midi.h"

const ubyt4 LRN_BT = M_WHOLE/64;       // quicker wakeup when _lrn.vwNt
const ubyt4 NONE   = 0x7FFFFFFF;       // time of this means "no time"
                                       // .p of this means "no event/note"
const ubyt2 W_Q  = 24;                 // w of cue area (not incl chord w)
const ubyt2 W_NT = 14;                 // w of black/drum note  white=24  th=20
const ubyt2 H_NW = 24;                 // h of now line
const ubyt2 H_T  = H_NW+8;             // h of tmp cnv for bg behind now
                                       // plus half dots below it
const ubyt2 H_KB = 47;                 // h for keys (oct.bmp)

const ubyte MAX_DEVI = 8;              // max midiin devices to listen to
const ubyt4 MAX_RCRD = 32000;          // max events for recording in btw saves

// MAX_DEV used as max midiout devices for song
const ubyt2 MAX_CCH  = 512;            //     controllers to chase
const ubyt2 MAX_SIG  = 4096;           //     time,keysigs

const ubyte PRG_NONE = 255;            // prg value for "no program"

const ubyt2 SND_MAX  = 63*1024;        // max sounds per DevTyp
const ubyt2 SND_MAXD = 1024;           // max sound dirs per DevTyp
const ubyt4 SND_NONE = 0xFFFFFFFF;     // no sound


//______________________________________________________________________________
// midi devtype base stuph
struct CcRow  {WStr map;  ubyt2 raw;};
struct MapRow {WStr cci, cco;  TStr mod;};
struct SnRow  {TStr name;   ubyte prog, bank, bnkL;};


// midi IN stuph
struct MInDef {                        // midi in dev stuff
   MidiI          *mi;
   Arr<CcRow, 128> cc;
   Arr<MapRow,128> mp;
   static char *CcRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr);
   static char *MpRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr);
};


// midi OUT stuph
struct DevRow  {MidiO *mo;   ubyte dvt;};

class DevTyp {     // sounds and controls used per midiout device type
public:
         DevTyp ();
   void  Open (char *devTyp);
   void  Shut ();
   char *Name ()  {return _name;}

   ubyt2 CCID (char *name)             // str to dvt's raw out ubyt2
   {  for (ubyte c = 0; c < _cc.Ln; c++)  if (! StrCm (name, _cc [c].map))
                                                      return _cc [c].raw;
      return 0;
   }
   ubyt2 CCMap [128];                  // map song ctl id => raw ubyt2

   ubyt4  SndID  (char *name, bool newgrp = false, bool xmatch = false);
   SnRow *Snd    (ubyt4 id)  {if (id >= _sn.Ln) id = 0;   return & _sn [id];}
   bool   SndNew (ubyt4 *pnew, ubyt4 pos, char ofs);
   void   SGrp   (char *t);            // ...gui hooks
   void   SNam   (char *t, char *grp);
   void   Dump   ();

   TStr               _name;
   Arr<SnRow,SND_MAX> _sn;   ubyt4 _nDr;
   Arr<CcRow,128    > _cc;
private:
   static char *SnRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr);
   static char *CcRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr);
};


//______________________________________________________________________________
struct UTrkRow {
   ubyte dvt;   bool drm;   TStr lrn, ez, name, grp, snd, dev, notes, ctrls;
};
struct UpdLst {
   QPixmap *oct, *pnbg, *now, *dot, *fng, *cue, *bug, *lhmx, *fade, *tr;
   QIcon   *tbiPoz [2], *tbiLrn [3];
   QAction *tbbPoz,     *tbbLrn;
   ubyt2 txH;
   bool  uPoz;                         // user said poz, not just learn mode
   char  lrn;
   TStr  ttl, song, time, bars, tmpo, tsig, lyr, hey;
   ubyte lyrHiF, lyrHiL;
   Arr<DevTyp,MAX_DEV>  dvt;
   Arr<DevRow,MAX_DEV>  dev;
   Arr<UTrkRow,MAX_TRK> trk;           // n trk picked for editin
   ubyte rTrk, eTrk;                   // _f.trk.Ln-2 where rec drm,mel trks are
   ubyt2    w, h;                      // ctlNt's size
   QRect    tpos;                      // drawnow's area to update
   QPixmap *pm, *tpm;                  // main(all) and now(update) note pixmaps
   Canvas   cnv, tcnv;
};
extern UpdLst Up;                      // what gui needs from song

extern void DumpZ (char const *t, char *z);      // sigh


//______________________________________________________________________________
extern QColor CRng [128], CScl [12], CSclD [12], CTnt [4],
              CMid, CBBg, CBt;


//______________________________________________________________________________
struct CfgDef {
// global settings from etc/cfg.txt       (global prefs for all songs)
   ubyte cmdKey, ntCo;                 // cmdkey, pianonotes color
   bool  barCl;                        // send bar# to clipboard? (for lyr edit)
   bool  updt;                         // look for pcheetah updates?
   void  Init (), Load (), Save ();    // for global settings (not song)
   sbyte tran;
   bool  ezHop;
};
extern CfgDef Cfg;

struct FLstDef {
   Arr<TStr,65536> lst;                // last byte flags rand already picked
   const ubyte     X = sizeof (TStr)-1;
   ubyt4           pos;
   void Load (), Save ();
   bool DoFN  (char *fn);
   bool DoDir (char *dir, char *srch = nullptr);
};
extern FLstDef FL;


#endif  // GLO_H
