// midicfg.h - midi configuration fer rockin88

#ifndef MIDICFG_H
#define MIDICFG_H

#include "../../stv/ui.h"
#include "../../stv/midi.h"
#include "ui_midicfg.h"

extern BStr DevTyp;

class CmboDel: public QStyledItemDelegate {
   Q_OBJECT
public:
   CmboDel (QObject *par = nullptr): QStyledItemDelegate (par)  {}
  ~CmboDel ()  {}

public slots:
   void cbChanged (int i)
   { QComboBox *cb = static_cast<QComboBox *>(sender ());
      (void)i;
      emit commitData (cb);   emit closeEditor (cb);
   }

public:
   QWidget *createEditor (QWidget *par, const QStyleOptionViewItem &opt,
                          const QModelIndex &ind)  const override
   {  if (ind.column () == 1) {
        char *s;
        QComboBox *cb = new QComboBox (par);
         for (s = DevTyp;  *s;  s = & s [StrLn (s)+1])  cb->addItem (s);
         return cb;
      }
      return QStyledItemDelegate::createEditor (par, opt, ind);
   }

   void setEditorData (QWidget *ed, const QModelIndex &ind)  const override
   { QComboBox *cb = qobject_cast<QComboBox *>(ed);
      if (cb) {
        int i = cb->findText (ind.data (Qt::EditRole).toString ());
         if (i < 0)  i = 0;
         cb->setCurrentIndex (i);   cb->showPopup ();
         connect (cb, SIGNAL(currentIndexChanged(int)),
                  this, SLOT(cbChanged(int)));
      }
      else
         QStyledItemDelegate::setEditorData (ed, ind);
   }

   void setModelData (QWidget *ed, QAbstractItemModel *mod,
                      const QModelIndex &ind)  const override
   { QComboBox *cb = qobject_cast<QComboBox *>(ed);
      if (cb) mod->setData (ind, cb->currentText (), Qt::EditRole);
      else
         QStyledItemDelegate::setModelData (ed, mod, ind);
   }
};


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

   void Save (), Mv (char du);

public:
   MidiCfg (QWidget *par = nullptr)
   : QMainWindow (par), ui (new Ui::MidiCfg)  {ui->setupUi (this);}

  ~MidiCfg ()   {delete ui;}

   void Init ();
   void ShutMIn (), RedoMIn ();
   void DumpEv  (ubyte mi, MidiEv e);

signals:
  void Reload ();

protected:
   bool eventFilter (QObject *ob, QEvent *ev);

public slots:
   void Load  ();
   void TestO ();
   void MidiIEv ();
   void Up    ();
   void Dn    ();
   void Updt  ();
};

#endif // MIDICFG_H
