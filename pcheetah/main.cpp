#include "pcheetah.h"

#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   PCheetah w;
   w.show();
   return a.exec();
}
