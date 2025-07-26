#ifndef A1_MAZE_1_SRC_MAZE_MAINWINDOW_H_
#define A1_MAZE_1_SRC_MAZE_MAINWINDOW_H_

#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QVariantAnimation>
#include <QGraphicsRectItem>
#include <QMessageBox>

#include "../controllers/algo_trading_controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

 private slots:

 private:
  Ui::MainWindow *ui;
  s21::MazeController *controller_;

};

#endif  // A1_MAZE_1_SRC_MAZE_MAINWINDOW_H_
