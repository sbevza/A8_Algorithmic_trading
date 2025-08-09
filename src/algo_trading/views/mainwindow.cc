#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot/qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    controller_(new s21::AlgoTradingController)
{
    ui->setupUi(this);

    // === Настройка QCustomPlot ===
    ui->plotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->plotWidget->axisRect()->setupFullAxesBox();

    // Тикер времени
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("dd.MM.yyyy");
    dateTicker->setDateTimeSpec(Qt::UTC);
    ui->plotWidget->xAxis->setTicker(dateTicker);

    // Оси
    ui->plotWidget->xAxis->setLabel("Дата");
    ui->plotWidget->yAxis->setLabel("Цена");

    // Фон
    ui->plotWidget->setBackground(Qt::white);

    // Сетка
    ui->plotWidget->xAxis->grid()->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    ui->plotWidget->yAxis->grid()->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    ui->plotWidget->xAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->plotWidget->yAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->plotWidget->xAxis->grid()->setSubGridVisible(true);
    ui->plotWidget->yAxis->grid()->setSubGridVisible(true);

    ui->spinBox->setMaximum(10000);

    updateUiState();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete controller_;
}

void MainWindow::updateUiState()
{
    bool hasData = controller_->getDataCount() > 0;
    ui->PlotCubicSpline->setEnabled(hasData);
    ui->PlotNewtonPolynomial->setEnabled(hasData);
}

void MainWindow::on_clean_button_clicked()
{
    ui->plotWidget->clearGraphs();
    ui->plotWidget->clearItems();
    ui->plotWidget->replot();
    updateUiState();
}

void MainWindow::on_LoadDataCsv_clicked()
{
    QString defaultDir = "../materials";
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Открыть CSV файл с торговыми данными"),
        defaultDir,
        tr("CSV файлы (*.csv);;Все файлы (*)")
        );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл:\n%1").arg(fileName));
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    if (controller_->loadTradingDataFromCsv(content)) {
        int count = controller_->getDataCount();
        if (count == 0) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Файл загружен, но данные не найдены"));
            return;
        }

        QMessageBox::information(this, tr("Успех"),
                                 tr("Данные успешно загружены!\nЗаписей: %1").arg(count));

        QFileInfo fileInfo(fileName);
        QString shortFileName = fileInfo.fileName();
        this->setWindowTitle(this->windowTitle() + " - " + shortFileName);

        // Отрисовка исходных точек
        ui->plotWidget->clearGraphs();
        auto data = controller_->getTradeData();
        QVector<double> xData, yData;
        xData.reserve(data.size());
        yData.reserve(data.size());

        for (const auto &point : data) {
            xData.append(point.timestamp.toSecsSinceEpoch());
            yData.append(point.close);
        }

        QCPGraph *graph = ui->plotWidget->addGraph();
        graph->setData(xData, yData);
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::blue, 5));

        ui->plotWidget->rescaleAxes();
        ui->plotWidget->replot();

        ui->spinBox->setMinimum(count);

    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при парсинге CSV-файла"));
    }

    updateUiState();
}

void MainWindow::on_PlotCubicSpline_clicked()
{
    if (controller_->getDataCount() == 0) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Нет данных для построения сплайна. Сначала загрузите CSV."));
        return;
    }

    if (ui->plotWidget->graphCount() >= 5) {
        QMessageBox::warning(this, tr("Лимит"), tr("Можно отображать не более 5 графиков."));
        return;
    }

    controller_->buildSplineFromLoadedData();
    auto data = controller_->getTradeData();
    if (data.size() < 2) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Недостаточно данных для построения сплайна (минимум 2 точки)."));
        return;
    }

    double xStart = data.first().timestamp.toSecsSinceEpoch();
    double xEnd = data.last().timestamp.toSecsSinceEpoch();
    int numPoints = ui->spinBox->value();

    QVector<double> xInterp, yInterp;
    double step = (xEnd - xStart) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double x = xStart + i * step;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        double y = controller_->getInterpolatedValue(dt);
        xInterp.append(x);
        yInterp.append(y);
    }

    QString label = createGraphLabel();
    plotInterpolatedGraph(xInterp, yInterp, label);
}

void MainWindow::on_PlotNewtonPolynomial_clicked()
{
    if (controller_->getDataCount() == 0) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Нет данных для построения полинома. Сначала загрузите CSV."));
        return;
    }

    if (ui->plotWidget->graphCount() >= 5) {
        QMessageBox::warning(this, tr("Лимит"), tr("Можно отображать не более 5 графиков."));
        return;
    }

    // Здесь будет логика полинома Ньютона
    auto data = controller_->getTradeData();
    if (data.size() < 2) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Недостаточно данных для построения полинома (минимум 2 точки)."));
        return;
    }

    double xStart = data.first().timestamp.toSecsSinceEpoch();
    double xEnd = data.last().timestamp.toSecsSinceEpoch();
    int numPoints = ui->spinBox->value();

    QVector<double> xPoly, yPoly;
    double step = (xEnd - xStart) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double x = xStart + i * step;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        double y = controller_->getInterpolatedValue(dt); // ← замени на метод Ньютона
        xPoly.append(x);
        yPoly.append(y);
    }

    QString label = createGraphLabel();
    plotInterpolatedGraph(xPoly, yPoly, label);
}

// === УНИВЕРСАЛЬНЫЕ МЕТОДЫ ===

QString MainWindow::createGraphLabel()
{
    QString fileName = "unknown.csv";
    QString windowTitle = this->windowTitle();
    if (windowTitle.contains(" - ")) {
        fileName = windowTitle.split(" - ").last();
    }
    int graphNumber = ui->plotWidget->graphCount() + 1;
    return QString("График %1 (%2)").arg(graphNumber).arg(fileName);
}

void MainWindow::plotInterpolatedGraph(const QVector<double>& xData, const QVector<double>& yData, const QString& label)
{
    const QVector<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::darkCyan};
    QColor color = colors[ui->plotWidget->graphCount()];

    QCPGraph* graph = ui->plotWidget->addGraph();
    graph->setData(xData, yData);
    graph->setPen(QPen(color, 2));
    graph->setName(label);

    ui->plotWidget->legend->setVisible(true);
    ui->plotWidget->rescaleAxes();
    ui->plotWidget->replot();
}
