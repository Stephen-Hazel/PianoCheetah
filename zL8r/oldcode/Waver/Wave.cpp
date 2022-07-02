// Wave.cpp

#include "Waver.h"


char *Wave::PosStr (ulong p)
{ static char  buf [200];
  ulong m, x;
   x = p;
   *buf = '\0';
   if (_frq) {
      m = p / (_frq*60);   x = p % (_frq*60);
      if (m)  StrFmt (& buf [StrLn (buf)], "`d:",   m);
   }
   StrFmt (           & buf [StrLn (buf)], "`05d",  x);
   return buf;
}


slong Wave::Smp (ulong s, ubyte lr)
// get pos in the left/right sample as a uword (0-65535)
{ static char buf [161];
  ubyte *p = (ubyte *)_mem;
  slong  smp = 0;
   if (s >= _len)
      {StrFmt (buf, "Smp past Len!  (`d>=`d)\n", s, _len);   Die (buf);}
   p += (s * ((_mono?1:2)*_byts));   if (lr)  p += _byts;
   switch (_byts) {
      case 1:  smp = ((slong)(*p++) - 128) << 24;          break;
      case 3:  smp |= *p++ <<  8;
      case 2:  smp |= *p++ << 16;   smp |= (*p++ << 24);   break;
      case 4:  smp = *((slong *)p);   p += 4;              break;
   }
//DBG("Smp pos=`d lr=`d => `d", s, lr, smp);
   return smp;
}


//------------------------------------------------------------------------------
void Wave::Wipe ()  // wipe all data and "empty" display
{  _wo.Shut ();  _mf.Shut ();
   _name [0] = '\0';  _mem = NULL;  _len = 0;
   MemSet (& _fmt, 0, sizeof (_fmt));   _smp = NULL;
}

void Wave::Puke (char *m1, char *m2)  {Hey (m1, m2);  Wipe ();}

void Wave::Load (char *fn)
{ ubyte *p, *pb, *pe;
  ulong  l;
  struct {char ckID [4];  ulong ckSize;} chnk;
  char   id [4];
  TStr   ts;
  bool   got [3];
  slong  sm, mn, mx;
   Wipe ();
   StrCp (_name, fn);
   App.SetTitle ("Waver", _name);
   MemSet (got, 0, sizeof (got));
   if ((pb = (ubyte *) _mf.Open (fn)) == NULL)
                                return Puke ("Wave::Load  Can't read file", fn);
   p = pb;
   if (_mf.LenH ())             return Puke ("Wave::Load  file TOO big", fn);
   if ((l = _mf.Len ()) < 12)   return Puke ("Wave::Load  not a WAV file", fn);
   MemCp (& chnk, p, 8);   MemCp (id, p+8, 4);   p += 12;  l -= 12;
   if (MemCm (chnk.ckID, "RIFF", 4, 'x'))
                                return Puke ("Wave::Load  not a WAV file", fn);
   if (MemCm (id,        "WAVE", 4, 'x'))
                                return Puke ("Wave::Load  not a WAV file", fn);
   for (pe = p + l;  p < pe;) {
//DBG("p=`08x=`d  ofs=`08x=`d", p, p, (int)(p-pb), (int)(p-pb));
      if (p + 8 > pe)  break;          // hit end?  bail
      MemCp (& chnk, p, 8);  p += 8;
      if (chnk.ckSize & 0x80000000)  break; // bail on rogue ckSize
      if (p + chnk.ckSize > pe)  break;
      if      (MemCm (chnk.ckID, "fmt ", 4, 'x') == 0) {
         got [0] = true;
         MemCp (& _fmt, p, sizeof (_fmt));
      }
      else if (MemCm (chnk.ckID, "data", 4, 'x') == 0) {
         got [1] = true;
         _mem = p;   _len = chnk.ckSize;
      }
      else if (MemCm (chnk.ckID, "smpl", 4, 'x') == 0) {
         got [2] = true;
         _smp = (WAVESMPL *) p;
      }
      p += EVEN_UP (chnk.ckSize);
   }
   if (! got [0])
      return Puke ("Wave::Load  couldn't find fmt chunk", fn);
   if (! got [1])
      return Puke ("Wave::Load  couldn't find data chunk", fn);
   _frq  = _fmt.nSamplesPerSec;
   _bits = (char)_fmt.wBitsPerSample;
   _mono = (_fmt.nChannels == 1) ? true : false;
   if (_fmt.nChannels > 2)
      return Puke ("Wave::Load  can't do multichannel beyond stereo");
   if (_fmt.wBitsPerSample > 32)
      return Puke ("Wave::Load  can't do beyond 32 bit samples");
   if      (_bits > 24)  _byts = 4;
   else if (_bits > 16)  _byts = 3;
   else if (_bits >  8)  _byts = 2;
   else                  _byts = 1;
   _len /= (_byts*(_mono?1:2));
   _bgn = 0;   _end = _len - 1;
   if (got [2]) {
      _loop = true;    _lBgn  = _smp->bgn;
                       _lEnd  = _smp->end - 1;
                       _lBgn2 = _smp->bgn/(_byts*(_mono?1:2));
                       _lEnd2 = _smp->end/(_byts*(_mono?1:2)) - 1;
                       if (_lBgn2 > _end)  _lBgn2 = _lBgn;
                       if (_lEnd2 > _end)  _lEnd2 = _lEnd;
      _key = (ubyte)_smp->key;
      _cnt = (ubyte)((ulong)_smp->cnt / ((ulong)0x80000000/50));
      if (_lBgn > _lEnd)  { ulong t = _lEnd;   _lEnd = _lBgn;   _lBgn = t;}
      if (_lBgn > _end)  _lBgn = _end;
      if (_lEnd > _end)  _lEnd = _end;
   }
   else {
      _loop = false;   _lBgn = _lBgn2 = 0;   _lEnd = _lEnd2 = _end;
      _key = M_NT(M_C,4);        _cnt = 0;
   }
   _lBgn1 = _lBgn;   _lEnd1 = _lEnd;
   _c->loop.SetCheck (_loop);
   _c->bgn.Set  (PosStr ( _bgn));  _c->end.Set  (PosStr ( _end));
   _c->lBgn.Set (PosStr (_lBgn));  _c->lEnd.Set (PosStr (_lEnd));
   _c->lBgn.Show    (_loop);  _c->lEnd.Show    (_loop);
   _c->setLBgn.Show (_loop);  _c->setLEnd.Show (_loop);

   _c->frq.Set (_frq);
   _c->key.Set (MKey2Str (ts, _key));
   _c->cnt.Set (_cnt);

   mn = mx = Smp (0, 0);
   for (l = 1; l < _len; l++)
      {sm = Smp (l, 0);   if (sm < mn) mn = sm;   if (sm > mx) mx = sm;}
   if (! _mono)  for (l = 1; l < _len; l++)
      {sm = Smp (l, 1);   if (sm < mn) mn = sm;   if (sm > mx) mx = sm;}
  char inf [800];
   StrFmt (inf, "bits=`d min=`d max=`d len=`s",
      _bits, mn>>((4-_byts)*8), mx>>((4-_byts)*8), PosStr (_len));
   _c->info.Set (inf);
DBG(inf);
   _c->scrl.SetPos (0);
   SetPos (0);
   _wo.Open (& _fmt);
}


void Wave::Save (char *fni)
{ File  f;
  TStr  fn, fnb, fns, buf;
  ulong ln1, ln2, ln3, ln4, ln;
  WAVESMPL smp;
   if (fni == NULL) {
      if (_name [0] == '\0')  {Puke ("...no file to save");   return;}
      StrCp (fn, _name);
      Fn2Path (fn);   StrAp (fn, "\\____temp.wav");
   }
   else
      StrCp (fn, fni);
   _c->key.Get (buf, sizeof (buf));   _key = MKey2Int (buf);
   _c->cnt.Get (buf, sizeof (buf));   _cnt = (ubyte) Str2Int (buf);
   _c->frq.Get (buf, sizeof (buf));
   _fmt.nSamplesPerSec = Str2Int (buf);

// build smp from existing _smp if avail
   if (_smp)  MemCp  (& smp, _smp, sizeof (smp));
   else       MemSet (& smp, 0,    sizeof (smp));
   smp.per = 1000000000 / _frq;
   smp.key = _key;
   smp.cnt = (ulong)_cnt * (0x80000000/50);
   smp.num = 1;

   if (! _loop)  _lBgn = _lEnd = _end-_bgn+1;
   smp.bgn = (_lBgn     - _bgn);
   smp.end = (_lEnd + 1 - _bgn);

// calc lengths n dump into a file
   ln4 = sizeof (WAVESMPL);
   ln3 = _end - _bgn + 1;  if (ln3 == 0)  return;
   if (! _mono) ln3 <<= 1;   ln3 *= _byts;
   ln2 = 16;  // can't use sizeof (WAVEFORMATEX)?
   ln1 = 4 + 8 + ln2 + 8 + ln3;   ln1 += (8 + ln4); /* smpl too */
   if (! f.Open (fn, "w"))  return;
   f.Put ("RIFF");  f.Put (& ln1, 4);  f.Put ("WAVE");
   f.Put ("fmt ");  f.Put (& ln2, 4);  f.Put (& _fmt, ln2);
   f.Put ("data");  f.Put (& ln3, 4);
   f.Put (& ((ubyte *)_mem) [_bgn*(_mono?1:2)*_byts], ln3);
   f.Put ("smpl");  f.Put (& ln4, 4);  f.Put (& smp, ln4);
   f.Shut ();

   if (fni == NULL) {
      StrCp (fns, _name);              // save it - we're gonna Wipe()

   // chop .ext
      StrCp (fnb, _name);   Fn2Name (fnb);   ln = StrLn (fnb);

   // chop old @yyyymmdd...
      if ((ln >= 20) && (fnb [ln-20] == '@'))  fnb [ln-20] = '\0';

   // append new @yyyymmdd.hhmmss.day.mid
      StrAp (fnb, "@");   StrAp (fnb, Now (buf));   StrAp (fnb, ".WAV");
//DBG("fns=`s fnb=`s fn=`s _name=`s", fns, fnb, fn, _name);
      Wipe ();
      f.ReNm (fns, fnb);
      f.ReNm (fn,  fns);
      StrCp  (_name, fns);   Load (fns);
   }
}


void Wave::All ()
{ int at;
   _c->loop.SetCheck (true);
   if      ((_lBgn == _end  ) && (_lEnd == _end  ))  at = 1;
   else if ((_lBgn == 0     ) && (_lEnd == _end  ))  at = 0;
   else if ((_lBgn == _lBgn1) && (_lEnd == _lEnd1))  at = 2;
   else                                              at = 3;
   if (++at == 4)  at = 0;
   switch (at) {
      case 0:  _lBgn = _end;    _lEnd = _end;     break;
      case 1:  _lBgn = 0;       _lEnd = _end;     break;
      case 2:  _lBgn = _lBgn1;  _lEnd = _lEnd1;   break;
      default: _lBgn = _lBgn2;  _lEnd = _lEnd2;   break;
   }
   _c->lBgn.Set (PosStr (_lBgn));   _c->lEnd.Set (PosStr (_lEnd));
}


//#define NOISE  (38)  // max offset from 0 for silence picking
#define NOISE  (80)  // max offset from 0 for silence picking

void Wave::Chop ()
{ ulong p, ln;
  slong v, v2;
  ubyte s = 0, i, n = 1;
  ulong sB [100], sE [100], pBgn, pEnd;
  TStr  fn;
  File  f;
  char  dbg[800], d1[800], d2[800];
   if ((_name [0] == '\0') || (_len == 0))  return;

   for (p = 0; p < _len;) {
      v = Smp (p, 0);   v2 = _mono ? 0 : Smp (p, 1);
      if ( (v  >= (-NOISE)) && (v  <= NOISE) &&
           (v2 >= (-NOISE)) && (v2 <= NOISE) ) {
         sB [s] = p++;
         while (p < _len) {
            v = Smp (p, 0);   v2 = _mono ? 32768 : Smp (p, 1);
            if ( (v  >= (-NOISE)) && (v  <= NOISE) &&
                 (v2 >= (-NOISE)) && (v2 <= NOISE) )
                 p++;
            else break;
         }
         sE [s] = p;
         if ((p - sB [s]) >= _frq)  s++;  // hasta be at least a sec
         if (s >= BITS (sB)) {
            Hey ("Dang - can't chop - Too many silence blocks, dude");
            return;
         }
      }
      else p++;
   }
   if (s == 0) {Hey ("There ain't no silence blocks");  return;}

   pBgn = _bgn;  pEnd = _end;  // save em
   StrCp (fn, _name);   Fn2Path (fn);   ln = StrLn (fn);
   StrAp (fn, "\\chop.txt");
   if (! f.Open (fn, "w"))
      {Hey ("can't write chop.txt, man");  return;}
   if (sB [0] == 0) {_bgn = sE [0];   _end = sB [i = 1] - 1;}
   else             {_bgn = 0;        _end = sB [i = 0] - 1;}
   StrFmt (& fn [ln], "\\`02d.wav", n++);
   StrCp (d1, PosStr (_bgn));
   StrCp (d2, PosStr (_end));
   StrFmt (dbg,"fn=`02d bgn=`s end=`s\r\n", n-1, d1, d2);
   f.Put (dbg);
   Save (fn);
   while (i < s) {
      if (sE [i] >= _len)  break;
      StrFmt (& fn [ln], "\\`02d.wav", n++);
      _bgn = sE [i++];
      _end = (i < s) ? (sB [i] - 1) : (_len - 1);
      StrCp (d1, PosStr (_bgn));
      StrCp (d2, PosStr (_end));
      StrFmt (dbg,"fn=`02d bgn=`s end=`s\r\n", n-1, d1, d2);
      f.Put (dbg);
      Save (fn);
   }
   f.Shut ();
   _bgn = pBgn;  _end = pEnd;  // restore em
}


//------------------------------------------------------------------------------
void Wave::Play (char p)
{ ulong p1, clp = _frq * 5;  // 5 sec clip From/To _pos
   if (! _wo.On ()) return;
   if (p == '*') {
      p1 = _end;
      if (_bgn + _frq*60 < _end)  p1 = _bgn + _frq*60;  // max 60 secs
      _wo.Put (_mem, _len, _bgn, p1, _loop?1024:0, _lBgn, _lEnd);
   }
   else if (p == 'f') {
      p1 = _pos + clp;   if (p1 > _end)  p1 = _end;
      _wo.Put (_mem, _len, _pos, p1);
   }
   else if (p == 't') {
      if (clp >= _pos)  p1 = _bgn;
      else {p1 = _pos - clp;   if (p1 < _bgn)  p1 = _bgn;}
      _wo.Put (_mem, _len, p1, _pos);
   }
}

void Wave::Stop ()  {if (_wo.On ()) _wo.Clr ();}

void Wave::SetPos (ulong p)
{ char buf [200];
   _pos = p;
   StrFmt (buf, "pos=`s", PosStr (p));
   if (p < _len) {
      StrFmt (& buf [StrLn (buf)], " smp=`d", Smp (p, 0) >> (8*(4-_byts)));
      if (! _mono)  StrFmt (& buf [StrLn (buf)], "/`d", Smp (p, 1));
   }
   _c->pos.Set (buf);
}

void Wave::SetLp ()
{  _loop = _c->loop.Check ();
//   if (_loop) {
//      _c->lBgn.Set (PosStr (_lBgn = _bgn));
//      _c->lEnd.Set (PosStr (_lEnd = _end));
//   }
   _c->lBgn.Show    (_loop);  _c->lEnd.Show    (_loop);
   _c->setLBgn.Show (_loop);  _c->setLEnd.Show (_loop);
}

void Wave::SetMrk (char typ)
{  if (_pos >= _len)  return;
   switch (typ) {
   case '[': _bgn = _pos;
             if ( _bgn > _lBgn)  _lBgn =  _bgn;
             if ( _bgn > _lEnd)  _lEnd =  _bgn;
             if ( _bgn >  _end)   _end =  _bgn;   break;
   case ']': _end = _pos;
             if ( _end <  _bgn)   _bgn =  _end;
             if ( _end < _lBgn)  _lBgn =  _end;
             if ( _end < _lEnd)  _lEnd =  _end;   break;
   case '(': _lBgn = _pos;
             if (_lBgn <  _bgn)   _bgn = _lBgn;
             if (_lBgn > _lEnd)  _lEnd = _lBgn;
             if (_lBgn >  _end)   _end = _lBgn;   break;
   case ')': _lEnd = _pos;
             if (_lEnd <  _bgn)   _bgn = _lEnd;
             if (_lEnd < _lBgn)  _lEnd = _lEnd;
             if (_lEnd >  _end)   _end = _lEnd;   break;
   }
   _c->bgn.Set  (PosStr (_bgn));    _c->end.Set  (PosStr (_end));
   _c->lBgn.Set (PosStr (_lBgn));   _c->lEnd.Set (PosStr (_lEnd));
}


void Wave::NewMrk (char typ)
{  switch (typ) {
      case '[': _bgn  = _c->bgn.Get ();    break;
      case ']': _end  = _c->end.Get ();    break;
      case '(': _lBgn = _c->lBgn.Get ();   break;
      case ')': _lEnd = _c->lEnd.Get ();   break;
   }
}


Wave::Wave (CtlDef *c)
: _mem (NULL), _len (0), _c (c)
{  _name [0] = '\0';
// make a CourierNew 8 font
  Canvas cnv (_c->show.Wndo ());
   _font.Init (& cnv, "CourierNew", 8);
}


Wave::~Wave ()  {Wipe ();}
