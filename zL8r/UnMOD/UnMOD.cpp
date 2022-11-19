// UnMOD.c - suck in a MOD, save out somethin USABle:  WAVs and MIDi

#include "os.h"
#include "MidiIO.h"

// our parsin' limits
const ulong MAX_EVNT = 256*1024;
const ubyte NTRK = 31;                 // instrument 1..31 => track 0..30
const ubyte EVSZ = 4;
const ubyte NCHN = 4;
const ubyte NLIN = 64;                 // 4 bars of 4/4 1e&a2e&a3e&a4e&a


// mod format stuffs...
const ulong STD_TEMPO = 6;             // 6 clocks per 16th note

const ubyte MC_ARPEG = 0x00;           // arpeggio if param!=0
const ubyte MC_PORTU = 0x01;           // portamento up
const ubyte MC_PORTD = 0x02;           // portamento down
const ubyte MC_PORTT = 0x03;           // tone portamento
const ubyte MC_VIBRA = 0x04;           // vibrato
const ubyte MC_PTVOL = 0x05;           // tone portamento volume
const ubyte MC_VBVOL = 0x06;           // vibrato volume
const ubyte MC_TREME = 0x07;           // tremelo
const ubyte MC_PAN   = 0x08;           // panning8
const ubyte MC_OFFST = 0x09;           // offset
const ubyte MC_VOLSL = 0x0A;           // volume slide
const ubyte MC_POSJP = 0x0B;           // position jump
const ubyte MC_VOLUM = 0x0C;           // volume
const ubyte MC_ENDJP = 0x0D;           // pattern break
const ubyte MC_EX    = 0x0E;           // mod command ex
const ubyte MC_TEMPO = 0x0F;           // speed / tempo

const uword Period [] = {              // 5 x 12 notes 1c..5b
  1712,1616,1524,1440,1356,1280,1208,1140,1076,1016, 960, 906,
   856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
   428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
   214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
   107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57
};

typedef struct {
   char  songname [20];
   struct {
      char  name [22];
      uword length;                    // ...these 4 gotta get swabbed :/
      uword volume;
      uword repeat;                    // loop start
      uword replen;                    // loop length
   }     sample [31];
   ubyte songlen;                      // len of playseq
   ubyte pad;
   ubyte playseq [128];                // order to play blocks in
   char  magic [4];                    // "M.K." (version id) for plain mod
} STMOD;


// globs -----------------------------------------------------------------------
TStr   FN, To;                         // arg path\filename.mod and new out dir
TStr   SS;                             // sampleset
File   LF;                             // To\log.txt fer loggin

ubyte  Mod [16*1024*1024];             // mem to hold the whole .mod file
ulong  ModLen, ModPos;                 // len of buf and pos we're parsin' at

STMOD *Hdr;                            // pointers to spots in Mod[] to parse
ubyte *Blk [128];
uword NBlk;
ulong  TempoDur = STD_TEMPO;

TStr   TSn [NTRK];                     // sound name per track
TrkEv  TEv [NTRK][MAX_EVNT];           // events - results of parsin'
ulong  NEv [NTRK];                     // how many per track (instrument)

ubyte PanChn [4]  = {0, 255, 255, 0};
ubyte PanTrk [31] = {128, 128, 128, 128, 128, 128, 128, 128,
                     128, 128, 128, 128, 128, 128, 128, 128,
                     128, 128, 128, 128, 128, 128, 128, 128,
                     128, 128, 128, 128, 128, 128, 128};

//------------------------------------------------------------------------------
#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)

void PutWav (ubyte t, ulong len, ulong lpBgn, ulong lpEnd)
// convert soundtracker sample to .wav file
{ TStr  fn, pa;
  PStr  ls;
  File  f;
  ulong ln1, ln2, ln3, ln4, i;
  WAVEFORMATEX wf;
  struct {ulong manuf;  ulong prod;  ulong per;  ulong note;  ulong frac;
          ulong sfmt;   ulong sofs;  ulong num;  ulong dat;   ulong cue;
          ulong loop;   ulong bgn;   ulong end;  ulong frc;   ulong times;
  } smpl;
  ubyte *ptr = & Mod [ModPos];
  ulong  frq = 16726;                  // <=ntsc,  (not 16574=pal??)
   len *= 2;                           // that's teh way MODs do it...:/
   for (i = 0;  i < len;  i++)  ptr [i] = (ubyte)(((sbyte *)ptr) [i] + 128);
   ln4 = 60;
   ln3 = EVEN_UP (len);
   ln2 = 16;
   ln1 = 12 + ln2 + 8 + ln3 + 8 + ln4;

   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = 1;
   wf.wBitsPerSample  = 8;
   wf.nSamplesPerSec  = frq;
   wf.nBlockAlign     = wf.nChannels   * wf.wBitsPerSample / 8;
   wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;

   MemSet (& smpl, 0, sizeof (smpl));
   smpl.per  = 1000000000 / frq;
   smpl.note = 60;                     // middle c
   smpl.frac = 0;                      // cents
   smpl.num  = 1;
   if ((lpEnd-lpBgn) > 4) {
      smpl.bgn = lpBgn*2;              // those dang MODs...:/
      smpl.end = lpEnd*2;
   }
   else  {smpl.bgn = smpl.end = len;}

   StrFmt (fn, "`s\\etc\\`s\\`s.WAV", To, TSn [t], TSn [t]);
   StrCp (pa, fn);   Fn2Path (pa);   f.PathMake (pa);

   if (! f.Open (fn, "w")) {LF.Put (StrFmt (ls, "can't write file `s\r\n", fn));
                            return;}

   f.Put ("RIFF"    );   f.Put (& ln1, 4);
   f.Put ("WAVEfmt ");   f.Put (& ln2, 4);   f.Put (& wf,   ln2);
   f.Put ("data"    );   f.Put (& len, 4);   f.Put (ptr,    len);
   if (len % 2)  f.Put ("", 1);
   f.Put ("smpl");       f.Put (& ln4, 4);   f.Put (& smpl, ln4);

   f.Shut ();
}


//------------------------------------------------------------------------------
void FixFn (char *s)
// make s safe for a filename
{ ulong c;
   for (c = 0;  c < StrLn (s);  c++) {
      if ((s [c] == '@') || (s [c] == '\r') || (s [c] == '\n') ||
          (s [c] == '/') || (s [c] == '\\') || (s [c] == ':' ) ||
          (s [c] == '?') || (s [c] == '*'))
         s [c] = '_';
      if ((s [c] <= ' ') || (s [c] > '~'))    s [c] = '_';
   }
}


void Swab (uword *uw)
// swap uword around cuz Amiga was MSB first, we're MSB last
{ ubyte *ub = (ubyte *)uw;
  ubyte  t = ub [0];   ub [0] = ub [1];   ub [1] = t;
}


ulong DUR (ulong t)
{ ulong out, res = M_WHOLE/64;
   out = ((4*t) / TempoDur) * res;
   if ((((4*t) % TempoDur) * 2) > TempoDur) out += res;
   return (out);
}


void CvtTrk (ubyte t)
{ ulong n = 0, time = 0, dur, maxdur;
  uword per, vol, note;
  ulong b, bl, l, c, i, j;
  ubyte in, tempo = 6, ins [4], cmd [4], byt [4],
        ttempo, tcmd, tbyt, *p;
  bool  got, rep;
  TStr  ts, ts2;
  PStr  ps;
   in = t + 1;                         // trk 0..30 <=> ins 1..31
LF.Put (StrFmt (ps, "inst=`d ----------\r\n", in));
LF.Put ("tr time        dur key vel   vol cmd\r\n");
   rep    = (Hdr->sample [t].replen > 1) ? true : false;
   maxdur =  Hdr->sample [t].length * (M_WHOLE/64) / 110;
   for (b = 0;  b < Hdr->songlen;  b++) {   // each block of song
      bl = Hdr->playseq [b];
      for (l = 0;  l < NLIN;  l++) {        // each line of block
         for (c = 0; c < NCHN; c++) {       // each chan of line
         // blk's lin's trk's ev (4 bytes of)
            p = & Blk [bl][(l*EVSZ*NCHN)+(c*EVSZ)];
            ins [c] = (p [0] & 0x10) | (p [2] >> 4);
            cmd [c] =  p [2] & 0x0F;
            byt [c] =  p [3];
            if ((cmd [c] == MC_TEMPO) && (byt [c] >= 1) && (byt [c] <= 32))
               tempo = byt [c];
         }
         for (c = 0; c < 4; c++) {
            p = & Blk [bl][(l*EVSZ*NCHN)+(c*EVSZ)];
            if (ins [c] != in)  continue;   // ev has to be for THIS ins

            vol = Hdr->sample [t].volume;   // usually 64
            if ((cmd [c] == MC_VOLUM) && (byt [c] <= 64))  vol = byt [c];

         // hi nibble reused :/
            per = *((uword *)p);   Swab (& per);   per &= 0x0FFF;
            if (! per)  continue;           // no note frq, no nothin

            got = false;
            for (note = 0;  note < BITS (Period);  note++)
               if (per == Period [note])  {got = true;  break;}
            if (! got) {
LF.Put (StrFmt (ps,
" Unknown note period `d($`03x) at block=`d line=`d channel=`d\r\n",
per, per, bl, l, c));
               continue;
            }

            dur = DUR (tempo);
            ttempo = tempo;   got = true;
            for (i = l+1;  got && (i < NLIN);  i++) {
               for (j = 0;  j < NCHN;  j++) {
                  tcmd = Blk [bl][(i*16)+(j*4)+2] & 0x0F;
                  tbyt = Blk [bl][(i*16)+(j*4)+3];
                  if ((tcmd == MC_TEMPO) && (tbyt >= 1) && (tbyt <= 32))
                     ttempo = tbyt;
                  if (tcmd == MC_ENDJP) {got = false;  break;}
                  if (tcmd == MC_POSJP)  got = false;
               }
               if (tcmd == MC_ENDJP) break;      // i know,  ...YUK

               p = & Blk [bl][(i*NCHN*EVSZ)+(c*EVSZ)];
               if ( (((uword *) p)[0]         )            ||
                   ((((uword *) p)[1] & 0x0FFF) == ((uword)MC_VOLUM<<8)) )
                     got = false;
               else  dur += (DUR (ttempo));
            }
            if ((! rep) && (dur > maxdur))  dur = maxdur;
            if (cmd [c] == MC_PAN)  PanChn [c] = byt [c];
            if (PanTrk [t] != PanChn [c]) {
                PanTrk [t]  = PanChn [c];
               if (n >= MAX_EVNT)  Die ("hit max events");
               TEv [t][n].time = time;
               TEv [t][n].ctrl = 0x80;      // pan
               TEv [t][n].valu = PanChn [c] >> 1;
               n++;
            }
            TEv [t][n].time = time;
            TEv [t][n].ctrl = M_NT(M_C,1) + note;
            TEv [t][n].valu = 0x80 | (ubyte)((127 * vol) / 64);
            if (TEv [t][n].valu == 0x80)  TEv [t][n].valu = 0x81;
LF.Put (StrFmt (ps,
"`>2d `s `>3d `<3s `>3d    `>2d `02x\r\n",
in, TmS (ts2, time), dur, MKey2Str (ts, M_NT(M_C,1)+note),
TEv [t][n].valu & 0x7F, vol, cmd [c]));
            n++;
            if (n >= MAX_EVNT)  Die ("hit max events");
            TEv [t][n].time = time + dur - 1;
            TEv [t][n].ctrl = M_NT(M_C,1) + note;
            TEv [t][n].valu = 0;
            n++;
         }
         if ((cmd [0] == MC_ENDJP) || (cmd [1] == MC_ENDJP) ||
             (cmd [2] == MC_ENDJP) || (cmd [3] == MC_ENDJP))  break;
         time += (DUR (tempo));
         if ((cmd [0] == MC_POSJP) || (cmd [1] == MC_POSJP) ||
             (cmd [2] == MC_POSJP) || (cmd [3] == MC_POSJP))  break;
      }
   }
   NEv [t] = n;
}


//------------------------------------------------------------------------------
void CvtMod ()
{ ubyte  trk, i, t;
  ulong  b;
  File   f;
  TStr   s, sfn, tfn;
  char   buf [8192];
  PStr   ps;
  TrkHdr hdr;

// parse header
   Hdr = (STMOD *)Mod;
   if (ModLen < sizeof (STMOD))
      {LF.Put ("Header too small\r\n");   return;}
   if (MemCm (Hdr->magic, "M.K.", 4))
      {LF.Put ("not a regular (M.K.) MOD file\r\n");   return;}

   ModPos += sizeof (STMOD);
   LF.Put ("id length volume repeat repLen name\r\n");
   for (i = 0;  i < BITS (Hdr->sample);  i++) {
      Hdr->sample [i].name [21] = '\0';
      FixFn (Hdr->sample [i].name);
      Swab (& Hdr->sample [i].length);   Swab (& Hdr->sample [i].volume);
      Swab (& Hdr->sample [i].repeat);   Swab (& Hdr->sample [i].replen);
      LF.Put (StrFmt (ps,
"`>2d `>6d `>6d `>6d `>6d `s\r\n",
i+1, Hdr->sample [i].length, Hdr->sample [i].volume,
     Hdr->sample [i].repeat, Hdr->sample [i].replen, Hdr->sample [i].name));
   }

// setup block pointers (to 1K blocks of bytes in file)
   for (i = 0;  i < Hdr->songlen;  i++)
      if (Hdr->playseq [i] >= NBlk)  NBlk = Hdr->playseq [i] + 1;
   if (NBlk ==  0)  {LF.Put ("hmmm, no blocks?\r\n");     return;}
   if (NBlk > 128)  {LF.Put ("hmmm, >128 blocks?\r\n");   return;}

   for (b = 0;  b < NBlk;  b++)  {
      LF.Put (StrFmt (ps,
"block `d => `d=$`08x\r\n", b, ModPos, ModPos));
      Blk [b] = & Mod [ModPos];   ModPos += (EVSZ * NCHN * NLIN);  // 1024
   }

// makin' tracks fer each instrument (trk 0..30 => ins 1..31)
   for (trk = 0;  trk < NTRK;  trk++)  CvtTrk (trk);

// save out the samples as To\no_instName.WAV  (space => _
   for (t = 0;  t < NTRK;  t++) {
      StrFmt (TSn [t], "`02d_`s`s", t+1, Hdr->sample [t].name,
                                   (Hdr->sample [t].replen > 4) ? "" : "_hold");
//DBG("sample for trk `d => `d=$`08x  (len=`d)  `s",
//t, ModPos, ModPos, ModLen, TSn [t]);
      if (Hdr->sample [t].length)
         PutWav (t, Hdr->sample [t].length,  Hdr->sample [t].repeat,
                    Hdr->sample [t].repeat + Hdr->sample [t].replen);
      ModPos += (2 * Hdr->sample [t].length);
   }

// write .song in pc\etc dir
   App.Path (sfn, 'e');   StrAp (sfn, "\\tmp_m.song");
   App.Path (tfn, 'e');   StrAp (tfn, "\\tmp_m.trak");

   if (! f.Open (sfn, "w"))
      {LF.Put (StrFmt (ps, "couldn't write `s\r\n", sfn));   return;}

   f.Put ("learn=0\r\n");       // hearAll
   f.Put ("id length volume repeat repLen name\r\n");
   for (t = 0;  t < NTRK;  t++) {
      StrFmt (buf, "`>2d `>6d `>6d `>6d `>6d `s\r\n",
         t+1, Hdr->sample [t].length, Hdr->sample [t].volume,
         Hdr->sample [t].repeat, Hdr->sample [t].replen, Hdr->sample [t].name);
      f.Put (buf);
   }
   f.Put ("\r\nTrack:\r\n");
   for (t = 0;  t < NTRK;  t++)  if (NEv [t])
      f.Put (StrFmt (ps, "syn1  etc|`s\\`s  .`d\r\n",  SS, TSn [t], t+1));
   f.Put ("Control:\r\n");
   f.Put ("Pan\r\n");
   f.Shut ();

// write .trak
   if (! f.Open (tfn, "w"))
      {LF.Put (StrFmt (ps, "couldn't write `s\r\n", tfn));   return;}
   f.Put ("TRAK", 4);
   for (t = 0;  t < NTRK;  t++)  if (NEv [t])
      {hdr.ne = NEv [t];   hdr.dur = 0;   f.Put (& hdr, 8);}
   f.Put ("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8);
   for (t = 0;  t < NTRK;  t++)  if (NEv [t])
      f.Put (& TEv [t], sizeof (TrkEv) * NEv [t]);
   f.Shut ();

// turn em into .song w song2mid  get FN minus path,.ext with .mid into sfn
   StrCp (sfn, FN);   StrAp (sfn, ".mid", 4);
   RunWait (StrFmt (s, "`s\\song2mid.exe m `s", App.Path (tfn), sfn));
}


int Go ()
{ TStr  fn;
  File  f;
DBG("{ UnMOD Go `s", App.parm);
// MODs have loads of weird .exts so can't check
   if (App.parm [0] == '\0')  Die ("UnMOD filename.mod", NULL, 99);
   StrCp (FN, App.parm);

// sampleset is MOD_fn of arg w spaces => _
   StrCp (fn, FN);   Fn2Name (fn);     // toss .mod leaving path\fn
   StrCp (SS, "MOD_");                 // MOD_ prefix
   FnName (& SS [4], fn);              // toss path leaving just fn
   FixFn (SS);                         // spaces => _ etc

// already got it?  (or w leading _) musta already run so vamoose
   App.Path (To, 'd');   StrAp (To, "\\device\\syn\\_");   StrAp (To, SS);
   if (LF.PathGot (To))  {DBG ("already GOT `s", To);   return 0;}

   App.Path (To, 'd');   StrAp (To, "\\device\\syn\\");    StrAp (To, SS);
   if (LF.PathGot (To))  {DBG ("already GOT `s", To);   return 0;}

// ok, make To dir n open log.txt there
   LF.PathMake (To);   StrCp (fn, To);   StrAp (fn, "\\log.txt");
   if (! LF.Open (fn, "w"))   Die ("couldn't write `s ?", fn);

LF.Put (StrFmt (fn, "`s\r\n", FN));

// load mod file into memory
   if ((ModLen = f.Load (FN, Mod, sizeof (Mod))) == 0)
                              Die ("UnMOD  .mod not found", FN);
   if (ModLen == BITS (Mod))  Die ("UnMOD  .mod too big",   FN);

   CvtMod ();                          // do eeeet

   LF.Shut ();
DBG("} UnMOD Go");
   return 0;
}
