#include <QApplication>

#include "views/mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/icon/icon.png"));
  MainWindow w;

  w.show();
  return a.exec();

}
