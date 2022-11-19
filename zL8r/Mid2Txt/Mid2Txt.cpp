// Mid2Txt.cpp - pull all the text out of a midi file to give it a decent fname

#include "ui.h"
#include "MidiIO.h"

ubyte Mid [1024*1024];                 // mem to hold the whole .mid file
ulong MidLn, MidP;                     // len of buf and pos we're parsin' at
TStr  TMS;                             // useful globals...:/
char *Desc  [] = {"SeqNo","Text","Copyright","Track","Instrument",
                  "Lyric","Marker","Cue","Sound","Device",
                  "Text10","Text11","Text12","Text13","Text14","Text15"},
      StrBuf [64*1024], BUF [64*1024];
File  F;

// track readin dudes ----------------------------------------------------------
bool GetByt (ulong *p, ulong l, ubyte *it)
{ static TStr err;
   if (*p >= l) {
F.Put (StrFmt (BUF, "   EOF on byte...:(p=`d l=`d)\r\n", *p, l));
      return false;
   }
   *it = Mid [MidP + *p];  (*p)++;
   return true;
}

bool GetVar (ulong *p, ulong l, ulong *it)
// Read oneuh those wierd variable length numbers...
{ ulong val = 0, len = 0;
  ubyte b;
  static TStr err;
   do {
      if (*p >= l) {
F.Put (StrFmt (BUF, "   EOF on VarLen...:(p=`d l=`d)\r\n", *p, l));
         return false;
      }
      val = (val << 7) | ((b = Mid [MidP + (*p)++]) & 0x7F);
      if (++len > 4) {
F.Put (StrFmt (BUF, "   Bad VarLen...:(p=`d l=`d len=`d)\r\n", *p, l, len));
         return false;
      }
   } while (b & 0x80);
   *it = val;
   return true;
}

bool Skip (ulong *p, ulong l, ulong skp)
{  if ((*p += skp) >= l)  {F.Put ("   EOF on skip");  return false;}
   return true;
}

void FixStrX (char *s)
{  while (*s) {
      if      ((*s == '\r') || (*s == '\n') ||
               (*s == '/')  || (*s == '\\'))  *s = '/';
      else if ((*s < ' ') || (*s > '~'))      *s = '_';
      s++;
   }
}


//------------------------------------------------------------------------------
void Trk2Ev (ubyte tp, uword res, ulong l)
// load a midi track into e and print text info, etc in song file
{ ulong p, hmmlen, ln, x,       // parse pos (offset from MidP)
        delta, ftime, time;     // delta => filetime*res => time
  ubyte pStat, c, tc;           // midi running status byte
  uword chan;
// init filetime and running status
   ftime = 0;   pStat = 0;   chan = 0;

   for (p = 0;  p < l;) {
      if (! GetVar (& p, l, & delta))  return;
      ftime += delta;  time = (ftime * 192) / res;
//DBG("   trk=`d MidP=`d(p=`d/l=`d) delta=`d res=`d=> time=`s",
//tp, MidP+p, p, l, delta, res, TmS(TMS,time));
      if (! GetByt (& p, l, & c))  return;
//DBG("    c=$`02X", c);
      if (c == 0xFF) {
      // parse those damn midifile "meta" events
      // 0=SeqNo, 1=Text, 2=Copyright, 3=SeqName/TrackName, 4=Instrument,
      // 5=Lyric, 6=Marker, 7=CuePoint, 8=Program(Patch), 9=Device(Port),
      // $20=cakewalkChan, $21=cakewalkPort,
      // $2F=EndOfTrack, $51=Tempo, $54=SMPTEOffset, $58=TimeSig, $59=KeySig
         pStat = 0;
         if (! GetByt (& p, l, & c))  return;
         if (! GetVar (& p, l, & hmmlen))  return;
         if ((p + hmmlen) > l)
            {F.Put ("   EOF on $FF??\r\n");   return;}
         switch (c) {
            case 0:   //...sequence number
               break;
            case 1:   case 2:   case 3:   case 4:   case 5:
            case 6:   case 7:   case 8:   case 9:   case 10:
            case 11:  case 12:  case 13:  case 14:  case 15:
            // all the text events
//DBG("   txtc=`d txtln=`d", c, hmmlen);
               MemCp (StrBuf, & Mid [MidP+p], hmmlen);
               StrBuf [hmmlen] = '\0';
               FixStrX (StrBuf);
               if (time > M_WHOLE*5/2)  break;
//StrFmt(BUF, "   `02d `s `s `s\r\n", tp+1, TmS (TMS, time), Desc [c], StrBuf);
F.Put (StrFmt (BUF, "   `s\r\n", StrBuf));
               break;
            case 0x20: // INVALID(yet used) midi channel prefix (chan LSB)
               break;
            case 0x21: // INVALID(yet used) midi port prefix    (chan MSB)
               break;
            case 0x2F: //...End track - end time is set (REQUIRED)
               break;
            case 0x51: //...Tempo
               break;
            case 0x54: //...SMPTE offset
               break;
            case 0x58: //...TimeSignature
               break;
            case 0x59: //...KeySignature
               break;
            case 0x7F:
               break;
//          default:
//F.Put (StrFmt(BUF, "   `02d `s $FF`02X(Len `d)\r\n",
//tp+1, TmS (TMS, time), (int)c, hmmlen));
         }
         p += hmmlen;
      }
      else if ((c == M_SYSX) || (c == M_EOX)) {  // ...toss sysex
         if (! GetVar (& p, l, & hmmlen))  return;
         ln = (hmmlen > (sizeof (StrBuf)-2)) ? (sizeof (StrBuf)-2) : hmmlen;
         MemCp (StrBuf, & Mid [MidP+p], ln);
         StrBuf [ln] = '\0';
         for (x = 0; x < ln-1; x++)  if (StrBuf [x] == '\0')  StrBuf [x] = '_';
         FixStrX (StrBuf);
//StrFmt(BUF, "   `02d `s SYSEX `s\r\n", tp+1, TmS (TMS, time), StrBuf);
F.Put (StrFmt (BUF, "   `s\r\n", StrBuf));
         if (! Skip   (& p, l,   hmmlen))  return;
      }
      else if (c > M_SYSX) {                     // ...toss realtime msg
         pStat = 0;
//DBG("   `02d SysRealtime Cmd $`02X tossed\r\n", tp+1, (int)c);
         if (c == M_SONGPOS)  if (! Skip (& p, l, 2))  return;
         if (c == M_SONGSEL)  if (! Skip (& p, l, 1))  return;
      }
      else {
         if (c & 0x80)
            {pStat = c;   if (! GetByt (& p, l, & c))  return;}
         chan = (chan & 0xFFF0) | (pStat & 0x0F);
         switch (pStat & 0xF0) {
            case M_NOTE: if (! GetByt (& p, l, & tc)) return;   break;
            case M_NPRS: if (! GetByt (& p, l, & tc)) return;   break;
            case M_NOFF: if (! GetByt (& p, l, & tc)) return;   break;
            case M_PROG: break;
            case M_PRSS: break;
            case M_PBND: if (! GetByt (& p, l, & tc)) return;   break;
            case M_CTRL: if (! GetByt (& p, l, & tc)) return;   break;
//          default:
//StrFmt(BUF, "   `02d `s pos=`d badMIDIevent=$`02X`02X\r\n",
//tp+1, TmS (TMS, time), MidP+p, (int)pStat, (int)c);
//F.Put  (BUF);
         }
      }
   }
}


//------------------------------------------------------------------------------
void DoMid ()
// parse the whole midi file, then reparse and write out .song n .trak files
{ ulong tlen;                // len isn't really PART of the header info
  ubyte tp;
  struct {ulong len;  uword fmt, ntrk, res;} MThd;
   if (StrCm ((char *)Mid, "MThd", 'x'))
      {F.Put ("   (bad MThd header)\r\n");   return;}
   MThd.len  = Mid [4]<<24 | Mid [5]<<16 | Mid [6]<<8 | Mid [7];
   MThd.fmt  = Mid [8] <<8 | Mid [9];
   MThd.ntrk = Mid [10]<<8 | Mid [11];
   MThd.res  = Mid [12]<<8 | Mid [13];
   if (MidLn < (MidP = 8 + MThd.len))
      {F.Put ("   (No .mid hdr)\r\n");   return;}
   for (tp = 0;  tp < MThd.ntrk;  tp++, MidP += tlen) {
      if (StrCm ((char *)(& Mid [MidP]), "MTrk", 'x'))
         {F.Put ("   (Bad trk hdr)\r\n");   return;}
      tlen = Mid [MidP+4]<<24 | Mid [MidP+5]<<16 | Mid [MidP+6]<<8  |
                                                   Mid [MidP+7];
      if (MidLn < ((MidP += 8) + tlen))
         {F.Put ("   (track size goes beyond EOF)\r\n");   return;}
//DBG("tr=`d/`d", tp, MThd.ntrk);
      Trk2Ev (tp, MThd.res, tlen);
   }
}


//------------------------------------------------------------------------------
TStr Dir, FNm [350*1024];   ulong NFNm;

void Find (char *dir)
// find any song files and put em in _list
{ char            src [MAX_PATH];
  HANDLE          fh;
  WIN32_FIND_DATA ff;
// find it all (FindFirstFile dies if dir ain't there)
   StrFmt (src, "`s\\*.*", dir);
   if ((fh = ::FindFirstFile (src, & ff)) == INVALID_HANDLE_VALUE)
      return;
   do {
      StrFmt (src, "`s\\`s", dir, ff.cFileName);
      if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
         if (StrCm (ff.cFileName, ".") && StrCm (ff.cFileName, ".."))
            Find (src);
      }
      else {                           // add mid files to list
         if ((StrLn (src) > 3) && PosInZZ (& src [StrLn (src)-3],
             "mid\0kar\0rmi\0sty\0bcs\0prs\0sst\0pcs\0pst\0fps\0")) {
            if (NFNm >= BITS (FNm))  Die ("Find - too many files :(");
            StrCp (FNm [NFNm++], src);
         }
      }
   } while (::FindNextFile (fh, & ff));
   if (! ::FindClose (fh))  DieWn ("Find  FindClose");
}

int Go ()
{ TStr  fn, fnb;
  File  f;
  ulong i, ln;
   if (! f.AskDir (Dir, "Pick a dir with MIDI files in it"))  return 0;
   Find (Dir);
   StrCp (fnb, Dir);   StrAp (fnb, "\\fix.bat");
   if (! F.Open (fnb, "w"))  return 0;

   for (i = 0;  i < NFNm;  i++) {
F.Put (StrFmt (BUF, "ren \"`s\" \"\r\n", FNm [i]));
      StrCp (fn, FNm [i]);
      if ((MidLn = f.Load (fn, Mid, sizeof (Mid))) == 0)
                                       Die ("Mid2Txt  .mid not found", fn);
      if (MidLn == BITS (Mid))  {F.Put ("   (.mid too big)\r\n");   continue;}
      if (MemCm ((char *)Mid, "RIFF", 4, 'x') == 0)  // damn .RMI files...
         MemCp (Mid, & Mid [20], MidLn -= 20);
      DoMid ();
      StrAp (fn, "txt", 3);
      if (ln = f.Load (fn, StrBuf, sizeof (StrBuf))) {
         StrBuf [ln] = '\0';   FixStrX (StrBuf);
F.Put (StrFmt (BUF, "   `s\r\n", StrBuf));
      }
   }
   F.Shut ();
   RUN ("edit", fnb);
   return 0;
}
