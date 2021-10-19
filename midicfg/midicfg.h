// midicfg.h - midi configuration fer rockin88

#ifndef MIDICFG_H
#define MIDICFG_H

#include "../../stv/ui.h"
#include "../../stv/midi.h"
#include "ui_midicfg.h"

extern BStr DevTyp;

QT_BEGIN_NAMESPACE
namespace Ui { class MidiCfg; }
QT_END_NAMESPACE

class MidiCfg: public QMainWindow {
   Q_OBJECT
private:
   Ui::MidiCfg *ui;
   MidiI *_mi [16];
   ubyte _nMI;
   char   _io;
   CtlTabl _ti, _to;
   
   void Save (), Mv (char du);

protected:
   bool eventFilter (QObject *ob, QEvent *ev);

public:
   MidiCfg (QWidget *par = nullptr)
   : QMainWindow (par), ui (new Ui::MidiCfg)  {ui->setupUi (this);}

  ~MidiCfg ()   {delete ui;}

   void Init (), Quit ();
   void ShutMIn (), RedoMIn ();
   void TestI (ubyte mi, MidiEv e);

signals:
   void Reload ();

public slots:
   void Load  ();
   void TestO ();
   void MidiIEv ();
   void Up    ();
   void Dn    ();
   void Updt  ();
};

#endif // MIDICFG_H
