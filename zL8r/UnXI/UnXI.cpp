// UnXI.cpp - split a .xi to .WAV files in the xi fn dir

#include "ui.h"
#include "MidiIO.h"

typedef struct {
   ulong len, bgn, lln;
   ubyte vol, fine, type, pan;   sbyte trans;   ubyte nameln;
   char  name [22];
} SmpHdr;

TStr  XIPath;
ubyte B [64*1024*1024];
ulong L;
ulong Frq, Len, LpBgn, LpEnd;


char *FixFn (char *fn, char *in)
{  MemCp (fn, in, 22);   fn [22] = '\0';
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

/*
0136   B01            Volume
0137   B01            Finetune (signed)
0138   B01            Sample Type; b0,1=loop: 0=none 1=fwd 2=bidi
                                   b4 = 16bit sample
                                   b6 = two channels (stereo) [PsyTexx feature]
0139   B01            Panning (unsigned)
013A   B01            Relative Note (signed, 00=C-4 NOPE IT'S C6)
                                    (call it Transpose)
013B   B01            Sample Name Length
013C   C16    C22     Sample Name, padded w/ zeroes
   ulong len, bgn, lln;
   ubyte vol, fine, type, pan, trans, nameln;
   char  name [22];
*/

void SaveWav (uword w, SmpHdr *s, ulong p, File ft)
{ TStr  ts, wn, fn, out;
  File  f;
  ulong s16, ste, ln1, ln2, ln3, ln4, i;
  sbyte *pb, lb = 0;
  sword *pw, lw = 0;
  ubyte  nt, ntm = 255;
  WAVEFORMATEX wf;
  struct {ulong manuf;  ulong prod;  ulong per;  ulong note;  ulong frac;
          ulong sfmt;   ulong sofs;  ulong num;  ulong dat;   ulong cue;
          ulong loop;   ulong bgn;   ulong end;  ulong frc;   ulong times;
  } smpl;
   MemCp (ts, s->name, s->nameln);   ts [s->nameln] = '\0';   FixFn (wn, ts);
   sprintf (fn, "%s\\%02d_%s.wav", XIPath, w, wn);

// find sample note
   for (nt = M_NT (M_C, 4), i = 0;  i < 96;  i++)
      if (B [0x0042+i] == w) {
         if (ntm == 255)  ntm = (ubyte)(i + M_NT (M_C, 1));
         nt = (ubyte)(i + M_NT (M_C, 1));
      }

// assume 8 bit mono unlooped
   Len = s->len;
   s16 = 0;   if (s->type & 0x10)  s16 = 1;   if (s16) Len /= 2;
   ste = 0;   if (s->type & 0x40)  ste = 1;   if (ste) Len /= 2;

   sprintf (out, "%s s16=%d ste=%d type=%02X note=%d minNt=%d maxNt=%d "
                 "len=%d lpBgn=%d lpLen=%d\r\n",
            fn, s16, ste, s->type, s->trans, ntm, nt, s->len, s->bgn, s->lln);
   ft.Put (out);
DBG(out);

   LpBgn = LpEnd = Len;
   if ((s->type & 0x03) && s->lln)  {LpBgn = s->bgn;   LpEnd = LpBgn + s->lln;}
   Frq = 44100;
   if (s16)  for (pw = (sword *)(& B [p]), i = 0;  i < Len;  i++)
                {lw += pw [i];   pw [i] = lw;}
   else      for (pb = (sbyte *)(& B [p]), i = 0;  i < Len;  i++)
                {lb += pb [i];   pb [i] = (ubyte)(lb + 128);}
   ln4 = 60;
   ln3 = s->len;   if (ln3 == 0)  return;

   ln2 = 16;
   ln1 = 12 + ln2 + 8 + ln3 + 8 + ln4;
   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = (ste ? 2 : 1);
   wf.wBitsPerSample  = (s16 ? 2 : 1) * 8;
   wf.nSamplesPerSec  = Frq;
   wf.nBlockAlign     = wf.nChannels   * wf.wBitsPerSample / 8;
   wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
   if (! f.Open (fn, "w"))  return;

   f.Put ("RIFF");      f.Put (& ln1, 4);
   f.Put ("WAVEfmt ");  f.Put (& ln2, 4);  f.Put (& wf,    ln2);
   f.Put ("data");      f.Put (& ln3, 4);  f.Put (& B [p], ln3);
   MemSet (& smpl, 0, sizeof (smpl));
   smpl.per  = 1000000000 / Frq;
   smpl.note = nt;
   smpl.frac = 0;
   smpl.num  = 1;
   smpl.bgn  = LpBgn;
   smpl.end  = LpEnd;
   f.Put ("smpl");  f.Put (& ln4, 4);  f.Put (& smpl, ln4);
   f.Shut ();
}


//------------------------------------------------------------------------------
int Go ()
{ TStr fn, pre;
  File f;
// check ext
   if (StrCm (& App.parm [StrLn (App.parm)-3], ".xi"))
      Die (App.parm, "not a .xi file");

// suck in the whole dang thing
   L = f.Load (App.parm, B, sizeof (B));
   if ((L == 0) || (L == sizeof (B)))  Die ("file has len=0 or is TOO BIG");

// XIPath = path of .dls file
   StrCp (XIPath, App.parm);   Fn2Path (XIPath);
   StrCp (pre, & App.parm [StrLn (XIPath)+1]);
   StrAp (pre, "", 3);
   StrAp (XIPath, "\\");   StrAp (XIPath, pre);

   f.PathMake (XIPath);
   sprintf (fn, "%s\\_info.txt",  XIPath);
   if (! f.Open  (fn, "w"))  Die (fn, "couldn't write _info.txt file?");

// check header
   if (MemCm ((char *)B, "Extended Instrument: ", 21))
      Die ("not .xi format :(");
  SmpHdr *s;
  uword   nWave = B [0x0128] + 256 * B [0x129];
  ulong   p = 0x012A + (nWave * 40);
   for (uword w = 0;  w < nWave;  w++, p += s->len)
      {s = (SmpHdr *)(& B [0x12A + w*40]);   SaveWav (w, s, p, f);}

   f.Shut ();
   return 0;
}


/*
--------------------------------------------------------------------------
                          XI format description
                   (FastTracker II Extended Instrument)
          reverse engineered by KB / The Obsessed Maniacs / Reflex
                Changed for PsyTexx by Alex Zolotov (2007)
                Edited for Samplicity by Andrew Magalich (2012)
--------------------------------------------------------------------------
C = Chars, B = Byte, W = Word, D = Double word

Pos(h) Len(h) Len(d)  Meaning
-------------------------------------------------------------- file header
0000   C15    C21     "Extended Instrument: "
0015   C16    C22     Instrument name, padded w/ spaces

002b   B01    B01     $1a
002c   C14    C20     Tracker name, padded w/ spaces
0040   W02    W02     Version Number (current $0102)
                      ($5050 - PsyTexx instrument)

-------------------------------------------------------------- inst header
0042   B60    B96     Sample number for notes 1..96

00a2   B30    B48     12 volume envelope points:
                      +0 Time index for Point 1 (ticks since note)
                      +2 Volume for Point 1     (00..40)
                      +4 Time index for Point 2
                      +6 Volume for Point 2
                      [...]
00d2   B30    B48     12 panning envelope points
                      (Same structure as volume envelope)

0102   B01            Number of volume points
0103   B01            Number of panning points
0104   B01            Volume sustain point
0105   B01            Volume loop start point
0106   B01            Volume loop end point
0107   B01            Panning sustain point
0108   B01            Panning loop start point
0109   B01            Panning loop end point
010a   B01            Volume type;   b0=on, b1=sustain, b2=loop
010b   B01            Panning type;  b0=on, b1=sustain, b2=loop

010c   B01            Vibrato type
010d   B01            Vibrato sweep
010e   B01            Vibrato depth
010f   B01            Vibrato rate

0110   W02            Volume fadeout (0..fff)
0112   B16    B22     ????? (Zeroes or extened info for PsyTexx
                             (vol,finetune,pan,relative,flags))
0128   W02            Number of Samples

---------------------------------------------------------- sample headers

012a   D04            Sample Length
012e   D04            Sample loop start
0132   D04            Sample loop length
0136   B01            Volume
0137   B01            Finetune (signed)
0138   B01            Sample Type; b0,1=loop: 0=none 1=fwd 2=bidi
                                   b4 = 16bit sample
                                   b6 = two channels (stereo) [PsyTexx feature]
0139   B01            Panning (unsigned)
013A   B01            Relative Note (signed, 00=C-4 NOPE IT'S C6)
                                    (call it Transpose)
013B   B01            Sample Name Length
013C   C16    C22     Sample Name, padded w/ zeroes

And so on w/ samples 1 to x
Length: $28 (40) bytes for each sample

------------------------------------------------------------- sample data

$012a+(number of samples)*$28 : sample data for all samples in delta values
                                (signed)
*/
