// midicfg.cpp - midi configurin - edit stuff in PianoCheetah's device dir
// /device.txt
// /cc.txt  (comes from install)
// these all have to be d/l via devtype or text edited:
//    /<devtype>/ccin.txt  ccmap.txt  ccout.txt  sound.txt

#include "midicfg.h"

struct {TStr nm, info;   ubyt4 sz;} DTyp [100];   ubyte NDTyp;
BStr DevTyp;
char Buf [1024*1024];   ubyt4 Len;

void InitDevTyp ()
{ ubyt4 p, q;
  TStr  rec;
   Len = WGet (Buf, sizeof (Buf),
               CC("http://PianoCheetah.app/rep/_dev/_list.txt"));
   for (p = 0;  p < Len;) {
      p = NextLn (rec, Buf, Len, p);
     ColSep cs (rec, 2);          // parse a record into name, size, info
      StrCp (DTyp [NDTyp].nm,           cs.Col [0]);
             DTyp [NDTyp].sz = Str2Int (cs.Col [1]);
      StrCp (DTyp [NDTyp].info,         cs.Col [2]);
//DBG("`d name=`s size=`d info=`s",
//NDTyp, DTyp [NDTyp].nm, DTyp [NDTyp].sz, DTyp [NDTyp].info);
      NDTyp++;
   }
   if (NDTyp < 2)  DBG ("rats, I neeed the internet");
   StrCp (DevTyp, CC("OFF"));          // make zz str
   for (q = 4, p = 0;  p < NDTyp;  p++) {
      StrCp (& DevTyp [q], DTyp [p].nm);
      q += (StrLn (DTyp [p].nm) + 1);
   }
   DevTyp [q] = '\0';
}


void MidiCfg::ShutMIn ()
{  while (_nMI) {
      --_nMI;
      disconnect (_mi [_nMI], & MidiI::MidiIEv, this, & MidiCfg::MidiIEv);
      delete _mi [_nMI];   _mi [_nMI] = nullptr;
   }
}


void MidiCfg::RedoMIn ()
{ TStr  iname, t, xs;
  ubyte i;
DBG("RedoMIn bgn");
   ShutMIn ();
   i = 0;
   while (Midi.GetPos ('i', i++, iname, t, xs, xs))
                                                    if (StrCm (t, CC("OFF"))) {
      if (_nMI == BITS (_mi))  break;  // got room?
      _mi [_nMI] = new MidiI (iname);
      connect (_mi [_nMI], & MidiI::MidiIEv, this, & MidiCfg::MidiIEv);
      _nMI++;
   }
// sigh...
   for (i = 0;  i < _nMI;  i++)  if (_mi [i]->Dead ())  break;
   if (i < _nMI) {                     // one was dead, let's try ONE mo time
DBG("Eh, oooone mo taaaahm");
      ShutMIn ();
      i = 0;
      while (Midi.GetPos ('i', i++, iname, t, xs, xs))
                                                    if (StrCm (t, CC("OFF"))) {
         if (_nMI == BITS (_mi))  break;    // got room?
         _mi [_nMI] = new MidiI (iname);
         connect (_mi [_nMI], & MidiI::MidiIEv, this, & MidiCfg::MidiIEv);
         _nMI++;
      }
   }
DBG("RedoMIn end");
}


/* turn dev desc into default name,devtype
DBG("  new dsc=`s", dsc [i]);
      StrFmt (c[0], "in");   StrCp (c[1], "DEFAULT");   StrCp (c[2], dsc [i]);
      if      (StrSt (c[2], "MIDISPORT Uno"))
         {StrCp (c[0], "midi");     StrCp (c[1], "DEFAULT");}
      else if (StrSt (c[2], "MiniNova"))
         {StrCp (c[0], "mini");     StrCp (c[1], "novation_mininova");}
      else if (StrSt (c[2], "DTX drums"))
         {StrCp (c[0], "drums");    StrCp (c[1], "yamaha_dtx502");}
      else if (StrSt (c[2], "CME GPP-3"))
         {StrCp (c[0], "pedals");   StrCp (c[1], "cme_gpp3");}
      else if (StrSt (c[2], "Vortex Wireless"))
         {StrCp (c[0], "keytar");   StrCp (c[1], "alesis_vortex");}
*/
void MidiCfg::Load ()
{ Tabl ti (ui->tblI), to (ui->tblO);
  ubyte i;
  TStr  nm, ty, ds, dv, chk;
  char *rp [4];
   Midi.Load ();
   *chk = '\0';
   for (i = 0;  Midi.GetPos ('i', i, nm, ty, & ds [1], dv);  i++)
      if (*dv == '?')  {StrFmt (chk, "`s/`s/`s", nm, ty, & ds [1]);   break;}
   for (i = 0;  Midi.GetPos ('o', i, nm, ty, & ds [1], dv);  i++)
      if (*dv == '?')  {StrFmt (chk, "`s/`s/`s", nm, ty, & ds [1]);   break;}
   if (*chk) {
      Gui.Hey (StrFmt (nm,
         "Is device `s off?\n"
         "Turn on all devices, wait a bit, hit ok to recheck.", chk));
      Midi.Load ();
      *nm = '\0';
      for (i = 0;  i < Midi._len;) {
         if (Midi._lst [i].dev [0] == '?') {
            *nm = 'y';
            RecDel (Midi._lst, Midi._len--, sizeof (Midi._lst [0]), i);
         }
         else i++;
      }
      if (*nm)  Save ();
   }
   rp [0] = nm;   rp [1] = ty;   rp [2] = ds;   rp [3] = nullptr;
                                         *ds = '\r';
   ti.Open ();   to.Open ();
   for (i = 0;  Midi.GetPos ('i', i, nm, ty, & ds [1], dv);  i++)  ti.Put (rp);
   for (i = 0;  Midi.GetPos ('o', i, nm, ty, & ds [1], dv);  i++)  to.Put (rp);
   ti.Shut ();   to.Shut ();
   RedoMIn ();
}

void MidiCfg::Save ()
{ TStr  fn, ts, dt [64];
  BStr  buf;
  ubyte i, d, n;
  bool  got;
  File  f;
DBG("Save");
   if (Midi._len == 0)
      Gui.Hey ("You don't SEEM to have any midi devices :(");
   App.Path (fn, 'c');   StrAp (fn, CC("/device/device.txt"));
   if (! f.Open (fn, "w"))
      {Gui.Hey ("Save couldn't write device.txt");   return;}
// in
   f.Put (CC(
      "# device.txt - list of Midi Devices (In and Out)\n"
      "# Name  Type/OFF  Description\n"
      "#\n"
      "MidiIn:\n"));
   for (i = 0;  i < Midi._len;  i++)
      if ((Midi._lst [i].io == 'i') &&
             StrCm (Midi._lst [i].type, CC("OFF")) )
         f.Put (StrFmt (buf, "`s  `s  `s\n",
            Midi._lst [i].name, Midi._lst [i].type, Midi._lst [i].desc));
// OFFs last...
   for (i = 0;  i < Midi._len;  i++)
      if ((Midi._lst [i].io == 'i') &&
          (! StrCm (Midi._lst [i].type, CC("OFF"))))
         f.Put (StrFmt (buf, "`s  `s  `s\n",
            Midi._lst [i].name, Midi._lst [i].type, Midi._lst [i].desc));
// out
   f.Put (CC(
      "\n"
      "MidiOut:\n"));
   for (i = 0;  i < Midi._len;  i++)
      if ((Midi._lst [i].io == 'o') &&
             StrCm (Midi._lst [i].type, CC("OFF")) )
         f.Put (StrFmt (buf, "`s  `s  `s\n",
            Midi._lst [i].name, Midi._lst [i].type, Midi._lst [i].desc));
   for (i = 0;  i < Midi._len;  i++)
      if ((Midi._lst [i].io == 'o') &&
          (! StrCm (Midi._lst [i].type, CC("OFF"))))
         f.Put (StrFmt (buf, "`s  `s  `s\n",
            Midi._lst [i].name, Midi._lst [i].type, Midi._lst [i].desc));
   f.Shut ();

// collect our distinct devTypes in dt [n]
   n = 0;
   for (i = 0;  i < Midi._len;  i++) {
      StrCp (ts, Midi._lst [i].type);
      for (got = false, d = 0;  d < n;  d++)
         if (! StrCm (ts, dt [d]))  {got = true;   break;}
      if (! got)  StrCp (dt [n++], ts);
   }
// for (d = 0;  d < n;  d++)  DLDevTyp (dt [d]);
// turn off devs if they hit cancel on devtyp d/l

// if ( 0 == system (StrFmt (ts, "cat `s", fn))) {}
   emit Reload ();
}


void MidiCfg::Mv (char du)
{
DBG("a");
   if ((_io != 'i') && (_io != 'o'))  {Gui.Hey ("Click a row, dude");   return;}
  ubyte cr, r;
  Tabl  t ((_io == 'i') ? ui->tblI : ui->tblO);
  TStr  n;
DBG("b");
   StrCp (n, t.Get (cr = t.CurRow (), 0));
   for (r = 0;  r < Midi._len;  r++)
      if ((_io == Midi._lst [r].io) && (! StrCm (n, Midi._lst [r].name)))
         break;
DBG("du=`c io=`c n=`s row=`d => Midi row=`d", du, _io, n, cr, r);
// can't use RecMv's built in rec check cuz 2 weird lists in arr
   if ((du == 'u') && (cr == 0))
      {Gui.Hey ("you're already at the top");      return;}
   if ((du == 'd') && (cr == t.NRow ()-1))
      {Gui.Hey ("you're already at the bottom");   return;}
   RecMv (Midi._lst, Midi._len, sizeof (Midi._lst [0]), r, du);
   Save ();
   t.HopTo ((du == 'd') ? cr+1 : cr-1, 0);
}

void MidiCfg::Dn ()  {Mv ('d');}
void MidiCfg::Up ()  {Mv ('u');}


void MidiCfg::Updt ()
{  if ((_io != 'i') && (_io != 'o'))  {Gui.Hey ("Click a row, dude");   return;}
  Tabl t ((_io == 'i') ? ui->tblI : ui->tblO);
  TStr  nm, ty, ds;
  ubyte c = t.CurCol ();
  sbyt2 r = t.CurRow (), ro;
   StrCp (nm, t.Get (r, 0));   StrCp (ty, t.Get (r, 1));
   StrCp (ds, t.Get (r, 2));
DBG("Updt r=`d c=`d nm=`s ty=`s ds=`s", r, c, nm, ty, ds);
   if (c == 0) {
   // test nonempty, <= 32 chars, no spaces, nondup
      if (StrLn (nm) == 0)
         {Gui.Hey ("name can't be empty");    Load ();   return;}
      if (StrLn (nm) > 32)
         {Gui.Hey ("name is 32 chars max");   Load ();   return;}
      if (StrCh (nm, ' '))
         {Gui.Hey ("no spaces in name");      Load ();   return;}
      for (ro = 0;  ro < Midi._len;  ro++)
         if ( (! StrCm (Midi._lst [ro].name, nm)) &&
                 StrCm (Midi._lst [ro].desc, ds) )
         {Gui.Hey ("no dup names");           Load ();   return;}
   // change both lists for given desc
      for (ro = 0;  ro < Midi._len;  ro++)
         if (! StrCm (Midi._lst [ro].desc, ds))
            StrCp (Midi._lst [ro].name, nm);
   }
   if (c == 1)
      for (ro = 0;  ro < Midi._len;  ro++) {
         if (StrCm (ty, CC("OFF"))) {  // only do both i,o if NOT goin OFF
            if (! StrCm (Midi._lst [ro].desc, ds))
               if (StrCm (Midi._lst [ro].type, CC("OFF")) ||
                         (Midi._lst [ro].io == _io))  // only THIS rec
                  StrCp (Midi._lst [ro].type, ty);
         }
         else
            if (! StrCm (Midi._lst [ro].desc, ds) && (Midi._lst [ro].io == _io))
               StrCp (Midi._lst [ro].type, ty);
      }
   Save ();
}


//------------------------------------------------------------------------------
bool MidiCfg::eventFilter (QObject *ob, QEvent *ev)
{  if (ev->type () == QEvent::FocusIn) {
      if (ob == ui->tblI)  {_io = 'i'; DBG("io=i");}
      if (ob == ui->tblO)  {_io = 'o'; DBG("io=o");}
   }
   return QObject::eventFilter (ob, ev);
}


char *CtrlSt [3] = {CC("Prog"),CC("Prss"),CC("PBnd")};

void MidiCfg::TestI (ubyte mi, MidiEv e)
{ TStr  buf, b2;
  BStr  t;
  ubyt4 i, ln;
   StrFmt (buf, "`s.`d ", (char *)_mi [mi]->Name (), e.chan+1);
   if      (e.ctrl < 128)           // note
      MNt2Str (& buf [StrLn (buf)], & e);
   else if (e.ctrl < MC_CC) {       // std midi ctrl
      if (e.ctrl > MC_PBND)  StrAp (buf, CC("?"));
      else {
         StrFmt (& buf [StrLn (buf)], "`s=`d",
            CtrlSt [e.ctrl-128], (e.ctrl == MC_PBND) ? (e.valu-64) : e.valu);
         if (e.ctrl == MC_PBND)
            StrFmt (& buf [StrLn (buf)], ",`d", e.val2);
         if (e.ctrl == MC_PROG)
            StrFmt (& buf [StrLn (buf)], "=`s(`d)", MProg [e.valu], e.valu);
      }
   }
   else     StrFmt (& buf [StrLn (buf)], "`s=`d",
                                            MCtl2Str (b2, e.ctrl, 'r'), e.valu);
   StrCp (t, UnQS (ui->txtEv->toPlainText ()));
   ln = 0;
   for (i = 0;  i < StrLn (t);  i++)  if (t [i] == '\n')  ln++;
   if (ln >= 20)  StrCp (t, StrCh (t, '\n') + 1);
   StrAp (t, buf);   StrAp (t, CC("\n"));
   ui->txtEv->clear ();   ui->txtEv->insertPlainText (t);
}


void MidiCfg::TestO ()
{ Tabl  t (ui->tblO);
  MidiO m (t.Get (t.CurRow (), 0), 'x');    // no gm init
   m.Put (9, MDrm(CC("snar")), 0x80|90);   m.Put (0, MKey (CC("4C")), 0x80|90);
   Zzz (300);                          // 3/10 sec
   m.Put (9, MDrm(CC("snar")),      64);   m.Put (0, MKey (CC("4C")),      64);
}


void MidiCfg::MidiIEv ()
{ MidiEv e;
   for (ubyte i = 0; i < _nMI; i++)  while (_mi [i]->Get (& e))  TestI (i, e);
}


void MidiCfg::Init ()
{  _io = ' ';
  TBar tb (this,
   "view-refresh"
    "`Refresh device lists\n"
    "(if you've installed/uninstalled/forgot to power on devices)`\0"
   "go-up"
    "`Scoot device up a row`\0"
   "go-down"
    "`Scoot device down a row`\0");
  Tabl ti (ui->tblI), to (ui->tblO);
   ti.SetHdr (
      "input device\0"
      "type\0"
      "driver description\0");
   to.SetHdr (
      "output device\0"
      "type\0"
      "driver description\0");
   connect (tb.Act (0), & QAction::triggered,        this, & MidiCfg::Load);
   connect (tb.Act (1), & QAction::triggered,        this, & MidiCfg::Up);
   connect (tb.Act (2), & QAction::triggered,        this, & MidiCfg::Dn);
   connect (ui->tblI,   & QTableWidget::itemChanged, this, & MidiCfg::Updt);
   connect (ui->tblO,   & QTableWidget::itemChanged, this, & MidiCfg::Updt);
   connect (ui->tblO,   & QTableWidget::itemClicked, this, & MidiCfg::TestO);
   connect (this, & MidiCfg::Reload, this, & MidiCfg::Load,
            Qt::QueuedConnection);
   ui->tblI->installEventFilter (this);
   ui->tblO->installEventFilter (this);
   ui->tblI->setItemDelegate (new CmboDel (ui->tblI));
   ui->tblO->setItemDelegate (new CmboDel (ui->tblO));
   ui->tblI->setEditTriggers (QAbstractItemView::SelectedClicked |
                              QAbstractItemView::AnyKeyPressed);
   ui->tblO->setEditTriggers (QAbstractItemView::SelectedClicked |
                              QAbstractItemView::AnyKeyPressed);
   _nMI = 0;
   emit Reload ();
}


int main (int argc, char *argv [])
{ QApplication app (argc, argv);
  MidiCfg      win;
   App.Init (CC("pcheetah"), CC("MidiCfg"));
   Gui.Init (& app, & win);   win.Init ();   InitDevTyp ();
  int rc = Gui.Loop ();
   win.ShutMIn ();
   return rc;
}


/*
class ThrDLDevTyp: public Thread {
public:
   ThrDLDevTyp (Waiter *w): _w (w), Thread (w->wnd)  {}
private:
   Waiter *_w;

   int End ()  {PostDadW (MSG_CLOSE, 0, 0);   return 0;}

   DWORD Go ()
   { MSG  msg;
     TStr wn, fn, zn, db, arg;
     INet in;
     File f;
     ubyt4 sz;
DBG("{ ThrDLDevTyp::Go `s", _w->arg);
      InitMsgQ ();   msg.message = 0;   StrCp (arg, _w->arg);

      Len = sz = 0;
      for (ubyte i = 0;  i < NDTyp;  i++)
         if (! StrCm (DTyp [i].nm, arg))  {sz = DTyp [i].sz;   break;}
DBG("name=`s size=`d", arg, sz);

      StrFmt (wn, "http://PianoCheetah.app/rep/_dev/`s.zip", arg);
      if (_w->Set (5, "downloading...")) {
DBG("} ThrDLDevTyp::Go - cancel1");
         return End ();
      }
      if (! in.GetInit (wn, Buf, sizeof (Buf))) {
         _w->msg.Set (StrFmt (db, "couldn't open url `s", wn));
DBG("} ThrDLDevTyp::Go - couldn't open `s", wn);
         return End ();
      }
      while (in.GetNext (& Len)) {
         StrFmt (db, "downloading DeviceType=`s  `dK of `dK...", arg, Len/1024,
                                                                       sz/1024);
         if (_w->Set (sz ? (ubyte)(5+(uhuge)70*Len/sz) : 0, db)) {
DBG("} ThrDLDevTyp::Go - cancel2");
            return End ();
         }
      }

      StrFmt (db, "end download - `d bytes;  begin unzip", Len);
      if (_w->Set (75, db)) {
DBG("} ThrDLDevTyp::Go - cancel3");
         return End ();
      }
      if (Len >= sizeof (Buf))  Die ("couldn't download devicetype", arg);
      Buf [Len] = '\0';
      if (StrSt (Buf, "<html>")) {
DBG("} ThrDLDevTyp::Go - couldn't download devicetype :(", arg);
         return End ();
      }
      StrFmt (fn, "`s\\Device\\`s.zip",  App.Path (wn, 'd'), arg);
      f.Save (fn, Buf, Len);
      if (_w->die) {
DBG("} ThrDLDevTyp::Go - died");
         return End ();
      }

      StrCp (zn, fn);   Fn2Name (zn);   Zip (zn, 'u');
      f.Kill (fn);

DBG("} ThrDLDevTyp::Go - OK!");        // ok, you can close (too?), Pop
      _w->rc = 0;                      // it WERKED :)
      return End ();
   }
};


class DlgDLDevTyp: public Dialog {
public:
   DlgDLDevTyp (char *arg): Dialog (IDD_WAIT, IDI_APP), _t (NULL)
                   {StrCp (_w.arg, arg);_w.rc = 99;   DBG(arg);}
  ~DlgDLDevTyp ()  {delete _t;}
   bool Cool ()    {return (_w.rc == 0) ? true : false;}
private:
   ThrDLDevTyp *_t;
   Waiter       _w;
   void Open ()  {_w.Init (Wndo ());   _t = new ThrDLDevTyp (& _w);}
   void Done ()  {_w.Quit ();   _t->PostKid (WM_CLOSE, 0, 0);}
};


void DLDevTyp (char *nm)
{ char *p;
  TStr  s, fn;
  File  f;
DBG("{ DLDevTyp `s", nm);
   StrCp (s, nm);   if (p = StrCh (s, ' '))  *p = '\0';
   App.Path (fn, 'd');   StrAp (fn, "\\device\\");   StrAp (fn, s);

   if ( (! StrCm (s, "OFF")) || f.PathGot (fn) ) {
DBG("} DLDevTyp  (already got or OFF)");
      return;
   }

  DlgDLDevTyp dlg (s);
   dlg.Ok (App.wndo);
   if (! dlg.Cool ()) {                // didn't get all the way done :(
DBG("cancel so killin it");
      f.PathKill (fn);
   }
DBG("} DLDevTyp");
}

*/
