#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "qcustomplot/qcustomplot.h"

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

            // --- НАЧАЛО: Отрисовка графика ---
            ui->plotWidget->clearGraphs();
            ui->plotWidget->clearItems();

            auto data = controller_->getTradeData();
            QVector<double> xData, yData;

            for (const auto &point : data) {
                xData.append(point.timestamp.toMSecsSinceEpoch()); // Миллисекунды!
                yData.append(point.close);
            }

            QCPGraph *dataPoints = ui->plotWidget->addGraph();
            dataPoints->setData(xData, yData);
            dataPoints->setLineStyle(QCPGraph::lsNone);
            dataPoints->setScatterStyle(QCPScatterStyle(
                QCPScatterStyle::ssCircle,
                Qt::blue,
                Qt::blue,
                5
                ));

            QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
            dateTicker->setDateTimeFormat("dd.MM.yyyy");
            dateTicker->setDateTimeSpec(Qt::UTC);
            ui->plotWidget->xAxis->setTicker(dateTicker);


            ui->plotWidget->xAxis->setLabel("Дата");
            ui->plotWidget->yAxis->setLabel("Цена");

            ui->plotWidget->rescaleAxes();
            ui->plotWidget->replot();
            // --- КОНЕЦ: Отрисовка графика ---

        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при парсинге CSV"));
        }
    }
}

