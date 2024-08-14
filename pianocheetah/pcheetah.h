// pcheetah.h - PianoCheetah - Steve's weird midi sequencer

#ifndef PCHEETAH_H
#define PCHEETAH_H

#include "ui_pcheetah.h"
#include "ui_dlgcfg.h"
#include "ui_dlgfl.h"
#include "ui_dlgtdr.h"
#include "ui_dlgcue.h"
#include "ui_dlgfng.h"
#include "ui_dlgchd.h"
#include "ui_dlgctl.h"
#include "ui_dlgtpo.h"
#include "ui_dlgtsg.h"
#include "ui_dlgksg.h"
#include "ui_dlgmov.h"
#include "ui_dlghlp.h"
#include "ctlNt.h"
#include "song.h"

extern TStr Kick;                      // kick up an app on exit - like MidiCfg

QT_BEGIN_NAMESPACE
namespace Ui { class DlgFL; }
QT_END_NAMESPACE

class DlgFL: public QDialog {
   Q_OBJECT

public:
   explicit DlgFL (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgFL;   ui->setupUi (this);  }
  ~DlgFL ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void ReDo ();
   void Pik  ();
   void Find ();
   void Up   ();
   void Dn   ();
   void Song2Wav ();
   void Sfz2Syn  ();
   void Mod2Song ();
   void MidImp   ();
   void Brow     ();

private:
   Ui::DlgFL *ui;
   CtlTabl   _t;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgCfg; }
QT_END_NAMESPACE

class DlgCfg: public QDialog {
   Q_OBJECT

public:
   explicit DlgCfg (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgCfg;   ui->setupUi (this);  }
  ~DlgCfg ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

signals:
   void sgCmd (char *s);

private:
   Ui::DlgCfg *ui;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgTDr; }
QT_END_NAMESPACE

class DlgTDr: public QDialog {
   Q_OBJECT

public:
   explicit DlgTDr (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgTDr;   ui->setupUi (this);  }
  ~DlgTDr ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void Cmd ();

signals:
   void sgCmd (char *s);

private:
   Ui::DlgTDr *ui;
   CtlTabl    _t;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgCue; }
QT_END_NAMESPACE

class DlgCue: public QDialog {
   Q_OBJECT

public:
   explicit DlgCue (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgCue;   ui->setupUi (this);  }
  ~DlgCue ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void Set (const char *s);

signals:
   void sgCmd (char *s);

private:
   Ui::DlgCue *ui;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgFng; }
QT_END_NAMESPACE

class DlgFng: public QDialog {
   Q_OBJECT

public:
   explicit DlgFng (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgFng;   ui->setupUi (this);  }
  ~DlgFng ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void Set (ubyte f);

signals:
   void sgCmd (char *s);

private:
   TStr _s;
   Ui::DlgFng *ui;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgChd; }
QT_END_NAMESPACE

class DlgChd: public QDialog {
   Q_OBJECT

public:
   explicit DlgChd (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgChd;   ui->setupUi (this);  }
  ~DlgChd ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void Cmd (char *s);

signals:
   void sgCmd (char *s);

private:
   ulong _cp, _tm, _tm1;
   bool  _got;
   void ReDo ();
   void Set  (char *s);
   void UnDo ();
   void Pop  ();
   Ui::DlgChd *ui;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgCtl; }
QT_END_NAMESPACE

class DlgCtl: public QDialog {
   Q_OBJECT

public:
   explicit DlgCtl (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgCtl;   ui->setupUi (this);  }
  ~DlgCtl ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

public slots:
   void Upd ();

private:
   Ui::DlgCtl *ui;
   CtlTabl    _t;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgTpo; }
QT_END_NAMESPACE

class DlgTpo: public QDialog {
   Q_OBJECT

public:
   explicit DlgTpo (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgTpo;   ui->setupUi (this);  }
  ~DlgTpo ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

signals:
   void sgCmd (char *s);

private:
   Ui::DlgTpo *ui;
   TStr       _s;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgTSg; }
QT_END_NAMESPACE

class DlgTSg: public QDialog {
   Q_OBJECT

public:
   explicit DlgTSg (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgTSg;   ui->setupUi (this);  }
  ~DlgTSg ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

signals:
   void sgCmd (char *s);

private:
   Ui::DlgTSg *ui;
   TStr       _s;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgKSg; }
QT_END_NAMESPACE

class DlgKSg: public QDialog {
   Q_OBJECT

public:
   explicit DlgKSg (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgKSg;   ui->setupUi (this);  }
  ~DlgKSg ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

signals:
   void sgCmd (char *s);

private:
   Ui::DlgKSg *ui;
   TStr       _s;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgMov; }
QT_END_NAMESPACE

class DlgMov: public QDialog {
   Q_OBJECT

public:
   explicit DlgMov (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgMov;   ui->setupUi (this);  }
  ~DlgMov ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

private:
   Ui::DlgMov *ui;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class DlgHlp; }
QT_END_NAMESPACE

class DlgHlp: public QDialog {
   Q_OBJECT

public:
   explicit DlgHlp (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgHlp;   ui->setupUi (this);  }
  ~DlgHlp ()         {delete ui;}

   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}

private:
   Ui::DlgHlp *ui;
   CtlTabl    _t;
};


//______________________________________________________________________________
QT_BEGIN_NAMESPACE
namespace Ui { class PCheetah; }
QT_END_NAMESPACE

class PCheetah: public QMainWindow {
   Q_OBJECT

public:
   PCheetah (QWidget *par = nullptr)
   : QMainWindow (par)
   {  ui = new Ui::PCheetah;   ui->setupUi (this);  }
  ~PCheetah ()         {delete ui;}

   void Init (), Quit ();

   Song *_s;

private:
   Ui::PCheetah *ui;
   CtlTabl      _tr;
   CtlNt       *_nt;
   DlgFL       *_dFL;
   DlgCfg      *_dCfg;
   DlgTDr      *_dTDr;
   DlgCue      *_dCue;
   DlgFng      *_dFng;
   DlgChd      *_dChd;
   DlgCtl      *_dCtl;
   DlgTpo      *_dTpo;
   DlgTSg      *_dTSg;
   DlgKSg      *_dKSg;
   DlgMov      *_dMov;
   DlgHlp      *_dHlp;
   QThread      _thrSong;

   void SongKill (), SongRate ();

protected:
   void keyPressEvent (QKeyEvent *e);

public slots:
   void Load   ();
   void LoadGo ();
   void MCfg   ();
   void GCfg   ();
   void TDr    ();

   void SongPrv  ();
   void SongNxt  ();
   void SongRand ();

   void Trak ();

// CtlTabl _tr stuph - also TrPop() global func
   void TrClk  ();
   void TrClkR (const QPoint &pos);
   void TrUpd  ();

   void Upd (QString upd);             // page, trk(msg_seth), etc

signals:
   void sgCmd (QString cmd);           // shhh, load, draw, cmds, etc
};


#endif // PCHEETAH_H
