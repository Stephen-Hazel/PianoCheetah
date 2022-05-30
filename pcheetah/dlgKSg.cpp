// dlgKSg.cpp - edit keysig

#include "pcheetah.h"

static char *Ma = CC(
"C   (0b)\0"                  "Db  (5b  also C# below)\0"
"D   (2#)\0"                  "Eb  (3b)\0"
"E   (4#)\0"                  "F   (1b)\0"
"Gb  (6b  also F# below)\0"   "G   (1#)\0"
"Ab  (4b)\0"                  "A   (3#)\0"
"Bb  (2b)\0"                  "B   (5#  also Cb below)\0"
"F#  (6#  also Gb above)\0"   "C#  (7#  also Db above)\0"
"Cb  (7b  also B  above)\0"),
            *Mi = CC(
"A   (0b)\0"                  "Bb  (5b  also A# below)\0"
"B   (2#)\0"                  "C   (3b)\0"
"C#  (4#)\0"                  "D   (1b)\0"
"Eb  (6b  also D# below)\0"   "E   (1#)\0"
"F   (4b)\0"                  "F#  (3#)\0"
"G   (2b)\0"                  "G#  (5#  also Ab below)\0"
"D#  (6#  also Eb above)\0"   "A#  (7#  also Bb above)\0"
"Ab  (7b  also G# above)\0");

void DlgKSg::Open ()
{  show ();   raise ();   activateWindow ();
   StrCp (_s, Up.pos.etc);
  TStr s;
   StrCp (s, Up.pos.str);
  char *m = & s [StrLn (s)-1];
  CtlList maj (ui->maj, CC("Major\0Minor\0")),
          key (ui->key, *m == 'm' ? Mi : Ma);
   maj.Set (*m == 'm' ? 1 : 0);
   if (*m == 'm')  *m = '\0';
   StrAp (s, CC(" "));   key.SetS (s);
   Gui.DlgMv (this, Up.gp, "tR");
}

void DlgKSg::Shut ()
{ TStr s;
  CtlList key (ui->key), maj (ui->maj);
   key.GetS (s);
   if (s [1] == ' ')  s [1] = '\0';   else s [2] = '\0';
   if (maj.Get ())  StrAp (s, CC("m"));
   StrAp (_s, s);
   emit sgCmd (_s);
   done (true);   lower ();   hide ();
}

void DlgKSg::Init ()
{  Gui.DlgLoad (this, "DlgKSg");
   connect (ui->maj, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
     ubyt2   p =  ui->key->currentIndex ();
     CtlList key (ui->key, ui->maj->currentIndex () ? Mi : Ma);
      key.Set (p);
   });
}

void DlgKSg::Quit ()  {Gui.DlgSave (this, "DlgKSg");}
