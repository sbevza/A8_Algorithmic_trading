// src/algo_trading/controllers/algo_trading_controller.cc

#include "algo_trading_controller.h"

#include <QDebug>

#include "models/file_reader.h"

namespace s21 {
AlgoTradingController::AlgoTradingController(QObject *parent)
    : QObject(parent) {}

bool AlgoTradingController::loadTradingDataFromCsv(const QString &content) {
  const std::string path = content.toStdString();
  if (!FileReader::FileExists(path)) {
    qWarning() << "File does not exist: " << content;
    return false;
  }

  tradeData_.clear();
  auto parsed_data = parser_.Parse(FileReader::ReadFile(path));

  if (parsed_data.empty()) {
    qWarning() << "Failed to parse CSV:"
               << QString::fromStdString(parser_.GetError());
    return false;
  }

  tradeData_ = parsed_data;
  qDebug() << "Successfully loaded" << tradeData_.size() << "records";

  buildSplineFromLoadedData();

  return true;
}

const std::vector<TradeData> &AlgoTradingController::getTradeData() const {
  return tradeData_;
}

size_t AlgoTradingController::getDataCount() const { return tradeData_.size(); }

QString AlgoTradingController::getLastError() const {
  return QString::fromStdString(parser_.GetError());
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
        {td.timestamp, td.close});
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

double AlgoTradingController::getInterpolatedValue(
    const QDateTime &dateTime) const {
  if (!spline_interpolator_) {
    qWarning() << "Attempted to interpolate, but spline is not built.";
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Владос, тут какая-то ерунда, всё время приходится в секунды конвертить!!!!!
  // Может сразу в парсинге в секундах сохранять????????
  double x_val = static_cast<double>(dateTime.toSecsSinceEpoch());

  return spline_interpolator_->interpolate(x_val);
}

void AlgoTradingController::buildNewtonPolynomial(int degree) {
  if (tradeData_.size() < 2) {
    qWarning() << "Not enough data points to build Newton polynomial (need at "
                  "least 2). Current count:"
               << tradeData_.size();
    newton_interpolator_.reset();
    return;
  }

  if (degree < 1) {
    qWarning() << "Degree must be at least 1. Got:" << degree;
    newton_interpolator_.reset();
    return;
  }

  // Преобразуем данные? Опрять пребразуем????
  std::vector<s21::SplinePoint> points;
  points.reserve(tradeData_.size());
  for (const auto &td : tradeData_) {
    points.push_back(
        {td.timestamp, td.close});
  }

  try {
    newton_interpolator_ =
        std::make_unique<s21::NewtonInterpolator>(points, degree);
    qDebug() << "Newton polynomial (degree" << degree
             << ") built successfully.";
  } catch (const std::exception &e) {
    qCritical() << "Error building Newton polynomial:" << e.what();
    newton_interpolator_.reset();
  }
}

double AlgoTradingController::getInterpolatedValueNewton(
    const QDateTime &dateTime, int degree) {
  if (degree < 1 || degree >= static_cast<int>(tradeData_.size())) {
    qWarning() << "Invalid degree for Newton interpolation:" << degree;
    return std::numeric_limits<double>::quiet_NaN();
  }

  buildNewtonPolynomial(degree);

  double x = static_cast<double>(dateTime.toSecsSinceEpoch());
  return newton_interpolator_->interpolate(x);
}

}  // namespace s21
