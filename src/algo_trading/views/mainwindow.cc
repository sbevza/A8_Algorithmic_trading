#include "mainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QMessageBox>
#include <QPen>
#include <QProgressDialog>
#include <QSharedPointer>
#include <chrono>
#include <vector>

#include "qcustomplot/qcustomplot.h"
#include "ui_mainwindow.h"

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(std::make_unique<Ui::MainWindow>()),
      controller_(std::make_unique<s21::AlgoTradingController>()) {
  ui->setupUi(this);

  setupPlot(ui->plotWidget);
  setupPlot(ui->plot_approximation);
  setupPlot(ui->plot_timing);

  ui->spin_interpolation_points->setMaximum(10000);

  // --- Сигналы: Интерполяция ---
  connect(ui->btn_plot_cubic_spline, &QPushButton::clicked, this,
          &MainWindow::onPlotCubicSplineClicked);
  connect(ui->btn_plot_newton_polynomial, &QPushButton::clicked, this,
          &MainWindow::onPlotNewtonPolynomialClicked);
  connect(ui->btn_get_interpolated_value, &QPushButton::clicked, this,
          &MainWindow::onGetInterpolatedValueClicked);

  // --- Сигналы: Аппроксимация ---
  connect(ui->btn_plot_lsq_no_weights, &QPushButton::clicked, this,
          &MainWindow::onPlotLsqNoWeightsClicked);
  connect(ui->btn_plot_lsq_with_weights, &QPushButton::clicked, this,
          &MainWindow::onPlotLsqWithWeightsClicked);
  connect(ui->btn_get_approx_value, &QPushButton::clicked, this,
          &MainWindow::onGetApproxValueClicked);
  connect(ui->btn_plot_four_graphs, &QPushButton::clicked, this,
          &MainWindow::onPlotFourGraphsClicked);
  connect(ui->spin_extrapolate_days,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &MainWindow::setupDateTimeEditLimits);

  // --- Сигналы: Исследования ---
  connect(ui->btn_run_timing_study, &QPushButton::clicked, this,
          &MainWindow::onRunTimingStudyClicked);

  // --- Глобальные действия ---
  connect(ui->action_CSV, &QAction::triggered, this,
          &MainWindow::onLoadDataCsvClicked);
  connect(ui->action_clear_graph, &QAction::triggered, this,
          &MainWindow::clearPlots);
  connect(ui->action_views_points, &QAction::triggered, this,
          &MainWindow::toggleDataPoints);
  connect(ui->action_base_graph, &QAction::triggered, this,
          &MainWindow::plotBaseGraph);

  updateUiState();
}

MainWindow::~MainWindow() = default;

const QVector<QColor> MainWindow::kGraphColors = {
    Qt::blue, Qt::red, Qt::darkGreen, Qt::magenta, QColor(255, 140, 0)};

void MainWindow::setupPlot(QCustomPlot* plot) {
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                        QCP::iSelectPlottables);
  plot->axisRect()->setupFullAxesBox();

  auto dateTicker = QSharedPointer<QCPAxisTickerDateTime>::create();
  dateTicker->setDateTimeFormat("dd.MM.yyyy");
  dateTicker->setDateTimeSpec(Qt::UTC);
  plot->xAxis->setTicker(dateTicker);

  plot->xAxis->setLabel("Дата");
  plot->yAxis->setLabel("Цена");
  plot->xAxis->setLabelColor(Qt::darkBlue);
  plot->yAxis->setLabelColor(Qt::darkBlue);
  plot->xAxis->setTickLabelColor(Qt::darkGray);
  plot->yAxis->setTickLabelColor(Qt::darkGray);

  plot->setBackground(QColor(245, 245, 245));

  const QPen majorGridPen(Qt::lightGray, 1, Qt::SolidLine);
  const QPen minorGridPen(Qt::gray, 0.8, Qt::DotLine);
  plot->xAxis->grid()->setPen(majorGridPen);
  plot->yAxis->grid()->setPen(majorGridPen);
  plot->xAxis->grid()->setSubGridPen(minorGridPen);
  plot->yAxis->grid()->setSubGridPen(minorGridPen);
  plot->xAxis->grid()->setSubGridVisible(true);
  plot->yAxis->grid()->setSubGridVisible(true);

  plot->legend->setVisible(true);
  plot->legend->setBrush(QColor(255, 255, 255, 220));
  plot->legend->setBorderPen(QPen(Qt::lightGray, 1));
  plot->legend->setFont(QFont("Arial", 9));

  const QDateTime start = QDateTime::fromString("2020-01-01", "yyyy-MM-dd");
  const QDateTime end = QDateTime::fromString("2025-01-01", "yyyy-MM-dd");
  plot->xAxis->setRange(start.toSecsSinceEpoch(), end.toSecsSinceEpoch());
  plot->yAxis->setRange(0, 100);
  plot->xAxis->setRangeLower(start.toSecsSinceEpoch());

  QFont axisFont = font();
  axisFont.setPointSize(9);
  plot->xAxis->setTickLabelFont(axisFont);
  plot->yAxis->setTickLabelFont(axisFont);
}

void MainWindow::addOriginalDataPoints(QCustomPlot* plot) {
  auto* points = plot->addGraph();
  QVector<double> px, py;
  const auto& data = controller_->getTradeData();
  px.reserve(data.size());
  py.reserve(data.size());

  for (const auto& td : data) {
    px.push_back(td.timestamp);
    py.push_back(td.close);
  }

  points->setData(px, py);
  points->setScatterStyle(
      QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::yellow, 8));
  points->setPen(QPen(Qt::yellow, kDefaultGraphLineWidth));
  points->setName("Исходные точки");
}

void MainWindow::updateUiState() const {
  const bool hasData = !controller_->getTradeData().empty();
  const auto enableWidgets =
      [hasData](const std::initializer_list<QWidget*>& widgets) {
        for (auto* w : widgets) w->setEnabled(hasData);
      };

  // Интерполяция
  enableWidgets({ui->btn_plot_cubic_spline, ui->btn_plot_newton_polynomial,
                 ui->btn_get_interpolated_value, ui->dt_interpolation_input,
                 ui->spin_interpolation_points, ui->spin_newton_degree,
                 ui->lbl_interpolation_points_label, ui->lbl_newton_value,
                 ui->lbl_spline_value, ui->lbl_newton_degree_label});

  // Аппроксимация
  enableWidgets({ui->btn_plot_lsq_no_weights, ui->btn_plot_lsq_with_weights,
                 ui->btn_plot_four_graphs, ui->btn_get_approx_value,
                 ui->dt_approx_input, ui->spin_approx_points,
                 ui->spin_approx_degree, ui->spin_extrapolate_days,
                 ui->lbl_approx_value, ui->lbl_approx_points_label,
                 ui->lbl_approx_degree_label, ui->lbl_extrapolate_days_label});

  // Исследования
  enableWidgets({ui->btn_run_timing_study, ui->spin_stats_max_points,
                 ui->spin_stats_partitions, ui->table_timing_results});
}

void MainWindow::clearPlots(bool all) {
  QList<QCustomPlot*> plotsToClear;
  if (all) {
    plotsToClear = findChildren<QCustomPlot*>();
  } else {
    QWidget* currentTab = ui->tabWidget->currentWidget();
    if (currentTab == ui->aproximation) {
      plotsToClear << ui->plot_approximation;
    } else if (currentTab == ui->inerpolation) {
      plotsToClear << ui->plotWidget;
    } else if (currentTab == ui->timeStats) {
      plotsToClear << ui->plot_timing;
    }
  }

  for (auto* plot : plotsToClear) {
    plot->clearGraphs();
    plot->clearItems();
    plot->replot();
  }
}

void MainWindow::onLoadDataCsvClicked() {
  clearPlots(true);

  const QString defaultDir = "../materials";
  const QString fileName = QFileDialog::getOpenFileName(
      this, tr("Открыть CSV файл с торговыми данными"), defaultDir,
      tr("CSV файлы (*.csv);;Все файлы (*)"));

  if (fileName.isEmpty()) return;

  if (!controller_->loadTradingDataFromCsv(fileName)) {
    QMessageBox::warning(this, tr("Ошибка"),
                         tr("Ошибка при загрузке или парсинге файла"));
    setWindowTitle(windowTitle().section(" — ", 0, 0) +
                   " — Ошибка загрузки данных");
    updateUiState();
    return;
  }

  const size_t count = controller_->getDataCount();
  if (count == 0) {
    QMessageBox::warning(this, tr("Ошибка"),
                         tr("Файл загружен, но данные не найдены"));
    updateUiState();
    return;
  }

  const QFileInfo fileInfo(fileName);
  currentFileName_ = fileInfo.fileName();
  const QString baseTitle = windowTitle().section(" — ", 0, 0);
  setWindowTitle(
      QString("%1 — Данные из файла '%2' успешно загружены, точек: %3")
          .arg(baseTitle, currentFileName_)
          .arg(count));

  ui->spin_interpolation_points->setMinimum(static_cast<int>(count));
  ui->spin_approx_points->setMinimum(static_cast<int>(count));
  ui->spin_stats_max_points->setMinimum(static_cast<int>(count));

  updateUiState();
  setupDateTimeEditLimits();
}

QString MainWindow::getCurrentFileName() const {
  return currentFileName_.isEmpty() ? "unknown.csv" : currentFileName_;
}

void MainWindow::plotBaseGraph() {
  if (controller_->getDataCount() == 0) {
    QMessageBox::information(this, "Нет данных",
                             "Нет загруженных данных для отображения.");
    return;
  }

  QWidget* currentTab = ui->tabWidget->currentWidget();
  QCustomPlot* plot = nullptr;
  if (currentTab == ui->aproximation) {
    plot = ui->plot_approximation;
  } else if (currentTab == ui->inerpolation) {
    plot = ui->plotWidget;
  } else if (currentTab == ui->timeStats) {
    plot = ui->plot_timing;
  } else {
    return;
  }

  const QString baseLabel = "Исходные данные";
  for (int i = 0; i < plot->graphCount(); ++i) {
    if (plot->graph(i)->name().contains(baseLabel)) {
      QMessageBox::information(this, "Уже добавлено",
                               "График исходных данных уже отображается.");
      return;
    }
  }

  if (plot->graphCount() >= kMaxGraphsPerPlot) {
    QMessageBox::warning(this, "Лимит",
                         "Не более 5 графиков на одном графике.");
    return;
  }

  const auto& data = controller_->getTradeData();
  QVector<double> x, y;
  x.reserve(data.size());
  y.reserve(data.size());
  for (const auto& point : data) {
    x.push_back(point.timestamp);
    y.push_back(point.close);
  }

  const QString label =
      QString("● Исходные данные — %1").arg(getCurrentFileName());
  plotGraph(plot, x, y, label);
}

void MainWindow::onPlotCubicSplineClicked() {
  plotInterpolatedFunction(
      "spline", 0,
      [this](const QDateTime& dt) {
        return controller_->getInterpolatedValue(dt);
      },
      ui->spin_interpolation_points->value(), ui->plotWidget);
}

void MainWindow::onPlotNewtonPolynomialClicked() {
  const int degree = ui->spin_newton_degree->value();
  const auto& data = controller_->getTradeData();

  if (degree > kMaxNewtonDegree) {
    QMessageBox::information(this, tr("Не построено"),
                             tr("Степень полинома %1 — это слишком много.\n"
                                "Полиномы степени выше %2:\n"
                                "• Сильно колеблются (явление Рунге)\n"
                                "• Дают неадекватные значения\n"
                                "• Численно нестабильны\n\n"
                                "Снизьте степень до %2 или меньше.")
                                 .arg(degree)
                                 .arg(kMaxNewtonDegree));
    return;
  }

  if (data.size() < static_cast<size_t>(degree + 1)) {
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
      ui->spin_interpolation_points->value(), ui->plotWidget);
}

QString MainWindow::createGraphLabel(const QString& type, int degree,
                                     int pointCount) const {
  QString prefix;
  if (type == "spline") {
    prefix = QString("Spline (%1)").arg(pointCount);
  } else if (type == "newton") {
    prefix = QString("Newton (%1) n=%2").arg(pointCount).arg(degree);
  } else {
    prefix = QString("%1 (%2)").arg(type.toUpper()).arg(pointCount);
  }
  return QString("● %1 — %2").arg(prefix, getCurrentFileName());
}

void MainWindow::setupDateTimeEditLimits() const {
  const auto& data = controller_->getTradeData();
  if (data.empty()) return;

  const QDateTime minDate = QDateTime::fromSecsSinceEpoch(
      static_cast<qint64>(data.front().timestamp));
  const QDateTime maxDataDate =
      QDateTime::fromSecsSinceEpoch(static_cast<qint64>(data.back().timestamp));
  const int extrapolateDays = ui->spin_extrapolate_days->value();
  const QDateTime maxAllowedDate = maxDataDate.addDays(extrapolateDays);

  ui->dt_interpolation_input->setDateTimeRange(minDate, maxDataDate);
  ui->dt_approx_input->setDateTimeRange(minDate, maxAllowedDate);
  ui->dt_interpolation_input->setDateTime(minDate);
  ui->dt_approx_input->setDateTime(minDate);
}

void MainWindow::onGetInterpolatedValueClicked() {
  const QDateTime dateTime = ui->dt_interpolation_input->dateTime();
  const int degree = ui->spin_newton_degree->value();

  const double splineValue = controller_->getInterpolatedValue(dateTime);
  const QString splineText =
      std::isnan(splineValue) ? "—" : QString::number(splineValue, 'f', 6);
  ui->lbl_spline_value->setText("Значение сплайна: " + splineText);

  const double newValue =
      controller_->getInterpolatedValueNewton(dateTime, degree);
  const QString newtonText =
      std::isnan(newValue) ? "—" : QString::number(newValue, 'f', 6);
  ui->lbl_newton_value->setText("Значение полинома: " + newtonText);
}

void MainWindow::toggleDataPoints() {
  showPoints_ = !showPoints_;
  for (auto* plot : findChildren<QCustomPlot*>()) {
    for (int i = 0; i < plot->graphCount(); ++i) {
      auto* g = plot->graph(i);
      const double pointSize = g->pen().widthF() * 2.0;
      g->setScatterStyle(showPoints_
                             ? QCPScatterStyle(QCPScatterStyle::ssCircle,
                                               Qt::black, Qt::black, pointSize)
                             : QCPScatterStyle::ssNone);
    }
    plot->replot();
  }
}

void MainWindow::plotInterpolatedFunction(
    const QString& type, int degree,
    const std::function<double(const QDateTime&)>& valueFunc, int pointCount,
    QCustomPlot* plot) {
  if (controller_->getDataCount() < 2) {
    QMessageBox::warning(this, "Ошибка", "Минимум 2 точки.");
    return;
  }

  const auto& data = controller_->getTradeData();
  const double xStart = data.front().timestamp;
  const double xEnd = data.back().timestamp;
  const auto x = generateX(xStart, xEnd, pointCount);
  QVector<double> y;
  y.reserve(x.size());

  for (double xi : x) {
    const QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(xi));
    y.push_back(valueFunc(dt));
  }

  const QString label = createGraphLabel(type, degree, pointCount);
  plotGraph(plot, x, y, label);
}

void MainWindow::plotGraph(QCustomPlot* plot, const QVector<double>& x,
                           const QVector<double>& y, const QString& label,
                           bool allowRescale) {
  if (plot->graphCount() >= kMaxGraphsPerPlot) {
    QMessageBox::warning(this, "Лимит", "Не более 5 графиков.");
    return;
  }

  auto* graph = plot->addGraph();
  graph->setData(x, y);
  graph->setName(label);
  const int colorIndex = plot->graphCount() - 1;
  const QColor color = kGraphColors[colorIndex % kGraphColors.size()];
  graph->setPen(QPen(color, kDefaultGraphLineWidth));
  graph->setScatterStyle(QCPScatterStyle::ssNone);

  if (allowRescale) {
    plot->rescaleAxes();
  }
  plot->replot();
}

QVector<double> MainWindow::generateX(double xStart, double xEnd,
                                      int numPoints) {
  if (numPoints <= 0) return {};
  if (numPoints == 1) return {xStart};

  QVector<double> x;
  x.reserve(numPoints);
  const double step = (xEnd - xStart) / (numPoints - 1);
  for (int i = 0; i < numPoints; ++i) {
    x.push_back(xStart + i * step);
  }
  return x;
}

void MainWindow::onPlotLsqNoWeightsClicked() { plotApproximation(false); }
void MainWindow::onPlotLsqWithWeightsClicked() { plotApproximation(true); }

void MainWindow::plotApproximation(bool use_weights) {
  const int degree = ui->spin_approx_degree->value();
  const int days = ui->spin_extrapolate_days->value();
  const int numPoints = ui->spin_approx_points->value();

  if (days != last_extrapolate_days_ || degree != last_approx_degree_) {
    ui->plot_approximation->clearGraphs();
    ui->plot_approximation->clearItems();
    last_extrapolate_days_ = days;
    last_approx_degree_ = degree;
  }

  controller_->buildLeastSquaresModel(degree, use_weights);
  const auto curve = controller_->generateApproximationCurve( numPoints, days);
  if (curve.empty()) {
    QMessageBox::warning(this, "Ошибка", "Не удалось построить кривую.");
    return;
  }

  QVector<double> x, y;
  x.reserve(curve.size());
  y.reserve(curve.size());
  for (const auto& [xi, yi] : curve) {
    x.push_back(xi);
    y.push_back(yi);
  }

  const QString method = use_weights ? "LSQ (с весами)" : "LSQ (без весов)";
  const QString label = QString("● %1, степень=%2, M=%3 — %4")
                            .arg(method)
                            .arg(degree)
                            .arg(days)
                            .arg(getCurrentFileName());

  plotGraph(ui->plot_approximation, x, y, label);

  if (showApproxPoints_) {
    addOriginalDataPoints(ui->plot_approximation);
  }
}

void MainWindow::onGetApproxValueClicked() {
  const QDateTime dt = ui->dt_approx_input->dateTime();
  const int degree = ui->spin_approx_degree->value();

  const double value_no_weight =
      controller_->getApproximatedValue(dt, degree, false);
  const QString val_str_no = std::isnan(value_no_weight)
                                 ? "—"
                                 : QString::number(value_no_weight, 'f', 6);

  const double value_with_weight =
      controller_->getApproximatedValue(dt, degree, true);
  const QString val_str_w = std::isnan(value_with_weight)
                                ? "—"
                                : QString::number(value_with_weight, 'f', 6);

  ui->lbl_approx_value->setText(
      QString("Без весов: %1; С весами: %2").arg(val_str_no, val_str_w));
}

void MainWindow::onPlotFourGraphsClicked() {
  QCustomPlot* plot = ui->plot_approximation;

  if (plot->graphCount() + 4 > kMaxGraphsPerPlot) {
    QMessageBox::warning(this, "Лимит", "Не более 5 графиков.");
    return;
  }

  const int numPoints = ui->spin_approx_points->value();
  const int days = ui->spin_extrapolate_days->value();
  last_extrapolate_days_ = days;

  struct PlotConfig {
    int degree;
    bool use_weights;
    QString label_suffix;
  };

  const QVector<PlotConfig> configs = {{1, false, "n=1, без весов"},
                                       {2, false, "n=2, без весов"},
                                       {1, true, "n=1, с весами"},
                                       {2, true, "n=2, с весами"}};

  for (const auto& config : configs) {
    controller_->buildLeastSquaresModel(config.degree, config.use_weights);
    const auto curve = controller_->generateApproximationCurve(numPoints, days);
    if (curve.empty()) continue;

    QVector<double> x, y;
    x.reserve(curve.size());
    y.reserve(curve.size());
    for (const auto& [xi, yi] : curve) {
      x.push_back(xi);
      y.push_back(yi);
    }

    const QString label =
        QString("● %1 — %2").arg(config.label_suffix, getCurrentFileName());
    plotGraph(plot, x, y, label, false);
  }

  if (showApproxPoints_) {
    addOriginalDataPoints(plot);
  }

  plot->rescaleAxes();
  plot->replot();
}

void MainWindow::onRunTimingStudyClicked() {
  const int h = ui->spin_stats_partitions->value();
  const int k_max = ui->spin_stats_max_points->value();
  const int newton_degree = kDefaultNewtonDegreeForTiming;

  const auto& data = controller_->getTradeData();
  if (data.size() < 2) {
    QMessageBox::warning(this, "Ошибка", "Нужно хотя бы 2 точки.");
    return;
  }

  const int N = static_cast<int>(data.size());
  if (k_max < N || h < 2) {
    QMessageBox::warning(
        this, "Ошибка",
        QString("k_max (%1) >= N (%2) и h (%3) >= 2").arg(k_max).arg(N).arg(h));
    return;
  }

  // Изменим максимальное значение на 2 * h
  const int totalSteps = 2 * h;  // Сплайн + Ньютон для каждого k
  QProgressDialog progress("Выполнение измерений...", "Отмена", 0, totalSteps,
                           this);
  progress.setWindowTitle("Исследование временных характеристик");
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.show();

  const double xStart = data.front().timestamp;
  const double xEnd = data.back().timestamp;

  ui->plot_timing->clearGraphs();
  ui->plot_timing->clearItems();
  ui->table_timing_results->setRowCount(0);
  ui->table_timing_results->setColumnCount(3);
  ui->table_timing_results->setHorizontalHeaderLabels(
      {"k", "Spline (мс)", "Newton (мс)"});

  std::vector<int> k_values(h);
  std::vector<double> times_spline(h), times_newton(h);

  auto measureTime = [&](const std::function<void()>& func) -> double {
    double total = 0.0;
    for (int m = 0; m < kDefaultTimingMeasurements; ++m) {
      const auto start = std::chrono::high_resolution_clock::now();
      func();
      const auto end = std::chrono::high_resolution_clock::now();
      total += std::chrono::duration<double, std::milli>(end - start).count();
    }
    return total / kDefaultTimingMeasurements;
  };

  int currentStep = 0;  // Счетчик текущего шага для прогресса
  for (int i = 0; i < h; ++i) {
    if (progress.wasCanceled()) {
      QMessageBox::information(this, "Отменено",
                               "Исследование прервано пользователем.");
      return;
    }

    const int k_i = N + (k_max - N) * i / (h - 1);
    k_values[i] = k_i;

    // Обновление прогресса перед измерением сплайна
    progress.setLabelText(QString("Измерение %1/%2 (k=%3) — сплайн...")
                              .arg(i + 1)
                              .arg(h)
                              .arg(k_i));
    QApplication::processEvents();

    const auto x = generateX(xStart, xEnd, k_i);
    std::vector<QDateTime> dt_vec;
    dt_vec.reserve(x.size());
    for (double xi : x) {
      dt_vec.emplace_back(
          QDateTime::fromSecsSinceEpoch(static_cast<qint64>(xi)));
    }

    double total_spline_time = measureTime([&]() {
      double sum = 0.0;
      for (const auto& dt : dt_vec) {
        sum += controller_->getInterpolatedValue(dt);
      }
      if (std::isnan(sum)) {
        // Обработка NaN, если нужно
      }
    });

    times_spline[i] = total_spline_time;

    // Увеличиваем счетчик и обновляем прогресс после измерения сплайна
    ++currentStep;
    progress.setValue(currentStep);
    QApplication::processEvents();  // Обязательно для обновления UI

    if (progress.wasCanceled()) {  // Проверка отмены после каждого шага
      QMessageBox::information(this, "Отменено",
                               "Исследование прервано пользователем.");
      return;
    }

    // Обновление прогресса перед измерением Ньютона
    progress.setLabelText(QString("Измерение %1/%2 (k=%3) — Ньютон...")
                              .arg(i + 1)
                              .arg(h)
                              .arg(k_i));
    QApplication::processEvents();

    double total_newton_time = measureTime([&]() {
      double sum = 0.0;
      for (const auto& dt : dt_vec) {
        sum += controller_->getInterpolatedValueNewton(dt, newton_degree);
      }
      if (std::isnan(sum)) {
        // Обработка NaN, если нужно
      }
    });

    times_newton[i] = total_newton_time;

    // Увеличиваем счетчик и обновляем прогресс после измерения Ньютона
    ++currentStep;
    progress.setValue(currentStep);
    QApplication::processEvents();  // Обязательно для обновления UI

    // Проверка отмены после второго шага итерации (опционально, можно и не
    // проверять здесь)
    if (progress.wasCanceled()) {
      QMessageBox::information(this, "Отменено",
                               "Исследование прервано пользователем.");
      return;
    }

    const int row = ui->table_timing_results->rowCount();
    ui->table_timing_results->insertRow(row);
    ui->table_timing_results->setItem(
        row, 0, new QTableWidgetItem(QString::number(k_i)));
    ui->table_timing_results->setItem(
        row, 1, new QTableWidgetItem(QString::number(times_spline[i], 'f', 3)));
    ui->table_timing_results->setItem(
        row, 2, new QTableWidgetItem(QString::number(times_newton[i], 'f', 3)));

    QApplication::processEvents();  // Обновление таблицы
  }

  // Прогресс должен быть уже равен totalSteps (2*h) в конце цикла
  // progress.setValue(totalSteps); // Этот вызов теперь избыточен, но не
  // помешает
  progress.setLabelText("Построение графиков...");
  QApplication::processEvents();

  // Построение графиков (ваш код)
  const QVector<double> k_plot(k_values.begin(), k_values.end());
  const QVector<double> t_spline(times_spline.begin(), times_spline.end());
  const QVector<double> t_newton(times_newton.begin(), times_newton.end());

  plotGraph(ui->plot_timing, k_plot, t_spline, "Кубический сплайн", false);
  plotGraph(ui->plot_timing, k_plot, t_newton, "Полином Ньютона (n=5)", false);

  ui->plot_timing->xAxis->setLabel("Число точек k");
  ui->plot_timing->yAxis->setLabel("Среднее время расчета (мс)");
  ui->plot_timing->legend->setVisible(true);
  ui->plot_timing->legend->setBrush(QColor(255, 255, 255, 200));
  ui->plot_timing->rescaleAxes();
  ui->plot_timing->replot();

  QMessageBox::information(this, "Исследование завершено",
                           QString("Исследование завершено!\n\n"
                                   "• Точек в файле (N): %1\n"
                                   "• Максимальное k: %2\n"
                                   "• Разбиений h: %3\n"
                                   "• Измерений на точку: %4")
                               .arg(N)
                               .arg(k_max)
                               .arg(h)
                               .arg(kDefaultTimingMeasurements));
}
