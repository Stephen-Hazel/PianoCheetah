// SynSound.cpp  build device\syn\sound.txt from all the dirs of wav files

#include "rc\resource.h"
#include "ui.h"
#include "MidiIO.h"

TStr DPath, Snd [63*1024], SampSetM, SampSetD, DrumSet;
ulong      NSnd;                       // default melo/drum samp n drumsets

char *Grp [] = {
// Drum ones
   "Kick\\", "Snar\\", "HHat\\", "Cymb\\", "Toms\\", "Misc\\", "Latn\\", "x\\",
// Inst ones
   "Piano\\", "Organ\\", "SynLead\\", "SynPad\\", "SynFX\\",
   "Guitar\\", "SoloStr\\", "Ensemble\\",
   "Brass\\", "Reed\\", "Pipe\\", "Ethnic\\",
   "Bass\\",
   "Perc\\", "ChromPerc\\", "SndFX\\",
   "etc\\"
};


void FindSnd (char *dir)
// list all the dirs under device\syn going a certain level deep, etc
// sampset is removed from prefix, tacked on with |sampset suffix
// drumset "  "       "    "       "      "  "    :drumset suffix
// sampset\gmGrp\gmSnd_morename
//     ==> gmGrp\gmSnd|sampset\morename
// sampset\etc\name\
//     ==> etc|sampset\name
// sampset\Drum\drumset\drGrp\drSnd_morename
//     ==> Drum\drGrp\drSnd|sampset:drumset\morename
{ char *p, *q, *r, *ps;
  TStr  fn, s, t, ofn;
  char  df;
  ulong ln, n, i;
  bool  dr;
  FDir  d;
   if (! (df = d.Open (fn, dir)))  return;
   do {
      ln = StrLn (fn);
      if (df == 'd') {                 // sound names are ALWAYS dirs
         p = & fn [StrLn (DPath)+1];   // skip past ...\device\syn\   ;klj
         StrCp (ofn, p);               // p points to sampset, ofn fer warnings

      // count up num \ s fer dir levl
         for (n = 0, q = p;  *q;  q++)  if (*q == '\\')  n++;
         dr = StrSt (p, "\\Drum\\") ? true : false;   // skip sampset;  drum ?

         if      (n <  2) {FindSnd (fn);   n = 0;}    // sampset or gmgrp/Drum
         else if (n == 2)             // gmSnd_x or drumset;  more if dr
            {if (dr)      {FindSnd (fn);   n = 0;}}
         else if (n <  4) {FindSnd (fn);   n = 0;}    // drGrp  ...one more
         if ( (((  dr) && (n == 4)) ||
               ((! dr) && (n == 2)))  &&  (NSnd < BITS (Snd)) ) {
//DBG("got snd `s  `d", p, NSnd);
            q = StrCh (p, '\\');   *q++ = '\0';       // p=sampset, q=rest
            if (dr) {
               q += 5;                 // skip Drum\

            // now, p=sampset, q=drumset, r=rest
               r = StrCh (q, '\\');   *r++ = '\0';

            // validate drGrp\drSnd (s set to drSnd minus any _morename)
               StrCp (s, r);   if (ps = StrCh (s, '_'))  *ps = '\0';
               for (i = 0;   i < 128;  i++)
                  {MDrm2StG (t, (ubyte)i);   if (! StrCm (& t [5], s))  break;}

               if (i < 128) {          // :) ok to ins it
               // ps = morename of rest if any;  r left w drGrp\drSnd
                  if (ps = StrCh (r, '_'))  *ps++ = '\0';
                  StrFmt (Snd [NSnd], "Drum\\`s|`s:`s", r, p, q);
                  if (ps)  {StrAp (Snd [NSnd], "\\");
                            StrAp (Snd [NSnd], ps);}
                  NSnd++;
//DBG("    ==> `s", Snd [NSnd-1]);
               }
               else
DBG("`s ==> NOPE !!", ofn);            // :(
            }
            else {                     // melodic way easier but etc is weird
            // validate etc| or gmGrp\gmSnd
               if (! MemCm (q, "etc\\", 4)) {
                  StrFmt (Snd [NSnd++], "etc|`s\\`s", p, & q [4]);
//DBG("    ==> `s", Snd [NSnd-1]);
               }
               else {                  // now p=sampset,q=rest,s=gmGrp\gmSnd onl
                  StrCp (s, q);   if (ps = StrCh (s, '_'))  *ps = '\0';
                  for (i = 0;  i < 128;  i++)  if (! StrCm (s, MProg [i]))
                                                  break;
                  if (i < 128) {          // ins it w optional sampset
                     StrFmt (Snd [NSnd], "`s|`s", s, p);
                     if (ps)  {StrAp (Snd [NSnd], "\\");
                               StrAp (Snd [NSnd], ++ps);}
                     NSnd++;
//DBG("    ==> `s", Snd [NSnd-1]);
                  }
                  else
DBG("`s ==> NOPE !!", ofn);
               }
            }
         }
      }
   } while (df = d.Next (fn));
   d.Shut ();
}


void SampSet ()
// rename:
// ==> gmGrp\gmSnd.morename            for SampSetM   (morename #1 stripped)
// ^   gmGrp\gmSnd|sampset\morename    non default
//     etc|sampset\name                non default always here
// ==> Drum\drGrp\drSnd.morename       for SampSetD & DrumSet
// ==> Drum\drGrp\drSnd:drumset.morename   SampSetD & non DrumSet
// ^   Drum\drGrp\drSnd|sampset:drumset\morename      non default
{ ulong i;
  char *ps, *px;
  TStr  ss, ds, ssm, ssd;
// pick out default sampset for melo,drum;  n default drumset
   for (i = 0;  i < NSnd;  i++) {
      ps = StrCh (Snd [i], '|');       // parse out sampset into ss
      if (! ps)  {DBG("no | ??  `s", Snd [i]);   continue;}

      StrCp (ss, ps+1);
      if (! MemCm (Snd [i], "Drum\\", 5)) {      // drum
         px = StrCh (ss, ':');
         if (! px)  {DBG("no : ??  `s", ss);   continue;}

         *px = '\0';
         StrCp (ds, 1 + StrCh (Snd [i], ':'));   // we KNOw : exists
         if (px = StrCh (ds, '\\'))  *px = '\0';
         if (*ds == '_') {             // ding ding!  check for 2nd+
            if (*SampSetD && StrCm (SampSetD, ss))
                  DBG("got 2nd+ default drum sampset `s", ss);
            else  StrCp (SampSetD, ss);
            if (*DrumSet  && StrCm (DrumSet,  ds))
                  DBG("got 2nd+ default drumset `s", ds);
            else  StrCp (DrumSet, ds);
         }
      }
      else {                                     // melo
         if (px = StrCh (ss, '\\'))  *px = '\0';      // trim any \morename
         if (*ss == '_') {             // ding ding!  check for 2nd+
            if (*SampSetM && StrCm (SampSetM, ss))
                  DBG("got 2nd+ default melo sampset `s", ss);
            else  StrCp (SampSetM, ss);
         }
      }
   }
   if (! *SampSetM)  StrCp (SampSetM, "NO_DEFAULT_MELODIC_SAMPSET");
   if (! *SampSetD)  StrCp (SampSetD, "NO_DEFAULT_DRUM_SAMPSET");
   if (! *DrumSet)   StrCp (DrumSet, "NO_DEFAULT_DRUMSET");
//DBG("SampSetM=`s SampSetD=`s DrumSet=`s", SampSetM, SampSetD, DrumSet);

// now we can rename
   StrFmt (ssm, "|`s", SampSetM);   StrFmt (ssd, "|`s", SampSetD);
                                    StrFmt (ds,  ":`s", DrumSet);
   for (i = 0;  i < NSnd;  i++) {      // rename via defaults
//DBG(Snd [i]);
      if (! MemCm (Snd [i], "Drum\\", 5)) {      // drum
         if (ps = StrSt (Snd [i], ssd))  StrCp (ps, ps+StrLn (ssd));
         if (ps = StrSt (Snd [i], ds ))  StrCp (ps, ps+StrLn (ds));
      }
      else {                                     // melo
         if (! MemCm (Snd[i], "etc|", 4))  continue;  // no messin

         if (ps = StrSt (Snd [i], ssm))  *ps = '\0';  // NOTE: killed morename 2
      }
//DBG("  ==> `s", Snd [i]);
   }
}


int SndCmp (void *p1, void *p2)
// got these to sort:  Drum\drGrp\drSnd|sampset:drumset\morename
//                          gmGrp\gmSnd|sampset\morename
//                          etc|sampset\name
// sort by Drum\ vs not;  gmGrp/drGrp order;
//         etc=>straight cmp; no |sset vs not; no :dset vs not; gmSnd/drSnd;
//         then straight cmp
{ char *s1, *s2;
  int   d1, d2, g1, g2, o, n1, n2;
  TStr  b1, b2;
   StrCp (b1, (char *)p1);   s1 = & b1 [0];
   StrCp (b2, (char *)p2);   s2 = & b2 [0];
//DBG("'`s' <=> '`s'", b1, b2);
   d1 = MemCm (s1, "Drum\\", 5) ? 1 : 0;    // 0=drum 1=melo
   d2 = MemCm (s2, "Drum\\", 5) ? 1 : 0;
   if (d1 - d2) {
//DBG("   drum diff");
      return d1 - d2;                  // by drum vs melo
   }
   for (g1 = 0;  g1 < BITS (Grp);  g1++)
      if (! MemCm (& s1 [d1?0:5], Grp [g1], StrLn (Grp [g1])))  break;
   for (g2 = 0;  g2 < BITS (Grp);  g2++)
      if (! MemCm (& s2 [d2?0:5], Grp [g2], StrLn (Grp [g2])))  break;
   if (g1 - g2) {
//DBG("   grp diff");
      return g1 - g2;                  // by gmGrp/drGrp order
   }
   if (! MemCm (s1, "etc\\", 4)) {     // no need to check etc stuff further
//DBG("   etc diff");
      return StrCm (s1, s2);
   }
   g1 = StrCh (s1, '|') ? 1 : 0;   g2 = StrCh (s2, '|') ? 1 : 0;
   if (g1 - g2) {
//DBG("   | diff");
      return g1 - g2;                  // !sset vs not
   }
   g1 = StrCh (s1, ':') ? 1 : 0;   g2 = StrCh (s2, ':') ? 1 : 0;
   if (g1 - g2) {
//DBG("   : diff");
      return g1 - g2;                  // !sset vs not
   }
   if (d1) {                           // melo - check gmSnd
      for (n1 = 0;  n1 < 128;  n1++)
         if (! MemCm (s1, MProg [n1], StrLn (MProg [n1])))  break;
      for (n2 = 0;  n2 < 128;  n2++)
         if (! MemCm (s2, MProg [n2], StrLn (MProg [n2])))  break;
   }
   else {                              // drum - check drSnd
      o = 5 + StrLn (Grp [g1]);        // ofs to drum name
      for (n1 = 0;  n1 < NMDrum;  n1++)
         if (! MemCm (& s1 [o], MDrum [n1].sym, StrLn (MDrum [n1].sym)))
            break;
      for (n2 = 0;  n2 < NMDrum;  n2++)
         if (! MemCm (& s2 [o], MDrum [n2].sym, StrLn (MDrum [n2].sym)))
            break;
   }
   if (n1 - n2) {
//DBG("   gmSnd/drSnd diff");
      return n1 - n2;                  // diff gmSnd/drSnd pos
   }

// just alpha leftover |sampset:drumset\morename now
   return StrCm (b1, b2);
}


int Go ()
// load;  remap samplesets,drumsets;  sort all sound dirs
{ ulong i;
  File  f;
  PStr  buf;
   App.Path (DPath, 'd');   StrAp (DPath, "\\device\\syn");

   FindSnd (DPath);   SampSet ();   Sort (Snd, NSnd, sizeof (Snd[0]), SndCmp);

   StrAp (DPath, "\\sound.txt");
   if (! f.Open (DPath, "w"))  return 99;
   f.Put (StrFmt (buf, "#SS `s `s `s\r\n", SampSetM, SampSetD, DrumSet));
   for (i = 0;  i < NSnd;  i++)
      f.Put (StrFmt (buf, "`<40s . . .\r\n", Snd [i]));
   f.Shut ();

   return 0;
}
