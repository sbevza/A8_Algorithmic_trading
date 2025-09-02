// src/algo_trading/models/spline_point.h
#ifndef A8_ALGORITHMIC_TRADING_1_SPLINE_POINT_H
#define A8_ALGORITHMIC_TRADING_1_SPLINE_POINT_H

namespace s21 {

struct SplinePoint {
  double x;
  double y;
  double weight = 1.0;
};

}  // namespace s21

#endif  // A8_ALGORITHMIC_TRADING_1_SPLINE_POINT_H
