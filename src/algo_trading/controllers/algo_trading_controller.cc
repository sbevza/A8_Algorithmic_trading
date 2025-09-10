// src/algo_trading/controllers/algo_trading_controller.cc

#include "algo_trading_controller.h"

#include <QDateTime>
#include <QDebug>

#include "models/file_reader.h"

namespace s21 {
AlgoTradingController::AlgoTradingController(QObject *parent)
    : QObject(parent) {}

bool AlgoTradingController::loadTradingDataFromCsv(const QString &filepath) {
  const std::string path = filepath.toStdString();
  if (!FileReader::FileExists(path)) {
    qWarning() << "File does not exist: " << filepath;
    return false;
  }

  trade_data_.clear();
  const auto parsed_data = parser_.Parse(FileReader::ReadFile(path));

  if (parsed_data.empty()) {
    qWarning() << "Failed to parse CSV:"
               << QString::fromStdString(parser_.GetError());
    return false;
  }

  trade_data_ = parsed_data;
  qDebug() << "Successfully loaded" << trade_data_.size() << "records";

  buildSplineFromLoadedData();

  return true;
}

const std::vector<TradeData> &AlgoTradingController::getTradeData() const {
  return trade_data_;
}

size_t AlgoTradingController::getDataCount() const {
  return trade_data_.size();
}

QString AlgoTradingController::getLastError() const {
  return QString::fromStdString(parser_.GetError());
}

void AlgoTradingController::buildSplineFromLoadedData() {
  if (trade_data_.size() < 2) {  // Spline needs 2 points
    qWarning() << "Not enough data points to build a spline (need at least 2). "
                  "Current count:"
               << trade_data_.size();
    spline_interpolator_.reset();
    return;
  }

  try {
    spline_interpolator_ =
        std::make_unique<s21::CubicSplineInterpolator>(trade_data_);
    qDebug() << "Cubic spline successfully built with" << trade_data_.size()
             << "points.";
  } catch (const std::exception &e) {
    qCritical() << "Error building cubic spline:" << e.what();
    spline_interpolator_.reset();
  }
}

double AlgoTradingController::getInterpolatedValue(
    const QDateTime &dateTime) const {
  if (!spline_interpolator_) {
    qWarning() << "Attempted to interpolate, but spline is not built.";
    return std::numeric_limits<double>::quiet_NaN();
  }

  const auto x_val = static_cast<double>(dateTime.toSecsSinceEpoch());

  return spline_interpolator_->interpolate(x_val);
}

void AlgoTradingController::buildNewtonPolynomial(int degree) {
  if (degree < 1 || degree >= static_cast<int>(trade_data_.size())) {
    newton_interpolator_.reset();
    current_newton_degree_ = -1;
    return;
  }

  if (newton_interpolator_ && current_newton_degree_ == degree) {
    return;
  }

  try {
    newton_interpolator_ =
        std::make_unique<s21::NewtonInterpolator>(trade_data_, degree);
    current_newton_degree_ = degree;
    qDebug() << "Newton polynomial (degree" << degree << ") built or reused.";
  } catch (const std::exception &e) {
    qCritical() << "Error building Newton polynomial:" << e.what();
    newton_interpolator_.reset();
    current_newton_degree_ = -1;
  }
}

double AlgoTradingController::getInterpolatedValueNewton(
    const QDateTime &dateTime, const int degree) {
  if (degree < 1 || degree >= static_cast<int>(trade_data_.size())) {
    qWarning() << "Invalid degree:" << degree;
    return std::nan("");
  }

  if (!newton_interpolator_ || current_newton_degree_ != degree) {
    buildNewtonPolynomial(degree);
  }

  if (!newton_interpolator_) {
    return std::nan("");
  }

  const auto x = static_cast<double>(dateTime.toSecsSinceEpoch());
  return newton_interpolator_->interpolate(x);
}

}  // namespace s21
