// src/algo_trading/views/mainwindow.cc

#include "mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QPen>
#include <QSharedPointer>
#include <functional>

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
  const QSharedPointer<QCPAxisTickerDateTime> dateTicker(
      new QCPAxisTickerDateTime);
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
  const QPen majorGridPen(Qt::lightGray, 1, Qt::SolidLine);
  const QPen minorGridPen(Qt::gray, 0.8, Qt::DotLine);

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
  const QDateTime start = QDateTime::fromString("2020-01-01", "yyyy-MM-dd");
  const QDateTime end = QDateTime::fromString("2025-01-01", "yyyy-MM-dd");

  const auto start_sec = static_cast<double>(start.toSecsSinceEpoch());
  const auto end_sec = static_cast<double>(end.toSecsSinceEpoch());

  ui->plotWidget->xAxis->setRange(start_sec, end_sec);
  ui->plotWidget->yAxis->setRange(0, 100);

  // Ограничение масштабирования
  ui->plotWidget->xAxis->setRangeLower(start_sec);

  // Настройка шрифтов
  QFont axisFont = font();
  axisFont.setPointSize(9);
  ui->plotWidget->xAxis->setTickLabelFont(axisFont);
  ui->plotWidget->yAxis->setTickLabelFont(axisFont);

  // Количество точек
  ui->spin_interpolation_points->setMaximum(10000);
  ui->spin_interpolation_points->setValue(200);

  connect(ui->btn_load_data_csv, &QPushButton::clicked, this,
          &MainWindow::onLoadDataCsvClicked);

  connect(ui->btn_clear_interpolation, &QPushButton::clicked, this,
          &MainWindow::onClearInterpolationClicked);

  connect(ui->btn_plot_cubic_spline, &QPushButton::clicked, this,
          &MainWindow::onPlotCubicSplineClicked);

  connect(ui->btn_plot_newton_polynomial, &QPushButton::clicked, this,
          &MainWindow::onPlotNewtonPolynomialClicked);

  connect(ui->btn_get_interpolated_value, &QPushButton::clicked, this,
          &MainWindow::onGetInterpolatedValueClicked);

  connect(ui->btn_show_data_points, &QPushButton::clicked, this,
          &MainWindow::onShowDataPointsToggled);

  updateUiState();
}

MainWindow::~MainWindow() {
  delete ui;
  delete controller_;
}

void MainWindow::updateUiState() const {
  const bool hasData = controller_->getDataCount() > 0;
  ui->btn_plot_cubic_spline->setEnabled(hasData);
  ui->btn_plot_newton_polynomial->setEnabled(hasData);
  ui->btn_get_interpolated_value->setEnabled(hasData);
  ui->btn_show_data_points->setEnabled(hasData);
  ui->dt_interpolation_input->setEnabled(hasData);
  ui->lbl_interpolation_points_label->setEnabled(hasData);
  ui->spin_newton_degree->setEnabled(hasData);
  ui->spin_interpolation_points->setEnabled(hasData);
  ui->lbl_newton_value->setEnabled(hasData);
  ui->lbl_spline_value->setEnabled(hasData);
  ui->btn_clear_interpolation->setEnabled(hasData);
  ui->lbl_newton_degree_label->setEnabled(hasData);
}

void MainWindow::onClearInterpolationClicked() {  // NOLINT
  ui->plotWidget->clearGraphs();
  ui->plotWidget->clearItems();
  ui->plotWidget->replot();
}

void MainWindow::onLoadDataCsvClicked() {
  onClearInterpolationClicked();

  const QString defaultDir = "../materials";
  const QString fileName = QFileDialog::getOpenFileName(
      this, tr("Открыть CSV файл с торговыми данными"), defaultDir,
      tr("CSV файлы (*.csv);;Все файлы (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  if (controller_->loadTradingDataFromCsv(fileName)) {
    if (const size_t count = controller_->getDataCount(); count == 0) {
      QMessageBox::warning(this, tr("Ошибка"),
                           tr("Файл загружен, но данные не найдены"));
    } else {
      const QFileInfo fileInfo(fileName);
      QString shortFileName = fileInfo.fileName();
      QString baseTitle = windowTitle().section(" — ", 0, 0);
      setWindowTitle(
          QString("%1 — Данные из файла '%2' успешно загружены, точек: %3")
              .arg(baseTitle, shortFileName)
              .arg(count));

      ui->spin_interpolation_points->setMinimum(static_cast<int>(count));
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

void MainWindow::onPlotCubicSplineClicked() {
  plotInterpolatedFunction(
      "spline", 0,
      [this](const QDateTime& dt) {
        return controller_->getInterpolatedValue(dt);
      },
      ui->spin_interpolation_points->value());
}

void MainWindow::onPlotNewtonPolynomialClicked() {
  int degree = ui->spin_newton_degree->value();
  const auto data = controller_->getTradeData();

  if (degree > 10) {
    const QString warningMsg =
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
      ui->spin_interpolation_points->value());
}

QString MainWindow::createGraphLabel(const QString& type, int degree,
                                     int pointCount) const {
  // Извлекаем имя файла из заголовка: 'имя.csv'
  QString fileName = "unknown.csv";
  QString title = this->windowTitle();
  auto start = title.indexOf("'");
  auto end = title.indexOf("'", start + 1);
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
  return QString("%1 %2 — %3").arg(bullet, prefix, fileName);
}

void MainWindow::setupDateTimeEditLimits() const {
  const auto data = controller_->getTradeData();
  if (data.empty()) return;

  const QDateTime minDate = QDateTime::fromSecsSinceEpoch(
      static_cast<qint64>(data.front().timestamp));
  const QDateTime maxDate =
      QDateTime::fromSecsSinceEpoch(static_cast<qint64>(data.back().timestamp));

  ui->dt_interpolation_input->setDateTimeRange(minDate, maxDate);
  ui->dt_interpolation_input->setDateTime(minDate);
}

void MainWindow::onGetInterpolatedValueClicked() {  // NOLINT
  const QDateTime dateTime = ui->dt_interpolation_input->dateTime();
  const int degree = ui->spin_newton_degree->value();

  // === Сплайн ===
  const double splineValue = controller_->getInterpolatedValue(dateTime);
  const QString splineText =
      std::isnan(splineValue) ? "—" : QString::number(splineValue, 'f', 6);
  ui->lbl_spline_value->setText("Значение сплайна: " + splineText);

  // === Полином Ньютона ===
  const double newValue =
      controller_->getInterpolatedValueNewton(dateTime, degree);
  const QString newtonText =
      std::isnan(newValue) ? "—" : QString::number(newValue, 'f', 6);
  ui->lbl_newton_value->setText("Значение полинома: " + newtonText);
}

void MainWindow::onShowDataPointsToggled() {
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
    const QString& type, const int degree,
    const std::function<double(const QDateTime&)>& valueFunc,
    const int pointCount) {
  if (controller_->getDataCount() == 0) {
    QMessageBox::warning(this, "Ошибка", "Нет данных. Загрузите CSV.");
    return;
  }

  const auto data = controller_->getTradeData();
  if (data.size() < 2) {
    QMessageBox::warning(this, "Ошибка", "Минимум 2 точки.");
    return;
  }

  const double xStart = data.front().timestamp;
  const double xEnd = data.back().timestamp;

  QVector<double> x = generateX(xStart, xEnd, pointCount);
  QVector<double> y;
  y.reserve(x.size());

  for (double xi : x) {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(xi));
    y.append(valueFunc(dt));
  }

  const QString label = createGraphLabel(type, degree, pointCount);
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
  const int index = ui->plotWidget->graphCount() - 1;
  QColor color = colors[index % colors.size()];
  graph->setPen(QPen(color, 2));

  // Применяем стиль: точки или нет
  graph->setScatterStyle(
      showPoints_ ? QCPScatterStyle(QCPScatterStyle::ssCircle, color, color, 6)
                  : QCPScatterStyle::ssNone);

  ui->plotWidget->rescaleAxes();
  ui->plotWidget->replot();
}

QVector<double> MainWindow::generateX(const double xStart, const double xEnd,
                                      const int numPoints) {
  QVector<double> x;
  x.reserve(numPoints);
  if (numPoints == 1) {
    x.append(xStart);
  } else {
    const double step = (xEnd - xStart) / (numPoints - 1);
    for (int i = 0; i < numPoints; ++i) {
      x.append(xStart + i * step);
    }
  }
  return x;
}
