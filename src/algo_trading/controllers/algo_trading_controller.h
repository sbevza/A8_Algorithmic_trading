// src/algo_trading/controllers/algo_trading_controller.h

#ifndef SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_
#define SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_

#include <QObject>
#include <QVector>

#include "models/csv_parser.h"
#include "models/cubic_spline_interpolator.h"
#include "models/newton_interpolator.h"

namespace s21 {

class AlgoTradingController : public QObject {
  Q_OBJECT

 public:
  explicit AlgoTradingController(QObject* parent = nullptr);

  bool loadTradingDataFromCsv(const QString& filepath);
  [[nodiscard]] const std::vector<TradeData>& getTradeData() const;
  [[nodiscard]] size_t getDataCount() const;
  [[nodiscard]] QString getLastError() const;

  void buildSplineFromLoadedData();
  [[nodiscard]] double getInterpolatedValue(const QDateTime &dateTime) const;

  void buildNewtonPolynomial(int degree);
  double getInterpolatedValueNewton(const QDateTime& dateTime, int degree);
 private:
  std::vector<TradeData> tradeData_;
  std::unique_ptr<s21::CubicSplineInterpolator> spline_interpolator_;
  std::unique_ptr<s21::NewtonInterpolator> newton_interpolator_;
  CsvParser parser_;
};

}  // namespace s21

#endif  // SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_
