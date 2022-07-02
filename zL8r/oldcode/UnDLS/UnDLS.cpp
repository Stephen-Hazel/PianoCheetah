// dlsSplit.cpp - split .dls to .pset+.WAV files in the DLS dirs

#include "ui.h"
#include "MidiIO.h"
#include <dls1.h>
#include <dls2.h>

#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)

typedef struct {char id [4];  ulong sz;} ChDef;

typedef struct {
   char       name [21];
   ulong      prog;
   INSTHEADER Pset;
} PsetDef;

typedef struct {
   ulong     Pset;
   RGNHEADER inst;
   WAVELINK  link;
   bool      smplGot;
   WSMPL     smpl;
   WLOOP     loop;
} InstDef;

typedef struct {
   ulong      Pset;
   ulong      Inst;
   CONNECTION artc;
} ArtcDef;

typedef struct {
   char         name [21];
   WAVEFORMATEX fmt;
   ulong        pos, len;
   bool         smplGot;
   WSMPL        smpl;
   WLOOP        loop;
} WaveDef;


ubyte B [16*1024*1024];
ulong L;

ulong NPset;   PsetDef Pset [1000];
ulong NInst;   InstDef Inst [10000];
ulong NArtc;   ArtcDef Artc [10000];
ulong NWave;   WaveDef Wave [1000];

struct {uword id;  char *sym;} Src [] = {
   {CONN_SRC_NONE,            ""},
   {CONN_SRC_KEYNUMBER,       "key"},
   {CONN_SRC_KEYONVELOCITY,   "vel"},
   {CONN_SRC_POLYPRESSURE,    "nprs"},
   {CONN_SRC_CHANNELPRESSURE, "prss"},
   {CONN_SRC_MONOPRESSURE,    "mprs"},
   {CONN_SRC_PITCHWHEEL,      "pbnd"},
   {CONN_SRC_CC1,             "mod"},
   {CONN_SRC_CC7,             "vol"},
   {CONN_SRC_CC11,            "expr"},
   {CONN_SRC_CC10,            "pan"},
   {CONN_SRC_CC91,            "reverb"},
   {CONN_SRC_CC93,            "chorus"},
   {CONN_SRC_EG1,             "env1"},
   {CONN_SRC_EG2,             "env2"},
   {CONN_SRC_LFO,             "lfo1"},
   {CONN_SRC_VIBRATO,         "lfov"}
// how bout RPN0,RPN1,RPN2???
};

struct {uword id;  char *sym;} Dst [] = {
   {CONN_DST_NONE,             ""},
   {CONN_DST_KEYNUMBER,        "key"},
   {CONN_DST_PITCH,            "oscCent"},
   {CONN_DST_ATTENUATION,      "ampLvl"},
   {CONN_DST_FILTER_CUTOFF,    "fltCut"},
   {CONN_DST_FILTER_Q,         "fltRes"},
   {CONN_DST_PAN,              "panPos"},
   {CONN_DST_REVERB,           "reverb"},
   {CONN_DST_CHORUS,           "chorus"},
   {CONN_DST_LEFT,             "chnL"},
   {CONN_DST_RIGHT,            "chnR"},
   {CONN_DST_CENTER,           "chnC"},
   {CONN_DST_LEFTREAR,         "chnLR"},
   {CONN_DST_RIGHTREAR,        "chnRR"},
   {CONN_DST_LFE_CHANNEL,      "chnLFE"},
   {CONN_DST_LFO_STARTDELAY,   "lfo1Dlay"},
   {CONN_DST_LFO_FREQUENCY,    "lfo1Freq"},
   {CONN_DST_VIB_STARTDELAY,   "lfovDlay"},
   {CONN_DST_VIB_FREQUENCY,    "lfovFreq"},
   {CONN_DST_EG1_DELAYTIME,    "env1Dlay"},
   {CONN_DST_EG1_ATTACKTIME,   "env1Atak"},
   {CONN_DST_EG1_HOLDTIME,     "env1Hold"},
   {CONN_DST_EG1_DECAYTIME,    "env1Dcay"},
   {CONN_DST_EG1_SUSTAINLEVEL, "env1Sust"},
   {CONN_DST_EG1_RELEASETIME,  "env1Rels"},
   {CONN_DST_EG1_SHUTDOWNTIME, "env1Off"},
   {CONN_DST_EG2_DELAYTIME,    "env2Dlay"},
   {CONN_DST_EG2_ATTACKTIME,   "env2Atak"},
   {CONN_DST_EG2_HOLDTIME,     "env2Hold"},
   {CONN_DST_EG2_DECAYTIME,    "env2Dcay"},
   {CONN_DST_EG2_SUSTAINLEVEL, "env2Sust"},
   {CONN_DST_EG2_RELEASETIME,  "env2Rels"}
};

struct {uword id;  char *sym;} Trn [] = {
   {CONN_TRN_NONE,    ""},
   {CONN_TRN_CONCAVE, "[conc]"},
   {CONN_TRN_CONVEX,  "[conv]"},
   {CONN_TRN_SWITCH,  "[swch]"}
};


ulong  Len, LpBgn, LpEnd, Frq;
sword *Ptr;
sword  Smp [64*1024*1024];


//------------------------------------------------------------------------------
TStr DLSPath;
char PsetNm  [BITS (Pset)][24];
char WaveNm  [BITS (Wave)][24];

char *GMDir [16] = {
   "Piano", "ChromPerc", "Organ",    "Guitar",
   "Bass",  "SoloStr",   "Ensemble", "Brass",
   "Reed",  "Pipe",      "SynLead",  "SynPad",
   "SynFX", "Ethnic",    "Perc",     "SndFX"
};


//------------------------------------------------------------------------------
char *FixFn (char *fn, char *in)
{  memcpy (fn, in, 20);  fn [20] = '\0';
   for (char *p = fn; *p; p++)
      if ((*p <  ' ') || (*p >  '~') ||
          (*p == '*') || (*p == '?') || (*p == '/') || (*p == '\\') ||
          (*p == ':') || (*p == '"') || (*p == ' ') ||
          (*p == '>') || (*p == '<') || (*p == '|'))
         *p = '_';
   for (; *fn && (fn [StrLn (fn)-1] == '_');)  fn [StrLn (fn)-1] = '\0';
   if (*fn == '\0')  StrCp (fn, "x");
   return fn;
}


//------------------------------------------------------------------------------
void Load (File fo)
{ ulong p, i, w, u, t; // byte pos in B[L]
  ChDef ch;
  char  spot [80];  // inst, inst/regn, inst/artc, inst/regn/artc, wave
  ulong endp [80];
  char  id [5], zstr [1000], out [8000];
   p = 0;   StrCp (spot, "");

// check header for RIFF, DLS
   if (MemCm ((char *)(& B [0]), "RIFF", 4) ||
       MemCm ((char *)(& B [8]), "DLS ", 4))
      Die ("not a RIFF-DLS file");

// start parsin chunks
   for (p = 12;  (p + 8) <= L;) {
   // get chunk
      memcpy (& ch, & B [p], 8);    p += 8;
//sprintf(out, "%c%c%c%c %d NInst=%d\n",
//ch.id[0],ch.id[1],ch.id[2],ch.id[3],ch.sz, NInst);   DBG(out);

   // see if LIST outa scope
      while (StrLn (spot) && (p >= endp [StrLn (spot) - 1]))
         spot [StrLn (spot) - 1] = '\0';
      if (MemCm (ch.id, "LIST", 4) == 0) {
      // LISTs...
         if (p+4 > L)
            Die ("bad LIST chunk");
         memcpy (id, & B [p], 4);  p += 4;  id [4] = '\0';
      // ignore
         if    ( (StrCm (id, "wvpl") == 0) ||
                 (StrCm (id, "lins") == 0) ||
                 (StrCm (id, "lrgn") == 0) ||
                 (StrCm (id, "INFO") == 0) )
            *id = '\0';
      // spot
         else if ( StrCm (id, "ins ") == 0) {
            *id = 'i';
            if (++NPset == BITS (Pset))  Die ("too many Psets");
         }
         else if ((StrCm (id, "rgn ") == 0) || (StrCm (id, "rgn2") == 0)) {
            *id = 'r';
            if (++NInst == BITS (Inst))  Die ("too many Insts");
            Inst [NInst-1].Pset = NPset-1;
         }
         else if ((StrCm (id, "lart") == 0) || (StrCm (id, "lar2") == 0)) {
            *id = 'a';
         }
         else if ( StrCm (id, "wave") == 0) {
            *id = 'w';
            if (++NWave == BITS (Wave))  Die ("too many Waves");
         }
      // huh?
         else {
            sprintf (out, "%s LIST %s %d???\r\n", spot, id, ch.sz);
            fo.Put(out);
            *id = '\0';
         }
         if (*id) {
            endp [StrLn (spot)] = p + ch.sz;
            id [1] = '\0';   StrAp (spot, id);
         }
      }
      else {
      // chunk...
         if ((p + EVEN_UP (ch.sz)) > L)
            Die ("bad chunk size");
      // ignore
         if      ( (MemCm (ch.id, "colh", 4) == 0) ||
                   (MemCm (ch.id, "ptbl", 4) == 0) ||
                   (MemCm (ch.id, "smpl", 4) == 0) ||
                   (MemCm (ch.id, "guid", 4) == 0) ||
                   (MemCm (ch.id, "dlid", 4) == 0) )  ;
      // zstr in various spots
         else if ( (MemCm (ch.id, "IARL", 4) == 0) ||
                   (MemCm (ch.id, "IART", 4) == 0) ||
                   (MemCm (ch.id, "ICMS", 4) == 0) ||
                   (MemCm (ch.id, "ICMT", 4) == 0) ||
                   (MemCm (ch.id, "ICOP", 4) == 0) ||
                   (MemCm (ch.id, "ICRD", 4) == 0) ||
                   (MemCm (ch.id, "IENG", 4) == 0) ||
                   (MemCm (ch.id, "IGNR", 4) == 0) ||
                   (MemCm (ch.id, "IKEY", 4) == 0) ||
                   (MemCm (ch.id, "IMED", 4) == 0) ||
                   (MemCm (ch.id, "INAM", 4) == 0) ||
                   (MemCm (ch.id, "IPRD", 4) == 0) ||
                   (MemCm (ch.id, "ISBJ", 4) == 0) ||
                   (MemCm (ch.id, "ISFT", 4) == 0) ||
                   (MemCm (ch.id, "ISRC", 4) == 0) ||
                   (MemCm (ch.id, "ISRF", 4) == 0) ||
                   (MemCm (ch.id, "ITCH", 4) == 0) ||
                   (MemCm (ch.id, "DATE", 4) == 0) ) {
            if (ch.sz >= sizeof (zstr))
               Die ("zstr chunk TOO BIG");
            memcpy (zstr, & B [p], ch.sz);
            zstr [ch.sz] = '\0';  // just in case
            if      ((! MemCm (ch.id, "INAM", 4)) && (*spot == 'w'))
                 {FixFn (Wave [NWave-1].name, zstr);}
            else if ((! MemCm (ch.id, "INAM", 4)) && (*spot == 'i'))
                 {FixFn (Pset [NPset-1].name, zstr);}
            else {
               sprintf (out, "%s %c%c%c%c: %s\r\n",
                        spot, ch.id[0],ch.id[1],ch.id[2],ch.id[3], zstr);
               fo.Put (out);
            }
         }

         else if (MemCm (ch.id, "vers", 4) == 0) {
           DLSVERSION v;
            memcpy (& v, & B [p], sizeof (v));
            sprintf (out, "%s vers %d,%d,%d,%d\r\n", spot,
                     HIWORD(v.dwVersionMS), LOWORD(v.dwVersionMS),
                     HIWORD(v.dwVersionLS), LOWORD(v.dwVersionLS));
            fo.Put (out);
         }

      // Wave[].fmt
         else if (MemCm (ch.id, "fmt ", 4) == 0)
            {memcpy (& Wave [NWave-1].fmt, & B [p], sizeof (WAVEFORMATEX));}
      // Wave[].data
         else if (MemCm (ch.id, "data", 4) == 0)
            {Wave [NWave-1].pos = p;  Wave [NWave-1].len = ch.sz;}
      // {Wave|Inst}[].smpl+.loop
         else if (MemCm (ch.id, "wsmp", 4) == 0) {
            if (StrCm (spot, "w") == 0) {
               Wave [NWave-1].smplGot = true;
               memcpy (& Wave [NWave-1].smpl, & B [p],
                       sizeof (WSMPL) + sizeof (WLOOP));
               if (Wave [NWave-1].smpl.cSampleLoops == 0)
                  memset (& Wave [NWave-1].loop, 0, sizeof (WLOOP));
            }
            else {
               Inst [NInst-1].smplGot = true;
               memcpy (& Inst [NInst-1].smpl, & B [p],
                       sizeof (WSMPL) + sizeof (WLOOP));
               if (Inst [NInst-1].smpl.cSampleLoops == 0)
                  memset (& Inst [NInst-1].loop, 0, sizeof (WLOOP));
            }
         }

      // Pset[].Pset+.prog
         else if (MemCm (ch.id, "insh", 4) == 0) {
            memcpy (& Pset [NPset-1].Pset, & B [p], sizeof (INSTHEADER));
            Pset [NPset-1].prog =
                (Pset [NPset-1].Pset.Locale.ulBank & F_INSTRUMENT_DRUMS) |
               ((Pset [NPset-1].Pset.Locale.ulBank & 0x0000FFFF) << 8)  |
                (Pset [NPset-1].Pset.Locale.ulInstrument & 0x00FF);
         }

      // Inst[]
         else if (MemCm (ch.id, "rgnh", 4) == 0)
            {memcpy (& Inst [NInst-1].inst, & B [p], sizeof (RGNHEADER));}
         else if (MemCm (ch.id, "wlnk", 4) == 0)
            {memcpy (& Inst [NInst-1].link, & B [p], sizeof (WAVELINK));}

      // Artc[]
         else if ((MemCm (ch.id, "art1", 4) == 0) ||
                  (MemCm (ch.id, "art2", 4) == 0)) {
           CONNECTIONLIST *cl = (CONNECTIONLIST *)(& B [p]);
            i = cl->cConnections;
            if (NArtc + i > BITS (Artc))
               Die ("too many Artcs");
            for (i = 0; i < cl->cConnections; i++) {
               memcpy (& Artc [NArtc+i].artc,
                       & B [p + sizeof (CONNECTIONLIST) +
                            i * sizeof (CONNECTION)], sizeof (CONNECTION));
               Artc [NArtc+i].Pset = NPset-1;
               if (spot [1] == 'r')  Artc [NArtc+i].Inst = NInst-1;
               else                  Artc [NArtc+i].Inst = 0xFFFFFFFF;
            }
            NArtc += i;
         }

      // huh?
         else {
            sprintf (out, "%s %c%c%c%c %d???\r\n",
                     spot, ch.id[0],ch.id[1],ch.id[2],ch.id[3], ch.sz);
            fo.Put(out);
         }

         p += EVEN_UP (ch.sz);
      }
   }

// cleanup fn;  unDup
   for (p = 0; p < NPset; p++)  FixFn (PsetNm [p], Pset [p].name);
   for (p = 0; p < NInst; p++)
      for (u = 2, t = p+1; t < NPset; t++)
         if (StrCm (PsetNm [p], PsetNm [t]) == 0) {
            sprintf (out, "_%02d", u++);    StrAp (PsetNm [t], out);
            sprintf (out, "renamed pset %s\r\n", PsetNm [t]);
            fo.Put  (out);
         }
   for (w = 0; w < NWave; w++)  FixFn (WaveNm [w], Wave [w].name);
   for (w = 0; w < NWave; w++)
      for (u = 2, t = w+1; t < NWave; t++)
         if (StrCm (WaveNm [w], WaveNm [t]) == 0) {
            sprintf (out, "_%02d", u++);    StrAp (WaveNm [t], out);
            sprintf (out, "renamed Wave %s\r\n", WaveNm [t]);
            fo.Put  (out);
         }
}


//------------------------------------------------------------------------------
void SaveWav (File fo, int id)
{ File  f;
  ulong ln1, ln2, ln3, ln4;
  WAVEFORMATEX wf;
  struct {ulong manuf;  ulong prod;  ulong per;  ulong note;  ulong frac;
          ulong sfmt;   ulong sofs;  ulong num;  ulong dat;   ulong cue;
          ulong loop;   ulong bgn;   ulong end;  ulong frc;   ulong times;
  } smpl;
  TStr fn;
   sprintf (fn, "%s\\%s.wav", DLSPath, WaveNm [id]);

// assume mono
   Len = Wave [id].len / (Wave [id].fmt.wBitsPerSample / 8);
   if (Wave [id].smplGot && Wave [id].smpl.cSampleLoops) {
      LpBgn = Wave [id].loop.ulStart;
      LpEnd = Wave [id].loop.ulStart + Wave [id].loop.ulLength;
   }
   else {LpBgn = 0;  LpEnd = Len;}
   Frq = Wave [id].fmt.nSamplesPerSec;
   Ptr = (sword *)(& B [Wave [id].pos]);

   ln4 = 60;
   ln3 = Len*(Wave [id].fmt.wBitsPerSample / 8);  if (ln3 == 0)  return;
   ln2 = 16;
   ln1 = 12 + ln2 + 8 + ln3 + 8 + ln4;
   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = Wave [id].fmt.nChannels;
   wf.wBitsPerSample  = Wave [id].fmt.wBitsPerSample;
   wf.nSamplesPerSec  = Frq;
   wf.nBlockAlign     = wf.nChannels   * wf.wBitsPerSample / 8;
   wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
   if (f.Open (fn, "w")) {
      f.Put ("RIFF");      f.Put (& ln1, 4);
      f.Put ("WAVEfmt ");  f.Put (& ln2, 4);  f.Put (& wf,   ln2);
      f.Put ("data");      f.Put (& ln3, 4);  f.Put (Ptr,    ln3);
//    if (LenR) {
         memset (& smpl, 0, sizeof (smpl));
         smpl.per  = 1000000000 / Frq;
         smpl.note = Wave [id].smpl.usUnityNote;
         smpl.frac = Wave [id].smpl.sFineTune;
         smpl.num  = 1;
         smpl.bgn  = LpBgn;
         smpl.end  = LpEnd;
         f.Put ("smpl");  f.Put (& ln4, 4);  f.Put (& smpl, ln4);
//    }
      f.Shut ();
   }
  char out [301];
   sprintf (out, "Wave %d: name='%s' bits=%d frq=%d len=%d\r\n",
            id, WaveNm [id], Wave [id].fmt.wBitsPerSample, Frq, Len);
   fo.Put (out);
}


void PutArtc (File *f, ulong a)
{ uword s, c, d, t;
  TStr  out;
   for (d = 0; d < BITS (Dst); d++)
      if (Dst [d].id == Artc [a].artc.usDestination)  break;
   if (d < BITS (Dst))
        sprintf (out, "   %s",        Dst  [d].sym);
   else sprintf (out, "   ($%04X?)",  Artc [a].artc.usDestination);
   f->Put (out);

   for (t = 0; t < BITS (Trn); t++)
      if (Trn [t].id == Artc [a].artc.usTransform)    break;
   if (t < BITS (Trn))
        sprintf (out, "%s=",          Trn  [t].sym);
   else sprintf (out, "[$%04X]=",     Artc [a].artc.usTransform);
   f->Put (out);

   sprintf (out, "%d*",               Artc [a].artc.lScale);
   f->Put (out);

   for (c = 0; c < BITS (Src); c++)
      if (Src [c].id == Artc [a].artc.usControl)      break;
   if (c < BITS (Src))
        sprintf (out, "%s+",          Src  [c].sym);
   else sprintf (out, "($%04X)+",     Artc [a].artc.usControl);
   f->Put (out);

   for (s = 0; s < BITS (Src); s++)
      if (Src [s].id == Artc [a].artc.usSource)       break;
   if (s < BITS (Src))
        sprintf (out, "%s\r\n",       Src  [s].sym);
   else sprintf (out, "($%04X?)\r\n", Artc [a].artc.usSource);
   f->Put (out);
}


//------------------------------------------------------------------------------
void Proc (File fo, File fp)
{ char  out [4000];
  ulong p, i, a, w;
  TStr  ts, t2;

// write _info,_preset file
   for (p = 0; p < NPset; p++) {
//    if (Pset [p].bank & 0xFF80)  {DoDrum (fo, gm, p);  continue;}

   // list fn in _info.txt file
      sprintf (out, "%lu($%08X)  %s\r\n",
               Pset [p].prog, Pset [p].prog, PsetNm [p]);
      fo.Put (out);
      fp.Put (out);

   // write pset info: GLOBAL, then REGION based insts
      for (a = 0; a < NArtc; a++)
         if ((Artc [a].Pset == p) && (Artc [a].Inst == 0xFFFFFFFF))  break;
      if (a < NArtc) {
         fp.Put ("GLOBAL\r\n");
         while ((Artc [a].Pset == p) && (Artc [a].Inst == 0xFFFFFFFF))
            PutArtc (& fp, a++);
      }
      for (i = 0; i < NInst; i++)  if (Inst [i].Pset == p) {
         sprintf (out, "%s\r\n", WaveNm [Inst [i].link.ulTableIndex]);
         fp.Put (out);
         if ( Inst [i].inst.RangeKey.usLow ||
             (Inst [i].inst.RangeKey.usHigh != 127)) {
            sprintf (out, "   keyRng=%s-%s\r\n",
                     MKey2Str (ts, (ubyte)(Inst [i].inst.RangeKey.usLow)),
                     MKey2Str (t2, (ubyte)(Inst [i].inst.RangeKey.usHigh)));
            fp.Put (out);
         }
         if ( Inst [i].inst.RangeVelocity.usLow ||
             (Inst [i].inst.RangeVelocity.usHigh != 127)) {
            sprintf (out, "   velRng=%d-%d\r\n",
                     Inst [i].inst.RangeVelocity.usLow,
                     Inst [i].inst.RangeVelocity.usHigh);
            fp.Put (out);
         }
         if (Inst [i].smplGot) {
            sprintf (out, "   overRoot=%s\r\n",
                     MKey2Str (ts, (ubyte)(Inst [i].smpl.usUnityNote)));
            fp.Put (out);
            if (Inst [i].smpl.sFineTune) {
               sprintf (out, "   oscCent=%d\r\n", Inst [i].smpl.sFineTune);
               fp.Put (out);
            }
            if (Inst [i].smpl.lAttenuation) {
               sprintf (out, "   ampLvl=%d\r\n",  Inst [i].smpl.lAttenuation);
               fp.Put (out);
            }
            if (Inst [i].smpl.cSampleLoops) {
               sprintf (out, "   smLpBgn=%d\r\n"
                             "   smLpEnd=%d\r\n",
                   Inst [i].loop.ulStart & 0x0000FFFF,
                  (Inst [i].loop.ulStart + Inst [i].loop.ulLength)
                                         & 0x0000FFFF);
               fp.Put (out);
               if (Inst [i].loop.ulStart + Inst [i].loop.ulLength > 65535) {
                  sprintf (out, "   smLpBgnH=%d\r\n"
                                "   smLpEndH=%d\r\n",
                      Inst [i].loop.ulStart >> 16,
                     (Inst [i].loop.ulStart + Inst [i].loop.ulLength) >> 16);
                  fp.Put (out);
               }
            }
         }
         for (a = 0; a < NArtc; a++)
            if ((Artc [a].Pset == p) && (Artc [a].Inst == i))
               PutArtc (& fp, a);
      }
      fp.Put ("==========================================================\r\n");
   }

// write wavs  (whether ref'd or not :/ )
   for (w = 0; w < NWave; w++)  SaveWav (fo, w);
}


//------------------------------------------------------------------------------
int Go ()
{ TStr fn, pre;
  File f, fp;
// check ext
   if (StrCm (& App.parm [StrLn (App.parm)-4], ".dls"))
      Die (App.parm, "not a .dls file");

// suck in the whole dang thing
   L = f.Load (App.parm, B, sizeof (B));
   if ((L == 0) || (L == sizeof (B)))  Die ("file has len=0 or is TOO BIG");

// DLSPath = path of .dls file
   StrCp (DLSPath, App.parm);   Fn2Path (DLSPath);
   StrCp (pre, & App.parm [StrLn (DLSPath)+1]);
   StrAp (pre, "", 4);
   StrAp (DLSPath, "\\");   StrAp (DLSPath, pre);

   f.PathMake (DLSPath);
   sprintf (fn, "%s\\_info.txt",   DLSPath);
   if (! f.Open  (fn, "w"))  Die (fn, "couldn't write _info.txt file?");
   sprintf (fn, "%s\\_preset.txt", DLSPath);
   if (! fp.Open (fn, "w"))  Die (fn, "couldn't write _preset.txt file?");

   Load (f);
   Proc (f, fp);

   f.Shut ();   fp.Shut ();
   return 0;
}


// need the stupid crt startup due to dang dls .h files :(
int WINAPI WinMain (HINSTANCE inst, HINSTANCE pInst, LPSTR cmdLn, int nShowCmd)
{  return AppBoot ();  }
