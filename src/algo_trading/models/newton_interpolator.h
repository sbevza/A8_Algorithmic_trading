#ifndef A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
#define A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
#include <vector>

#include "spline_point.h"

class NewtonInterpolator {
 public:
  explicit NewtonInterpolator(const std::vector<s21::SplinePoint>& points);

  [[nodiscard]] double interpolate(double x_val) const;

 private:
};

#endif  // A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
