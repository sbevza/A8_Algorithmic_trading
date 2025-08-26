#ifndef A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
#define A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
#include <vector>

#include "spline_point.h"

namespace s21 {

class LeastSquaresApproximator {

public:
  explicit LeastSquaresApproximator(const std::vector<SplinePoint>& points);

  void fit(int degree);
  [[nodiscard]] double predict(double x) const;

private:
std::vector<double> x_vals_;
  std::vector<double> y_vals_;
    std::vector<double> coefficients_;
int degree_ = 1;
};

}  // namespace s21

#endif  // A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
