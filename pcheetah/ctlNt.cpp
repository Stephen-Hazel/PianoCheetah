// ctlNt.cpp - note widget - do editing on da pixmap

#include "pcheetah.h"
//#include "dlgChd.cpp"

void CtlNt::RePM ()
// moved to new screen?  resize pixmap
{  Up.pm = nullptr;
  QString scr = Gui.W ()->windowHandle ()->screen ()->name ();
  QList<QScreen *> scrLs = QGuiApplication::screens ();
   for (int i = 0;  i < scrLs.size ();  i++)  if (scrLs [i]->name () == scr)
      {Up.pm = new QPixmap (scrLs [i]->size ());   break;}
   if (Up.pm == nullptr)  DBG("couldn't alloc CtlNt pixmap :(");
}


void CtlNt::paintEvent (QPaintEvent *e)
{  (void)e;
   if (Up.pm == nullptr)  {DBG("no pm");   return;}
  Canvas c (this);
   c.Blt (*Up.pm,   0, 0,  0, 0,  Up.w, Up.h);
   c.Blt (*Up.tpm,  Up.tpos.left (), Up.tpos.top (),  0, 0,
                                           Up.tpos.width (), Up.tpos.height ());
}


void CtlNt::resizeEvent (QResizeEvent *e)
{  Up.w = (ubyt2)e->size ().width  (); // maybe we changed screens
   Up.h = (ubyt2)e->size ().height (); // otherwise we should always be beeg nuf
   if ((Up.pm == nullptr) || (Up.w > Up.pm->width  ()) ||
                             (Up.h > Up.pm->height ()))  RePM ();
TRC("resize w=`d h=`d", Up.w, Up.h);
   emit sgReSz ();
}


void CtlNt::keyPressEvent (QKeyEvent *e)
{ KeyMap km;
  key    k;
   if ((k = km.Map (e->modifiers (), e->key ())))  DBG("nt key=`s", km.Str (k));
}


void CtlNt::wheelEvent (QWheelEvent *e)
{ int j = e->angleDelta ().y () / 40;
DBG("wheel=`d", j);
}


void CtlNt::mouseMoveEvent (QMouseEvent *e)
{ int b, x, y;
   b = e->buttons ();   x = e->pos ().x ();   y = e->pos ().y ();
DBG("msMv b=`d x=`d y=`d", b, x, y);
}


void CtlNt::mousePressEvent (QMouseEvent *e)
{ int b, x, y;
   b = e->buttons ();   x = e->pos ().x ();   y = e->pos ().y ();
DBG("msDn b=`d x=`d y=`d", b, x, y);
}


void CtlNt::mouseReleaseEvent (QMouseEvent *e)
{ int b, x, y;
   b = e->buttons ();   x = e->pos ().x ();   y = e->pos ().y ();
DBG("msUp b=`d x=`d y=`d", b, x, y);
}



/*
//------------------------------------------------------------------------------
CtlListHdr CtlHdr [] = { {"show", 'l'}, {"control", 'l'} };

class DlgCtl: public Dialog {          // show/hide ctl
public:
   DlgCtl (Song *s)
   : Dialog (IDD_CTL, IDI_APP), _s (s)  {}
private:
   Song   *_s;
   Toolbar _t;
   CtlBttn _ck;
   CtlList _ls;

   void Pik (LPARAM l)
   { sword p = _ls.Pos ();
     TStr  s;
      if (p >= 0)
         {_ls.SetPos (p);   _ls.GetCol (p, 0, s);
                            _ls.SetCol (p, 0, (*s == 'y') ? "no" : "yep");}
   }

   void Open ()
   { char *ro [10];
     uword i, j;
     TStr  ts;
      _t.Init  (Wndo(), IDB_OKCAN);
      _ck.Init (Wndo(), IDC_CHD);
      _ls.Init (Wndo(), IDC_LIST, CtlHdr, BITS (CtlHdr));
      _ls.Clear ();                    // load up _f.ctl[]
      _ck.SetCheck (_s->_lrn.chd);
      ro [0] = ts;   ro [2] = NULL;
      for (i = 0;  i < _s->_f.ctl.Ln;  i++) {
         StrCp (ts, _s->_f.ctl [i].sho ? "yep" : "no");
         ro [1] = _s->_f.ctl [i].s;
         _ls.Ins (ro);
      }
      StrCp (ts, "no");
      for (i = 0;  i < NMCC;  i++) {
         for (j = 0;  j < _s->_f.ctl.Ln;  j++)
            if (! StrCm (_s->_f.ctl [j].s, MCC [i].s))  break;
         if (j >= _s->_f.ctl.Ln) {ro [1] = MCC [i].s;   _ls.Ins (ro);}
      }
      _ls.ReW ();
      MoveMs ("tc");
   }

   void Done ()                        // set _f.ctl[].sho's
   { uword i, e;
     TStr  s;                          // skip if hit cancel
      if (_ok) {
         _s->_lrn.chd = _ck.Check () ? true : false;
         e = (uword)_s->_f.ctl.Ln;
         for (i = 0;  i < e;  i++) {
            _ls.GetCol ((sword)i, 0, s);
            _s->_f.ctl [i].sho = (*s == 'y') ? true : false;
         }
         for (i = e;  i < _ls.Len ();  i++) {
            _ls.GetCol ((sword)i, 0, s);
            if (*s == 'y') {
               _ls.GetCol ((sword)i, 1, s);
               _s->_f.ctl.Ln++;
               StrCp (_s->_f.ctl [e].s, s);   _s->_f.ctl [e++].sho = true;
            }
         }
      }
   }

   bool Do (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }

   static EvMap<DlgCtl> _evMap [];
};

EvMap<DlgCtl> DlgCtl::_evMap [] = {
   {IDC_LIST, NM_CLICK, & DlgCtl::Pik},
   {0}
};


//------------------------------------------------------------------------------
static char
   *Ma = "C   (0b)\0"                  "Db  (5b  also C# below)\0"
         "D   (2#)\0"                  "Eb  (3b)\0"
         "E   (4#)\0"                  "F   (1b)\0"
         "Gb  (6b  also F# below)\0"   "G   (1#)\0"
         "Ab  (4b)\0"                  "A   (3#)\0"
         "Bb  (2b)\0"                  "B   (5#  also Cb below)\0"
         "F#  (6#  also Gb above)\0"   "C#  (7#  also Db above)\0"
         "Cb  (7b  also B  above)\0",
   *Mi = "A   (0b)\0"                  "Bb  (5b  also A# below)\0"
         "B   (2#)\0"                  "C   (3b)\0"
         "C#  (4#)\0"                  "D   (1b)\0"
         "Eb  (6b  also D# below)\0"   "E   (1#)\0"
         "F   (4b)\0"                  "F#  (3#)\0"
         "G   (2b)\0"                  "G#  (5#  also Ab below)\0"
         "D#  (6#  also Eb above)\0"   "A#  (7#  also Bb above)\0"
         "Ab  (7b  also G# above)\0";

class DlgKSig: public Dialog {         // key sig ins/updater
public:
   DlgKSig (char *st): Dialog (IDD_KSIG, IDI_APP), _st (st)  {}
private:
   char   *_st;
   Toolbar _t;
   CtlCmbo _key, _maj;

   void Pik (LPARAM l)
   { sword p = _key.Pos ();
      _key.LstZZ (_maj.Pos () ? Mi : Ma);   _key.SetPos (p);
   }

   void Open ()
   { char *m;
      _t.Init   (Wndo(), IDB_OKCAN);
      _maj.Init (Wndo(), IDC_MAJ);
      _key.Init (Wndo(), IDC_KEY);
      m = & _st [StrLn (_st)-1];
      _maj.LstZZ  ("Major\0Minor\0");
      _key.LstZZ  ((*m == 'm') ? Mi : Ma);
      _maj.SetPos ((*m == 'm') ? 1  : 0 );
      if (*m == 'm')  *m = '\0';
      StrAp (_st, " ");   _key.SetStr (_st);
      *_st = '\0';        MoveMs ("tR");
   }

   void Done ()
   {  if (! _ok)  return;              // hit cancel

      _key.Get (_st, sizeof (TStr));
      if (_st [1] == ' ')  _st [1] = '\0';   else _st [2] = '\0';
      if (_maj.Pos ())  StrAp (_st, "m");
   }

   bool Do (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }

   static EvMap<DlgKSig> _evMap [];
};

EvMap<DlgKSig> DlgKSig::_evMap [] = {
   {IDC_MAJ, CBN_SELCHANGE, & DlgKSig::Pik},
   {0}
};


//------------------------------------------------------------------------------
class DlgTSig: public Dialog {
public:
   DlgTSig (char *st): Dialog (IDD_TSIG, IDI_APP), _st (st)  {}
private:
   char   *_st;
   Toolbar _tbar;
   CtlEdit _num;
   CtlSpin _spn;
   CtlCmbo _den, _sub;

   void Open ()
   { ubyte i;
     TStr  n;
     char *d, *s;
      _tbar.Init (Wndo(), IDB_OKCAN);
      _num.Init (Wndo(), IDC_NUM);   _spn.Init (Wndo(), IDC_NUM_SP, 1, 255);
      _den.Init (Wndo(), IDC_DEN);
      _sub.Init (Wndo(), IDC_SUB);
      for (i = 0;  i < 16;  i++)  _den.LstIns (StrFmt (n, "`d", 1 << i));
      for (i = 0;  i <  9;  i++)  _sub.LstIns (StrFmt (n, "`d", i +  1));
      StrCp (n, _st);
      d = StrCh (n, '/');   *d++ = '\0';
      s = StrCh (d, '/');   if (s)  *s++ = '\0';  else s = "1";
      _num.Set (n);   _den.Set (d);   _sub.Set (s);
   }

   void Done ()
   {  if (! _ok)  return;              // hit cancel
     TStr s;
      StrFmt (_st, "`d/`d", _num.Get (), _den.Get ());
      if (_sub.Pos ())  StrAp (_st, StrFmt (s, "/`d", _sub.Get ()));
   }
};


//------------------------------------------------------------------------------
class DlgTmpo: public Dialog {
public:
   DlgTmpo (char *st): Dialog (IDD_TMPO, IDI_APP), _st (st)  {}
private:
   char   *_st;
   Toolbar _tbar;
   CtlEdit _num;
   CtlSpin _spn;

   void Open ()
   {  _tbar.Init (Wndo(), IDB_OKCAN);
      _num.Init  (Wndo(), IDC_NUM, _st);
      _spn.Init  (Wndo(), IDC_NUM_SP, 1, 960);
   }

   void Done ()
   {  if (! _ok)  return;              // hit cancel
      _num.Get (_st);
   }
};


//------------------------------------------------------------------------------
int TBCueID  [] = {ID_M_REDO, ID_M_DEL, 0},
    TBCue2ID [] = {ID_M_VE, ID_M_CH, ID_M_BR, ID_M_SCT, ID_M_CR, ID_M_DE, 0},
    TBCue3ID [] = {ID_M_FER, ID_M_TRE, ID_M_STA, ID_M_HAP, ID_M_SAD, ID_M_MAD,
                                                                             0};
class DlgCue: public Dialog {          // edit _f.cue[]s
public:
   DlgCue (char *s): Dialog (IDD_CUE, IDI_APP), _s (s)  {}
private:
   char   *_s;
   Control _c;
   Toolbar _t, _t2, _t3;
   void Open ()
   { uword x, y, w, h;
      _t.Init (Wndo(), IDB_TBARCUE, TBCueID,
         "re-init loops and erase all bug history\0" "delete this cue\0"
      );
      _t2.Init (Wndo(), IDB_TBARCUE2, TBCue2ID,
         "verse\0" "chorus\0" "break\0"
         "other section (coda, bridge, etc) (name in textbox below)\0"
         "crescendo\0" "decrescendo\0"
      );
      _t3.Init (Wndo(), IDB_TBARCUE3, TBCue3ID,
         "fermata\0" "tremolo\0" "star\0" "happy\0" "sad\0" "mad\0"
      );
      _t.XY (& x, & y);   _t.WH (& w, & h);
      y += h;   _t2.Move (0, y);   _t2.WH (& w, & h);
      y += h;   _t3.Move (0, y);
      _c.Init (Wndo(), IDC_STR);   _c.Set (_s);   _c.Focus ();
      MoveMs ("tL");
   }
   bool Do (int ctrl, int evnt, LPARAM l)
   { TStr s;
      if (evnt == BN_CLICKED) {
         switch (ctrl) {
         case ID_M_REDO:  _c.Set ("loopInit");  break;
         case ID_M_DEL:   _c.Set ("");          break;

         case ID_M_VE:   _c.Set ("(verse");              break;
         case ID_M_CH:   _c.Set ("(chorus");             break;
         case ID_M_BR:   _c.Set ("(break");              break;
         case ID_M_SCT:  _c.Get (s);
                         if (*s != '(')  {StrCp (& s [1], s);   *s = '(';
                                          _c.Set (s);}   break;
         case ID_M_CR:   _c.Set ("<");     break;
         case ID_M_DE:   _c.Set (">");     break;

         case ID_M_FER:  _c.Set ("`fer");  break;
         case ID_M_TRE:  _c.Set ("`tre");  break;
         case ID_M_STA:  _c.Set ("`sta");  break;
         case ID_M_HAP:  _c.Set ("`hap");  break;
         case ID_M_SAD:  _c.Set ("`sad");  break;
         case ID_M_MAD:  _c.Set ("`mad");  break;

         case IDC_FPPP: _c.Set ("ppp"); break;
         case IDC_FPP:  _c.Set ("pp");  break;
         case IDC_FP:   _c.Set ("p");   break;
         case IDC_FMP:  _c.Set ("mp");  break;
         case IDC_FMF:  _c.Set ("mf");  break;
         case IDC_FF:   _c.Set ("f");   break;
         case IDC_FFF:  _c.Set ("ff");  break;
         case IDC_FFFF: _c.Set ("fff"); break;
         case IDC_FFZ:  _c.Set ("Fz");  break;
         }
         Post (WM_COMMAND, IDOK, 0);
      }
      return false;
   }
   void Done ()  {_c.Get (_s, sizeof (TStr));}
};


//------------------------------------------------------------------------------
class DlgMove: public Dialog {         // what do we do w rect'd notes?
public:
   DlgMove (char *s): Dialog (IDD_MOVE, IDI_APP), _s (s)  {}
private:
   char *_s;

   void Open ()  {*_s = '\0';   MoveMs ("Tc");}

   bool Do (int ctrl, int evnt, LPARAM l)
   {  if (evnt == BN_CLICKED) {
         switch (ctrl) {
            case IDC_H2L: StrCp (_s, " <");   break;
            case IDC_H2R: StrCp (_s, " >");   break;
            case IDC_H2B: StrCp (_s, " #");   break;
            case IDC_DEL: StrCp (_s, " x");   break;
         }
         Post (WM_CLOSE, 0, 0);
         return true;
      }
      return false;
   }
};


//------------------------------------------------------------------------------
int TBFingID  [] = {ID_M_SWAP, ID_M_DELNOTE,  ID_M_DELFING, 0},
    TBFing2ID [] = {ID_M_LH, ID_M_RH, ID_M_LF, ID_M_RF, ID_M_AC, ID_M_ST, 0};

class DlgFing: public Dialog {         // fingering picker
public:
   DlgFing (ubyte *f, char *i): Dialog (IDD_FING, IDI_APP), _f (f), _i (i)  {}
private:
   ubyte *_f;
   char  *_i;
   Toolbar _t, _t2;

   void Open ()
   { Control ic;
     uword   x, y;
      _t.Init (Wndo (), IDB_TBARFING, TBFingID,
         "swap note to other hand's track\0"
         "delete this note\0"
         "delete =ALL= fingering in the song (BE CAREFUL)\0"
      );
      _t2.Init (Wndo (), IDB_TBARFING2, TBFing2ID,
         "left hand (drums)\0"
         "right hand (drums)\0"
         "left foot (drums/organ)\0"
         "right foot (drums/organ)\0"
         "accent\0"
         "staccatissimo (or WHAtever really)\0"
      );
      ic.Init (Wndo(), IDC_TXT);    ic.Set (_i);
      ic.Init (Wndo(), IDC_BORD);   ic.XY (& x, & y);   _t2.Move (x+3, y+18);
      MoveMs ("Tc");
      *_f = 100;                       // default to fake-ish "cancel"
   }

   bool Do (int ctrl, int evnt, LPARAM l)
   {  if (evnt == BN_CLICKED) {
         switch (ctrl) {
         case ID_M_SWAP:    *_f = 97;  break;
         case ID_M_DELNOTE: *_f = 98;  break;
         case ID_M_DELFING: *_f = 99;  break;

         case IDC_FX:  *_f =  0;  break;    case IDC_F1:  *_f =  1;  break;
         case IDC_F2:  *_f =  2;  break;    case IDC_F3:  *_f =  3;  break;
         case IDC_F4:  *_f =  4;  break;    case IDC_F5:  *_f =  5;  break;
         case IDC_F12: *_f =  6;  break;    case IDC_F13: *_f =  7;  break;
         case IDC_F14: *_f =  8;  break;    case IDC_F15: *_f =  9;  break;
         case IDC_F21: *_f = 10;  break;    case IDC_F23: *_f = 11;  break;
         case IDC_F24: *_f = 12;  break;    case IDC_F25: *_f = 13;  break;
         case IDC_F31: *_f = 14;  break;    case IDC_F32: *_f = 15;  break;
         case IDC_F34: *_f = 16;  break;    case IDC_F35: *_f = 17;  break;
         case IDC_F41: *_f = 18;  break;    case IDC_F42: *_f = 19;  break;
         case IDC_F43: *_f = 20;  break;    case IDC_F45: *_f = 21;  break;
         case IDC_F51: *_f = 22;  break;    case IDC_F52: *_f = 23;  break;
         case IDC_F53: *_f = 24;  break;    case IDC_F54: *_f = 25;  break;

         case ID_M_LH: *_f = 26;  break;    case ID_M_RH: *_f = 27;  break;
         case ID_M_LF: *_f = 28;  break;    case ID_M_RF: *_f = 29;  break;
         case ID_M_AC: *_f = 30;  break;    case ID_M_ST: *_f = 31;  break;
         }
//DBG("f=`d", *_f);
         Post (WM_CLOSE, 0, 0);
         return true;
      }
      return false;
   }
};


//------------------------------------------------------------------------------
void CtlNt::DragRect ()
{ Canvas c (Wndo ());
  sword  x1, x2, y1, y2;
   c.SetROp (R2_NOTXORPEN);
   x1 = _x1;   x2 = _x2;   if (x2 < x1) {x2 = _x1;  x1 = _x2;}
   y1 = _y1;   y2 = _y2;   if (y2 < y1) {y2 = _y1;  y1 = _y2;}
   c.Rect (x1, y1, x2-x1+1, y2-y1+1);
}


const ulong DRAG = 2;                  // drag threshold (if <, then plain clik)

char CtlNt::MsPos (sword x, sword y)
// find our _pg _co _tm n _pos:
//    \0 [k]eys [c]hord [q]cue [r]cue.tend [x]control [f]inger [d]ur [n]ewNote
//    _got=\0 or 'y' for pos=q,r,x
// _pos x:    _ct
// _pos f,d:  _sy
{ ulong c, p, ne, tm1, tm2;
  uword nx, cx, th = _c->txH;
  PagDef *pg;
  ColDef  co;
   _pos = _got = '\0';   p = _s->_pgP;   if (! p)  return _pos;
//DBG("CtlNt::MsPos p=`d", p);

// if showing transition(got _rc) next page unless w/in rect
   p--;
   if (x < _s->_rc.left || y < _s->_rc.top)  p++;
   pg = & _s->_pag [p];
   for (c = 0;  (c+1 < pg->nCol) && (x >= pg->col [c+1].x);  c++)  ;
   MemCp (& co, & pg->col [c], sizeof (co));               // load column
   if ( (y > co.h) || (x >= (co.x+co.w-4)) || (x < (co.x+4)) )
      return _pos;                     // outa col or in border - we got nothin
   nx = _s->Nt2X (co.nMn, & co);   cx = _s->CtlX (& co);

   _pg = p;   _co = c;
   _tm = _s->Y2Tm (y, & co);   tm1 = _s->Y2Tm (y-2, & co);
                               tm2 = _s->Y2Tm (y+2, & co);
   if (y < H_KB)  return (_pos = 'k');           // keys area?

   if      (x < nx) {                  // chord/cue area
      if (_s->_lrn.chd && x < (nx-W_Q))
         return (_pos = 'c');          // chord area (no dragging)

      ne = _s->_f.cue.Ln;   _p = NONE;   *_str = '\0';
      for (p = 0;  (p < ne) && (_s->_f.cue [p].time <  tm1);  p++)  ;
      if (         (p < ne) && (_s->_f.cue [p].time <= tm2)) {
         _got = 'y';   StrCp (_str, _s->_f.cue [_p = p].s);
         return (_pos = 'q');
      }                                // now look thru all (unsorted) .tends
      for (p = 0;  p < ne;  p++)  if (_s->_f.cue [p].tend &&
                 (_s->_f.cue [p].tend >= tm1) && (_s->_f.cue [p].tend <= tm2)) {
         _got = 'y';   StrCp (_str, _s->_f.cue [_p = p].s);
         return (_pos = 'r');
      }
      return (_pos = 'q');             // if new just do q
   }
   else if (x >= cx) {                 // control area
     sword tx = (sword)cx;
      _cp = 0;
      for (ubyte i = 0;  i < _s->_f.ctl.Ln;  i++)  if (_s->_f.ctl [i].sho)
         {if (x < (tx += th))  {_ct = i;   break;}
          else                  _cp++;}
     TrkEv *e;
     ulong ne;
     TStr  cs;
     bool  cg = false;
     ubyte td = 255;
      StrCp (cs, _s->_f.ctl [_ct].s);
      if ( (! StrCm ("tmpo", cs)) || (! StrCm ("ksig", cs)) ||
                                     (! StrCm ("tsig", cs)) ) {
         cg = true;
         for (ubyte t = 0;  t < _s->_rTrk;  t++)
            if (_s->TDrm (t))  {td = t;   break;}
      }
      for (_tr = 0;  _tr < _s->_rTrk;  _tr++)
                      if ( (cg && (_tr == td)) || ((! cg) && _s->TSho (_tr)) ) {
         for (e = _s->_f.trk [_tr].e, ne = _s->_f.trk [_tr].ne,
              _p = 0;  _p < ne;  _p++)
            if ( (e [_p].ctrl == (0x80|_ct)) && (e [_p].time >= tm1) &&
                                                (e [_p].time <= tm2) )
               {_got = 'y';   break;}
         if (_got)  break;             // break out ALL the way :/
      }
//DBG("pos=x ct=`d cp=`d got=`b tr=`d p=`d tm=`d",
//_ct, _cp, _got, _tr, _p, _tm);
      return (_pos = 'x');
   }
// ok, has ta be nt area so hunt down a symbol
// d[ur] for bot half, f[ing] for top, n[ew] for not over sym
  SymDef *it = NULL;
  ulong   s, sy1;
   for (s = 0;  s < co.nSym;  s++)
      if ((x >= nx+co.sym [s].x) && (x < nx+co.sym [s].x + co.sym [s].w) &&
          (y >=    co.sym [s].y) && (y <    co.sym [s].y + co.sym [s].h))
         it = & co.sym [sy1 = s];
   if (! it)  return (_pos = 'n');     // no sym?  [n]ew note area
   _sy = sy1;
   if ((it->h >= 10) && (y >= it->y + it->h*2/3))  return (_pos = 'd');
                                                   return (_pos = 'f');
}


void CtlNt::MsDn (ulong btn, sword x, sword y)
// given _pos, see if we're dragging
// _drg:  [q]cue [x]ctlUpd rect[m]ov [d]ur [n]oteHop
{ PagDef *pg;
  ColDef  co;
  TStr    s;
  uword   nx;
   if (! MsPos (x, y))        return;
   if (   btn & MK_RBUTTON) {          // just for ctl killin
      if ((_pos == 'x') && _got) {
         StrFmt (s, "`d `d 0 KILL KILL", _tr, _p);
         _dt->SongCmd ("setCtl", s);
      }
      return;
   }
   if (! (btn & MK_LBUTTON))  return;  // need regular button

DBG("CtlNt::MsDn x=`d y=`d btn=`d _pos=`c _got=`c _drg=`c",
x, y, btn, _pos, _got ? _got : ' ', _drg ? _drg : ' ');
   pg = & _s->_pag [_pg];
   MemCp (& co, & pg->col [_co], sizeof (co));   // load column
   nx = _s->Nt2X (co.nMn, & co);
   _pPoz = _s->Poz (true);   _s->NotesOff ();

   if (_pos == 'k') {                  // keyboard area - ctl[].sho editin
     DlgCtl dlg (_s);   dlg.Ok (Wndo ());
      _s->_pgP = 0;   _s->PostKid (MSG_DRAW, 0, 0);
      if (! _pPoz) _s->Poz (false);
      return;
   }
   if (_pos == 'c') {                  // chord area
     DlgChd dlg (_dt, _tm);        dlg.Ok (Wndo ());
      _s->_pgP = 0;   _s->PostKid (MSG_DRAW, 0, 0);
      if (! _pPoz) _s->Poz (false);
      return;
   }
   if ((_pos == 'q') || (_pos == 'r')) {    // cue time or tend
      _drg = _pos;   _x1 = co.x+4;    _x2 = co.x + co.w - 4;
                     _yp = _y1 = y;   _y2 = y + 1;
      DragRect ();   return;
   }
   if (_pos == 'x') {                  // control area - drag time,valu
      _drg = 'x';   _xp = x;   _x1 = nx;   _x2 = x;
                    _yp = y;   _y1 = y;    _y2 = y+1;
      DragRect ();   return;
      return;
   }

   if (_pos == 'n') {                  // not on sym: drag rect n move nt group
      _drg = 'm';   _x1 = _x2 = x;   _y1 = _y2 = y;             // else ins note
      DragRect ();   return;
   }
  SymDef *it = & co.sym [_sy];
   if (_pos == 'd') {                  // drag a new dur
      _drg = 'd';   _x1 = nx + it->x;   _x2 = nx + it->w - 1;
                    _y1 = it->y;        _y2 = y;
      DragRect ();   return;
   }
   if (_pos == 'f') {                  // hop a note else fingering dlg
      _drg = 'n';
      _xp = x;   _xo = x - (nx + it->x);   _x1 = x - _xo;
                                           _x2 = _x1 + W_NT;
      _yp = y;   _yo = y -       it->y;    _y1 = y - _yo;
                                           _y2 = _y1 + W_NT*2;
      DragRect ();   return;
   }
}


void CtlNt::MsMv (ulong btn, sword x, sword y)
{ LPCTSTR c;
  static LPCTSTR pC = 0;
  PagDef *pg;
  ColDef  co;
  ulong   t;
  uword   cx, th = _c->txH;
  ubyte   v1;
  TStr    s, s2, cs;
  char    ct;
  TrkEv  *e;
//DBG("CtlNt::MsMv x=`d y=`d btn=`d _pos=`c _got=`c _drg=`c",
//x, y, btn, _pos ? _pos : ' ', _got ? _got : ' ', _drg ? _drg : ' ');
   if (! btn) {
      switch (MsPos (x, y)) {
         case 'q':
         case 'r': if (_got)  {c = IDC_SIZENS;    break;}
         case 'x': if (_got)  {c = IDC_SIZEALL;   break;}
         case 'n': c = IDC_CROSS;    break;
         case 'f': c = IDC_HAND;     break;
         case 'd': c = IDC_SIZENS;   break;
         default:  c = IDC_ARROW;
      }
      if (c != pC)  ::SetCursor (::LoadCursor (NULL, pC = c));
      *s = '\0';
      if ((_pos == 'f') || (_pos == 'd')) {
      // ?tEnd,track#,name,dev#,+,upVelo
         pg = & _s->_pag [_pg];   MemCp (& co, & pg->col [_co], sizeof (co));
        ubyte  nt, tr = co.sym [_sy].tr;
        bool       dr = _s->TDrm (tr);
//DBG("pg=`d co=`d sy=`d", _pg, _co, _sy);
        ulong  tm, te;
        TrkEv *ev = NULL;
         if (_s->TEz (tr)) {
            tm = te = co.sym [_sy].tm;   nt = (ubyte)co.sym [_sy].nt;
            te += (M_WHOLE/8*3/4);
//DBG("ez      tr=`d nt=`d tm=`d te=`d", tr, nt, tm, te);
         }
         else {
           TrkNt *n = & _s->_f.trk [tr].n [co.sym [_sy].nt];
            tm = n->tm;   te = n->te;   nt = n->nt;
            if (n->dn != NONE)  ev = & _s->_f.trk [tr].e [n->dn];
//DBG("dr/real tr=`d nt=`d tm=`d te=`d", tr, nt, tm, te);
         }
         _s->TmSt           (s , tm);    StrAp (s, "-");
         StrAp (s, _s->TmSt (s2, te));   StrAp (s, " ");
         StrAp (s, dr ? MDrm2Str (s2, nt) : MKey2Str (s2, nt));
         StrAp (s, ev ? StrFmt (s2, "_`d ", ev->valu & 0x007F) : "_? ");
         if (! dr)  {StrAp (s, _s->SndName (tr));   StrAp (s, " ");}
         StrAp (s, _s->DevName (tr));
         if (! dr)  StrAp (s, StrFmt (s2, ".`d", _s->_f.trk [tr].chn+1));
         _s->Heya (s);
      }
      if ((_pos == 'x') && _got) {
         e = &      _s->_f.trk [_tr].e [_p];
         StrCp (cs, _s->_f.ctl [_ct].s);
         _s->TmSt (s, e->time);   StrAp (s, " ");   StrAp (s, cs);
         for (t = 0;  t < NMCC;  t++)  if (! StrCm (MCC [t].s, cs))  break;
         ct = (t >= NMCC) ? 'u' : MCC [t].typ;
         v1 = e->valu;
         if (! StrCm (cs, "tmpo"))
            StrAp (s, StrFmt (s2, "=`d", _s->TmpoAct (v1 | e->val2 << 8)));
         else if (ct != 'x')
            StrAp (s, StrFmt (s2, "=`d", (ct=='s') ? ((slong)v1-64) : v1));
         _s->Heya (s);
      }
//DBG("_pos=`c got=`b _str=`s", _pos, _got, _str);
      if ( (_pos == 'q') && _got &&
           (((_str [0] == '(') && (! StrCh ("vc", _str [1]))) ||
            (*_str == '.')) )
         _s->Heya (& _str [1]);
      return;
   }
   if (! (btn & MK_LBUTTON))  return;

// draggin - erase old cursor rect n draw new one
   if ((_drg == 'q') || (_drg == 'r'))
                     {DragRect ();   _y1 = y;   _y2 = y + 1;   DragRect ();
      pg = & _s->_pag [_pg];   MemCp (& co, & pg->col [_co], sizeof (co));
      t = _s->Y2Tm (y, & co);   _s->TmSt (s, t);   _s->Heya (s);
   }
   if (_drg == 'x')  {DragRect ();   _y1 = y;   _y2 = y + 1;   DragRect ();
      pg = & _s->_pag [_pg];   MemCp (& co, & pg->col [_co], sizeof (co));
      t = _s->Y2Tm (y, & co);   _s->TmSt (s, t);
      StrCp (cs, _s->_f.ctl [_ct].s);
      StrFmt (s2, "time=`s control=`s", s, cs);
      for (t = 0;  t < NMCC;  t++)  if (! StrCm (MCC [t].s, cs))  break;
      ct = (t >= NMCC) ? 'u' : MCC [t].typ;
      if (ct != 'x') {

         if (_got)  v1 = _s->_f.trk [_tr].e [_p].valu;     // init v1
         else      {cx = _s->CtlX (& co) + th * _cp;
                    v1 = (ubyte)(127 * (_xp - cx) / (th-1));}
         if (x >= _xp)                                     // offset by xpos
               {if ((x - _xp) >= (127-v1))  v1  = 127;   else v1 += (x - _xp);}
         else  {if ((_xp - x) <=      v1 )  v1  = 0;     else v1 -= (_xp - x);}
         StrAp (s2, StrFmt (s, " value=`d", (ct=='s') ? ((slong)v1-64) : v1));
      }
      _s->Heya (s2);
   }
   if (_drg == 'm')  {DragRect ();   _x2 = x;   _y2 = y;       DragRect ();}
   if (_drg == 'd')  {DragRect ();   _y2 = y;                  DragRect ();}
   if (_drg == 'n')  {DragRect ();   _x1 = x - _xo;   _x2 = _x1 + W_NT;
           // NAW keep time/dur same _y1 = y - _yo;   _y2 = _y1 + W_NT*2;
                                                               DragRect ();}
}


void CtlNt::MsUp (ulong btn, sword x, sword y)
{ TStr  st, c, s1, s2;
  ubyte t, v1;
  uword cx, th = _c->txH;
  char  ct;
  PagDef *pg;
  ColDef  co;
   if (! (btn & MK_LBUTTON))  return;

DBG("CtlNt::MsUp x=`d y=`d btn=`d _pos=`c _got=`c _drg=`c",
x, y, btn, _pos ? _pos : ' ', _got ? _got : ' ', _drg ? _drg : ' ');
   if (_drg)  DragRect ();             // clear it out

   if ((_drg == 'q') || (_drg == 'r')) {         // cue
      _y2 = _y1 = y;
      _s->Heya ("");
      if (_got && (ABSL (y - _yp) > DRAG)) {     // move existing cue
         pg = & _s->_pag [_pg];   MemCp (& co, & pg->col [_co], sizeof (co));
         _tm = _s->Y2Tm (y, & co);
         if (_drg == 'q') {
            if (_s->_f.cue [_p].tend) {
               if (_tm >= _s->_f.cue [_p].tend)
                   _tm  = _s->_f.cue [_p].tend - M_WHOLE/2;
               StrFmt (_str, "/`d`s",
                       _s->_f.cue [_p].tend-_tm, _s->_f.cue [_p].s);
            }
         }
         else { // _drg == 'r'
            if (_tm <= _s->_f.cue [_p].time)
               _tm = _s->_f.cue [_p].time + M_WHOLE/2;
            StrFmt (_str, "/`d`s",
               _tm - _s->_f.cue [_p].time, _s->_f.cue [_p].s);
            _tm = _s->_f.cue [_p].time;
         }
         _s->_f.cue.Del (_p);          // del+ins to keep sorted :/
         if (*_str == '(')             // chop section time to bar
            _tm = _s->Bar2Tm (_s->Tm2Bar (_tm));
         _s->TxtIns (_tm, _str, & _s->_f.cue, 'c');
      }
      else {                           // ins/upd/del cue
        DlgCue dlg (_str);
         if (dlg.Ok (Wndo ())) {
            if (! StrCm (_str, "loopInit"))   _dt->SongCmd (_str, "x");
            else if (*_str != '[') {   // can't del loops
              ulong tm = _tm, te = 0;
               if (_got)  {tm = _s->_f.cue [_p].time;
                           te = _s->_f.cue [_p].tend;   _s->_f.cue.Del (_p);}
               if (*_str) {
                  if (StrCh ("<>", *_str)) {
                     if (! _got)  te = tm + M_WHOLE;
                     StrCp (s1, _str);   StrFmt (_str, "/`d`s", te-tm, s1);
                  }
                  if (*_str == '(')    // chop tm at bar boundary for sections
                     tm = _s->Bar2Tm (_s->Tm2Bar (tm));
                  _s->TxtIns (tm, _str, & _s->_f.cue, 'c');
               }
            }
         }
      }
      _s->SetLp ('.');
      _s->_pgP = 0;   _s->PostKid (MSG_DRAW, 0, 0);
      if (! _pPoz) _s->Poz (false);
   }
   if (_drg == 'x') {                  // control
      pg = & _s->_pag [_pg];   MemCp (& co, & pg->col [_co], sizeof (co));
      _tm = _s->Y2Tm (y, & co);
      StrCp (c, _s->_f.ctl [_ct].s);
      for (t = 0;  t < NMCC;  t++)  if (! StrCm (MCC [t].s, c))  break;
      ct = (t >= NMCC) ? 'u' : MCC [t].typ;
     TrkEv *e = _got ? & _s->_f.trk [_tr].e [_p] : NULL;
      if (ct == 'x') {                 // if ctl type x, pop dlg;  else use x
         CtlX2Str (st, c, e);
         if      (! StrCm (c, "ksig"))  { DlgKSig d (st);   d.Ok (Wndo ());}
         else if (! StrCm (c, "tsig"))  { DlgTSig d (st);   d.Ok (Wndo ());}
         else if (! StrCm (c, "tmpo"))  { DlgTmpo d (st);   d.Ok (Wndo ());}
         else                           StrCp (st, "*");   // prog
      }
      else {
         if (e)  v1 = e->valu;         // init v1
         else   {cx = _s->CtlX (& co) + th * _cp;
                 v1 = (ubyte)(127 * (_xp - cx) / (th-1));}
         if (x >= _xp)                 // offset by xpos
               {if ((x - _xp) >= (127-v1))  v1  = 127;   else v1 += (x - _xp);}
         else  {if ((_xp - x) <=      v1 )  v1  = 0;     else v1 -= (_xp - x);}
         StrFmt (st, "`d", v1);
      }
      if (*st) {
         StrFmt (s2, "`d `d `d `s `s", _got ? _tr : 128, _p, _tm, c, st);
         _dt->SongCmd ("setCtl", s2);
      }
      _s->_pgP = 0;   _s->PostKid (MSG_DRAW, 0, 0);
      if (! _pPoz) _s->Poz (false);
   }
   if (_drg == 'm') {                  // rect[M]ov
      _x2 = x;   _y2 = y;
      StrFmt (st, "`d `d `d `d `d `d", _pg, _co, _x1, _y1, _x2, _y2);
      if ((ABSL (_x2 - _x1) > DRAG) || (ABSL (_y2 - _y1) > DRAG)) {
        DlgMove dlg (c);   dlg.Ok (Wndo ());
         if (*c == '\0') *st = '\0';   else StrAp (st, c);
      }
      if (*st)  _dt->SongCmd ("rectMov", st);
                                      if (! _pPoz) _s->Poz (false);
   }
   if (_drg == 'd') {                  // [d]ur
      _y2 = y;
      StrFmt (st, "`d `d `d `d", _pg, _co, _sy, _y2);
      _dt->SongCmd ("noteDur", st);   if (! _pPoz) _s->Poz (false);
   }
   if (_drg == 'n') {                  // [n]oteHop
      _x1 = x - _xo;   _y1 = y - _yo;
     PagDef *pg = & _s->_pag [_pg];
     ColDef  co;
      MemCp (& co, & pg->col [_co], sizeof (co));   // load column
     SymDef *it = & co.sym [_sy];
     ubyte   t  = it->tr;
      if (ABSL (x - _xp) > DRAG) {
         StrFmt (st, "`d `d `d `d", _pg, _co, _sy, _x1 + W_NT/2);
         _dt->SongCmd ("noteHop", st);
                                      if (! _pPoz) _s->Poz (false);
                                      _drg = '\0';
                                      return;    // SORRY me :/
      }
     TrkNt *nt = & _s->_f.trk [t].n [it->nt];
      if (nt->dn == NONE)  StrCp (st, "ntDn: NONE\r\n");
      else {
         _s->TmSt (s1, _s->_f.trk [t].e [nt->dn].time);
         StrFmt (st,                "ntDn: `s velo=`d\r\n",
                   s1, _s->_f.trk [t].e [nt->dn].valu & 0x7F);
      }
      if (nt->up == NONE)  StrAp (st, "ntUp: NONE");
      else {
         _s->TmSt (s2, _s->_f.trk [t].e [nt->up].time);
         StrFmt (& st [StrLn (st)], "ntUp: `s velo=`d",
                   s2, _s->_f.trk [t].e [nt->up].valu & 0x7F);
      }
     ubyte f = _s->_f.trk [t].e [nt->dn].val2 & 0x3F;
     DlgFing dlg (& f, st);
      dlg.Ok (Wndo ());
      if (f != 100)  {StrFmt (st, "`d `d `d", t, it->nt, f);
                      _dt->SongCmd ("noteUpd", st);}
      if (! _pPoz) _s->Poz (false);
   }
   _drg = '\0';
}

*/
