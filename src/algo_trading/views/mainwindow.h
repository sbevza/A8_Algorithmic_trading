#ifndef SRC_MAINWINDOW_H_
#define SRC_MAINWINDOW_H_

#include <QColor>
#include <QMainWindow>
#include <QVector>
#include <functional>
#include <memory>

#include "controllers/algo_trading_controller.h"
#include "qcustomplot/qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

 private slots:
  // Общие действия
  void clearPlots(bool all = false);
  void toggleDataPoints();
  void plotBaseGraph();
  void onLoadDataCsvClicked();

  // Интерполяция
  void onPlotCubicSplineClicked();
  void onPlotNewtonPolynomialClicked();
  void onGetInterpolatedValueClicked();

  // Аппроксимация
  void onPlotLsqNoWeightsClicked();
  void onPlotLsqWithWeightsClicked();
  void onGetApproxValueClicked();
  void onPlotFourGraphsClicked();

  // Исследования
  void onRunTimingStudyClicked();

  // Вспомогательные слоты
  void setupDateTimeEditLimits() const;

 private:
  static constexpr int kMaxGraphsPerPlot = 5;
  static constexpr int kMaxNewtonDegree = 10;
  static constexpr int kDefaultTimingMeasurements = 10;
  static constexpr int kDefaultNewtonDegreeForTiming = 5;
  static constexpr double kDefaultGraphLineWidth = 2.0;

  std::unique_ptr<Ui::MainWindow> ui;
  std::unique_ptr<s21::AlgoTradingController> controller_;

  bool showPoints_ = false;
  bool showApproxPoints_ = false;
  int last_extrapolate_days_ = 0;
  int last_approx_degree_ = 1;
  QString currentFileName_;

  static const QVector<QColor> kGraphColors;

  [[nodiscard]] QString getCurrentFileName() const;
  [[nodiscard]] QString createGraphLabel(const QString& type, int degree,
                                         int pointCount) const;

  void setupPlot(QCustomPlot* plot);
  void addOriginalDataPoints(QCustomPlot* plot);
  void updateUiState() const;

  void plotGraph(QCustomPlot* plot, const QVector<double>& x,
                 const QVector<double>& y, const QString& label,
                 bool allowRescale = true);

  void plotInterpolatedFunction(
      const QString& type, int degree,
      const std::function<double(const QDateTime&)>& valueFunc, int pointCount,
      QCustomPlot* plot);

  void plotApproximation(bool use_weights);
  [[nodiscard]] static QVector<double> generateX(double xStart, double xEnd,
                                                 int numPoints);
};

#endif  // SRC_MAINWINDOW_H_
