#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      controller_(new s21::AlgoTradingController) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete ui;
  delete controller_;
}


void MainWindow::on_pushButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Открыть CSV файл с торговыми данными"),
        "",
        tr("CSV файлы (*.csv);;Все файлы (*)")
        );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл"));
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // Передаём данные в контроллер
        if (controller_->loadTradingDataFromCsv(content)) {
            int count = controller_->getDataCount();
            QMessageBox::information(this, tr("Успех"),
                                     tr("Данные успешно загружены!\nЗаписей: %1").arg(count));

            // Пример: вывести первую цену
            auto first = controller_->getTradeData().first();
            qDebug() << "First price:" << first.close
                     << "at" << first.timestamp.toString();
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при парсинге CSV"));
        }
    }
}

