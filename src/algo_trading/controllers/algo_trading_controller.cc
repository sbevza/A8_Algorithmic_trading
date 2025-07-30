// algo_trading_controller.cc
#include "algo_trading_controller.h"

#include <QDebug>

namespace s21 {
AlgoTradingController::AlgoTradingController(QObject *parent)
    : QObject(parent) {
  // Ничего не нужно делать — parser_ уже создан
}

bool AlgoTradingController::loadTradingDataFromCsv(const QString &content) {
  tradeData_.clear();
  auto parsed_data = parser_.Parse(content);

  if (parsed_data.isEmpty()) {
    qWarning() << "Failed to parse CSV:" << parser_.GetError();
    return false;
  }

  tradeData_ = parsed_data;
  qDebug() << "Successfully loaded" << tradeData_.size() << "records";
  return true;
}

const QVector<TradeData> &AlgoTradingController::getTradeData() const {
  return tradeData_;
}

int AlgoTradingController::getDataCount() const { return tradeData_.size(); }

QString AlgoTradingController::getLastError() const {
  return parser_.GetError();
}
void AlgoTradingController::buildSplineFromLoadedData() {
  // 1.
  if (tradeData_.size() < 2) {  // Spline needs 2 points
    qWarning() << "Not enough data points to build a spline (need at least 2). "
                  "Current count:"
               << tradeData_.size();
    spline_interpolator_.reset();
    return;
  }

  // 2.
  std::vector<s21::SplinePoint> spline_points;
  spline_points.reserve(tradeData_.size());

  for (const auto &td : tradeData_) {
    spline_points.push_back(
        {static_cast<double>(td.timestamp.toSecsSinceEpoch()), td.close});
  }

  // 3.
  try {
    spline_interpolator_ =
        std::make_unique<s21::CubicSplineInterpolator>(spline_points);
    qDebug() << "Cubic spline successfully built with" << spline_points.size()
             << "points.";
  } catch (const std::exception &e) {
    qCritical() << "Error building cubic spline:" << e.what();
    spline_interpolator_.reset();
  }
}

double AlgoTradingController::getInterpolatedValue(    const QDateTime &dateTime) const {
  if (!spline_interpolator_) {
    qWarning() << "Attempted to interpolate, but spline is not built.";
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Владос, тут какая-то ерунда, всё время приходится в секунды конвертить!!!!!
  // Может сразу в парсинге в секундах сохранять????????
  double x_val = static_cast<double>(dateTime.toSecsSinceEpoch());

  return spline_interpolator_->interpolate(x_val);
}

}  // namespace s21
