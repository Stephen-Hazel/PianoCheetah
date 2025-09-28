// sChd.cpp - auto calc chords and stick em in _f.chd

#include "song.h"

const ubyt4 SKIP = 0xFFFFFFFF;         // this .time means Set[] is continuation

struct SetDef {ubyt4 time, nmap [4], scor;};
static SetDef Set [256*1024];                 // enough?
static ubyt4 NSet;

// don't use MChd[] cuz it makes stuff crazy - use LESS
MChdDef MChdX [] = {            // all need >= 4 defined so oct,5,off are weird
// {"oct",   {0,'x','x','x'}, 'n', 30, "1+8 unison"},
// {"5",     {0,7,  'x','x'}, 'n', 31, "1+5"},   // only guy missing 3rd
   {"",      {0,4,7,    'x'}, 'y',  0, "maj"},   // WITH later stuff
   {"m",     {0,3,7,    'x'}, 'y',  8, "min -"}
// {"7",     {0,4,7,10, 'x'}, 'y', 19, "dom7"},
// {"M7",    {0,4,7,11, 'x'}, 'y',  2, "Maj7"},
// {"m7",    {0,3,7,10, 'x'}, 'y', 10, "min7"}
};
ubyte NMChdX = BITS (MChdX);


static void SetSet (ubyt4 s [4], ubyte on [128])
{ ubyte l, b;
   MemSet (s, 0, 16);                  // 16 bytes => 128 bits
   for (l = 0; l < 4; l++)  for (b = 0; b < 32; b++)
                               if (on [l*32+b])  s [l] |= (0x80000000 >> b);
}


struct TryRow {ubyte root, tmpl, bass, hit [12];
               ubyt4 pos, neg, mis, sc;};
TryRow Try [256*1024];
ubyt4 NTry;

int TryCmp (void *p1, void *p2)
{ TryRow *t1 = (TryRow *)p1, *t2 = (TryRow *)p2;
  int t;
   if ((t = t2->sc      - t1->sc  ))     return t;
   if ((t = t2->hit [0] - t1->hit [0]))  return t;
   if (( (t1->hit [1] != 'x') && (t1->hit [2] != 'x') &&
         (t2->hit [1] != 'x') && (t2->hit [2] != 'x') ))
   if ((t = t2->hit [2] - t1->hit [2]))  return t;
   return   t1->tmpl    - t2->tmpl;
}


static ubyt4 Score (ubyt4 p, ubyt4 ln, char *lbl = NULL, bool fl = false,
                                       char *otmp = NULL, ubyte *orr = NULL)
// score possible chord root.template/bass for these sets of notedowns
{ ubyt4 nmap, pos, neg, s, n,              rc = 0;
  ubyte t, r, b, i, sh, note, step, mis,   rr, rt;
  char  tmp [sizeof (MChdX [0].tmp)],  hit [sizeof (MChdX [0].tmp)],
       *pt, *ph, c;
  bool  in;
   NTry = 0;
//TStr ts;
//DBG("{ Score time=`s p=`d ln=`d", TmSt (ts, Set [p].time), p, ln);
   for (t = 0; t < NMChdX; t++)  if (MChdX [t].calc == 'y') {
      MemCp (tmp, MChdX [t].tmp, sizeof (tmp));
      for (r = 0;  r < 12;  r++) {     // step thru each root for tmp
//DBG("`s `s", MKeyStr [r], MChdX [t].lbl);
      // init score params;  (misses counted from hit[] later)
         b = 255;   pos = neg = 0;   MemSet (hit, 0, sizeof (hit));

      // sum notes in each Set into pos or neg
         for (s = p, n = ln;  n;  s++, n--) {    // step s thru each set
            for (note = i = 0;  i < 4;  i++) {
               for (nmap = Set [s].nmap [i], sh = 32;  sh;      // note=> 0..127
                    nmap <<= 1, note++,      sh--) {
                  if (nmap & 0x80000000) {       // set HAS note!
                     if (b == 255)  b = note;    // 1st set's lowest note
                     step = note % 12;           // strip octave
                     for (in = false, pt = tmp, ph = hit;  *pt != 'x';
                                      pt++,     ph++)
                        if (*pt == step)  {*ph = 1;   in = true;   break;}
                     if (in) pos++;   else neg++;
                  }
               }
            }
//DBG("   s=`d/`d time=`s   pos=`d neg=`d",
//s-p, ln, TmSt(ts, Set [s].time),   pos, neg);
         }

      // count template misses, check if bass=root
         for (mis = 0, pt = tmp, ph = hit;  *pt != 'x';  pt++, ph++)
            if (*ph == 0)  mis++;

//DBG("   bass=`s mis=`d => score=`d",
//MKey2Str(ts, b), mis, pos-neg-mis);
      // got a score worth beans?  notesIN-notesOUT+basses-notesMISSING
         if (pos > (neg + mis)) {
            Try [NTry].root = r;
            Try [NTry].tmpl = t;
            Try [NTry].bass = b;
            MemCp (Try [NTry].hit, hit, sizeof (MChdX[0].tmp));
            Try [NTry].pos = pos;
            Try [NTry].neg = neg;
            Try [NTry].mis = mis;
            Try [NTry].sc  = pos - neg - mis;
            NTry++;
         }

      // adj template to next root note (inc each one & modulo 12)
         for (pt = tmp; *pt != 'x'; pt++)
            {c = *pt;   if (++c >= 12) c = 0;   *pt = c;}
      }
   }
   Sort (Try, NTry, sizeof (Try[0]), TryCmp);
   rc = Try [0].sc;   rr = Try [0].root;   rt = Try [0].tmpl;
//DBG("FINAL `s `s sc=`d", MKeyStr [rr], MChdX [rt].lbl, rc);
//for (n = 0;  n < 5;  n++) {
//   if (n >= NTry)  break;
//DBG("   `s.`s/`s sc=`d pos=`d neg=`d mis=`d nr=`d",
//MKeyStr [Try [n].root], MChdX [Try [n].tmpl].lbl, MKey2Str(ts,Try [n].bass),
//Try [n].sc, Try [n].pos, Try [n].neg, Try [n].mis, Try [n].hit [0]);
//}
   if (lbl) {
      if (rc) {
           MemCp (otmp, MChdX [rt].tmp, sizeof (tmp));   *orr = rr;
           for (i = 0;  otmp [i] != 'x';  i++)  otmp [i] = (otmp [i] + rr) % 12;
           StrFmt (lbl, "`s`s",
                   fl ? MKeyStrB [rr] : MKeyStr [rr],  MChdX [rt].lbl);
           *lbl = CHUP (*lbl);
//         b = Try [0].bass % 12;
//         n = StrLn (lbl);
//         if (b != Try [0].root) {
//            StrFmt (& lbl [n], "/`s", fl ? MKeyStrB [b] : MKeyStr [b]);
//            lbl [n+1] = CHUP (lbl [n+1]);
//         }
      }
      else *lbl = '\0';
   }
//DBG("} Score");
   return rc;
}


//______________________________________________________________________________
// qualities of Prg/Equ chord progressions
// normal   m  1/3  hDim7  m6  7  9  1/5  dim7  m7  sus  5/1
// equiv    2  6  M7  M9  m2  m9  m2/4  11  13  m3/5  7sus
// all      2  6  7  9  11  13  m  m2  m6  m7  m9  M7  M9
//          sus  7sus  1/3  1/5  5/1  m2/4  m3/5  dim7  hDim7
// 23 used versus 39+ in MChd
// all regularish progressions defined relative to key of c

static WStr Prg [][2] = {
// home can go to tons of em (start w ones w mult possibs after)
   {"C","F"}, {"C","Dm"}, {"C","G"}, {"C","Em"}, {"C","Am"},
   {"C","E1\\3"},  // C/E = E1/3
   {"C","F#hDim7"}, {"C","EhDim7"}, {"C","Cm6"},

// home to single possibs after
   {"C","A"}, {"C","B"}, {"C","D"}, {"C","E"}, {"C","G#7"}, {"C","A#9"},
   {"C","G1\\5"}, {"C","D1\\5"},       // C/G = G1\5,  G/D = D5/2 = D1\5
   {"C","G#"}, {"C","C#dim7"}, {"C","D#dim7"}, {"C","G#dim7"},
   {"C","AhDim7"}, {"C","BhDim7"},

// home to single possib of back to home
   {"C","A#"}, {"C","Fm7"}, {"C","C#7"}, {"C","Gm"},
   {"C","Csus"}, {"C","C5\\1"},        // F/C=Csus,  G/C=C5\1

// ok done w home, now nonhome chords that have multiple possibs
   {"F","Dm"},       {"F","G"},   {"F", "G1\\5"}, {"F", "E1\\3"}, {"F","C"},
   {"Dm","G"},       {"Dm","Em"}, {"Dm","G1\\5"}, {"Dm","E1\\3"},
                                  {"Dm","Fm7"},   {"Dm","C#7"},
   {"G","Em"},       {"G","C"},
   {"Em","F"},       {"Em","Am"},
   {"Am","F"},       {"Am","Dm"},
   {"E1\\3","F"},    {"E1\\3","Dm"},
   {"F#hDim7","G"},  {"F#hDim7","B"},  {"F#hDim7","G1\\5"},
   {"EhDim7","F"},   {"EhDim7","A"},
   {"Cm6","D"},      {"Cm6","D1\\5"},

// nonhome to one possib
   {"A","Dm"}, {"B","Em"}, {"D","G"}, {"E","Am"}, {"G#7","G1\\5"},
   {"A#9","G1\\5"}, {"G1\\5","G"}, {"D1\\5","D"},
   {"G#","A#"}, {"C#dim7","Dm"}, {"D#dim7","Em"}, {"G#dim7","Am"},
   {"AhDim7","D"}, {"BhDim7","E"},

// nonhome to always home
   {"A#","C"}, {"Fm7","C"}, {"C#7","C"}, {"Gm","C"}, {"Csus","C"},
   {"C5\\1","C"}
};
static ubyte NPrg = BITS (Prg);

static WStr Equ [][2] = {
   {"C","C2"},    {"C","C6"},     {"C","CM7"},   {"C","CM9"},
                  {"C","Csus"},   {"C","E1\\3"},
   {"F","F2"},    {"F","F6"},     {"F","FM7"},   {"F","Fm"},
                  {"F","Fm6"},    {"F","FM9"},   {"F","Fsus"},   {"F","A1\\3"},
   {"Dm","Dm2"},  {"Dm","Dm6"},   {"Dm","Dm7"},  {"Dm","Dm9"},
                  {"Dm","Dsus"},  {"Dm","Fm2\\4"},
   {"G","G2"},    {"G","G6"},     {"G","G7"},    {"G","G9"},
                  {"G","G11"},    {"G","G13"},   {"G","Gsus"},   {"G","B1\\3"},
   {"Em","Em7"},  {"Em","Esus"},  {"Em","Gm3\\5"},
   {"Am","Am2"},  {"Am","Am7"},   {"Am","Am9"},  {"Am","Asus"}, {"Am","Cm2\\4"},
   {"A","A2"},    {"A","A7"},     {"A","A9"},    {"A","Ab9"},
                  {"A","A11"},    {"A","A7sus"}, {"A","C#1\\3"},
   {"B","B2"},    {"B","B7"},     {"B","B9"},    {"B","Bb9"},
                  {"B","B11"},    {"B","B7sus"}, {"B","D#1\\3"},
   {"D","D2"},    {"D","D7"},     {"D","D9"},    {"D","Db9"},
                  {"D","D11"},    {"D","D7sus"}, {"D","F#1\\3"},
   {"E","E2"},    {"E","E7"},     {"E","E9"},    {"E","Eb9"},
                  {"E","E11"},    {"E","E7sus"}, {"E","G#1\\3"},
   {"A#","A#9"},
   {"Gm","G7"}
};
static ubyte NEqu = BITS (Equ);

static ubyte ChdNxt (WStr **out, char *ch)
// put chords comin after ch into out
{ static WStr ls [100];
  ubyte   i, nls = 0;
   for (i = 0;  i < NPrg;  i++)  if (! StrCm (ch, Prg [i][0], 'x'))
      StrCp (ls [nls++], Prg [i][1]);
   *out = ls;   return nls;
}

static ubyte ChdEqu (WStr **out, char *ch)
// put equiv chords for ch into out
{ static WStr ls [100];
  ubyte   i, nls = 0;
   for (i = 0;  i < NEqu;  i++)  if (! StrCm (ch, Equ [i][0], 'x'))
      StrCp (ls [nls++], Equ [i][1]);
   *out = ls;   return nls;
}

ubyte ChdBtw (TStr **out, char *i1, char *i2)
// list chords fittin between chords i1 and i2 into out
{ ubyte i, j, got, nnx, nls = 0;
  WStr  c1, c2, *nx;
  static TStr ls [100];
TRC("ChdBtw i1=`s i2=`s", i1, i2);
// first, turn equiv chords back to standard-ish chord
   StrCp (c1, i1);
   for (i = 0;  i < NEqu;  i++)  if (! StrCm (c1, Equ [i][1], 'x'))
      {StrCp (c1, Equ [i][0]);   break;}
   StrCp (c2, i2);
   for (i = 0;  i < NEqu;  i++)  if (! StrCm (c2, Equ [i][1], 'x'))
      {StrCp (c2, Equ [i][0]);   break;}
TRC("c1=`s c2=`s", c1, c2);

// get ones following c1 n copy to my ls[] (which is TStr, not WStr)
   nls = ChdNxt (& nx, c1);
   for (i = 0;  i < nls;  i++)  StrCp (ls [i], nx [i]);
TRC("nls=`d", nls);
// if a ls[] chord doesn't go to c2, kill it off
   for (i = 0;  i < nls;) {
//DBG("`d/`d", i, nls);
      nnx = ChdNxt (& nx, ls [i]);
      for (got = j = 0;  j < nnx;  j++)  if (! StrCm (nx [j], c2, 'x'))
                                            {got = 1;   break;}
      if (! got)  MemCp (& ls [i], & ls [i+1], (--nls-i)*sizeof (TStr));
      else        i++;
   }

// get equiv chords for each main one
   for (i = 0;  i < nls;  i++) {
      nnx = ChdEqu (& nx, ls [i]);     // reusin nx[] for equiv lst :/
      for (j = 0;  j < nnx;  j++)
         {StrAp (ls [i], CC(" "));   StrAp (ls [i], nx [j]);}
TRC(ls [i]);
   }
   *out = ls;   return nls;
}

static bool ChdProg (ubyte bars, WStr *prog, ubyt4 maxp)
// build a randomized chord progression for bars that start on C and end just b4
// on rand beats with rand equiv chords
{ ubyte nls, r;
  WStr *ls;
  ubyt2 tm, nbt, tb, ttm;              // time, numbeat, totbeat, testtime
  ubyt4 nprog;
  TStr  ch, tCh;
   nbt = 0;   tm = 0;   nprog = 0;
   StrCp (ch, CC("C"));                // start on tonic major
   tb = nbt = 1 + Rand (5);            // init totalBeats,numBeats to  1-5;
// stamp nbt and ch into prog[nprog++]
   StrFmt (prog [nprog++], "`d`s", nbt, ch);

// calc next chords (seq of chords back to (pre)tonic)
   while (tb <= bars*4) {
      nls = ChdNxt (& ls, ch);
      r = (ubyte)Rand (nls);
      StrCp (ch, ls [r]);
DBG(" Prg=> `s (`d/`d)", ch, r, nls);

   // 1/3 of time, pick rand equivalent chord into Ch;  restore Ch w tCh l8r
      StrCp (tCh, ch);
      if (! Rand (3))  if ((nls = ChdEqu (& ls, ch))) {
         r = (ubyte)Rand (nls);
         StrCp (ch, ls [r]);
DBG("  Equ> `s (`d/`d)", ch, r, nls);
      }

   // set NBt randomly from 1-5
      tm += nbt;   nbt = 1 + Rand (5);
      ttm = (tm / 8 * 8) + 8;          // every odd bar needs chord on beat 1
      if (tm + nbt > ttm)  nbt = ttm - tm;
//DBG("`04d.`d `s\n", 1+tm/4, 1+(tm%4), ch);
      if (nprog >= maxp)  DBG ("prog ain't big enough?");  // ...can't happen

   // stamp nbt and ch into prog[nprog++]
      StrFmt (prog [nprog++], "`d`s", nbt, ch);
      tb += nbt;
      StrCp (ch, tCh);                 // restore chord (cuz coulda equiv'd)
   }
// not ending in tonic ?  return false :(
   if (StrCm (ch, CC("C"))) {
DBG("^ ..........NAW.......... ^");
      nprog = 0;   return false;
   }

   tb -= (prog [--nprog][0] - '0');                        // chop off last C
   if (tb < bars*4)  prog [nprog-1][0] += (tb-bars*4);     // and time to "bars"
   prog [nprog][0] = '\0';
for (ubyt2 i = 0;  prog [i][0];  i++)  DBG("`d `s", i, prog [i]);
   return true;
}


void Song::PopChd (ubyt4 itm)
// add pleasant (boring) chord prog
{ TxtRow m [64];
  ubyte nm, i, in;
  ubyt2 p, bb, be;
  ubyt4 tm, tb, te, j;
  WStr  prog [64*4+2];                 // 6Cm  (qdur=6 root=C qual=minor)
  TStr  s1, s2;
TRC("PopChd tm=`s", TmSt (s1, itm));
   nm = GetSct (m);                    // get all the sections

   if (nm == 0)            {Hey (CC("add some sections, first"));   return;}
   if (itm <  m [0].time)  {Hey (CC("pick a section"));             return;}

// set in  to section we're in
   for (in = 0;  in < nm;  in++)
      if ((itm >= m [in].time) && ((in+1==nm) || (itm < m [in+1].time)))  break;
   tm = m [in].time;
   te = (in+1 < nm) ? m [in+1].time : _tEnd;
   bb =               Tm2Bar (tm);
   be = (in+1 < nm) ? Tm2Bar (te)   : _bEnd;
TRC("tm=`s te=`s bb=`d be=`d", TmSt (s1, tm), TmSt (s2, te), bb, be);
   for (j = 0;  j < _f.chd.Ln;) {      // kill any old chds in time range
      if ((_f.chd [j].time >= tm) && (_f.chd [j].time < te))
            _f.chd.Del (j);
      else  j++;
   }

// make the prog - rand # qbeats/equiv for n bars (home - pre home)
   while (! ChdProg (be-bb, prog, BITS (prog)))  ;

// stamp Prog[] into _f.chd[] for our sections
   for (j = 0;  j < _f.chd.Ln;  j++)   // find our ins spot
      if ((j >= _f.chd.Ln) || (_f.chd [j].time >= tm))  break;
   for (tb = 0, p = 0;  prog [p][0];  p++) {
   // done if outa chd[] or if other scts shorter than picked
      if ( _f.chd.Full () || (tm + tb >= te) )  break;

      _f.chd.Ins (j);               // ins new chd offset from tm by tb
//DBG("ins j=`d tm=`s ch=`s", j, TmSt (s1, tm+tb), & prog [p][1]);
             _f.chd [j].time = tm + tb;
      StrCp (_f.chd [j].s, & prog [p][1]);
      j++;
      tb += ((prog [p][0] - '0') * M_WHOLE/4);
   }
// add end chord of C if none
   for (i = 0;  i < nm;  i++)  if (! StrCm (m [in].s, m [i].s)) {
//DBG("i=`d/`d _tEnd=`d", i, nm, _tEnd);
      if (i+1 < nm)  te = m [i+1].time;   else te = _tEnd;
//DBG("check for end at `d/`s", te, TmSt(s1, te));
      for (j = 0;  j < _f.chd.Ln;  j++)  if (_f.chd [j].time >= te)  break;
//DBG("j=`d/`d", j, _f.chd.Ln);
      if ( ((j >= _f.chd.Ln) || (_f.chd [j].time != te)) &&
                              (! _f.chd.Full ()) ) {
//DBG("   ins'd");
         _f.chd.Ins (j);   _f.chd [j].time = te;
         StrCp (_f.chd [j].s, CC("C"));
      }
   }
   ChdArr (itm);                       // chords => notes n TmHop :)
}


int CmpB (void *p1, void *p2)
{ ubyte b1 = *((ubyte *)p1),  b2 = *((ubyte *)p2);
   return b1 - b2;
}

void Song::ChdArr (ubyt4 itm)
// arrange chord events into tracks of notes by very simple arranging
{ ubyt4 c, tb, te, ftb, fte, tm, j, k;
  TStr  ch, qs, s, s1, s2;
  ubyte nm, t, i, in, root, qual, x, n,  nnt, nt [25];
  TxtRow m [64];
TRC("ChdArr itm=`s", TmSt (s, itm));
// dup chords of section we're in to the other sections
   nm = GetSct (m);                    // get all the sections
   if (nm && (itm >= m [0].time)) {
      for (i = 0;  i < nm;  i++)  if ((itm >= m [i  ].time) && ((i+1 == nm) ||
                                      (itm <  m [i+1].time)))  break;
      in = i;                          // section we're in
TRC("in=`d/`d", in, nm);
      itm = m [in].time;
   // dup it into all the sections we're not in
      for (i = 0;  i < nm;  i++)  if ((i != in) &&
                                      (! StrCm (m [in].s, m [i].s))) {
         tb = m [i].time;   te = (i+1 >= nm) ? _tEnd : m [i+1].time;
TRC(" sc=`d/`d tb=`s te=`s", i, nm, TmSt (s1, tb), TmSt (s2, te));
         for (j = 0;  j < _f.chd.Ln;) {     // kill any old chds in time range
            if ((_f.chd [j].time >= tb) && (_f.chd [j].time < te))
                  _f.chd.Del (j);
            else  j++;
         }
      // find our ins spot.  copy from sct we're in
         ftb = m [in].time;   fte = (in+1 >= nm) ? _tEnd : m [in+1].time;
TRC(" ftb=`s fte=`s", TmSt (s1, ftb), TmSt (s2, fte));
         for (j = 0;  j < _f.chd.Ln;  j++)       // j is to spot
            if ((j >= _f.chd.Ln) || (_f.chd [j].time >= tb))  break;
//DBG(" j=`d/`d", j, _f.chd.Ln);
         for (k = 0;  k < _f.chd.Ln;  k++) {     // k is from spot
            tm = _f.chd [k].time;
//DBG("  k=`d tm=`s", k, TmSt(s1, tm));
            if ((tm >= ftb) && (tm < fte) && (tm - ftb + tb < te) &&
                                                           (! _f.chd.Full ())) {
//DBG("   did ins j=`d k=`d", j, k);
               _f.chd.Ins (j);         // ins new chd offset properly
               if (k >= j)  k++;
               _f.chd [j].time = tm - ftb + tb;
               StrCp (_f.chd [j].s, _f.chd [k].s);
               j++;
            }
         }
      }
   }

// get trk pos of our pop_arp track (or drumtrack)
   for (t = 0;  t < _f.trk.Ln;  t++)
      if ((! TDrm (t)) && (! StrCm (_f.trk [t].name, CC("pop_arp"))))  break;
   if (t >= _f.trk.Ln)  for (t = 0;  t < _f.trk.Ln;  t++)  if (TDrm (t))  break;
//DBG("pop_arp trk=`d", t);
   if (StrCm (_f.trk [t].name, CC("pop_arp"))) {
   // make the 3 tracks we write to if missin'
//DBG("makin trks");
      if (TrkIns (t,   CC("pop_arp"),   CC("SynLead/Square")) == MAX_TRK)
         Hey (CC("couldnt insert pop_arp track"));
      if (TrkIns (t+1, CC("pop_chord"), CC("SynPad/PolySyn")) == MAX_TRK)
         Hey (CC("couldnt insert pop_chord track"));
      if (TrkIns (t+2, CC("pop_bass"),  CC("Bass/Syn2"))      == MAX_TRK)
         Hey (CC("couldnt insert pop_bass track"));
      _f.trk [t].ht = _f.trk [t+1].ht = _f.trk [t+2].ht = 'S';
   }
// kill all old events
   EvDel (t, 0, _f.trk [t].ne);   EvDel (t+1, 0, _f.trk [t+1].ne);
                                  EvDel (t+2, 0, _f.trk [t+2].ne);
   for (c = 0;  c < _f.chd.Ln;  c++) {
      tb = _f.chd [c].time;
      te = (c+1 < _f.chd.Ln) ? _f.chd [c+1].time : _tEnd;
      if (te - tb > 3*M_WHOLE)  te = tb + M_WHOLE;
//TStr s1,s2;
//DBG("ch#=`d tb=`s te=`s ch=`s",
//c, TmSt (s1, tb), TmSt (s2, te), _f.chd [c].s);

   // turn ch into root str n qs (chord quality str)
      StrCp (ch, _f.chd [c].s);        // set ch,root,qual from prog[c]
      if      (ch [1] == '#')  {StrCp (qs, & ch [2]);   ch [2] = '\0';}
      else if (ch [1] == 'b')  {StrCp (qs, & ch [2]);   ch [2] = '\0';}
      else                     {StrCp (qs, & ch [1]);   ch [1] = '\0';}
   // turn rootstr(ch) n qs into root(0-11) n qual (MChdX index)
      root = 12;
      for (i = 0;  i < 12;  i++)
         if (! StrCm (MKeyStr  [i], ch))  {root = i;   break;}
      for (i = 0;  i < 12;  i++)
         if (! StrCm (MKeyStrB [i], ch))  {root = i;   break;}
      if (root >= 12)
         Die (StrFmt (s, "couldn't get root from ch `s", ch));
      for (qual = 0;  qual < NMChdX;  qual++)    // lookup qs in MChd[].lbl
         if (! StrCm (qs, CC(MChdX [qual].lbl), 'x'))  break;
      if (qual >= NMChdX)
         Die (StrFmt (s, "couldn't find chord quality `s", qs));

   // arp - 16ths btw 4g..6g
      MemCp (s, MChdX [qual].tmp, 8);
      for (nnt = i = 0;  s [i] != 'x';  i++) {
         x = (root + s [i]) % 12;
         for (n = MKey (CC("4g"));  n <= MKey (CC("6g"));  n++)
            if ((n % 12) == x)  nt [nnt++] = n;
      }
      Sort (nt, nnt, sizeof (nt [0]), CmpB);
      for (i = 0, tm = tb;  tm < te-1;  tm += M_WHOLE/16, i++)
//       NtIns (t, tm, tm+M_WHOLE/16-1, nt [i % nnt],    32 + Rand (96));
         NtIns (t, tm, tm+M_WHOLE/16-1, nt [Rand (nnt)], 32 + Rand (96));

   // chord - chord notes 3f-4e(nonroot)
      for (i = 1;  s [i] != 'x';  i++) {
         n = (root + s [i]) % 12;   while (n < MKey (CC("3f")))  n += 12;
         if (n > MKey (CC("4e")))  n -= 12;
         NtIns (t+1, tb, te-1, n);
      }

   // bass - root in oct 1 for full dur
      NtIns (t+2, tb, te-1, 2*12+root);
   }
   ReDo ();
   TmHop (itm);
}


void Song::PreChd ()
{ ubyt4 cp, i, tm1, tm = Up.pos.tm, hbt = Up.pos.hBt;
  TStr  ch1, ch2, s;
  bool  got;
// if we got a chd within hbt of orig tm, use that time instead
   cp = _f.chd.Ln;                     // default to NO _f.chd[] pos
   *s = '\0';
   for (i = 0;  i < _f.chd.Ln;  i++) {
      if (_f.chd [i].time     >  tm+hbt)  break;
      if (_f.chd [i].time+hbt >= tm) {
         if      (cp >= _f.chd.Ln)                            cp = i;
         else if (ABSL((sbyt4)_f.chd [cp].time - (sbyt4)tm) >
                  ABSL((sbyt4)_f.chd [i ].time - (sbyt4)tm))  cp = i;
      }
   }
   tm1 = 0;   StrCp (ch1, CC("C"));   StrCp (ch2, ch1);
   if (cp == _f.chd.Ln) {              // new dude, use bar,beat time
      got = false;                     // no chd pos yet
      tm  = Up.pos.tmBt;               // closest bar to click point
      for (cp = 0;  cp < _f.chd.Ln;  cp++)
         if (_f.chd [cp].time >= tm)  break;     // ins point

   // use cp-1,cp for ChdBtw
      if (cp)               {StrCp (ch1, _f.chd [cp-1].s);
                             tm1 =       _f.chd [cp-1].time;}
      if (cp < _f.chd.Ln)    StrCp (ch2, _f.chd [cp  ].s);
   }
   else {
      got = true;                      // got existin cp
      tm =      _f.chd [cp].time;
      StrCp (s, _f.chd [cp].s);

   // use cp-1,cp+1 for ChdBtw
      if (cp)               {StrCp (ch1, _f.chd [cp-1].s);
                             tm1 =       _f.chd [cp-1].time;}
      if (cp+1 < _f.chd.Ln)  StrCp (ch2, _f.chd [cp+1].s);
   }

// passin got, cp, tm, s(this chord)
// tm1(for hopTo), ch1, ch2, (for ChdBtw)
   Up.pos.got = got ? 'y' : '\0';
   Up.pos.cp = cp;   Up.pos.tm = tm;   Up.pos.tmBt = tm1;
   StrCp (Up.pos.str, s);   StrCp (Up.d [0][0], ch1);
                            StrCp (Up.d [0][1], ch2);
   MemCp (& Up.pos.kSg, KSig (tm), sizeof (KSgRow));
   emit sgUpd ("dChd");
}


void Song::Chd (char *arg)
{ ubyte  on [128], t, bass;
  bool   got = true, dn;
  TrkEv *e;
  ubyt4  tm = 0, p, p2, csc, ne, lp, tp [MAX_TRK];
  TStr   lbl, chd, s;
  bool   fl;
  char   tmp [sizeof (MChdX [0].tmp)];
  TSgRow *ts;
   if (*arg == '+')  return PopChd (Str2Int (& arg [1]));
   if (*arg == '@')  return ChdArr (Str2Int (& arg [1]));
   if (*arg == '.') {                  // ins/upd/del a chord
     ColSep c (& arg [2], 4);
      got = (arg [1] == 'y') ? true : false;
      p   = Str2Int (c.Col [1]);
      tm  = Str2Int (c.Col [2]);
      StrCp (chd, c.Col [3]);
   // update _f.chd (ins/del/upd) based on got,p,tm,chd
      *s = '\0';   if (got)  StrCp (s, _f.chd [p].s);
DBG("new: chd='`s' old: s='`s' got=`b", chd, s, got);
      if (StrCm (s, chd)) {            // diff?  set _f.chd [_cp], _got
         if      (   got  && (! *chd))  _f.chd.Del (p);
         else if ((! got) &&    *chd )  _f.chd.Ins (p);
         if (*chd)  {StrCp (_f.chd [p].s, chd);
                            _f.chd [p].time = tm;}
      }
      return;
   }
// kill em all
   _f.chd.Ln = 0;   _pChd = 0;
   if (*arg == 'x')  return;           // just wiping em?

// arg was ? so recalc em
   NSet = 0;
   MemSet (on, 0, sizeof (on));        // notes all off, track poss all 0
   MemSet (tp, 0, sizeof (tp));

// get each unique time and assoc'd notesets
   while (got) {
      got = dn = false;                // get 1st time of note across melo trks
      for (   t = 0; t < _f.trk.Ln; t++)  if ((! TDrm (t)) && TLrn (t))
         for (p = tp [t], e = _f.trk [t].e, ne = _f.trk [t].ne;  p < ne;  p++)
            if (! (e [p].ctrl & 0x80)) {
               if      (! got)           {tm = e [p].time;   got = true;}
               else if (e [p].time < tm)  tm = e [p].time;
               break;
            }
      if (got) {                       // build on[] noteset from note evs @ tm
         for (t = 0; t < _f.trk.Ln; t++)  if ((! TDrm (t)) && TLrn (t)) {
            for (p = tp [t], e = _f.trk [t].e, ne = _f.trk [t].ne;
                 (p < ne) && (e [p].time <= tm);  p++)
               if (! (e [p].ctrl & 0x80)) {
                  if      (! (e [p].valu & 0x80))  on [e [p].ctrl]--;
                  else if (! (e [p].val2 & 0x80))     // skip prss evs
                                     {dn = true;   on [e [p].ctrl]++;}
               }
            tp [t] = p;
         }
      // stamp a Set unless had ONLY noteoffs
         if (dn) {
            if (NSet >= BITS (Set))  Die (CC("DoChd  outa room in Set[]"));
            Set [NSet].time = tm;   SetSet (Set [NSet].nmap, on);
            NSet++;
         }
      }
   }

TRC("get initial 1 Set scores");
   for (p = 0;  p < NSet;  p++)  Set [p].scor = Score (p, 1);

TRC("keep tacking on trailing Sets while combo scores better");
   for (p = 0;  p < NSet;  p = p2) {
//TStr db; DBG(" p=`d/`d `s",  p, NSet, TmSt(db,Set [p].time));
      for (p2 = p+1;  p2 < NSet;) {
         csc = Score (p, p2-p+1);
//DBG("   comb=`d vs sep=`d(`d + `d)",
//csc, Set [p].scor + Set [p2].scor,  Set [p].scor, Set [p2].scor);
         if (csc >= (Set [p].scor + Set [p2].scor)) {
//DBG("   GLUE");
            Set [p].scor = csc;   Set [p2].time = SKIP;   p2++;
         }
         else  break;
      }
//DBG("   p2=`d num=`d", p2, p2-p+1);
   }

TRC("draw in chords");
   for (p = 0;  p < NSet;  p = ++p2) {
      for (p2 = p;  Set [p2+1].time == SKIP;  p2++)  ;
      fl = KSig (Set [p2].time)->flt ? true : false;
      if (Score (p, p2-p+1, lbl, fl, tmp, & bass) && (! _f.chd.Full ())) {
//TStr db; DBG("p=`d p2=`d tm=`s lbl=`s", p, p2, TmSt (ts, Set [p].time), lbl);
      // ins chord time,label in _f.chd[]
         lp = _f.chd.Ins ();   _f.chd [lp].time = Set [p].time;
                        StrCp (_f.chd [lp].s, lbl);
      }
   }
// kill chords with teeny durs
   for (p = 0;  p < _f.chd.Ln;) {
      if (p+1 < _f.chd.Ln) {
         ts = TSig (_f.chd [p].time);
         if ( (_f.chd [p+1].time - _f.chd [p].time) < ((M_WHOLE/ts->den)/2) )
               _f.chd.Del (p);
         else  p++;
      }
      else     p++;
   }
// kill resulting same following chords
   for (p = 1;  p < _f.chd.Ln;) {
      if (! StrCm (_f.chd [p-1].s, _f.chd [p].s))  _f.chd.Del (p);
      else                                                     p++;
   }
   ReDo ();
}
