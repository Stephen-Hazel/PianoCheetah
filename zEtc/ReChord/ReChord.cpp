// ReChord.cpp - store all of a midi in dev's evs to a file (to start off w)

//#define TRC_ON
#include "resource.h"
#include <ui.h>
#include <MidiIO.h>

struct CtlLst {
   Control time, list;
   CtlEdit chan;
   CtlBttn bttn;
};

const enum AppMsg  {MSG_TIMER = WM_APP+20,  MSG_MODE, MSG_CHAN};

const ulong MAX_EV = 1024*1024;


//------------------------------------------------------------------------------
class Song: public Thread {
public:
   Song (CtlLst *c): Thread (App.wndo), _c (c)
   {  _tm = NULL;   *_dv = '\0';   _mi = NULL;   _mo = NULL;
      _ch = 0;
      _rep = _pos = _eBgn = _eEnd = _now = 0;   _onBt = true;
      _mode = 'o';  // off => arm => record => play =>
   }

   char *TmStr (char *str, ulong tm, ulong *tL8r = NULL);
   void  Put (), Get (), NextMode ();
   DWORD Go ();
private:
   CtlLst *_c;
   DWORD   _syn;
   TStr    _dv;
   Timer  *_tm;
   MidiI  *_mi;
   MidiO  *_mo;
   ulong   _now, _eBgn, _eEnd, _rep, _pos;
   bool    _onBt;
   char    _mode;
   ubyte   _ch;
   Arr<MidiEv,MAX_EV> _e;
};


char *Song::TmStr (char *str, ulong tm, ulong *tL8r)
// put song time into a string w bar.beat;  maybe return time of next bt
{ int br, bt, bx;
   if (tL8r) {                         // absolute time,get tL8r,no bx
      br = 1 + (tm / M_WHOLE);
      bt = 1 + (tm % M_WHOLE) / (M_WHOLE/4);
      *tL8r = (br-1) * M_WHOLE + bt * (M_WHOLE/4);
      sprintf (str, "%04d.%d", br, bt);
   }
   else {
      tm = tm + M_WHOLE - _eBgn;       // relative to _eBgn,no tL8r,get bx
      br = tm / M_WHOLE;
      bt = 1 + (tm % M_WHOLE) / (M_WHOLE/4);
      bx = (tm % (M_WHOLE/4)) / 8;
      sprintf (str, "%04d.%d.%02d", br, bt, bx);
   }
   return str;
}


void Song::Put ()
// PianoCheetah's heartbeat:
// writes current slice of song (.p .. songtime) to midiouts;  updates screen
{ ulong tL8r, t;
  TStr  bar;
   while (_tm->Get () >= _now) {
   // get bar.beat str for now n tL8r (default to wakeup on next subbeat)
      _c->time.Set (TmStr (bar, _now, & tL8r));   _onBt = true;

//TRCF("Str=%s _now=%d tl8r=%d tmr=%d",&bar[1],_now,tL8r,_tm->Get());
      if (_mode == 'p')  for (;;) {
         t = _e [_pos].time + _eEnd + _rep * (_eEnd - _eBgn) - _eBgn;
//DBGF("now=%d tL8r=%d pos=%d/%d rep=%d t=%d eBgn=%d eEnd=%d e.time=%d",
//_now, tL8r, _pos, _e.Ln, _rep, t, _eBgn, _eEnd, _e [_pos].time);
         if (t <= _now) {
            _mo->Put (_e [_pos].chan, _e [_pos].ctrl, _e [_pos].valu, 0);
            if (++_pos >= _e.Ln)  {_pos = 0;   _rep++;}
         }
         else {
            if (t < tL8r)  {tL8r = t;   _onBt = false;}
            break;
         }
      }
      _now = tL8r;
   }
   _tm->SetSig (_now);                 // new wakeup
}


void Song::Get ()
{ MidiEv ev;
  ulong  r;
  char   buf [8192];
  TStr   t, t2;
TRC("{ Song::Get");
   while (_mi->Get (& ev))  if (! _e.Full ()) {
TRC("got ev");
      if ( ((_mode == 'a') || (_mode == 'r')) &&
           (! (ev.ctrl & 0xFF80)) && (ev.chan == _ch) ) {
         if (_mode == 'a') {
            _eBgn = ev.time / M_WHOLE * M_WHOLE;
            if ((ev.time % M_WHOLE) > (M_WHOLE*3/4))  _eBgn += M_WHOLE;
            _e.Ln = _rep = _pos = 0;
            _mode = 'r';   _c->bttn.Set ("record=>play");
         }
         r = _e.Ins ();   MemCp (& _e [r], & ev, sizeof (ev));
         *buf = '\0';
         for (r = 0;  r < _e.Ln;  r++) {
            sprintf (& buf [StrLn (buf)], "%s %s %s",
                     TmStr (t, _e [r].time),
                     (_e [r].valu & 0x80) ? "Dn" : "Up",
                     MKey2Str (t2, (ubyte)_e [r].ctrl));
            while ( (r+1 < _e.Ln) &&
                    ((_e [r].valu & 0x80) == (_e [r+1].valu & 0x80)) &&
                    ((_e [r+1].time - _e [r].time) < 24) ) {
               sprintf (& buf [StrLn (buf)], " %s",
                        MKey2Str (t2, (ubyte)_e [r].ctrl));
               r++;
            }
            StrAp (buf, "\r\n");
         }
         _c->list.Set (buf);
      }
   }
TRC("} Song::Get");
}


void Song::NextMode ()
{ char *s;
  ulong t;
DBG("{ Song::NextMode");
   switch (_mode) {
      case 'o': s = "arm=>record";   break;
      case 'r': s = "play=>off";     break;
      case 'a':                        // manual mode change from arm => off
      case 'p': s = "off=>arm";      break;
   }
   _c->bttn.Set (s);   _mode = *s;
   switch (_mode) {
      case 'o': _mo->NotesOff ();
                _e.Ln = _rep = _pos = 0;   _c->list.Set ("");
                break;
      case 'p': t = _tm->Get ();
                _eEnd = t / M_WHOLE * M_WHOLE;
                if ((t % M_WHOLE) >= M_WHOLE/4)  _eEnd += M_WHOLE;
                _rep = _pos = 0;
                break;
   }
DBG("} Song::NextMode");
}


DWORD Song::Go ()
{ MSG  msg;
  bool noClose = true;
  TStr fn, a;
  int  w;
DBG("{ Song::Go");
// hey wihnders!  gimme a message queue.
   ::PeekMessage (& msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);  msg.message = 0;
   ::CoInitialize (NULL);

// can wasapi n got any syn devtypes?  boot n wait for syn.exe to say hi
   _syn = NULL;
   App.CfgGet ("syn", fn);
   if (*fn == 'y') {
DBG("bootin SYN");
      App.Path (a);   StrAp (a, "\\syn.exe");   BOOT (a, "");   // boot syn
     ShMem sm;
      for (w = 40;  --w;)
         {if (! sm.Open ("syn.exe", sizeof (SynDef)))  ::Sleep (50);
          else                                         break;}
      if (! w)  Die ("can't boot syn.exe :(");
      _syn = *((DWORD *)sm.Ptr ());    // get syn.exe's thread id
      sm.Shut ();
DBG("syn is up");
   }

   _tm = new Timer ();                 // boot timer
   _tm->SetSig (1, ThrdID (), MSG_TIMER);

  TStr  iname, t, xs;                  // get midi dev name
  ubyte i = 0;
   Midi.Load ();
   _dv [0] = '\0';
   while (Midi.GetPos ('i', i++, iname, t, xs, xs))
      if (StrCm (t, "OFF") && Midi.Get ('o', iname, t, xs, xs))
         {StrCp (_dv, iname);   break;}
DBGF("dev=%s", _dv);
   if (! _dv [0])  Die ("couldn't find a midi in device with same named "
                        "midi out device",
                        "First midi in with midi out of same "
                        "name is the device ReChord uses");
   App.SetTitle ("ReChord", _dv);

// open midi in, out
   if (! (_mi = new MidiI (_dv, _tm, NULL, 's')))
                                   Die ("couldn't open midi in",  _dv);
   if (! (_mo = new MidiO (_dv)))  Die ("couldn't open midi out", _dv);
   _tm->Set (0);

DBG("loopin'");
   while (noClose) {
      while (::PeekMessage (& msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
         switch (msg.message) {
            case WM_CLOSE:  noClose = false;     break;
            case MSG_TIMER:                      break;
            case MIM_DATA:  Get ();              break;
            case MSG_MODE:  NextMode ();         break;
            case MSG_CHAN:  _ch = msg.wParam;    break;
            default:                             break;
         }
      }
      Put ();
      if (noClose && (! ::PeekMessage (& msg, (HWND)NULL, 0, 0, PM_NOREMOVE)))
         ::WaitMessage ();
   }
DBG("loopin' done");
   delete _mo;   delete _mi;   delete _tm;       // clean up
   if (_syn)  PosTM (_syn, MSG_CLOSE, 0, 0);

   PostDadW (MSG_CLOSE, 0, 0);                   // ok, you can close too, Pop
DBG("} Song::Go");
   return 0;
}


//------------------------------------------------------------------------------
class ReChord: public DialogTop {
public:
   ReChord (): DialogTop (IDD_APP, IDI_APP)  {_s = NULL;}
  ~ReChord ()                         {delete _s;}

private:
   void Mode (LPARAM l)  {_s->PostKid (MSG_MODE, 0, 0);}

   void Chan (LPARAM l)
   { TStr ch;
     int  c;
DBG("{ ReChord::Chan");
      _c.chan.GetText (ch, sizeof (ch));   c = Str2Int (ch);
      if ((c <= 1) || (c >= 16))  c = 1;
      _s->PostKid (MSG_CHAN, c-1, 0);
DBG("} ReChord::Chan");
   }

   virtual void Open ()
   { TStr ch;
DBG("{ ReChord::Open");
      InitComCtl ();
      _c.time.Init (Wndo(), IDC_TIME);   _c.chan.Init (Wndo(), IDC_CHAN);
      _c.bttn.Init (Wndo(), IDC_BTTN);   _c.list.Init (Wndo(), IDC_LIST);
      _c.time.Set ("0001.1");   _c.chan.Set ("1");   _c.list.Set ("");
      PosLoad ("ReChord_Wndo");        // reload window pos
      App.CfgGet ("ReChord_Chan", ch);   if (*ch) _c.chan.Set (ch);
      _s = new Song (& _c);
      ::Sleep (50);                    // and wait for thead to be init'd :(((
DBG("} ReChord::Open");
   }

   virtual bool Shut ()
   { TStr ch;
DBG("{ ReChord::Shut");
      _c.chan.GetText (ch, sizeof (ch));   App.CfgPut ("ReChord_Chan", ch);
      PosSave ("ReChord_Wndo");   _s->PostKid (WM_CLOSE,0,0);
DBG("} ReChord::Shut");
      return false;
   }

   virtual bool Do   (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }

   virtual void Size (char *state, uword w, uword h)
   { uword x, y;
      if (*state != '.')  return;
      if (w < 290) w = 290;    if (h < 126) h = 126;
      _c.list.XY (& x, & y);   _c.list.Move (x, y, w-x, h-y);
   }

   static EvMap<ReChord> _evMap [];
   CtlLst _c;
   Song  *_s;
};


EvMap<ReChord> ReChord::_evMap [] = {
   {IDC_BTTN, BN_CLICKED, & ReChord::Mode},   // off=>arm=>record=>play=>
   {IDC_CHAN, EN_CHANGE,  & ReChord::Chan},
   {0}
};


//------------------------------------------------------------------------------
int Go ()
{ int rc;
DBG("{ ReChord Go");
   { ReChord dlg;   dlg.Init ();   rc = App.EvPump ();}
DBG("} ReChord Go");
   return rc;
}
