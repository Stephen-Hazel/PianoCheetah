// BIAB2Song.cpp - find bandInTheBox files; convert to chord text events,etc
//                      .SG*;  .MG*         .song

#include "ui.h"
#include "MidiIO.h"

const ulong BB_WHOLE = 120*4;          // biab's ticks per bar

ubyte Buf [1024*1024];
ulong Len, Pos, FNR;

ubyte Sty, Key,                        // basic style#, keysig#
      ChorusBgn, ChorusEnd, ChorusRep; // bar# for intro,chorus n ch reps
uword CodaEnd;                         // last beat of coda (not bar)
uword NNt, NLy, NNtX;                  // #melo notes, #lyrics
TStr  Ttl, STY;                        // title, ??some weird style
File  FO;

ubyte StyMap [256];                    // style map per bar
ubyte ChdTyp [4*255+1];                // chord type per beat
ubyte ChdNam [4*255+1];                // chord name per beat
TStr  Ly [400];                        // lyrics
typedef struct {ulong time, dur;   ubyte m0, m1, m2, m3;} NtDef;
NtDef Nt [65535];                      // ^melody
ubyte MelOut;

char *StyStr [] = {                    // basic styles to get tsig from
   "4_4_JazzSwing",                    // only 12/8 n 4/4 :/
   "12_8_Country",
   "4_4_Country",
   "4_4_BossaNova",
   "4_4_Ethnic",
   "4_4_BluesShuffle",
   "4_4_BluesStraight",
   "3_4_Waltz",
   "4_4_PopBallad",
   "4_4_RockShuffle",
   "4_4_liteRock",
   "4_4_mediumRock",
   "4_4_HeavyRock",
   "4_4_MiamiRock",
   "4_4_MillyPop",
   "4_4_Funk",
   "3_4_JazzWaltz",
   "4_4_Rhumba",
   "4_4_ChaCha",
   "4_4_Bouncy",
   "4_4_Irish",
   "12_8_PopBallad",
   "12_8_CountryOld",
   "4_4_Reggae"
};


char *KeyStr [] = {                    // keysigs
   "/",
   "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B",
        "C#", "D#", "F#", "G#", "A#",
   "Cm","Dbm","Dm","Ebm","Em","Fm","Gbm","Gm","Abm","Am","Bbm","Bm",
        "C#m","D#m","F#m","G#m","A#m"
};

char *ExtStr [256] = {                 // chord types
   "", "",         // 1 major
   "",
   "?5b",
   "aug",
   "6",
   "M7",           // maj7
   "M9",           // maj9
   "?M9#11",
   "?M13#11",
   "?M13",         // 10
   "?",
   "aug",          // +
   "M7#5",
   "6,9",
   "2",
   "m",
   "?mAug",
   "mM7",
   "m7",
   "m9",           // 20
   "?m11",
   "?m13",
   "m6",
   "?m#5",         // 1/3
   "?m7#5",
   "?m69",
   "?","?","?",
   "?",            // 30
   "?",
   "m7b5",
   "dim",
   "?m9b5",
   "?","?","?","?","?",
   "5",            // 40
   "?","?","?","?","?","?","?","?","?",
   "?",            // 50
   "?","?","?","?","?",
   "7#5",          // 7+
   "aug",          // +
   "?13Aug",       // 13+
   "?",
   "?",            // 60
   "?","?","?",
   "7",
   "13",
   "7b13",
   "7#11",
   "?",
   "?",
   "9",            // 70
   "?9b13",
   "?",
   "?9#11",
   "?13#11",
   "?",
   "7b9",
   "?13b9",
   "?",
   "?7b9#11",
   "?",            // 80
   "?",
   "7#9",
   "?13#9",
   "?7#9b13",
   "?9#11",
   "?","?",
   "7b5",
   "?13b5",
   "?",            // 90
   "?9b5",
   "?",
   "?7b5b9",
   "?","?",
   "?7b5#9",
   "?","?",
   "7#5",
   "?",            // 100
   "?","?",
   "?9#5",
   "?",
   "?7#5b9",
   "?","?","?",
   "?7#5#9",
   "?",            // 110
   "?","?",
   "?7alt",
   "?","?","?","?","?","?",
   "?",            // 120
   "?","?","?","?","?","?","?",
   "7sus",
   "?13sus",
   "?",            // 130
   "?","?","?",
   "11",
   "?","?","?","?","?",
   "?7susb9",       // 140
   "?","?","?","?","?",
   "?7sus#9",
   "?","?","?",
   "?",            // 150
   "?","?","?","?","?","?","?","?","?",
   "?",            // 160
   "?","?",
   "?7sus#5",
   "?","?","?","?","?","?",
   "?",            // 170
   "?","?","?","?","?","?",
   "4",
   "?","?",
   "?",            // 180
   "?","?","?",
   "sus",          // 184 - last known...
   "?","?","?","?","?",
   "?",            // 190
   "?","?","?","?","?","?","?","?","?",
   "?",            // 200
   "?","?","?","?","?","?","?","?","?",
   "?",            // 210
   "?","?","?","?","?","?","?","?","?",
   "?",            // 220
   "?","?","?","?","?","?","?","?","?",
   "?",            // 230
   "?","?","?","?","?","?","?","?","?",
   "?",            // 240
   "?","?","?","?","?","?","?","?","?",
   "?",            // 250
   "?","?","?","?","?"
};

char *AllBass [] = {                   // bass strs
   "/","C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb",
           "B","C#","D#","F#","G#","A#"
};
char *BassFl [] = {"B","C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb"};
char *BassSh [] = {"B","C","C#","D","D#","E","F","F#","G","G#","A","A#"};

char Line [800];                       // buffer :/


//------------------------------------------------------------------------------
void DoChd (uword i, ubyte b1, ubyte b2)
// dump every 16 beats=4 bars else buff  (just debuggin)
{ TStr root, bass, ts;
   if (! (i % 16)) {
      if (Line [0])  FO.Put (StrFmt (ts, "   `<3d `s\r\n", i/16*4-3, Line));
      Line [0] = '\0';
   }
   bass [0] = '\0';
   StrCp (root, AllBass [b1 % 18]);
   if (b1 > 18) {                      // slash chord
      if (root [1] == 'b')
            StrCp (bass, BassFl [((b1 / 18) + (b1 % 18)) % 12]);
      else  StrCp (bass, BassSh [((b1 / 18) + (b1 % 18)) % 12]);
   }
   StrAp (root, ExtStr [b2]);
   if (bass [0])  {StrAp (root, "/");   StrAp (root, bass);}
   StrAp (Line, root);   StrAp (Line, " ");
   if (((i % 4) == 3) && ((i % 16) != 15))  StrAp (Line, "| ");
}


void PutLyr (ulong tm, uword i)
{ TStr  ts, ts2, ts3;
  uword j;
   if (i % 16)               return;   // only every 4 bars
   if ((j = i / 16) >= NLy)  return;   // no lyrs in that spot

   StrCp (ts3, Ly [j]);                // space => _
   for (j = 0;  j < StrLn (ts3);  j++)  if (ts3 [j] == ' ')  ts3 [j] = '_';
   TmS (ts2, tm);   ts2 [6] = '\0';
   FO.Put (StrFmt (ts, "`s `s/\r\n", ts2, ts3));
}


TStr TSty;

void PutChd (ulong tm, uword i)
{ ubyte b1 = ChdNam [i], b2 = ChdTyp [i];
  TStr  root, bass, ts, ts2, ts3;
   bass [0] = '\0';
   StrCp (root, AllBass [b1 % 18]);
   if (b1 > 18) {                      // slash chord
      if (root [1] == 'b')
            StrCp (bass, BassFl [((b1 / 18) + (b1 % 18)) % 12]);
      else  StrCp (bass, BassSh [((b1 / 18) + (b1 % 18)) % 12]);
   }
   StrAp (root, ExtStr [b2]);
   if (bass [0])  {StrAp (root, "/");   StrAp (root, bass);}
   if (! StrCm (root, "/"))  StrCp (root, "");
   if ((i % 4) == 0) {
     char ch = StyMap [i / 4 + 1];
      if (ch) {
         ts [0] = 'a' + ch - 1;   ts [1] = '\0';
         if (StrCm (ts, TSty))
            {StrCp (TSty, ts);   StrAp (root, ".");   StrAp (root, ts);}
      }
   }
   if (root [0]) {                     // "0001.1 $$.*Fm"
      TmS (ts2, tm);   ts2 [6] = '\0';
      FO.Put (StrFmt (ts3, "`s $$.*`s\r\n", ts2, root));
   }
}


void PutMel (ulong tm, uword i)
{ TStr ts, ts2, ts3;
  ulong tb, te;
  uword j;
   tb = i * BB_WHOLE / 4;   te = tb + BB_WHOLE / 4;
   for (j = 0;  j < NNt;  j++)  if ((Nt [j].time >= tb) && (Nt [j].time < te)) {
      TmS (ts, (tm + (Nt [j].time-tb))            * M_WHOLE / BB_WHOLE);
      MKey2Str (ts2, Nt [j].m1);
      if (MelOut)  FO.Put (StrFmt (ts3, "`s `s_`d\r\n", ts, ts2, Nt [j].m2));
      TmS (ts, (tm + (Nt [j].time+Nt [j].dur-tb)) * M_WHOLE / BB_WHOLE - 1);
      if (MelOut)  FO.Put (StrFmt (ts3, "`s `s^64\r\n", ts, ts2));
      NNtX += 2;
   }
}


/* "x",
   "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B",
        "C#", "D#", "F#", "G#", "A#",
   "Cm","Dbm","Dm","Ebm","Em","Fm","Gbm","Gm","Abm","Am","Bbm","Bm",
        "C#m","D#m","F#m","G#m","A#m"
*/
TStr FlM = "x" "bb#b#bb#b#b#" "#####",
     Flm = "x" "bbbb#bbbbbb#" "#####";


void DoFN (char *fni)
{ TStr  fn, ts;
  ulong X, tm, dur, nn1, q;
  uword i, j;
  ubyte bpm, tsN, tsD, x, s [80], r;
  char *p;
  bool  got;
  File  f;
   StrCp (fn, fni);
DBG("fni=`s", & fni [FNR]);
   MemSet (StyMap, 0, sizeof (StyMap));   MemSet (ChdTyp, 0, sizeof (ChdTyp));
   MemSet (ChdNam, 0, sizeof (ChdNam));   MemSet (Ly, 0, sizeof (Ly));
   NLy = 0;

   Len = f.Load (fn, Buf, sizeof (Buf));
   if (Len < 8)                        {DBG (" File too small");   return;}

// title
   i = Buf [1];
   if (i > sizeof (TStr))              {DBG (" title too long");   return;}

   if (Len < (ulong)(2+i))             {DBG (" File too small b");   return;}
   MemCp (Ttl, & Buf [2], i);   Ttl [i] = '\0';   FnFix (Ttl);
   for (j = (uword)StrLn (Ttl);  j && Ttl [0] == '_';  j--)
      StrCp (Ttl, & Ttl [1]);          // ltrim title

// get style id, key, and tempo
   Pos = 2 + i + 2;                    // skip 2 more bytes
   if (Len < Pos+3)                    {DBG (" File too small c");   return;}
   Sty = Buf [Pos++];   Key = Buf [Pos++];   bpm = Buf [Pos++];

// add tsig to new fn - Ttl
   if (Sty >= BITS (StyStr))           {DBG (" bad Sty");   return;}
   if (! MemCm (StyStr [Sty], "12_8_", 5))
        {tsN = 12;   tsD = 8;
         StrCp (& Ttl [5], Ttl);   MemCp (Ttl, "12_8_", 5);}
   else {tsN = 4;    tsD = 4;
         StrCp (& Ttl [4], Ttl);   MemCp (Ttl, "4_4_", 4);}

// open output .song fn w title
   StrCp (fn, fni);   Fn2Path (fn);   StrAp (fn, "\\");   StrAp (fn, Ttl);
   if (FO.PathGot (fn)) {              // rename if title.song is already there
      i = 2;   StrAp (fn, "_2");
      while (FO.PathGot (fn))  StrAp (fn, StrFmt (ts, "_`d", ++i), 2);
   }
   FO.PathMake (fn);   StrAp (fn, "\\a.song");
   if (! FO.Open (fn, "w"))  {DBG (" couldnt write `s", fn);   return;}

   FO.Put (StrFmt (ts, "Sty=`s\r\n", StyStr [Sty]));

// read Style Map (slot 0 unused, 1-255 used - per bar)
   for (i = 0;  i < 256;) {
      x = Buf [Pos++];
      if (x == 0) {
         x = Buf [Pos++];
         if (x == 0)                   {DBG (" bad style map");   return;}
         i += x;
      }
      else
         StyMap [i++] = x;
   }
   if (i > 256)                        {DBG (" bad style map(2)");   return;}

// read Chord Types - 0..255*4-1
   for (i = 0;  i < 255*4;) {
      x = Buf [Pos++];
      if (x == 0)  {x = Buf [Pos++];   i += x;}
      else         ChdTyp [CodaEnd = i++] = x;   // git our last beat
   }
   if (i > 255*4)                      {DBG (" bad chord type map");   return;}

// read Chord Names (same way as chord types) including bass note
   for (i = 0;  i < 4*255+1;) {        // not sure WHAT that last chdroot is ??
      x = Buf [Pos++];
      if (x == 0)  {x = Buf [Pos++];   i += x;}
      else         ChdNam [i++] = x;
   }
   if (i > 255*4+1)                    {DBG (" bad chord root map");   return;}

   ChorusBgn = Buf [Pos++];   ChorusEnd = Buf [Pos++];
                              ChorusRep = Buf [Pos++];
   FO.Put (StrFmt (ts, "ChorBgn=`d ChorEnd=`d ChorRep=`d\r\n",
                       ChorusBgn, ChorusEnd, ChorusRep));
   FO.Put ("StyMap...\r\n");
   for (i = 1;  i < 255;  i++)  if (StyMap [i])
      FO.Put (StrFmt (ts, "   `<3d `c\r\n", i, 'a'+StyMap [i]-1));

   if (CodaEnd < 4)  {FO.Shut ();   DBG(" no chords :(");
                      StrAp (fn, "", 5);   FO.PathKill (fn);
                      return;}
   FO.Put ("Chords...\r\n");
   Line [0] = '\0';                    // do a pretty listing
   for (i = 0; i <= CodaEnd; i++)  DoChd (i, ChdNam [i], ChdTyp [i]);
   i--;
   FO.Put (StrFmt (ts, "   `<3d `s\r\n", i/16*4+1, Line));

   if (Pos > Len)                      {DBG (" EOF after getn bars");   return;}

   MemCp (Buf, & Buf [Pos], Len-Pos);   Len -= Pos;
   X = Pos;
   if (p = MemSt (Buf, ".STY", Len)) {
      MemCp (STY, p-8, 8);   STY [8] = '\0';     // 12345678.STY
      FnFix (STY);                               // worthless weird style...
//DBG("`s.STY", STY);
   }

// lyrics
   s [0] = 0;   s [1] = 0xFF;   s [2] = 0;   s [3] = 0x55;
   for (q = 0;  q+4 < Len;  q++)
      if (! MemCm ((char *)(& Buf [q]), (char *)s, 4, 'x'))  break;
   if (q+4 < Len) {
      q += 4;
      for (;;) {
         x = Buf [q++];
         if (x == 0) {
            if (Buf [q] == 0xFF)  break;
            if (Ly [NLy][0])  NLy++;
            q++;
         }
         else {
            MemCp (Ly [NLy], & Buf [q], x);   Ly [NLy][x] = '\0';
            if (Ly [NLy][0])  NLy++;
            q += x;
         }
      }
   }

// number of melody notes
   NNt = 0;
   s [0] = 0;   s [1] = 0xFF;   s [2] = 0;   s [3] = 0x0D;
   for (q = 0;  q+4 < Len;  q++)
      if (! MemCm ((char *)(& Buf [q]), (char *)s, 4, 'x'))  break;
   if (q+4 < Len)  nn1 = Buf [q+4] + Buf [q+5]*256 - 1;
   else            nn1 = 999;

// find start of notes
   got = false;
   s [0] = 0xA0;   s [1] = 0xB0;   s [2] = 0xC0;
   for (q = 0;  q+3 < Len;  q++)
      if (! MemCm ((char *)(& Buf [q]), (char *)s, 3, 'x'))  break;
   if (q+3 < Len)  got = true;
   else {
      s [2] = 0xC1;
      for (q = 0;  q+3 < Len;  q++)
         if (! MemCm ((char *)(& Buf [q]), (char *)s, 3, 'x'))  break;
      if (q+3 < Len) got = true;
   }
   if (got) {
   // parse each note if ya got em and quit if hit end, get bad one
      Pos = q + 3;
      if (((Len - Pos) / 12) < nn1)  nn1 = (uword)((Len - Pos) / 12);
      for (q = 0;  q < nn1;  q++, Pos += 12) {
         MemCp (& Nt [q].time, & Buf [Pos+0], 4);
         MemCp (& Nt [q].dur,  & Buf [Pos+8], 4);
         MemCp (& Nt [q].m0,   & Buf [Pos+4], 4);
         NNt = (uword)(q+1);
         if ((! Nt [q].m0) ||
             (! Nt [q].m1) || (Nt [q].m1 > 127) ||
             (! Nt [q].m2) || (Nt [q].m2 > 127))
            {NNt = (uword)q;   break;}
      }
   }
   else NNt = 0;
/* for (i = 0;  i < NNt;  i++) {
**    TmS (ts, Nt [i].time * M_WHOLE / BB_WHOLE);
**    MKey2Str (ts2, Nt [i].m1);
**    FO.Put (StrFmt (Line, "`s `s_`d", ts, ts2, Nt [i].m2));
**    TmS (ts, (Nt [i].time+Nt [i].dur) * M_WHOLE / BB_WHOLE - 1);
**    FO.Put (StrFmt (Line, "`s `s^64", ts, ts2));
** }
*/
// OKAY WE'RE DONE LOADIN WHEW.  now write it out gooder as .song

   MelOut = 0;   NNtX = 0;
   if (NNt) {                          // count expanded notes :/
      tm  = NNt?(M_WHOLE*2):0;         // scoot past melody intro bars
      for    (i = 0;                i < (ChorusBgn-1)*4;  i++)
            {PutMel (tm, i);   tm += (M_WHOLE / 4);}
      for (r = 0;  r < ChorusRep;  r++)
         for (i = (ChorusBgn-1)*4;  i <  ChorusEnd   *4;  i++)
            {PutMel (tm, i);   tm += (M_WHOLE / 4);}
      for    (i =  ChorusEnd   *4;  i <= CodaEnd;         i++)
            {PutMel (tm, i);   tm += (M_WHOLE / 4);}
   }

   FO.Put (StrFmt (Line,
      "source=`s\r\n"
      "Track:\r\n"
      ".  Piano\\AcousticGrand  `d 0 .Melody\r\n"
      ".  Drum\\Drum            3 0 .DrumTrack\r\n"
      "Control:\r\n"  "TSig\r\n"  "Tmpo\r\n"  "KSig\r\n"
      "Lyric:\r\n",
      & fni [FNR], NNtX
   ));

   tm  = NNt?(M_WHOLE*2):0;            // scoot past melody intro bars
   dur = (ChorusEnd - ChorusBgn + 1) * M_WHOLE/4 - 1;
   MelOut = 1;

// lyrics n chords first - each beat of intro;  chorus(incl reps);  coda
   *TSty = '\0';
   for    (i = 0;                i < (ChorusBgn-1)*4;  i++)
         {PutLyr (tm, i);   PutChd (tm, i);   tm += (M_WHOLE / 4);}
   for (r = 0;  r < ChorusRep;  r++) {
      TmS (ts, tm);   ts [6] = '\0';   // chorus marker w dur cues
      FO.Put (StrFmt (Line, "`s $$./`d(c\r\n", ts, dur));
      for (i = (ChorusBgn-1)*4;  i <  ChorusEnd   *4;  i++)
         {PutLyr (tm, i);   PutChd (tm, i);   tm += (M_WHOLE / 4);}
   }
   for    (i =  ChorusEnd   *4;  i <= CodaEnd;         i++)
         {PutLyr (tm, i);   PutChd (tm, i);   tm += (M_WHOLE / 4);}

   FO.Put ("Event:\r\n");
   tm  = NNt?(M_WHOLE*2):0;            // scoot past melody intro bars
   for    (i = 0;                i < (ChorusBgn-1)*4;  i++)
         {PutMel (tm, i);   tm += (M_WHOLE / 4);}
   for (r = 0;  r < ChorusRep;  r++)
      for (i = (ChorusBgn-1)*4;  i <  ChorusEnd   *4;  i++)
         {PutMel (tm, i);   tm += (M_WHOLE / 4);}
   for    (i =  ChorusEnd   *4;  i <= CodaEnd;         i++)
         {PutMel (tm, i);   tm += (M_WHOLE / 4);}

// drum track tsig,tmpo,ksig
   FO.Put (StrFmt (Line, "0001 cc0 `d `d\r\n", tsN, (tsD==4)?2:3));  // tsig
   FO.Put (StrFmt (Line, "0001 cc1 `d\r\n", bpm));                   // tmpo

/* key 1..17 maj, 18-34 min
** valu=key as num, val2=80flat+1minr
*/
   StrCp (ts, KeyStr [Key]);
   j = 0;
   if (Key >= 18)  {
      j++;   StrAp (ts, "", 1);
      if (Flm [Key] == 'b')  j += 128;
   }
   else {
      if (FlM [Key] == 'b')  j += 128;
   }
   FO.Put (StrFmt (Line, "0001 cc2 `d `d\r\n", MNt2Int (ts), j));   // ksig

   FO.Shut ();
}


//------------------------------------------------------------------------------
bool DoDir (void *ptr, char dfx, char *fn)
// kill any existing .song/dirs and find all remaining files and put em in t
{ StrArr *t = (StrArr *)ptr;
  ulong ln;
   ln = StrLn (fn);
   if (dfx == 'f') {
      if ((ln > 5) && (! StrCm (& fn [ln-5], ".song")))  FO.Kill (fn);
      else                                               t->Add  (fn);
   }
   if (dfx == 'd')  if (StrLn (fn) > FNR)  FO.PathKill (fn);
   return false;
}

int Go ()
{ TStr dir;
  File f;
  StrArr t ("chordz", 4000, 4000*sizeof (TStr)/2);
   InitCom ();

   App.Path (dir, 'd');   StrAp (dir, "\\4_queue\\Chord");
   FNR = StrLn (dir) + 1;

   f.DoDir (dir, & t, DoDir);   t.Sort ();
   for (ulong i = 0;  i < t.num;  i++)  DoFN (t.str [i]);

   QuitCom ();   return 0;
}
