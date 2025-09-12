#ifndef SRC_MAINWINDOW_H_
#define SRC_MAINWINDOW_H_

#include <QFileDialog>
#include <QMainWindow>
#include <functional>

#include "controllers/algo_trading_controller.h"

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
  // === Интерполяция ===
  void onLoadDataCsvClicked();
  void onClearInterpolationClicked();
  void onPlotCubicSplineClicked();
  void onPlotNewtonPolynomialClicked();
  void onGetInterpolatedValueClicked();
  void onShowDataPointsToggled();

  // === Аппроксимация ===
  void onPlotLsqNoWeightsClicked();
  void onPlotLsqWithWeightsClicked();
  void onGetApproxValueClicked();
  void onClearApproximationClicked();
  void onShowApproxPointsToggled();
  void onPlotFourGraphsClicked();

  // === Исследования ===
  void onRunTimingStudyClicked();

 private:
  Ui::MainWindow* ui;
  s21::AlgoTradingController* controller_;
  [[nodiscard]] QString getFileNameFromTitle() const;
  [[nodiscard]] QString createGraphLabel(const QString& type, int degree,
                                         int pointCount) const;
  void updateUiState() const;
  void setupDateTimeEditLimits() const;
  bool showPoints_ = false;
  bool showApproxPoints_ = false;
  int last_extrapolate_days_ = 0;
  static QVector<double> generateX(double xStart, double xEnd, int numPoints);
  void plotInterpolatedGraph(const QVector<double>& x, const QVector<double>& y,
                             const QString& label);
  void plotInterpolatedFunction(
      const QString& type, int degree,
      const std::function<double(const QDateTime&)>& valueFunc, int pointCount);
  void plotApproximation(bool use_weights);
};

#endif  // SRC_MAINWINDOW_H_
