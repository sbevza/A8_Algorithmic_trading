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
    ui->plotWidget->xAxis->setLabelColor(Qt::darkBlue);
    ui->plotWidget->yAxis->setLabelColor(Qt::darkBlue);
    ui->plotWidget->xAxis->setTickLabelColor(Qt::darkGray);
    ui->plotWidget->yAxis->setTickLabelColor(Qt::darkGray);

    // Фон
    ui->plotWidget->setBackground(QColor(245, 245, 245)); // светло-серый фон

    // Сетка — мягкие линии
    QPen majorGridPen(Qt::lightGray, 1, Qt::SolidLine);
    QPen minorGridPen(Qt::gray, 0.8, Qt::DotLine);

    ui->plotWidget->xAxis->grid()->setPen(majorGridPen);
    ui->plotWidget->yAxis->grid()->setPen(majorGridPen);
    ui->plotWidget->xAxis->grid()->setSubGridPen(minorGridPen);
    ui->plotWidget->yAxis->grid()->setSubGridPen(minorGridPen);
    ui->plotWidget->xAxis->grid()->setSubGridVisible(true);
    ui->plotWidget->yAxis->grid()->setSubGridVisible(true);

    // Легенда
    ui->plotWidget->legend->setVisible(true);
    ui->plotWidget->legend->setBrush(QColor(255, 255, 255, 220));
    ui->plotWidget->legend->setBorderPen(QPen(Qt::lightGray, 1));
    ui->plotWidget->legend->setFont(QFont("Arial", 9));

    // Диапазон по умолчанию: 2020–2025
    QDateTime start = QDateTime::fromString("2020-01-01", "yyyy-MM-dd");
    QDateTime end = QDateTime::fromString("2025-01-01", "yyyy-MM-dd");
    ui->plotWidget->xAxis->setRange(start.toSecsSinceEpoch(), end.toSecsSinceEpoch());
    ui->plotWidget->yAxis->setRange(0, 100); // временной диапазон цены

    // Ограничение масштабирования
    ui->plotWidget->xAxis->setRangeLower(start.toSecsSinceEpoch());

    // Настройка шрифтов
    QFont axisFont = font();
    axisFont.setPointSize(9);
    ui->plotWidget->xAxis->setTickLabelFont(axisFont);
    ui->plotWidget->yAxis->setTickLabelFont(axisFont);

    // Количество точек
    ui->numPoints->setMaximum(10000);
    ui->numPoints->setValue(200); // разумное значение по умолчанию

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
    ui->GetValue->setEnabled(hasData);
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
        setWindowTitle(windowTitle().section(" — ", 0, 0) + " — Ошибка загрузки данных, попробуйте снова");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    if (controller_->loadTradingDataFromCsv(content)) {
        int count = controller_->getDataCount();
        if (count == 0) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Файл загружен, но данные не найдены"));
            setWindowTitle(windowTitle().section(" — ", 0, 0) + " — Ошибка загрузки данных, попробуйте снова");
            return;
        }

        QFileInfo fileInfo(fileName);
        QString shortFileName = fileInfo.fileName();
        QString baseTitle = windowTitle().section(" — ", 0, 0); // Исходное название до " — "
        setWindowTitle(QString("%1 — Данные из файла '%2' успешно загружены, точек: %3")
                           .arg(baseTitle, shortFileName).arg(count));

        ui->numPoints->setMinimum(count);
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при парсинге CSV-файла"));
        setWindowTitle(windowTitle().section(" — ", 0, 0) + " — Ошибка загрузки данных, попробуйте снова");
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
    int numPoints = ui->numPoints->value();

    QVector<double> xInterp, yInterp;
    double step = (xEnd - xStart) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double x = xStart + i * step;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        double y = controller_->getInterpolatedValue(dt);
        xInterp.append(x);
        yInterp.append(y);
    }

    // ✅ Правильно: QString вместо const char*
    QString label = createGraphLabel("spline", ui->numPoints->value());
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
    int numPoints = ui->numPoints->value();

    QVector<double> xPoly, yPoly;
    double step = (xEnd - xStart) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double x = xStart + i * step;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        double y = controller_->getInterpolatedValue(dt); // ← замени на метод Ньютона
        xPoly.append(x);
        yPoly.append(y);
    }

    QString label = createGraphLabel("newton", ui->degreeSpinBox->value());
    plotInterpolatedGraph(xPoly, yPoly, label);
}

// === УНИВЕРСАЛЬНЫЕ МЕТОДЫ ===

QString MainWindow::createGraphLabel(const QString& type, int value)
{
    // Извлекаем имя файла
    QString windowTitle = this->windowTitle();
    QString fileName = "data.csv";
    int start = windowTitle.indexOf("'");
    int end = windowTitle.indexOf("'", start + 1);
    if (start != -1 && end != -1) {
        fileName = windowTitle.mid(start + 1, end - start - 1);
    }

    QString prefix;
    if (type == "spline") {
        prefix = QString("Spline (%1)").arg(value);  // например: Spline (1000)
    } else if (type == "newton") {
        prefix = QString("Newton (n=%1)").arg(value);  // например: Newton (deg=3)
    } else {
        prefix = "Plot";
    }

    // Цветовой символ ●
    const QVector<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::darkCyan};

    QChar bullet(0x25CF);
    return QString("%1 %2 — %3").arg(bullet).arg(prefix).arg(fileName);
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
