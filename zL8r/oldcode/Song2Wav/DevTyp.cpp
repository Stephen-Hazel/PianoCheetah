// DevTyp.cpp - for Song2Wav...:/

#include "Song2Wav.h"


char *DevTyp::SnRec (char *buf, uword len, ulong pos, void *ptr)
// parse a record of the sound.txt file
{ static TStr Msg;
  DevTyp     *d = (DevTyp *)ptr;
   if (*buf == '#')  return NULL;      // comment?

  SpaceSep ss (buf, 4);                // got all the cols we need?
   if (ss.Col[3][0] == '\0')
      Die ("DevTyp::SnRec",
           StrFmt (Msg, "sound.txt for `s line `d missing cols",
                        d->_name, pos+1));
  ulong n = d->_nSn;                   // outa room?
   if (n >= BITS (d->_sn))
      Die ("DevTyp::SnRec",
           StrFmt (Msg, "sound.txt for `s is > `d lines",
                        d->_name, BITS (d->_sn)));
   StrCp (d->_sn [n].name, ss.Col[0]);
// get prog/bank/bnkL or default
   d->_sn [n].prog = (ss.Col [1][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[1]);
   d->_sn [n].bank = (ss.Col [2][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[2]);
   d->_sn [n].bnkL = (ss.Col [3][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[3]);
   d->_nSn++;
   return NULL;
}


char *DevTyp::CcRec (char *buf, uword len, ulong pos, void *ptr)
// parse a record of the ccout.txt file
{ static TStr Msg;
  DevTyp     *d = (DevTyp *)ptr;
// outa room?
   if (pos >= 128)
      Die ("DevTyp::CcRec",
           StrFmt (Msg, "ccout.txt for `s is > 128 lines", d->_name));
  SpaceSep ss (buf, 2);
// got all the cols we need?
   if (ss.Col[1][0] == '\0')
      Die ("DevTyp::DoCc",
           StrFmt (Msg, "ccout.txt for `s line `d missing cols",
                        d->_name, pos+1));
   StrCp (d->_cc [d->_nCc].map, ss.Col[0]);
   d->_cc [d->_nCc].raw = MCtl2Int (ss.Col[1]);
           d->_nCc++;
   return NULL;
}


DevTyp::DevTyp (): _nSn (0), _nDr (0), _nCc (0)  {}


void DevTyp::Open (char *devTyp)
// cache devTyp's sound.txt and ccout.txt files into mem
{ TStr fn;
  File f;
DBG("{ DevTyp::Open");
   StrCp (_name, devTyp);
   App.Path (fn, 'd');   StrAp (fn, "\\Device\\");   StrAp (fn, devTyp);
                                                     StrAp (fn, "\\sound.txt");
   f.DoText (fn, this, SnRec);
   while (MemCm (_sn [_nDr].name, "Drum\\", 5) == 0)  _nDr++;

   App.Path (fn, 'd');   StrAp (fn, "\\Device\\");   StrAp (fn, devTyp);
                                                     StrAp (fn, "\\ccout.txt");
   f.DoText (fn, this, CcRec);
DBG("} DevTyp::Open");
}


void DevTyp::Shut ()  {_nDr = _nSn = 0;  _nCc = 0;}


ulong DevTyp::SndID (char *s)
// map name to _sn index;   else return SND_NONE
{ ulong i, ln;
  TStr  nm;
  char *p, *p2;
   StrCp (nm, s);
DBG("DevTyp::SndID `s `s nDr=`d nSn=`d", _name, nm, _nDr, _nSn);
// gotta drum?
   if (! MemCm (nm, "Drum", 4)) {
      for (i = 0; i < _nDr; i++)  if (! StrCm (nm, _sn [i].name)) {
DBG("=> `d `s", i, _sn [i].name);
         return i;
      }

   // chop off at trailing \ while we got em and try that
      while (p = StrCh (nm, '\\')) {
         while (p2 = StrCh (p+1, '\\'))  p = p2;      // get last one into p
         *p = '\0';
         for (i = 0; i < _nDr; i++)
            if (! MemCm (nm, _sn [i].name, StrLn (nm))) {
DBG("=> `d `s", i, _sn [i].name);
               return i;
            }
      }
DBG("=> SND_NONE");
      return SND_NONE;
   }

// got melodic sound.
   for (i = _nDr; i < _nSn; i++)  if (! StrCm (nm, _sn [i].name))  {
DBG("=> `d `s", i, _sn [i].name);
      return i;
   }

// if we have a GS/XG .999999 sound, chop .999999 and look again...:/
   if (((ln = StrLn (nm)) > 7) && (nm [ln-7] == '.')) {
      nm [ln-7] = '\0';
      for (i = _nDr; i < _nSn; i++)  if (! StrCm (nm, _sn [i].name)) {
DBG("=> `d `s", i, _sn [i].name);
         return i;
      }
   }

// chop off at trailing \ while we got em and try that
   while (p = StrCh (nm, '\\')) {
      while (p2 = StrCh (p+1, '\\'))  p = p2;     // get last one into p
      *p = '\0';
      for (i = _nDr; i < _nSn; i++)
         if (! MemCm (nm, _sn [i].name, StrLn (nm))) {
DBG("=> `d `s", i, _sn [i].name);
            return i;
         }
   }

// give up
DBG("=> SND_NONE");
   return SND_NONE;
}


void DevTyp::Dump ()
{  DBG(" name=`s nSn=`d nDr=`d nCc=`d", _name,  _nSn,  _nDr,  _nCc);
/*
  ulong i;
  TStr  s;
   DBG("      snd prog bank bnkL name,desc");
   for (i = 0; i < _nSn; i++)
      DBG(" `>8d `>4d `>4d `>4d `s",
         i, _sn [i].prog, _sn [i].bank, _sn [i].bnkL, _sn [i].name);
   DBG("  cc map      raw");
   for (i = 0; i < _nCc; i++)
      DBG(" `>3d `<8s `s",
         i, _cc [i].map, MCtl2Str (s, _cc [i].raw));
   DBG(" CCMap raw");
   for (i = 0; i < 128; i++) DBG("   `>3d `s", i, MCtl2Str (s, CCMap [i]));
*/
}
