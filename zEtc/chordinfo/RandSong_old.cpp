// RandSong.cpp - build some random, yet "music theory ok" chord progressions
// random arp arrangements on top of chords

#include <os.h>
#include <MidiIO.h>


// all chords defined as strs then set of offsets
typedef struct {char *name, step [8];} ChdDef;
ChdDef Chd [] = {
   {"",      {0,4,7         ,99}},     // (major)
   {"m",     {0,3,7         ,99}},
   {"dim",   {0,3,6         ,99}},
   {"aug",   {0,4,8         ,99}},
   {"sus2",  {0,2,7         ,99}},
   {"sus4",  {0,5,7         ,99}},
   {"2",     {0,4,7,2       ,99}},
   {"4",     {0,4,7,5       ,99}},
   {"m2",    {0,2,3,7       ,99}},
   {"6",     {0,4,7,9       ,99}},
   {"m6",    {0,3,7,9       ,99}},
   {"7",     {0,4,7,10      ,99}},
   {"M7",    {0,4,7,11      ,99}},
   {"m7",    {0,3,7,10      ,99}},
   {"mM7",   {0,3,7,11      ,99}},
   {"7sus4", {0,5,7,10      ,99}},
   {"7b5",   {0,4,6,10      ,99}},
   {"7#5",   {0,4,8,10      ,99}},
   {"dim7",  {0,3,6,9       ,99}},
   {"hDim7", {0,3,6,10      ,99}},     // =m7b5=many slashes
   {"9",     {0,4,7,10,2    ,99}},
   {"M9",    {0,4,7,11,2    ,99}},
   {"m9",    {0,3,7,10,2    ,99}},
   {"mM9",   {0,3,7,11,2    ,99}},
   {"9sus4", {0,5,7,10,2    ,99}},
   {"b9",    {0,4,7,10,1    ,99}},
   {"11",    {0,4,7,10,2,5  ,99}},     // maybe these 2 shouldn't be used...?
   {"13",    {0,4,7,10,2,5,9,99}},     // 7 note chord - yikes :O
   {"1/5",   {0,5,9         ,99}},
   {"1/3",   {0,3,8         ,99}},
   {"5/1",   {0,2,5,9       ,99}},
   {"m2/4",  {0,4,9         ,99}},
   {"m3/5",  {0,2,5,10      ,99}}
};
ubyte NChd = BITS (Chd);


// all progressions defined relative to key of c
char *Pr1 [] = {                       // to any of these from tonic
   "5 f",                              // multiple possibs first
   "6 d m",
   "2 g",
   "2 e m",
   "2 a m",
   "2 e 1/3",                          // C/E = E 1/3
   "3 f# hDim7",
   "2 e hDim7",
   "2 c m6",

   "1 a",                              // single poss
   "1 b",
   "1 d",
   "1 e",
   "1 g# 7",
   "1 a# 9",
   "1 g 1/5",                          // C/G = G 1/5
   "1 d 1/5",                          // G/D = D 5/2 = D 1/5
   "1 g#",
   "1 c# dim7",
   "1 d# dim7",
   "1 g# dim7",
   "1 a hDim7",
   "1 b hDim7",

   "1 a#",
   "1 f m7",
   "1 c# 7",
   "1 g m",
   "1 c sus4",                         // F/C=C sus4
   "1 c 5/1"                           // G/C=C 5/1
};
ubyte NPr1 = BITS (Pr1);

char *Prg [] = {
   "f>d m",       "f>g",         "f>g 1/5",    "f>e 1/3",    "f>c",
   "d m>g",       "d m>e m",     "d m>g 1/5",  "d m>e 1/3",  "d m>f m7",
                                                                     "d m>c# 7",
   "g>e m",       "g>c",
   "e m>f",       "e m>a m",
   "a m>f",       "a m>d m",
   "e 1/3>f",     "e 1/3>d m",
   "f# hDim7>g",  "f# hDim7>b",  "f# hDim7>g 1/5",
   "e hDim7>f",   "e hDim7>a",
   "c m6>d",      "c m6>d 1/5",

   "a>d m",                            // from here on, only 1 spot possible
   "b>e m",
   "d>g",
   "e>a m",
   "g# 7>g 1/5",
   "a# 9>g 1/5",
   "g 1/5>g",
   "d 1/5>d",
   "g#>a#",
   "c# dim7>d m",
   "d# dim7>e m",
   "g# dim7>a m",
   "a hDim7>d",
   "b hDim7>e",

   "a#>c",                             // aaaand back to tonic
   "f m7>c",
   "c# 7>c",
   "g m>c",
   "c sus4>c",
   "c 5/1>c"
};
ubyte NPrg = BITS (Prg);


// equivalant chords for those with em
char *Eq1 [] = {
   "6 c",
   "8 f",
   "6 d m",
   "8 g",
   "3 e m",
   "5 a m",
   "7 a",
   "7 b",
   "7 d",
   "7 e",
   "1 a#",
   "1 g m"
};
ubyte NEq1 = BITS (Eq1);

char *Equ [] = {
   "c>c 2",     "c>c 6",       "c>c M7",    "c>c M9",      "c>c sus4",
                                                           "c>e 1/3",
   "f>f 2",     "f>f 6",       "f>f M7",    "f>f m",       "f>f m6",
                               "f>f M9",    "f>f sus4",    "f>a 1/3",
   "d m>d m2",  "d m>d m6",    "d m>d m7",  "d m>d m9",    "d m>d sus4",
                                                           "d m>f m2/4",
   "g>g 2",     "g>g 6",       "g>g 7",     "g>g 9",       "g>g 11",
                               "g>g 13",    "g>g sus4",    "g>b 1/3",
   "e m>e m7",  "e m>e sus4",  "e m>g m3/5",
   "a m>a m2",  "a m>a m7",    "a m>a m9",  "a m>a sus4",  "a m>c m2/4",
   "a>a 2",     "a>a 7",       "a>a 9",     "a>a b9",      "a>a 11",
                                            "a>a 7sus4",   "a>c# 1/3",
   "b>b 2",     "b>b 7",       "b>b 9",     "b>b b9",      "b>b 11",
                                            "b>b 7sus4",   "b>d# 1/3",
   "d>d 2",     "d>d 7",       "d>d 9",     "d>d b9",      "d>d 11",
                                            "d>d 7sus4",   "d>f# 1/3",
   "e>e 2",     "e>e 7",       "e>e 9",     "e>e b9",      "e>e 11",
                                            "e>e 7sus4",   "e>g# 1/3",
   "a#>a# 9",
   "g m>g 7"
};
ubyte NEqu = BITS (Equ);


//------------------------------------------------------------------------------
const ubyte Split = M_NT(M_F,3);       // min RH note to use for chords w oct/ch

ulong Time;                            // start time in #qnotes
ubyte NBt;                             // #qnotes (beats) for this chord
TStr  Dur,                             // string repr of NBt ending w spc
      Ch;                              // chord string we currently got
ubyte Bass, Qual;

typedef char ProgDef [58][16];         // 14bars*4qnotes=56 strs max (pad to 58)
                                       // 8Cmin  (NBt=8 root=c qual=min)
uword  NProg;
ProgDef Prog;

const uword    MAXPROG = 256;
uword  NDB [6];
ProgDef DB [6][MAXPROG];               // [6 bar lengths][256 progressions]
                                       // ^4,6,8,10,12,14   ^sorted by #changes
File RH, LH, SO;


//------------------------------------------------------------------------------
ubyte Rnd (ubyte n)                    // return 0..n-1
{ ulong r  = Rand ();
  ubyte rc = (ubyte)(((r?(r-1):0) * n) / RAND_MAX);
   return rc;
}


char *Dup (TStr s, char *dur, ubyte ne)     // dup a dur char ne times into s
{  *s = '\0';   while (ne--)  StrAp (s, dur);   return s;  }


void NxtDur ()
// bump Time by last NBt;   set NBt from 1-8;  set Dur to qnote dur str n space
// if next bar is 3,5,7,etc chop dur so next chord hits on beat 1
{  Time += NBt;
   NBt = 1 + Rnd (6);                  // new NBt = 1..6 beats (q notes)
  ulong maxT = (Time / 8 * 8) + 8;
   if (Time + NBt > maxT)              // so every odd bar hits on the 1
      NBt = (ubyte)(maxT - Time);
   Dup (Dur, "q", NBt);   StrAp (Dur, " ");
//DBG("`04d.`d `s`s\n", 1+Time/4, 1+(Time%4), Dur, Ch);
}


ubyte Nt (char *s, char *x = NULL)     // bass note string => number (0-11)
{ TStr ts;                             // x gets stuff following it
   StrCp (ts, s);
   if (ts [1] == '#')  {if (x)  StrCp (x, & ts [2]);   ts [2] = '\0';}
   else                {if (x)  StrCp (x, & ts [1]);   ts [1] = '\0';}
   for (ubyte i = 0;  i < 12;  i++)  if (! StrCm (MKeyStr [i], ts))  return i;
   Die ("Nt got nothin", s);   return 0;
}


ubyte BsMx ()
{ ubyte b = Split-1;   while ((b % 12) != Bass) b--;   return b;  }


ubyte ChdNt (ubyte *nt)
{ ubyte n = 0, b, i, st;
   for (b = Split;  b < Split+12;  b++)
      for (i = 0;  (st = Chd [Qual].step [i]) != 99;  i++)
         if ((b % 12) == ((Bass+st) % 12))  {nt [n++] = b;   break;}
   return n;
}


//------------------------------------------------------------------------------
void AddChd ()
// put `d`s NBt,Cchd into Prog[NProg++]
{ TStr  ts;
  char *s;
   NxtDur ();
   if (NProg >= BITS (Prog))  Die ("Prog ain't big enough");
   StrCp (ts, Ch);   ts [0] = CHUP (ts [0]);
   if (s = StrCh (ts, ' '))  StrCp (s, s+1);
   StrFmt (Prog [NProg++], "`d`s", NBt, ts);
}

ubyte CalcProg ()
// build a randomized chord progression (or at least try)
{ ubyte i, j, r;
  uword x;
  ulong td;
  TStr  tCh, ts;
  char *p;
   Time = 0;   NBt = 0;   NProg = 0;

// start on tonic major
   StrCp (Ch, "c");   AddChd ();   td = NBt;     // td sums NBt for whole prog

// do some chord progressions (seq of chords back to tonic)
   for (;;) {
      if (NProg == 1)                  // from tonic to ANYthing (in Pr1)
         StrCp (Ch, & Pr1 [Rnd (NPr1)][2]);
      else {                           // rand next chord in progression
         for (i = 0;  i < NPr1;  i++)  if (! StrCm (Ch, & Pr1 [i][2]))  break;
         if (i >= NPr1)  Die ("Couldn't find chord in Pr1", Ch);
         r = Rnd ((ubyte)Str2Int (Pr1 [i]));
         for (i = 0;  i < NPrg;  i++) {
            StrCp (ts, Prg [i]);   p = StrCh (ts, '>');   *p = '\0';
            if (! StrCm (Ch, ts))  break;
         }
         StrCp (ts, Prg [i+r]);   p = StrCh (ts, '>');   p++;
         StrCp (Ch, p);
//DBG("r=`d i=`d =>`s  `s", r, i, Prg [i+r], Ch);
      }

   // back to tonic?  DONE w progression.  (skip saving this known end tonic)
      if (! StrCm (Ch, "c"))  {
//DBG("HIT TONIC");
         break;
      }

   // 1/4 of time, pick rand equivalent chord into Ch;  restore Ch w tCh l8r
      StrCp (tCh, Ch);
      if (! Rnd (4))
         for (i = 0;  i < NEq1;  i++)  if (! StrCm (Ch, & Eq1 [i][2])) {
            r = Rnd ((ubyte)Str2Int (Eq1 [i]));       // DING DING DING !!
            for (j = 0;  j < NEqu;  j++) {
               StrCp (ts, Equ [j]);   p = StrCh (ts, '>');   *p = '\0';
               if (! StrCm (Ch, ts)) {
                  StrCp (ts, Equ [j+r]);   p = StrCh (ts, '>');   p++;
                  StrCp (Ch, p);
                  break;
               }
            }
            break;
         }
      AddChd ();   td += NBt;
      StrCp (Ch, tCh);

      if (td > 4*14) {
//DBG("NOPE  >14 bars");
         NProg = 0;   return 0;
      }
   }

   if (r = td % 8) {                   // try to stretch it out to 2 bar mult
      r = 8 - r;
      i = 1;
      while (i && r) {
         i = 0;
         for (x = NProg;  x;  x--) {
            if (Prog [x-1][0] < '8')
               {Prog [x-1][0]++;   i = 1;   td++;   if (! --r) break;}
         }
      }
      if (r) {
//DBG("NOPE  can't stretch to 2 bar multiple");
         NProg = 0;   return 0;
      }
   }

   if (td < 4*4) {
//DBG("NOPE  <4 bars");
      NProg = 0;   return 0;
   }

   for (i = r = 0;  i < NProg;  i++)  if (Prog [i][0] >= '7')  r++;
   if (r >= 2) {
//DBG("NOPE  2+ chords with dur=7");
      NProg = 0;   return 0;
   }
/*
DBG("____________________");
for (uword i = 0;  i < NProg;  i++)  DBG(Prog [i]);
DBG("TotalBars=`d", td/4);
*/
   return (ubyte)(td/4);
}


//------------------------------------------------------------------------------
char *ArrLH [][2] = {                  // lh 1,2,3,5 from bass below compact chd
   {"chd",  "*"           },           // chord for full dur
   {"chR",  "[*"          },           // chord on beat
   {"oct",  ".11"         },           // octave
   {"arp",  ".1 {.5 1"    },           // -1 [-5 1]
   {"arp2", ".1 .5 1 2 3" }            // -1 -5 1 2 3-
};

char *ArrRH [][2] = {                  // rh 1,3,5 from compacted chord
   {"chd",  "*"         },
   {"chR",  "[*"        },
   {"2n1",  "{*b 1"     },             // rip out bottom note (and limit to 4)
   {"3n1",  "{.*4 5"    },             // just limit to 4
   {"5131", ".5 {.1 .3" },             // 5 [1 3]
   {"5353", "{.5 .3"    },             // [5 3]
   {"arpD", ".5 {.3 .1" }              // 5 [3 1]
};


void Arr (char hn, char *ar)
{ TStr  st [80], ts, stp, n, d, s, os;
  char *p;
  ubyte pst, nst = 0, rTyp = 0, rPos = 0,
             nnt, nt [12], b, sb, oct, bs, thr, k;
   StrCp (ts, ar);

// calc st[] till ts is '';  set repeat type: 0=norep, 1=stepBeat, 2=stepRhythm
   while (*ts) {
      if      (ts [0] == '[')  {rTyp = 1;   rPos = nst;   StrCp (ts, & ts [1]);}
      else if (ts [0] == '{')  {rTyp = 2;   rPos = nst;   StrCp (ts, & ts [1]);}
      else {
         if (nst == BITS (st))  Die ("outa st[] slots");
         StrCp (st [nst], ts);
         if (p = StrCh (ts, ' '))
               {st [nst][(ubyte)(p - ts)] = '\0';   StrCp (ts, p+1);}
         else  *ts = '\0';
         nst++;
      }
   }

// get basic chord notes and bass note
   nnt = ChdNt (nt);
   n [0] = '\0';
   for (b = 0;  (b < nnt) && (b < 5);  b++)  // ya only got 5 fingers/hand...
      StrAp (n, MKey2Str (ts, nt [b]));
   bs = BsMx ();   thr = 4;            // default to major 3rd
   if (StrCh (ar, '3')) {              // may need to scoot bs
      thr = Chd [Qual].step [1];
      if ((bs+thr) >= Split)  bs -= 12;
   }

// arrange our arp,etc
   for (pst = 0, sb = 0;  sb < NBt*2;  sb++) {
   // get stp n pull 1st oct outa it
      StrCp (stp, st [pst]);
      oct = 0;   while (stp [0] == '.')  {StrCp (stp, & stp [1]);   oct++;}

   // get our step's notes into s
      *s = '\0';
      if (hn == 'l') {
         if (stp [0] == '*')  StrCp (s, n);  // no t,b for lh
         else
            do {
               if (stp [0] == '1')  k = bs;
               if (stp [0] == '2')  k = bs+2;
               if (stp [0] == '3')  k = bs+thr;
               if (stp [0] == '5')  k = bs+7;
               StrAp (s, MKey2Str (ts, k-oct*12));
               StrCp (stp, & stp [1]);
               oct = 0;
               while (stp [0] == '.')  {StrCp (stp, & stp [1]);   oct++;}
            } while (stp [0]);
      }
      else {
         if (stp [0] == '*') {
            StrCp (s, n);
            if      (stp [1] == 'b')
               for (*s = '\0', b = 1;  (b < nnt) && (b < 5);  b++)
                  StrAp (s, MKey2Str (ts, nt [b] + oct*12));
            else if (stp [1] == '4')
               for (*s = '\0', b = 0;  (b < nnt) && (b < 4);  b++)
                  StrAp (s, MKey2Str (ts, nt [b] + oct*12));
         }
         else
            do {
               if (stp [0] == '1')  k = nt [0];
               if (stp [0] == '3')  k = nt [1];
               if (stp [0] == '5')  k = nt [2];
               StrAp (s, MKey2Str (ts, k+oct*12));
               StrCp (stp, & stp [1]);
               oct = 0;
               while (stp [0] == '.')  {StrCp (stp, & stp [1]);   oct++;}
            } while (stp [0]);
      }

   // pick our dur now;  bump pst;  Put it to file
      if      (rTyp == 0) {
         StrCp (d, "e");
         if (pst >= (nst-1))  {Dup (d, "e", NBt*2-sb);   sb = NBt*2;}
      }
      else if (rTyp == 1) {StrCp (d, "q");   sb++;}
      else                 StrCp (d, "e");

      if (++pst >= nst)  pst = rPos;

      StrFmt (os, "`s `s\r\n", d, s);
      if (hn == 'l')  LH.Put (os);   else RH.Put (os);
   }
}


void Put (ubyte bp, uword p)
// put a chord progession into an arrangement
{ TStr  ts;
  char  buf [8000];
  ubyte l, r, c;
DBG("Put bp=`d p=`d/`d", bp, p, NDB [0]);
   for (c = 0;     DB [bp][p][c][0];  c++) {
   // get beats into NBt,Dur(q str)
      NBt =        DB [bp][p][c][0] - '0';   Dup (Dur, "q", NBt);

   // get Ch,Bass,Qual
      StrCp (Ch, & DB [bp][p][c][1]);
      Bass = Nt (Ch, ts);              // bass=0-11, ts gets stuff after bass
      for (Qual = 0;  Qual < NChd;  Qual++)
         if (! StrCm (ts, Chd [Qual].name, 'x'))  break;
      if (Qual >= NChd)  Die ("couldn't find chord", Ch);

   // pick random arrangement, make sure lh doesn't state chord if rh does
      r     = Rnd (BITS (ArrRH));
      do  l = Rnd (BITS (ArrLH));   while ((r < 4) && (l < 2));
                                       // * arr can only be in one hand
      StrFmt (buf, "-- `04d.`d `d  `s `s `s\r\n",
         1+Time/4, 1+(Time%4), NBt, Ch, ArrLH [l][0], ArrRH [r][0]);
      RH.Put (buf);   LH.Put (buf);
      LH.Put (StrFmt (buf, "!text=*`s\r\n", Ch));

      Arr ('l', ArrLH [l][1]);   Arr ('r', ArrRH [r][1]);

      Time += NBt;                     // bump the ooole time
   }
}


void Home ()                           // put final long tonic chord
{ char buf [8000];
   NBt = 4;   StrCp (Dur, "w ");       // get beats into NBt,Dur
   StrCp (Ch, "C");   Bass = 0;   Qual = 0;

   StrFmt (buf, "-- `04d.`d `d  `s oct chd\r\n", 1+Time/4, 1+(Time%4), NBt, Ch);
   RH.Put (buf);   LH.Put (buf);
   LH.Put (StrFmt (buf, "!text=*`s\r\n", Ch));

   Arr ('l', ArrLH [2][1]);   Arr ('r', ArrRH [0][1]);

   Time += NBt;
}


//------------------------------------------------------------------------------
int Go ()
{ TStr  que, fn, pr, ts;
  ubyte nb, bp, j;
  uword i, pos, miss = 0;
  int   cm;
  bool  dun, ins;
  char  op [8000];
DBG("{ RandSong::Go");
   RandInit ();
   for (;;) {
      if (nb = CalcProg ()) {
         bp = nb/2 - 2;                // num bars into 0..5 index of DB
         if (NDB [bp] < MAXPROG) {     // keep addin em till 256 progressions
         // if room n non dup, ins to DB[bp][0..255] sorted by NProg
            Prog [NProg][0] = '\0';   ins = true;   dun = false;
            for (pos = 0;  pos < NDB [bp];  pos++) {
               for (i = 0;  DB [bp][pos][i][0];  i++)  ;   // get #chds in pos
               if (NProg <  i)  break;           // new guy (fewest #chds)
               if (NProg == i)
                  for (j = 0;  j <= NProg;  j++) {
//DBG("dun=`b ins=`b pos=`d/`d j=`d DB=`s PR=`s",
//dun, ins, pos, NDB[bp], j, DB [bp][pos][j], Prog [j]);
                  // dup so no ins n done
                     if (! Prog [j][0])  {ins = false;   dun = true;   break;}

                  // if not same, if this is pos - ins it - else after here
                     cm = StrCm (& Prog [j][1], & DB [bp][pos][j][1]);
                     if      (cm < 0)                   {dun = true;   break;}
                     else if (cm > 0)                                  break;
                  }
//DBG("a dun=`s ins=`s", dun?"t":"f", ins?"t":"f");
               if (dun)  break;
            }
            if (ins) {
//DBG("ins bp=`d pos=`d/`d", bp, pos, NDB [bp]);
               RecIns (DB [bp], ++NDB [bp], sizeof (ProgDef), pos);
               for (j = 0;  j < NProg;  j++)  StrCp (DB [bp][pos][j], Prog [j]);
               DB [bp][pos][NProg][0] = '\0';
               miss = 0;
            }
         }
      }
      if (++miss == 512)  break;       // just ez way to see if 256 of all durs
   }

// dump our db of progressions
   for (bp = 0;  bp < 6;  bp++)
      for (i = 0;  i < NDB [bp];  i++) {
         for (j = 0;  (j < 56) && DB [bp][i][j][0];  j++)  ;
         StrFmt (op, "`>2d `>2d", (2+bp)*2, j);
         for (j = 0;  (j < 56) && DB [bp][i][j][0];  j++)
            {StrAp (op, " ");   StrAp (op, DB [bp][i][j]);}
DBG(op);
      }

// open up rh,lh,song txt files
   App.Path (que, 'd');   StrAp (que, "\\4_queue\\rand_song");
   SO.PathMake (que);
   StrCp (fn, que);   StrAp (fn, "\\rh.txt");
   if (! RH.Open (fn, "w"))  Die ("can't write rh file", fn);
   StrCp (fn, que);   StrAp (fn, "\\lh.txt");
   if (! LH.Open (fn, "w"))  Die ("can't write lh file", fn);
   StrCp (fn, que);   StrAp (fn, "\\rand_song.txt");
   if (! SO.Open (fn, "w"))  Die ("can't write song file", fn);

// turn progression into a random arrangement for preview
   Time = 0;
   for (i = NDB [0];  i && (i >= NDB [0]-50);  i--)  Put (0, i-1);
   Home ();                            // finish it all off w final tonic

// make que\rand_song.txt with keysig,drum reps
   SO.Put (StrFmt (ts,
"-- rand_song.txt\r\n"
"!KSig=C maj #\r\n"
"$cdefgab-\r\n"
"!name=?rh\r\n"
"#rh\r\n"
"NextTrack\r\n"
"!name=?lh\r\n"
"#lh\r\n"
"NextTrack\r\n"
"!Tmpo=90\r\n"
"!sound=Drum\\Drum\r\n"
"#Drum\\main\\std16 `d\r\n",
      Time/4+1
   ));
   RH.Shut ();   LH.Shut ();   SO.Shut ();

   App.Path (pr);                      // txt2song+song2mid
   StrFmt (fn, "`s\\txt2song.exe r `s\\rand_song.txt", pr, que);   RunWait (fn);
   StrFmt (fn, "`s\\song2mid.exe r `s\\rand_song.mid", pr, que);   RunWait (fn);

DBG("} RandSong::Go");
   return 0;
}
