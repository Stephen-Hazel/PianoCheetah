// midicfg.cpp - midi configurin - edit stuff in pianocheetah/device dir
// /device.txt
// /cc.txt  (comes from install)
// these all have to be d/l via devtype or text edited:
//    /<devtype>/ccin.txt  ccmap.txt  ccout.txt  sound.txt

#include "midicfg.h"

TStr SynCfg;
BStr DevTyp;

void InitDevType ()
{ TStr   dev;
  StrArr a;
  ubyt2  n, i, q;
  char  *p;
   App.Path (dev, 'd');   StrAp (dev, CC("/device"));
   a.GetDir (dev);   n = a.NRow ();
   StrCp (DevTyp, CC("OFF"));
   for (q = 4, i = 0;  i < n;  i++)    // make a zz str for gui
      {StrCp (& DevTyp [q], p = a.Get (i));   q += StrLn (p) + 1;}
   DevTyp [q] = '\0';
}


char TPop (char *ls, ubyt2 r, ubyte c)
{  *ls = '\0';
   if (c == 1) {ZZCp (ls, DevTyp);   return 'l';}
   if (c == 0)                       return 'e';
   return '\0';
}


void MidiCfg::ShutMIn ()
{  while (_nMI) {                      // shut midi ins
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
{ ubyte i;
  TStr  nm, ty, ds, dv, chk;
  char *rp [4];
   Midi.Load ();   Snd.Load ();
   *chk = '\0';
   for (i = 0;  Midi.GetPos ('i', i, nm, ty, ds, dv);  i++)
      if  (*dv == '?')  {StrFmt (chk, "`s/`s/`s", nm, ty, ds);   break;}
  bool syn = false;                    // add syn if missin
   for (i = 0;  Midi.GetPos ('o', i, nm, ty, ds, dv);  i++) {
      if (! StrCm (ty, CC("syn")))  syn = true;
      if ((*dv == '?') && (StrCm (ty, CC("syn"))))
                        {StrFmt (chk, "`s/`s/`s", nm, ty, ds);   break;}
   }
   if ((! syn) && (Midi._len < MAX_DEV)) {
             Midi._lst [Midi._len].io = 'o';
      StrCp (Midi._lst [Midi._len].name, CC("syn"));
      StrCp (Midi._lst [Midi._len].type, CC("syn"));
      StrCp (Midi._lst [Midi._len].desc, CC("PianoCheetah Synthesizer"));
      StrCp (Midi._lst [Midi._len].dev,  CC("!"));
      Midi._len++;
   }
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
   _ti.Open ();   _to.Open ();
   for (i = 0;  Midi.GetPos ('i', i, nm, ty, ds, dv);  i++)  _ti.Put (rp);
   for (i = 0;  Midi.GetPos ('o', i, nm, ty, ds, dv);  i++)  _to.Put (rp);
   _ti.Shut ();   _to.Shut ();
  CtlList so (ui->lstSyn);
   so.ClrLs ();
   for (i = 0;  i < Snd.len;  i++)  so.InsLs (Snd.lst [i].desc);
   so.InsLs (CC("OFF"));
   if (*SynCfg)  so.SetS (SynCfg);
   RedoMIn ();
}

void MidiCfg::Save ()
{ TStr  fn, ts, dt [64];
  BStr  buf;
  ubyte i, d, n;
  bool  got;
  File  f;
   if (Midi._len == 0)
      Gui.Hey ("You don't SEEM to have any midi devices :(");
   App.Path (fn, 'd');   StrAp (fn, CC("/device/device.txt"));
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

  CtlList so (ui->lstSyn);
   so.GetS (SynCfg);
   App.Path (fn, 'd');   StrAp (fn, CC("/device/syn.txt"));
   f.Save (fn, SynCfg, StrLn (SynCfg));
   emit Reload ();
}


void MidiCfg::Mv (char du)
{  if ((_io != 'i') && (_io != 'o'))  {Gui.Hey ("Click a row, dude");   return;}
  ubyte cr, r;
  TStr  n;
  CtlTabl *t = (_io == 'i') ? & _ti : & _to;
   StrCp (n, t->Get (cr = t->CurRow (), 0));
   for (r = 0;  r < Midi._len;  r++)
      if ((_io == Midi._lst [r].io) && (! StrCm (n, Midi._lst [r].name)))
         break;
// can't use RecMv's built in rec check cuz 2 weird lists in arr
   if ((du == 'u') && (cr == 0))
      {Gui.Hey ("you're already at the top");      return;}
   if ((du == 'd') && (cr == t->NRow ()-1))
      {Gui.Hey ("you're already at the bottom");   return;}
   RecMv (Midi._lst, Midi._len, sizeof (Midi._lst [0]), r, du);
   Save ();
   t->HopTo ((du == 'd') ? cr+1 : cr-1, 0);
}

void MidiCfg::Dn ()  {Mv ('d');}
void MidiCfg::Up ()  {Mv ('u');}


void MidiCfg::Updt ()
{  if ((_io != 'i') && (_io != 'o'))  {Gui.Hey ("Click a row, dude");   return;}
  CtlTabl *t = ((_io == 'i') ? & _ti : & _to);
  TStr  nm, ty, ds;
  ubyte c = t->CurCol ();
  sbyt2 r = t->CurRow (), ro;
//DBG("Updt r=`d c=`d", r, c);
   StrCp (nm, t->Get (r, 0));   StrCp (ty, t->Get (r, 1));
   StrCp (ds, t->Get (r, 2));
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
  CtlText tx (ui->txtEv);
   StrCp (t, tx.Get ());
   ln = 0;
   for (i = 0;  i < StrLn (t);  i++)  if (t [i] == '\n')  ln++;
   if (ln >= 20)  StrCp (t, StrCh (t, '\n') + 1);
   StrAp (t, buf);   StrAp (t, CC("\n"));
   tx.Clr ();   tx.Add (t);
}


void MidiCfg::TestO ()
{  if (! StrCm (_to.Get (_to.CurRow (), 1), CC("syn")))  return;
  MidiO m (_to.Get (_to.CurRow (), 0), 'x');    // no gm init
   m.Put (9, MDrm(CC("snar")), 0x80|90);   m.Put (0, MKey (CC("4C")), 0x80|90);
   Zzz (300);                          // 3/10 sec
   m.Put (9, MDrm(CC("snar")),      64);   m.Put (0, MKey (CC("4C")),      64);
   Zzz (300);                          // 3/10 sec
}


void MidiCfg::MidiIEv ()
{ MidiEv e;
   for (ubyte i = 0; i < _nMI; i++)  while (_mi [i]->Get (& e))  TestI (i, e);
}


void MidiCfg::Init ()
{  _io = ' ';   _nMI = 0;
DBG("Init");
   InitDevType ();
   Gui.WinLoad ();
  TStr fn;
  File f;
  ulong ln;
   App.Path (fn, 'd');   StrAp (fn, CC("/device/syn.txt"));
   ln = f.Load (fn, SynCfg, sizeof (SynCfg)-1);
   SynCfg [ln] = '\0';
   while (ln && (SynCfg [ln-1] == '\n'))  SynCfg [--ln] = '\0';
  CtlTBar tb (this,
      "Refresh device lists\n"
       "(if you've installed/uninstalled/forgot to power on devices)"
                                "`:/tbar/0" "`\0"
      "Scoot device up a row"   "`:/tbar/1" "`\0"
      "Scoot device down a row" "`:/tbar/2" "`\0"
   );
   connect (tb.Act (0), & QAction::triggered, this, & MidiCfg::Load);
   connect (tb.Act (1), & QAction::triggered, this, & MidiCfg::Up);
   connect (tb.Act (2), & QAction::triggered, this, & MidiCfg::Dn);
   _ti.Init  (ui->tblI,
      "_input device\0"
      "_type\0"
      "driver description\0",
      TPop);
   _to.Init  (ui->tblO,
      "_output device\0"
      "_type\0"
      "driver description\0",
      TPop);
   connect (ui->tblI, & QTableWidget::itemChanged, this, & MidiCfg::Updt);
   connect (ui->tblO, & QTableWidget::itemChanged, this, & MidiCfg::Updt);
   connect (ui->tblO, & QTableWidget::itemClicked, this, & MidiCfg::TestO);
   ui->tblI->installEventFilter (this);
   ui->tblO->installEventFilter (this);

   connect (this, & MidiCfg::Reload, this, & MidiCfg::Load,
            Qt::QueuedConnection);
   emit Reload ();
DBG("Init end");
}


void MidiCfg::Quit ()  {Save ();   ShutMIn ();   Gui.WinSave ();}


int main (int argc, char *argv [])
{ QApplication app (argc, argv);
  MidiCfg      win;
DBGTH("MidiCfg");
DBG("bgn");
   qRegisterMetaType<ubyte>("ubyte");
   qRegisterMetaType<Qt::MouseButtons>("Qt::MouseButtons");
   App.Init ();   Gui.Init (& app, & win, "MidiCfg");   win.Init ();
  int rc = Gui.Loop ();                                 win.Quit ();
   App.Spinoff (CC("pianocheetah"));
DBG("end");
   return rc;
}
