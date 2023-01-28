// initme.h - updater fer pcheetah
#pragma once

#include "../../stv/ui.h"
#include "./ui_initme.h"

QT_BEGIN_NAMESPACE
namespace Ui { class InitMe; }
QT_END_NAMESPACE

class InitMe: public QMainWindow {
   Q_OBJECT
private:
   Ui::InitMe *ui;

public:
   InitMe (QWidget *par = nullptr)
   : QMainWindow (par), ui (new Ui::InitMe)  {ui->setupUi (this);}

  ~InitMe ()   {delete ui;}

   void Init (), Quit ();
};
