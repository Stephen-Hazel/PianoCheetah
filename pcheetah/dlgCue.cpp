// dlgCue.cpp - pick a new drum track per section (patA,patB,fill)

#include "pcheetah.h"

void DlgCue::Set (const char *s, char stay)
{ CtlLine c (ui->str);
  TStr st;
   StrCp (Up.pos.str, CC(s));
   if (! stay)  Shut ();   

   StrCp (st, c.Get ());
   if (*st != '(')  {StrCp (& st [1], st);   *st = '(';   c.Set (st);}
}

void DlgCue::Open ()  
{  show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "tL");
}

void DlgCue::Shut ()  {done (true);   lower ();   hide ();}

void DlgCue::Init ()
{  Gui.DlgLoad (this, "DlgCue");
  CtlTBar tb (this,
      "redo loops and erase all bug history"
                         "`:/tbar/cue/a0"  "`\0"
      "delete this cue"  "`:/tbar/cue/a1"  "`\0"
      "verse"            "`:/tbar/cue/b0"  "`\0"
      "chorus"           "`:/tbar/cue/b1"  "`\0"
      "break"            "`:/tbar/cue/b2"  "`\0"
      "section\nother section (intro, coda, bridge, etc)\n"
      "name in textbox below  with leading ("
                         "`:/tbar/cue/b3"  "`\0"
      "crescendo"        "`:/tbar/cue/b4"  "`\0"
      "decrescendo"      "`:/tbar/cue/b5"  "`\0"
      "fermata"          "`:/tbar/cue/c0"  "`\0"
      "tremelo"          "`:/tbar/cue/c1"  "`\0"
      "star"             "`:/tbar/cue/c2"  "`\0"
      "happy"            "`:/tbar/cue/c3"  "`\0"
      "sad"              "`:/tbar/cue/c4"  "`\0"
      "mad"              "`:/tbar/cue/c5"  "`\0"
      "piano x3"         "`*ppp"           "`\0"
      "piano x2"         "`*pp"            "`\0"
      "piano"            "`*p"             "`\0"
      "mezzo piano"      "`*mp"            "`\0"
      "mezzo forte"      "`*mf"            "`\0"
      "forte"            "`*f"             "`\0"
      "forte x2"         "`*ff"            "`\0"
      "forte x3"         "`*fff"           "`\0"
      "forzando"         "`*Fz"            "`\0"
   );
   connect (tb.Act (0),  & QAction::triggered, this, [this]()
                                                           {Set ("loopInit");});
   connect (tb.Act (1),  & QAction::triggered, this, [this]()  {Set ("");});
   connect (tb.Act (2),  & QAction::triggered, this, [this]()
                                                           {Set ("(verse");});
   connect (tb.Act (3),  & QAction::triggered, this, [this]()
                                                           {Set ("(chorus");});
   connect (tb.Act (4),  & QAction::triggered, this, [this]()
                                                           {Set ("(break");});
   connect (tb.Act (5),  & QAction::triggered, this, [this]()
                                                           {Set ("(", 'y');});
   connect (tb.Act (6),  & QAction::triggered, this, [this]()  {Set ("<");});
   connect (tb.Act (7),  & QAction::triggered, this, [this]()  {Set (">");});
   connect (tb.Act (8),  & QAction::triggered, this, [this]()  {Set ("`fer");});
   connect (tb.Act (9),  & QAction::triggered, this, [this]()  {Set ("`tre");});
   connect (tb.Act (10), & QAction::triggered, this, [this]()  {Set ("`sta");});
   connect (tb.Act (11), & QAction::triggered, this, [this]()  {Set ("`hap");});
   connect (tb.Act (12), & QAction::triggered, this, [this]()  {Set ("`sad");});
   connect (tb.Act (13), & QAction::triggered, this, [this]()  {Set ("`mad");});
   connect (tb.Act (14), & QAction::triggered, this, [this]()  {Set ("ppp");});
   connect (tb.Act (15), & QAction::triggered, this, [this]()  {Set ("pp");});
   connect (tb.Act (16), & QAction::triggered, this, [this]()  {Set ("p");});
   connect (tb.Act (17), & QAction::triggered, this, [this]()  {Set ("mp");});
   connect (tb.Act (18), & QAction::triggered, this, [this]()  {Set ("mf");});
   connect (tb.Act (19), & QAction::triggered, this, [this]()  {Set ("f");});
   connect (tb.Act (20), & QAction::triggered, this, [this]()  {Set ("ff");});
   connect (tb.Act (21), & QAction::triggered, this, [this]()  {Set ("fff");});
   connect (tb.Act (22), & QAction::triggered, this, [this]()  {Set ("Fz");});
}

void DlgCue::Quit ()  {Gui.DlgSave (this, "DlgCue");}
