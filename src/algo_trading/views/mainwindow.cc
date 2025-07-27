#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "qcustomplot/qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    controller_(new s21::AlgoTradingController) {
    ui->setupUi(this);

    // === Настройка QCustomPlot ===
    ui->plotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->plotWidget->axisRect()->setupFullAxesBox();

    // Настройка тикера по умолчанию для X (время)
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("dd.MM.yyyy");
    dateTicker->setDateTimeSpec(Qt::UTC);
    ui->plotWidget->xAxis->setTicker(dateTicker);

    // Подписи осей (можно тоже задать по умолчанию)
    ui->plotWidget->xAxis->setLabel("Дата");
    ui->plotWidget->yAxis->setLabel("Цена");

    // Фон (опционально)
    ui->plotWidget->setBackground(Qt::white);

    // Дополнительно: сетка
    ui->plotWidget->xAxis->grid()->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    ui->plotWidget->yAxis->grid()->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    ui->plotWidget->xAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->plotWidget->yAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->plotWidget->xAxis->grid()->setSubGridVisible(true);
    ui->plotWidget->yAxis->grid()->setSubGridVisible(true);

    // Автомасштаб при первом открытии будет вызван в on_pushButton_clicked
}

MainWindow::~MainWindow() {
  delete ui;
  delete controller_;
}


void MainWindow::on_pushButton_clicked() {
    // Задаём папку по умолчанию: ../materials относительно рабочей директории
    QString defaultDir = "../materials";
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Открыть CSV файл с торговыми данными"),
        defaultDir,
        tr("CSV файлы (*.csv);;Все файлы (*)")
        );

    if (fileName.isEmpty()) {
        return; // Пользователь отменил выбор
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл:\n%1").arg(fileName));
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Передаём данные в контроллер
    if (controller_->loadTradingDataFromCsv(content)) {
        int count = controller_->getDataCount();
        if (count == 0) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Файл загружен, но данные не найдены"));
            return;
        }

        QMessageBox::information(this, tr("Успех"),
                                 tr("Данные успешно загружены!\nЗаписей: %1").arg(count));

        // Пример: вывести первую цену
        auto first = controller_->getTradeData().first();
        qDebug() << "First price:" << first.close << "at" << first.timestamp.toString();

        // --- Отрисовка графика ---
        ui->plotWidget->clearGraphs(); // Удаляем старые графики

        auto data = controller_->getTradeData();
        QVector<double> xData, yData;
        xData.reserve(data.size());
        yData.reserve(data.size());

        for (const auto &point : data) {
            xData.append(point.timestamp.toMSecsSinceEpoch()); // в миллисекундах для QCustomPlot
            yData.append(point.close);
        }

        // Добавляем график
        QCPGraph *graph = ui->plotWidget->addGraph();
        graph->setData(xData, yData);
        graph->setLineStyle(QCPGraph::lsNone); // только точки
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::blue, 5));

        // Масштабируем и перерисовываем
        ui->plotWidget->rescaleAxes();
        ui->plotWidget->replot();

        // --- Настройка spinBox ---
        ui->spinBox->setMinimum(count);   // минимум — число строк
        ui->spinBox->setValue(count);     // устанавливаем текущее значение
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при парсинге CSV-файла"));
    }
}


void MainWindow::on_clean_button_clicked()
{
    while (ui->plotWidget->graphCount() > 1) {
        ui->plotWidget->removeGraph(1); // Удаляем все, кроме нулевого (исходные точки)
    }

    // Если вы хотите очистить всё (включая исходные точки) — используйте:
    ui->plotWidget->clearGraphs();
    ui->plotWidget->clearItems();

    // Перерисовываем
    ui->plotWidget->replot();

}


