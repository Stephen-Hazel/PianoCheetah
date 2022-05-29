// dlgMov.cpp - what ta do with our rect o notes

#include "pcheetah.h"

void DlgMov::Open ()
{  show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgMov::Shut ()  {done (true);   lower ();   hide ();}

void DlgMov::Init ()
{  Gui.DlgLoad (this, "DlgMov");
   connect (ui->h2r, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC(">"));   Shut ();});
   connect (ui->h2l, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("<"));   Shut ();});
   connect (ui->h2b, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("#"));   Shut ();});
   connect (ui->del, & QPushButton::pressed,
            this, [this]() {StrCp (Up.pos.str, CC("x"));   Shut ();});
}

void DlgMov::Quit ()  {Gui.DlgSave (this, "DlgMov");}
