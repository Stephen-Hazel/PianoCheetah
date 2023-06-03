// sDevTyp.cpp - device type stuff for Song

#include "song.h"

char *DevTyp::SnRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr)
// parse a record of the sound.txt file
{ static TStr Msg;
  DevTyp     *d = (DevTyp *)ptr;
   (void)len;
   if (*buf == '#')  return NULL;      // comment?

  ColSep ss (buf, 4);                  // got all the cols we need?
   if (ss.Col [3][0] == '\0') {
      StrFmt (Msg, "sound.txt for `s line `d missing cols", d->_name, pos+1);
      DBG("DevTyp::SnRec `s", Msg);
   }
  ubyt4 n = d->_sn.Ins ();             // outa room?
   if (n >= d->_sn.Mx) {
      StrFmt (Msg, "sound.txt for `s is > `d lines", d->_name, d->_sn.Mx);
      DBG("DevTyp::SnRec `s", Msg);
   }
   StrCp (d->_sn [n].name, ss.Col [0]);     // get prog/bank/bnkL or default
   d->_sn [n].prog = (ss.Col [1][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[1]);
   d->_sn [n].bank = (ss.Col [2][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[2]);
   d->_sn [n].bnkL = (ss.Col [3][0] == '.') ? PRG_NONE :
                     (ubyte) Str2Int (ss.Col[3]);
   return NULL;
}


char *DevTyp::CcRec (char *buf, ubyt2 len, ubyt4 pos, void *ptr)
// parse a record of the ccout.txt file
{ static TStr Msg;
  DevTyp     *d = (DevTyp *)ptr;
   (void)len;
// outa room?
   if (pos >= d->_cc.Mx) {
      StrFmt (Msg, "ccout.txt for `s is > `d lines", d->_name, d->_cc.Mx);
      DBG("DevTyp::CcRec `s", Msg);
   }
   if (*buf == '#')  return NULL;
  ColSep ss (buf, 2);
// got all the cols we need?
   if (ss.Col [1][0] == '\0') {
      StrFmt (Msg, "ccout.txt for `s line `d  missing cols", d->_name, pos+1);
      DBG("DevTyp::CcRec `s", Msg);
   }
   if (StrLn (ss.Col [0]) > MAXWSTR) {
      StrFmt (Msg, "ccout.txt for `s line `d  map cc too long",
                                                              d->_name, pos+1);
      DBG("DevTyp::CcRec `s", Msg);
   }
  ubyte n = (ubyte) d->_cc.Ins ();
   StrCp (d->_cc [n].map,     ss.Col [0]);
   d->_cc [n].raw = MCtl (ss.Col [1]);
   return NULL;
}


DevTyp::DevTyp ()  {_nDr = 0;   _name [0] = '\0';}


void DevTyp::Open (char *devTyp)
// cache devTyp's sound.txt and ccout.txt files into mem
{ TStr fn;
  File f;
TRC("DevTyp::Open `s", devTyp);
   StrCp (_name, devTyp);   _nDr = _sn.Ln = _cc.Ln = 0;

   App.Path (fn, 'd');   StrAp (fn, CC("/device/"));   StrAp (fn, devTyp);
   StrAp (fn, CC("/sound.txt"));
   f.DoText (fn, this, SnRec);

   while ((_nDr < _sn.Ln) &&
          (MemCm (_sn [_nDr].name, CC("Drum/"), 5) == 0))  _nDr++;

   App.Path (fn, 'd');   StrAp (fn, CC("/device/"));   StrAp (fn, devTyp);
   StrAp (fn, CC("/ccout.txt"));
   f.DoText (fn, this, CcRec);
Dump ();
}


void DevTyp::Shut ()
{
TRC("DevTyp::Shut `s", _name);
   _name [0] = '\0';   _nDr = _sn.Ln = _cc.Ln = 0;
}


ubyt4 DevTyp::SndID (char *s, bool newgrp, bool xmatch)
// map name to _sn index;   else return SND_NONE
// newgrp means find 1st snd in the group;  xmatch means no resolving;
{ ubyt4 i, ln;
  char *p, *p2;
  TStr  nm;
   StrCp (nm, s);
TRC("DevTyp::SndID `s `s nDr=`d nSn=`d", _name, nm, _nDr, _sn.Ln);
// gotta drum?
   if (! MemCm (nm, CC("Drum"), 4)) {
      for (i = 0; i < _nDr; i++)
         if ( (! StrCm (nm, _sn [i].name)) ||
              (newgrp && (! MemCm (nm, _sn [i].name, StrLn (nm)))) ) {
//DBG("a=> `d `s", i, _sn [i].name);
            return i;
         }
//DBG("b=> NONE");
      if (xmatch)  return SND_NONE;    // no resolving

   // chop off at trailing _ then trailing / while we got em and try that
      if ((p = StrCh (nm, '_')))  *p = '\0';
      for (;;) {
//DBG("find `s", nm);
         for (i = 0; i < _nDr; i++) {
            if (! MemCm (nm, _sn [i].name, StrLn (nm))) {
//DBG("b=> `d `s", i, _sn [i].name);
               return i;
            }
         }
         if ((p = StrCh (nm, '_'))) {
            while ((p2 = StrCh (p+1, '_')))  p = p2;  // get last one into p
            *p = '\0';
         }
         else break;
      }
   // give up - use whatever drums ya got  (usually Drum/Kick_Kick / Drum/*)
      return 0;
   }

// got melodic sound.
   for (i = _nDr; i < _sn.Ln; i++)
      if ( (! StrCm (nm, _sn [i].name)) ||
           (newgrp && (! MemCm (nm, _sn [i].name, StrLn (nm)))) ) {
//DBG("d=> `d `s", i, _sn [i].name);
         return i;
      }

// if we have a GS/XG .999.999 sound, see if dev type has it, else xmatch over
   if (((ln = StrLn (nm)) > 8) && (nm [ln-8] == '.') && (nm [ln-4] == '.')) {
      nm [ln-8] = '\0';
     ubyte msb, lsb;
      msb = (ubyte) Str2Int (& nm [ln-7]);
      lsb = (ubyte) Str2Int (& nm [ln-3]);
      for (i = _nDr; i < _sn.Ln; i++)
         if ( (! MemCm (nm, _sn [i].name, StrLn (nm))) &&
              (_sn [i].bank == msb) && (_sn [i].bnkL == lsb) ) {
//DBG("e=> `d `s", i, _sn [i].name);
         return i;
      }
   }
   if (xmatch)  return SND_NONE;

// chop off at trailing _ then trailing / while we got em and try that
   if ((p = StrCh (nm, '_')))  *p = '\0';
   for (;;) {
//DBG("find `s", nm);
      for (i = _nDr; i < _sn.Ln; i++) {
         if (! MemCm (nm, _sn [i].name, StrLn (nm))) {
//DBG("f=> `d `s", i, _sn [i].name);
            return i;
         }
      }
      if ((p = StrCh (nm, '_'))) {
         while ((p2 = StrCh (p+1, '_')))  p = p2;     // get last one into p
         *p = '\0';
      }
      else break;
   }

// give up - use 1st sound (Piano/AcousticGrand for GM, or whatever ya got)
   return _nDr;  // SndID ("Piano/AcousticGrand");
}


bool DevTyp::SndNew (ubyt4 *newp, ubyt4 pos, char ofs)
// bump sound pos by 1;  return false if ya can't
{ ubyt4 b, e;
   b = 0;   e = _sn.Ln;                // are we in drum list or melo?
   if (pos < _nDr)  e = _nDr;   else b = _nDr;
   if (pos >= e)  pos = b;
   if (ofs)  {if (++pos == e)  return false;}
   else      {if (pos-- == b)  return false;}
   *newp = pos;                return true;
}


void DevTyp::SGrp (char *t)            // to get snd grp (dirs)
{ ubyt4 i;
  TStr  s, ps;
  char *p, *t1;
   t1 = t;
   *t = '\0';
   for (i = 0;  i < _sn.Ln;  i++) {
      StrCp (s, _sn [i].name);
      if (! MemCm (s, CC("Drum/"), 5))  continue;
      if ((p = StrCh (s, '_')))  *p = '\0';
      if (StrCm (ps, s))  {StrCp (t, s);   t += (StrLn (s)+1);   StrCp (ps, s);}
   }
   *t = '\0';
//DBG("DevTyp::SGrp t:"); DbgX(t1,'z');
}

void DevTyp::SNam (char *t, char *grp) // pop snd list for grp
{ ubyt4 i;
  TStr  s;
  char *p, *t1;
   t1 = t;
//DBG("DevTyp::SNam grp=`s", grp);
   for (i = 0;  i < _sn.Ln;  i++) {
      StrCp (s, _sn [i].name);
      if (! MemCm (s, CC("Drum/"), 5))  continue;
      if (! MemCm (s, grp, StrLn (grp)))
         {p = & s [StrLn (grp)+1];   StrCp (t, p);   t += (StrLn (p)+1);}
   }
   *t = '\0';
//DBG("DevTyp::SNam t:"); DbgX(t1,'z');
}


void DevTyp::Dump ()
{  DBG("   nCc=`d nSn=`d nDr=`d", _cc.Ln, _sn.Ln, _nDr);
/*
  TStr  s;
  ubyt4 i;
   DBG("  cc map      raw");
   for (i = 0;  i < _cc.Ln;  i++)
      DBG(" `>3d `<8s `s", i, _cc [i].map, MCtl2Str (s, _cc [i].raw));
   DBG("      snd prog bank bnkL name,desc");
   for (i = 0;  i < _sn.Ln;  i++)
      DBG(" `>8d `>4d `>4d `>4d `s",
          i, _sn [i].prog, _sn [i].bank, _sn [i].bnkL, _sn [i].name);
   DBG("  CCMap");
   for (i = 0;  i < 128;  i++)  if (CCMap [i])
      DBG("  `>3d `s", i, MCtl2Str (s, CCMap [i]));
*/
}
