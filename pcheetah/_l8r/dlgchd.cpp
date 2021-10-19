// dlgchd.cpp - edit chords

#include "pcheetah.h"

ubyte ChdBtw (TStr **out, char *c1, char *c2);

class DlgChd: public Dialog {          // chord picker (DlgChd.cpp)
public:
   DlgChd (PianoCheetah *d, ulong tm)
   : Dialog (IDD_CHD, IDI_APP), _d (d), _tm (tm)  {_s = _d->_song;}
private:
   PianoCheetah *_d;
   Song         *_s;
   bool    _got;
   ulong   _cp, _tm, _tm1;
   Toolbar _t;
   CtlCmbo _root, _type, _bass, _pop;
   CtlBttn _undo;
   void Set  (char *ch);
   void Msg  (char *x);
   void Get  (LPARAM lp);   void Ins (LPARAM lp);   void Del (LPARAM lp);
   void Pop  (LPARAM lp);   void UnDo (LPARAM lp);   void Sh (LPARAM lp);
   void ReDo (LPARAM lp);

   void Open ();   void Done ();

   bool Do (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }

   static EvMap<DlgChd> _evMap [];
};

int TBarChdID [] = {ID_M_LOAD, ID_M_INS, ID_M_DEL, 0};
char *IntvMap = "1m2m34d5a6m7";

EvMap<DlgChd> DlgChd::_evMap [] = {
   {ID_M_LOAD,  BN_CLICKED,   & DlgChd::Get},
   {ID_M_INS,   BN_CLICKED,   & DlgChd::Ins},
   {ID_M_DEL,   BN_CLICKED,   & DlgChd::Del},
   {IDC_ROOT,  CBN_SELCHANGE, & DlgChd::ReDo},
   {IDC_TYPE,  CBN_SELCHANGE, & DlgChd::ReDo},
   {IDC_BASS,  CBN_SELCHANGE, & DlgChd::ReDo},
   {IDC_POP,   CBN_SELCHANGE, & DlgChd::Pop},
   {IDC_UNDO,   BN_CLICKED,   & DlgChd::UnDo},
   {IDC_SHHH,   BN_CLICKED,   & DlgChd::Sh},
   {0}
};


void DlgChd::Set (char *s)
{ ubyte i, j;
  TStr  ch;
  char *c;
   StrCp (ch, s);
   _root.SetPos (0);   _type.SetPos (0);   _bass.SetPos (0);
   if (c = StrCh (ch, '/')) {          // if got bass, chop if off n set ctrl
      *c++ = '\0';
      for (i = 0;  i < 12;  i++)  {if (! StrCm (c, MKeyStr  [i]))  break;
                                   if (! StrCm (c, MKeyStrB [i]))  break;}
      if (i < 12) _bass.SetPos ((sword)(1+i));
   }
   if (*ch) {
      i = ((ch [1] == 'b') || (ch [1] == '#')) ? 2 : 1;    // type pos
      c = & ch [i];
      for (j = 0;  j < NMChd;  j++)  if (! StrCm (c, MChd [j].lbl, 'x'))
                                        {_type.SetPos ((sword)j);   break;}
      *c = '\0';                       // chop it off
   }
   for (i = 0;  i < 12;  i++)  {if (! StrCm (ch, MKeyStr  [i]))  break;
                                if (! StrCm (ch, MKeyStrB [i]))  break;}
   if (i < 12) _root.SetPos ((sword)i+1);
}

void DlgChd::Msg (char *x)
{ static TStr arg;   StrCp (arg, x);   _d->SongCmd ("chd", arg);}

void DlgChd::Done ()          { TStr x;   StrFmt (x, "@`d", _tm);   Msg (x);}
void DlgChd::Get (LPARAM lp)  {_d->SongCmd ("chd", "?", true);}
void DlgChd::Del (LPARAM lp)  {_d->SongCmd ("chd", "-", true);}
void DlgChd::Ins (LPARAM lp)  { TStr x;   StrFmt (x, "+`d", _tm);   Msg (x);}
void DlgChd::Sh  (LPARAM lp)  {_d->SongCmd ("timePause");}

void DlgChd::ReDo (LPARAM lp)
{ TStr  s, s1, s2, chd;
TRC("{ DlgChd::ReDo");
// pull our chord picks to make chd string
   _root.Str (chd);   StrCp (s1, MChd [_type.Pos ()].lbl);   _bass.Get (s2);
   if (! StrCm (chd, "(none)"))  *chd = '\0';    // no chord at all?
   else {
      StrAp (chd, s1);
      if (StrCm (s2, "(none)"))  {StrAp (chd, "/");   StrAp (chd, s2);}
   }
// enable/dis per picks
                 _type.Enable (true);    _bass.Enable (true);
   if (! *chd)  {_type.Enable (false);   _bass.Enable (false);
                 _type.SetPos (0);       _bass.SetPos (0);}
// update _f.chd (ins/del/upd) based on _got,_cp
   *s = '\0';   if (_got)  StrCp (s, _s->_f.chd [_cp].s);
   StrCp (s1, chd);
//DBG("new: chd='`s' old: cs='`s' got=`b", chd, s, _got);
   if (StrCm (s, chd)) {               // diff?  set _f.chd [_cp], _got
      if      (   _got  && (! *chd))  _s->_f.chd.Del (_cp);
      else if ((! _got) &&    *chd )  _s->_f.chd.Ins (_cp);
      if (*chd)  {StrCp (_s->_f.chd [_cp].s, chd);
                         _s->_f.chd [_cp].time = _tm;}
      _got = *chd ? true : false;
   }
TRC("} DlgChd::ReDo");
}

void DlgChd::Pop (LPARAM lp)
{ TStr s;
   _pop.Str (s);   while (*s == ' ')  StrCp (s, & s [1]);   Set (s);
   ReDo (0);   Msg (StrFmt (s, "@`d", _tm1));
}

void DlgChd::UnDo (LPARAM lp)
{ TStr s;
   _undo.GetText (s);   StrCp (s, & s [8]);                 Set (s);
   ReDo (0);   Msg (StrFmt (s, "@`d", _tm1));
}


void DlgChd::Open ()
{ ulong i, j, t1, t2, hbt;
  uword br;
  ubyte bt,                 nbtw;
  TStr  s, t, ch, ch1, ch2, *btw, it;
  char *c;
  KSgRow *ks;
TRC("{ DlgChd::Open");
   _t.Init (Wndo(), IDB_TBARCHD, TBarChdID,
      "calculate whole song's chords\r\n"
      "   (using notes of selected practice tracks)\0"
      "plop some random happy pop chords into this section\0"
      "delete all chords  (be carefulll)\0"
   );
   _root.Init   (Wndo(), IDC_ROOT);    // chord spec
   _type.Init   (Wndo(), IDC_TYPE);
   _bass.Init   (Wndo(), IDC_BASS);
   _pop.Init    (Wndo(), IDC_POP);
   _undo.Init   (Wndo(), IDC_UNDO);

// get closest beat to clicked _tm, or existing chord time
   _s->TmStr (s, _tm, & t2);   br = (uword)Str2Int (s, & c);
                               bt = (ubyte)Str2Int (c+1);
   t1 = _s->Bar2Tm (br, bt);                // trunc'd bar.beat spot
   if ((t2 - _tm) < (_tm - t1))  bt++;      // find NEARest beat
   hbt = (t2 - t1) / 2;                     // half a beat dur
//DBG("t=`d t1=`d t2=`d hbt=`d br=`d bt=`d s=`s", _tm, t1, t2, hbt, br, bt, s);

// if we got a _f.chd[] within hbt of orig _tm, use that time instead
   _cp = _s->_f.chd.Ln;                // default to NO _f.chd[] pos
   for (i = 0;  i < _s->_f.chd.Ln;  i++) {
      if (_s->_f.chd [i].time     >  _tm+hbt)  break;
      if (_s->_f.chd [i].time+hbt >= _tm) {
         if      (_cp >= _s->_f.chd.Ln)                             _cp = i;
         else if (ABSL((slong)_s->_f.chd [_cp].time - (slong)_tm) >
                  ABSL((slong)_s->_f.chd [i  ].time - (slong)_tm))  _cp = i;
      }
   }
   _tm1 = 0;   StrCp (ch1, "C");   StrCp (ch2, "C");
   if (_cp >= _s->_f.chd.Ln) {         // new dude, use bar,beat time
      _got = false;                    // no chordstyle yet
      _tm  = _s->Bar2Tm (br, bt);      // closest bar,bt to click point
      *s   = '\0';
      for (_cp = 0;  _cp < _s->_f.chd.Ln;  _cp++)
         if (_s->_f.chd [_cp].time >= _tm)  break;   // ins point if we do
   // use cp-1,cp for ChdBtw
      if (_cp)                   {StrCp (ch1, _s->_f.chd [_cp-1].s);
                                  _tm1 =      _s->_f.chd [_cp-1].time;}
      if (_cp   < _s->_f.chd.Ln)  StrCp (ch2, _s->_f.chd [_cp].s);
   }
   else {
      _got = true;     _tm = _s->_f.chd [_cp].time;
      StrCp (s,              _s->_f.chd [_cp].s);
   // use cp-1,cp+1 for ChdBtw
      if (_cp)                   {StrCp (ch1, _s->_f.chd [_cp-1].s);
                                  _tm1 =      _s->_f.chd [_cp-1].time;}
      if (_cp+1 < _s->_f.chd.Ln)  StrCp (ch2, _s->_f.chd [_cp+1].s);
   }
   nbtw = ChdBtw (& btw, ch1, ch2);
   _pop.Clear ();
   for (ubyte i = 0;  i < nbtw;  i++) {
     SpaceSep ss (btw [i], 80);
      _pop.LstIns (ss.Col [0]);
      for (ubyte j = 1;  ss.Col [j][0];  j++)
         {StrCp (it, " ");   StrAp (it, ss.Col [j]);   _pop.LstIns (it);}
   }
   StrCp (ch, s);
   ks = _s->KSig (_tm);
   if (! nbtw)  {_undo.Show (false);   _pop.LstIns ("(none)");}
   else {        _undo.Show (true);
      if (*s == '\0')  StrCp (s, "(none)");
      StrCp (it, "restore ");   StrAp (it, s);   _undo.SetText (it);
   }
   _pop.SetPos (0);

//DBG("got=`b cp=`d/`d tm=`s br=`d bt=`d ch=`s",
//_got, _cp,_s->_f.chd.Ln, _s->TmSt(s,_tm), br,bt, ch);

// init lists for chord's root/type/bass
   c = ks->flt ? "(none)\0C\0Db\0D\0Eb\0E\0F\0Gb\0G\0Ab\0A\0Bb\0B\0"
               : "(none)\0C\0C#\0D\0D#\0E\0F\0F#\0G\0G#\0A\0A#\0B\0";
   _root.LstZZ (c);   _bass.LstZZ (c);
   for (i = 0;  i < NMChd;  i++) {
      StrCp (t, "............");
      for (j = 0;  MChd [i].tmp [j] != 'x';  j++)
         t [MChd [i].tmp [j]] = IntvMap [MChd [i].tmp [j]];
      StrFmt (s, "`<5s  `s  `s", MChd [i].lbl, t, MChd [i].etc?MChd [i].etc:"");
      _type.LstIns (s);
   }

// chord control picks (root, type, bass)  (n disabling)
   Set (ch);   ReDo (0);
   MoveMs ("tR");
TRC("} DlgChd::Open");
}
