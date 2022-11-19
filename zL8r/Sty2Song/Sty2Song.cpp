// Sty2Song.cpp - load some .mid style clip files plus a song's chord list
//                write some note event files

#include "ui.h"
#include "MidiIO.h"
#include "SongLoad.cpp"

struct TrkRow {TStr name, snd;  ulong tr, ot, ne;  TrkEv *e;};
struct StyRow {char id [3];  ulong te, tsN, tsD, dur;
                                               TrkEv *e;  Arr<TrkRow,16> trk;};
struct TrORow {TStr name, snd;  ulong tr, is, it, ne;};

struct db {                            // cuz Arr<>s can't be global :(
   Arr<StyRow,4*26> sty;
   Arr<TrORow,128>  trk;               // output tracks (all ins combined)
};
db *DB;

TStr  TMS;
ulong TSigN = 4, TSigD = 4;

char *TmSt (char *str, ulong tm)       // HACK fer debuggin :/
{ ulong dBr, dBt, bx;
  uword    s = 0, br, bt;
   if (! TSigD)  {TSigN = TSigD = 4;}
   dBt = M_WHOLE / TSigD;
   dBr = dBt     * TSigN;
   br  = (uword)(1 +  tm / dBr);
   bt  = (uword)(1 + (tm % dBr) / dBt);
   bx  =             (tm % dBr) % dBt;
   return StrFmt (str, "`04d.`d.`d(`d)=`d", br, bt, bx, bx*100/dBt, tm);
}

ubyte Nm2Part (char *n)
{ TStr p;
   MemCp (p, & n [1], 4);   p [4] = '\0';
   if      (! StrCm (p, "melo"))  return 0;
   else if (! StrCm (p, "chrd"))  return 1;
   else if (! StrCm (p, "bass"))  return 2;
                                  return 3;
}


char *Grp [] = {
   "Piano\\", "Organ\\", "SynLead\\", "SynPad\\", "SynFX\\",
   "Guitar\\", "SoloStr\\", "Ensemble\\",
   "Brass\\", "Reed\\", "Pipe\\", "Ethnic\\",
   "Bass\\",
   "Perc\\", "ChromPerc\\", "SndFX\\"
};


int SndCmp (char *s1, char *s2)        // sort by Drum,GMgrp,snd
{ int g1, g2;
   g1 = MemCm (s1, "Drum\\", 5) ? 0 : 1;
   g2 = MemCm (s2, "Drum\\", 5) ? 0 : 1;
   if (g1 || g2)  return g1 - g2;

   for (g1 = 0; g1 < BITS (Grp); g1++)
      if (! MemCm (s1, Grp [g1], StrLn (Grp [g1])))  break;
   for (g2 = 0; g2 < BITS (Grp); g2++)
      if (! MemCm (s2, Grp [g2], StrLn (Grp [g2])))  break;
   if (g1 - g2)  return g1 - g2;

   for (g1 = 0; g1 < 128; g1++)  if (! StrCm (s1, MProg [g1]))  break;
   for (g2 = 0; g2 < 128; g2++)  if (! StrCm (s2, MProg [g2]))  break;
   return g1 - g2;
}


int TrkCmp (void *p1, void *p2)        // ...trk[] sortin: part,snd,tr
{ TrkRow *t1 = (TrkRow *)p1, *t2 = (TrkRow *)p2;
  int t;
   if (t = Nm2Part (t1->name) - Nm2Part (t2->name))  return t;
   if (t = SndCmp  (t1->snd,             t2->snd ))  return t;
   return  t1->tr - t2->tr;
}


void Load (char *sty)                  // load a style fn's guts into DB->sty[]
{ ulong s, t, te, n, d;
  TStr  fn, fnt, cmd;
  char *p, *p1;
//DBG("Sty2Song Load '`s'", sty);
   if (StrLn (sty) <= 2) {
DBG("Load - fn empty - omit");
      return;
   }
   s = DB->sty.Ln++;                   // never full by def :/
   MemCp (DB->sty [s].id, sty, 2);   DB->sty [s].id [2] = '\0';
   App.Path (fn, 'd');   StrAp (fn, "\\clip\\");
   switch (*sty) {
      case '<':  StrAp (fn, "intro\\");   break;
      case '>':  StrAp (fn, "outro\\");   break;
      case '!':  StrAp (fn, "fill\\");    break;
      default:   StrAp (fn, "main\\");
   }
   StrAp (fn, & sty [2]);   StrAp (fn, ".mid");

   App.Path (cmd);   StrAp (cmd, "\\Mid2Song d ");   StrAp (cmd, fn);
//DBG(" cmd=`s", cmd);
   RunWait  (cmd);
   App.Path (fnt, 'e');   StrAp (fnt, "\\tmp_d.song");
//DBG(" loaded");
// parse out tsN,tsD,dur
   DB->sty [s].tsN = DB->sty [s].tsD = 4;   DB->sty [s].dur = M_WHOLE;
   StrCp (fn, & sty [2]);   p1 = fn;
   if (p = StrCh (p1, '.')) {
      *p++ = '\0';          DB->sty [s].tsN = n = Str2Int (p1);   p1 = p;
      if (p = StrCh (p1, '_')) {
         *p++ = '\0';       DB->sty [s].tsD = d = Str2Int (p1);   p1 = p;
         if (p = StrCh (p1, '_'))
            {*p++ = '\0';   DB->sty [s].dur = Str2Int (p1) * M_WHOLE * n / d;}
      }
   }
  STable st [TB_MAX];
   DB->sty [s].e = NULL;               // MUST be init'd
//DBG(" SongLoad");
   if (! SongLoad (fnt, & DB->sty [s].e, & DB->sty [s].te, 0, st))
      Die ("Load  SongLoad() died");

//DBG(" SongLoad done");
   DB->sty [s].trk.Ln = st [TB_TRK].NRow ();
   for (te = 0, t = 0;  t < st [TB_TRK].NRow ();  t++) {
      StrCp (DB->sty [s].trk [t].snd,  st [TB_TRK].Get (t,1));
      StrCp (DB->sty [s].trk [t].name, st [TB_TRK].Get (t,2));
             DB->sty [s].trk [t].e  = & DB->sty [s].e [te];
      te += (DB->sty [s].trk [t].ne = Str2Int (st [TB_TRK].Get (t,3)));
             DB->sty [s].trk [t].tr = t;
   }
   Sort (DB->sty [s].trk.Ptr (), DB->sty [s].trk.Ln, DB->sty [s].trk.Siz (),
         TrkCmp);
//DBG(" sorted");
// renumber .tr per part,snd
   for (t = 0;  t < DB->sty [s].trk.Ln;  t++)
      if ( (! t) || (Nm2Part (DB->sty [s].trk [t  ].name) !=
                     Nm2Part (DB->sty [s].trk [t-1].name)) ||
           StrCm (DB->sty [s].trk [t].snd, DB->sty [s].trk [t-1].snd) )
            DB->sty [s].trk [t].tr = 0;
      else  DB->sty [s].trk [t].tr = DB->sty [s].trk [t-1].tr + 1;
//DBG(" Load done");
}


void TrkGlue ()
// go thru each sty fn and combine all the tracks into ONE outbound track set
{ ulong s, t, o, pi, po, s2, t2;
//DBG("{ TrkGlue");
   for (s = 0;  s < DB->sty.Ln;  s++)
      for (t = 0;  t < DB->sty [s].trk.Ln;  t++) {
//DBG("gluein s=`d t=`d...", s, t);
         for (o = 0;  o < DB->trk.Ln;  o++)
         // matching part,sound,psTrackNo diff sty means can reuse otrack
            if ( (Nm2Part (DB->sty [s].trk [t].name) ==
                  Nm2Part (        DB->trk [o].name)) &&
                 (! StrCm (DB->sty [s].trk [t].snd,  DB->trk [o].snd)) &&
                          (DB->sty [s].trk [t].tr == DB->trk [o].tr)   &&
                 (s != DB->trk [o].is) )
            // can link up reused otrk ?
               {DB->sty [s].trk [t].ot = o;
//DBG("   reuse ot=`d", o);
               break;
               }
         if (o >= DB->trk.Ln) {        // ins new trk sorted by part,snd,trk
            if (DB->trk.Full ())  Die ("Save  tooo many tracks");
            pi = Nm2Part (DB->sty [s].trk [t].name);
            for (o = 0;  o < DB->trk.Ln;  o++) {
               po = Nm2Part (DB->trk [o].name);
               if (po > pi)  break;
               if (po < pi)  continue;
               if (StrCm (DB->trk [o].snd, DB->sty [s].trk [t].snd) > 0)  break;
               if (StrCm (DB->trk [o].snd, DB->sty [s].trk [t].snd) < 0)
                                                                       continue;
               if (DB->trk [o].tr > DB->sty [s].trk [t].tr)  break;
            }
//DBG("   ins new ot=`d nm=`s sn=`s tr=`d",
//o, DB->sty [s].trk [t].name, DB->sty [s].trk [t].snd, DB->sty [s].trk [t].tr);
            DB->trk.Ins (o);
            StrCp (DB->trk [o].name, DB->sty [s].trk [t].name);
            StrCp (DB->trk [o].snd,  DB->sty [s].trk [t].snd);
                   DB->trk [o].tr =  DB->sty [s].trk [t].tr;
            DB->trk [o].is = s;        // link em up both ways for 1st sty trk
            DB->trk [o].it = t;
            DB->sty [s].trk [t].ot = o;

         // gotta adjust existing prev s,t's .ot's >= o
            for (s2 = 0;  s2 <= s;  s2++)
               for (t2 = 0;  t2 < ((s2 < s) ? DB->sty [s2].trk.Ln : t);  t2++)
                  if (DB->sty [s2].trk [t2].ot >= o)
                      DB->sty [s2].trk [t2].ot++;
         }
/*
DBG("----------");
for (s2 = 0;  s2 < DB->sty.Ln;  s2++) {
 DBG("s=`d/`d id=`s te=`d",
 s2, DB->sty.Ln, DB->sty [s2].id, DB->sty [s2].te);
 for (t2 = 0;  t2 < DB->sty [s2].trk.Ln;  t2++)
  DBG("  t=`d/`d name=`s snd=`s tr=`d ot=`d ne=`d",
  t2, DB->sty [s2].trk.Ln, DB->sty [s2].trk [t2].name,
  DB->sty [s2].trk [t2].snd, DB->sty [s2].trk [t2].tr,
  DB->sty [s2].trk [t2].ot, DB->sty [s2].trk [t2].ne);
}
for (t2 = 0;  t2 < DB->trk.Ln;  t2++)
 DBG("o=`d/`d name=`s snd=`s tr=`d is=`d it=`d ne=`d",
 t2, DB->trk.Ln, DB->trk [t2].name, DB->trk [t2].snd, DB->trk [t2].tr,
 DB->trk [t2].is, DB->trk [t2].it, DB->trk [t2].ne);
*/
      }
//DBG("} TrkGlue");
}


void Save (char *fnCh)
{ STable chT;
  File   f,   fo [128];
  bool   min, op [128];
  sbyte  kso, cho, tro;
  ulong  st, tr, otrk, i, fl, d, e, g, tm, dur, tmCl, durCl, durNt, nev;
  TStr   fn, ts, nm, sChd, t2, chd, sty, ks1, ch1, ch2, ch3, stySeq;
  PStr   dbg;
  char  *pc, *p, *cTmp, *sTmp, part, trn;
  bool   drm;
  ubyte  ci, yId, hs, nMp, mp [128], m, mpo [128], fng, df, styPos,
         root, sla;
  TrkEv *ev, eo;
//DBG("{ Save");
   TrkGlue ();   chT.Load (fnCh, NULL, 2);  // combine trk;  load chdSty txt

   StrFmt (fn, "`s\\sty.out", App.Path (ts, 'e'));    // open trk output file
   if (! f.Open (fn, "w"))  Die ("can't write", fn);
   MemSet (op, 0, sizeof (op));                       // init ev file tracker
   st = 0;                                  // OFF - no style (yet)

// keysig transpose offset into kso (and default the chord transpose offset)
   StrCp (ks1, chT.Get (chT.NRow ()-1, 1));
   if ((kso = MNt2Int (ks1)) > 4)  kso -= 12;
   cho = kso;
   min = (StrLn (ks1) && (ks1 [StrLn (ks1)-1] == 'm')) ? true : false;

// major,C,no slash
   yId = 34;   cTmp = MChd [MAJ_CHD].tmp;   root = 0;   sla = 99;

// default sty, tsig too
   StrCp (stySeq, ".");   styPos = 1;   st = 0;
   TSigN = DB->sty [0].tsN;
   TSigD = DB->sty [0].tsD;

   for (drm = true, i = 0;  i < chT.NRow ()-1;  i++)  // drumonly unless got chd
      if (StrCh (".!<>", chT.Get (i, 1) [0]) == NULL)  {drm = false;   break;}

   for (i = 0;  i < chT.NRow ()-1;  i++) {
      tm =       chT.GetI (i, 0);   dur = chT.GetI (i+1, 0) - tm;
      StrCp (ts, chT.Get  (i, 1));
//DBG("i=`d/`d tm=`s dur=`d ts=`s", i, chT.NRow ()-1, TmSt(TMS,tm), dur, ts);

   // split ts into chd,sty
      if      (pc = StrCh (ts, '.'))  ;
      else if (pc = StrCh (ts, '!'))  ;
      else if (pc = StrCh (ts, '<'))  ;
      else if (pc = StrCh (ts, '>'))  ;
      else     pc = & ts [StrLn (ts)];
      StrCp (sty, pc);   *pc = '\0';   StrCp (chd, ts);

      if (sty [0]) {                   // new sct,sty
         if (sty [0] == '.')  {styPos = 1;   StrCp (stySeq, sty);}
         else                  styPos = 0;  // restart stySeq at end o me
         for (st = 0;  st < DB->sty.Ln;  st++)
            if (! MemCm (sty, DB->sty [st].id, 2))  break;
         if (st >= DB->sty.Ln)  st = 0;
         else {
            st++;
         // sty in list could start off beat, bump tmCl if so
            tmCl = 0;
            if (        tm % (DB->sty [st-1].dur))
               tmCl += (tm % (DB->sty [st-1].dur));
            TSigN = DB->sty [st-1].tsN;
            TSigD = DB->sty [st-1].tsD;
         }
//DBG("  NEW STYLE sty=`s st=`d stySeq='`s' styPos=`d tmCl=`s nTr=`d dur=`d",
//sty, st, stySeq, styPos, TmSt(TMS,tmCl), st?(DB->sty [st-1].trk.Ln):0,
//st?(DB->sty [st-1].dur):0);
      }

      StrCp (ch1, chd);
      if (chd [0]) {                   // split so ch1=root, ch2=type, ch3=bass
         if ((ch1 [1] == 'b') || (ch1 [1] == '#'))
              {StrCp (ch2, & ch1 [2]);   ch1 [2] = '\0';}
         else {StrCp (ch2, & ch1 [1]);   ch1 [1] = '\0';}
         ch3 [0] = '\0';
         if (pc = StrCh (ch2, '/'))  {StrCp (ch3, pc+1);   *pc = '\0';}

      // get cho, root, yId, cTmp
         cho = root = MNt2Int (ch1);   if (cho > 4) cho -= 12;
         yId = 99;   cTmp = MChd [MAJ_CHD].tmp;   // major??
         for (ci = 0;  ci < NMChd;  ci++)
            if (! StrCm (MChd [ci].lbl, ch2, 'x'))
               {yId = MChd [ci].yId;   cTmp = MChd [ci].tmp;   break;}
         sla = 99;   if (*ch3)  sla = MNt2Int (ch3);
//DBG("  NEW CHORD `s `s `s cho=`d root=`d yId=`d sla=`d",
//ch1, ch2, ch3, cho, root, yId, sla);
      }

   // keep stampin sty clip in therez till dur is up;  st=0 means no stampin
      if (st == 0)  tm += dur;
      else          for (d = 0;  d < dur;  d += durCl) {
      // default to full rest of sty clip dur;  limit if dur left is less
         durCl = DB->sty [st-1].dur - tmCl;
         if (durCl > (dur - d))  durCl = dur - d;
//DBG("  d=`d tmCl=`s durCl=`d durClAll=`d",
//d, TmSt(TMS,tmCl), durCl, DB->sty [st-1].dur);

         for (tr = 0;  tr < DB->sty [st-1].trk.Ln;  tr++) {     // put each trk
            StrCp (nm, DB->sty [st-1].trk [tr].name);
//DBG("    tr=`d/`d nm=`s", tr, DB->sty [st-1].trk.Ln, nm);

         // see if filtered per drumonly (no chords in time,chdSty list)
            if ((CHDN(nm [1]) != 'd') && drm) {
//DBG ("      drumonly n not drum so NOPE");
               continue;
            }

         // see if filtered per chord root
            if (p = StrSt (nm, "?r")) {
               fl = ((p [2]-'0') << 8) | ((p [3]-'0') << 4) | (p [4]-'0');
               p += 5;                 // hex str to int :/
               if (! (fl && (1 << root))) {
//DBG ("      ?r sez NOPE");
                  continue;
               }
            }
            else  p = nm;

         // see if filtered per chord type
            if ((yId != 99) && (p = StrCh (p, '?'))) {
               if (yId >= 32) {
                  if (yId == (32 + p [1]-'0')) {
//DBG ("      ? sez NOPE");
                     continue;
                  }
               }
               else {                  // hex str to int :/
                  fl = ((p [ 3]-'0')<<28) | ((p [ 4]-'0')<<24) |
                       ((p [ 5]-'0')<<20) | ((p [ 6]-'0')<<16) |
                       ((p [ 8]-'0')<<12) | ((p [ 9]-'0')<< 8) |
                       ((p [10]-'0')<< 4) |  (p [11]-'0');
                  if (! (fl && (1 << yId))) {
//DBG ("      ? sez NOPE");
                     continue;
                  }
               }
            }
            trn = CHDN(nm [1]);        // m,c,b,d
            tro = (trn == 'd') ? 0 : cho;
//DBG("      trn=`c tro=`d", trn, tro);

            part = trn;
            nMp = 0;
            ev  = DB->sty [st-1].trk [tr].e;
            nev = DB->sty [st-1].trk [tr].ne;

            if (trn != 'd') {
            // if transposin, first, get all sty,trk notes
               for (e = 0;  e < nev;  e++) {
               // sync to tmCl / quit upon durCl
                  if (ev [e].time <  tmCl        )  continue;
                  if (ev [e].time >= tmCl + durCl)  break;
                  if (! (ev [e].ctrl & 0xFF80)) {     // note (not ctrl)
                     for (m = 0;  m < nMp;  m++) {
                        if (ev [e].ctrl <  mp [m]) {  // insert here
                           RecIns (mp, ++nMp, sizeof (mp[0]), m);
                           mp [m]  = (ubyte)ev [e].ctrl;
                           break;
                        }
                        if (ev [e].ctrl == mp [m])  break;      // already got
                     }
                     if (m >= nMp)                    // append here
                        mp [nMp++] = (ubyte)ev [e].ctrl;
                  }
               }

            // get template for sty trk's chord quality (find yId in MChd)
               if (p = StrCh (nm, '@'))
                    {StrCp (ts, p+1);   if (p = StrCh (ts, '@'))  *p = '\0';}
               else  StrCp (ts, "M7");
               if (! StrCm (ts, "maj"))  *ts = '\0';  // sigh - 'maj' iz weird
               for (df = 0;  df < NMChd;  df++)
                  if (! StrCm (MChd [df].lbl, ts, 'x'))  break;
               if (df >= NMChd)  Die ("unknown chord type: '`s'", ts);
               StrCp (sChd, ts);   sTmp = MChd [df].tmp;
//DBG("      sChd=`s MChd[]pos=`d", ts, df);

            // try to map to play chord quality's chordal
               for (m = 0;  m < nMp;  m++) {
                  hs = mp [m] % 12;
                  mpo [m] = mp [m];    // default to straight out

//DBG("      m=`d nt=`s hs=`d sChd=`s",
//m, MKey2Str (ts, (ubyte)mp [m]), hs, sChd);

               // sTmp tells us if we have a chordal (1,3,5,7,x)
                  for (fng = 0;  sTmp [fng] != 'x';  fng++)
                     if (sTmp [fng] == hs)  break;

               // if it's a 3,5,7,x per srcQual(root never adjusts)
                  if (fng && (sTmp [fng] != 'x')) {
                  // adjust to dstQual if it has that chordal (fng)
//DBG("         fng=`d", fng);
                     for (df = 0;  cTmp [df] != 'x';  df++)
                        if (df == fng)  break;
                     if (cTmp [df] != 'x') {
                        mpo [m] = mp [m] + cTmp [fng] - sTmp [fng];
//DBG("         got fng in play chd;  out=`s hsSty=`d hsPlay=`d",
//MKey2Str (ts, mpo [m]), sTmp [fng], cTmp [fng]);
                     }
                  }

                  if (trn == 'c') {
                  // if prev nt >= got one, use next cTmp chordal from prev
                     if (m && (mpo [m-1] >= mpo [m])) {
//DBG ("         too low - get chordal from prev nt");
                        hs = mpo [m-1];
                        do {hs++;
                            for (df = 0;  cTmp [df] != 'x';  df++)
                               if (cTmp [df] == (hs % 12))  break;
                        } while (  cTmp [df] == 'x');
                        mpo [m] = hs;
                     }

                  // only cTmp chordals can go
                     for (df = 0;  cTmp [df] != 'x';  df++)
                        if (cTmp [df] == (mpo [m] % 12))  break;
                     if (cTmp [df] == 'x') {
//DBG ("         get next chordal");
                        hs = mp [m];
                        do {hs++;
                            for (df = 0;  cTmp [df] != 'x';  df++)
                               if (cTmp [df] == (hs % 12))  break;
                        } while (  cTmp [df] == 'x');
                        mpo [m] = hs;
                     }
                  }
//DBG("      mp[`d]=`s => `s", m,
//MKey2Str (ts, (ubyte)mp [m]), MKey2Str (t2, (ubyte)mpo [m]));
               }
            }
//DBG("      tro=`d (kso=`d cho=`d)", tro, kso, cho);

            for (e = 0;  e < nev;  e++) {
            // sync to tmCl / quit upon durCl
               if (ev [e].time <  tmCl        )  continue;
               if (ev [e].time >= tmCl + durCl)  break;

//DBG("      e=`d/`d tm=`d tmCl=`d tmCl+durCl=`d",
//e, nev, ev [e].time, tmCl, tmCl+durCl);
               StrFmt (dbg, "        `d `s ",
                       tr, TmSt(TMS,tm+d+ev [e].time-tmCl));
               if ( (! (ev [e].ctrl & 0xFF80)) &&     // note (not ctrl)
                    ((ev [e].valu & 0x80) && (! (ev [e].val2 & 0x80))) ) {
                                                      // note DOWN (not up,nprs)
               // find my noteup else skip writin me
                  for (durNt = 0, g = e+1;  g != e;) {
                     if (g >= nev)  {g = 0;   continue;}
                     if (    (ev [e].ctrl == ev [g].ctrl) &&
                          (! (ev [g].valu & 0x80)) ) {
                        durNt = ev [g].time + ((g < e) ? durCl : 0) -
                                ev [e].time;
                        break;
                     }
                     g++;
                  }

                  eo.ctrl = (ubyte)ev [e].ctrl;
                  if (trn != 'd') {
                     for (m = 0;  m < nMp;  m++)
                        if (eo.ctrl == mp [m])  {eo.ctrl = mpo [m];   break;}
                     eo.ctrl += tro;
                     if ((trn == 'b') && (sla != 99))
                        eo.ctrl  = eo.ctrl / 12 * 12 + sla;
                  }
//DBG("        eo.ctrl=`02x", eo.ctrl);

                  if ((eo.ctrl < M_NT(M_A,0)) || (eo.ctrl > M_NT(M_C,8)))
                      durNt = 0;

                  if (durNt) {
                     StrFmt (& dbg [StrLn (dbg)],  "`<3s `s `d",
                             MKey2Str (ts, (ubyte)ev [e].ctrl),
                             (ev [e].valu & 0x0080) ? "Dn" : "Up",
                              ev [e].valu & 0x007F);
                     eo.time = tm + d + ev [e].time - tmCl;
                     eo.valu = ev [e].valu;
                     eo.val2 = ev [e].val2;

                     otrk = DB->sty [st-1].trk [tr].ot;
                     if (!    op [otrk]) {
                        op [otrk] = true;
                        StrFmt (ts, "`s\\`03d.ev", App.Path (t2, 'e'), otrk);
//DBG("        opening `s", ts);
                        if (! fo [otrk].Open (ts, "w")) Die ("can't write", ts);
                     }
                     fo [otrk].Put (& eo, sizeof (eo));
//TStr tt;
//DBG("        otrk=`02d ntDn ev.time=`s eo.time(tm+d+ev.time-tmCl)=> `s",
//otrk, TmSt(TMS,ev [e].time), TmSt(tt,eo.time));
//DBG(dbg);
                     eo.time += durNt;   eo.valu = ev [g].valu;
                                         eo.val2 = ev [g].val2;
                     if (eo.time >= tm+dur)  eo.time = tm+dur-1;
//DBG("                ntUp eo.time=`s  durNt=`d", TmSt(tt,eo.time), durNt);
                     fo [otrk].Put (& eo, sizeof (eo));
                  }
               }
            }
         }
         tmCl += durCl;
         if (tmCl >= (DB->sty [st-1].dur)) {     // bump st via styPos,stySeq
            tmCl = 0;
            styPos++;   if (! stySeq [styPos])  styPos = 1;     // inc;wrap
            *sty = '.';   sty [1] = stySeq [styPos];   sty [2] = '\0';
//DBG("  sty bump styseq='`s' pos=`d sty=`s", stySeq, styPos, sty);
            for (st = 0;  st < DB->sty.Ln;  st++)
               if (! MemCm (sty, DB->sty [st].id, 2))  break;
            if (st >= DB->sty.Ln)  st = 0;
            else {
               st++;
            // sty in list could start off beat, bump tmCl if so
               if (        tm % (DB->sty [st-1].dur))
                  tmCl += (tm % (DB->sty [st-1].dur));
               TSigN = DB->sty [st-1].tsN;
               TSigD = DB->sty [st-1].tsD;
            }
//DBG("  NEXT STYLE styseq='`s' pos=`d sty=`s st=`d tmCl=`s",
//stySeq, styPos, sty, st, TmSt(TMS,tmCl));
            if (st == 0)  {
//DBG("  exit loop cuz st=0");
               break;
            }
         }
//DBG("    BUMP tmCl=`s", TmSt(TMS,tmCl));
      }
   }

// write .song-ish file n close em down
   for (i = 0;  i < BITS (fo);  i++)  if (op [i]) {
      fo [i].Shut ();
      f.Put (StrFmt (ts, "`03d `s sty `s\r\n",
             i, DB->trk [i].snd, & DB->trk [i].name [1]));     // skip ./+
//DBG("Sty2Song save out '`s'", ts);
   }
   f.Shut ();

// free up ev mem that songLoad() alloc'd
   for (st = 0;  st < DB->sty.Ln;  st++)  delete [] DB->sty [st].e;
//DBG("} Save");
}


//------------------------------------------------------------------------------
int Go ()
{ StrArr sa;
  TStr   fn;                                // open up
   InitCom ();

   App.Path (fn, 'e');   StrAp (fn, "\\sty.in");
   sa.Init ("sty", 4*26);   sa.Load (fn);   // load fns w style clips
   DB = new db;
//DBG("sty.in has...");   sa.Dump ();
   for (ulong i = 0;  i < sa.NRow ();  i++)        Load (sa.Get (i));  // sty cl
   App.Path (fn, 'e');   StrAp (fn, "\\chd.in");   Save (fn);
                                            // loop thru tm,chd,sty n write it
   delete DB;       QuitCom ();             // shut dn
   return 0;
}
