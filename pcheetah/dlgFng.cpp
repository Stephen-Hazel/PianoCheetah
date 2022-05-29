// dlgFng.cpp - fingering picker

#include "pcheetah.h"

void DlgFng::Set (ubyte f)
{  emit sgCmd (StrFmt (Up.pos.str, "`s `d", _s, f));   Shut ();  }

void DlgFng::Open ()
{  StrFmt (_s, "fng `d `d", Up.pos.tr, Up.pos.p);
  CtlLabl l (ui->info);
   l.Set (Up.pos.str);
   show ();   raise ();   activateWindow ();
   Gui.DlgMv (this, Up.gp, "Tc");
}

void DlgFng::Shut ()  {done (true);   lower ();   hide ();}

void DlgFng::Init ()
{  Gui.DlgLoad (this, "DlgFng");
  CtlTBar tb (this,
      "swap note to other hand's track" "`:/tbar/fng/0" "`\0"
      "delete this note"                "`:/tbar/fng/1" "`\0"
      "delete =ALL= fingering in the song (BE CAREFUL)"
                                        "`:/tbar/fng/2" "`\0"
      "left hand (drums)"           "`:/tbar/fng/l0" "`\0"
      "right hand (drums)"          "`:/tbar/fng/l1" "`\0"
      "left foot (drums/organ)"     "`:/tbar/fng/l2" "`\0"
      "right foot (drums/organ)"    "`:/tbar/fng/l3" "`\0"
      "accent"                      "`:/tbar/fng/l4" "`\0"
      "staccatissimo (or WHAtever)" "`:/tbar/fng/l5" "`\0"
   );
   connect (tb.Act (0), & QAction::triggered,  this, [this]()  {Set (97);});
   connect (tb.Act (1), & QAction::triggered,  this, [this]()  {Set (98);});
   connect (tb.Act (2), & QAction::triggered,  this, [this]()  {Set (99);});

   connect (tb.Act (3), & QAction::triggered,  this, [this]()  {Set (26);});
   connect (tb.Act (4), & QAction::triggered,  this, [this]()  {Set (27);});
   connect (tb.Act (5), & QAction::triggered,  this, [this]()  {Set (28);});
   connect (tb.Act (6), & QAction::triggered,  this, [this]()  {Set (29);});
   connect (tb.Act (7), & QAction::triggered,  this, [this]()  {Set (30);});
   connect (tb.Act (8), & QAction::triggered,  this, [this]()  {Set (31);});

   connect (ui->fOff, & QPushButton::clicked,  this, [this]()  {Set (0);});

   connect (ui->f1,   & QPushButton::clicked,  this, [this]()  {Set (1);});
   connect (ui->f2,   & QPushButton::clicked,  this, [this]()  {Set (2);});
   connect (ui->f3,   & QPushButton::clicked,  this, [this]()  {Set (3);});
   connect (ui->f4,   & QPushButton::clicked,  this, [this]()  {Set (4);});
   connect (ui->f5,   & QPushButton::clicked,  this, [this]()  {Set (5);});

   connect (ui->f12,  & QPushButton::clicked,  this, [this]()  {Set (6);});
   connect (ui->f13,  & QPushButton::clicked,  this, [this]()  {Set (7);});
   connect (ui->f14,  & QPushButton::clicked,  this, [this]()  {Set (8);});
   connect (ui->f15,  & QPushButton::clicked,  this, [this]()  {Set (9);});

   connect (ui->f21,  & QPushButton::clicked,  this, [this]()  {Set (10);});
   connect (ui->f23,  & QPushButton::clicked,  this, [this]()  {Set (11);});
   connect (ui->f24,  & QPushButton::clicked,  this, [this]()  {Set (12);});
   connect (ui->f25,  & QPushButton::clicked,  this, [this]()  {Set (13);});

   connect (ui->f31,  & QPushButton::clicked,  this, [this]()  {Set (14);});
   connect (ui->f32,  & QPushButton::clicked,  this, [this]()  {Set (15);});
   connect (ui->f34,  & QPushButton::clicked,  this, [this]()  {Set (16);});
   connect (ui->f35,  & QPushButton::clicked,  this, [this]()  {Set (17);});

   connect (ui->f41,  & QPushButton::clicked,  this, [this]()  {Set (18);});
   connect (ui->f42,  & QPushButton::clicked,  this, [this]()  {Set (19);});
   connect (ui->f43,  & QPushButton::clicked,  this, [this]()  {Set (20);});
   connect (ui->f45,  & QPushButton::clicked,  this, [this]()  {Set (21);});

   connect (ui->f51,  & QPushButton::clicked,  this, [this]()  {Set (22);});
   connect (ui->f52,  & QPushButton::clicked,  this, [this]()  {Set (23);});
   connect (ui->f53,  & QPushButton::clicked,  this, [this]()  {Set (24);});
   connect (ui->f54,  & QPushButton::clicked,  this, [this]()  {Set (25);});
}

void DlgFng::Quit ()  {Gui.DlgSave (this, "DlgFng");}
