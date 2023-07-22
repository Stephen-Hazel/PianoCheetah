// Waver.cpp

#include "Waver.h"

CtlTip  TipCtl;
int     TBarID [] =
   {IDM_LOAD, IDM_SAVE, -1, IDM_PLAY, IDM_PLAT, IDM_PLAF, IDM_STOP,
    IDM_ALL, IDM_CHOP, 0};

EvMap<Waver> Waver::_evMap [] = {
   {IDM_LOAD,    0,             & Waver::Load},
   {IDM_SAVE,    0,             & Waver::Save},
   {IDM_PLAY,    0,             & Waver::Play},
   {IDM_PLAF,    0,             & Waver::PlayFr},
   {IDM_PLAT,    0,             & Waver::PlayTo},
   {IDM_STOP,    0,             & Waver::Stop},
   {IDM_ALL,     0,             & Waver::All},
   {IDM_CHOP,    0,             & Waver::Chop},
   {IDC_MAG,     CBN_SELCHANGE, & Waver::Redo},
   {IDC_LOOP,    BN_CLICKED,    & Waver::SetLp},
   {IDC_SETBGN,  BN_CLICKED,    & Waver::SetBgn},
   {IDC_SETEND,  BN_CLICKED,    & Waver::SetEnd},
   {IDC_SETLBGN, BN_CLICKED,    & Waver::SetLBgn},
   {IDC_SETLEND, BN_CLICKED,    & Waver::SetLEnd},
   {IDC_FRQ,     EN_CHANGE,     & Waver::FrqNew},
   {IDC_BGN,     EN_CHANGE,     & Waver::NewBgn},
   {IDC_END,     EN_CHANGE,     & Waver::NewEnd},
   {IDC_LBGN,    EN_CHANGE,     & Waver::NewLBgn},
   {IDC_LEND,    EN_CHANGE,     & Waver::NewLEnd},
   {0}
};


//------------------------------------------------------------------------------
void Waver::Load (LPARAM l)
{ File f;
  TStr name;
   StrCp (name, _midiPath);
   if (f.AskR (name, "load a .wav file",
                     "Wav Files (*.wav)\0*.wav\0"
                     "All Files (*.*)\0*.*\0"))
      {_wave->Load (name);  Redo (0);}
}


void Waver::Redo    (LPARAM l)  {App.Updt ();}
void Waver::Save    (LPARAM l)  {_wave->Save   ();}
void Waver::Play    (LPARAM l)  {_wave->Play   ('*');}
void Waver::PlayFr  (LPARAM l)  {_wave->Play   ('f');}
void Waver::PlayTo  (LPARAM l)  {_wave->Play   ('t');}
void Waver::Stop    (LPARAM l)  {_wave->Stop   ();   }
void Waver::SetLp   (LPARAM l)  {_wave->SetLp  ();     Redo (0);}
void Waver::SetBgn  (LPARAM l)  {_wave->SetMrk ('[');  Redo (0);}
void Waver::SetEnd  (LPARAM l)  {_wave->SetMrk (']');  Redo (0);}
void Waver::SetLBgn (LPARAM l)  {_wave->SetMrk ('(');  Redo (0);}
void Waver::SetLEnd (LPARAM l)  {_wave->SetMrk (')');  Redo (0);}
void Waver::NewBgn  (LPARAM l)  {_wave->NewMrk ('[');  Redo (0);}
void Waver::NewEnd  (LPARAM l)  {_wave->NewMrk (']');  Redo (0);}
void Waver::NewLBgn (LPARAM l)  {_wave->NewMrk ('(');  Redo (0);}
void Waver::NewLEnd (LPARAM l)  {_wave->NewMrk (')');  Redo (0);}
void Waver::All     (LPARAM l)  {_wave->All ();        Redo (0);}
void Waver::Chop    (LPARAM l)  {_wave->Chop ();}


void Waver::FrqNew (LPARAM l)
{  if (_wave->_wo.On ()) {
      _wave->_frq = _wave->_fmt.nSamplesPerSec = _c.frq.Get ();
      _wave->_wo.Open (& _wave->_fmt);
   }
}


//------------------------------------------------------------------------------
void Waver::Size (char *state, uword w, uword h)
// resize the wave display if needed
{  if (*state != '.')               return;
// if (! WaveShow.SetYet ())        return;
  uword x, y, w1, h1;
   _c.show.XY (& x, & y);  _c.show.WH (& w1, & h1);
   _c.show.Move (0, y, w, h-y);
   Redo (0);
}


bool Waver::Msg (LRESULT *r, HWND wnd, UINT msg, WPARAM w, LPARAM l)
{ uword w1, h1;
  ubyte mag;
  ulong p, max, ofs;
   if (msg == WM_HSCROLL) {
   // if (! WaveShow.SetYet ())  return false;

   // get w1, h1, p and mag
      _c.show.WH (& w1, & h1);      if (w1 < 50)                  w1  = 50;
      p = _c.scrl.Pos ();           if ((ulong)p >= _wave->_len)  p   = 0;
      mag = (ubyte) _c.mag.Pos ();  if (mag > 21)                 mag = 21;

   // skip out if no scroll bar needed
      ofs = w1;
      if (mag < 3) {ofs >>= (2-mag);   if (_wave->_len <= ofs) return false;}
      else         {ofs <<= (mag-2);   if (_wave->_len <= ofs) return false;}
      max = _wave->_len-ofs;

   // offset p per our wparam
      switch (LOWORD(w)) {
      case SB_LINELEFT  :  if (p)                p--;
                           break;
      case SB_LINERIGHT :  if (p < max)          p++;
                           break;
      case SB_PAGELEFT  :  if (p         >= ofs) p -= ofs;  else p = 0;
                           break;
      case SB_PAGERIGHT :  if ((p + ofs) <= max) p += ofs;  else p = max;
                           break;
      case SB_THUMBTRACK:  p = _c.scrl.PosNew ();
                           break;
      }
      _c.scrl.SetPos (p);   _wave->SetPos (p);
      Redo (0);
      *r = 0;  return true;
   }
   return false;
}


//------------------------------------------------------------------------------
void Waver::Open ()
{ char b [13];
   App.Path (_midiPath, 'd');   StrAp (_midiPath, "\\device\\syn");
   _c.tbar.Init (App.wndo, IDB_TBAR, TBarID,
      "load a new wave\0"
      "save as a new file\0"
      "play wave (max 60 secs)\0"
      "play 5 secs of wave to pos\0"
      "play 5 secs of wave from pos\0"
      "stop playing\0"
      "set loopBgn,loopEnd to all/none/asLoaded/asLoadedFixup\0"
      "chop wave on silent spots into seperate files\0"
   );
   _c.show.Init (App.wndo, IDC_SHOW, this);
   _c.scrl.Init (App.wndo, 0, 'h');
   _c.info.Init (App.wndo, IDC_INFO);
   _c.mag.Init  (App.wndo, IDC_MAG);
   _c.pos.Init  (App.wndo, IDC_POS);
   _c.bgn.Init  (App.wndo, IDC_BGN);
   _c.end.Init  (App.wndo, IDC_END);
   _c.lBgn.Init (App.wndo, IDC_LBGN);
   _c.lEnd.Init (App.wndo, IDC_LEND);
   _c.frq.Init  (App.wndo, IDC_FRQ);
   _c.key.Init  (App.wndo, IDC_KEY);
   _c.cnt.Init  (App.wndo, IDC_CNT);
   _c.loop.Init (App.wndo, IDC_LOOP);
   _c.setLBgn.Init (App.wndo, IDC_SETLBGN);
   _c.setLEnd.Init (App.wndo, IDC_SETLEND);
   _c.mag.Clear ();
   _c.mag.LstIns ("1/4");   _c.mag.LstIns ("1/2");
   for (ubyte i = 0; i <= 19; i++)  _c.mag.LstIns (Int2Str (1 << i, b));
   _c.mag.SetPos (2);
   TipCtl.Init  (App.wndo);
   TipCtl.Reg   (_c.info.Wndo (), "wave bitsPerSample info");
   TipCtl.Reg   (_c.frq.Wndo (),  "sampling frequency - samples/second");
   TipCtl.Reg   (_c.key.Wndo (),  "sampling midi key - octaveNote");
   TipCtl.Reg   (_c.cnt.Wndo (), "sampling midi key offset in cents (-99..99)");
   TipCtl.Reg   (_c.mag.Wndo (),  "magnification (sample/pixel ratio)");
   TipCtl.Reg   (_c.bgn.Wndo (),  "selection begin point");
   TipCtl.Reg   (_c.end.Wndo (),  "selection end point");
   TipCtl.Reg   (_c.lBgn.Wndo (), "loop begin point");
   TipCtl.Reg   (_c.lEnd.Wndo (), "loop end point (.WAV format actually stores "
                                 "1st sample pos AFTER loop)");
   TipCtl.Reg   (_c.loop.Wndo (), "looping on/off flag");

   _wave = new Wave (& _c);

   PosLoad ("Waver_Wndo");
  RECT r;
   ::GetClientRect (App.wndo, & r);
   Size (".", (uword)r.right, (uword)r.bottom);

   if (*App.parm)  {_wave->Load (App.parm);  Redo (0);}
}


bool Waver::Shut ()
{
TRC("{ Waver::Shut");
   PosSave ("Waver_Wndo");
   delete _wave;
TRC("} Waver::Shut");
   return true;
}


int Go ()
{ ClsCstm cc;   cc.Init  ();
  Waver   dlg;  dlg.Init ();   return App.EvPump ();
}
