// pcheetah.h - PianoCheetah - Steve's weird midi sequencer

#ifndef PCHEETAH_H
#define PCHEETAH_H

#include "ui_pcheetah.h"
#include "ui_dlgcfg.h"
#include "ui_dlgfl.h"
#include "ctlNt.h"
#include "song.h"

extern TStr Kick;                      // kick up an app after me like MidiCfg

namespace Ui { class DlgFL; }

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
   void Brow ();
   void Up   ();
   void Dn   ();
   
private:
   Ui::DlgFL *ui;
   CtlTabl   _t;
};


//______________________________________________________________________________
namespace Ui { class DlgCfg; }

class DlgCfg: public QDialog {
   Q_OBJECT
   
public:
   explicit DlgCfg (QWidget *parent = nullptr)
   : QDialog (parent)
   {  ui = new Ui::DlgCfg;   ui->setupUi (this);  }   
  ~DlgCfg ()         {delete ui;}
    
   void Init (), Quit (), Open (), Shut ();
   void closeEvent (QCloseEvent *e)  {(void)e;   Shut ();}
   
private:
   Ui::DlgCfg *ui;
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
   QThread      _thr;

   void SongRand (), SongKill (), SongRate ();
   
protected:
   void keyPressEvent (QKeyEvent *e);
   
public slots:
   void Load ();
   void LoadGo ();
   void MCfg ();
   void GCfg ();

   void SongPrv ();
   void SongNxt ();

   void Trak ();

// tblTr stuph - also TrPop() global func
   void TrClk  (); 
   void TrClkR (const QPoint &pos);
   void TrUpd  ();

   void Upd (QString upd);             // page, trk(msg_seth), etc

signals:
   void sgCmd (QString cmd);           // shhh, load, draw, cmds, etc
};


#endif // PCHEETAH_H
