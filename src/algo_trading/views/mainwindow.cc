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

  // =====================================================================
  // === Настройка QCustomPlot для аппроксимации ===
  ui->plot_approximation->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                          QCP::iSelectPlottables);
  ui->plot_approximation->axisRect()->setupFullAxesBox();

  // Тикер времени
  const QSharedPointer<QCPAxisTickerDateTime> dateTickerApprox(
      new QCPAxisTickerDateTime);
  dateTickerApprox->setDateTimeFormat("dd.MM.yyyy");
  dateTickerApprox->setDateTimeSpec(Qt::UTC);
  ui->plot_approximation->xAxis->setTicker(dateTickerApprox);

  // Оси
  ui->plot_approximation->xAxis->setLabel("Дата");
  ui->plot_approximation->yAxis->setLabel("Цена");
  ui->plot_approximation->xAxis->setLabelColor(Qt::darkBlue);
  ui->plot_approximation->yAxis->setLabelColor(Qt::darkBlue);
  ui->plot_approximation->xAxis->setTickLabelColor(Qt::darkGray);
  ui->plot_approximation->yAxis->setTickLabelColor(Qt::darkGray);

  // Фон
  ui->plot_approximation->setBackground(QColor(245, 245, 245));

  // Сетка — мягкие линии
  const QPen majorGridPenApprox(Qt::lightGray, 1, Qt::SolidLine);
  const QPen minorGridPenApprox(Qt::gray, 0.8, Qt::DotLine);

  ui->plot_approximation->xAxis->grid()->setPen(majorGridPenApprox);
  ui->plot_approximation->yAxis->grid()->setPen(majorGridPenApprox);
  ui->plot_approximation->xAxis->grid()->setSubGridPen(minorGridPenApprox);
  ui->plot_approximation->yAxis->grid()->setSubGridPen(minorGridPenApprox);
  ui->plot_approximation->xAxis->grid()->setSubGridVisible(true);
  ui->plot_approximation->yAxis->grid()->setSubGridVisible(true);

  // Легенда
  ui->plot_approximation->legend->setVisible(true);
  ui->plot_approximation->legend->setBrush(QColor(255, 255, 255, 220));
  ui->plot_approximation->legend->setBorderPen(QPen(Qt::lightGray, 1));
  ui->plot_approximation->legend->setFont(QFont("Arial", 9));

  // Диапазон по умолчанию: 2020–2025
  const QDateTime startApprox =
      QDateTime::fromString("2020-01-01", "yyyy-MM-dd");
  const QDateTime endApprox = QDateTime::fromString("2025-01-01", "yyyy-MM-dd");
  const auto start_secApprox =
      static_cast<double>(startApprox.toSecsSinceEpoch());
  const auto end_secApprox = static_cast<double>(endApprox.toSecsSinceEpoch());

  ui->plot_approximation->xAxis->setRange(start_secApprox, end_secApprox);
  ui->plot_approximation->yAxis->setRange(0, 100);

  // Ограничение масштабирования
  ui->plot_approximation->xAxis->setRangeLower(start_secApprox);

  // Настройка шрифтов
  QFont axisFontApprox = font();
  axisFontApprox.setPointSize(9);
  ui->plot_approximation->xAxis->setTickLabelFont(axisFontApprox);
  ui->plot_approximation->yAxis->setTickLabelFont(axisFontApprox);
  // =====================================================================

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

  // === Подключение сигналов для вкладки "Аппроксимация" ===
  connect(ui->btn_load_data_csv_approx, &QPushButton::clicked, this,
          &MainWindow::onLoadDataCsvClicked);

  connect(ui->btn_plot_lsq_no_weights, &QPushButton::clicked, this,
          &MainWindow::onPlotLsqNoWeightsClicked);

  connect(ui->btn_plot_lsq_with_weights, &QPushButton::clicked, this,
          &MainWindow::onPlotLsqWithWeightsClicked);

  connect(ui->btn_get_approx_value, &QPushButton::clicked, this,
          &MainWindow::onGetApproxValueClicked);

  connect(ui->btn_clear_approximation, &QPushButton::clicked, this,
          &MainWindow::onClearApproximationClicked);

  connect(ui->btn_show_approximation_points, &QPushButton::clicked, this,
          &MainWindow::onShowApproxPointsToggled);

  connect(ui->btn_plot_four_graphs, &QPushButton::clicked, this,
          &MainWindow::onPlotFourGraphsClicked);

  // === Подключение сигналов для вкладки "Исследования" ===
  connect(ui->btn_run_timing_study, &QPushButton::clicked, this,
          &MainWindow::onRunTimingStudyClicked);

  connect(ui->btn_load_data_csv_stats, &QPushButton::clicked, this,
          &MainWindow::onLoadDataCsvClicked);

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
  onClearApproximationClicked();

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
      ui->spin_approx_points->setMinimum(static_cast<int>(count));
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

void MainWindow::onPlotLsqNoWeightsClicked() {
  plotApproximation(false);  // без весов
}

void MainWindow::onPlotLsqWithWeightsClicked() {
  plotApproximation(true);  // с весами
}

void MainWindow::plotApproximation(bool use_weights) {
  const int degree = ui->spin_approx_degree->value();
  const int days = ui->spin_extrapolate_days->value();
  const int numPoints = ui->spin_approx_points->value();

  // По заданию: если M изменилось — очистить график
  if (days != last_extrapolate_days_) {
    ui->plot_approximation->clearGraphs();
    ui->plot_approximation->clearItems();
    last_extrapolate_days_ = days;
  }

  if (ui->plot_approximation->graphCount() >= 5) {
    QMessageBox::warning(this, "Лимит", "Не более 5 графиков.");
    return;
  }

  controller_->buildLeastSquaresModel(degree, use_weights);

  const auto curve = controller_->generateApproximationCurve(
      degree, use_weights, numPoints, days);

  if (curve.empty()) {
    QMessageBox::warning(this, "Ошибка", "Не удалось построить кривую.");
    return;
  }

  QVector<double> x, y;
  for (const auto& [xi, yi] : curve) {
    x.append(xi);
    y.append(yi);
  }

  QString method = use_weights ? "LSQ (с весами)" : "LSQ (без весов)";
  QString label = QString("● %1, степень=%2, M=%3 — %4")
                      .arg(method)
                      .arg(degree)
                      .arg(days)
                      .arg(getFileNameFromTitle());

  QCPGraph* graph = ui->plot_approximation->addGraph();
  graph->setData(x, y);
  graph->setName(label);

  static QVector<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta,
                                   Qt::darkCyan};
  QColor color = colors[ui->plot_approximation->graphCount() - 1];
  graph->setPen(QPen(color, 2));

  // Исходные точки — жирнее, другим цветом
  if (showApproxPoints_) {
    QCPGraph* points = ui->plot_approximation->addGraph();
    QVector<double> px, py;
    const auto& data = controller_->getTradeData();
    for (const auto& td : data) {
      px.append(td.timestamp);
      py.append(td.close);
    }
    points->setData(px, py);
    points->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::yellow, 8));
    points->setPen(QPen(Qt::yellow, 1.5));
    points->setName("Исходные точки");
  }

  ui->plot_approximation->rescaleAxes();
  ui->plot_approximation->replot();
}

void MainWindow::onGetApproxValueClicked() {
  QDateTime dt = ui->dt_approx_input->dateTime();
  int degree = ui->spin_approx_degree->value();

  // Без весов
  double value_no_weight = controller_->getApproximatedValue(dt, degree, false);
  QString val_str_no = std::isnan(value_no_weight)
                           ? "—"
                           : QString::number(value_no_weight, 'f', 6);

  // С весами
  double value_with_weight =
      controller_->getApproximatedValue(dt, degree, true);
  QString val_str_w = std::isnan(value_with_weight)
                          ? "—"
                          : QString::number(value_with_weight, 'f', 6);

  ui->lbl_approx_value->setText(
      QString("Без весов: %1; С весами: %2").arg(val_str_no, val_str_w));
}

QString MainWindow::getFileNameFromTitle() const {
  QString fileName = "unknown.csv";
  const QString title = this->windowTitle();

  const auto start = title.indexOf('\'');
  const auto end = title.indexOf('\'', start + 1);

  if (start != -1 && end != -1 && end > start) {
    fileName = title.mid(start + 1, end - start - 1);
  }

  return fileName;
}

void MainWindow::onClearApproximationClicked() {
  ui->plot_approximation->clearGraphs();
  ui->plot_approximation->clearItems();
  ui->plot_approximation->replot();
}

void MainWindow::onShowApproxPointsToggled() {
  showApproxPoints_ = !showApproxPoints_;

  // Удаляем существующие точки (если есть)
  for (int i = 0; i < ui->plot_approximation->graphCount(); ++i) {
    QCPGraph* g = ui->plot_approximation->graph(i);
    if (g->name() == "Исходные точки") {
      ui->plot_approximation->removeGraph(g);
      break;
    }
  }

  // Если включено — добавляем точки
  if (showApproxPoints_) {
    const auto& data = controller_->getTradeData();
    if (!data.empty()) {
      QCPGraph* points = ui->plot_approximation->addGraph();

      QVector<double> px, py;
      for (const auto& td : data) {
        px.append(td.timestamp);
        py.append(td.close);
      }

      points->setData(px, py);
      points->setName("Исходные точки");
      points->setPen(QPen(Qt::yellow, 1.5));
      points->setScatterStyle(QCPScatterStyle(
          QCPScatterStyle::ssCircle,  // форма
          Qt::black,                  // обводка
          Qt::yellow,                 // заливка
          8                           // радиус
          ));
    }
  }

  ui->plot_approximation->replot();
}

void MainWindow::onPlotFourGraphsClicked() {
  const int numPoints = ui->spin_approx_points->value();
  const int days = ui->spin_extrapolate_days->value();

  ui->plot_approximation->clearGraphs();
  ui->plot_approximation->clearItems();
  last_extrapolate_days_ = days;

  struct PlotConfig {
    int degree;
    bool use_weights;
    QString label_suffix;
    QColor color;
  };

  static QVector<PlotConfig> configs = {
      {1, false, "степень 1, без весов", Qt::blue},
      {2, false, "степень 2, без весов", Qt::red},
      {1, true, "степень 1, с весами", Qt::green},
      {2, true, "степень 2, с весами", Qt::magenta}};

  for (const auto& config : configs) {
    controller_->buildLeastSquaresModel(config.degree, config.use_weights);

    const auto curve = controller_->generateApproximationCurve(
        config.degree, config.use_weights, numPoints, days);

    if (curve.empty()) continue;

    QVector<double> x, y;
    for (const auto& [xi, yi] : curve) {
      x.append(xi);
      y.append(yi);
    }

    QCPGraph* graph = ui->plot_approximation->addGraph();
    graph->setData(x, y);

    QString label = QString("● %1 — %2")
                        .arg(config.label_suffix)
                        .arg(getFileNameFromTitle());

    graph->setName(label);
    graph->setPen(QPen(config.color, 2));
    graph->setScatterStyle(QCPScatterStyle::ssNone);
  }

  if (showApproxPoints_) {
    QCPGraph* points = ui->plot_approximation->addGraph();
    QVector<double> px, py;
    const auto& data = controller_->getTradeData();
    for (const auto& td : data) {
      px.append(td.timestamp);
      py.append(td.close);
    }
    points->setData(px, py);
    points->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::yellow, 8));
    points->setPen(QPen(Qt::yellow, 1.5));
    points->setName("Исходные точки");
  }

  ui->plot_approximation->rescaleAxes();
  ui->plot_approximation->replot();
}

void MainWindow::onRunTimingStudyClicked() {
  const int h = ui->spin_stats_partitions->value();      // число разбиений
  const int k_max = ui->spin_stats_max_points->value();  // макс. точек
  const int newton_degree = 5;  // фиксированная степень для теста

  const auto data = controller_->getTradeData();
  if (data.size() < 2) {
    QMessageBox::warning(this, "Ошибка", "Нужно хотя бы 2 точки.");
    return;
  }

  const int N = static_cast<int>(data.size());
  if (k_max < N) {
    QMessageBox::warning(this, "Ошибка", "Макс. число точек >= числа в файле");
    return;
  }
  if (h < 2) {
    QMessageBox::warning(this, "Ошибка", "Число разбиений h >= 2");
    return;
  }

  const double xStart = data.front().timestamp;
  const double xEnd = data.back().timestamp;

  // Очистка предыдущих результатов
  ui->plot_timing->clearGraphs();
  ui->plot_timing->clearItems();
  ui->table_timing_results->setRowCount(0);

  // Настройка таблицы
  ui->table_timing_results->setColumnCount(3);
  ui->table_timing_results->setHorizontalHeaderLabels(
      {"k", "Spline (мс)", "Newton (мс)"});

  std::vector<int> k_values(h);
  std::vector<double> times_spline(h), times_newton(h);

  const int measurements = 10;

  for (int i = 0; i < h; ++i) {
    const int k_i = N + (k_max - N) * i / (h - 1);
    k_values[i] = k_i;

    // Генерируем точки один раз
    QVector<double> x = generateX(xStart, xEnd, k_i);
    std::vector<QDateTime> dt_vec;
    dt_vec.reserve(k_i);
    for (double xi : x) {
      dt_vec.emplace_back(
          QDateTime::fromSecsSinceEpoch(static_cast<qint64>(xi)));
    }

    // === Измерение времени для сплайна ===
    auto start = std::chrono::high_resolution_clock::now();
    for (int m = 0; m < measurements; ++m) {
      for (const QDateTime& dt : dt_vec) {
        controller_->getInterpolatedValue(dt);  // вызов
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    times_spline[i] = duration_ms / measurements;

    // === Измерение времени для Ньютона ===
    start = std::chrono::high_resolution_clock::now();
    for (int m = 0; m < measurements; ++m) {
      for (const QDateTime& dt : dt_vec) {
        controller_->getInterpolatedValueNewton(dt, newton_degree);
      }
    }
    end = std::chrono::high_resolution_clock::now();
    duration_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    times_newton[i] = duration_ms / measurements;

    // Обновление таблицы
    int row = ui->table_timing_results->rowCount();
    ui->table_timing_results->insertRow(row);
    ui->table_timing_results->setItem(
        row, 0, new QTableWidgetItem(QString::number(k_i)));
    ui->table_timing_results->setItem(
        row, 1, new QTableWidgetItem(QString::number(times_spline[i], 'f', 3)));
    ui->table_timing_results->setItem(
        row, 2, new QTableWidgetItem(QString::number(times_newton[i], 'f', 3)));
  }

  // === Построение графиков ===
  QVector<double> k_plot(k_values.begin(), k_values.end());
  QVector<double> t_spline(times_spline.begin(), times_spline.end());
  QVector<double> t_newton(times_newton.begin(), times_newton.end());

  QCPGraph* graph_spline = ui->plot_timing->addGraph();
  graph_spline->setData(k_plot, t_spline);
  graph_spline->setName("Кубический сплайн");
  graph_spline->setPen(QPen(Qt::blue, 2));

  QCPGraph* graph_newton = ui->plot_timing->addGraph();
  graph_newton->setData(k_plot, t_newton);
  graph_newton->setName("Полином Ньютона (n=5)");
  graph_newton->setPen(QPen(Qt::red, 2));

  // Оси
  ui->plot_timing->xAxis->setLabel("Число точек k");
  ui->plot_timing->yAxis->setLabel("Среднее время (мс)");
  ui->plot_timing->legend->setVisible(true);
  ui->plot_timing->rescaleAxes();
  ui->plot_timing->replot();

  QMessageBox::information(this, "Готово",
                           "Исследование завершено. Построены графики "
                           "зависимости времени от числа точек.");
}
