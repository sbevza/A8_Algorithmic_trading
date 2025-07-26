#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      controller_(new s21::MazeController) {
  ui->setupUi(this);


  //устанавливаем первоначальную рамку в 2px
  auto *scene = new QGraphicsScene(this);
  scene->setSceneRect(0, 0, 498, 498);
  QPen pen(Qt::white, 2);
  scene->addRect(0, 0, 498, 498, pen);
  scene->setBackgroundBrush(Qt::black);
  ui->graphicsView->setScene(scene);

}

MainWindow::~MainWindow() {
  delete ui;
  delete controller_;
}






