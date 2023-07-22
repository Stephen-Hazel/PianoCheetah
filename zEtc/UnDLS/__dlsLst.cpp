// dlsLst.cpp - list .dls contents

#include "ui.h"
#include <dls1.h>
#include <dls2.h>

#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)

typedef struct {
   char         name [21];
   WAVEFORMATEX fmt;
   ulong        len;
   bool         smplGot;
   WSMPL        smpl;
   WLOOP        loop;
} WaveDef;

typedef struct {
   ulong      inst;
   ulong      regn;
   CONNECTION artc;
} ArtcDef;

typedef struct {
   ulong     inst;
   RGNHEADER regn;
   WAVELINK  link;
   bool      smplGot;
   WSMPL     smpl;
   WLOOP     loop;
} RegnDef;

typedef struct {
   char       name [21];
   ulong      prog;
   INSTHEADER inst;
} InstDef;


ubyte B [16*1024*1024];
ulong L;

ulong  nWave;   WaveDef Wave [1000];
ulong  nArtc;   ArtcDef Artc [100000];
ulong  nRegn;   RegnDef Regn [10000];
ulong  nInst;   InstDef Inst [1000];

char *Nt[12] =
   {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

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


//------------------------------------------------------------------------------
char *FixNm (char *fn, char *in)
{  memcpy (fn, in, 20);  fn [20] = '\0';
   for (char *p = fn; *p; p++)
      if ((*p <  ' ') || (*p >  '~') ||
          (*p == '*') || (*p == '?') || (*p == '/') || (*p == '\\') ||
          (*p == ':') || (*p == '"') || (*p == ' ') ||
          (*p == '>') || (*p == '<') || (*p == '|'))
         *p = '_';
   for (; *fn && (fn [strlen (fn)-1] == '_');)  fn [strlen (fn)-1] = '\0';
   return fn;
}


//------------------------------------------------------------------------------
typedef struct {char id [4];  ulong sz;} ChDef;

void Load (File f)
{ ulong p, i;       // byte pos in B[L]
  ChDef ch;
  char  spot [80];  // inst, inst/regn, inst/artc, inst/regn/artc, wave
  ulong endp [80];
  char  id [5], zstr [1000], out [8000];
   p = 0;   strcpy (spot, "");

// check header for RIFF, DLS
   if (memcmp (& B [0], "RIFF", 4) || memcmp (& B [8], "DLS ", 4))
      throw Excep ("not a RIFF-DLS file");

// start parsin chunks
   for (p = 12;  (p + 8) <= L;) {
   // get chunk
      memcpy (& ch, & B [p], 8);    p += 8;
//sprintf(out, "%c%c%c%c %d nRegn=%d\n",
//ch.id[0],ch.id[1],ch.id[2],ch.id[3],ch.sz, nRegn);   DBG(out);

   // see if LIST outa scope
      while (strlen (spot) && (p >= endp [strlen (spot) - 1]))
         spot [strlen (spot) - 1] = '\0';
      if (memcmp (ch.id, "LIST", 4) == 0) {
      // LISTs...
         if (p+4 > L)
            throw Excep ("bad LIST chunk");
         memcpy (id, & B [p], 4);  p += 4;  id [4] = '\0';
      // ignore
         if    ( (strcmp (id, "wvpl") == 0) ||
                 (strcmp (id, "lins") == 0) ||
                 (strcmp (id, "lrgn") == 0) ||
                 (strcmp (id, "INFO") == 0) )
            *id = '\0';
      // spot
         else if ( strcmp (id, "ins ") == 0) {
            *id = 'i';
            if (++nInst == BITS (Inst))  throw Excep ("too many Insts");
         }
         else if ((strcmp (id, "rgn ") == 0) || (strcmp (id, "rgn2") == 0)) {
            *id = 'r';
            if (++nRegn == BITS (Regn))  throw Excep ("too many Regns");
            Regn [nRegn-1].inst = nInst-1;
         }
         else if ((strcmp (id, "lart") == 0) || (strcmp (id, "lar2") == 0)) {
            *id = 'a';
         }
         else if ( strcmp (id, "wave") == 0) {
            *id = 'w';
            if (++nWave == BITS (Wave))  throw Excep ("too many Waves");
         }
      // huh?
         else {
            sprintf (out, "%s LIST %s %d???\r\n", spot, id, ch.sz);
            f.Put(out,strlen(out));
            *id = '\0';
         }
         if (*id) {
            endp [strlen (spot)] = p + ch.sz;
            id [1] = '\0';   strcat (spot, id);
         }
      }
      else {
      // chunk...
         if ((p + EVEN_UP (ch.sz)) > L)
            throw Excep ("bad chunk size");
      // ignore
         if      ( (memcmp (ch.id, "colh", 4) == 0) ||
                   (memcmp (ch.id, "ptbl", 4) == 0) ||
                   (memcmp (ch.id, "smpl", 4) == 0) ||
                   (memcmp (ch.id, "guid", 4) == 0) ||
                   (memcmp (ch.id, "dlid", 4) == 0) )  ;
      // zstr in various spots
         else if ( (memcmp (ch.id, "IARL", 4) == 0) ||
                   (memcmp (ch.id, "IART", 4) == 0) ||
                   (memcmp (ch.id, "ICMS", 4) == 0) ||
                   (memcmp (ch.id, "ICMT", 4) == 0) ||
                   (memcmp (ch.id, "ICOP", 4) == 0) ||
                   (memcmp (ch.id, "ICRD", 4) == 0) ||
                   (memcmp (ch.id, "IENG", 4) == 0) ||
                   (memcmp (ch.id, "IGNR", 4) == 0) ||
                   (memcmp (ch.id, "IKEY", 4) == 0) ||
                   (memcmp (ch.id, "IMED", 4) == 0) ||
                   (memcmp (ch.id, "INAM", 4) == 0) ||
                   (memcmp (ch.id, "IPRD", 4) == 0) ||
                   (memcmp (ch.id, "ISBJ", 4) == 0) ||
                   (memcmp (ch.id, "ISFT", 4) == 0) ||
                   (memcmp (ch.id, "ISRC", 4) == 0) ||
                   (memcmp (ch.id, "ISRF", 4) == 0) ||
                   (memcmp (ch.id, "ITCH", 4) == 0) ||
                   (memcmp (ch.id, "DATE", 4) == 0) ) {
            if (ch.sz >= sizeof (zstr))
               throw Excep ("zstr chunk TOO BIG");
            memcpy (zstr, & B [p], ch.sz);
            zstr [ch.sz] = '\0';  // just in case
            if      ((! memcmp (ch.id, "INAM", 4)) && (strcmp (spot, "w") == 0))
                 {FixNm (Wave [nWave-1].name, zstr);}
            else if ((! memcmp (ch.id, "INAM", 4)) && (strcmp (spot, "i") == 0))
                 {FixNm (Inst [nInst-1].name, zstr);}
            else {
               sprintf (out, "%s %c%c%c%c: %s\r\n",
                        spot, ch.id[0],ch.id[1],ch.id[2],ch.id[3], zstr);
               f.Put (out,strlen(out));
            }
         }

         else if (memcmp (ch.id, "vers", 4) == 0) {
           DLSVERSION v;
            memcpy (& v, & B [p], sizeof (v));
            sprintf (out, "%s vers %d,%d,%d,%d\r\n", spot,
                     HIWORD(v.dwVersionMS), LOWORD(v.dwVersionMS),
                     HIWORD(v.dwVersionLS), LOWORD(v.dwVersionLS));
            f.Put (out,strlen(out));
         }

      // Wave[].fmt
         else if (memcmp (ch.id, "fmt ", 4) == 0)
            {memcpy (& Wave [nWave-1].fmt, & B [p], sizeof (WAVEFORMATEX));}
      // Wave[].data
         else if (memcmp (ch.id, "data", 4) == 0)
            {Wave [nWave-1].len = ch.sz;}
      // {Wave|Regn}[].smpl+.loop
         else if (memcmp (ch.id, "wsmp", 4) == 0) {
            if (strcmp (spot, "w") == 0) {
               Wave [nWave-1].smplGot = true;
               memcpy (& Wave [nWave-1].smpl, & B [p],
                       sizeof (WSMPL) + sizeof (WLOOP));
               if (Wave [nWave-1].smpl.cSampleLoops == 0)
                  memset (& Wave [nWave-1].loop, 0, sizeof (WLOOP));
            }
            else {
               Regn [nRegn-1].smplGot = true;
               memcpy (& Regn [nRegn-1].smpl, & B [p],
                       sizeof (WSMPL) + sizeof (WLOOP));
               if (Regn [nRegn-1].smpl.cSampleLoops == 0)
                  memset (& Regn [nRegn-1].loop, 0, sizeof (WLOOP));
            }
         }

      // Inst[].inst+.prog
         else if (memcmp (ch.id, "insh", 4) == 0) {
            memcpy (& Inst [nInst-1].inst, & B [p], sizeof (INSTHEADER));
            Inst [nInst-1].prog =
               (Inst [nInst-1].inst.Locale.ulBank & F_INSTRUMENT_DRUMS) |
               ((Inst [nInst-1].inst.Locale.ulBank & 0x0000FFFF) << 8)  |
               (Inst [nInst-1].inst.Locale.ulInstrument & 0x00FF);
         }

      // Regn[]
         else if (memcmp (ch.id, "rgnh", 4) == 0)
            {memcpy (& Regn [nRegn-1].regn, & B [p], sizeof (RGNHEADER));}
         else if (memcmp (ch.id, "wlnk", 4) == 0)
            {memcpy (& Regn [nRegn-1].link, & B [p], sizeof (WAVELINK));}

      // Artc[]
         else if ((memcmp (ch.id, "art1", 4) == 0) ||
                  (memcmp (ch.id, "art2", 4) == 0)) {
           CONNECTIONLIST *cl = (CONNECTIONLIST *)(& B [p]);
            i = cl->cConnections;
            if (nArtc + i > BITS (Artc))
               throw Excep ("too many Artcs");
            for (i = 0; i < cl->cConnections; i++) {
               memcpy (& Artc [nArtc+i].artc,
                       & B [p + sizeof (CONNECTIONLIST) +
                            i * sizeof (CONNECTION)], sizeof (CONNECTION));
               Artc [nArtc+i].inst = nInst-1;
               if (stricmp (spot, "ia"))  Artc [nArtc+i].regn = nRegn-1;
               else                       Artc [nArtc+i].regn = 0xFFFFFFFF;
            }
            nArtc += i;
         }

      // huh?
         else {
            sprintf (out, "%s %c%c%c%c %d???\r\n",
                     spot, ch.id[0],ch.id[1],ch.id[2],ch.id[3], ch.sz);
            f.Put(out,strlen(out));
         }

         p += EVEN_UP (ch.sz);
      }
   }
}


//------------------------------------------------------------------------------
int Go ()
{ char fn [MAX_PATH];
  File f;
  ulong i, r, a, c1, c2;
   strcpy (fn, App.parm);  // "c:\\_\\Src\\Ditty\\_dlsLst\\_new1.dls");
// check ext;  make _lst.txt file
   if ((strlen (fn) <= 4) || stricmp (& fn [strlen (fn)-4], ".dls"))
      throw Excep (fn, "no .dls extension on file???");
   L = f.Load (fn, B, sizeof (B));
   if ((L == 0) || (L == sizeof (B)))
      throw Excep ("file has len=0 or is TOO BIG");

   strcpy (& fn [strlen (fn)-4], "_Lst.txt");
   if (! f.Open (fn, "w"))
      throw Excep (fn, "couldn't write file?");


// go!
   Load (f);


   for (i = 0; i < nInst; i++) {
      sprintf (fn, "----------------------------------------\r\n"
                   "%s ($%X)\r\n",   Inst [i].name, Inst [i].prog);
      f.Put(fn,strlen(fn));

      for (c1 = 1, a = 0; a < nArtc; a++)
         if ((Artc [a].inst == i) && (Artc [a].regn == 0xFFFFFFFF)) {
           uword s, c, d, t;
            sprintf (fn, "  %d) ", c1);
            f.Put(fn,strlen(fn));
            for (d = 0; d < BITS (Dst); d++)
               if (Dst [d].id == Artc [a].artc.usDestination)  break;
            if (d < BITS (Dst))
                 sprintf (fn, "%s",        Dst  [d].sym);
            else sprintf (fn, "($%04X?)",  Artc [a].artc.usDestination);
            f.Put(fn,strlen(fn));

            for (t = 0; t < BITS (Trn); t++)
               if (Trn [t].id == Artc [a].artc.usTransform)    break;
            if (t < BITS (Trn))
                 sprintf (fn, "%s=",       Trn  [t].sym);
            else sprintf (fn, "[$%04X]=",  Artc [a].artc.usTransform);
            f.Put(fn,strlen(fn));

            for (s = 0; s < BITS (Src); s++)
               if (Src [s].id == Artc [a].artc.usSource)       break;
            if (s < BITS (Src))
                 sprintf (fn, "%s+",       Src  [s].sym);
            else sprintf (fn, "($%04X?)+", Artc [a].artc.usSource);
            f.Put(fn,strlen(fn));

            for (c = 0; c < BITS (Src); c++)
               if (Src [c].id == Artc [a].artc.usControl)      break;
            if (c < BITS (Src))
                 sprintf (fn, "%s*",       Src  [c].sym);
            else sprintf (fn, "($%04X)*",  Artc [a].artc.usControl);
            f.Put(fn,strlen(fn));

            sprintf (fn, "%d\r\n", Artc [a].artc.lScale);
            f.Put(fn,strlen(fn));
            c1++;
         }

      for (c1 = 1, r = 0; r < nRegn; r++)
         if (Regn [r].inst == i) {
            sprintf (fn, "  %d) key=%d%s-%d%s ",
               c1, Regn [r].regn.RangeKey.usLow / 12,
               Nt [Regn [r].regn.RangeKey.usLow % 12],
                   Regn [r].regn.RangeKey.usHigh / 12,
               Nt [Regn [r].regn.RangeKey.usHigh % 12]);
            f.Put(fn,strlen(fn));
            if (Regn [r].regn.RangeVelocity.usLow ||
                (Regn [r].regn.RangeVelocity.usHigh != 127)) {
               sprintf (fn, "vel=%d-%d ",
                  Regn [r].regn.RangeVelocity.usLow,
                  Regn [r].regn.RangeVelocity.usHigh);
               f.Put(fn,strlen(fn));
            }
            sprintf (fn, "grp=%d %s\r\n",
               Regn [r].regn.usKeyGroup, 
               Regn [r].regn.fusOptions & F_RGN_OPTION_SELFNONEXCLUSIVE
               ? "_selfNonExcl" : "");
            f.Put(fn,strlen(fn));
            sprintf (fn, "    wav=%d phaseGrp=%d chn=$%X %s%s\r\n",
               Regn [r].link.ulTableIndex,  Regn [r].link.usPhaseGroup,
               Regn [i].link.ulChannel,
               (Regn [r].link.fusOptions & F_WAVELINK_PHASE_MASTER)
               ? "_phaseMast" : "",
               (Regn [r].link.fusOptions & F_WAVELINK_MULTICHANNEL)
               ? "_multiChan" : "");
            f.Put(fn,strlen(fn));
            if (Regn [r].smplGot) {
               sprintf (fn, "    note=%d%s tune=%d levl=%d %s%s\r\n",
                      Regn [r].smpl.usUnityNote / 12,  
                  Nt [Regn [r].smpl.usUnityNote % 12],  
                  Regn [r].smpl.sFineTune,   Regn [r].smpl.lAttenuation,
                  (Regn [r].smpl.fulOptions & F_WSMP_NO_TRUNCATION)
                  ? "_noTrunc" : "",
                  (Regn [r].smpl.fulOptions & F_WSMP_NO_COMPRESSION)
                  ? "_noCompr" : "");
               f.Put(fn,strlen(fn));
               if (Regn [r].smpl.cSampleLoops) {
                  sprintf (fn, "    bgn=%d len=%d %s\r\n",
                     Regn [r].loop.ulStart,  Regn [r].loop.ulLength,
                     (Regn [r].loop.ulType == WLOOP_TYPE_FORWARD) ? "_frwd" : (
                     (Regn [r].loop.ulType == WLOOP_TYPE_RELEASE) ? "_rels" : 
                     ""));
                  f.Put(fn,strlen(fn));
               }
            }

            for (c2 = 1, a = 0; a < nArtc; a++)
               if ((Artc [a].inst == i) && (Artc [a].regn == r)) {
                 uword s, c, d, t;
                  sprintf (fn, "      %d) ", c2);
                  f.Put(fn,strlen(fn));
                  for (d = 0; d < BITS (Dst); d++)
                     if (Dst [d].id == Artc [a].artc.usDestination)  break;
                  if (d < BITS (Dst))
                       sprintf (fn, "%s",        Dst  [d].sym);
                  else sprintf (fn, "($%04X?)",  Artc [a].artc.usDestination);
                  f.Put(fn,strlen(fn));

                  for (t = 0; t < BITS (Trn); t++)
                     if (Trn [t].id == Artc [a].artc.usTransform)    break;
                  if (t < BITS (Trn))
                       sprintf (fn, "%s=",       Trn  [t].sym);
                  else sprintf (fn, "[$%04X]=",  Artc [a].artc.usTransform);
                  f.Put(fn,strlen(fn));

                  for (s = 0; s < BITS (Src); s++)
                     if (Src [s].id == Artc [a].artc.usSource)       break;
                  if (s < BITS (Src))
                       sprintf (fn, "%s+",       Src  [s].sym);
                  else sprintf (fn, "($%04X?)+", Artc [a].artc.usSource);
                  f.Put(fn,strlen(fn));

                  for (c = 0; c < BITS (Src); c++)
                     if (Src [c].id == Artc [a].artc.usControl)      break;
                  if (c < BITS (Src))
                       sprintf (fn, "%s*",       Src  [c].sym);
                  else sprintf (fn, "($%04X)*",  Artc [a].artc.usControl);
                  f.Put(fn,strlen(fn));

                  sprintf (fn, "%d\r\n", Artc [a].artc.lScale);
                  f.Put(fn,strlen(fn));
                  c2++;
               }
            c1++;
         }
   }
   strcpy (fn, "----------------------------------------\r\n");
   f.Put(fn,strlen(fn));
   for (i = 0; i < nWave; i++) {
      sprintf (fn, "Wav[%d]  %s\r\n", i, Wave [i].name);
      f.Put(fn,strlen(fn));
      sprintf (fn, "  bits=%d frq=%d chn=%d len=%d\r\n",
         Wave [i].fmt.wBitsPerSample, Wave [i].fmt.nSamplesPerSec,
         Wave [i].fmt.nChannels,      Wave [i].len);
      f.Put(fn,strlen(fn));
      if (Wave [i].smplGot) {
         sprintf (fn, "  note=%d%s tune=%d levl=%d %s%s\r\n",
                Wave [i].smpl.usUnityNote / 12, 
            Nt [Wave [i].smpl.usUnityNote % 12], 
            Wave [i].smpl.sFineTune,   Wave [i].smpl.lAttenuation,
            (Wave [i].smpl.fulOptions & F_WSMP_NO_TRUNCATION)  
            ? "_noTrunc" : "",
            (Wave [i].smpl.fulOptions & F_WSMP_NO_COMPRESSION) 
            ? "_noCompr" : "");
         f.Put(fn,strlen(fn));
         if (Wave [i].smpl.cSampleLoops) {
            sprintf (fn, "  bgn=%d len=%d %s\r\n",
               Wave [i].loop.ulStart,  Wave [i].loop.ulLength,
               (Wave [i].loop.ulType == WLOOP_TYPE_FORWARD) ? "_frwd" : (
               (Wave [i].loop.ulType == WLOOP_TYPE_RELEASE) ? "_rels" : ""));
            f.Put(fn,strlen(fn));
         }
      }
   }


   f.Shut ();
   return 0;
}
