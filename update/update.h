// update.h - updater fer pcheetah
#pragma once

#include "../../stv/ui.h"
#include "./ui_update.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Update; }
QT_END_NAMESPACE

class Update: public QMainWindow {
   Q_OBJECT
private:
   Ui::Update *ui;

public:
   Update (QWidget *par = nullptr)
   : QMainWindow (par), ui (new Ui::Update)  {ui->setupUi (this);}

  ~Update ()   {delete ui;}

   void Init (), Quit ();
};
