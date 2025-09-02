#include "mainwindow.h"

#include "qcustomplot/qcustomplot.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      controller_(new s21::AlgoTradingController) {
  ui->setupUi(this);

  // === Настройка QCustomPlot ===
  ui->plotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                  QCP::iSelectPlottables);
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
  ui->plotWidget->setBackground(QColor(245, 245, 245));  // светло-серый фон

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
  ui->plotWidget->xAxis->setRange(start.toSecsSinceEpoch(),
                                  end.toSecsSinceEpoch());
  ui->plotWidget->yAxis->setRange(0, 100);  // временной диапазон цены

  // Ограничение масштабирования
  ui->plotWidget->xAxis->setRangeLower(start.toSecsSinceEpoch());

  // Настройка шрифтов
  QFont axisFont = font();
  axisFont.setPointSize(9);
  ui->plotWidget->xAxis->setTickLabelFont(axisFont);
  ui->plotWidget->yAxis->setTickLabelFont(axisFont);

  // Количество точек
  ui->numPoints->setMaximum(10000);
  ui->numPoints->setValue(200);

  updateUiState();
}

MainWindow::~MainWindow() {
  delete ui;
  delete controller_;
}

void MainWindow::updateUiState() {
  bool hasData = controller_->getDataCount() > 0;
  ui->PlotCubicSpline->setEnabled(hasData);
  ui->PlotNewtonPolynomial->setEnabled(hasData);
  ui->GetValue->setEnabled(hasData);
  ui->showPoints->setEnabled(hasData);
  ui->dateTimeEdit->setEnabled(hasData);
  ui->numPointsLabel->setEnabled(hasData);
  ui->degreeSpinBox->setEnabled(hasData);
  ui->numPoints->setEnabled(hasData);
  ui->ValueNewton->setEnabled(hasData);
  ui->ValueSpline->setEnabled(hasData);
  ui->clean_button->setEnabled(hasData);
  ui->degreeLabel->setEnabled(hasData);
}

void MainWindow::on_clean_button_clicked() {
  ui->plotWidget->clearGraphs();
  ui->plotWidget->clearItems();
  ui->plotWidget->replot();
}

void MainWindow::on_LoadDataCsv_clicked() {
  on_clean_button_clicked();

  QString defaultDir = "../materials";
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Открыть CSV файл с торговыми данными"), defaultDir,
      tr("CSV файлы (*.csv);;Все файлы (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  if (controller_->loadTradingDataFromCsv(fileName)) {
    int count = controller_->getDataCount();
    if (count == 0) {
      QMessageBox::warning(this, tr("Ошибка"),
                           tr("Файл загружен, но данные не найдены"));
    } else {
      QFileInfo fileInfo(fileName);
      QString shortFileName = fileInfo.fileName();
      QString baseTitle = windowTitle().section(" — ", 0, 0);
      setWindowTitle(
          QString("%1 — Данные из файла '%2' успешно загружены, точек: %3")
              .arg(baseTitle, shortFileName)
              .arg(count));

      ui->numPoints->setMinimum(count);
    }
  } else {
    QMessageBox::warning(this, tr("Ошибка"),
                         tr("Ошибка при загрузке или парсинге файла"));
    setWindowTitle(windowTitle().section(" — ", 0, 0) +
                   " — Ошибка загрузки данных");
  }

  updateUiState();
  setupDateTimeEditLimits();
}

void MainWindow::on_PlotCubicSpline_clicked() {
  plotInterpolatedFunction(
      "spline", 0,
      [this](const QDateTime& dt) {
        return controller_->getInterpolatedValue(dt);
      },
      ui->numPoints->value());
}

void MainWindow::on_PlotNewtonPolynomial_clicked() {
  int degree = ui->degreeSpinBox->value();
  auto data = controller_->getTradeData();

  if (degree > 10) {
    QString warningMsg =
        tr("Степень полинома %1 — это слишком много.\n\n"
           "Полиномы степени выше 10:\n"
           "• Сильно колеблются (явление Рунге)\n"
           "• Дают неадекватные значения между точками\n"
           "• Численно нестабильны\n\n"
           "Построение не выполняется — результат будет фигня.\n"
           "Снизь степень, братиш, и всё будет ок.")
            .arg(degree);

    QMessageBox::information(this, tr("Не построено"), warningMsg);
    return;
  }

  if (data.size() < degree + 1) {
    QMessageBox::warning(this, "Ошибка",
                         QString("Для степени %1 нужно %2 точек, доступно: %3")
                             .arg(degree)
                             .arg(degree + 1)
                             .arg(data.size()));
    return;
  }

  plotInterpolatedFunction(
      "newton", degree,
      [this, degree](const QDateTime& dt) {
        return controller_->getInterpolatedValueNewton(dt, degree);
      },
      ui->numPoints->value());
}

QString MainWindow::createGraphLabel(const QString& type, int degree,
                                     int pointCount) {
  // Извлекаем имя файла из заголовка: 'имя.csv'
  QString fileName = "unknown.csv";
  QString title = this->windowTitle();
  int start = title.indexOf("'");
  int end = title.indexOf("'", start + 1);
  if (start != -1 && end != -1 && end > start) {
    fileName = title.mid(start + 1, end - start - 1);
  }

  // Формируем префикс по типу
  QString prefix;
  if (type == "spline") {
    prefix = QString("Spline (%1)").arg(pointCount);
  } else if (type == "newton") {
    prefix = QString("Newton (%1) n=%2").arg(pointCount).arg(degree);
  } else {
    prefix = QString("%1 (%2)").arg(type.toUpper()).arg(pointCount);
  }

  // Маркер ● и финальный вид
  QChar bullet(0x25CF);  // ●
  return QString("%1 %2 — %3").arg(bullet).arg(prefix).arg(fileName);
}

void MainWindow::setupDateTimeEditLimits() {
  auto data = controller_->getTradeData();
  if (data.isEmpty()) return;

  QDateTime minDate = QDateTime::fromSecsSinceEpoch(
      static_cast<qint64>(data.first().timestamp));
  QDateTime maxDate =
      QDateTime::fromSecsSinceEpoch(static_cast<qint64>(data.last().timestamp));

  ui->dateTimeEdit->setDateTimeRange(minDate, maxDate);
  ui->dateTimeEdit->setDateTime(minDate);
}

void MainWindow::on_GetValue_clicked() {
  QDateTime dateTime = ui->dateTimeEdit->dateTime();
  int degree = ui->degreeSpinBox->value();

  // === Сплайн ===
  double splineValue = controller_->getInterpolatedValue(dateTime);
  QString splineText =
      std::isnan(splineValue) ? "—" : QString::number(splineValue, 'f', 6);
  ui->ValueSpline->setText("Значение сплайна: " + splineText);

  // === Полином Ньютона ===
  double newValue = controller_->getInterpolatedValueNewton(dateTime, degree);
  QString newtonText =
      std::isnan(newValue) ? "—" : QString::number(newValue, 'f', 6);
  ui->ValueNewton->setText("Значение полинома: " + newtonText);
}

void MainWindow::on_showPoints_clicked() {
  showPoints_ = !showPoints_;

  for (int i = 0; i < ui->plotWidget->graphCount(); ++i) {
    QCPGraph* g = ui->plotWidget->graph(i);
    QColor color = g->pen().color();
    g->setScatterStyle(showPoints_ ? QCPScatterStyle(QCPScatterStyle::ssCircle,
                                                     color, color, 6)
                                   : QCPScatterStyle::ssNone);
  }

  ui->plotWidget->replot();
}

void MainWindow::plotInterpolatedFunction(
    const QString& type, int degree,
    std::function<double(const QDateTime&)> valueFunc, int pointCount) {
  if (controller_->getDataCount() == 0) {
    QMessageBox::warning(this, "Ошибка", "Нет данных. Загрузите CSV.");
    return;
  }

  auto data = controller_->getTradeData();
  if (data.size() < 2) {
    QMessageBox::warning(this, "Ошибка", "Минимум 2 точки.");
    return;
  }

  double xStart = data.first().timestamp;
  double xEnd = data.last().timestamp;

  QVector<double> x = generateX(xStart, xEnd, pointCount);
  QVector<double> y;
  y.reserve(x.size());

  for (double xi : x) {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(xi));
    y.append(valueFunc(dt));
  }

  QString label = createGraphLabel(type, degree, pointCount);
  plotInterpolatedGraph(x, y, label);
}

void MainWindow::plotInterpolatedGraph(const QVector<double>& x,
                                       const QVector<double>& y,
                                       const QString& label) {
  if (ui->plotWidget->graphCount() >= 5) {
    QMessageBox::warning(this, "Лимит", "Не более 5 графиков.");
    return;
  }

  QCPGraph* graph = ui->plotWidget->addGraph();  // ← теперь это QCPGraph*
  graph->setData(x, y);
  graph->setName(label);

  static QVector<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta,
                                   Qt::darkCyan};
  int index = ui->plotWidget->graphCount();
  QColor color = colors[index % colors.size()];
  graph->setPen(QPen(color, 2));

  // Применяем стиль: точки или нет
  graph->setScatterStyle(
      showPoints_ ? QCPScatterStyle(QCPScatterStyle::ssCircle, color, color, 6)
                  : QCPScatterStyle::ssNone);

  ui->plotWidget->rescaleAxes();
  ui->plotWidget->replot();
}

QVector<double> MainWindow::generateX(double xStart, double xEnd,
                                      int numPoints) {
  QVector<double> x;
  x.reserve(numPoints);
  double step = (xEnd - xStart) / (numPoints - 1);
  for (int i = 0; i < numPoints; ++i) {
    x.append(xStart + i * step);
  }
  return x;
}
