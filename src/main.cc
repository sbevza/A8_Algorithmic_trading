int main(int argc, char* argv[]) {
  // test test test
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/icon/icon.png"));
  MainWindow w;

  w.show();
  return a.exec();
  // Надо всё удалить, это только для теста
}
