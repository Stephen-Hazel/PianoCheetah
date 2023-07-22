// UnSF2.cpp - convert .sf2 to preset.txt+.WAV files in new dir

#include "rc\resource.h"
#include "ui.h"
#include "MidiIO.h"

#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)

TStr   FN, To;                         // path to SF2 n out dir for SF2's wavs
bool   GM;
File   LF;                             // log file
sword *Smp;                            // big buf of SF2's mono 16 bit samples
ulong  SLn;                            // n total len


#pragma pack(push)
#pragma pack(1)

typedef struct {ubyte lo, hi;}                  SFRng;
typedef union  {SFRng rng; sword sw; uword uw;} SFAmt;

typedef struct {uword gen, mod;}                SFZon;
typedef struct {uword gen;  SFAmt amt;}         SFGen;
typedef struct {
   uword mod1, gen;
   sword amt;
   uword modA, tran;}                           SFMod;

struct {
   char  name [20];
   uword prog, bank;
   uword zon;
   ulong libr, genr, mrph;
}     Pset [  2*1024];  ulong  nPset;
SFZon PZon [  2*1024];  ulong  nPZon;
SFGen PGen [ 12*1024];  ulong  nPGen;
SFMod PMod [  4*1024];  ulong  nPMod;

struct {
   char  name [20];
   uword zon;
}     Inst [    1024];  ulong nInst;
SFZon IZon [  8*1024];  ulong nIZon;
SFGen IGen [128*1024];  ulong nIGen;
SFMod IMod [  4*1024];  ulong nIMod;

struct {
   char  name [20];
   ulong bgn, end, bgnLp, endLp;
   ulong frq;
   ubyte key;                          // key is midikey.  cnt is -99..99
   sbyte cnt;                          // lnk never used.  same w typ but
   uword lnk, typ;                     // DEFIned as 1=mono,2=right,4=left,
                                       // 8=linked,0x8000 set for ROM
                                       // (but can only ever be mono so ignore)
}     Samp [  8*1024];  ulong nSamp;

#pragma pack(pop)


char PsetNm  [BITS (Pset)][24];        // extra _02d suffix possible ;/
TStr InstNm  [BITS (Inst)];            // ...gets renamed SampNm ;/
char SampNm  [BITS (Samp)][24];


//------------------------------------------------------------------------------
char *FixFn (char *fn, char *in)
// don't want no dang spaces mostly, but keep the filenames clean...
{  MemCp (fn, in, 25);  fn [25] = '\0';
   for (char *p = fn;  *p;  p++)
      if ((*p <  ' ') || (*p >  '~') ||
          (*p == '*') || (*p == '?') || (*p == '/') || (*p == '\\') ||
          (*p == ':') || (*p == '"') || (*p == ' ') ||
          (*p == '>') || (*p == '<') || (*p == '|'))
         *p = '_';
   for (;  *fn && (fn [StrLn (fn)-1] == '_');)  StrAp (fn, "", 1);
   return fn;                          // ^ kill trailing _ s
}


//------------------------------------------------------------------------------
struct {uword id;  char *name, arg, sho;}  GenLst [] = {
   {43, "keyRng",      'r', 'y'}, // keyRange        (nonrt)
   {44, "velRng",      'r', 'y'}, // velRange        (nonrt)

// (inst ONLY for these 13)
   {0,  "sampBgn",     'u', 'y'}, // startAddrsOffset
   {1,  "sampEnd",     'u', 'y'}, // endAddrsOffset
   {2,  "smLpBgn",     'u', 'y'}, // startloopAddrsOffset
   {3,  "smLpEnd",     'u', 'y'}, // endloopAddrsOffset
   {4,  "sampBgnH",    'u', 'y'}, // startAddrsCoarseOffset
   {12, "sampEndH",    'u', 'y'}, // endAddrsCoarseOffset
   {45, "smLpBgnH",    'u', 'y'}, // startloopAddrsCoarseOffset
   {50, "smLpEndH",    'u', 'y'}, // endloopAddrsCoarseOffset
   {54, "sampMode",    's', 'y'}, // sampleModes       (nonrt)
                                  // 0=off,1=on,2=off,3=loop till keyup play end
   {58, "overRoot",    'k', 'y'}, // overridingRootKey (nonrt)
   {51, "oscStep",     's', 'y'}, // coarseTune
   {52, "oscCent",     's', 'y'}, // fineTune
   {17, "panPos",      's', 'y'}, // pan
   {56, "scale",       's', 'y'}, // scaleTuning       (nonrt)
   {46, "key",         'k', 'y'}, // keynum            (nonrt)
   {47, "vel",         's', 'y'}, // velocity          (nonrt)
/*
samp*,smLp* - SB,SE,LB,LE add/sub from samp.bgn/end/bgnLp/endLp
sampMode - LO 0=off,1/3=on
overRoot - KY replaces samp.key
oscStep  - TR add/sub to pitch by 100 cents
oscCent  - CT                     1
panPos   - PA -500=L, +500=R
scale    - PT 0 or 100, 0=no melodic pitch scaling
key      - FK replaces midi key
vel      - FV replaces midi velocity
*/

   {57, "exclInst",    's', 'n'}, // exclusiveClass    (nonrt)

   {23, "lfoODlay",    's', 'n'}, // delayVibLFO
   {24, "lfoOFreq",    's', 'n'}, // freqVibLFO
   {6,  "lfoO2Osc",    's', 'n'}, // vibLfoToPitch

   {21, "lfoMDlay",    's', 'n'}, // delayModLFO
   {22, "lfoMFreq",    's', 'n'}, // freqModLFO
   {5,  "lfoM2Osc",    's', 'n'}, // modLfoToPitch
   {10, "lfoM2Flt",    's', 'n'}, // modLfoToFilterFc
   {13, "lfoM2Amp",    's', 'n'}, // modLfoToVolume

   {39, "key2EnvAHold",'s', 'n'}, // keynumToVolEnvHold
   {40, "key2EnvADcay",'s', 'n'}, // keynumToVolEnvDecay
   {33, "envADlay",    's', 'n'}, // delayVolEnv
   {34, "envAAtak",    's', 'n'}, // attackVolEnv
   {35, "envAHold",    's', 'n'}, // holdVolEnv
   {36, "envADcay",    's', 'n'}, // decayVolEnv
   {37, "envASust",    's', 'n'}, // sustainVolEnv
   {38, "envARels",    's', 'n'}, // releaseVolEnv

   {31, "key2EnvMHold",'s', 'n'}, // keynumToModEnvHold
   {32, "key2EnvMDcay",'s', 'n'}, // keynumToModEnvDecay
   {25, "envMDlay",    's', 'n'}, // delayModEnv
   {26, "envMAtak",    's', 'n'}, // attackModEnv
   {27, "envMHold",    's', 'n'}, // holdModEnv
   {28, "envMDcay",    's', 'n'}, // decayModEnv
   {29, "envMSust",    's', 'n'}, // sustainModEnv
   {30, "envMRels",    's', 'n'}, // releaseModEnv
   {7,  "envM2Osc",    's', 'n'}, // modEnvToPitch
   {11, "envM2Flt",    's', 'n'}, // modEnvToFilterFc

   {8,  "fltCut",      'u', 'n'}, // initialFilterFc
   {9,  "fltRes",      'u', 'n'}, // initialFilterQ
   {48, "ampLvl",      's', 'n'}, // initialAttenuation
   {15, "fxChoir",     'u', 'n'}, // chorusEffectsSend
   {16, "fxRevrb",     'u', 'n'}, // reverbEffectsSend

   {14, "Undef14",     's', 'n'}, // undefined ones
   {18, "Undef18",     's', 'n'},
   {19, "Undef19",     's', 'n'},
   {20, "Undef20",     's', 'n'},
   {42, "Undef42",     's', 'n'},
   {49, "Undef49",     's', 'n'},
   {55, "Undef55",     's', 'n'},
   {59, "Undef59",     's', 'n'},

   {41, "instID",      'u', 'n'}, // instrument      (ID preset ONLY)
   {53, "sampID",      'u', 'n'}, // sampleID        (ID inst ONLY)
   {60, "endOpr",      'u', 'n'}  //                 (not really used?)
};


//------------------------------------------------------------------------------
char *Gen (uword g)
{ static TStr buf;
   for (int i = 0; i < BITS (GenLst); i++)
      if (GenLst [i].id == g)  return GenLst [i].name;
   return StrFmt (buf, "gen`d???", g);
}

char *Amt (uword g, SFAmt *a, bool dr)
{ static TStr buf;
  TStr b1, b2;
   for (int i = 0; i < BITS (GenLst); i++)  if (GenLst [i].id == g) {
      if      (GenLst [i].arg == 'r') {
         if (g != 43) {
            if (a->rng.lo == a->rng.hi)
                  StrFmt (buf, "`d",    a->rng.lo);
            else  StrFmt (buf, "`d-`d", a->rng.lo, a->rng.hi);
         }
         else {
            if (a->rng.lo == a->rng.hi)
                  StrFmt (buf, "`s",
                     dr ? MDrm2Str (b1, a->rng.lo) : MKey2Str (b1, a->rng.lo));
            else  StrFmt (buf, "`s-`s",
                     dr ? MDrm2Str (b1, a->rng.lo) : MKey2Str (b1, a->rng.lo),
                     dr ? MDrm2Str (b2, a->rng.hi) : MKey2Str (b2, a->rng.hi));

         }
      }
      else if (GenLst [i].arg == 'u')
            StrFmt (buf, "`d", a->uw);
      else if (GenLst [i].arg == 'k')
            StrFmt (buf, "`s", dr ? MDrm2Str (b1, (ubyte)(a->uw))
                                  : MKey2Str (b1, (ubyte)(a->uw)));
      else  StrFmt (buf, "`d", a->sw);
      return buf;
   }
   return StrFmt (buf, "`d=$`04x???", a->sw, a->sw);
}


//------------------------------------------------------------------------------
void Load ()
{ File  f;
  char  id [4];
  struct {char ckID [4];  ulong ckSize;}  chnk;
  sword vers [2];
  PStr  ts, zs;
  ulong p, i, s, t, u;
   if (! f.Open (FN, "r"))                      Die ("Couldn't read `s", FN);
   if (f.Get (& chnk, sizeof (chnk)) != sizeof (chnk))
                                                Die ("invalid SF2 a", FN);
   if (MemCm (chnk.ckID, "RIFF", 4, 'x'))       Die ("invalid SF2 b", FN);
   if (f.Get (id, sizeof (id)) != sizeof (id))  Die ("invalid SF2 c", FN);
   if (MemCm (id, "sfbk", 4, 'x'))              Die ("invalid SF2 d", FN);
   while (f.Get (& chnk, sizeof (chnk)) == sizeof (chnk)) {
      if      (MemCm (chnk.ckID, "LIST", 4, 'x') == 0)
        {if (f.Get (id, 4) != 4)                Die ("bad LIST chunk", FN);}
      else if ((MemCm (chnk.ckID, "ifil", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "iver", 4, 'x') == 0)) {
         if (f.Get (vers, 4) != 4)              Die ("invalid SF2 e", FN);
LF.Put (StrFmt (ts,
"`c`c`c: `d.`d\r\n", chnk.ckID[1],chnk.ckID[2],chnk.ckID[3], vers[0],vers[1]));
      }
      else if ((MemCm (chnk.ckID, "isng", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "INAM", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "irom", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "ICRD", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "IENG", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "IPRD", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "ICOP", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "ICMT", 4, 'x') == 0) ||
               (MemCm (chnk.ckID, "ISFT", 4, 'x') == 0)) {
         if (f.Get (zs, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 f", FN);
LF.Put (StrFmt (ts,
"`c`c`c: `s\r\n", chnk.ckID[1], chnk.ckID[2], chnk.ckID[3], zs));
      }

      else if (MemCm (chnk.ckID, "smpl", 4, 'x') == 0) {
         Smp = new sword [chnk.ckSize / 2 + 8];  // padding fer safety :/
         SLn = chnk.ckSize / 2;        // always 16 bit samples so #smp=bytes/2
         if (Smp == NULL)  Die ("SF2 too big :(", FN);
         if (f.Get (Smp, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 g", FN);
      }

      else if (MemCm (chnk.ckID, "phdr", 4, 'x') == 0) {
         nPset = chnk.ckSize / sizeof (Pset[0]);
         if (nPset > BITS (Pset))  Die ("too many presets", FN);
         if (f.Get (Pset, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 h", FN);
      }
      else if (MemCm (chnk.ckID, "pbag", 4, 'x') == 0) {
         nPZon = chnk.ckSize / sizeof (PZon[0]);
         if (nPZon > BITS (PZon))  Die ("too many preset bags", FN);
         if (f.Get (PZon, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 i", FN);
      }
      else if (MemCm (chnk.ckID, "pgen", 4, 'x') == 0) {
         nPGen = chnk.ckSize / sizeof (PGen[0]);
         if (nPGen > BITS (PGen))  Die ("too many preset gens", FN);
         if (f.Get (PGen, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 j", FN);
      }
      else if (MemCm (chnk.ckID, "pmod", 4, 'x') == 0) {
         nPMod = chnk.ckSize / sizeof (PMod[0]);
         if (nPMod > BITS (PMod))  Die ("too many preset mods", FN);
         if (f.Get (PMod, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 k", FN);
      }

      else if (MemCm (chnk.ckID, "inst", 4, 'x') == 0) {
         nInst = chnk.ckSize / sizeof (Inst[0]);
         if (nInst > BITS (Inst))  Die ("too many insts", FN);
         if (f.Get (Inst, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 l", FN);
      }
      else if (MemCm (chnk.ckID, "ibag", 4, 'x') == 0) {
         nIZon = chnk.ckSize / sizeof (IZon[0]);
         if (nIZon > BITS (IZon))  Die ("too many inst bags", FN);
         if (f.Get (IZon, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 m", FN);
      }
      else if (MemCm (chnk.ckID, "igen", 4, 'x') == 0) {
         nIGen = chnk.ckSize / sizeof (IGen[0]);
         if (nIGen > BITS (IGen))  Die ("too many inst gens", FN);
         if (f.Get (IGen, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 n", FN);
      }
      else if (MemCm (chnk.ckID, "imod", 4, 'x') == 0) {
         nIMod = chnk.ckSize / sizeof (IMod[0]);
         if (nIMod > BITS (IMod))  Die ("too many inst mods", FN);
         if (f.Get (IMod, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 o", FN);
      }

      else if (MemCm (chnk.ckID, "shdr", 4, 'x') == 0) {
         nSamp = chnk.ckSize / sizeof (Samp[0]);
         if (nSamp > BITS (Samp))  Die ("too many samples", FN);
         if (f.Get (Samp, chnk.ckSize) != chnk.ckSize)
            Die ("invalid SF2 p", FN);
      }

      else {
         StrFmt (ts, "skippin `c`c`c`c `d\r\n",
         chnk.ckID[0], chnk.ckID[1], chnk.ckID[2], chnk.ckID[3], chnk.ckSize);
         LF.Put (ts);
         f.Seek (EVEN_UP (chnk.ckSize), ".");  // skip unused Chunks
      }
   }
   f.Shut ();

// cleanup fn;  unDup  preset/inst/sample names
   if (nPset)  nPset--;                // always decr really cuz [n+1].zon
   if (nInst)  nInst--;                // (even samp w/out .zon)
   if (nSamp)  nSamp--;
   for (p = 0;  p < nPset;  p++)  FixFn (PsetNm [p], Pset [p].name);
   for (p = 0;  p < nPset;  p++)  for (u = 2, t = p+1;  t < nPset;  t++)
      if (! StrCm (PsetNm [p], PsetNm [t]))  StrAp (PsetNm [t],
                                                    StrFmt (ts, "_`02d", u++));
   for (i = 0;  i < nInst;  i++)  FixFn (InstNm [i], Inst [i].name);
   for (i = 0;  i < nInst;  i++)  for (u = 2, t = i+1; t < nInst; t++)
      if (! StrCm (InstNm [i], InstNm [t]))  StrAp (InstNm [t],
                                                    StrFmt (ts, "_`02d", u++));
   for (s = 0;  s < nSamp;  s++)  FixFn (SampNm [s], Samp [s].name);
LF.Put ("sample list:\r\n");
PStr ls, tk;
for (s = 0;  s < nSamp;  s++)
LF.Put (StrFmt (ls,
"`d=`s key=`s.`d frq=`d bgn=`d len=`d lpBgn=`d lpEnd=`d\r\n",
s, SampNm [s], MKey2Str (tk, Samp [s].key), Samp [s].cnt, Samp [s].frq,
Samp [s].bgn, Samp [s].end-Samp [s].bgn,
Samp [s].bgnLp-Samp [s].bgn, Samp [s].endLp-Samp [s].bgn));
   for (s = 0;  s < nSamp;  s++)  for (u = 2, t = s+1; t < nSamp; t++)
      if (! StrCm (SampNm [s], SampNm [t]))  StrAp (SampNm [t],
                                                    StrFmt (ts, "_`02d", u++));
}


void PutWav (char *fn, ulong fr, slong ky, slong ct,
             slong sb, slong se, slong lb, slong le)
{ sword *Ptr;
  ulong  Len, LpBgn, LpEnd, Frq;
  File   f;
  ulong  ln1, ln2, ln3, ln4;
  WAVEFORMATEX wf;
  struct {ulong manuf;  ulong prod;  ulong per;  ulong note;  ulong frac;
          ulong sfmt;   ulong sofs;  ulong num;  ulong dat;   ulong cue;
          ulong loop;   ulong bgn;   ulong end;  ulong frc;   ulong times;
  } smpl;
   Len = se-sb;   LpBgn = lb-sb;   LpEnd = le-sb;   Frq = fr;
   Ptr = & Smp [sb];

   ln4 = 60;
   ln3 = Len*2;  if (ln3 == 0)  return;
   ln2 = 16;
   ln1 = 12 + ln2 + 8 + ln3 + 8 + ln4;  // (LenR ? (8 + ln4) : 0);
   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = 1;
   wf.wBitsPerSample  = 16;
   wf.nSamplesPerSec  = Frq;
   wf.nBlockAlign     = wf.nChannels   * wf.wBitsPerSample / 8;
   wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
   if (f.Open (fn, "w")) {
      f.Put ("RIFF"    );  f.Put (& ln1, 4);
      f.Put ("WAVEfmt ");  f.Put (& ln2, 4);  f.Put (& wf,   ln2);
      f.Put ("data"    );  f.Put (& ln3, 4);  f.Put (Ptr,    ln3);
      MemSet (& smpl, 0, sizeof (smpl));
      smpl.per  = 1000000000 / Frq;
      smpl.note = ky;
      smpl.frac = ct;
      smpl.num  = 1;
      smpl.bgn  = LpBgn;
      smpl.end  = LpEnd;
      f.Put ("smpl");  f.Put (& ln4, 4);  f.Put (& smpl, ln4);
      f.Shut ();
   }
}


//------------------------------------------------------------------------------
const enum SF2Lvl    {INST_Z,   INST_G,   PRES_Z,   PRES_G,   NLVL};
char *LblL [NLVL] = {"izon", "iglb", "pzon", "pglb"};

const enum SF2Prm {KY, TR, CT,   KL, KH,   VL, VH,   SB, SE, LO, LB, LE,
                   PA, PT,   FK, FV,   NPRM};
char *LblP [NPRM] = {
   "root", "tran", "tune",   "key", "to",   "vel", "to",
   "bgn", "end", "loop", "lpBgn", "lpEnd",
   "pan", "pTrk", "fkey", "fvel"
};

char  Got [NLVL][NPRM];                // Set builds these from gens o p,i
slong SF2 [NLVL][NPRM];

#define OMAX  (32*1024)                // Put builds these
typedef struct {
   ulong sm, in;          // samp#,inst# list for preset
   char  got [NPRM];      // resolved Got
   slong sf2 [NPRM];      // resolved SF2
} ODef;
slong OLen;               // list size of preset's params,etc,etc
ODef  Out [OMAX];


//------------------------------------------------------------------------------
void Set (ubyte lvl, SFGen *g, bool dr)
// if it's a generator we care about, add to Got,SF2
{ int   p;
  bool  hi;
  SFAmt a;
  PStr  ls;
// if it's an unknown or non show=y, we're done
   for (p = 0;  p < BITS (GenLst);  p++)  if (GenLst [p].id == g->gen) {
      if (GenLst [p].sho != 'y')  return;   // don't care bout THAT gen
      break;                                // ok set dat
   }
   if (p >= BITS (GenLst))  return;         // won't happen...

LF.Put (StrFmt (ls, "     `s=`s\r\n",
Gen (g->gen), Amt (g->gen, & g->amt, dr)));

// map it to SF2P
   a = g->amt;   hi = false;
   switch (g->gen) {
      case 43: p = KL;                break;   // keyRng
      case 44: p = VL;                break;   // velRng
      case 0:  p = SB;                break;   // sampBgn
      case 1:  p = SE;                break;   // sampEnd
      case 2:  p = LB;                break;   // smLpBgn
      case 3:  p = LE;                break;   // smLpEnd
      case 4:  p = SB;   hi = true;   break;   // sampBgnH
      case 12: p = SE;   hi = true;   break;   // sampEndH
      case 45: p = LB;   hi = true;   break;   // smLpBgnH
      case 50: p = LE;   hi = true;   break;   // smLpEndH
      case 54: p = LO;                break;   // sampMode
      case 58: p = KY;                break;   // overRoot
      case 51: p = TR;                break;   // oscStep
      case 52: p = CT;                break;   // oscCent
      case 17: p = PA;                break;   // panPos
      case 56: p = PT;                break;   // scale
      case 46: p = FK;                break;   // key
      case 47: p = FV;                break;   // vel
      default: return;                 // won't happen
   }
   Got [lvl][p] = 'y';
   if      (p == KL) {SF2 [lvl][p]  = a.rng.lo;   Got [lvl][KH] = 'y';
                      SF2 [lvl][KH] = a.rng.hi;}
   else if (p == VL) {SF2 [lvl][p]  = a.rng.lo;   Got [lvl][VH] = 'y';
                      SF2 [lvl][VH] = a.rng.hi;}
   else               SF2 [lvl][p] += (hi ? (32768 * a.sw) : a.sw);
//LF.Put (StrFmt (ls, "DID `s`s\r\n",
//LblP [p], ((p == KL) || (p == VL)) ? ",to" : ""));
}


void DumpO ()
{ slong i;
  ubyte j;
  TStr  ts, ks;
  PStr  ls, os;
   for (i = 0;  i < OLen;  i++) {
      *ls = '\0';
      for (j = 0;  j < NPRM;  j++)  if (Out [i].got [j] != 'n') {
         if ((j == KL) || (j == KH) || (j == KY) || (j == FK))
               StrFmt (ts, " `s=`s", LblP [j],
                          MKey2Str (ks, (ubyte)Out [i].sf2 [j]));
         else  StrFmt (ts, " `s=`d", LblP [j], Out [i].sf2 [j]);
         StrAp (ls, ts);
      }
      LF.Put (StrFmt (os, "`d  samp=`s(`s.`d)  `s\r\n",
         i, SampNm [Out [i].sm], MKey2Str (ks, Samp [Out [i].sm].key),
         Samp [Out [i].sm].cnt,   ls));
   }
}


void Put (ulong in, ulong sm)
// resolve all the levels down to this samp's final params.
// store for later saving if non dup-yyy
{ ubyte i;
  slong j;
  bool  ins;
   if (Samp [sm].typ & 0x1000)  return;     // skip out if ROM sample
   if (OLen+1 >= BITS (Out))   return;      // no more room :(

//LF.Put (StrFmt (ls,
//"     Put inst=`d samp=`d\r\n", in, sm));

// resolve preset/inst global/zone levels down to final params
   for (j = 0;  j < NPRM;  j++) {
      for (i = 0;  i < 4;  i++)  if (Got [i][j] == 'y')  break;
      if (i < 4)  {Got [0][j] = '0'+i;   SF2 [0][j] = SF2 [i][j];}
   }

// default stuff ya ain't got that ya gotta have
   if (Got [0][KL] == 'n')  {Got [0][KL] = 'd';   SF2 [0][KL] = 0;}
   if (Got [0][KH] == 'n')  {Got [0][KH] = 'd';   SF2 [0][KH] = 127;}
   if (Got [0][VL] == 'n')  {Got [0][VL] = 'd';   SF2 [0][VL] = 1;}
   if (Got [0][VH] == 'n')  {Got [0][VH] = 'd';   SF2 [0][VH] = 127;}
   if (SF2 [0][VL] == 0)  SF2 [0][VL] = 1;  // 0 can't happen TECHnically
//LF.Put (            "    --- resolves to ---\r\n");
//for (j = 0; j < NPRM; j++) if (Got [0][j] != 'n')
//LF.Put (StrFmt (ls,
//"         `c `s=`d\r\n", Got [0][j], LblP [j], SF2 [0][j]));

// add to Out[] IF new  (diff sample/Got/SF2)
   ins = true;
   for (j = 0;  j < OLen;  j++) {
      if (sm != Out [j].sm)  continue;

      if (MemCm (Got [0], Out [j].got, sizeof (Got [0])))  continue;

      for (i = 0;  i < NPRM;  i++)
         if ((i != VL) && (i != VH) && (i != KL) && (i != KH))
            if (Got [0][i] != 'n')  if (SF2 [0][i] != Out [j].sf2 [i])  break;
      if (i < NPRM)  continue;

   // if got dup cept only for key/vel diff, upd existing range.
   // n always break out n don't ins
      if ((Got [0][KL] != 'n') && (SF2 [0][KL] < Out [j].sf2 [KL]))
                Out [j].sf2 [KL] = SF2 [0][KL];
      if ((Got [0][KH] != 'n') && (SF2 [0][KH] > Out [j].sf2 [KH]))
                Out [j].sf2 [KH] = SF2 [0][KH];
      if ((Got [0][VL] != 'n') && (SF2 [0][VL] < Out [j].sf2 [VL]))
                Out [j].sf2 [VL] = SF2 [0][VL];
      if ((Got [0][VH] != 'n') && (SF2 [0][VH] > Out [j].sf2 [VH]))
                Out [j].sf2 [VH] = SF2 [0][VH];
      ins = false;   break;
   }
   if (ins) {
      Out [OLen].sm = sm;   Out [OLen].in = in;
      MemCp (Out [OLen].got, Got, sizeof (Got [0]));
      MemCp (Out [OLen].sf2, SF2, sizeof (SF2 [0]));
      OLen++;
   }
   else  LF.Put ("         (dup)\r\n");
}


int OCmp (void *p1, void *p2)          // by ky,vl,sm
{ ODef *o1 = (ODef *)p1, *o2 = (ODef *)p2;
  int t;
   if (t = o1->sf2 [KL] - o2->sf2 [KL])  return t;
   if (t = o1->sf2 [VL] - o2->sf2 [VL])  return t;
   return  o1->sm - o2->sm;
}


void Save (ulong pr, bool dr)
{ PStr  ls;
  TStr  ts, ts2, sPath, pre, mid, suf, fn;
  slong o, i, k,  sm, sb, se, lb, le,  ky, ct;
  File  f;
   LF.Put (StrFmt (ls, "SAVE OLen=`d  pr=`d=`s(prog=`d,bank=`d) dr=`b\r\n",
                   OLen, pr, PsetNm [pr], Pset [pr].prog, Pset [pr].bank, dr));
   Sort (Out, OLen, sizeof (Out [0]), OCmp);
DumpO ();

// ok, do params
   for (o = 0;  o < OLen;  o++) {
   // pull Samp[] stuff (name later)
      sm = Out [o].sm;
      ky = Samp [sm].key;   sb = Samp [sm].bgn;   lb = Samp [sm].bgnLp;
      ct = Samp [sm].cnt;   se = Samp [sm].end;   le = Samp [sm].endLp;

   // PROCESSIN DOZE SF2 PARAMS
   // melo always got prefix of _kX_ fer KL,KH
      *pre = '\0';
      *suf = '\0';
      if (! dr)  StrFmt (pre, "_k`s_",
                 MKey2Str (ts, (ubyte)Out [o].sf2 [KH]));
   // got _vX_ based on VL,VH
      if ((Out [o].sf2 [VL] != 1) || (Out [o].sf2 [VH] != 127))
         StrAp (pre, StrFmt (ts, "`sv`03d_",
                                 *pre ? "" : "_", Out [o].sf2 [VH]));
   // suffix _L or _R based on pan ya got (only doin hard pannin here)
      if      (Out [o].sf2 [PA] <= -350)  StrCp (suf, "_L");
      else if (Out [o].sf2 [PA] >=  350)  StrCp (suf, "_R");

   // adjust wv key,cnt based off KY,TR,CT params
      if ((Out [o].got [KY] != 'n') && (Out [o].got [KY] != -1))
         ky = Out [o].sf2 [KY];

      if (dr && (Out [o].sf2 [KL] != Out [o].sf2 [KH])) {
         if      (Out [o].got [KY] != 'n')
               Out [o].sf2 [KL] = Out [o].sf2 [KH] = Out [o].sf2 [KY];
         else if (Out [o].sf2 [KL] > 0)
               Out [o].sf2 [KH] = Out [o].sf2 [KL];
         else  Out [o].sf2 [KL] = Out [o].sf2 [KH];
LF.Put (StrFmt (ls,
"        ERROR drum keyLo != keyHi.  pickin `s=`s\r\n",
MKey2Str (ts,  (ubyte)Out [o].sf2 [KL]),
MDrm2Str (ts2, (ubyte)Out [o].sf2 [KL])));
      }

      if (Out [o].got [TR] != 'n')  ky += Out [o].sf2 [TR];

      if (Out [o].got [CT] != 'n') {
         i = Out [o].sf2 [CT];
         if (i < -99)  {k = -i / 100;   ky -= k;   i += (k*100);}
         if (i >  99)  {k =  i / 100;   ky += k;   i -= (k*100);}
         if (i < 0)    {ky--;   i += 100;}
         ct = i;
      }

   // loop biz
      if (Out [o].got [SB] != 'n')  sb += Out [o].sf2 [SB];
      if (Out [o].got [SE] != 'n')  se += Out [o].sf2 [SE];
      if (Out [o].got [LB] != 'n')  lb += Out [o].sf2 [LB];
      if (Out [o].got [LE] != 'n')  le += Out [o].sf2 [LE];

   // no loop?  set loop points at end
   // (technically 0=off,1=on no tail,3=on+tail but meh)
      if (Out [o].got [LO] != 'n')
         {if ((Out [o].sf2 [LO] & 1) == 0)  lb = le = se;}

      *ts = '\0';
      if ((sb < 0)  || (sb >= (slong)SLn))      StrCp (ts, "bad sb");
      if ((se < sb) || (se >= (slong)SLn))      StrCp (ts, "bad se");
      if (lb < sb)  sb = lb;           // only SF2 can do that so reset sb :(
      if (lb > se)                              StrCp (ts, "bad lb");
      if ((le < sb) || (le > se) || (le < lb))  StrCp (ts, "bad le");
      if (*ts) {
LF.Put (StrFmt (ls,
"        ERROR `s sb=`d se=`d lb=`d le=`d bufLen=`d\r\n",
ts, sb, se, lb, le, SLn));
         return;
      }

   // build sPath
      if (dr) {
         MDrm2StG (ts, (ubyte)Out [o].sf2 [KL]);
         StrCp (ts, & ts [5]);            // toss Drum\ bit
         StrFmt (           sPath, "`s\\Drum\\`s\\`s",
                            To, PsetNm [pr], ts);
      }
      else {
         if (! GM)  StrFmt (sPath, "`s\\etc\\`s",
                            To, PsetNm [pr]);
         else       StrFmt (sPath, "`s\\`s_`s",
                            To, MProg [Pset [pr].prog & 0x007F],
                                PsetNm [pr]);
      }
      f.PathMake (sPath);

      StrCp (mid, SampNm [sm]);   i = StrLn (mid);

   // rip off an existing suffix and build wav fn
      if (i && (CHUP (mid [i-1]) == 'L'))    {i--;   StrAp (mid, "", 1);}
      if (i && (CHUP (mid [i-1]) == 'R'))    {i--;   StrAp (mid, "", 1);}
      while (i && StrCh (" -_", mid [i-1]))  {i--;   StrAp (mid, "", 1);}

//DBG("=>  sPath=`s `s `s `s ky=`d ct=`d sb=`d se=`d lb=`d le=`d",
//& sPath [StrLn (To)+1], pre, mid, suf, ky, ct, sb, se-sb, lb-sb, le-sb);

      StrFmt (fn, "`s\\`s`s`s.WAV", sPath, pre, mid, suf);
LF.Put (StrFmt (ls,
"        dir=`s\r\n"
"        wav=`s\r\n",
& sPath [StrLn (To)+1], & fn [StrLn (sPath)+1]));

   // write dat baby OUT
      PutWav (fn, Samp [sm].frq, ky, ct, sb, se, lb, le);
   }
}


//------------------------------------------------------------------------------
class ThrUnSF2: public Thread {
public:
   ThrUnSF2 (Waiter *w): _w (w), Thread (w->wnd)  {}
private:
   Waiter *_w;

   int End ()  {PostDadW (MSG_CLOSE, 0, 0);   return 0;}

   DWORD Go ()
   // go thru SF2 structs n parse out generators we care about.
   // call Put w each samp
   { ulong p, pz, pg, i, iz, ig, s, x;
     TStr  ts, tk;
     bool  dr, goti, gots;
     PStr  ls;
     StrArr d;
     MSG    msg;
TRC("{ ThrUnSF2::Go `s", _w->arg);
      InitMsgQ ();   msg.message = 0;
LF.Put (StrFmt (ls, "`s\r\n", FN));

   // load any drum.txt
      StrCp (ts, FN);   StrAp (ts, "", 4);  // toss .SF2 ext
      if (GM)  StrAp (ts, "", 3);           // toss _gm suffix
      StrAp (ts, "_drum.txt");
      d.Init ("drum", 128);
      d.Load (ts);
      if (d.NRow ()) {
LF.Put ("_drum.txt:\r\n");
         for (x = 0;  x < d.NRow ();  x++)
{LF.Put (d.str [x]);   LF.Put ("\r\n");}
      }

      Load ();                         // load that full .SF2
      FnName (ts, FN);                 // just fn.SF2 for gui
   // process each preset + used inst + used samp info
      for (p = 0;  p < nPset;  p++) {
         StrFmt (ls, "Converting `s preset `d of `d `s",
                 ts, p+1, nPset, PsetNm [p]);
         if (_w->Set ((ubyte)(100*p/nPset), ls))  return End ();

         MemSet (Got, 'n', sizeof (Got));
         MemSet (SF2, 0,   sizeof (SF2));
         dr = (Pset [p].bank & 0xFF80) ? true : false;
         for (x = 0;  x < d.NRow ();  x++)
            if (! StrCm (PsetNm [p], d.str [x]))  {dr = true;   break;}

LF.Put (StrFmt (ls,
"========================================\r\n"
"----- PSET=`s prog=`d bank=`d drum=`b\r\n",
PsetNm [p], Pset [p].prog, Pset [p].bank, dr));
         OLen = 0;

      // rifle thru this preset's zones to find any global zone (w no insts)
         for (pz = Pset [p].zon;  pz < Pset [p+1].zon;  pz++) {
            goti = false;
            for (pg = PZon [pz].gen;  pg < PZon [pz+1].gen;  pg++)
               if (PGen [pg].gen == 41)  {goti = true;   break;}
            if (! goti) {                 // ok global so load it up
LF.Put (StrFmt (ls,
" ----- pzone `d/`d GLOBAL\r\n",
pz-Pset [p].zon+1, Pset [p+1].zon-Pset [p].zon));
               MemSet (& Got [PRES_G], 'n', sizeof(Got [0]));
               MemSet (& SF2 [PRES_G], 0,   sizeof(SF2 [0]));
               for (pg = PZon [pz].gen;  pg < PZon [pz+1].gen;  pg++)
                  Set (PRES_G, & PGen [pg], dr);
            }
         }

      // now only the non-global preset zones
         for (pz = Pset [p].zon;  pz < Pset [p+1].zon;  pz++) {
            goti = false;
            for (pg = PZon [pz].gen;  pg < PZon [pz+1].gen;  pg++)
               if (PGen [pg].gen == 41)  {goti = true;   break;}
            if (  goti) {                 // ok non global so load it up
LF.Put (StrFmt (ls,
" ----- pzone `d/`d\r\n",
pz-Pset [p].zon+1, Pset [p+1].zon-Pset [p].zon));
               MemSet (& Got [PRES_Z], 'n', sizeof(Got [0]));
               MemSet (& SF2 [PRES_Z], 0,   sizeof(SF2 [0]));
               for (pg = PZon [pz].gen;  pg < PZon [pz+1].gen;  pg++) {
                  Set (PRES_Z, & PGen [pg], dr);

                  if (PGen [pg].gen == 41) {     // INST ID
                     i = PGen [pg].amt.uw;
LF.Put (StrFmt (ls,
"  ----- INST=`d=`s\r\n",
i, InstNm [i]));

                  // rifle thru this inst's zones to find any global zones
                     for (iz = Inst [i].zon;  iz < Inst [i+1].zon;  iz++) {
                        gots = false;
                        for (ig = IZon [iz].gen;  ig < IZon [iz+1].gen;  ig++)
                           if (IGen [ig].gen == 53)  {gots = true;   break;}
                        if (! gots) {     // ok global so load it up
LF.Put (StrFmt (ls,
"   ----- izone `d/`d GLOBAL\r\n",
iz-Inst [i].zon+1, Inst [i+1].zon-Inst [i].zon));
                           MemSet (& Got [INST_G], 'n', sizeof(Got [0]));
                           MemSet (& SF2 [INST_G], 0,   sizeof(SF2 [0]));
                           for (ig = IZon [iz].gen;
                                ig < IZon [iz+1].gen;  ig++)
                              Set (INST_G, & IGen [ig], dr);
                        }
                     }

                  // now only the non-global inst zones
                     for (iz = Inst [i].zon;  iz < Inst [i+1].zon;  iz++) {
                        gots = false;
                        for (ig = IZon [iz].gen;  ig < IZon [iz+1].gen;  ig++)
                           if (IGen [ig].gen == 53)  {gots = true;   break;}
                        if (  gots) {     // ok non global so load it up
LF.Put (StrFmt (ls,
"   ----- izone `d/`d\r\n",
iz-Inst [i].zon+1, Inst [i+1].zon-Inst [i].zon));
                           MemSet (& Got [INST_Z], 'n', sizeof(Got [0]));
                           MemSet (& SF2 [INST_Z], 0,   sizeof(SF2 [0]));
                           for (ig = IZon [iz].gen;
                                ig < IZon [iz+1].gen;  ig++) {
                              Set (INST_Z, & IGen [ig], dr);

                              if (IGen [ig].gen == 53) {   // SAMP ID
                                 s = IGen [ig].amt.uw;
LF.Put (StrFmt (ls,
"    ----- SAMP=`d=`s `s\r\n",
s, SampNm [s], MKey2Str (tk, Samp [s].key)));
                                 Put (i, s);
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         Save (p, dr);
      }
      if (Smp)  delete [] Smp;
TRC("} ThrUnSF2::Go");
      return End ();                   // ok, you can close, Pop
   }
};


class DlgUnSF2: public Dialog {
public:
   DlgUnSF2 (char *arg): Dialog (IDD_WAIT, IDI_APP), _t (NULL)
                {StrCp (_w.arg, arg);}
  ~DlgUnSF2 ()  {delete _t;}
private:
   ThrUnSF2 *_t;
   Waiter    _w;
   void Open ()  {_w.Init (Wndo ());   _t = new ThrUnSF2 (& _w);}
   void Done ()  {_w.Quit ();   _t->PostKid (WM_CLOSE, 0, 0);}
};


int Go ()
{ ulong ln;
  TStr  fn;
  char *pc;
DBG("{ UnSF2::Go");
// arg is path\fn.SF2   check .SF2 ext
   StrCp (FN, App.parm);   ln = StrLn (FN);      // check ext
   if ((ln < 4) || StrCm (& FN [ln-4], ".sf2"))
      {DBG ("not a .SF2 file", FN);   return 0;}

// take off path,ext so fn part => fn (sampleset)
   FnName (fn, FN);   StrAp (fn, "", 4);   ln = StrLn (fn);

// _gm suffix on fn means use GM dirs, etc
   GM = ((ln < 3) || StrCm (& fn [ln-3], "_gm")) ? false : true;
   if (GM)  StrAp (fn, "", 3);
   while (pc = StrCh (fn, ' '))  *pc = '_';      // spaces => _

// syn\sampleset (or _sampleset) => To
   App.Path (To, 'd');   StrAp (To, "\\device\\syn\\_");   StrAp (To, fn);

// already got it?  (or w leading _) musta already run so vamoose
   if (LF.PathGot (To))  {DBG ("already GOT `s", To);   return 0;}
   App.Path (To, 'd');   StrAp (To, "\\device\\syn\\");    StrAp (To, fn);
   if (LF.PathGot (To))  {DBG ("already GOT `s", To);   return 0;}

// ok, make To dir n open log.txt there
   LF.PathMake (To);   StrCp (fn, To);   StrAp (fn, "\\log.txt");
   if (! LF.Open (fn, "w"))  Die ("couldn't write `s?", fn);

// go !!!
   InitCom ();
  DlgUnSF2 dlg ("");
   dlg.Ok (NULL);

// cleanup
   LF.Shut ();
   QuitCom ();
DBG("} UnSF2::Go");
   return 0;
}
