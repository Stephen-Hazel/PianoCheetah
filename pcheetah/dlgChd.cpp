// dlgChd.cpp - edit chords

#include "pcheetah.h"

static char *IntvMap = CC("1m2m34d5a6m7");


void DlgChd::Cmd (char *s)
{ TStr u;
   emit sgCmd (StrFmt (u, "chd `s", s));
}


void DlgChd::ReDo ()
{ TStr s, s2, chd;
TRC("DlgChd::ReDo");
// pull our chord picks to make chd string
  CtlList root (ui->root), type (ui->type), bass (ui->bass);
   root.GetS (chd);   StrCp (s, CC(MChd [type.Get ()].lbl));   bass.GetS (s2);
   if (! StrCm (chd, CC("(none)")))  *chd = '\0';     // no chord at all?
   else {
      StrAp (chd, s);
      if (StrCm (s2, CC("(none)")))  {StrAp (chd, CC("/"));   StrAp (chd, s2);}
   }
// enable/dis per picks
                 type.Enable (true);    bass.Enable (true);
   if (! *chd)  {type.Enable (false);   bass.Enable (false);
                 type.Set (0);          bass.Set (0);}
// update dat song's _f.chd (ins/del/upd) based on _got,_cp
   Cmd (StrFmt (s, ".`b `d `d `s", _got, _cp, _tm, chd));
   _got = *chd ? true : false;
TRC("DlgChd::ReDo end");
}


void DlgChd::Set (char *s)
{ ubyte i, j;
  TStr  ch;
  char *c;
   StrCp (ch, s);
  CtlList root (ui->root), type (ui->type), bass (ui->bass);
   root.Set (0);   type.Set (0);   bass.Set (0);
   if ((c = StrCh (ch, '/'))) {        // if got bass, chop if off n set ctrl
      *c++ = '\0';
      for (i = 0;  i < 12;  i++)  {if (! StrCm (c, MKeyStr  [i]))  break;
                                   if (! StrCm (c, MKeyStrB [i]))  break;}
      if (i < 12)  bass.Set (1+i);
   }
   if (*ch) {
      i = ((ch [1] == 'b') || (ch [1] == '#')) ? 2 : 1;    // type pos
      c = & ch [i];
      for (j = 0;  j < NMChd;  j++)  if (! StrCm (c, CC(MChd [j].lbl), 'x'))
                                        {type.Set (j);   break;}
      *c = '\0';                       // chop it off
   }
   for (i = 0;  i < 12;  i++)  {if (! StrCm (ch, MKeyStr  [i]))  break;
                                if (! StrCm (ch, MKeyStrB [i]))  break;}
   if (i < 12) root.Set (i+1);
}


void DlgChd::Pop ()
{ TStr s;
  CtlList pop (ui->pop);
   pop.GetS (s);   while (*s == ' ')  StrCp (s, & s [1]);   Set (s);
   ReDo ();   Cmd (StrFmt (s, "@`d", _tm1));
}


void DlgChd::UnDo ()
{ TStr s;
  CtlBttn undo (ui->undo);
   undo.Get (s);   StrCp (s, & s [8]);   Set (s);
   ReDo ();   Cmd (StrFmt (s, "@`d", _tm1));
}


//______________________________________________________________________________
void DlgChd::Init ()
{  Gui.DlgLoad (this, "DlgChd");
  CtlTBar tb (this,
      "guess chords\n"
      "   calculate whole song's chords\n"
      "   using notes of selected practice tracks"
         "`:/tbar/chd/0" "`\0"
      "pop chords\n"
      "   plop some random happy pop chords into this section"
         "`:/tbar/chd/1" "`\0"
      "delete all chords  (be carefulll)"
         "`:/tbar/chd/2" "`\0"
   );
   connect (tb.Act (0), & QAction::triggered, this, [this]() {Cmd (CC("?"));});
   connect (tb.Act (1), & QAction::triggered, this, [this]() {Cmd (CC("+"));});
   connect (tb.Act (2), & QAction::triggered, this, [this]() {Cmd (CC("x"));});
   connect (ui->shhh, & QPushButton::clicked, this, [this]()
                                              {emit sgCmd (CC("timePause"));});
   connect (ui->undo, & QPushButton::clicked, this, [this]() {UnDo ();});
   connect (ui->pop,  QOverload<int>::of(& QComboBox::currentIndexChanged),
                                              this, [this]() {Pop  ();});
}


void DlgChd::Open ()
{ ubyt4 i, j;
  ubyte nbtw;
  TStr  *btw, s, it, t, ch, ch1, ch2;
  char  *c;
TRC("DlgChd::Open");
   show ();   raise ();   activateWindow ();
   _got = Up.pos.got ? true : false;
   _cp = Up.pos.cp;   _tm = Up.pos.tm;   _tm1 = Up.pos.tmBt;
   StrCp (ch1, Up.d [0][0]);   StrCp (ch2, Up.d [0][1]);
   nbtw = ChdBtw (& btw, ch1, ch2);
  CtlList pop  (ui->pop);
  CtlBttn undo (ui->undo);
   pop.ClrLs ();
   for (ubyte i = 0;  i < nbtw;  i++) {
     ColSep c (btw [i], 80);
      pop.InsLs (c.Col [0]);
      for (ubyte j = 1;  c.Col [j][0];  j++)
         {StrCp (it, CC(" "));   StrAp (it, c.Col [j]);   pop.InsLs (it);}
   }
   StrCp (ch, Up.pos.str);   StrCp (s, ch);
   if (! nbtw)  {undo.Enable (false);   pop.InsLs (CC("(none)"), 0);}
   else {        undo.Enable (true);
      if (! Up.pos.got)  StrCp (s, CC("(none)"));
      StrCp (it, CC("restore "));   StrAp (it, s);   undo.Set (it);
   }
   pop.Set (0);

// init lists for chord's root/type/bass
   c = Up.pos.kSg.flt ? CC("(none)\0C\0Db\0D\0Eb\0E\0F\0Gb\0G\0Ab\0A\0Bb\0B\0")
                      : CC("(none)\0C\0C#\0D\0D#\0E\0F\0F#\0G\0G#\0A\0A#\0B\0");
  CtlList root (ui->root, c), type (ui->type), bass (ui->bass, c);
   type.ClrLs ();
   for (i = 0;  i < NMChd;  i++) {
      StrCp (t, CC("............"));
      for (j = 0;  MChd [i].tmp [j] != 'x';  j++)
         t [(ubyte)(MChd [i].tmp [j])] = IntvMap [(ubyte)(MChd [i].tmp [j])];
      StrFmt (s, "`<5s  `s  `s", MChd [i].lbl, t, MChd [i].etc?MChd [i].etc:"");
      type.InsLs (s);
   }

// chord control picks (root, type, bass)  (n disabling)
   Set (ch);   ReDo ();
   Gui.DlgMv (this, Up.gp, "tR");
TRC("} DlgChd::Open");
}


void DlgChd::Shut ()
{ TStr x;
   Cmd (StrFmt (x, "@`d", _tm));
   done (true);   lower ();   hide ();
}


void DlgChd::Quit ()  {Gui.DlgSave (this, "DlgChd");}
